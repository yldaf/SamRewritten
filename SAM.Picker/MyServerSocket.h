#pragma once
#include "MySocket.h"

class MyServerSocket : public MySocket
{
private:
    /* data */
public:
    void run_server();
    MyServerSocket(AppId_t appid);
    MyServerSocket::~MyServerSocket();
};

MyServerSocket::MyServerSocket(AppId_t appid) : MySocket(appid)
{
    
    if (file_exists(m_socket_path))
    {
        std::cerr << "It looks like the server before me did not shutdown properly." << std::endl;
        if(unlink(m_socket_path.c_str()) < 0) {
            std::cout << "Something is wrong. Are you the right user? Exitting." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    if ((m_socket_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, m_socket_path.c_str(), sizeof(addr.sun_path) - 1);

    if(bind(m_socket_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        std::cerr << "Failed to bind server socket " << m_socket_path << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(m_socket_fd, 20) < 0)
    {
        std::cerr << "Unable to listen to the socket. Exitting." << std::endl;
        exit(EXIT_FAILURE);
    }

}

MyServerSocket::~MyServerSocket() 
{
    unlink(m_socket_path.c_str());
}

void
MyServerSocket::run_server()
{
    int data_socket;
    bool down_flag = false;
    for (;;) {

        /* Wait for incoming connection. */
        data_socket = accept(m_socket_fd, NULL, NULL);
        if (data_socket == -1) {
            std::cerr << "Server failed to accept. Exitting." << std::endl;
            exit(EXIT_FAILURE);
        }

        // Read all the client's request
        std::string request = receive_message(data_socket);
        if (request == END_OF_SERVICE)
        {
            down_flag = true;
        }

        send_message(data_socket, "got u");
        

        /* Close socket. */
        close(data_socket);

        /* Quit on DOWN command. */
        if (down_flag) {
            break;
        }
    }
}
