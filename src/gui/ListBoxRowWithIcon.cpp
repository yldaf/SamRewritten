#include "ListBoxRowWithIcon.h"
#include <iostream>

ListBoxRowWithIcon::ListBoxRowWithIcon()
{
}

ListBoxRowWithIcon::ListBoxRowWithIcon(const std::string& path, const int width, const int height)
{
    set_icon(path, width, height);
}

ListBoxRowWithIcon::~ListBoxRowWithIcon()
{
}

void
ListBoxRowWithIcon::set_icon(const std::string& path, const int width, const int height) {
    auto resized = Gdk::Pixbuf::create_from_file(path)->scale_simple(width, height, Gdk::InterpType::INTERP_NEAREST);
    m_icon.set(resized);
}

void
ListBoxRowWithIcon::set_missing() {
    m_icon.set_from_icon_name("gtk-missing-image", Gtk::BuiltinIconSize::ICON_SIZE_DIALOG);
}