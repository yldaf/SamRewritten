#pragma once

class MySteam;
class MainPickerWindow;

// Reason for the globals is that it's easier to access them in GTK callbacks
// All those variables are initialised in main.cpp

/**
 * g_steam is the backend of all of this, the M of the MVC. It will analyse the
 * steam folder and give data about it: games owned, icons, etc.
 */
extern MySteam *g_steam;

/**
 * g_main_gui is the main application window. If you can read this, it is very
 * glitchy, freezes instead of showing the loading widget, but still looks
 * half-decent. It's the V of the MVC.
 */
extern MainPickerWindow *g_main_gui;

/**
 * The absolute path to the cache folder. It's global, because everyone will
 * need to write or read into this folder at some point. 
 * As of right now the default path is:
 * 
 *      ~/.SamRewritten
 */
extern char *g_cache_folder;
