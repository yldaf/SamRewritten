#include "AppBoxRow.h"

#include <gtkmm-3.0/gtkmm/box.h>
#include <gtkmm-3.0/gtkmm/arrow.h>
#include <gtkmm-3.0/gtkmm/label.h>
#include <gtkmm-3.0/gtkmm/button.h>
#include "../common/GtkDefineHelper.h"
#include <iostream>

 #define APP_BOX_ROW_HEIGHT 80

AppBoxRow::AppBoxRow(const Game_t& app)
: m_app(app)
{
    set_size_request(-1, APP_BOX_ROW_HEIGHT);
    set_missing();
    Gtk::Box* layout = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_HORIZONTAL, 0);
    Gtk::Label* label = Gtk::make_managed<Gtk::Label>(app.app_name);
    Gtk::Button* new_window_button = Gtk::make_managed<Gtk::Button>();
    new_window_button->set_image_from_icon_name("window-new-symbolic", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
    new_window_button->set_tooltip_text("Launch game in a new window of SamRewritten");
    // Make this square, depends on APP_BOX_ROW_HEIGHT
    new_window_button->set_margin_top(20);
    new_window_button->set_margin_bottom(20);
    new_window_button->set_size_request(40, 40);
    Gtk::Arrow* arrow = Gtk::make_managed<Gtk::Arrow>(Gtk::ArrowType::ARROW_RIGHT, Gtk::ShadowType::SHADOW_OUT);
    arrow->set_margin_start(40);

    layout->pack_start(m_icon, false, false, 0);
    layout->pack_start(*label, true, true, 0);
    layout->pack_start(*new_window_button, false, false, 0);
    layout->pack_start(*arrow, false, false, 0);

    new_window_button->signal_clicked().connect(sigc::mem_fun(this, &AppBoxRow::launch_new_window));

    add(*layout);
}

AppBoxRow::AppBoxRow() {
    Game_t placeholder;
    placeholder.app_id = 0;
    placeholder.app_name = "Error app";
    placeholder.number_achievements = 0;
    m_app = placeholder;
    
    set_size_request(-1, APP_BOX_ROW_HEIGHT);
}

void
AppBoxRow::launch_new_window() {
    int pid;
    if ((pid = fork()) == 0) {
        // Child process
        // Completely replace this program
        // This depends on the launch script to put it in the right location
        execlp("bin/samrewritten", "bin/samrewritten", "-p", std::to_string(m_app.app_id).c_str(), (char *) 0);
        std::cerr << "Failed to exec new game achievement picker. Exiting." << std::endl;
        exit(EXIT_FAILURE);
    } else if (pid == -1) {
        std::cerr << "An error occurred while forking. Exiting." << std::endl;
        exit(EXIT_FAILURE);
    }

    return;
}

Game_t
AppBoxRow::get_app() const {
    return m_app;
}

AppBoxRow::~AppBoxRow()
{
}
