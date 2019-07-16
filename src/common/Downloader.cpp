#include "Downloader.h"
#include "functions.h"
#include <curl/curl.h>
#include <future>
#include <iostream>

Downloader*
Downloader::get_instance() {
    static Downloader me;
    return &me;
}

void 
Downloader::download_file(const std::string& file_url, const std::string& local_path, const unsigned long& dl_id) {
    //If the file exists, there's no need to download it again.
    //We assume the banners don't change
    if(!file_exists(local_path)) {        
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
            std::cerr << "An error occurred creating curl. Please report it to the developers!" << std::endl;
            exit(EXIT_FAILURE);
        }

        if(res != 0) {
            std::cerr << "Curl errored with status " << res << ". (file: " <<  file_url << ")" << std::endl;
            std::cout << "An error occurred while fetching an icon, are you connected to the internet?" << std::endl;

            //exit(EXIT_FAILURE);
        }
    }

    notify(dl_id);
}

void 
Downloader::download_file_async(const std::string& file_url, const std::string& local_path, const unsigned long& dl_id) {
    // Observers are already notified via the notify call in download_file.
    // So silence the return value.
    (void)std::async([this, file_url, local_path, dl_id]{this->download_file(file_url, local_path, dl_id);});
}