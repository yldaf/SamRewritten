#include "cli_funcs.h"
#include "../controller/MySteam.h"
#include "../globals.h"
#include "../common/cxxopts.hpp"

#include <string>
#include <iostream>
#include <csignal>
#include <unistd.h>

void handle_sigint_cli(int signum) 
{
	std::cout << "Quitting cli idling" << std::endl;
	g_steam->quit_game();
	exit(EXIT_SUCCESS);
}

void idle_app(AppId_t appid) 
{
	std::cout << "Idling from command line " << appid << std::endl;
	g_steam->launch_app(appid);
	signal(SIGINT, handle_sigint_cli);

	// Wait for ctrl+c, so we can kill both processes, otherwise
	// GUI process will exit while game process still goes on
	for(;;) {
		sleep(10000);
	}
}

bool compareByUnlockRateDesc(const Achievement_t &a, const Achievement_t &b)
{
	return a.global_achieved_rate > b.global_achieved_rate;
}

bool go_cli_mode(int argc, char* argv[]) {
	cxxopts::Options options(argv[0], "Steam Achievements Manager");
    
	options
      .positional_help("[AppId]")
      .show_positional_help()
	  .add_options()
		("apps", "Get the list of your owned apps.")
		("h,help", "Show CLI help.")
		("a,app", "Set which AppId you want to use. Same as using positional 'AppId'", cxxopts::value<AppId_t>())
		("i,idle", "Set your Steam profile as 'ingame'. Ctrl+c to stop.")
		("ls", "Display stats (TODO) and achievements for selected app.")
		("sort", "Sort option for --ls. You can leave empty or set to 'unlock_rate'", cxxopts::value<std::string>())
		("unlock", "Unlock achievements for an AppId. Separate achievement names by a comma.", cxxopts::value<std::vector<std::string>>())
		("lock", "Lock achievements for an AppId. Separate achievement names by a comma.", cxxopts::value<std::vector<std::string>>())
		("launch", "Actually just launch the app.");


	options.parse_positional({"app"});

	auto result = options.parse(argc, argv);
	bool cli = false;
	AppId_t app = 0;

	if (result.count("help"))
    {
		std::cout << options.help() << std::endl;
		return true;
    }
	
	if (result.count("app") > 0)
	{
		cli = true;
		app = result["app"].as<AppId_t>();
	}
	
	if (result.count("apps") > 0)
	{
		cli = true;

		g_steam->refresh_owned_apps();
		auto apps = g_steam->get_subscribed_apps();

		std::cout << "AppId \t App Name" << std::endl;
		for ( Game_t& it : apps )
		{
			std::cout << it.app_id << "\t " << it.app_name << std::endl;
		}
	}

	if (result.count("idle") > 0)
	{
		cli = true;
		if (app == 0)
		{
			std::cout << "Please provide an AppId argument before idling." << std::endl;
			return cli;
		}
		
		idle_app(app);
	}

	if (result.count("ls") > 0) {
		cli = true;
		if (app == 0)
		{
			std::cout << "Please provide an AppId argument before listing stats." << std::endl;
			return cli;
		}

		g_steam->launch_app(app);
		g_steam->refresh_achievements();
		auto achievements = g_steam->get_achievements();
		g_steam->quit_game();
		
		if (result.count("sort") > 0)
		{
			const std::string sort = result["sort"].as<std::string>();

			if (sort == "unlock_rate")
			{
				std::sort(achievements.begin(), achievements.end(), compareByUnlockRateDesc);
			}
		}
		

		// https://github.com/haarcuba/cpp-text-table -> worth? nah but best I've found
		std::cout << "Sorted by global unlock rate\n" << std::endl;
		std::cout << "API Name \t\tName \t\tDescription \t\tUnlock rate \t\tUnlocked" << std::endl;
		std::cout << "--------------------------------------------------------------" << std::endl;
		for ( Achievement_t& it : achievements )
		{
			std::cout 
				<< it.id << " \t" 
				<< it.name << " \t" 
				<< it.desc << " \t" 
				<< it.global_achieved_rate << "% \t"
				<< (it.achieved ? "✔️" : "❌") << std::endl;
		}
	}

	if (result.count("unlock") > 0)
	{
		if (app == 0)
		{
			std::cout << "Please provide an AppId argument before unlocking achievements." << std::endl;
			return true;
		}

		cli = true;
		const std::vector<std::string> ids = result["unlock"].as<std::vector<std::string>>();

		for ( std::string it : ids )
		{
			g_steam->add_modification_ach(it, true);
		}

		g_steam->launch_app(app);
		g_steam->commit_changes();
		g_steam->quit_game();
	}

	if (result.count("lock") > 0)
	{
		if (app == 0)
		{
			std::cout << "Please provide an AppId argument before relocking achievements." << std::endl;
			return true;
		}

		cli = true;
		const std::vector<std::string> ids = result["lock"].as<std::vector<std::string>>();

		for ( std::string it : ids )
		{
			g_steam->add_modification_ach(it, false);
		}

		g_steam->launch_app(app);
		g_steam->commit_changes();
		g_steam->quit_game();
	}

	if (result.count("launch") > 0)
	{
		if (app == 0)
		{
			std::cout << "Please provide an AppId argument before launching the app." << std::endl;
			return true;
		}

		std::cout << "Launching app with Steam. Make sure you have xdg-open installed." << std::endl;
		system(("xdg-open steam://run/" + std::to_string(app)).c_str());
	}

	return cli;
}
