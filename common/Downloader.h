#ifndef DOWNLOADER
#define DOWNLOADER

#pragma once
#include "ObserverClasses.h"
#include "c_processes.h"
#include <curl/curl.h>
#include <string>
#include <future>

//TODO comment & tidy

class Downloader : public Subject<unsigned long> {
public:
    static Downloader* get_instance() {
        static Downloader me;
        return &me;
    };
    void download_file(const std::string& file_url, const std::string& local_path, const unsigned long& dl_id);
    void download_file_async(const std::string& file_url, const std::string& local_path, const unsigned long& dl_id);

    Downloader(Downloader const&)                 = delete;
    void operator=(Downloader const&)             = delete;
private:
    Downloader() {};
    ~Downloader() {};
};

#endif