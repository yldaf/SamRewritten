#pragma once

#include "../types/StatValue.h"

#include <string>
#include <gtkmm-3.0/gtkmm/listboxrow.h>
#include <gtkmm-3.0/gtkmm/spinbutton.h>
#include <gtkmm-3.0/gtkmm/box.h>

/**
 * This class represents a stat entry on the stats view
 */
class StatBoxRow : public Gtk::ListBoxRow
{
public:
    StatBoxRow(const StatValue_t& stat);
    virtual ~StatBoxRow();

    /**
     * Interpret a change in the text field
     */
    void on_new_value_changed(void);

private:
    StatValue_t m_data;
    Gtk::SpinButton m_new_value_entry;
    Gtk::Box m_invalid_conversion_box;
    bool m_b_has_invalid_conversion_image_been_set_up = false;
};