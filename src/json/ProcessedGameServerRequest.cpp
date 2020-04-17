#include "ProcessedGameServerRequest.h"
#include "../common/functions.h"
#include <iostream>

SAM_ACTION
str2action(const std::string& str) {
    if (str == GET_ACHIEVEMENTS_STR)
    {
        return GET_ACHIEVEMENTS;
    }
    else if (str == COMMIT_CHANGES_STR)
    {
        return COMMIT_CHANGES;
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
    const char * achievement_list_path[] = { ACHIEVEMENT_LIST_STR, (const char*)0 };
    const char * stat_list_path[] = { STAT_LIST_STR, (const char*)0 };

    yajl_val v = yajl_tree_get(m_fulltree, path, yajl_t_string);
    if (v == NULL) {
        std::cerr << "failed to get " << SAM_ACTION_STR << std::endl;
        zenity();
        exit(EXIT_FAILURE);
    }

    // Some requests include a payload (eg COMMIT_CHANGES, we'll have to deal with it at some point)
    switch (m_action = str2action(YAJL_GET_STRING(v))) {
        case COMMIT_CHANGES:
            m_achievement_changes_array = yajl_tree_get(m_fulltree, achievement_list_path, yajl_t_array);
            m_stat_changes_array = yajl_tree_get(m_fulltree, stat_list_path, yajl_t_array);
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
ProcessedGameServerRequest::get_achievement_changes() const
{
    std::vector<AchievementChange_t> changes;

    const char * id_path[] = { ID_STR, (const char*)0 };
    const char * achieved_path[] = { ACHIEVED_STR, (const char*)0 };
    
    if (m_achievement_changes_array == NULL) {
        std::cerr << "parsing error" << std::endl;
        return changes;
    }

    yajl_val *w = YAJL_GET_ARRAY(m_achievement_changes_array)->values;
    size_t array_len = YAJL_GET_ARRAY(m_achievement_changes_array)->len;

    changes.resize(array_len);

    for(unsigned i = 0; i < array_len; i++) {
        yajl_val cur_node = w[i];
        yajl_val cur_val;

        // Verification is done via the type argument to yajl_tree_get
        // and via YAJL_IS_* checks if type alone isn't sufficient.
        cur_val = yajl_tree_get(cur_node, id_path, yajl_t_string);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
            goto error;
        }
        changes[i].id = YAJL_GET_STRING(cur_val);
        
        // why is bool parsing weird
        cur_val = yajl_tree_get(cur_node, achieved_path, yajl_t_any);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
            goto error;
        }
        if (!YAJL_IS_TRUE(cur_val) && !YAJL_IS_FALSE(cur_val)) {
            std::cerr << "bool parsing error" << std::endl;
            goto error;
        }
        changes[i].achieved = YAJL_IS_TRUE(cur_val);
    }

    return changes;

error:
    changes.clear();
    return changes;
}

std::vector<StatChange_t> 
ProcessedGameServerRequest::get_stat_changes() const
{
    std::vector<StatChange_t> changes;

    const char * id_path[] = { STAT_ID_STR, (const char*)0 };
    const char * type_path[] = { STAT_TYPE_STR, (const char*)0 };
    const char * new_value_path[] = { STAT_VALUE_STR, (const char*)0 };
    
    if (m_stat_changes_array == NULL) {
        return changes;
    }

    yajl_val *w = YAJL_GET_ARRAY(m_stat_changes_array)->values;
    size_t array_len = YAJL_GET_ARRAY(m_stat_changes_array)->len;

    changes.resize(array_len);

    for(unsigned i = 0; i < array_len; i++) {
        yajl_val cur_node = w[i];
        yajl_val cur_val;

        cur_val = yajl_tree_get(cur_node, id_path, yajl_t_string);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
            goto error;
        }
        changes[i].id = YAJL_GET_STRING(cur_val);

        cur_val = yajl_tree_get(cur_node, type_path, yajl_t_number);
        if (cur_val == NULL) {
            std::cerr << "parsing error (stat type)" << std::endl;
            goto error;
        }
        changes[i].type = (UserStatType)YAJL_GET_INTEGER(cur_val);

        if (changes[i].type == UserStatType::Integer)
        {
            cur_val = yajl_tree_get(cur_node, new_value_path, yajl_t_number);
            if (cur_val == NULL) {
                std::cerr << "parsing error (stat value int)" << std::endl;
                goto error;
            }

            if (YAJL_GET_INTEGER(cur_val) != (int)YAJL_GET_INTEGER(cur_val)) {
                std::cerr << "loss of int precision" << std::endl;
                goto error;
            }

            changes[i].new_value = (int)(YAJL_GET_INTEGER(cur_val));
        }
        else if (changes[i].type == UserStatType::Float) {
            cur_val = yajl_tree_get(cur_node, new_value_path, yajl_t_number);
            if (cur_val == NULL) {
                std::cerr << "parsing error (stat value float)" << std::endl;
                goto error;
            }

            if (YAJL_GET_DOUBLE(cur_val) != (float)YAJL_GET_DOUBLE(cur_val)) {
                std::cerr << "loss of float precision" << std::endl;
                goto error;
            }

            changes[i].new_value = (float)YAJL_GET_DOUBLE(cur_val);
        }
        else {
            std::cerr << "Unable to get stat value: Unsupported type. " << changes[i].id << std::endl;
            goto error;
        }
    }

    return changes;

error:
    changes.clear();
    return changes;
}