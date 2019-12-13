 #include "AppBoxRow.h"

 #include <gtkmm-3.0/gtkmm/box.h>
 #include <gtkmm-3.0/gtkmm/arrow.h>
 #include <gtkmm-3.0/gtkmm/label.h>

 #define APP_BOX_ROW_HEIGHT 80

AppBoxRow::AppBoxRow(const Game_t& app)
: m_app(app)
{
    set_size_request(-1, APP_BOX_ROW_HEIGHT);
    set_missing();
    Gtk::Box* layout = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_HORIZONTAL, 0);
    Gtk::Label* label = Gtk::make_managed<Gtk::Label>(app.app_name);
    Gtk::Arrow* arrow = Gtk::make_managed<Gtk::Arrow>(Gtk::ArrowType::ARROW_RIGHT, Gtk::ShadowType::SHADOW_OUT);

    layout->pack_start(m_icon, false, false, 0);
    layout->pack_start(*label, true, true, 0);
    layout->pack_start(*arrow, false, false, 0);

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

Game_t
AppBoxRow::get_app() const {
    return m_app;
}

AppBoxRow::~AppBoxRow()
{
}
