#include "GtkAchievementBoxRow.h"
#include "MySteam.h"
#include "globals.h"

extern "C"
{
    void 
    on_achievement_button_toggle(GtkToggleButton* but, gpointer achievement) {
        const bool active = gtk_toggle_button_get_active(but);
        const bool achieved = (*(Achievement_t *)achievement).achieved;
        const std::string ach_id = std::string((*(Achievement_t *)achievement).id);

        if(active && achieved) {
            gtk_button_set_label(GTK_BUTTON(but), "Unlocked");
            g_steam->remove_modification_ach(ach_id);
        }
        else if (active && !achieved) {
            gtk_button_set_label(GTK_BUTTON(but), "To unlock");
            g_steam->add_modification_ach(ach_id, true);
        }
        else if (!active && achieved) {
            gtk_button_set_label(GTK_BUTTON(but), "To relock");
            g_steam->add_modification_ach(ach_id, false);
        }
        else if (!active && !achieved) {
            gtk_button_set_label(GTK_BUTTON(but), "Locked");
            g_steam->remove_modification_ach(ach_id);
        }
    }
}

GtkAchievementBoxRow::GtkAchievementBoxRow(const Achievement_t& data) 
:
m_data(data)
{
    //TODO achievement icons
    // TODO REwrite. Ugly AF for unknown reasons
    char ach_title_text[MAX_ACHIEVEMENT_NAME_LENGTH + 7];
    char ach_player_percent_text[50];
    char ach_locked_text[9];
    gboolean pressed;
    if ( data.achieved ) {
        sprintf(ach_locked_text, "%s", "Unlocked");
        pressed = TRUE;
    } else {
        sprintf(ach_locked_text, "%s", "Locked");
        pressed = FALSE;
    }
    sprintf(ach_title_text, "<b>%s</b>", data.name);
    sprintf(ach_player_percent_text, "Achieved by %.1f%% of the players", data.global_achieved_rate);
    

    //TODO create and set new level bar only if ach has progress bar
    m_main_box = gtk_list_box_row_new();
    
    GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *ach_pic = gtk_image_new_from_icon_name("gtk-missing-image", GTK_ICON_SIZE_DIALOG);
    GtkWidget *title_desc_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *title_label = gtk_label_new("");
    GtkWidget *desc_label = gtk_label_new(data.desc);
    GtkWidget *more_info_button = gtk_menu_button_new();
    GtkWidget *more_info_image = gtk_image_new_from_icon_name("gtk-about", GTK_ICON_SIZE_BUTTON);
    GtkWidget *lock_unlock_button = gtk_toggle_button_new_with_label(ach_locked_text);
    GtkWidget *popover_menu = gtk_popover_new( more_info_button );
    GtkWidget *popover_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    GtkWidget *more_info_label = gtk_label_new("");
    GtkWidget *percentage_players_label = gtk_label_new(ach_player_percent_text);
    GtkWidget *sep_one = gtk_separator_menu_item_new();
    GtkWidget *sep_two = gtk_separator_menu_item_new();
    GtkWidget *progression_label = gtk_label_new("Achievement progress");
    GtkWidget *ach_level_bar = gtk_level_bar_new(); //gtk_level_bar_new_for_interval (gdouble min_value, gdouble max_value);
    GtkWidget *ach_progress_label_value = gtk_label_new("TODO / TODO");

    gtk_label_set_markup(GTK_LABEL(title_label), ach_title_text);
    gtk_label_set_markup(GTK_LABEL(more_info_label), "<b>Additional information</b>");
    gtk_widget_set_size_request(m_main_box, -1, 80);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(more_info_button), GTK_WIDGET(popover_menu));
    gtk_widget_set_valign(GTK_WIDGET(more_info_button), GTK_ALIGN_CENTER);
    gtk_widget_set_margin_end(GTK_WIDGET(more_info_button), 10);
    gtk_widget_set_size_request(GTK_WIDGET(lock_unlock_button), 150, -1);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lock_unlock_button), pressed);
    gtk_container_set_border_width(GTK_CONTAINER(popover_box), 5);
    gtk_style_context_add_class(
        gtk_widget_get_style_context( GTK_WIDGET(more_info_button) ),
        "circular"
    );

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

    g_signal_connect(lock_unlock_button, "toggled", (GCallback)on_achievement_button_toggle,  (gpointer)&data);
}

GtkAchievementBoxRow::~GtkAchievementBoxRow() {
    //std::cerr << "Deleting a row" << std::endl;
    gtk_widget_destroy( GTK_WIDGET(m_main_box) );
}