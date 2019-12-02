#pragma once

#include "../UserGameStatsSchemaParser.h"

#include <glib-2.0/glib.h>

/**
 * This file is what is left of the former implementation with GTK C.
 * TODO: Move this somewhere wher it makes sense
 */

/**
 * Information for implementing lazy loading of games.
 * source: https://www.bassi.io/pages/lazy-loading/
 */

/* states in the GUI-loading FSMs */
enum {
    APPS_STATE_STARTED,                 /* start state */
    APPS_STATE_WAITING_FOR_OWNED_APPS,  /* waiting for owned apps thread to finish */
    APPS_STATE_LOADING_GUI,             /* feeding game rows to the model */
    APPS_STATE_DOWNLOADING_ICONS,       /* fire off icon downloads (to be added to the model) */
    APPS_STATE_FINISHED                 /* finish state */
};

enum {
    ACH_STATE_STARTED,                  /* start state */
    ACH_STATE_WAITING_FOR_ACHIEVEMENTS, /* waiting for child to fetch achievements */
    ACH_STATE_LOADING_GUI,              /* feeding game rows to the model */
    ACH_STATE_WAITING_FOR_SCHEMA_PARSER,/* waiting for stats schema parser to get icons download names */
    ACH_STATE_DOWNLOADING_ICONS,        /* fire off icon downloads (to be added to the model) */
    ACH_STATE_FINISHED                  /* fin ish state */
};

/* data to be passed to the idle handler */
typedef struct
{
    /* the current state */
    guint state;

    /**
     * other information is just pulled from the global
     * or other singleton info for now
     */

    /* the currently loaded item */
    guint current_item;
} IdleData;

class AsyncGuiLoader 
{
public:
    /** 
     * When the user wants to refresh the game list.
     * This is also called when the main window just got spawned.
     * - Clear the game list (will show the loading widget)
     * - Do the backend work (get owned apps..)
     * - Add all the retrieved data to the view.
     * - Do the backend work again for icons: view must be initialized first
     *   to display the app logos.
     * - Draw the result.
     */
    void 
    on_refresh_games_button_clicked_old();

    void 
    populate_achievements();
private:
    bool load_achievements_idle();
    bool load_apps_idle();

    IdleData m_idle_data;
    UserGameStatsSchemaParser m_schema_parser;
};
