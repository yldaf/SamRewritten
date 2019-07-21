#include "MyClientSocket.h"
#include "../types/Actions.h"

#include <thread>
#include <chrono>
#include <iostream>

MyClientSocket::MyClientSocket(AppId_t appid) : MySocket(appid)
{
    /*
    * For portability clear the whole structure, since some
    * implementations have additional (nonstandard) fields in
    * the structure.
    */
    memset(&m_address, 0, sizeof(struct sockaddr_un));

    /* Connect socket to socket address */
    m_address.sun_family = AF_UNIX;
    strncpy(m_address.sun_path, m_socket_path.c_str(), sizeof(m_address.sun_path) - 1);
}

void
MyClientSocket::connect_to_server()
{
    m_socket_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (m_socket_fd == -1) {
        std::cerr << "Unable to create client socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    short unsigned retries = 0;

    while(connect(m_socket_fd, (const struct sockaddr *) &m_address, sizeof(struct sockaddr_un)) == -1)
    {
        retries++;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (retries == 20)
        {
            std::cerr << "Unable to connect to server after 20 retries: " << m_socket_path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

std::string
MyClientSocket::request_response(std::string request)
{
    connect_to_server();
    send_message(request);
    std::string ret = receive_message();
    std::cerr << "client receieved" << ret << std::endl;
    // need to parse this in the case of GET_ACHIEVEMENTS    
    disconnect();
    return ret;
}

void
MyClientSocket::disconnect()
{
    m_socket_fd = close(m_socket_fd);
}

void
MyClientSocket::kill_server()
{
    request_response(QUIT_GAME_STR);
}