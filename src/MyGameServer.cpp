#include "MyGameServer.h"
#include <iostream>

MyGameServer::MyGameServer(AppId_t appid) : m_appid(appid), m_socket(appid)
{

}

void
MyGameServer::run()
{
    setenv("SteamAppId", std::to_string(m_appid).c_str(), 1);

    if (!SteamAPI_Init()) {
        std::cerr << "An error occurred launching the Steam API. Aborting." << std::endl;
        std::cerr << "Make sure you are trying to run an app you own, and running with lauch.sh" << std::endl;
        exit(EXIT_FAILURE);
    }

    m_socket.run_server();
    SteamAPI_Shutdown();
}