#include "MyGameServer.h"
#include "../common/functions.h"
#include <iostream>

MyGameServer::MyGameServer(AppId_t appid)
: m_appid(appid), 
  m_socket(appid)
{

}

void
MyGameServer::run()
{
    setenv("SteamAppId", std::to_string(m_appid).c_str(), 1);

    if (!SteamAPI_Init()) {
        std::cerr << "An error occurred launching the Steam API. Aborting." << std::endl;
        std::cerr << "Make sure you are trying to run an app you own." << std::endl;
        zenity("Make sure you are trying to run an app you own.");
        exit(EXIT_FAILURE);
    }

    m_socket.run_server();
    // Make sure to destruct this to make sure we unlink files nicely during
    // destruction rather than when the child process is being terminated
    m_socket.~MyGameSocket();
    SteamAPI_Shutdown();
}