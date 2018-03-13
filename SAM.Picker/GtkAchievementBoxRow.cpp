#include "GtkAchievementBoxRow.h"

GtkAchievementBoxRow::GtkAchievementBoxRow(const Achievement_t& data) 
:
m_data(data)
{
    //TODO create all the BS
    //TODO create and set new level bar only if ach has progress bar
    m_main_box = gtk_list_box_row_new();
    
    GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *ach_pic = gtk_image_new_from_icon_name("gtk-missing-image", GTK_ICON_SIZE_DIALOG);
    GtkWidget *title_desc_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *title_label = gtk_label_new("");
    GtkWidget *desc_label = gtk_label_new(data.desc);
    GtkWidget *more_info_button = gtk_menu_button_new();
    GtkWidget *more_info_image = gtk_image_new_from_icon_name("gtk-about", GTK_ICON_SIZE_BUTTON);
    GtkWidget *lock_unlock_button = gtk_toggle_button_new_with_label("TODO");
    GtkWidget *popover_menu = gtk_popover_new( more_info_button );
    GtkWidget *popover_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *more_info_label = gtk_label_new("");
    GtkWidget *percentage_players_label = gtk_label_new("Achieved by TODO% of the players");
    GtkWidget *sep_one = gtk_separator_menu_item_new();
    GtkWidget *sep_two = gtk_separator_menu_item_new();
    GtkWidget *progression_label = gtk_label_new("Achievement progress");
    GtkWidget *ach_level_bar = gtk_level_bar_new(); //gtk_level_bar_new_for_interval (gdouble min_value, gdouble max_value);
    GtkWidget *ach_progress_label_value = gtk_label_new("TODO / TODO");

    gtk_label_set_markup(GTK_LABEL(title_label), "<b>Text to be bold</b>");
    gtk_label_set_markup(GTK_LABEL(more_info_label), "<b>Additional information</b>");
    gtk_widget_set_size_request(m_main_box, -1, 80);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(more_info_button), GTK_WIDGET(popover_menu));

    gtk_container_add(GTK_CONTAINER(more_info_button), GTK_WIDGET(more_info_image));
    gtk_box_pack_start(GTK_BOX(title_desc_box), GTK_WIDGET(title_label), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(title_desc_box), GTK_WIDGET(desc_label), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(layout), GTK_WIDGET(ach_pic), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(layout), GTK_WIDGET(title_desc_box), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(layout), GTK_WIDGET(more_info_button), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(layout), GTK_WIDGET(lock_unlock_button), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(popover_box), GTK_WIDGET(more_info_label), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(popover_box), GTK_WIDGET(sep_one), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(popover_box), GTK_WIDGET(percentage_players_label), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(popover_box), GTK_WIDGET(sep_two), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(popover_box), GTK_WIDGET(progression_label), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(popover_box), GTK_WIDGET(ach_level_bar), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(popover_box), GTK_WIDGET(ach_progress_label_value), FALSE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(popover_menu), GTK_WIDGET(popover_box));
    gtk_widget_show_all(popover_box);
    gtk_container_add(GTK_CONTAINER(m_main_box), GTK_WIDGET(layout));
}