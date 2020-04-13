#include "StatBoxRow.h"
#include "../controller/MySteam.h"
#include "../globals.h"
#include "../common/functions.h"

#include <string>
#include <gtkmm-3.0/gtkmm/box.h>
#include <gtkmm-3.0/gtkmm/label.h>
#include <gtkmm-3.0/gtkmm/entry.h>

StatBoxRow::StatBoxRow(const StatValue_t& data) 
: m_data(data)
{
    Gtk::Box* layout = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_HORIZONTAL, 0);
    Gtk::Box* title_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_VERTICAL, 0);
    Gtk::Box* type_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_VERTICAL, 0);
    Gtk::Box* values_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_VERTICAL, 0);
    Gtk::Box* new_values_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_HORIZONTAL, 0);
    Gtk::Label* title_label = Gtk::make_managed<Gtk::Label>("");
    Gtk::Label* type_label = Gtk::make_managed<Gtk::Label>("");
    Gtk::Label* cur_value_label = Gtk::make_managed<Gtk::Label>("");
    Gtk::Label* new_value_label = Gtk::make_managed<Gtk::Label>("");

    set_size_request(-1, 40);
    type_box->set_size_request(150, -1);
    values_box->set_size_request(150, -1);

    m_new_value_entry.set_width_chars(10);
    m_new_value_entry.set_max_width_chars(10);
    // Bound this to some reasonable value
    m_new_value_entry.set_max_length(100);

    if (data.type == UserStatType::Integer) {
        type_label->set_label("Type: Integer");
        // TODO: may want to bound the size of this
        cur_value_label->set_label("Current value: " + std::to_string(std::any_cast<long long>(data.value)));
    } else if (data.type == UserStatType::Float) {
        type_label->set_label("Type: Float");
        // TODO: may want to bound the size of this
        cur_value_label->set_label("Current value: " + std::to_string(std::any_cast<double>(data.value)));
    } else {
        type_label->set_label("Type: Unknown");
        cur_value_label->set_label("Current value: Unknown");
    }

    new_value_label->set_label("New value:      ");

    // Left align labels
    cur_value_label->set_xalign(0);
    new_value_label->set_xalign(0);

    title_label->set_label(data.display_name);
    title_box->pack_start(*title_label, false, true, 0);
    type_box->pack_start(*type_label, false, true, 0);
    new_values_box->pack_start(*new_value_label, true, true, 0);
    new_values_box->pack_start(m_new_value_entry, true, true, 0);
    values_box->pack_start(*cur_value_label, false, true, 0);
    values_box->pack_start(*new_values_box, false, true, 0);
    layout->pack_start(*title_box, true, true, 0);
    layout->pack_start(*type_box, false, true, 0);
    layout->pack_start(*values_box, false, true, 0);

    add(*layout);

    m_new_value_entry.signal_changed().connect(sigc::mem_fun(this, &StatBoxRow::on_new_value_changed));
}

StatBoxRow::~StatBoxRow() {}

void
StatBoxRow::on_new_value_changed(void) {
    // Note this is not run after a short delay of the text not being changed
    // so maybe we don't want to edit the list every single time a button is pressed.
    // The alternative is to read through all stat boxes when commit changes is
    // pressed.
    bool valid_conversion;
    std::any new_value;
    const std::string& buf = m_new_value_entry.get_text();

    valid_conversion = convert_user_stat_value(m_data.type, buf, &new_value);

    // Remove pending modifications with different values, if any
    g_steam->remove_modification_stat(m_data);
    
    if (valid_conversion) {
        g_steam->add_modification_stat(m_data, new_value);
    }
}
