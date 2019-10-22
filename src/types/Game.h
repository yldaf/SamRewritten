#pragma once

#include <string>
#include "../../steam/steamtypes.h"

struct Game_t 
{
    AppId_t app_id;
    unsigned long number_achievements;
    std::string app_name;
    //number of achievements?
    //completion rate?
};