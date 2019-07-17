#pragma once
#include "MySocket.h"
#include <string>

class MyClientSocket : public MySocket
{
private:
    void connect_to_server();
    void disconnect();
    struct sockaddr_un m_address;
public:
    std::string request_response(std::string request);
    void kill_server();
    MyClientSocket(AppId_t appid);
};