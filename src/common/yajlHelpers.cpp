
#include "yajlHelpers.h"

#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>
#include "../types/Achievement.h"
#include "../types/Actions.h"

void yajl_gen_string_wrap(yajl_gen handle, const char * a) {
    if (yajl_gen_string(handle, (const unsigned char *)a, strlen(a)) != yajl_gen_status_ok) {
        std::cerr << "failed to make json" << std::endl;
    }
}

void encode_request(yajl_gen handle, const char * request) {
    if (yajl_gen_map_open(handle) != yajl_gen_status_ok) {
        std::cerr << "failed to make json" << std::endl;
    }

    yajl_gen_string_wrap(handle, SAM_ACTION_STR);
    yajl_gen_string_wrap(handle, request);

    if (yajl_gen_map_close(handle) != yajl_gen_status_ok) {
        std::cerr << "failed to make json" << std::endl;
    }
}

std::string decode_request(std::string request) {

    yajl_val node = yajl_tree_parse(request.c_str(), NULL, 0);

    if (node == NULL) {
        std::cerr << "Parsing error";
        exit(EXIT_FAILURE);
    }

    const char * path[] = { SAM_ACTION_STR, (const char*)0 };
    yajl_val v = yajl_tree_get(node, path, yajl_t_string);
    if (v == NULL || !YAJL_IS_STRING(v)) {
        std::cerr << "failed to get" << SAM_ACTION_STR << std::endl;
        exit(EXIT_FAILURE);
    }

    return std::string (YAJL_GET_STRING(v));
}

/**
 * Encode an individual achievement into a given YAJL handle
 */
void encode_achievement(yajl_gen handle, Achievement_t achievement) {

    yajl_gen_string_wrap(handle, NAME_STR);
    yajl_gen_string_wrap(handle, achievement.name.c_str());

    yajl_gen_string_wrap(handle, DESC_STR);
    yajl_gen_string_wrap(handle, achievement.desc.c_str());

    yajl_gen_string_wrap(handle, ID_STR);
    yajl_gen_string_wrap(handle, achievement.id.c_str());

    yajl_gen_string_wrap(handle, RATE_STR);
    if (yajl_gen_double(handle, (double)achievement.global_achieved_rate) != yajl_gen_status_ok) {
            std::cerr << "failed to make json" << std::endl;
    }

    yajl_gen_string_wrap(handle, ICON_STR);
    if (yajl_gen_integer(handle, achievement.icon_handle) != yajl_gen_status_ok) {
            std::cerr << "failed to make json" << std::endl;
    }

    yajl_gen_string_wrap(handle, ACHIEVED_STR);
    if (yajl_gen_bool(handle, achievement.achieved) != yajl_gen_status_ok) {
        std::cerr << "failed to make json" << std::endl;
    }

    yajl_gen_string_wrap(handle, HIDDEN_STR);
    if (yajl_gen_bool(handle, achievement.hidden) != yajl_gen_status_ok) {
        std::cerr << "failed to make json" << std::endl;
    }
}

/**
 * Encode an achievement vector into a given YAJL handle
 */
void encode_achievements(yajl_gen handle, std::vector<Achievement_t> achievements) {

    yajl_gen_string_wrap(handle, ACHIEVEMENT_LIST_STR);

    if (yajl_gen_array_open(handle) != yajl_gen_status_ok) {
        std::cerr << "failed to make json" << std::endl;
    }

    for (Achievement_t achievement : achievements) {
        std::cout << "encoding achievement.id " << achievement.id << std::endl;

        if (yajl_gen_map_open(handle) != yajl_gen_status_ok) {
            std::cerr << "failed to make json" << std::endl;
        }

        encode_achievement(handle, achievement);

        if (yajl_gen_map_close(handle) != yajl_gen_status_ok) {
            std::cerr << "failed to make json" << std::endl;
        }            
    }

    if (yajl_gen_array_close(handle) != yajl_gen_status_ok) {
        std::cerr << "failed to make json" << std::endl;
    }

}

//parsing the array inline would not be nice, so just extract them all here

std::vector<Achievement_t> decode_achievements(std::string response) {
    std::vector<Achievement_t> achievements;

    //parse response
    yajl_val node = yajl_tree_parse(response.c_str(), NULL, 0);

    if (node == NULL) {
        std::cerr << "parsing error";
        exit(EXIT_FAILURE);
    }

    // TODO: separate out into decode_ack()
    const char * ack_path[] = { SAM_ACK_STR, (const char*)0 };
    yajl_val v = yajl_tree_get(node, ack_path, yajl_t_string);
    if (v == NULL || !YAJL_IS_STRING(v)) {
        std::cerr << "failed to parse " << SAM_ACK_STR << std::endl;
    }

    if (std::string(YAJL_GET_STRING(v)) != std::string(SAM_ACK_STR)) {
        std::cerr << "failed to receive ack" << std::endl;
    }

    // dumb defines for required interface for yajl_tree
    const char * list_path[] = { ACHIEVEMENT_LIST_STR, (const char*)0 };
    const char * name_path[] = { NAME_STR, (const char*)0 };
    const char * desc_path[] = { DESC_STR, (const char*)0 };
    const char * id_path[] = { ID_STR, (const char*)0 };
    const char * rate_path[] = { RATE_STR, (const char*)0 };
    const char * icon_path[] = { ICON_STR, (const char*)0 };
    const char * achieved_path[] = { ACHIEVED_STR, (const char*)0 };
    const char * hidden_path[] = { HIDDEN_STR, (const char*)0 };
    
    v = yajl_tree_get(node, list_path, yajl_t_array);
    if (v == NULL) {
        std::cerr << "parsing error" << std::endl;
    }

    yajl_val *w = YAJL_GET_ARRAY(v)->values;
    size_t array_len = YAJL_GET_ARRAY(v)->len;

    achievements.clear();
    achievements.resize(array_len);

    for(unsigned i = 0; i < array_len; i++) {
        yajl_val cur_node = w[i];
        yajl_val cur_val;

        // verification is done via the type argument to yajl_tree_get
        // and via YAJL_IS_* checks if type alone isn't sufficient
        cur_val = yajl_tree_get(cur_node, name_path, yajl_t_string);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
        }
        achievements[i].name = YAJL_GET_STRING(cur_val);

        cur_val = yajl_tree_get(cur_node, desc_path, yajl_t_string);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
        }
        achievements[i].desc = YAJL_GET_STRING(cur_val);
        
        cur_val = yajl_tree_get(cur_node, id_path, yajl_t_string);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
        }
        achievements[i].id = YAJL_GET_STRING(cur_val);
        
        cur_val = yajl_tree_get(cur_node, rate_path, yajl_t_number);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
        }
        if (!YAJL_IS_DOUBLE(cur_val)) {
            std::cerr << "double float parsing error" << std::endl;
        }
        achievements[i].global_achieved_rate = YAJL_GET_DOUBLE(cur_val);
        
        cur_val = yajl_tree_get(cur_node, icon_path, yajl_t_number);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
        }
        if (!YAJL_IS_INTEGER(cur_val)) {
            std::cerr << "integer parsing error" << std::endl;
        }
        achievements[i].icon_handle = YAJL_GET_INTEGER(cur_val);
        
        // why is bool parsing weird
        cur_val = yajl_tree_get(cur_node, achieved_path, yajl_t_any);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
        }
        if (!YAJL_IS_TRUE(cur_val) && !YAJL_IS_FALSE(cur_val)) {
            std::cerr << "bool parsing error" << std::endl;
        }
        achievements[i].achieved = YAJL_IS_TRUE(cur_val);

        cur_val = yajl_tree_get(cur_node, hidden_path, yajl_t_any);
        if (cur_val == NULL) {
            std::cerr << "parsing error" << std::endl;
        }
        if (!YAJL_IS_TRUE(cur_val) && !YAJL_IS_FALSE(cur_val)) {
            std::cerr << "bool parsing error" << std::endl;
        }
        achievements[i].achieved = YAJL_IS_TRUE(cur_val);
    }

    yajl_tree_free(node);
    return achievements;
}
