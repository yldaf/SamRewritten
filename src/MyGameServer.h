#pragma once
#include "sockets/MyGameSocket.h"
#include "../steam/steam_api.h"

class MyGameServer
{
private:
    AppId_t m_appid;
    MyGameSocket m_socket;
public:
    void run();
    MyGameServer(AppId_t appid);
};