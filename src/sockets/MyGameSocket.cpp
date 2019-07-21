#include "MyGameSocket.h"
#include "../types/Actions.h"

MyGameSocket::MyGameSocket(AppId_t appid) :
MyServerSocket(appid),
m_CallbackUserStatsReceived( this, &MyGameSocket::OnUserStatsReceived )
{

}

std::string
MyGameSocket::process_request(std::string request) {
    // Logic goes here TODO

    //TODO: yajl parse the JSON-encoded request string (then get request type?) or are strings good enough?
    //SAM_ACTION request_type = get_request_type(request)
    std::string ret;

//    switch (request_type) {
    if (request == GET_ACHIEVEMENTS_STR) {
    //case GET_ACHIEVEMENTS:
        //this requires an async callback
        std::vector<Achievement_t> achievements = get_achievements();
        // Steam api is launched in this context, other possible imlementation: game_utils->get_achievements()
        //ret = JSON::achievement_vector(achievements);
        //break;
    } else if (request == STORE_ACHIEVEMENTS_STR) {
    //case STORE_ACHIEVEMENTS:
        // TODO: achievement ID string, lock or relock boolean
        //std::vector<std::pair<string, bool>> operations = JSON::parse_achievement_array(request);
        //long term TODO: extend to stats
        
        //process_achievements(operations); // or game_utils->process_achievements(..)
        //ret = SAM_ACK_BUT_IN_JSON;
        //break;
    } else if (request == QUIT_GAME_STR) {
    //case QUIT_GAME:
        SteamAPI_Shutdown();
    } else {
    //default:
        std::cerr << "Invalid command" << std::endl;
        ret.clear();
    }

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
        //std::cerr << "waiting for callback" << std::endl;
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
            m_achievement_list.reserve(num_ach);

            for (unsigned i = 0; i < num_ach ; i++) {
                // TODO: strncpy is slow, because it fills the remaining space with NULLs
                // This is last stage optimisation but, could have used strcpy, or sprintf,
                // making sure strings are NULL terminated
                // see "man strncpy" for a possible implementation
                strncpy(
                    m_achievement_list[i].id,
                    stats_api->GetAchievementName(i),
                    MAX_ACHIEVEMENT_ID_LENGTH);

                strncpy(
                    m_achievement_list[i].name,
                    stats_api->GetAchievementDisplayAttribute(m_achievement_list[i].id, "name"),
                    MAX_ACHIEVEMENT_NAME_LENGTH);

                strncpy(
                    m_achievement_list[i].desc,
                    stats_api->GetAchievementDisplayAttribute(m_achievement_list[i].id, "desc"),
                    MAX_ACHIEVEMENT_DESC_LENGTH);

                // TODO
                // https://partner.steamgames.com/doc/api/ISteamUserStats#RequestGlobalAchievementPercentages
                //stats_api->GetAchievementAchievedPercent(m_achievement_list[i].id, &(m_achievement_list[i].global_achieved_rate));
                m_achievement_list[i].global_achieved_rate = 0;
                stats_api->GetAchievement(m_achievement_list[i].id, &(m_achievement_list[i].achieved));
                m_achievement_list[i].hidden = (bool)strcmp(stats_api->GetAchievementDisplayAttribute( m_achievement_list[i].id, "hidden" ), "0");
                m_achievement_list[i].icon_handle = stats_api->GetAchievementIcon( m_achievement_list[i].id );
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