#pragma once
#include <string>
#include <csignal>
#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include "Achievement.h"
#include "../steam/steam_api.h"

/**
 * This class will play the part of being the emulated app
 * Todo comment more
 */

class GameEmulator {
public:
    static GameEmulator* get_instance();
    bool init_app(const std::string& app_id);
    bool kill_running_app();
    void update_view();

    STEAM_CALLBACK( GameEmulator, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived );

    GameEmulator(GameEmulator const&)            = delete;
    void operator=(GameEmulator const&)          = delete;

private:
    //TODO: DOC:
    // Used in init_app
    void retrieve_achievements();

    Achievement_t *m_achievement_list;
    pid_t m_son_pid;
    bool m_have_stats_been_requested;
    int m_pipe[2];

    friend void handle_sigchld(int);
    friend void handle_sigusr1_parent(int);
    friend void handle_sigusr1_child(int);

    GameEmulator();
    ~GameEmulator() {};
};