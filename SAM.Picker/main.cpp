/*************************************
 * Please read the license before modifying or distributing any of the code from
 * this project. Thank you.
 *************************************/

#include <iostream>
#include <gmodule.h>
#include "MySteam.h"
#include "MySteamClient.h"
#include "MyGameServer.h"
#include "MainPickerWindow.h"
#include "globals.h"
#include "cli_funcs.h"
#include "../common/functions.h"

/**************************************
 *  Declare global variables imported from globals.h
 **************************************/
MySteam* g_steam = nullptr;
MainPickerWindow* g_main_gui = nullptr;
char* g_cache_folder = nullptr;
MySteamClient* g_steamclient = nullptr;


/**************************************
 * Main entry point
 **************************************/
int
main(int argc, char *argv[])
{
    // Test if glib2 is installed, gtk will not work without it.
    if( !g_module_supported() ) {
        std::cout << "You are missing the Gnome libraries. Please read the README to know which libraries to install." << std::endl;
        exit(EXIT_FAILURE);
    }

    gtk_init(&argc, &argv);

    g_steamclient = new MySteamClient();
    g_cache_folder = concat( getenv("HOME"), "/.SamRewritten" );
    g_steam = MySteam::get_instance();
    g_main_gui = new MainPickerWindow();

    MyGameServer server(206690);
    server.run();
    exit(EXIT_SUCCESS);

    // Check for command-line options, which may prevent showing the GUI
    // Note that a rewriting should be done to further separate the GUI
    // from a command-line interface
	if(!go_cli_mode(argc, argv)) {
		gtk_widget_show( g_main_gui->get_main_window() );
		gtk_main();
	}

    return 0;
}
