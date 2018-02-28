#include "SteamAppDAO.h"

// Wtf am I doing? Anyway thanks StackOverflow
//TODO: Find a more elegant way to fix this shit.
std::map<unsigned long, std::string> SteamAppDAO::m_app_names = std::map<unsigned long, std::string>();
const char* SteamAppDAO::CACHE_FOLDER = concat(getenv("HOME"), "/.SamRewritten");

void 
SteamAppDAO::update_name_database() {
    bool need_to_redownload = false;
    struct stat file_info;
    static const char* local_file_name = concat(SteamAppDAO::CACHE_FOLDER, "/app_names");
    const std::time_t current_time(std::time(0));

	// Make sure the cache folder is there
    const int mkdir_error = mkdir(SteamAppDAO::CACHE_FOLDER, S_IRWXU | S_IRWXG | S_IROTH);
    if(mkdir_error != 0 && errno != EEXIST) {
		std::cerr << "Unable to create the cache folder ( ~/.SamRewritten/, errno " << errno << ")." << std::endl;
        std::cerr << "Don't tell me you're running this as root.." << std::endl;
        exit(EXIT_FAILURE);
	}

    // Check if the file is already there
    if (file_exists(local_file_name)) {
        //Check the last time it was updated
        if(stat(local_file_name, &file_info) == 0) {
            //If a week has passed
            if(current_time - file_info.st_mtime > 60 * 60 * 24 * 7) {
                need_to_redownload = true;
            } else {
                // An up-to-date file is present on the system.
                // If our map is already filled, there's no need to refill it.
                // If the program was just launched, we need to fill it.
                if(SteamAppDAO::m_app_names.empty()) {
                    SteamAppDAO::parse_app_names();
                }
            }
        }
        else {
            std::cerr << "~/.SamRewritten/app_names exists but an error occurred analyzing it. To avoid further complications, ";
            std::cerr << "the program will stop here. Before retrying make sure you have enough privilege to read and write to ";
            std::cerr << "your home folder folder." << std::endl;

            exit(EXIT_FAILURE);
        }
    }
    else {
        need_to_redownload = true;
    }

    if(need_to_redownload) {
        Downloader::get_instance()->download_file("http://api.steampowered.com/ISteamApps/GetAppList/v0002/", local_file_name, 0);
        SteamAppDAO::parse_app_names();
    }
}

std::string 
SteamAppDAO::get_app_name(const unsigned long& app_id) {
    return m_app_names[app_id];
}


void 
SteamAppDAO::download_app_icon(const unsigned long& app_id) {
    const std::string local_folder(std::string(SteamAppDAO::CACHE_FOLDER) + "/" + std::to_string(app_id));
    const std::string local_path(local_folder + "/banner");
    const std::string url("http://cdn.akamai.steamstatic.com/steam/apps/" + std::to_string(app_id) + "/header_292x136.jpg");

    const int mkdir_error = mkdir(local_folder.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
    if(mkdir_error != 0 && errno != EEXIST) {
		std::cerr << "Unable to create the cache folder (" << local_folder << ", errno " << errno << ")." << std::endl;
        exit(EXIT_FAILURE);
	}

    Downloader::get_instance()->download_file_async(url, local_path, app_id);
}

void 
SteamAppDAO::parse_app_names() {
    m_app_names.clear();

    static const std::string file_path(std::string(SteamAppDAO::CACHE_FOLDER) + "/app_names");
    std::ifstream input(file_path, std::ios::in);
    std::string word;
    std::string app_name;
    bool next_is_appId = false;
    bool next_is_name = false;
    unsigned long curr_id;
    std::stringstream converter;

    if(input) {
        while(input >> word) {
            if(next_is_appId) {
                converter.clear();
                converter << word.substr(0, word.size() - 1); //Remove the comma
                if(!(converter >> curr_id)) {
                    std::cerr << "An error occurred parsing the files. Make sure you left " << file_path << " unaltered." << std::endl;
                    std::cerr << "Error on word: " << word.substr(0, word.size() - 1) << std::endl;
                    exit(EXIT_FAILURE);
                }
                next_is_appId = false;
                continue;
            }
            else if(next_is_name) {
                if(word == "}," || word == "}") {
                    app_name = app_name.substr(1, app_name.size() - 3);
                    m_app_names.insert(std::pair<unsigned long, std::string>(curr_id, app_name));
                    next_is_name = false;
                    app_name.clear();
                    continue;
                }
                app_name += word + " ";
                continue;
            }
            else if (word == "\"appid\":") {
                next_is_appId = true;
                continue;
            }
            else if (word == "\"name\":") {
                next_is_name = true;
                continue;
            }
        }
    }
    else {
        std::cerr << "Could not retrieve app names. Unable to open " << file_path << std::endl;
        exit(EXIT_FAILURE);
    }
}

void
SteamAppDAO::update(unsigned long i) {
    g_main_gui->refresh_app_icon(i);
}