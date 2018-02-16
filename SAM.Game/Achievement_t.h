#pragma once

#include <string>

struct Achievement_t {
	std::string achievementID; //I fucked up and used camelCase. To fix later
	std::string achievementName;
	std::string achievementDesc;
    float global_achieved_rate;  
    int icon_handle; //0 : incorrect, error occurred, RTFM
	bool achieved;
    bool hidden;
    
    //bool has_stats
    //unsigned stat_value
    //unsigned stat_min
    //unsigned stat_max
};