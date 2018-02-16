/**
 * The purpose of this program is to manage available achievements.
 * This program is not meant to have a GUI.  It's a background task of SAM.Picker
 * This program runs until a SIGTERM is sent to it
 */

#include <thread>
#include <chrono>
#include <csignal>
#include <iostream>
#include <unistd.h>
#include <exception>
#include <sys/stat.h>
#include "../steam/steam_api.h"
#include "StatsAndAchievements.h"

void handle_sigterm(int signum);
void terminate_func();

int main(int argc, char * argv[]) {
    //Handle signals and check the arguments
    signal(SIGTERM, handle_sigterm);
    signal(SIGINT, handle_sigterm);
    std::set_terminate(terminate_func);
    if(argc != 2) {
        std::cout << "Incorrect number of arguments. Expected one, received " << argc << std::endl;
        return EXIT_FAILURE;
    }

    //Create the cache folder
    const std::string cache_dir = "/tmp/SamRewritten/" + std::string(argv[1]) + "/";
    mkdir("/tmp/SamRewritten", S_IRWXU | S_IRWXG | S_IROTH);
	const int mkdir_error = mkdir(cache_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
    if(mkdir_error != 0 && errno != EEXIST) {
		std::cerr << "Unable to create the cache folder (" << cache_dir << ", errno " << errno << ")." << std::endl;
        return EXIT_FAILURE;
	}

    //Start the steam API
    setenv("SteamAppId", argv[1], 1);
    if(!SteamAPI_Init()) {
        std::cout << "Could not emulate the game, Steam API encountered issues." << std::endl;
        return EXIT_FAILURE;
    }

    //Create the Achievement manager
    CStatsAndAchievements *manager = new CStatsAndAchievements();

    // Default location for schemas is /home/<user>/.local/share/Steam/appcache/stats/UserGameStatsSchema_<appId>.bin
    // The result can be observed in the callback function, OnUserStatsReceived
    manager->AskForSchema();

    //This is the game loop. It stops when a SIGTERM is sent to this program.
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        SteamAPI_RunCallbacks();
    }

    //The only way for this program to stop is to receive a SIGTERM
    //There's no way exitting here is a success
    return EXIT_FAILURE;
}

void handle_sigterm(int signum) {
    switch(signum) {
        case 2:
            std::cout << std::endl << "SIGINT received. Have a wonderful day!" << std::endl;
            break;

        //Other signals aren't supposed to be made by users, so there's no need to output anything.
        
    }

    SteamAPI_Shutdown();
    exit(EXIT_SUCCESS);
}

void terminate_func() {
    puts("An error occurred and the program could not run correctly. Please report any bugs on the github repository of SamRewritten. I hope you will still have a good day!\n");
}