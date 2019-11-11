#include "gtk_input_appid_game_row.h"
#include <iostream>

GtkWidget* 
gtk_input_appid_game_row_new() {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

    GtkWidget *wrapper = gtk_list_box_row_new();
    GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *label = gtk_label_new("");
    GtkWidget *game_logo = gtk_image_new_from_icon_name ("preferences-system", GTK_ICON_SIZE_DIALOG);
    GtkWidget *nice_arrow = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_OUT);

    #pragma GCC diagnostic pop

    gtk_widget_set_size_request(wrapper, -1, 80);
    gtk_box_pack_start(GTK_BOX(layout), GTK_WIDGET(game_logo), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(layout), GTK_WIDGET(label), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(layout), GTK_WIDGET(nice_arrow), FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(wrapper), GTK_WIDGET(layout));

    return wrapper;
}

void
gtk_input_appid_game_row_set_appid(GtkWidget* row, const std::string& input) {
    GList* children;

    children = gtk_container_get_children(GTK_CONTAINER(row)); // Children = horizontal layout
    children = gtk_container_get_children(GTK_CONTAINER(children->data)); // Children = game logo
    children = g_list_next(children); // children = label

    GtkWidget* label = GTK_WIDGET(children->data);

    if (GTK_IS_LABEL(label))
    {
        gtk_widget_set_name(row, input.c_str());
        gtk_label_set_markup( GTK_LABEL(label), std::string("<b>Launch appid " + input + "</b>").c_str() );
    }
    else {
        std::cerr << "Label not found.." << std::endl;
    }
}

AppId_t
gtk_input_appid_game_row_get_appid(GtkListBoxRow* row) {
    char* endptr;
    const gchar* name = gtk_widget_get_name(GTK_WIDGET(row));
    return strtoul(name, &endptr, 10);
}