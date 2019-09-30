#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>
#include "MyGameSocket.h"
#include "../types/Actions.h"
#include "../json/ProcessedGameServerRequest.h"
#include "../json/yajlHelpers.h"

MyGameSocket::MyGameSocket(AppId_t appid) :
MyServerSocket(appid),
m_CallbackUserStatsReceived( this, &MyGameSocket::OnUserStatsReceived )
{

}

std::string
MyGameSocket::process_request(std::string request, bool& quit) {
    ProcessedGameServerRequest r(request);
    std::string ret;
    const unsigned char * buf;
    size_t len;

    // Generate the ack
    yajl_gen handle = yajl_gen_alloc(NULL); 
    yajl_gen_map_open(handle);

    encode_ack(handle);

    switch (r.getAction()) {
        case GET_ACHIEVEMENTS:
            // Write achievements to handle
            encode_achievements(handle, get_achievements());
            break;

        case STORE_ACHIEVEMENTS:
            process_changes( r.payload_to_ach_changes() );
            break;

        case QUIT_GAME:
            SteamAPI_Shutdown();
            quit = true;
            break;

        default:
            std::cerr << "Invalid command" << std::endl;
            break;
    }

    if (yajl_gen_map_close(handle) != yajl_gen_status_ok) {
        std::cerr << "Failed to make json." << std::endl;
        exit(EXIT_FAILURE);
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
        SteamAPI_RunCallbacks();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
                m_achievement_list[i].icon_handle = stats_api->GetAchievementIcon(pchName);
                //m_achievement_list[i].icon_handle = 0;
            }

        } else {
            std::cerr << "Received stats for the game, but an error occurrred." << std::endl;
        }
    } else {
        std::cerr << "Received stats for wrong game" << std::endl;
    }

    m_stats_callback_received = true;
}


void
MyGameSocket::process_changes(std::vector<AchievementChange_t> changes) {
    //Untested

    ISteamUserStats *stats_api = SteamUserStats();

    for (AchievementChange_t change : changes) {

        const char* achievement_id = change.id.c_str();

        if (change.achieved) {
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