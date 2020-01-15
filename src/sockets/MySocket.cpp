#include "MySocket.h"

#include "../globals.h"
#include "../common/functions.h"
#include "../controller/MySteam.h"

#include <iostream>

MySocket::MySocket(AppId_t appid) : m_appid(appid), m_socket_fd(-1)
{ 
    m_socket_path = g_steam->get_runtime_path() + "/" + std::to_string(m_appid) + "-ipc.sock";
}

MySocket::~MySocket()
{
    // Both server and client must unlink the socket file for it
    // to be deleted on the filesystem.
    close(m_socket_fd);
    unlink(m_socket_path.c_str());
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
    char buffer[BUFFER_SIZE+1];
    /* Ensure buffer is 0-terminated. */
    buffer[BUFFER_SIZE] = '\0';

    for (;;) {
        /* Wait for next data packet. */
        read_count(fd, buffer, BUFFER_SIZE);
        //std::cerr << " received packet: " << buffer << std::endl;
        ret += std::string(buffer);

        if (strlen(buffer) < BUFFER_SIZE) {
            // Got a NULL in the actual buffer, so must be the end
            // of the string
            break;
        }
    }

    //std::cerr << "PID: " << getpid() << " received message: " << ret << std::endl;
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
    char buffer[BUFFER_SIZE+1]; //+1 for printing chunks
    buffer[BUFFER_SIZE] = '\0';

    unsigned iterator = 0;
    //std::cerr << "PID: " << getpid() << " sending message: " << message << std::endl;

    for (;;) {
        strncpy(buffer, message.substr(iterator, BUFFER_SIZE).c_str(), BUFFER_SIZE);
        //std::cerr << " sending packet: " << buffer << std::endl;
        write_count(fd, buffer, BUFFER_SIZE);

        iterator += BUFFER_SIZE;
        if (iterator > message.size()) {
            break;
        }
    }
}