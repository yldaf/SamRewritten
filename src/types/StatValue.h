#pragma once

#include <string>
#include <any>

#include "UserStatType.h"

struct StatValue_t {
    UserStatType type;
    std::string id;
    std::string display_name;
    std::any value;
    std::any original_value;
    bool incrementonly;
    int permission;
};

typedef struct StatValue_t StatValue_t;
