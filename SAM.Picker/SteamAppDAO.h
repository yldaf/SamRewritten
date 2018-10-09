#pragma once
#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <yajl/yajl_tree.h>
#include <stdio.h>
#include <stdlib.h>
#include "../common/functions.h"
#include "../common/Downloader.h"
#include "globals.h"
#include "MainPickerWindow.h"

class SteamAppDAO : public Observer<unsigned long> {
public:
    /**
     * Singleton method to get the unique instance
     */
    static SteamAppDAO* get_instance();
    
    /**
     * Redownloads http://api.steampowered.com/ISteamApps/GetAppList/v0002/
     * if necessary. Redownloads every few days.
     * TODO: Maybe pass a boolean too as argument for "Override redownload"
     */
    void update_name_database();

    /**
     * Feed it an appId, returns the app name.
     * Make sure to call update_name_database at least once 
     * before using.
     */
    std::string get_app_name(const unsigned long& app_id);

    /**
     * Download the app's banner ASYNCHRONOUSLY.
     * If it fails, nothing is written on the disk.
     */
    void download_app_icon(const unsigned long& app_id);

    /**
     * Observer inherited method. The update will refresh 
     * the image for app id "i" on the view.
     */
    void update(unsigned long i);
    
    /**
     * Delete these to avoid any singleton bypass
     */
    SteamAppDAO(SteamAppDAO const&)                 = delete;
    void operator=(SteamAppDAO const&)              = delete;
private:
    SteamAppDAO() {Downloader::get_instance()->attach(this);};
    ~SteamAppDAO() {};
    static void parse_app_names_v2();

    static std::map<unsigned long, std::string> m_app_names;
};