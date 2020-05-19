#include "MainPickerWindowFactory.h"
#include "../common/functions.h"
#include <gtkmm-3.0/gtkmm/builder.h>
#include <iostream>

MainPickerWindow*
MainPickerWindowFactory::create(AppId_t initial_app_id) {
    const std::string ui_file = "glade/main_window.glade";
    Glib::RefPtr<Gtk::Builder> builder;

    try {
        builder = Gtk::Builder::create_from_file(ui_file);
    }
    catch(const Glib::Error& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        zenity("Unable to load the UI file.. Was the installation completed? Please report this to us on Github!");
        exit(EXIT_FAILURE);
    }
    
    MainPickerWindow* window = nullptr;
    builder->get_widget_derived("main_window", window, initial_app_id);
    
    if (window)
    {
        return window;
    }

    std::cerr << "Fatal error: Could not get derived widget main_window." << std::endl;
    zenity("Unable to load the main window.. Please report this to us on Github!");
    exit(EXIT_FAILURE);
    return nullptr;
}