#pragma once
#include "../sockets/MyClientSocket.h"

// This class handles:
// - Creation of GameServer which acts like a Steam game
// - Creation of a client socket which the main window uses to
//   communicate with that server

class GameServerManager
{
private:
    //std::vector<MyGameServer*> m_servers;
public:
    MyClientSocket* quick_server_create(AppId_t appid);
};