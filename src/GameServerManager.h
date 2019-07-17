#pragma once
#include "sockets/MyClientSocket.h"

class GameServerManager
{
private:
    //std::vector<MyGameServer*> m_servers;
public:
    MyClientSocket* quick_server_create(AppId_t appid);
};