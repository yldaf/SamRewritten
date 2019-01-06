#include "MySteam.h"
#define MAX_PATH 1000


MySteam::MySteam() {

}
// => Constructor

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
MySteam::launch_game(std::string appID) {
    // Print an error if a game is already launched, maybe allow multiple games at the same time in the future?
    GameEmulator* emulator = GameEmulator::get_instance();
    
    //TODO if
    emulator->init_app(appID);

    return false;
}
// => launch_game


/**
 * If a fake game is running, stops it and returns true, else false.
 */
bool 
MySteam::quit_game() {
    GameEmulator* emulator = GameEmulator::get_instance();
    return emulator->kill_running_app();
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
    const std::string path_to_cache_dir(MySteam::get_steam_install_path() + "/appcache/stats/");
    DIR* dirp = opendir(path_to_cache_dir.c_str());
    struct dirent * dp;
    std::string filename;
    const std::string prefix("UserGameStats_" + MySteam::get_user_steamId3() + "_");
    const std::string input_scheme_c(prefix + "%lu.bin");
    Game_t game;
    unsigned long app_id;
    SteamAppDAO* appDAO = SteamAppDAO::get_instance();

    // The whole update will really occur only once in a while, no worries
    appDAO->update_name_database();
    m_all_subscribed_apps.clear();

    while ((dp = readdir(dirp)) != NULL) {
        filename = dp->d_name;
        if(filename.rfind(prefix, 0) == 0) {
            if(sscanf(dp->d_name, input_scheme_c.c_str(), &app_id) == 1) {
                game.app_id = app_id;
                game.app_name = appDAO->get_app_name(app_id);

                m_all_subscribed_apps.push_back(game);
            }
        }
    }

    closedir(dirp);
}
// => refresh_owned_apps


/**
 * Could parse /home/user/.local/share/Steam/config/loginusers.vdf, but wrong id type
 * Parses STEAM/logs/parental_log.txt, hoping those logs can't be disabled
 * Return the most recently logged in user id
 * Returns empty string on error
 */
std::string 
MySteam::get_user_steamId3() {
    static const std::string file_path(MySteam::get_steam_install_path() + "/logs/parental_log.txt");
    std::ifstream input(file_path, std::ios::in);
    std::string word;
    
    if(!input) {
        std::cerr << "Could not open " << file_path << std::endl;
        std::cerr << "Make sure you have a default steam installation, and that logging is not disabled." << std::endl;
        input.close();
        exit(EXIT_FAILURE);
    }

    //We're done setting up and checking, let's parse this file
    bool next_is_id = false;
    std::string latest_id = "";

    while(input >> word) {
        if(word == "ID:") {
            next_is_id = true;
            continue;
        }

        if(next_is_id) {
            latest_id = word;
            next_is_id = false;
        }
    }

    input.close();

    return latest_id;
}
// => get_user_steamId3


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
    std::cerr << "Summary of owned apps with stats or achievements" << std::endl << "========================" << std::endl;

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