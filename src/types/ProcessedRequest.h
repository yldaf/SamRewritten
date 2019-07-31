#pragma once

#include <string>
#include "Actions.h"

struct ProcessedRequest {
    SAM_ACTION action;
    std::string payload;
};

typedef struct ProcessedRequest ProcessedRequest;