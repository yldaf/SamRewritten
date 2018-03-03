#pragma once

#include <gtk/gtk.h>
#include <iostream>
#include <map>
#include "../common/functions.h"
#include "globals.h"
#include "Game.h"
#include "gtk_callbacks.h"

/**
 * The main GUI class to display both the games ans the achievements to the user
 */
class MainPickerWindow {
public:
    //Todo: destructor

    /**
     * The constructor loads the gtk builder, the main elements,
     * and shows the main window.
     */
    MainPickerWindow();

    /**
     * Empty the game list, leaving only the placeholder widget,
     * which means the loading widget.
     */
    void reset_game_list();

    /**
     * Adds a game to the game list. The new item will be added and saved,
     * but not drawn.
     */
    void add_to_game_list(const Game_t& app);

    /**
     * Commits all the changes made since the last commit
     * Shows all widget that has been added to the list, removes all
     * the deleted entries from the GUI list.
     */
    void confirm_game_list();

    /**
     * When a game is added to the list, the "missing icon" is used by default
     * Once the correct icon has been retrieved, this method will be called
     * to set the right image retrieved from Steam
     */
    void refresh_app_icon(const unsigned long app_id);

    /**
     * Filters the game list. For a title to stay displayed,
     * filter_text must be included in it
     */
    void filter_games(const char* filter_text);

    /**
     * Getter for the main window
     */
    GtkWidget* get_main_window() { return m_main_window; };

private:
    GtkWidget *m_main_window;
    GtkListBox *m_game_list;  
    GtkBuilder *m_builder;
    std::map<unsigned long, GtkWidget*> m_rows;
};