#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>
#include "MyGameSocket.h"
#include "../types/Actions.h"
#include "../common/yajlHelpers.h"

MyGameSocket::MyGameSocket(AppId_t appid) :
MyServerSocket(appid),
m_CallbackUserStatsReceived( this, &MyGameSocket::OnUserStatsReceived )
{

}

std::string
MyGameSocket::process_request(std::string request) {

    //TODO encapsulate these into a json parser?
    //encoding this response is still tightly coupled to the
    // logic in this function, so it's hard to push it to a helper

    std::string action = decode_request(request);

    std::string ret;
    const unsigned char * buf; 
    size_t len;

    // Generate the ack
    //TODO encapsulate these into a json generator
    yajl_gen handle = yajl_gen_alloc(NULL); 
    yajl_gen_map_open(handle);

    // generate ACK with same variable name and content
    yajl_gen_string_wrap(handle, SAM_ACK_STR);
    yajl_gen_string_wrap(handle, SAM_ACK_STR);

    // TODO: change to enums? since it's JSON, using strings is necessary sometime
//    switch (request_type) {
    if (action == GET_ACHIEVEMENTS_STR) {
    //case GET_ACHIEVEMENTS:
        std::vector<Achievement_t> achievements = get_achievements();
        // Steam api is launched in this context, other possible implementation: game_utils->get_achievements()
        
        // Append the achievements to the ack
        encode_achievements(handle, achievements);

        //break;
    } else if (action == STORE_ACHIEVEMENTS_STR) {
    //case STORE_ACHIEVEMENTS:
        // TODO: achievement ID string, lock or relock boolean
        //std::vector<std::pair<string, bool>> operations = JSON::parse_achievement_array(request);
        //long term TODO: extend to stats
        
        //process_achievements(operations); // or game_utils->process_achievements(..)
        //ret = SAM_ACK_BUT_IN_JSON;
        //break;
    } else if (action == QUIT_GAME_STR) {
    //case QUIT_GAME:
        SteamAPI_Shutdown();
            
    } else {
    //default:
        std::cerr << "Invalid command" << std::endl;
        ret.clear();
    }

    if (yajl_gen_map_close(handle) != yajl_gen_status_ok) {
        std::cerr << "failed to make json" << std::endl;
    }
    yajl_gen_get_buf(handle, &buf, &len);
    ret = std::string((const char*)buf);
    yajl_gen_free(handle);

    return ret;
}

std::vector<Achievement_t>
MyGameSocket::get_achievements() {

    m_stats_callback_received = false;

    ISteamUserStats *stats_api = SteamUserStats();
    if (!stats_api->RequestCurrentStats()) {
        std::cerr << "ERROR: User not logged in, exiting" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (!m_stats_callback_received) {
        // for debugging how long steam callbacks take
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        SteamAPI_RunCallbacks();
    }

    return m_achievement_list;
}

void
MyGameSocket::OnUserStatsReceived(UserStatsReceived_t *callback) {

    // Check if we received the values for the correct app
    if (std::string(getenv("SteamAppId")) == std::to_string(callback->m_nGameID)) {
        if ( k_EResultOK == callback->m_eResult ) {
            ISteamUserStats *stats_api = SteamUserStats();

            // ==============================
            // RETRIEVE IDS
            // ==============================
            const unsigned num_ach = stats_api->GetNumAchievements();

            if (num_ach == 0) {
                std::cerr << "No achievements for current game" << std::endl;
            }

            m_achievement_list.clear();
            m_achievement_list.resize(num_ach);

            for (unsigned i = 0; i < num_ach ; i++) {

                m_achievement_list[i].id   = stats_api->GetAchievementName(i);

                const char * pchName = m_achievement_list[i].id.c_str();

                m_achievement_list[i].name = stats_api->GetAchievementDisplayAttribute(pchName, "name");
                m_achievement_list[i].desc = stats_api->GetAchievementDisplayAttribute(pchName, "desc");

                // TODO
                // https://partner.steamgames.com/doc/api/ISteamUserStats#RequestGlobalAchievementPercentages
                //stats_api->GetAchievementAchievedPercent(m_achievement_list[i].id, &(m_achievement_list[i].global_achieved_rate));
                m_achievement_list[i].global_achieved_rate = 0;
                stats_api->GetAchievement(pchName, &(m_achievement_list[i].achieved));
                m_achievement_list[i].hidden = (bool)strcmp(stats_api->GetAchievementDisplayAttribute(pchName, "hidden" ), "0");
                // TODO: incorrect as is
                //m_achievement_list[i].icon_handle = stats_api->GetAchievementIcon(pchName);
                m_achievement_list[i].icon_handle = 0;
            }

            m_stats_callback_received = true;

        } else {
            std::cerr << "Received stats for the game, but an error occurrred." << std::endl;
        }
    } else {
        std::cerr << "Received stats for wrong game" << std::endl;
    }
}


void
MyGameSocket::process_achievements(std::vector<std::pair<std::string, bool>> changes) {
    //Untested

    ISteamUserStats *stats_api = SteamUserStats();

    for (unsigned i = 0; i < changes.size(); i++)
    {
        const char* achievement_id = changes[i].first.c_str();

        if (changes[i].second) {
            // We want to unlock an achievement
            if (!stats_api->SetAchievement(achievement_id)) {
                std::cerr << "Unlocking achievement " << achievement_id << " failed " << std::endl;
            } else {
                std::cerr << "Unlocked achievement " << achievement_id << std::endl;
            }
        } else {
            // We want to relock an achievement
            if (!stats_api->ClearAchievement(achievement_id)) {
                std::cerr << "Relocking achievement " << achievement_id << " failed" << std::endl;
            } else {
                std::cerr << "Relocked achievement " << achievement_id << std::endl;
            }
        }

        //TODO: stats

    }

    // Auto-commit after we receive everything
    if (!stats_api->StoreStats()) {
        std::cerr << "Committing changes failed" << std::endl;
    }

}