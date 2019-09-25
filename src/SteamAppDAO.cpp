#include "SteamAppDAO.h"
#include <ctime>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <yajl/yajl_tree.h>
#include <stdio.h>
#include <stdlib.h>
#include "common/functions.h"
#include "common/Downloader.h"
#include "../steam/steam_api.h"
#include "MySteamClient.h"
#include "gui/MainPickerWindow.h"
#include "globals.h"

// Wtf am I doing? Anyway thanks StackOverflow
//TODO: Find a more elegant way to fix this shit.
std::map<AppId_t, std::string> SteamAppDAO::m_app_names = std::map<AppId_t, std::string>();

/**
 * Observer pattern setup
 */
SteamAppDAO::SteamAppDAO() {
    Downloader::get_instance()->attach(this);
}

/**
 * Lazy singleton pattern
 */
SteamAppDAO*
SteamAppDAO::get_instance() {
    static SteamAppDAO me;
    return &me;
}
// => get_instance


void 
SteamAppDAO::update_name_database() {
    bool need_to_redownload = false;
    struct stat file_info;
    static const char* local_file_name = concat(g_cache_folder, "/app_names");
    const std::time_t current_time(std::time(0));

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
            std::cerr << "~/.cache/SamRewritten/app_names exists but an error occurred analyzing it. To avoid further complications, ";
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
SteamAppDAO::get_app_name(const AppId_t& app_id) {
    return m_app_names[app_id];
}


void 
SteamAppDAO::download_app_icon(const AppId_t& app_id) {
    const std::string local_folder(std::string(g_cache_folder) + "/" + std::to_string(app_id));
    const std::string local_path(local_folder + "/banner");
    const std::string url("http://cdn.akamai.steamstatic.com/steam/apps/" + std::to_string(app_id) + "/header_292x136.jpg");

    mkdir_default(local_folder.c_str());

    Downloader::get_instance()->download_file_async(url, local_path, app_id);
}

void
SteamAppDAO::parse_app_names() {
    m_app_names.clear();

    size_t rd;
    yajl_val node;
    char errbuf[1024];
    char fileData[5000000];
    static const std::string file_path(std::string(g_cache_folder) + "/app_names");
    FILE *f = fopen(file_path.c_str(), "rb");

    /* null plug buffers */
    fileData[0] = errbuf[0] = 0;

    /* read the entire config file */
    rd = fread((void *) fileData, 1, sizeof(fileData) - 1, f);

    /* file read error handling */
    if (rd == 0 && !feof(stdin)) {
        std::cerr << "error encountered on file read" << std::endl;
        exit(EXIT_FAILURE);
    } else if (rd >= sizeof(fileData) - 1) {
        std::cerr << "app_names file too big (just increase the buffer size)" << std::endl;
        exit(EXIT_FAILURE);
    }

    /* we have the whole config file in memory. let's parse it ... */
    node = yajl_tree_parse((const char *) fileData, errbuf, sizeof(errbuf));

    /* parse error handling */
    if (node == NULL) {
        std::cerr << "Parsing error: ";
        if (strlen(errbuf)) {
            std::cerr << errbuf << std::endl;
        } else {
            std::cerr << "Unknown error" << std::endl;
        }
        exit(EXIT_FAILURE);
    }

    /* Save the result */
    const char * path[] = { "applist", "apps", (const char*)0 };
    yajl_val v = yajl_tree_get(node, path, yajl_t_array);
    if (v == NULL) {
        std::cerr << "app_names contains valid JSON, but its format is not supported" << std::endl;
        exit(EXIT_FAILURE);
    }

    unsigned array_length = v->u.array.len;
    AppId_t tmp_appid;
    std::string tmp_appname;
    for(unsigned i = 0; i < array_length; i++) {
        yajl_val obj = v->u.array.values[i];
        //std::cerr << "Appid: " << obj->u.object.values[0]->u.number.i << ", name: " << obj->u.object.values[1]->u.string << std::endl;
        tmp_appid = obj->u.object.values[0]->u.number.i;
        tmp_appname = (std::string)obj->u.object.values[1]->u.string;
        m_app_names.insert(std::pair<AppId_t, std::string>(tmp_appid, tmp_appname));
    }

    yajl_tree_free(node);
}

bool 
SteamAppDAO::app_is_owned(const AppId_t& app_id) {
    ISteamApps* sa = g_steamclient->getSteamApps();
    
    return sa->BIsSubscribedApp(app_id);
}

void
SteamAppDAO::update(AppId_t i) {
    g_main_gui->refresh_app_icon(i);
}