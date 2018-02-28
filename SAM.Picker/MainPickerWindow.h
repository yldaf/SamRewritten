#pragma once

#include <gtk/gtk.h>
#include <iostream>
#include "Game.h"

class MainPickerWindow {
public:
    //Todo: destructor and tidy
    MainPickerWindow();
    void reset_game_list();
    void add_to_game_list(const Game_t& app);
    void confirm_game_list();
    void refresh_app_icon(const unsigned long app_id);
    GtkWidget* get_main_window() { return m_main_window; };

private:
    GtkWidget *m_main_window;
    GtkListBox *m_game_list;  
    GtkBuilder *m_builder;
};