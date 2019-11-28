#pragma once

#include "../../steam/steamtypes.h"
#include <string>

#include <gtkmm-3.0/gtkmm/box.h>
#include <gtkmm-3.0/gtkmm/label.h>
#include <gtkmm-3.0/gtkmm/image.h>
#include <gtkmm-3.0/gtkmm/arrow.h>

#include "AppBoxRow.h"

/**
 * This class represents the row in the app list that only shows when the user
 * is entering an appid in the search bar
 */
class InputAppidBoxRow : public AppBoxRow
{
public:
    InputAppidBoxRow();
    virtual ~InputAppidBoxRow();

    void set_appid(const std::string& appid);
    AppId_t get_appid();
protected:
    std::string m_appid;
    Gtk::Label m_label;
};