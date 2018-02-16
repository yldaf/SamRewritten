#define DEBUG

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gmodule.h>
#include "MySteam.h"

int launcher_main();

int main(int argc, char *argv[])
{
    if(!g_module_supported()) {
        std::cerr << "Sorry, but gmodules are not supported on your platform :(. Try installing as many gnome libs as you can maybe.." << std::endl;
        exit(EXIT_FAILURE);
    }
    
    gtk_init(&argc, &argv);

    setenv("SteamAppId", "480", 1);
    if(!SteamAPI_Init()){
        std::cerr << std::endl << "Make sure you have launched steam and that you are logged in." << std::endl;
        
        GError *error = NULL;
        GtkBuilder *builder = gtk_builder_new();
        const char ui_file[] = "glade/error_initialize.glade";
        gtk_builder_add_from_file (builder, ui_file, &error);
        if(error != NULL) {
            std::cerr << "You won't believe it, but an error occurred opening the error message's window.. Make sure " << ui_file << " exists and is a valid file." << std::endl;
            exit(EXIT_FAILURE);
        }

        // Will connect "on_close_button_clicked"
        gtk_builder_connect_signals(builder, NULL);

        GtkWidget *close_button;
        GtkWidget *popup_window;

        popup_window = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_window"));
        close_button = GTK_WIDGET(gtk_builder_get_object(builder, "button_close"));

        g_object_unref(builder);

        gtk_widget_show(popup_window);
        gtk_main();

        exit(EXIT_FAILURE);
    }

    launcher_main();
    return 0;
}

int launcher_main() {

    MySteam *steam = MySteam::get_instance();
    steam->print_all_owned_games();

    //steam->launch_game("368230");

    //std::this_thread::sleep_for(std::chrono::seconds(5));

    //steam->quit_game();

    

    return 0;
}

//Gtk Callbacks
extern "C" 
{
    // When you click on the close button if steam is not running
    void on_close_button_clicked () {
        gtk_main_quit();
    }
}