#pragma once

#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cstring>
#include <iostream>
#include "../common/c_processes.h"
#include "../steam/steam_api.h"

class CAchievementsDAO 
{
public:
    CAchievementsDAO();
    ~CAchievementsDAO();
    bool load_binary_file(std::string& file_path);
    std::vector<std::string> get_all_achievements_ids();
    bool app_has_achievements() const { return m_has_achievements; };

protected:
    std::vector<std::string> get_all_achievements_ids_from_bin();
    std::vector<std::string> get_all_achievements_ids_from_api();
    
    std::vector<std::string> m_all_bin_data;
    bool m_has_achievements;
};