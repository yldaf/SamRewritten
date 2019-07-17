#pragma once
#include "MySocket.h"

class MyServerSocket : public MySocket
{
protected:
    void unlink_file();
public:
    void run_server();
    virtual std::string process_request(std::string request) = 0;
    MyServerSocket(AppId_t appid);
    ~MyServerSocket();
};