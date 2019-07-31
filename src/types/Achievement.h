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