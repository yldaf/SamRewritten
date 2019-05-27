/*************************************
 * Please read the license before modifying or distributing any of the code from
 * this project. Thank you.
 *************************************/

#include <iostream>
#include <gmodule.h>
#include "MySteam.h"
#include "MainPickerWindow.h"
#include "globals.h"
#include "cli_funcs.h"

/**************************************
 *  Declare global variables imported from globals.h
 **************************************/
MySteam* g_steam = nullptr;
MainPickerWindow* g_main_gui = nullptr;
char* g_cache_folder = nullptr;



/**************************************
 * Main entry point
 **************************************/
int
main(int argc, char *argv[])
{
    // Test if glib2 is installed, gtk will not work without it.
    if( !g_module_supported() ) {
        std::cerr << "Sorry, but gmodules are not supported on your platform :(. Try installing as many gnome libs as you can maybe.." << std::endl;
        exit(EXIT_FAILURE);
    }

    gtk_init(&argc, &argv);

    g_cache_folder = concat( getenv("HOME"), "/.SamRewritten" );
    g_steam = MySteam::get_instance();
    g_main_gui = new MainPickerWindow();

    // Check for command-line options, which may prevent showing the GUI
    // Note that a rewriting should be done to further separate the GUI
    // from a command-line interface
	if(!go_cli_mode(argc, argv)) {
		gtk_widget_show( g_main_gui->get_main_window() );
		gtk_main();
	}

    return 0;
}
