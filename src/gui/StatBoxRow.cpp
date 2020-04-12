#include "StatBoxRow.h"
#include "../common/functions.h"

#include <gtkmm-3.0/gtkmm/box.h>
#include <gtkmm-3.0/gtkmm/label.h>

StatBoxRow::StatBoxRow(const StatValue_t& data) 
: m_data(data)
{
    std::string ach_title_text, escaped_name;

    escaped_name = data.display_name;
    escape_html(escaped_name);
    ach_title_text = "<b>" + escaped_name + "</b>";

    Gtk::Box* layout = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_HORIZONTAL, 0);
    Gtk::Box* title_desc_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_VERTICAL, 0);
    Gtk::Label* title_label = Gtk::make_managed<Gtk::Label>("");
    set_size_request(-1, 80);

    title_label->set_markup(ach_title_text);
    title_desc_box->pack_start(*title_label, false, true, 0);
    layout->pack_start(*title_desc_box, true, true, 0);

    add(*layout);
}

StatBoxRow::~StatBoxRow() {}