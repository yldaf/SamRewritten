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
    void process_changes(std::vector<AchievementChange_t> changes);

    MyGameSocket(AppId_t appid);

    /**
     * Steam API callback to handle the received stats and achievements
     */
    STEAM_CALLBACK( MyGameSocket, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived );

private:
    std::atomic_bool m_stats_callback_received;
    std::vector<Achievement_t> m_achievement_list;

};

