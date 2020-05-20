#include "InputAppidBoxRow.h"

#include <gtkmm-3.0/gtkmm/box.h>
#include <gtkmm-3.0/gtkmm/arrow.h>
#include "../common/GtkDefineHelper.h"

InputAppidBoxRow::InputAppidBoxRow()
: m_label("")
{
    m_icon.set_from_icon_name("preferences-system", Gtk::BuiltinIconSize::ICON_SIZE_DIALOG);

    Gtk::Box* layout = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_HORIZONTAL, 0);
    Gtk::Arrow* arrow = Gtk::make_managed<Gtk::Arrow>(Gtk::ArrowType::ARROW_RIGHT, Gtk::ShadowType::SHADOW_OUT);

    layout->pack_start(m_icon, FALSE, FALSE, 0);
    layout->pack_start(m_label, TRUE, TRUE, 0);
    layout->pack_start(*arrow, FALSE, FALSE, 0);
    add(*layout);
}

InputAppidBoxRow::~InputAppidBoxRow() {}

void
InputAppidBoxRow::set_appid(const std::string& appid) {
    m_label.set_markup("<b>Launch appid " + appid + "</b>");
    m_appid = appid;
}

AppId_t
InputAppidBoxRow::get_appid() {
    char* endptr;
    return strtoul(m_appid.c_str(), &endptr, 10);
}