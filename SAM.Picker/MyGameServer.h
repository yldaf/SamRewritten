#pragma once
#include "MyServerSocket.h"

class MyGameServer
{
private:
    MyServerSocket* m_socket;
public:
    void run();
    MyGameServer(AppId_t appid);
    ~MyGameServer();
};

MyGameServer::MyGameServer(AppId_t appid)
{
    m_socket = new MyServerSocket(appid);
}

MyGameServer::~MyGameServer()
{
    delete m_socket;
}

void
MyGameServer::run()
{
    //setEnv
    //SteamAPI_Init();
    m_socket->run_server();
}