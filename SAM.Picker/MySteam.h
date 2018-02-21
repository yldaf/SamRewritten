#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <csignal>
#include <dirent.h>
#include "Game.h"
#include "SteamAppDAO.h"
#include "../steam/steam_api.h"
#include "../common/c_processes.h"


class MySteam {
public:
    static MySteam* get_instance();
    static std::string get_user_steamId3();
    static std::string get_steam_install_path();

    bool launch_game(std::string appId);
    bool quit_game();
    void print_all_owned_games() const;
    void refresh_owned_apps();

    // Make sure to call refresh_owned_apps at least once to get correct results
    std::vector<Game_t> get_all_games_with_stats() { return m_all_subscribed_apps; };

    MySteam(MySteam const&)                 = delete;
    void operator=(MySteam const&)          = delete;
private:
    MySteam();
    ~MySteam();

    pid_t m_child_pid;
    std::vector<Game_t> m_all_subscribed_apps;
};