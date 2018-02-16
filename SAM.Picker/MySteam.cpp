/**
 * Steam is a class that serves the purpose of being a "launcher" for thesteam games and apps.
 * It will refuse to do anything unless a real Steam client can be accessed, with a user logged in.
 */

#define MAX_PATH 1000
#include "MySteam.h"

MySteam::MySteam() : m_child_pid(-1) {
    get_all_owned_games();
}

MySteam::~MySteam() {
    #ifdef DEBUG
    puts("MySteam deleted");
    #endif
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

void MySteam::get_all_owned_games() {
    ISteamApps* steamapps = SteamApps();
    ISteamAppList* list = SteamAppList();
    //ISteamUserStats* stats = SteamUserStats();

    AppId_t appid;
    char name[256];
    Game_t app;
    int name_length;

    m_all_subscribed_apps.clear();
    for(unsigned long i = 10; i < 10000; i++) {
        appid = i;
        if(steamapps->BIsSubscribedApp(appid)) {
            name_length = list->GetAppName(i, name, 256);
            if(name_length > 0) {
                app.app_name = std::string(name);
                app.app_id = i;
                app.number_achievements = 1; // TODO put a real value here
                m_all_subscribed_apps.push_back(app);
            }
        }
    }
}

/**
 * Could parse /home/paul/.local/share/Steam/config/loginusers.vdf, but wrong id type
 * Parses STEAM/logs/parental_log.txt, hoping those logs can't be disabled
 * Return the most recently logged in user id
 * Returns empty string on error
 */
std::string MySteam::get_user_steamId3() {
    static const std::string file_path(std::string(getenv("HOME")) + "/.local/share/Steam/logs/parental_log.txt");
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

    std::cerr << "User ID is " << latest_id << std::endl;

    return latest_id;
}

void MySteam::print_all_owned_games() const {
    for(Game_t i : m_all_subscribed_apps) {
        std::cerr << "App owned: " << i.app_name << std::endl;
    }
}