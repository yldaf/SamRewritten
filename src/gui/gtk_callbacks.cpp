#include "gtk_callbacks.h"
#include <iostream>
#include <future>
#include <glibmm-2.4/glibmm.h>
#include "MainPickerWindow.h"
#include "../common/PerfMon.h"
#include "../MySteam.h"
#include "../globals.h"

// See comments in the header file

// see comments in load_apps_idle
bool
AsyncGuiLoader::load_achievements_idle()
{
    if (m_idle_data.state == ACH_STATE_STARTED) {
        g_perfmon->log("Starting achievement retrieval");
        g_main_gui->achievements_future = std::async(std::launch::async, []{g_steam->refresh_achievements();});
        m_idle_data.state = ACH_STATE_WAITING_FOR_ACHIEVEMENTS;
        return G_SOURCE_CONTINUE;
    }

    if (m_idle_data.state == ACH_STATE_WAITING_FOR_ACHIEVEMENTS) {
        if (g_main_gui->achievements_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            g_perfmon->log("Done retrieving achievements");

            // Fire off the schema parsing now.
            // It will modify g_steam->m_icon_download_names directly
            // TODO: figure out if all the icons are already there and skip parsing schema
            g_main_gui->schema_parser_future = std::async(std::launch::async, [this]{return m_schema_parser.load_user_game_stats_schema();});
            m_idle_data.state = ACH_STATE_LOADING_GUI;
        }
        return G_SOURCE_CONTINUE;
    }

    if (m_idle_data.state == ACH_STATE_LOADING_GUI) {
        if (m_idle_data.current_item == g_steam->get_achievements().size()) {
            g_perfmon->log("Done adding achievements to GUI");
            g_main_gui->confirm_achievement_list();
            m_idle_data.state = ACH_STATE_WAITING_FOR_SCHEMA_PARSER;
            m_idle_data.current_item = 0;
            return G_SOURCE_CONTINUE;
        }

        auto achievement = g_steam->get_achievements()[m_idle_data.current_item];
        g_main_gui->add_to_achievement_list(achievement);
        m_idle_data.current_item++;
        return G_SOURCE_CONTINUE;
    }

    if (m_idle_data.state == ACH_STATE_WAITING_FOR_SCHEMA_PARSER) {
        if (g_main_gui->schema_parser_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            g_perfmon->log("Done parsing schema to find achievement icon download names");
            if (!g_main_gui->schema_parser_future.get()) {
                std::cerr << "Schema parsing failed, skipping icon downloads" << std::endl;
                m_idle_data.state = ACH_STATE_FINISHED;
                g_perfmon->log("Achievements retrieved, no icons.");
                g_main_gui->show_no_achievements_found_placeholder();
                g_main_gui->m_achievement_refresh_lock.unlock();
                return G_SOURCE_REMOVE;
            }

            m_idle_data.state = ACH_STATE_DOWNLOADING_ICONS;
        }
        return G_SOURCE_CONTINUE;
    }

    if (m_idle_data.state == ACH_STATE_DOWNLOADING_ICONS) {
        // this could hang if we failed to parse all the icon download names
        bool done_starting_downloads = (m_idle_data.current_item == g_steam->get_achievements().size());

        if (done_starting_downloads && (g_main_gui->achievement_icon_download_futures.size() == 0)) {
                m_idle_data.state = ACH_STATE_FINISHED;
                g_perfmon->log("Achievements retrieved with icons.");
                g_main_gui->show_no_achievements_found_placeholder();
                g_main_gui->m_achievement_refresh_lock.unlock();
                return G_SOURCE_REMOVE;
        }

        if ( !done_starting_downloads && (g_main_gui->outstanding_icon_downloads < MAX_OUTSTANDING_ICON_DOWNLOADS))  {
            // Fire off a new download thread
            std::string id = g_steam->get_achievements()[m_idle_data.current_item].id;
            std::string icon_download_name = m_schema_parser.get_icon_download_names()[id];
            
            // Assuming it returns empty string on failing to lookup
            if (icon_download_name.empty()) {
                std::cerr << "Failed to lookup achievement icon name: " << id << std::endl;
            } else {
                g_main_gui->achievement_icon_download_futures.insert(std::make_pair(
                    id, std::async(std::launch::async, [id, icon_download_name]{g_steam->refresh_achievement_icon(id, icon_download_name);})));
                g_main_gui->outstanding_icon_downloads++;
            }

            m_idle_data.current_item++;

            // continue on to service a thread if it's finished
        }

        for (auto const& [id, this_future] : g_main_gui->achievement_icon_download_futures) {
            if (this_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                g_main_gui->refresh_achievement_icon(g_steam->get_current_appid(), id);
                g_main_gui->achievement_icon_download_futures.erase(id);
                g_main_gui->outstanding_icon_downloads--;
                // let's only process one at a time
                return G_SOURCE_CONTINUE;
            }
        }

        // return G_SOURCE_CONTINUE;
    }

    // Should never reach here
    return G_SOURCE_CONTINUE;
}
// => load_achievements_idle

void
AsyncGuiLoader::populate_achievements() {
    if (g_main_gui->m_achievement_refresh_lock.try_lock()) {
        m_idle_data.current_item = 0;
        m_idle_data.state = ACH_STATE_STARTED;
        g_main_gui->outstanding_icon_downloads = 0;
        g_main_gui->reset_achievements_list();
        g_main_gui->show_fetch_achievements_placeholder();

        Glib::signal_idle().connect(sigc::mem_fun(this, &AsyncGuiLoader::load_achievements_idle), G_PRIORITY_LOW);
        // g_idle_add_full (G_PRIORITY_LOW,
        //                 this->load_achievements_idle,
        //                 data,
        //                 finish_load_achievements);

    } else {
        std::cerr << "Not launching game/refreshing achievements because a refresh is already in progress" << std::endl;
    }
}
// => populate_achievements

/*
    * The actual loading function
    * 
    * We CANNOT update the GUI from any thread but the main thread because
    * it is explicitly deprecated in gtk...
    * (e.g. https://developer.gnome.org/gdk3/stable/gdk3-Threads.html#gdk-threads-init)
    * See the gtk_callbacks.h for the FSM rationale.
    * 
    * For anything that isn't the GUI, we can fire off worker threads, and doing
    * so is simpler than splitting out the worker threads into the main GUI loop.
    * Additionally, the worker threads depend on calling functions which may
    * take a while to return. So, splitting them out into a thread won't expose the
    * main loop to these latencies and potentially make it laggy.
    */
bool
AsyncGuiLoader::load_apps_idle()
{
    if (m_idle_data.state == APPS_STATE_STARTED) {
        g_main_gui->reset_game_list();
        g_perfmon->log("Starting library parsing.");
        g_main_gui->owned_apps_future = std::async(std::launch::async, []{g_steam->refresh_owned_apps();});
        m_idle_data.state = APPS_STATE_WAITING_FOR_OWNED_APPS;
        return G_SOURCE_CONTINUE;
    }

    if (m_idle_data.state == APPS_STATE_WAITING_FOR_OWNED_APPS) {
        if (g_main_gui->owned_apps_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            g_perfmon->log("Done retrieving and filtering owned apps");
            m_idle_data.state = APPS_STATE_LOADING_GUI;
        }
        return G_SOURCE_CONTINUE;
    }

    if (m_idle_data.state == APPS_STATE_LOADING_GUI) {
        if (m_idle_data.current_item == g_steam->get_subscribed_apps().size()) {
            g_perfmon->log("Done adding apps to GUI");
            g_main_gui->confirm_game_list();
            m_idle_data.state = APPS_STATE_DOWNLOADING_ICONS;
            m_idle_data.current_item = 0;
            return G_SOURCE_CONTINUE;
        }

        Game_t app = g_steam->get_subscribed_apps()[m_idle_data.current_item];
        g_main_gui->add_to_game_list(app);
        m_idle_data.current_item++;
        return G_SOURCE_CONTINUE;
    }

    if (m_idle_data.state == APPS_STATE_DOWNLOADING_ICONS) {
        // This must occur after the main gui game_list is
        // complete, otherwise we might have concurrent
        // access and modification of the GUI's game_list

        bool done_starting_downloads = (m_idle_data.current_item == g_steam->get_subscribed_apps().size());

        // Make sure we're done starting all downloads and finshed with outstanding downloads
        if (done_starting_downloads && (g_main_gui->app_icon_download_futures.size() == 0)) {
            g_perfmon->log("Done downloading app icons");
            m_idle_data.state = APPS_STATE_FINISHED;
            g_main_gui->show_no_games_found_placeholder();
            g_main_gui->m_game_refresh_lock.unlock();
            return G_SOURCE_REMOVE;
        }

        // We could have the threads IPC the data back to the main thread
        // but that's a bit heavyweight for what we need here:
        // the main thread just needs to know the app_id that completed, and
        // it can figure out the rest.

        // Implement a poor man's thread pool for now - only fire off
        // a few threads at a time (though threads are not recycled).
        // We would use a semaphore here for outstanding_icon_downloads,
        // but the GTK main loop is forcibly single-threaded
        // (the whole reason we need to do these shenanigans anyway),
        // so only 1 thread will ever be here at a time anyway.
        if ( !done_starting_downloads && (g_main_gui->outstanding_icon_downloads < MAX_OUTSTANDING_ICON_DOWNLOADS))  {
            // Fire off a new download thread
            Game_t app = g_steam->get_subscribed_apps()[m_idle_data.current_item];
            g_main_gui->app_icon_download_futures.insert(std::make_pair(app.app_id, std::async(std::launch::async, g_steam->refresh_app_icon, app.app_id)));
            g_main_gui->outstanding_icon_downloads++;
            m_idle_data.current_item++;

            // continue on to service a thread if it's finished
        }

        // Try to find a thread that is finished. Only process at most 1 per GTK main loop.
        // The max time this takes to traverse is controlled by the size of the
        // app_icon_download_futures size, which is controlled by MAX_ICON_DOWNLOADS.
        // Increasing this could lead to GUI stutter if it needs to traverse a large map,
        // although the map has logarithmic traversal and update complexity.
        for (auto const& [app_id, this_future] : g_main_gui->app_icon_download_futures) {
            if (this_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                // TODO: remove the app if it has a bad icon? (because then it's mostly likely not a game)
                g_main_gui->refresh_app_icon(app_id);
                g_main_gui->app_icon_download_futures.erase(app_id);
                g_main_gui->outstanding_icon_downloads--;
                // let's only process one at a time
                return G_SOURCE_CONTINUE;
            }
        }

        return G_SOURCE_CONTINUE;
    }

    // Should never reach here
    return G_SOURCE_CONTINUE;
}
// => load_apps_idle

void 
AsyncGuiLoader::on_refresh_games_button_clicked_old() {
    if (g_main_gui->m_game_refresh_lock.try_lock()) {
        IdleData *data = g_new(IdleData, 1);
        m_idle_data.current_item = 0;
        m_idle_data.state = APPS_STATE_STARTED;
        g_main_gui->outstanding_icon_downloads = 0;
        g_main_gui->show_fetch_games_placeholder();

        // Use low priority so we don't block showing the main window
        // This allows the main window to show up immediately
        Glib::signal_idle().connect(sigc::mem_fun(this, &AsyncGuiLoader::load_apps_idle), G_PRIORITY_LOW);
    } else {
        std::cerr << "Not refreshing games because a refresh is already in progress" << std::endl;
    }
}
// => on_refresh_games_button_clicked