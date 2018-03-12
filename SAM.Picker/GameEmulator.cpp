#include "GameEmulator.h"

/****************************
 * SIGNAL CALLBACKS
 ****************************/

/**
 * Used by the child process when the parent tells him to 
 * stop the steam app. The child process will die.
 */
void 
handle_sigterm(int signum) {
    SteamAPI_Shutdown();
    exit(EXIT_SUCCESS);
}

/**
 * Used by the parent process to remove the zombie process
 * of the child, once it terminated
 */
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
 * When the parent receives SIGUSR1, it will read the pipe,
 * and if everything goes well, it should fill the member 
 * achievements list, and the achievement count.
 * Once done, will update the view
 * 
 * TODO: Check for errors
 */
void handle_sigusr1_parent(int signum) {
    GameEmulator *inst = GameEmulator::get_instance();
    Achievement_t achievement;

    read(inst->m_pipe[0], &inst->m_achievement_count, sizeof(unsigned));

    inst->m_achievement_list = (Achievement_t*)malloc(inst->m_achievement_count * sizeof(Achievement_t));
    
    if( !inst->m_achievement_list ) {
        std::cerr << "ERROR: could not allocate memory." << std::endl;
        exit(EXIT_FAILURE);
    }

    //TODO see if I can read directly in the achievements list
    //That would avoid the memcpy
    for(unsigned i = 0; i < inst->m_achievement_count; i++) {
        read(inst->m_pipe[0], &achievement, sizeof(Achievement_t));
        memcpy(&(inst->m_achievement_list[i]), &achievement, sizeof(Achievement_t));
    }

    inst->update_view();
}

/**
 * When the son process receives SIGUSR1, it will retrieve all achievements,
 * and send a SIGUSR1 signal to the parent when done, and start writing 
 * to the pipe.
 */
void handle_sigusr1_child(int signum) {
    GameEmulator *inst = GameEmulator::get_instance();
    inst->retrieve_achievements();
}


/*********************************
 * CLASS METHODS DEFINITION
 ********************************/

GameEmulator::GameEmulator() : 
m_CallbackUserStatsReceived( this, &GameEmulator::OnUserStatsReceived ),
m_achievement_list( nullptr ),
m_son_pid( -1 ),
m_have_stats_been_requested( false )
{

}
// => Constructor

GameEmulator*
GameEmulator::get_instance() {
    static GameEmulator me;
    return &me;
}
// => get_instance

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
// => init_app


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
        if (g_main_gui != NULL)
            std::cerr << "Warning: trying to kill the Steam Game while it's not running." << std::endl;
        
        return true;
    }

    return true;
}
// => kill_running app


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
// => retrieve_achievements


void
GameEmulator::update_view() {
    for(unsigned i = 0; i < m_achievement_count; i++) {
        g_main_gui->add_to_achievement_list(m_achievement_list[i]);
    }

    g_main_gui->confirm_stats_list();
}
// => update_view

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