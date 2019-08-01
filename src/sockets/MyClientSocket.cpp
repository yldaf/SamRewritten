#include "MyClientSocket.h"
#include "../types/Actions.h"
#include <yajl/yajl_gen.h>
#include "../common/yajlHelpers.h"

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
    std::cerr << "client receieved " << ret << std::endl;
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
    std::string response;
    const unsigned char * buf; 
    size_t len;

    yajl_gen handle = yajl_gen_alloc(NULL);
    if (yajl_gen_map_open(handle) != yajl_gen_status_ok) {
        std::cerr << "failed to make json" << std::endl;
    }
    encode_request(handle, QUIT_GAME_STR);
    if (yajl_gen_map_close(handle) != yajl_gen_status_ok) {
        std::cerr << "failed to make json" << std::endl;
    }
    yajl_gen_get_buf(handle, &buf, &len);
    response = request_response(std::string((const char*)buf));
    yajl_gen_free(handle);

    if (!decode_ack(response)) {
        std::cerr << "Failed to close game!" << std::endl;
    }
}