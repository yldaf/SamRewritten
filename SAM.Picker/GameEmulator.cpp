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

/**
 * The parent has to read the pipe
 */
void handle_sigusr1_parent(int signum) {
    GameEmulator *inst = GameEmulator::get_instance();
    unsigned num_ach;
    Achievement_t achievement;

    read(inst->m_pipe[0], &num_ach, sizeof(unsigned));

    inst->m_achievement_list = (Achievement_t*)malloc(num_ach * sizeof(Achievement_t));
    
    if( !inst->m_achievement_list ) {
        std::cerr << "ERROR: could not allocate memory." << std::endl;
        exit(EXIT_FAILURE);
    }

    for(unsigned i = 0; i < num_ach; i++) {
        read(inst->m_pipe[0], &achievement, sizeof(Achievement_t));
        memcpy(&(inst->m_achievement_list[i]), &achievement, sizeof(Achievement_t));
    }

    inst->update_view();
}

/**
 * The son must update stats and achievements and pipe
 * everything to parent
 */
void handle_sigusr1_child(int signum) {
    GameEmulator *inst = GameEmulator::get_instance();
    inst->retrieve_achievements();
}


GameEmulator::GameEmulator() : 
m_CallbackUserStatsReceived( this, &GameEmulator::OnUserStatsReceived ),
m_achievement_list( nullptr ),
m_son_pid( -1 ),
m_have_stats_been_requested( false )
{

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

    // Create the pipe
    if( pipe(m_pipe) == -1 ) {
        std::cerr << "Could not create a pipe. Exitting." << std::endl;
        std::cerr << "errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    if((pid = fork()) == 0) {
        //Son's process
        setenv("SteamAppId", app_id.c_str(), 1);
        if( !SteamAPI_Init() ) {
            std::cerr << "An error occurred launching the steam API. Aborting." << std::endl;
            exit(EXIT_FAILURE);
        }

        signal(SIGTERM, handle_sigterm);
        signal(SIGUSR1, handle_sigusr1_child);

        // Close the input, son will only output
        close(m_pipe[0]);

        retrieve_achievements();
        for(;;) {
            SteamAPI_RunCallbacks();
            sleep(1);
        }
    }
    else if (pid == -1) {
        std::cerr << "An error occurred while forking. Exitting." << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        //Main process
        signal(SIGCHLD, handle_sigchld);
        signal(SIGUSR1, handle_sigusr1_parent);

        close(m_pipe[1]); // Close output
        m_son_pid = pid;
    }

    //Only a successful parent will reach this
    return true;
}


bool
GameEmulator::kill_running_app() {
    if(m_son_pid > 0) {
        kill(m_son_pid, SIGTERM);
        free(m_achievement_list);
        m_achievement_list = nullptr;

        // We will set the pid back to -1 when son's death is confirmed
        return true;
    }
    else {
        std::cerr << "Warning: trying to kill the Steam Game while it's not running." << std::endl;
        return true;
    }

    return true;
}

void
GameEmulator::retrieve_achievements() {
    if(m_achievement_list != nullptr) {
        std::cerr << "WARNING: Achievement list given may already have been initialized" << std::endl;
    }

    if( !m_have_stats_been_requested ) {
        m_have_stats_been_requested = true;
        ISteamUserStats *stats_api = SteamUserStats();
        stats_api->RequestCurrentStats();
    }
}

void
GameEmulator::update_view() {
    //TODO
}

/*****************************************
 * STEAM API CALLBACKS BELOW
 ****************************************/

void
GameEmulator::OnUserStatsReceived(UserStatsReceived_t *callback) {
    // Check if we received the values for the good app
    if(std::string(getenv("SteamAppId")) == std::to_string(callback->m_nGameID)) {
        if ( k_EResultOK == callback->m_eResult ) {

            ISteamUserStats *stats_api = SteamUserStats();
            
            // ==============================
            // RETRIEVE IDS
            // ==============================
            const unsigned num_ach = stats_api->GetNumAchievements();
            m_achievement_list = (Achievement_t*)malloc(num_ach * sizeof(Achievement_t));

            for (unsigned i = 0; i < num_ach ; i++) {
                // TODO: strncpy is slow, because it fills the remaining space with NULLs
                // This is last stage optimisation but, could have used strcpy, or sprintf,
                // making sure strings are NULL terminated
                // see "man strncpy" for a possible implementation
                strncpy(
                    m_achievement_list[i].id, 
                    stats_api->GetAchievementName(i), 
                    MAX_ACHIEVEMENT_ID_LENGTH);
                
                strncpy(
                    m_achievement_list[i].name, 
                    stats_api->GetAchievementDisplayAttribute(m_achievement_list[i].id, "name"),
                    MAX_ACHIEVEMENT_NAME_LENGTH);

                strncpy(
                    m_achievement_list[i].desc,
                    stats_api->GetAchievementDisplayAttribute(m_achievement_list[i].id, "desc"),
                    MAX_ACHIEVEMENT_DESC_LENGTH);

                stats_api->GetAchievementAchievedPercent(m_achievement_list[i].id, &(m_achievement_list[i].global_achieved_rate));
                stats_api->GetAchievement(m_achievement_list[i].id, &(m_achievement_list[i].achieved));
                m_achievement_list[i].hidden = (bool)strcmp(stats_api->GetAchievementDisplayAttribute( m_achievement_list[i].id, "hidden" ), "0");
                m_achievement_list[i].icon_handle = stats_api->GetAchievementIcon( m_achievement_list[i].id );
            }

            // std::cerr << m_achievement_list[0].id << std::endl;
            // std::cerr << m_achievement_list[0].name << std::endl;
            // std::cerr << m_achievement_list[0].global_achieved_rate << std::endl;
            // std::cerr << m_achievement_list[0].desc << std::endl;

            //TODO: pipe the achievement list to daddy

            //Tell parent that he must read
            kill(getppid(), SIGUSR1);
            

            //Start writing
            write(m_pipe[1], &num_ach, sizeof(unsigned));

            // We could send all the memory bloc at once, but the pipe buffer 
            // might not be big enough on some systems, so let's just loop
            for(unsigned i = 0; i < num_ach; i++) {
                write(m_pipe[1], &(m_achievement_list[i]), sizeof(Achievement_t));
            }

            m_have_stats_been_requested = false;
        } else {
            std::cerr << "Received stats for the game, but an erorr occurrred." << std::endl;
        }
    }
}