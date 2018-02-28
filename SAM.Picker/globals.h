#pragma once

class MySteam;
#include "MainPickerWindow.h"
//TODO tidy & comment
//Globals
//Reason for the globals is that it's easier to access them in GTK callbacks
extern MySteam *g_steam; // The Model
extern MainPickerWindow *g_main_gui; // The view