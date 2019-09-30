#include "gtk_callbacks.h"
#include <iostream>
#include <thread>
#include "MainPickerWindow.h"
#include "../common/PerfMon.h"
#include "../MySteam.h"
#include "../globals.h"

// See comments in the header file

extern "C" 
{

    void
    populate_achievements() {
        // Get_achievements from game server
        std::vector<Achievement_t> achievements = g_steam->get_achievements();

        g_main_gui->reset_achievements_list();

        //TODO: just pass in the array directly?
        for(Achievement_t achievement : achievements) {
            g_main_gui->add_to_achievement_list(achievement);
        }

        // TODO: these are not stats they are general achievements
        g_main_gui->confirm_stats_list();
    }
    // => populate_achievements

    void 
    on_close_button_clicked() {
        g_main_gui->stop();
        
        delete g_main_gui;
        g_main_gui = nullptr;

        g_steam->quit_game();
    }
    // => on_close_button_clicked

    void
    on_store_button_clicked() {
        std::cerr << "Saving stats and achievements." << std::endl;

        g_steam->commit_changes();

        populate_achievements();
    }
    // => on_store_button_clicked

    /* the actual loading function */
    static gboolean
    load_items_idle (gpointer data_)
    {
        IdleData *data = (IdleData *)data_;
            
        // To improve launch-to-window-show time, relinquish some cycles
        // to the main loop. Limit is experimentally determined.
        // Only do it for the first load, subsequent refreshes will not
        // be affected.
        static int wait_cycles_counter = 0;
        if (wait_cycles_counter < 500) {
            wait_cycles_counter++;
            return G_SOURCE_CONTINUE;
        }

        if (data->state == STATE_STARTED)
        {
            // Trigger updates for data initialization here
            // just to get it out of the critical path for
            // showing the window.
            // TODO: Further split out the app_is_owned loop
            // in refresh_owned_apps to give better responsiveness
            g_perfmon->log("Starting library parsing.");
            g_main_gui->reset_game_list();
            g_steam->refresh_owned_apps();
            data->state = STATE_LOADING;
        }

        if (data->current_item == g_steam->get_subscribed_apps().size()) {
            data->state = STATE_FINISHED;
            return G_SOURCE_REMOVE;
        }

        Game_t app  = g_steam->get_subscribed_apps()[data->current_item];
        g_main_gui->add_to_game_list(app);
        data->current_item++;

        return G_SOURCE_CONTINUE;
    }
    // => load_items_idle

    /* the finish function */
    static void
    finish_load_items (gpointer data_)
    {
        IdleData *data = (IdleData *)data_;

        // This must occur after the main gui game_list is
        // complete, otherwise we might have concurrent
        // access and modification of the game_list
        g_steam->refresh_icons();
        g_main_gui->confirm_game_list();
        g_perfmon->log("Library parsed.");

        g_free (data);
    }
    // => finish_load_items

    void 
    on_ask_game_refresh() {
        // TODO: we may want some mutexes around here to prevent
        // double clicking refresh button from corrupting the main
        // window

        IdleData *data;

        data = g_new(IdleData, 1);
        data->current_item = 0;
        data->state = STATE_STARTED;

        // Use low priority so we don't block showing the main window
        // This allows the main window to show up immediately
        g_idle_add_full (G_PRIORITY_LOW,
                        load_items_idle,
                        data,
                        finish_load_items);

    }
    // => on_ask_game_refresh

    void 
    on_main_window_show() {
        on_ask_game_refresh();
    }
    // => on_main_window_show


    void
    on_search_changed(GtkWidget* search_widget) {
        const char* filter_text = gtk_entry_get_text( GTK_ENTRY(search_widget) );

        //if !g_steam->isgamerunning
        g_main_gui->filter_games(filter_text);
        //else 
        //g_main_gui->filter_stats(filter_text)
    }
    // => on_search_changed


    void 
    on_game_row_activated(GtkListBox *box, GtkListBoxRow *row) {

        const AppId_t appId = g_main_gui->get_corresponding_appid_for_row(row);

        if( appId != 0 ) {
            g_main_gui->switch_to_stats_page();
            g_steam->launch_game(appId);
            populate_achievements();
        } else {
            std::cerr << "An error occurred figuring out which app to launch.. You can report this to the developer." << std::endl;
        }

    }
    // => on_game_row_activated

    void
    on_back_button_clicked() {
        g_steam->quit_game();
        g_main_gui->switch_to_games_page();
    }
    // => on_back_button_clicked
}