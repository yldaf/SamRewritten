#include "AchievementsDAO.h"

CAchievementsDAO::CAchievementsDAO() 
    : m_has_achievements(false)
{

}

/**
 * This method retrieves all the achievements ID, and stores everything in m_all_bin_data
 * TO BE UPGRADED FOR STATS TODO
 */
bool 
CAchievementsDAO::load_binary_file(std::string& file_path)
{
    m_all_bin_data.clear();

    if(!file_exists(file_path))
        return false;

    try {
        std::ifstream input;
        input.open(file_path, std::ios::binary);
    
        if (input) 
        {
            // get length of file:
            input.seekg (0, input.end);
            int length = input.tellg();
            input.seekg (0, input.beg);

            // allocate memory:
            char * buffer = new char [length];
            char * current;

            // read data as a block:
            input.read (buffer, length);
            input.close();

            unsigned string_beginning(0);
            short unsigned int ascii_code;
            char curr_str[256];

            for (unsigned i = 0; i < length; i++) {
                current = (buffer + i);
                ascii_code = (short unsigned int)*current;

                //printf("* %d - %c - #%d\n" , ascii_code, *current, i);
                //Usually, < 10 is garbage or delimiter. To inspect, need for documentation.
                if(ascii_code < 10) {
                    //If this is not an empty word
                    if(i - string_beginning > 0){
                        strncpy(curr_str, buffer + string_beginning, i - string_beginning);
                        m_all_bin_data.push_back(std::string(curr_str));
                        memset(curr_str, 0, sizeof(curr_str));
                    } else {
                        //std::cout << "No string" << std::endl;
                    }
                    string_beginning = i + 1;
                }
            }

            delete[] buffer;
        }
        return true;
    }
    catch (int code)
    {
        puts("Error opening or parsing binary file.\n");
    }
    return false;    
}

std::vector<std::string> 
CAchievementsDAO::get_all_achievements_ids_from_bin() {
    bool name_found_once = false;
    bool found_delimiter = true; //First has no delimiter
    bool ignore_next = false;

    std::vector<std::string> ach_ids;

    for(std::string &s : m_all_bin_data) {

        if(ignore_next) {
            if(s == "bits") {
                ignore_next = false;
            }
            continue;
        }
        if(s == "bit"){
            found_delimiter = true;
            continue;
        } 
        if (s == "type_int") {
            //Sometimes, there are some type_int in the middle of files for no reason. Fortunately, when relevant data
            //comes back, it is preceded by the word "bits"
            ignore_next = true;
            continue;
        }
        if(name_found_once) {
            ach_ids.push_back(s);
            name_found_once = false;
            found_delimiter = false;
            continue;
        }
        if(found_delimiter && s == "name") {
            name_found_once = true;
        }
    }

    const unsigned total_achievements = SteamUserStats()->GetNumAchievements();
    if(ach_ids.size() != total_achievements) {
        std::cerr << "Using the bin file, SAM could only retrieve " << ach_ids.size() << " achievements out of " << total_achievements << "." << std::endl;
        ach_ids.clear();
    }

    return ach_ids;
}

/**
 * If we fail to retrieve all achivements, returns an empty vector
 */
std::vector<std::string> 
CAchievementsDAO::get_all_achievements_ids_from_api() {
    ISteamUserStats *stats_api = SteamUserStats();
    const unsigned num_ach = stats_api->GetNumAchievements();
    std::vector<std::string> all_ids;

    for (unsigned i = 0; i < num_ach ; i++) {
        all_ids.push_back(std::string(stats_api->GetAchievementName(i)));
    }

    if(all_ids.size() != num_ach) {
        std::cerr << "Using the API, SAM could only retrieve " << all_ids.size() << " achievements out of " << num_ach << "." << std::endl;
        all_ids.clear();
        return all_ids;
    }

    return all_ids;
}

/**
 * Returns all achievements IDs for the appid, set in main.
 * If any error occurs, all is aborted and an error must be shown.
 * The return value in this case is an empty array.
 */
std::vector<std::string> 
CAchievementsDAO::get_all_achievements_ids() {
    std::vector<std::string> all_ids;

    all_ids = get_all_achievements_ids_from_api();

    if (all_ids.empty()) {
        all_ids = get_all_achievements_ids_from_bin();
    }

    if(SteamUserStats()->GetNumAchievements() == 0) {
        std::cerr << "It looks like this game has no achievements." << std::endl;
        m_has_achievements = false;
    }
    else {
        m_has_achievements = true;
    }

    return all_ids;
}

CAchievementsDAO::~CAchievementsDAO()
{

}