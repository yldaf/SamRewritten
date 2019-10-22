#pragma once
#include <yajl/yajl_tree.h>
#include <vector>
#include "../types/Actions.h"
#include "../types/Achievement.h"

/**
 * This is a serverside class
 */

class ProcessedGameServerRequest {
public:
    ProcessedGameServerRequest(const std::string& request);
    ~ProcessedGameServerRequest();

    SAM_ACTION getAction() const {return m_action;};
    yajl_val getPayload() const {return m_payload;};
    std::vector<AchievementChange_t> payload_to_ach_changes() const;
protected:
    SAM_ACTION m_action;
    yajl_val m_payload;
    yajl_val m_fulltree;
};