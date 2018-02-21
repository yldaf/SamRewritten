#include "MainPickerWindow.h"

MainPickerWindow::MainPickerWindow() 
: 
m_main_window(nullptr),
m_game_list(nullptr),
m_builder(nullptr)
{
    GError *error = NULL;
    m_builder = gtk_builder_new();
    const char ui_file[] = "glade/main_window.glade";
    gtk_builder_add_from_file (m_builder, ui_file, &error);

    if(error != NULL) {
        std::cerr << "An error occurred opening the main window.. Make sure " << ui_file << " exists and is a valid file." << std::endl;
        exit(EXIT_FAILURE);
    }

    gtk_builder_connect_signals(m_builder, NULL);

    m_game_list = GTK_LIST_BOX(gtk_builder_get_object(m_builder, "game_list"));
    m_main_window = GTK_WIDGET(gtk_builder_get_object(m_builder, "main_window"));
    GtkWidget* game_placeholder = GTK_WIDGET(gtk_builder_get_object(m_builder, "game_placeholder"));

    gtk_list_box_set_placeholder(m_game_list, game_placeholder);
    gtk_widget_show(game_placeholder);
}

// See https://stackoverflow.com/questions/9192223/remove-gtk-container-children-repopulate-it-then-refresh
void MainPickerWindow::reset_game_list() {
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(m_game_list));

    for(iter = children; iter != NULL; iter = g_list_next(iter))
        gtk_container_remove(GTK_CONTAINER(m_game_list), GTK_WIDGET(iter->data));
    
    g_list_free(children);

}

// Todo: Add more parameters (AppId, icon path...)
void MainPickerWindow::add_to_game_list(const char app_name[]) {
    // Because you can't clone widgets with GTK, I'm going to recreate 
    // GTK_LIST_BOX_ROW(gtk_builder_get_object(m_builder, "game_entry"));
    // By hand. Which is dead stupid.

    //Also, fuck the police I still use GtkArrow what you gonna do

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

    GtkWidget *wrapper = gtk_list_box_row_new();
    GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *label = gtk_label_new(app_name);
    GtkWidget *game_logo = gtk_image_new_from_file("");
    GtkWidget *nice_arrow = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_OUT);

    #pragma GCC diagnostic pop

    gtk_widget_set_size_request(wrapper, -1, 80);

    gtk_box_pack_start(GTK_BOX(layout), GTK_WIDGET(game_logo), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(layout), GTK_WIDGET(label), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(layout), GTK_WIDGET(nice_arrow), FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(wrapper), GTK_WIDGET(layout));

    gtk_list_box_insert(m_game_list, GTK_WIDGET(wrapper), -1);
    // Do not show widgets yet
}

void MainPickerWindow::confirm_game_list() {
    gtk_widget_show_all(GTK_WIDGET(m_game_list));
    // Show all widgetss
}