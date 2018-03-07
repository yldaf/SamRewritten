#include "GameEmulator.h"

void 
handle_sigterm(int signum) {
    SteamAPI_Shutdown();
    exit(EXIT_SUCCESS);
}

// Terminate the zombie process
void 
handle_sigchld(int signum) {
    pid_t pid;
    GameEmulator* emulator = GameEmulator::get_instance();

    while (true) {
        pid = waitpid(WAIT_ANY, NULL, WNOHANG);
        if (pid == 0)
            return;
        else if (pid == -1)
            return;
        else {
            std::cerr << "Steam game terminated." << std::endl;
            emulator->m_son_pid = -1;
        }
    }
}

GameEmulator*
GameEmulator::get_instance() {
    static GameEmulator me;
    return &me;
}

bool
GameEmulator::init_app(const std::string& app_id) {

    if(m_son_pid > 0) {
        std::cerr << "Warning: trying to initialize a Steam App while one is already running." << std::endl;
        return false;
    }

    pid_t pid;
    if((pid = fork()) == 0) {
        //Son's process
        setenv("SteamAppId", app_id.c_str(), 1);
        if( !SteamAPI_Init() ) {
            // TODO Pipe to parent a fail
            exit(EXIT_FAILURE);
        }

        signal(SIGTERM, handle_sigterm);

        // TODO Pipe all the shit to parent

        for(;;) {
            sleep(10);
            std::cerr << "I'm a Steam game lol" << std::endl;
        }
    }
    else if (pid == -1) {
        std::cerr << "An error occurred while forking. Exitting." << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        //Main process
        signal(SIGCHLD, handle_sigchld);
        m_son_pid = pid;
    }

    //Only a successful parent will reach this
    return true;
}


bool
GameEmulator::kill_running_app() {
    if(m_son_pid > 0) {
        kill(m_son_pid, SIGTERM);
        // We will set the pid back to -1 when son's death is confirmed
        return true;
    }
    else {
        std::cerr << "Warning: trying to kill the Steam Game while it's not running." << std::endl;
        return true;
    }

    return true;
}