#include "MyServerSocket.h"

#include "../common/functions.h"
#include <iostream>

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

    errno = 0;
    if ((m_socket_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1) 
    { 
        std::cerr << "Could not create the server socket. Exitting. Code: " << errno << std::endl;
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
    close(m_socket_fd);
    unlink_file();
}

void
MyServerSocket::unlink_file()
{
    unlink(m_socket_path.c_str());
}

void
MyServerSocket::run_server()
{
    int data_socket;
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
            send_message(data_socket, "SAM_ACK");
            close(data_socket);
            close(m_socket_fd);
            unlink_file();
            break;
        }

        send_message(data_socket, process_request(request));

        /* Close socket. */
        close(data_socket);
    }
}
