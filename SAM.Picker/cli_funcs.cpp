#include "cli_funcs.h"
#include "MySteam.h"
#include <string>
#include <iostream>
#include <csignal>
#include <unistd.h>

void handle_sigint_cli(int signum) {
	std::cout << "Quitting cli idling" << std::endl;
	g_steam->quit_game();
	exit(0);
}

void idle_app(std::string appid) {
	std::cout << "Idling from command line " << appid << std::endl;
	g_steam->launch_game(appid);
	signal(SIGINT, handle_sigint_cli);
}

bool go_cli_mode(int argc, char* argv[]) {
	bool cli = false;
	int opt;
    while((opt = getopt(argc, argv, ":a:")) != -1)
    {
        switch(opt)
        {
            case 'a':
				const std::string appid = std::string(optarg);
				idle_app(appid);
                cli = true;
                break;
        }
    }

    // If cli, wait for ctrl+c, so we can kill both processes, otherwise
    // GUI process will exit while game process still goes on
    if ( cli ) {
		for(;;) {
			sleep(10000);
		}
	}

	return cli;
}
