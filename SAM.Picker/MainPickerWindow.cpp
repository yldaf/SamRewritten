#include "MainPickerWindow.h"

MainPickerWindow::MainPickerWindow() 
: 
m_main_window(nullptr),
m_game_list(nullptr),
m_loading_game_list_box(nullptr)
{
    GError *error = NULL;
    GtkBuilder *builder = gtk_builder_new();
    const char ui_file[] = "glade/main_window.glade";
    gtk_builder_add_from_file (builder, ui_file, &error);

    if(error != NULL) {
        std::cerr << "An error occurred opening the main window.. Make sure " << ui_file << " exists and is a valid file." << std::endl;
        exit(EXIT_FAILURE);
    }

    gtk_builder_connect_signals(builder, NULL);

    m_game_list = GTK_BOX(gtk_builder_get_object(builder, "game_list"));
    m_main_window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    m_loading_game_list_box = GTK_BOX(gtk_builder_get_object(builder, "loading_game_box"));

    g_object_unref(builder);
}

// See https://stackoverflow.com/questions/9192223/remove-gtk-container-children-repopulate-it-then-refresh
void MainPickerWindow::reset_game_list() {
    GList *children, *iter;

    children = gtk_container_get_children(GTK_CONTAINER(m_game_list));

    for(iter = children; iter != NULL; iter = g_list_next(iter))
        gtk_container_remove(GTK_CONTAINER(m_game_list), GTK_WIDGET(iter->data));
    
    g_list_free(children);

    //List is cleared, let's readd the spinner

    if(gtk_box_get_center_widget(m_game_list) == NULL) {
        gtk_box_set_center_widget(GTK_BOX(m_game_list), GTK_WIDGET(m_loading_game_list_box));
    }
    else {
        std::cerr << "There already is a center widget. Cannot add a new one" << std::endl;
    }
}

void MainPickerWindow::add_to_game_list(const char app_name[]) {
    GtkWidget *entry = gtk_button_new_with_label(app_name);
    gtk_box_pack_start(GTK_BOX(m_game_list), entry, FALSE, TRUE, 0);

    //TODO continue here, get the list to display onr item
    //TODO remove the central widget if there is one
        g_object_ref(m_loading_game_list_box);
}