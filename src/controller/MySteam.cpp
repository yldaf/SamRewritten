#include "MySteam.h"
#include "SteamAppDAO.h"
#include "../types/Game.h"
#include "../types/Actions.h"
#include "../common/functions.h"
#include "../json/yajlHelpers.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <dirent.h>
#include <bits/stdc++.h>

MySteam::MySteam() {
    std::string data_home_path;
    if (getenv("XDG_DATA_HOME") != NULL) {
        data_home_path = getenv("XDG_DATA_HOME");
    } else {
        data_home_path = getenv("HOME") + std::string("/.local/share");
    }

    if (file_exists(data_home_path + "/Steam/appcache/appinfo.vdf")) {
        m_steam_install_dir = std::string(data_home_path + "/Steam");
        return;
    }

    const std::string home_path = getenv("HOME");
    if (file_exists(home_path + "/.steam/appcache/appinfo.vdf")) {
        m_steam_install_dir = std::string(home_path + "/.steam");
        return;
    }
    else if (file_exists(home_path + "/.steam/steam/appcache/appinfo.vdf")) {
        m_steam_install_dir = std::string(home_path + "/.steam/steam");
        return;
    }
    else {
        std::cerr << "Unable to locate the steam directory." << std::endl;
        zenity("Unable to find your Steam installation directory.. Please report this on Github!");
        exit(EXIT_FAILURE);
    }
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
MySteam::launch_app(AppId_t appID) {
    // Print an error if a game is already launched
    // allow multiple games at the same time in the future via new window launching

    if (m_ipc_socket != nullptr) {
        std::cerr << "I will not launch the game as one is already running" << std::endl;
        return false;
    }
    
    m_ipc_socket = m_server_manager.quick_server_create(appID);

    if (m_ipc_socket == nullptr) {
        std::cerr << "Failed to get connection to game" << std::endl;
        return false;
    }

    m_app_id = appID;
    return true;
}
// => launch_app


/**
 * If a fake game is running, stops it and returns true, else false.
 */
bool 
MySteam::quit_game() {
    if (m_ipc_socket != nullptr) {
        m_ipc_socket->kill_server();
        delete m_ipc_socket;
        m_ipc_socket = nullptr;
        return true;
    } else {
        return false;
    }
}
// => quit_game


/**
 * This retrieves all owned apps, can include garbage apps
 * depending on the users settings, in a near future.
 * Stores the owned games in m_all_subscribed_apps.
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
    return m_steam_install_dir;
}
// => get_steam_install_path

/**
 * Reminder that download_app_icon does check if the file is 
 * already there before attempting to download from the web.
 */
void 
MySteam::refresh_app_icon(AppId_t app_id) {
    SteamAppDAO *appDAO = SteamAppDAO::get_instance();
    appDAO->download_app_icon(app_id);
}
// => refresh_app_icon

void
MySteam::refresh_achievement_icon(std::string id, std::string icon_download_name) {
    SteamAppDAO *appDAO = SteamAppDAO::get_instance();
    appDAO->download_achievement_icon(m_app_id, id, icon_download_name);
}
// => refresh_achievement_icon

void
MySteam::refresh_achievements() {

    if (m_ipc_socket == nullptr) {
        std::cerr << "Connection to game is broken" << std::endl;
        zenity("Connection to game is broken");
        exit(EXIT_FAILURE);
    }

    std::string response = m_ipc_socket->request_response(make_get_achivements_request_string());

    if (!decode_ack(response)) {
        std::cerr << "Failed to receive ack!" << std::endl;
    }

    m_achievements = decode_achievements(response);

    set_special_flags();
}
// => refresh_achievements

/**
 * Adds an achievement to the list of achievements to unlock/lock
 */
void 
MySteam::add_modification_ach(const std::string& ach_id, const bool& new_value) {
    std::cout << "Adding modification: " << ach_id << ", " << (new_value ? "to unlock" : "to relock") << std::endl;
    if ( m_pending_ach_modifications.find(ach_id) == m_pending_ach_modifications.end() ) {
        m_pending_ach_modifications.insert( std::pair<std::string, bool>(ach_id, new_value) );
    } else {
        std::cerr << "Warning: Cannot append " << ach_id << ", value already exists." << std::endl;
    }
}
// => add_modification_ach

/**
 * Removes an achievement to the list of achievements to unlock/lock
 */
void 
MySteam::remove_modification_ach(const std::string& ach_id) {
    std::cout << "Removing modification: " << ach_id << std::endl;
    if ( m_pending_ach_modifications.find(ach_id) == m_pending_ach_modifications.end() ) {
        std::cerr << "WARNING: Could not cancel: modification was not pending: " << ach_id << std::endl;
    } else {
        m_pending_ach_modifications.erase(ach_id);
    }
}
// => remove_modification_ach

/**
 * Commit pending achievement changes
 */
void
MySteam::commit_changes() {
    std::vector<AchievementChange_t> changes;

    for ( const auto& [key, val] : m_pending_ach_modifications) {
        std::cerr << "key " << key << "val " << val << std::endl;
        changes.push_back( (AchievementChange_t){ key, val } );
    }
    
    std::string response = m_ipc_socket->request_response(make_store_achivements_request_string(changes));

    if (!decode_ack(response)) {
        std::cerr << "Failed to store achievement changes!" << std::endl;
    }

    // Clear all pending changes
    m_pending_ach_modifications.clear();
}
// => commit_changes

void
MySteam::clear_changes() {
    // Clear all pending changes
    m_pending_ach_modifications.clear();
}
// => clear_changes

void
MySteam::set_special_flags() {
    // TODO: Maybe split this up to be more amenable to threaded GUI loading, could fire off in thread

    long next_most_achieved_index = -1;
    float next_most_achieved_rate = 0;
    Achievement_t tmp;

    for(size_t i = 0; i < m_achievements.size(); i++)
    {
        tmp = m_achievements[i];
        m_achievements[i].special = ACHIEVEMENT_NORMAL;

        if ( !tmp.achieved && tmp.global_achieved_rate > next_most_achieved_rate )
        {
            next_most_achieved_rate = tmp.global_achieved_rate;
            next_most_achieved_index = i;
        }

        if ( tmp.global_achieved_rate <= 5.f )
        {
            m_achievements[i].special = ACHIEVEMENT_RARE;
        }
    }

    if ( next_most_achieved_index != -1 )
    {
        m_achievements[next_most_achieved_index].special |= ACHIEVEMENT_NEXT_MOST_ACHIEVED;
    }
}