#pragma once

#include "../types/StatValue.h"

#include <string>
#include <gtkmm-3.0/gtkmm/listboxrow.h>

/**
 * This class represents a stat entry on the stats view
 */
class StatBoxRow : public Gtk::ListBoxRow
{
public:
    StatBoxRow(const StatValue_t& stat);
    virtual ~StatBoxRow();

private:
    StatValue_t m_data;
    // more stuff here
};