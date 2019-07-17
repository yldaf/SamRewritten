#include "GameServerManager.h"

#include "MyGameServer.h"
#include <signal.h>
#include <iostream>
#include <unistd.h>

MyClientSocket*
GameServerManager::quick_server_create(AppId_t appid)
{
    pid_t pid;
    if((pid = fork()) == 0) {
        // Son's process: server role
        
        MyGameServer server(appid);
        server.run();

        // The server will stop running when the client sends a quit request.

        exit(EXIT_SUCCESS);
    }
    else if (pid == -1) {
        std::cerr << "Could not fork in GameServerManager." << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        // Parent process
        // TODO watch out for zombie processes
        signal(SIGCHLD, SIG_IGN);
        return new MyClientSocket(appid);
    }
}