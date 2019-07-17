#include "MySteam.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <dirent.h>
#include "types/Game.h"
#include "SteamAppDAO.h"
#include "GameEmulator.h"
#include "common/functions.h"

#define MAX_PATH 1000


MySteam::MySteam() {

}
// => Constructor

/**
 * Compare the app_name of a Game_t for sorting
 */
bool 
MySteam::comp_app_name(Game_t app1, Game_t app2) {
    return strcasecmp(app1.app_name.c_str(), app2.app_name.c_str()) < 0;
}
// => comp_app_name

/**
 * Gets the unique instance. See "Singleton design pattern" for help
 */
MySteam* 
MySteam::get_instance() {
    static MySteam instance;
    return &instance;
}
// => get_instance


/**
 * Fakes a new game being launched. Keeps running in the background until quit_game is called.
 */
bool 
MySteam::launch_game(AppId_t appID) {
    // Print an error if a game is already launched, maybe allow multiple games at the same time in the future?
    // GameEmulator* emulator = GameEmulator::get_instance();
    
    // //TODO if
    // emulator->init_app(appID);

    if (m_ipc_socket != nullptr)
    {
        std::cerr << "I will not launch the game as one is already running" << std::endl;
        return false;
    }
    
    m_ipc_socket = m_server_manager.quick_server_create(appID);

    return true;
}
// => launch_game


/**
 * If a fake game is running, stops it and returns true, else false.
 */
bool 
MySteam::quit_game() {
    // GameEmulator* emulator = GameEmulator::get_instance();
    // return emulator->kill_running_app();
    m_ipc_socket->kill_server();
    delete m_ipc_socket;
    m_ipc_socket = nullptr;
    return true;
}
// => quit_game


/**
 * This does NOT retrieves all owned games.
 * It does retrieve all owned games WITH STATS or ACHIEVEMENTS
 * Stores the owned games in m_all_subscribed_apps
 * We assume the user didn't put any garbage in his steam folder as well.
 */
void 
MySteam::refresh_owned_apps() {
    Game_t game;
    SteamAppDAO* appDAO = SteamAppDAO::get_instance();

    // The whole update will really occur only once in a while, no worries
    appDAO->update_name_database(); // Downloads and parses app list from Steam
    m_all_subscribed_apps.clear();

    auto all_apps = appDAO->get_all_apps();
    for (auto pair : all_apps) {
        auto app_id = pair.first;
        if (appDAO->app_is_owned(app_id))
        {
            game.app_id = pair.first;
            game.app_name = pair.second;

            m_all_subscribed_apps.push_back(game);
        }
    }

    std::sort(m_all_subscribed_apps.begin(), m_all_subscribed_apps.end(), comp_app_name);
}
// => refresh_owned_apps


/**
 * Tries to locate the steam folder in multiple locations,
 * which is not a failsafe implementation.
 */
std::string 
MySteam::get_steam_install_path() {
    static const std::string home_path(getenv("HOME"));
    if(file_exists(home_path + "/.local/share/Steam/appcache/appinfo.vdf")) {
        return std::string(home_path + "/.local/share/Steam");
    }
    else if(file_exists(home_path + "/.steam/appcache/appinfo.vdf")) {
        return std::string(home_path + "/.steam");
    }
    else if(file_exists(home_path + "/.steam/steam/appcache/appinfo.vdf")) {
        return std::string(home_path + "/.steam/steam");
    }
    else {
        std::cerr << "Unable to locate the steam directory. TODO: implement a folder picker here" << std::endl;
        exit(EXIT_FAILURE);
    }
}
// => get_steam_install_path


/**
 * Mostly used for debug purposes
 */
void 
MySteam::print_all_owned_games() const {
    std::cerr << "Summary of owned apps" << std::endl << "========================" << std::endl;

    for(Game_t i : m_all_subscribed_apps) {
        std::cerr << i.app_id << " -> " << i.app_name << std::endl;
    }
}
// => print_all_owned_games


/**
 * Reminder that download_app_icon does check if the file is 
 * already there before attempting to download from the web.
 * It also has a "callback" that will refresh the view.
 */
void 
MySteam::refresh_icons() {
    SteamAppDAO *appDAO = SteamAppDAO::get_instance();
    
    for(Game_t i : m_all_subscribed_apps) {
        appDAO->download_app_icon(i.app_id);
    }
}
// => refresh_icons

/**
 * Adds an achievement to the list of achievements to unlock/lock
 */
void 
MySteam::add_modification_ach(const std::string& ach_id, const bool& new_value) {
    std::cout << ach_id << ": " << (new_value ? "to unlock" : "to relock") << std::endl;
    m_pending_ach_modifications.insert( std::pair<std::string, bool>(ach_id, new_value) );
}
// => add_modification_ach

/**
 * Removes an achievement to the list of achievements to unlock/lock
 */
void 
MySteam::remove_modification_ach(const std::string& ach_id) {
    if ( m_pending_ach_modifications.find(ach_id) == m_pending_ach_modifications.end() ) {
        std::cerr << "WARNING: Could not cancel: modification was not pending";
    } else {
        m_pending_ach_modifications.erase(ach_id);
    }
}
// => remove_modification_ach