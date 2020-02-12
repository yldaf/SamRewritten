#include "AsyncGuiLoader.h"

#include "../controller/MySteam.h"
#include "../common/PerfMon.h"
#include "../globals.h"
#include "MainPickerWindow.h"

#include <iostream>
#include <future>
#include <glibmm-2.4/glibmm.h>

// TODO use #ifndef __VALGRIND_H or similar
// #include <valgrind/valgrind.h>

AsyncGuiLoader::AsyncGuiLoader(MainPickerWindow* window)
: m_window(window)
{

}

bool
AsyncGuiLoader::load_achievements_idle()
{
    if (m_achievement_idle_data.state == ACH_STATE_STARTED) {
        g_perfmon->log("Starting achievement retrieval");
        m_achievements_future = std::async(std::launch::async, []{g_steam->refresh_stats_and_achievements();});
        m_achievement_idle_data.state = ACH_STATE_WAITING_FOR_ACHIEVEMENTS;
        return G_SOURCE_CONTINUE;
    }

    if (m_achievement_idle_data.state == ACH_STATE_WAITING_FOR_ACHIEVEMENTS) {
        if (m_achievements_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            g_perfmon->log("Done retrieving achievements");

            #if DEBUG_CERR
            std::cerr << "Printing achievement ID & icon:" << std::endl;
            for (auto a : g_steam->get_achievements()) {
                std::cerr << a.id << " " << a.icon_name << std::endl;
            }
            #endif

            // Fire off the schema parsing now.
            // TODO: figure out if all the icons are already there and skip parsing schema
            // TODO: We want schema parsing to be done serverside now, and pass the result via IPC
            m_achievement_idle_data.state = ACH_STATE_LOADING_GUI;
        }
        return G_SOURCE_CONTINUE;
    }

    if (m_achievement_idle_data.state == ACH_STATE_LOADING_GUI) {
        if (m_achievement_idle_data.current_item == g_steam->get_achievements().size()) {
            g_perfmon->log("Done adding achievements to GUI");
            m_window->confirm_achievement_list();

            if ( g_steam->get_achievements().size() > 1000 ) {
                // The game is an achievement printer of some kind, downloading the icons
                // leads to serious performance issues and takes a lot of cache
                std::cerr << "App has excessive achievements, skipping icon downloads" << std::endl;
                g_perfmon->log("Achievements retrieved, no icons (Achievement farming app).");
                m_achievement_idle_data.state = ACH_STATE_FINISHED;
                m_window->show_no_achievements_found_placeholder();
                m_achievement_refresh_lock.unlock();
                return G_SOURCE_REMOVE;
            }

            m_achievement_idle_data.state = ACH_STATE_DOWNLOADING_ICONS;
            m_achievement_idle_data.current_item = 0;
            return G_SOURCE_CONTINUE;
        }

        auto achievement = g_steam->get_achievements()[m_achievement_idle_data.current_item];
        m_window->add_to_achievement_list(achievement);
        m_achievement_idle_data.current_item++;
        return G_SOURCE_CONTINUE;
    }

    if (m_achievement_idle_data.state == ACH_STATE_DOWNLOADING_ICONS) {
        // this could hang if we failed to parse all the icon download names
        bool done_starting_downloads = (m_achievement_idle_data.current_item == g_steam->get_achievements().size());

        if (done_starting_downloads && (m_achievement_icon_download_futures.size() == 0)) {
            m_achievement_idle_data.state = ACH_STATE_FINISHED;
            g_perfmon->log("Achievements retrieved with icons.");
            m_window->show_no_achievements_found_placeholder();
            m_achievement_refresh_lock.unlock();

            // See top of the file
            // VALGRIND_MONITOR_COMMAND("detailed_snapshot");

            return G_SOURCE_REMOVE;
        }

        if ( !done_starting_downloads && (m_concurrent_icon_downloads < MAX_CONCURRENT_ICON_DOWNLOADS))  {
            const Achievement_t ach = g_steam->get_achievements()[m_achievement_idle_data.current_item];
            
            // Fire off a new download thread
            std::string id = ach.id;
            std::string icon_download_name = ach.icon_name;

            // Assuming it returns empty string on failing to lookup
            if (icon_download_name.empty()) {
                std::cerr << "Failed to lookup achievement icon name: " << id << std::endl;
            } else {
                m_achievement_icon_download_futures.insert(std::make_pair(
                    id, std::async(std::launch::async, [id, icon_download_name]{g_steam->refresh_achievement_icon(id, icon_download_name);})));
                m_concurrent_icon_downloads++;
            }

            m_achievement_idle_data.current_item++;

            // continue on to service a thread if it's finished
        }

        for (auto const& [id, this_future] : m_achievement_icon_download_futures) {
            if (this_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                m_window->refresh_achievement_icon(g_steam->get_current_appid(), id);
                m_achievement_icon_download_futures.erase(id);
                m_concurrent_icon_downloads--;
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
    if (m_achievement_refresh_lock.try_lock()) {
        m_achievement_idle_data.current_item = 0;
        m_achievement_idle_data.state = ACH_STATE_STARTED;
        m_concurrent_icon_downloads = 0;
        m_window->reset_achievements_list();
        m_window->show_fetch_achievements_placeholder();

        Glib::signal_idle().connect(sigc::mem_fun(this, &AsyncGuiLoader::load_achievements_idle), G_PRIORITY_LOW);

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
    * See the AsyncGuiLoader.h for the FSM rationale.
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
    if (m_app_idle_data.state == APPS_STATE_STARTED) {
        m_window->reset_game_list();
        g_perfmon->log("Starting library parsing.");
        m_owned_apps_future = std::async(std::launch::async, []{g_steam->refresh_owned_apps();});
        m_app_idle_data.state = APPS_STATE_WAITING_FOR_OWNED_APPS;
        return G_SOURCE_CONTINUE;
    }

    if (m_app_idle_data.state == APPS_STATE_WAITING_FOR_OWNED_APPS) {
        if (m_owned_apps_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            g_perfmon->log("Done retrieving and filtering owned apps");
            m_app_idle_data.state = APPS_STATE_LOADING_GUI;
        }
        return G_SOURCE_CONTINUE;
    }

    if (m_app_idle_data.state == APPS_STATE_LOADING_GUI) {
        if (m_app_idle_data.current_item == g_steam->get_subscribed_apps().size()) {
            g_perfmon->log("Done adding apps to GUI");
            m_window->confirm_game_list();
            m_app_idle_data.state = APPS_STATE_DOWNLOADING_ICONS;
            m_app_idle_data.current_item = 0;
            return G_SOURCE_CONTINUE;
        }

        Game_t app = g_steam->get_subscribed_apps()[m_app_idle_data.current_item];
        m_window->add_to_game_list(app);
        m_app_idle_data.current_item++;
        return G_SOURCE_CONTINUE;
    }

    if (m_app_idle_data.state == APPS_STATE_DOWNLOADING_ICONS) {
        // This must occur after the main gui game_list is
        // complete, otherwise we might have concurrent
        // access and modification of the GUI's game_list

        bool done_starting_downloads = (m_app_idle_data.current_item == g_steam->get_subscribed_apps().size());

        // Make sure we're done starting all downloads and finshed with outstanding downloads
        if (done_starting_downloads && (m_app_icon_download_futures.size() == 0)) {
            g_perfmon->log("Done downloading app icons");
            m_app_idle_data.state = APPS_STATE_FINISHED;
            m_window->show_no_games_found_placeholder();
            m_game_refresh_lock.unlock();
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
        if ( !done_starting_downloads && (m_concurrent_icon_downloads < MAX_CONCURRENT_ICON_DOWNLOADS))  {
            // Fire off a new download thread
            Game_t app = g_steam->get_subscribed_apps()[m_app_idle_data.current_item];
            m_app_icon_download_futures.insert(std::make_pair(app.app_id, std::async(std::launch::async, g_steam->refresh_app_icon, app.app_id)));
            m_concurrent_icon_downloads++;
            m_app_idle_data.current_item++;

            // continue on to service a thread if it's finished
        }

        // Try to find a thread that is finished. Only process at most 1 per GTK main loop.
        // The max time this takes to traverse is controlled by the size of the
        // app_icon_download_futures size, which is controlled by MAX_ICON_DOWNLOADS.
        // Increasing this could lead to GUI stutter if it needs to traverse a large map,
        // although the map has logarithmic traversal and update complexity.
        for (auto const& [app_id, this_future] : m_app_icon_download_futures) {
            if (this_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                m_window->refresh_app_icon(app_id);
                m_app_icon_download_futures.erase(app_id);
                m_concurrent_icon_downloads--;
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
AsyncGuiLoader::populate_apps() {
    if (m_game_refresh_lock.try_lock()) {
        m_app_idle_data.current_item = 0;
        m_app_idle_data.state = APPS_STATE_STARTED;
        m_concurrent_icon_downloads = 0;
        m_window->show_fetch_games_placeholder();

        // Use low priority so we don't block showing the main window
        // This allows the main window to show up immediately
        Glib::signal_idle().connect(sigc::mem_fun(this, &AsyncGuiLoader::load_apps_idle), G_PRIORITY_LOW);
    } else {
        std::cerr << "Not refreshing games because a refresh is already in progress" << std::endl;
    }
}
// => on_refresh_games_button_clicked
