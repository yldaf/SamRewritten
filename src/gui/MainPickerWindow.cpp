#include "MainPickerWindow.h"
#include "../common/functions.h"
#include "../common/PerfMon.h"
#include "../globals.h"
#include "../types/Achievement.h"
#include "../controller/MySteam.h"

#include <iostream>


MainPickerWindow::MainPickerWindow(GtkApplicationWindow* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::ApplicationWindow(cobject),
  m_builder(builder),
  m_async_loader(this)
{
    // Load the required widgets through the builder
    m_builder->get_widget("game_list", m_game_list);
    m_builder->get_widget("game_search_bar", m_game_search_bar);
    m_builder->get_widget("achievement_search_bar", m_achievement_search_bar);
    m_builder->get_widget("achievement_list", m_achievement_list);
    m_builder->get_widget("about_dialog", m_about_dialog);
    m_builder->get_widget("main_stack", m_main_stack);
    m_builder->get_widget("game_list_view", m_game_list_view);
    m_builder->get_widget("achievement_list_view", m_achievement_list_view);
    m_builder->get_widget("back_button", m_back_button);
    m_builder->get_widget("store_button", m_store_button);
    m_builder->get_widget("refresh_games_button", m_refresh_games_button);
    m_builder->get_widget("about_button", m_about_button);
    m_builder->get_widget("refresh_achievements_button", m_refresh_achievements_button);
    m_builder->get_widget("unlock_all_achievements_button", m_unlock_all_achievements_button);
    m_builder->get_widget("lock_all_achievements_button", m_lock_all_achievements_button);
    m_builder->get_widget("invert_all_achievements_button", m_invert_all_achievements_button);
    m_builder->get_widget("fetch_games_placeholder", m_fetch_games_placeholder);
    m_builder->get_widget("no_games_found_placeholder", m_no_games_found_placeholder);
    m_builder->get_widget("fetch_achievements_placeholder", m_fetch_achievements_placeholder);
    m_builder->get_widget("no_achievements_found_placeholder", m_no_achievements_found_placeholder);

    // Connect them manually to slots
    signal_delete_event().connect(sigc::mem_fun(this, &MainPickerWindow::on_delete));
    signal_show().connect(sigc::mem_fun(this, &MainPickerWindow::on_refresh_games_button_clicked));
    m_game_list->signal_row_activated().connect(sigc::mem_fun(this, &MainPickerWindow::on_game_row_activated));
    m_game_search_bar->signal_search_changed().connect(sigc::mem_fun(this, &MainPickerWindow::on_game_search_changed));
    m_achievement_search_bar->signal_search_changed().connect(sigc::mem_fun(this, &MainPickerWindow::on_achievement_search_changed));
    m_refresh_games_button->signal_clicked().connect(sigc::mem_fun(this, &MainPickerWindow::on_refresh_games_button_clicked));
    m_refresh_achievements_button->signal_clicked().connect(sigc::mem_fun(this, &MainPickerWindow::on_refresh_achievements_button_clicked));
    m_back_button->signal_clicked().connect(sigc::mem_fun(this, &MainPickerWindow::on_back_button_clicked));
    m_store_button->signal_clicked().connect(sigc::mem_fun(this, &MainPickerWindow::on_store_button_clicked));
    m_about_button->signal_clicked().connect(sigc::mem_fun(this, &MainPickerWindow::on_about_button_clicked));
    m_invert_all_achievements_button->signal_clicked().connect(sigc::mem_fun(this, &MainPickerWindow::on_invert_all_achievements_button_clicked));
    m_lock_all_achievements_button->signal_clicked().connect(sigc::mem_fun(this, &MainPickerWindow::on_lock_all_achievements_button_clicked));
    m_unlock_all_achievements_button->signal_clicked().connect(sigc::mem_fun(this, &MainPickerWindow::on_unlock_all_achievements_button_clicked));
    m_about_dialog->signal_response().connect(sigc::mem_fun(this, &MainPickerWindow::on_close_about_dialog));

    // Finishing touches
    m_game_list->insert(m_input_appid_row, -1);
    show_fetch_games_placeholder();
}
// => Constructor

MainPickerWindow::~MainPickerWindow() {}
// => Destructor

/**
 * ======================================================
 *                     S I G N A L S
 * ======================================================
 */

void
MainPickerWindow::on_game_search_changed() {
    const std::string filter_text = m_game_search_bar->get_text();
    m_game_list->show_all();
    m_input_appid_row.hide();

    if ( filter_text.empty() ) {
        return;
    }

    if ( digits_only(filter_text) ) {
        m_input_appid_row.show_all();
        m_input_appid_row.set_appid(filter_text);
    }

    for ( AppBoxRow* it : m_app_list_rows )
    {
        std::string app_name = it->get_app().app_name;

        //Holy shit C++, why can't you even do case insensitive comparisons wtf
        //strstri is just a shitty workaround there's no way to do this properly
        if( !strstri(app_name, filter_text) ) {
            it->hide();
        }
    }
}
// => on_game_search_changed

void
MainPickerWindow::on_achievement_search_changed() {
    const std::string filter_text = m_achievement_search_bar->get_text();
    m_achievement_list->show_all();
    
    if ( filter_text.empty() ) {
        return;
    }

    for ( AchievementBoxRow* row : m_achievement_list_rows ) {    
        if ( !strstri(row->get_achievement().name, filter_text) ) {
            row->hide();
        }
    }
}
// => on_achievement_search_changed

void
MainPickerWindow::on_game_row_activated(Gtk::ListBoxRow* row) {
    AppBoxRow* game_row = static_cast<AppBoxRow*>(row);
    AppId_t appid = game_row->get_app().app_id;
    
    if ( appid == 0 ) {
        std::cerr << "An error occurred figuring out which app to launch.. You can report this to the developer." << std::endl;
        return;
    }

    switch_to_achievement_page();
    g_steam->launch_game(appid);
    m_async_loader.populate_achievements();
}
// => on_game_row_activated

void
MainPickerWindow::on_refresh_achievements_button_clicked() {
    g_steam->clear_changes();
    m_async_loader.populate_achievements();
}
// => on_refresh_achievements_button_clicked

void
MainPickerWindow::on_refresh_games_button_clicked() {
    m_game_search_bar->set_text("");
    m_async_loader.populate_apps();
}
// => on_refresh_games_button_clicked

void
MainPickerWindow::on_store_button_clicked() {
    g_steam->commit_changes();
    m_async_loader.populate_achievements();
}
// => on_store_button_clicked

void
MainPickerWindow::on_back_button_clicked() {
    g_steam->quit_game();
    switch_to_games_page();
}
// => on_back_button_clicked

void
MainPickerWindow::on_invert_all_achievements_button_clicked() {
    for ( AchievementBoxRow* row : m_achievement_list_rows )
    {
        row->invert_scripted();
    }
}
// => on_invert_all_achievements_button_clicked

void
MainPickerWindow::on_lock_all_achievements_button_clicked() {
    for ( AchievementBoxRow* row : m_achievement_list_rows )
    {
        row->lock_scripted();
    }
}
// => on_lock_all_achievements_button_clicked

void
MainPickerWindow::on_unlock_all_achievements_button_clicked() {
    for ( AchievementBoxRow* row : m_achievement_list_rows )
    {
        row->unlock_scripted();
    }
}
// => on_unlock_all_achievements_button_clicked

bool
MainPickerWindow::on_delete(GdkEventAny* evt) {
    hide();
    g_steam->quit_game();
    return false;
}
// => on_delete

void
MainPickerWindow::on_about_button_clicked() {
    m_about_dialog->show();
}
// => on_about_button_clicked

void
MainPickerWindow::on_close_about_dialog(int response_id) {
    // Not sure what to do with the response
    // https://developer.gnome.org/gtk3/stable/GtkDialog.html#GtkResponseType
    m_about_dialog->hide();
}
// => on_close_about_dialog

/**
 * ======================================================
 *               C O N T R O L L E R S
 * ======================================================
 */

void 
MainPickerWindow::reset_game_list() {
    for ( AppBoxRow* row : m_app_list_rows )
    {
        delete row;
        row = nullptr;
    }

    m_app_list_rows.clear();
}
// => reset_game_list

void
MainPickerWindow::reset_achievements_list() {
    for ( AchievementBoxRow* row : m_achievement_list_rows) {    
        delete row;
        row = nullptr;
    }

    m_achievement_list_rows.clear();
}
// => reset_achievements_list

/**
 * Add a game to the list.
 * Some logic remains from the C implementation, but we keep a copy of the 
 * row pointers to access or filter them easily by appid or name
 * The new entry is not shown yet, call confirm_game_list for that.
 */
void 
MainPickerWindow::add_to_game_list(const Game_t& app) {
    AppBoxRow* row = new AppBoxRow(app);
    m_game_list->insert(*row, -1);
    m_app_list_rows.push_back(row);
}
// => add_to_game_list

void
MainPickerWindow::add_to_achievement_list(const Achievement_t& achievement) {
    AchievementBoxRow* row = new AchievementBoxRow(achievement);
    m_achievement_list->insert(*row, -1);
    m_achievement_list_rows.push_back(row);
}
// => add_to_achievement_list

/**
 * Draws all the achievements that have not been shown yet
 */
void
MainPickerWindow::confirm_achievement_list() {
    m_achievement_list->show_all();
}
// => confirm_achievement_list

/**
 * Draws all the games that have not been shown yet
 */
void 
MainPickerWindow::confirm_game_list() {
    m_game_list->show_all();
    m_input_appid_row.hide();
}
// => confirm_game_list

/**
 * Refreshes the icon for the specified app ID
 * As per GTK, this must only ever be called from the main thread.
 */
void
MainPickerWindow::refresh_app_icon(AppId_t app_id) {
    const std::string path = get_app_icon_path(app_id);

    for ( AppBoxRow* i : m_app_list_rows )
    {
        if ( i->get_app().app_id == app_id )
        {
            i->set_icon(path, 146, 68);
            return;
        }
    }
}
// => refresh_app_icon

/**
 * Refreshes the icon for the specified achievement icon
 * Requires app_id information to know where to find the icon
 */
void 
MainPickerWindow::refresh_achievement_icon(AppId_t app_id, std::string id) {
    const std::string path = get_achievement_icon_path(app_id, id);

    // replace_icon(path, m_achievement_list_rows[id]->get_main_widget(), 64, 64);
    for ( AchievementBoxRow* i : m_achievement_list_rows )
    {
        if ( i->get_achievement().id == id ) {
            i->set_icon(path, 64, 64);
        }
    }
}
// => refresh_achievement_icon

void
MainPickerWindow::show_fetch_games_placeholder() {
    m_game_list->set_placeholder(*m_fetch_games_placeholder);
    m_fetch_games_placeholder->show();
}
// => show_fetch_games_placeholder

void
MainPickerWindow::show_no_games_found_placeholder() {
    m_game_list->set_placeholder(*m_no_games_found_placeholder);
    m_no_games_found_placeholder->show();
}
// => show_no_games_found_placeholder

void
MainPickerWindow::show_fetch_achievements_placeholder() {
    m_achievement_list->set_placeholder(*m_fetch_achievements_placeholder);
    m_fetch_achievements_placeholder->show();
}
// => show_fetch_achievements_placeholder

void
MainPickerWindow::show_no_achievements_found_placeholder() {
    m_achievement_list->set_placeholder(*m_no_achievements_found_placeholder);
    m_no_achievements_found_placeholder->show();
}
// => show_no_achievements_found_placeholder

void
MainPickerWindow::switch_to_achievement_page() {
    m_back_button->set_visible(true);
    m_achievement_search_bar->set_visible(true);
    m_store_button->set_visible(true);
    m_refresh_achievements_button->set_visible(true);
    m_unlock_all_achievements_button->set_visible(true);
    m_lock_all_achievements_button->set_visible(true);
    m_invert_all_achievements_button->set_visible(true);

    m_refresh_games_button->set_visible(false);
    m_game_search_bar->set_visible(false);

    m_main_stack->set_visible_child(*m_achievement_list_view);
}
// => switch_to_achievement_page

void
MainPickerWindow::switch_to_games_page() {
    m_back_button->set_visible(false);
    m_achievement_search_bar->set_visible(false);
    m_store_button->set_visible(false);
    m_refresh_achievements_button->set_visible(false);
    m_unlock_all_achievements_button->set_visible(false);
    m_lock_all_achievements_button->set_visible(false);
    m_invert_all_achievements_button->set_visible(false);

    m_refresh_games_button->set_visible(true);
    m_game_search_bar->set_visible(true);

    m_main_stack->set_visible_child(*m_game_list_view);
    m_achievement_search_bar->set_text("");
    reset_achievements_list();
}
// => switch_to_games_page