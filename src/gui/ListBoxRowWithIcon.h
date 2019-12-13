#pragma once

#include <string>
#include <gtkmm-3.0/gtkmm/listboxrow.h>
#include <gtkmm-3.0/gtkmm/image.h>

class ListBoxRowWithIcon : public Gtk::ListBoxRow
{
public:
    ListBoxRowWithIcon();
    ListBoxRowWithIcon(const std::string& path, const int width, const int height);
    virtual ~ListBoxRowWithIcon();

    void set_icon(const std::string& path, const int width, const int height);
    void set_missing();

protected:
    Gtk::Image m_icon;
};
