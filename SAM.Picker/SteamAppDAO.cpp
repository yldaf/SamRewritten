#include "SteamAppDAO.h"

void 
SteamAppDAO::update_name_database() {
    bool need_to_redownload = false;
    struct stat file_info;
    static const char local_file_name[] = "/tmp/SamRewritten/app_names";
    const std::time_t current_time(std::time(0));

	// Make sure the cache folder is there
    const int mkdir_error = mkdir("/tmp/SamRewritten", S_IRWXU | S_IRWXG | S_IROTH);
    if(mkdir_error != 0 && errno != EEXIST) {
		std::cerr << "Unable to create the cache folder ( /tmp/SamRewritten, errno " << errno << ")." << std::endl;
        exit(EXIT_FAILURE);
	}

    // Check if the file is already there
    if (file_exists(local_file_name)) {
        //Check the last time it was updated
        if(stat(local_file_name, &file_info) == 0) {
            //If a week has passed
            if(current_time - file_info.st_mtime > 60 * 60 * 24 * 7) {
                need_to_redownload = true;
            }
        }
        else {
            std::cerr << "/tmp/SamRewritten/app_names exists but an error occurred analyzing it. To avoid further complications, ";
            std::cerr << "the program will stop here. Before retrying make sure you have enough privilege to read and write to ";
            std::cerr << "the /tmp folder." << std::endl;

            exit(EXIT_FAILURE);
        }
    }
    else {
        need_to_redownload = true;
    }

    if(need_to_redownload) {
        SteamAppDAO::download_file("http://api.steampowered.com/ISteamApps/GetAppList/v0002/", local_file_name);
    }
}

std::string 
SteamAppDAO::get_app_name(const unsigned long& app_id) {
    return "";
}


void 
SteamAppDAO::download_app_icon(const unsigned long& app_id) {

}

void 
SteamAppDAO::download_file(const std::string& file_url, const std::string& local_path) {
    std::cerr << "Downloading " << std::endl;
    
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(local_path.c_str(),"wb");
        curl_easy_setopt(curl, CURLOPT_URL, file_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    else {
        std::cerr << "An error occurred creating curl. Please report to the developers!" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(res != 0) {
        std::cerr << "Curl returned with status " << res << ", which is unexpected. Make sure you are connected to the internet, and you have access";
        std::cerr << " to Steam, and try again." << std::endl;
        //TODO make a gui for this

        exit(EXIT_FAILURE);
    }
}

void 
SteamAppDAO::parse_app_names() {

}