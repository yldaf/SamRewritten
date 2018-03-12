#include "GtkAchievementBoxRow.h"

GtkAchievementBoxRow::GtkAchievementBoxRow(const Achievement_t& data) 
:
m_data(data)
{
    //TODO create all the BS
    m_main_box = gtk_list_box_row_new();
    GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *ach_pic = gtk_image_new_from_icon_name("gtk-missing-image", GTK_ICON_SIZE_DIALOG);

    gtk_widget_set_size_request(m_main_box, -1, 80);

    gtk_box_pack_start(GTK_BOX(layout), GTK_WIDGET(ach_pic), TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(m_main_box), GTK_WIDGET(layout));
}