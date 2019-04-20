#pragma once
#include <string>
#include <csignal>
#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include "../common/functions.h"
#include "globals.h"
#include "Achievement.h"
#include "MainPickerWindow.h"
#include "../steam/steam_api.h"

/**
 * This class will play the part of being the emulated app
 * It is responsible for retrieving all stats and achievements
 * for a give steam app id.
 * 
 * Technically, it calls fork, and the child process will have 
 * the role of a steam app, that will retrieve all the data
 * and pipe it to the parent process.
 * It uses SIGUSR1, so don't be surprised if you get callbacks
 * triggered if you use this signal somewhere else.
 */

class GameEmulator {
public:
    /**
     * Singleton method to get the unique instance
     */
    static GameEmulator* get_instance();

    /**
     * Starts the Steam app corresponding to the given app_id
     * Will automatically call update_view
     */
    bool init_app(const std::string& app_id);

    /**
     * Will stop the currently running Steam app, launched with
     * init_app
     */
    bool kill_running_app();

    /**
     * Will update the main view, adding all achievements to the 
     * list of achievements
     */
    void update_view();

    /**
     * Will refetch data from the steam API.
     * Will update the main view, adding all achievements to the 
     * list of achievements
     */
    void update_data_and_view();

    /**
     * Will unlock the achivement given it's API name.
     * Will return the value of SetAchievement.
     * https://partner.steamgames.com/doc/api/ISteamUserStats#SetAchievement
     */
    bool unlock_achievement(const char* ach_api_name) const;

    /**
     * Will relock the achivement given it's API name.
     * Will return the value of ClearAchievement.
     * https://partner.steamgames.com/doc/api/ISteamUserStats#ClearAchievement
     */
    bool relock_achievement(const char* ach_api_name) const;

    /**
     * Steam API callback to handle the received stats and achievements
     */
    STEAM_CALLBACK( GameEmulator, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived );

    /**
     * Prevent using the default constructor because we use the 
     * singleton pattern
     */
    GameEmulator(GameEmulator const&)            = delete;
    void operator=(GameEmulator const&)          = delete;

private:
    void retrieve_achievements();

    Achievement_t *m_achievement_list;
    pid_t m_son_pid;
    bool m_have_stats_been_requested;
    int m_pipe[2];
    unsigned m_achievement_count;

    friend void handle_sigchld(int);
    friend void handle_sigusr1_parent(int);
    friend void handle_sigusr1_child(int);
    friend void handle_sigusr2_child(int);

    GameEmulator();
    ~GameEmulator() {};
};