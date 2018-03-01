/**
 * Please read the license before modifying or distributing any of the code from
 * this project. Thank you.
 */
#include <iostream>
#include <unistd.h>
#include <gmodule.h>
#include "MySteam.h"
#include "MainPickerWindow.h"
#include "globals.h"

/**************************************
 *  Declare global variables imported from globals.h
 **************************************/
MySteam* g_steam = nullptr;
MainPickerWindow* g_main_gui = nullptr;
char* g_cache_folder = nullptr;



/**************************************
 * Main entry point
 **************************************/
int 
main(int argc, char *argv[])
{
    // Test if glib2 is installed, gtk will not work without it.
    if(!g_module_supported()) {
        std::cerr << "Sorry, but gmodules are not supported on your platform :(. Try installing as many gnome libs as you can maybe.." << std::endl;
        exit(EXIT_FAILURE);
    }
    
    gtk_init(&argc, &argv);
    
    g_cache_folder = concat(getenv("HOME"), "/.SamRewritten");
    g_steam = MySteam::get_instance();
    g_main_gui = new MainPickerWindow();

    gtk_widget_show(g_main_gui->get_main_window());
    gtk_main();

    //steam->launch_game("368230");
    //steam->quit_game();

    return 0;
}



/**************************************
 * Gtk Callbacks
 **************************************/
extern "C" 
{
    /**
     * When you click on the close button if steam is not running
     * Quit the GTK process, and destroy as much as possible: the program
     * is about to be exitted completely.
     */
    void 
    on_close_button_clicked() {
        gtk_main_quit();
        gtk_widget_destroy(g_main_gui->get_main_window());
    }

    /** 
     * When the user wants to refresh the game list.
     * This is also called when the main window just got spawned.
     * - Clear the game list (will show the loading widget)
     * - Do the backend work (get owned apps..)
     * - Add all the retrieved data to the view.
     * - Do the backend work again for icons: view must be initialized first
     *   to display the app logos.
     * - Draw the result.
     */
    void 
    on_ask_game_refresh() {
        g_main_gui->reset_game_list();
        g_steam->refresh_owned_apps();

        for(Game_t app : g_steam->get_all_games_with_stats()) {
            g_main_gui->add_to_game_list(app);
        }

        g_steam->refresh_icons();
        g_main_gui->confirm_game_list();
    }

    /** 
     * When the main window... started showing / will be showing??
     * TODO: Make sure the loading widget is showing while starting to 
     * do the backend work
     */
    void 
    on_main_window_show() {
        on_ask_game_refresh(); //Run this async?
    }

    void
    on_search_changed(GtkWidget* search_widget) {
        const char* filter_text = gtk_entry_get_text( GTK_ENTRY(search_widget) );

        //if !g_steam->isgamerunning
        g_main_gui->filter_games(filter_text);
        //else 
        //g_main_gui->filter_stats()
    }
}