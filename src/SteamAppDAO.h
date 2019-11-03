#pragma once
#include <map>
#include "../steam/steamtypes.h"


class SteamAppDAO
{
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
    std::string get_app_name(AppId_t app_id);

    /**
     * Download the app's banner.
     * If it fails, nothing is written on the disk.
     */
    static void download_app_icon(AppId_t app_id);

    /**
     * After it parsed all apps from the latest updates, returns the parsed apps
     */
    auto get_all_apps() {return m_app_names;};

    /**
     * Returns true if the given appId is owned by the curent steam user
     * */
    bool app_is_owned(const AppId_t& app_id);
    
    /**
     * Delete these to avoid any singleton bypass
     */
    SteamAppDAO(SteamAppDAO const&)                 = delete;
    void operator=(SteamAppDAO const&)              = delete;
private:
    SteamAppDAO() {};
    ~SteamAppDAO() {};

    static void parse_app_names(const char * file_path);
    static std::map<AppId_t, std::string> m_app_names;
};