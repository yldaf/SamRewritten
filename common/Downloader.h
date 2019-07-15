#pragma once
#include <string>
#include "ObserverClasses.h"

/**
 * This class is used to download files from the internet
 * It follows a single singleton pattern and is meant to be very basic
 * It inherits Subject, and will notify observers only on a successful
 * download.
 */
class Downloader : public Subject<unsigned long> {
public:
    /**
     * Singleton method to get the unique instance
     */
    static Downloader* get_instance();

    /**
     * Downloads a file synchronously. If the download fails, the program will exit.
     * If the local_path already exists on the disk, the download will be skipped (as it if was successful)
     * The download ID is something that will be passed to every observer once the download 
     * completed successfully.
     */
    void download_file(const std::string& file_url, const std::string& local_path, const unsigned long& dl_id);

    /**
     * Does the same thing than download_file, but asynchronously.
     */
    void download_file_async(const std::string& file_url, const std::string& local_path, const unsigned long& dl_id);

    /**
     * Delete these, as no one will need them with the singleton pattern
     */
    Downloader(Downloader const&)                 = delete;
    void operator=(Downloader const&)             = delete;

private:
    Downloader() {};
    ~Downloader() {};
};