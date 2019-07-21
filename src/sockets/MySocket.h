#pragma once
#include "../../steam/steam_api.h"
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUFFER_SIZE 12

// Strings are the fundamental unit of transmission,
// so just use NULL terminator as end of message

class MySocket
{
protected:
    AppId_t m_appid;
    std::string m_socket_path;
    int m_socket_fd;
public:
    void send_message(const std::string message);
    void send_message(const int fd, const std::string message);
    std::string receive_message();
    std::string receive_message(const int fd);
    MySocket(AppId_t appid);
    ~MySocket();
};