#pragma once

#include <gtk/gtk.h>
#include <iostream>

class MainPickerWindow {
public:
    MainPickerWindow();
    void reset_game_list();
    void add_to_game_list(const char app_name[]);
    GtkWidget* get_main_window() { return m_main_window; };

private:
    GtkWidget *m_main_window;
    GtkBox *m_game_list;    
    GtkBox *m_loading_game_list_box;
};