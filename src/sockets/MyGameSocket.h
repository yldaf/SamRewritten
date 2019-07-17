#pragma once
#include "MyServerSocket.h"

class MyGameSocket : public MyServerSocket
{
public:
    std::string process_request(std::string request);
    using MyServerSocket::MyServerSocket;
};

