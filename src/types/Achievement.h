#pragma once

#include <string>

/**
 * Achievement structure.
 * Upgraded to use C++ types
 */
struct Achievement_t {
    std::string name;
    std::string desc;
    std::string id;
    float global_achieved_rate;  
    int icon_handle; //0 : incorrect, error occurred, RTFM
	bool achieved;
    bool hidden;
};

typedef struct Achievement_t Achievement_t;

/**
 * AchievementChange structure
 * Minimum information needed to change an
 * achievement
 * TODO add stats
 */
struct AchievementChange_t {
    std::string id;
	bool achieved;
};

typedef struct AchievementChange_t AchievementChange_t;