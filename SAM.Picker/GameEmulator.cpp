#include "GameEmulator.h"
#include <csignal>
#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include "../common/functions.h"
#include "globals.h"
#include "MainPickerWindow.h"

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
 * This one might need a little more documentation on the technical side.
 * This method is only received on the parent process. The son,
 * will retrieve the stats and achievements from steam, and once it is done,
 * it will send a SIGUSR1 to the parent back. The parent will then save the
 * new data, and update the view accordingly.
 *
 * TODO: Check for errors
 */
void handle_sigusr1_parent(int signum) {
    GameEmulator *inst = GameEmulator::get_instance();

    read_count(inst->m_pipe[0], &inst->m_achievement_count, sizeof(unsigned));

    if (inst->m_achievement_list != nullptr) {
        free(inst->m_achievement_list);
        inst->m_achievement_list = nullptr;
    }

    inst->m_achievement_list = (Achievement_t*)malloc(inst->m_achievement_count * sizeof(Achievement_t));

    if (!inst->m_achievement_list) {
        std::cerr << "ERROR: could not allocate memory." << std::endl;
        exit(EXIT_FAILURE);
    }

    for (unsigned i = 0; i < inst->m_achievement_count; i++) {
        read_count(inst->m_pipe[0], &(inst->m_achievement_list[i]), sizeof(Achievement_t));
    }

    inst->update_view();
}

/**
 * We are the child and we will receive data/stats of achievements to unlock/relock
 * then auto-commit changes and update the parent. Coodinating based on the number
 * of changes avoids potentially losing signals because of multiple being in flight.
 *
 * Data will have this shitty format:
 * - 1 unsigned for number of changes
 * - 1 char, 'a' for achievement, 's' for "stat"
 * - 1 unsigned int, 0 => locked, 1 => unlocked, or the stat progression
 * - The length of MAX_ID_LENGTH to get the achievement ID
 *
 * If someone wants to update this repo, maybe IPC is too low level for what I want
 * to achieve. This doesn't require super fast speed or anything, maybe see if
 * TCP server is easier to use. Or at least one type of interfacing because this code
 * gets really ugly over time, bad practices become usual.
 */
void handle_sigusr1_child(int signum) {
    GameEmulator *inst = GameEmulator::get_instance();
    ISteamUserStats *stats_api = SteamUserStats();
    int* pipe = inst->m_pipe;
    char type;

    // Read number of changes
    unsigned num_changes;
    read_count(pipe[0], &num_changes, sizeof(unsigned));

    for (unsigned i = 0; i < num_changes; i++)
    {
        read_count(pipe[0], &type, sizeof(char));

        if (type == 'a') {
            // We want to edit an achievement
            unsigned value;
            char achievement_id[MAX_ACHIEVEMENT_ID_LENGTH];

            read_count(pipe[0], &value, sizeof(unsigned));
            read_count(pipe[0], &achievement_id, MAX_ACHIEVEMENT_ID_LENGTH * sizeof(char));

            if (value == 0) {
                // We want to relock an achievement
                if (!stats_api->ClearAchievement(achievement_id)) {
                    std::cerr << "Relocking achievement " << achievement_id << " failed" << std::endl;
                    //keep going so we don't corrupt the pipe
                } else {
                    std::cerr << "Relocked achievement " << achievement_id << std::endl;
                }
            } else {
                // We want to unlock an achievement
                if (!stats_api->SetAchievement(achievement_id)) {
                    std::cerr << "Unlocking achievement " << achievement_id << " failed " << std::endl;
                    //keep going so we don't corrupt the pipe
                } else {
                    std::cerr << "Unlocked achievement " << achievement_id << std::endl;
                }
            }
        } else if (type == 's') {
            // We want to edit a stat
            // TODO
        }
    }

    // Auto-commit and auto-update after we receive everything

    if (!stats_api->StoreStats()) {
        std::cerr << "Committing changes failed" << std::endl;
    }

    // After the child changes achievements/stats, it will retrieve all achievements,
    // and send a SIGUSR1 signal to the parent when done, and start writing
    // to the pipe.
    std::cerr << "Child is updating parent" << std::endl;
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
    // We never close it because they are bidirectional (I know it's ugly)
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
            std::cerr << "An error occurred launching the Steam API. Aborting." << std::endl;
            std::cerr << "Make sure you are trying to run an app you own, and running with lauch.sh" << std::endl;
            exit(EXIT_FAILURE);
        }

        signal(SIGTERM, handle_sigterm);
        // Communicate game stats to parent, and
        // read from parents stats to modify
        signal(SIGUSR1, handle_sigusr1_child);

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
    if (!m_have_stats_been_requested) {
        m_have_stats_been_requested = true;
        ISteamUserStats *stats_api = SteamUserStats();
        if (!stats_api->RequestCurrentStats()) {
            std::cerr << "ERROR: User not logged in, exiting" << std::endl;
            exit(EXIT_FAILURE);
        }
    } else {
        std::cerr << "WARNING: Stats have already been requested" << std::endl;
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

/**
 * This method must only be called on the parent process.
 * It reset the GUI window in anticipation of the child
 * process sending it new information on the achievements
 * via handle_sigusr1_parent
 *
 */
void
GameEmulator::update_data_and_view() {
    // Must be run by the parent
    if(m_son_pid > 0) {
        g_main_gui->reset_achievements_list();
        g_main_gui->confirm_stats_list();

        // Child will autoupdate the parent after committing changes
    } else {
        std::cerr << "Could not update data & view, no child found." << std::endl;
    }
}
// => update_data_and_view

bool
GameEmulator::send_num_changes(unsigned num_changes) const {
    // We assume the son process is already running

    // Write the number of changes
    write(m_pipe[1], &num_changes, sizeof(unsigned));

    // Send it a signal after buffering the number of changes
    kill(m_son_pid, SIGUSR1);

    return false; // Yeah error handling? Maybe later (TODO)
}
// => unlock_achievement

/**
 * The parent process requests the son process to unlock an
 * achievement. So this code will be executed in the parent process,
 * so send a message to the son, associated with an achievement ID
 */
bool
GameEmulator::unlock_achievement(const char* ach_api_name) const {
    // We assume the son process is already running
    static const unsigned unlock_state = 1;

    // Write "a1" (for achievement unlock) then the achievement id
    write(m_pipe[1], "a", sizeof(char));
    write(m_pipe[1], &unlock_state, sizeof(unsigned));
    write(m_pipe[1], ach_api_name, MAX_ACHIEVEMENT_ID_LENGTH * sizeof(char));

    return false; // Yeah error handling? Maybe later (TODO)
}
// => unlock_achievement

bool
GameEmulator::relock_achievement(const char* ach_api_name) const {
    // We assume the son process is already running
    static const unsigned unlock_state = 0;

    // Write "a1" (for achievement unlock) then the achievement id
    write(m_pipe[1], "a", sizeof(char));
    write(m_pipe[1], &unlock_state, sizeof(unsigned));
    write(m_pipe[1], ach_api_name, MAX_ACHIEVEMENT_ID_LENGTH * sizeof(char));

    return false; // Yeah error handling? Maybe later (TODO)
}
// => relock_achievement

//TODO: send stats

/*****************************************
 * STEAM API CALLBACKS BELOW
 ****************************************/

/**
 * Retrieves all achievements data, then pipes the data to the
 * parent process.
 */
void
GameEmulator::OnUserStatsReceived(UserStatsReceived_t *callback) {
    // Check if we received the values for the good app
    if (std::string(getenv("SteamAppId")) == std::to_string(callback->m_nGameID)) {
        if ( k_EResultOK == callback->m_eResult ) {

            ISteamUserStats *stats_api = SteamUserStats();

            // ==============================
            // RETRIEVE IDS
            // ==============================
            const unsigned num_ach = stats_api->GetNumAchievements();

            if (num_ach == 0) {
                std::cerr << "No achievements for current game" << std::endl;
            }

            if (m_achievement_list != nullptr) {
                free(m_achievement_list);
                m_achievement_list = nullptr;
            }

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

                // TODO
                // https://partner.steamgames.com/doc/api/ISteamUserStats#RequestGlobalAchievementPercentages
                //stats_api->GetAchievementAchievedPercent(m_achievement_list[i].id, &(m_achievement_list[i].global_achieved_rate));
                m_achievement_list[i].global_achieved_rate = 0;
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
            std::cerr << "Received stats for the game, but an error occurrred." << std::endl;
        }
    } else {
        std::cerr << "Received stats for wrong game" << std::endl;
    }
}
// => OnUserStatsReceived
