#pragma once
#include <yajl/yajl_tree.h>
#include <vector>
#include "../types/Actions.h"
#include "../types/Achievement.h"
#include "../types/StatValue.h"

/**
 * This is a serverside class
 */

class ProcessedGameServerRequest {
public:
    ProcessedGameServerRequest(const std::string& request);
    ~ProcessedGameServerRequest();

    SAM_ACTION getAction() const {return m_action;};
    std::vector<AchievementChange_t> get_achievement_changes() const;
    std::vector<StatChange_t> get_stat_changes() const;
protected:
    SAM_ACTION m_action;
    yajl_val m_fulltree;
    yajl_val m_achievement_changes_array;
    yajl_val m_stat_changes_array;
};