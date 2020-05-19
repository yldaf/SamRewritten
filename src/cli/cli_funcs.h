#pragma once

#include "../../steam/steamtypes.h"

/**
 * Process command-line arguments. If there are some, returns true
 */
bool go_cli_mode(int argc, char* argv[], AppId_t* return_app_id);
