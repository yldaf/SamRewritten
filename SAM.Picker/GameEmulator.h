#pragma once
#include <string>
#include <csignal>
#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
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

    GameEmulator(GameEmulator const&)            = delete;
    void operator=(GameEmulator const&)          = delete;

private:
    pid_t m_son_pid;

    friend void handle_sigchld(int);

    GameEmulator() : m_son_pid(-1) {};
    ~GameEmulator() {};
};