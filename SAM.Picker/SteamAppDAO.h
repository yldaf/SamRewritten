#pragma once
#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <sys/stat.h>
#include <dirent.h>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include "../common/c_processes.h"

class SteamAppDAO {
public:
    /**
     * Redownloads http://api.steampowered.com/ISteamApps/GetAppList/v0002/
     * if necessary. Redownloads every few days.
     * TODO: Maybe pass a boolean too as argument for "Override redownload"
     */
    static void update_name_database();

    /**
     * Feed it an appId, returns the app name.
     * Make sure to call update_name_database at least once 
     * before using.
     */
    static std::string get_app_name(const unsigned long& app_id);

    /**
     * Download the app's banner ASYNCHRONOUSLY.
     * If it fails, nothing is written.
     */
    static void download_app_icon(const unsigned long& app_id);

    /**
     * Path name to the root of the cache folder. By now it is
     * ~/.SamRewritten
     */
    static const char *CACHE_FOLDER;

private:
    static void download_file(const std::string& file_url, const std::string& local_path);
    static void parse_app_names();

    static std::map<unsigned long, std::string> m_app_names;
};