#pragma once

#include "../../steam/steamtypes.h"
#include "../schema_parser/UserGameStatsSchemaParser.h"
#include <mutex>
#include <future>

#define MAX_CONCURRENT_ICON_DOWNLOADS 10

class MainPickerWindow;

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
    unsigned state;

    /**
     * other information is pulled from the
     * AsyncGuiLoader object
     */

    /* the currently loaded item */
    unsigned current_item;
} IdleData;

class AsyncGuiLoader
{
public:
    AsyncGuiLoader(MainPickerWindow* window);

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
    populate_apps();

    void
    populate_achievements();
private:
    bool load_achievements_idle();
    bool load_apps_idle();

    size_t m_concurrent_icon_downloads;
    std::map<AppId_t, std::future<void>> m_app_icon_download_futures;
    std::map<std::string, std::future<void>> m_achievement_icon_download_futures;
    std::future<void> m_owned_apps_future;
    std::future<void> m_achievements_future;
    std::future<bool> m_schema_parser_future;

    /**
     * Mutex to prevent on_refresh_games_button_clicked from being reentrant
     * and allowing multiple idle threads to corrupt the main window.
     */
    std::mutex m_game_refresh_lock;
    std::mutex m_achievement_refresh_lock;

    IdleData m_app_idle_data;
    IdleData m_achievement_idle_data;

    UserGameStatsSchemaParser m_schema_parser;
    MainPickerWindow* m_window;
};
