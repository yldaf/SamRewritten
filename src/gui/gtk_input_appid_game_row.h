// Full blown going back to C until this gets rewritten with GTKMM
#pragma once
#include <string>
#include <gtk/gtk.h>
#include "../../steam/steamtypes.h"

GtkWidget* 
gtk_input_appid_game_row_new();

void
gtk_input_appid_game_row_set_appid(GtkWidget* row, const std::string& input);

AppId_t
gtk_input_appid_game_row_get_appid(GtkListBoxRow* row);