#pragma once
#include "../types/Achievement.h"
#include "../types/Game.h"
#include "AchievementBoxRow.h"
#include "InputAppidBoxRow.h"
#include "AppBoxRow.h"

#include <vector>
#include <map>
#include <mutex>
#include <future>
#include <gtkmm-3.0/gtkmm/applicationwindow.h>
#include <gtkmm-3.0/gtkmm/listbox.h>
#include <gtkmm-3.0/gtkmm/builder.h>
#include <gtkmm-3.0/gtkmm/box.h>
#include <gtkmm-3.0/gtkmm/searchentry.h>
#include <gtkmm-3.0/gtkmm/scrolledwindow.h>
#include <gtkmm-3.0/gtkmm/modelbutton.h>
#include <gtkmm-3.0/gtkmm/aboutdialog.h>
#include <gtkmm-3.0/gtkmm/stack.h>

#define MAX_OUTSTANDING_ICON_DOWNLOADS 10

/**
 * The main GUI class to display both the games ans the achievements to the user
 */
class MainPickerWindow : public Gtk::ApplicationWindow {
public:
    /**
     * Regular GtkMM constructor
     * https://developer.gnome.org/gtkmm-tutorial/stable/sec-builder-using-derived-widgets.html
     */
    MainPickerWindow(GtkApplicationWindow* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~MainPickerWindow();

    /**
     * Empty the game list, leaving only the placeholder widget,
     * which means the loading widget.
     */
    void reset_game_list();

    /**
     * Empty the achievements list, leaving only the placeholder widget,
     * which means the loading widget.
     */
    void reset_achievements_list();

    /**
     * Adds a game to the game list. The new item will be added and saved,
     * but not drawn.
     */
    void add_to_game_list(const Game_t& app);

    /**
     * Adds an achievement to the achievement list. The new item will be added 
     * and saved, but not drawn.
     */
    void add_to_achievement_list(const Achievement_t& achievement);

    /**
     * Shows all widget that has been added to the list, removes all
     * the deleted entries from the GUI list.
     */
    void confirm_game_list();

    /**
     * Shows all widget that has been added to the list, removes all
     * the deleted entries from the GUI list.
     */
    void confirm_achievement_list();

    /**
     * When a game is added to the list, the "missing icon" is used by default
     * Once the correct icon has been retrieved, this method will be called
     * to set the right image retrieved from Steam
     */
    void refresh_app_icon(AppId_t app_id);

    /**
     * Same as above for each achievement
     */
    void refresh_achievement_icon(AppId_t app_id, std::string id);

    /**
     * Set the game_list placeholder to the no games found placeholder
     * and show it
     */
    void show_fetch_games_placeholder();

    /**
     * Set the game_list placeholder to the fetching games placeholder
     * and show it
     */
    void show_no_games_found_placeholder();

    /**
     * Set the achievement_list placeholder to the no achievements found placeholder
     * and show it
     */
    void show_fetch_achievements_placeholder();

    /**
     * Set the game_list placeholder to the fetching achievements placeholder
     * and show it
     */
    void show_no_achievements_found_placeholder();

    /**
     * Shows the achievements list instead of the game list
     */
    void switch_to_achievement_page();

    /**
     * Shows the games list instead of the stats and achievements list
     */
    void switch_to_games_page();

    /**
     * TODO:
     * All these public variables may better belong in
     * gtk_callbacks.h if that becomes a class.
     * Make them public for now so we can get to them from
     * C functions in gtk_callbacks.cpp without yet having to
     * deal with C++/C linkage differences
     * 
     * Putting std::futures in a C struct is a bad idea because C
     * runtime libraries become confused and crash the program,
     * so keep them in this C++ class for now.
     */

    /**
     * Mutex to prevent on_refresh_games_button_clicked from being reentrant
     * and allowing multiple idle threads to corrupt the main window.
     */
    std::mutex m_game_refresh_lock;
    std::mutex m_achievement_refresh_lock;

    int outstanding_icon_downloads;
    std::future<void> owned_apps_future;
    std::map<AppId_t, std::future<void>> app_icon_download_futures;

    // Achievement info for the currently running game
    std::future<void> achievements_future;
    std::future<bool> schema_parser_future;
    std::map<std::string, std::future<void>> achievement_icon_download_futures;

private:
    // Gtk Callbacks
    void on_game_search_changed();
    void on_achievement_search_changed();
    void on_game_row_activated(Gtk::ListBoxRow* row);
    void on_refresh_games_button_clicked();
    void on_refresh_achievements_button_clicked();
    void on_back_button_clicked();
    void on_store_button_clicked();
    void on_invert_all_achievements_button_clicked();
    void on_lock_all_achievements_button_clicked();
    void on_unlock_all_achievements_button_clicked();
    void on_about_button_clicked();
    void on_close_about_dialog(int response_id);
    bool on_delete(GdkEventAny* evt);

    // Member variables
    Glib::RefPtr<Gtk::Builder> m_builder;
    Gtk::AboutDialog *m_about_dialog;
    Gtk::Button *m_back_button;
    Gtk::Button *m_store_button;
    Gtk::Button *m_refresh_games_button;
    Gtk::ModelButton *m_about_button;
    Gtk::ModelButton *m_refresh_achievements_button;
    Gtk::ModelButton *m_unlock_all_achievements_button;
    Gtk::ModelButton *m_lock_all_achievements_button;
    Gtk::ModelButton *m_invert_all_achievements_button;
    Gtk::ListBox *m_game_list;
    Gtk::ListBox *m_achievement_list;
    Gtk::Stack *m_main_stack;
    Gtk::SearchEntry *m_game_search_bar;
    Gtk::SearchEntry *m_achievement_search_bar;
    Gtk::ScrolledWindow *m_game_list_view;
    Gtk::ScrolledWindow *m_achievement_list_view;
    Gtk::Box *m_fetch_games_placeholder;
    Gtk::Box *m_no_games_found_placeholder;
    Gtk::Box *m_fetch_achievements_placeholder;
    Gtk::Box *m_no_achievements_found_placeholder;
    
    InputAppidBoxRow m_input_appid_row;

    std::vector<AppBoxRow*> m_app_list_rows;
    std::vector<AchievementBoxRow*> m_achievement_list_rows;
};