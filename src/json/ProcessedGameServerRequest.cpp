#include "ProcessedGameServerRequest.h"
#include "../common/functions.h"
#include <iostream>

SAM_ACTION
str2action(const std::string& str) {
    if (str == GET_ACHIEVEMENTS_STR)
    {
        return GET_ACHIEVEMENTS;
    }
    else if (str == STORE_ACHIEVEMENTS_STR)
    {
        return STORE_ACHIEVEMENTS;
    }
    else if (str == QUIT_GAME_STR)
    {
        return QUIT_GAME;
    }
    else {
        return INVALID;
    }
}

ProcessedGameServerRequest::ProcessedGameServerRequest(const std::string& request) 
{
    m_fulltree = yajl_tree_parse(request.c_str(), NULL, 0);

    if (m_fulltree == NULL) {
        std::cerr << "Parsing error. Data: " << std::endl;
        std::cerr << request << std::endl;
        zenity();
        exit(EXIT_FAILURE);
    }

    const char * path[] = { SAM_ACTION_STR, (const char*)0 };
    const char * list_path[] = { ACHIEVEMENT_LIST_STR, (const char*)0 };

    yajl_val v = yajl_tree_get(m_fulltree, path, yajl_t_string);
    if (v == NULL || !YAJL_IS_STRING(v)) {
        std::cerr << "failed to get " << SAM_ACTION_STR << std::endl;
        zenity();
        exit(EXIT_FAILURE);
    }

    // Some Requests include a payload (eg STORE_ACHIEVEMENTS, we'll have to deal with it at some point)
    switch (m_action = str2action(YAJL_GET_STRING(v)))
    {
    case STORE_ACHIEVEMENTS:
        m_payload = yajl_tree_get(m_fulltree, list_path, yajl_t_array);
        break;
    
    default:
        break;
    }
}

ProcessedGameServerRequest::~ProcessedGameServerRequest()
{
    yajl_tree_free(m_fulltree);
}

std::vector<AchievementChange_t> 
ProcessedGameServerRequest::payload_to_ach_changes() const
{
    std::vector<AchievementChange_t> changes;

    const char * id_path[] = { ID_STR, (const char*)0 };
    const char * achieved_path[] = { ACHIEVED_STR, (const char*)0 };

    yajl_val *w = YAJL_GET_ARRAY(m_payload)->values;
    size_t array_len = YAJL_GET_ARRAY(m_payload)->len;

    changes.resize(array_len);

    for(unsigned i = 0; i < array_len; i++) {
        yajl_val cur_node = w[i];
        yajl_val cur_val;

        // TODO: pull these out into a decode_change()?
        // verification is done via the type argument to yajl_tree_get
        // and via YAJL_IS_* checks if type alone isn't sufficient
        cur_val = yajl_tree_get(cur_node, id_path, yajl_t_string);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
        }
        changes[i].id = YAJL_GET_STRING(cur_val);
        
        // why is bool parsing weird
        cur_val = yajl_tree_get(cur_node, achieved_path, yajl_t_any);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
        }
        if (!YAJL_IS_TRUE(cur_val) && !YAJL_IS_FALSE(cur_val)) {
            std::cerr << "bool parsing error" << std::endl;
        }
        changes[i].achieved = YAJL_IS_TRUE(cur_val);
    }

    return changes;
}
