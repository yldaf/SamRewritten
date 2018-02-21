/**
 * Steam is a class that serves the purpose of being a "launcher" for thesteam games and apps.
 * It will refuse to do anything unless a real Steam client can be accessed, with a user logged in.
 */

#define MAX_PATH 1000
#include "MySteam.h"

MySteam::MySteam() : m_child_pid(-1) {
    //Not necessary, going to call it only when neededs
    //refresh_owned_apps();
}

MySteam::~MySteam() {

}

MySteam* MySteam::get_instance() {
    static MySteam instance;
    return &instance;
}

/**
 * Fakes a new game being launched. Keeps running in the background until quit_game is called.
 */
bool MySteam::launch_game(std::string appID) {
    // Print an error if a game is already launched, maybe allow multiple games at the same time in the future?
    if(m_child_pid > 0) {
        std::cout << "Sorry, a game is already running. Please try again later." << std::endl;
        return false;
    }

    //TODO Change this for release as the cwd will be different
    if(!file_exists("./bin/samgame")) {
        std::cout << "samgame doesn't exist. Aborting launch." << std::endl;
        return false;
    }

    m_child_pid = create_process();

    switch(m_child_pid) {
        case -1:
            std::cout << "An error occurred while creating the cild process." << std::endl;
            perror("fork");
            break;

        case 0:
            execl("./bin/samgame", "samgame", appID.c_str(), (char*) NULL);
            break;

        default:
            return true;
            break;
    }
    return false;
}

bool MySteam::quit_game() {
    if(m_child_pid > 0) {
        kill(m_child_pid, SIGTERM);
    }
    else {
        std::cout << "Cannot quit game, no game launched." << std::endl;
        return false;
    }

    m_child_pid = -1;
    return true;
}

/**
 * This does NOT retrieves all owned games.
 * It does retrieve all owned games WITH STATS or ACHIEVEMENTS
 * Stores the owned games in m_all_subscribed_apps
 * We assume the user didn't put any garbage in his steam folder as well.
 */
void MySteam::refresh_owned_apps() {
    const std::string path_to_cache_dir(MySteam::get_steam_install_path() + "/appcache/stats/");
    DIR* dirp = opendir(path_to_cache_dir.c_str());
    struct dirent * dp;
    std::string filename;
    const std::string prefix("UserGameStats_" + MySteam::get_user_steamId3() + "_");
    const std::string input_scheme_c(prefix + "%lu.bin");
    Game_t game;
    unsigned long app_id;

    // The whole update will really occur only once in a while, no worries
    SteamAppDAO::update_name_database();
    m_all_subscribed_apps.clear();

    while ((dp = readdir(dirp)) != NULL) {
        filename = dp->d_name;
        if(filename.rfind(prefix, 0) == 0) {
            if(sscanf(dp->d_name, input_scheme_c.c_str(), &app_id) == 1) {
                game.app_id = app_id;
                game.app_name = "To retrieve";

                m_all_subscribed_apps.push_back(game);
            }
        }
    }

    closedir(dirp);
}

/**
 * Could parse /home/paul/.local/share/Steam/config/loginusers.vdf, but wrong id type
 * Parses STEAM/logs/parental_log.txt, hoping those logs can't be disabled
 * Return the most recently logged in user id
 * Returns empty string on error
 */
std::string MySteam::get_user_steamId3() {
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

std::string MySteam::get_steam_install_path() {
    static const std::string home_path(getenv("HOME"));
    if(file_exists(home_path + "/.local/share/Steam/appcache/appinfo.vdf")) {
        return std::string(home_path + "/.local/share/Steam");
    }
    else if(file_exists(home_path + "/.steam/appcache/appinfo.vdf")) {
        return std::string(home_path + "/.steam");
    }
    else {
        std::cerr << "Unable to locate the steam directory. TODO: implement a folder picker here" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void MySteam::print_all_owned_games() const {
    std::cerr << "Summary of owned apps with stats or achievements" << std::endl << "========================" << std::endl;

    for(Game_t i : m_all_subscribed_apps) {
        std::cerr << i.app_id << " -> " << i.app_name << std::endl;
    }
}