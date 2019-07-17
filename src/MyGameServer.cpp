#include "MyGameServer.h"

MyGameServer::MyGameServer(AppId_t appid) : m_appid(appid), m_socket(appid)
{

}

void
MyGameServer::run()
{
    setenv("SteamAppId", std::to_string(m_appid).c_str(), 1);
    SteamAPI_Init();
    m_socket.run_server();
}