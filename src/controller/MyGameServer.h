#pragma once
#include "../sockets/MyGameSocket.h"
#include "../../steam/steamtypes.h"

class MyGameServer
{
public:
    void run();
    MyGameServer(AppId_t appid);
private:
    AppId_t m_appid;
    MyGameSocket m_socket;
};