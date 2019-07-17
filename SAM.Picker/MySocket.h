#pragma once
#include "globals.h"
#include "../steam/steam_api.h"
#include "../common/functions.h"
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <iostream>
#include <unistd.h>

#define BUFFER_SIZE 12
#define END_OF_MESSAGE "SAM_STOP"
#define END_OF_SERVICE "SAM_QUIT"

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

MySocket::MySocket(AppId_t appid) : m_appid(appid), m_socket_fd(-1)
{
    m_socket_path = std::string(g_cache_folder) + "/" + std::to_string(m_appid) + "/ipc.sock";
}

MySocket::~MySocket()
{
    close(m_socket_fd);
}

std::string
MySocket::receive_message()
{
    return receive_message(m_socket_fd);
}

std::string
MySocket::receive_message(const int fd)
{
    std::string ret("");
    char buffer[BUFFER_SIZE];
    for (;;) {

        /* Wait for next data packet. */
        if (read(fd, buffer, BUFFER_SIZE) == -1) {
            std::cerr << "Socket could not read." << std::endl;
            exit(EXIT_FAILURE);
        }

        /* Ensure buffer is 0-terminated. */
        buffer[BUFFER_SIZE - 1] = 0;
        
        if (!strncmp(buffer, END_OF_MESSAGE, BUFFER_SIZE)) {
            break;
        }

        ret += std::string(buffer);
    }

    return ret;
}

void
MySocket::send_message(const std::string message)
{
    MySocket::send_message(m_socket_fd, message);
}

void
MySocket::send_message(const int fd, const std::string message)
{
    char buffer[BUFFER_SIZE];
    unsigned iterator = 0;

    for (;;) {
        strncpy(buffer, message.substr(iterator, BUFFER_SIZE - 1).c_str(), BUFFER_SIZE - 1);

        std::cerr << "sending packet: " << buffer << std::endl;

        if (write(fd, buffer, strlen(buffer) + 1) == -1) {
            std::cerr << "There was an error writing the message." << std::endl;
            exit(EXIT_FAILURE);
        }

        iterator += BUFFER_SIZE - 1;

        if (iterator > message.size())
        {
            // Close the request
            strcpy(buffer, END_OF_MESSAGE);
            if (write(fd, buffer, strlen(buffer) + 1) == -1) {
                std::cerr << "There was an error closing the message." << std::endl;
                exit(EXIT_FAILURE);
            } 
            break;
        }
    }
}