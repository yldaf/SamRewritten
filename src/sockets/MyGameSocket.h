#pragma once
#include "MyServerSocket.h"
#include "../types/Achievement.h"
#include "../../steam/steam_api.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <thread>
#include <atomic>

class MyGameSocket : public MyServerSocket
{
public:
    std::string process_request(std::string request, bool& quit);
    std::vector<Achievement_t> get_achievements(void);
    bool get_global_stats(void);
    bool get_global_achievements_stats(void);
    void process_changes(std::vector<AchievementChange_t> changes);

    MyGameSocket(AppId_t appid);

    /**
     * Steam API callback to handle the received stats and achievements
     */
    STEAM_CALLBACK( MyGameSocket, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived );

private:
    std::atomic_bool m_stats_callback_received;
    std::atomic_bool m_global_callback_received;
    std::atomic_bool m_global_achievements_callback_ready;

    void OnGlobalStatsReceived(GlobalStatsReceived_t* callback, bool bIOFailure);
    CCallResult< MyGameSocket, GlobalStatsReceived_t > m_GlobalStatsReceivedCallResult;

    void OnGlobalAchievementPercentagesReceived(GlobalAchievementPercentagesReady_t* callback, bool bIOFailure);
    CCallResult< MyGameSocket, GlobalAchievementPercentagesReady_t > m_GlobalAchievementPercentagesReadyCallResult;

    std::vector<Achievement_t> m_achievement_list;

};

