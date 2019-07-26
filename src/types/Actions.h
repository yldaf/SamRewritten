
// Message format shall be
// all GET format
#define GET_ACHIEVEMENTS_STR "GET_ACHIEVEMENTS"
#define STORE_ACHIEVEMENTS_STR "STORE_ACHIEVEMENTS"
#define QUIT_GAME_STR "QUIT_GAME"

#define SAM_ACTION_STR "SAM_ACTION"
#define ACHIEVEMENT_LIST_STR "ACHIEVEMENT_LIST"
#define ACHIEVEMENT_NAME_STR "ACHIEVEMENT_NAME"
#define ACHIEVED_STR "ACHIEVED"
#define SAM_ACK_STR "SAM_ACK"

// TODO: Add SAM_START as an action to this too?
// Would require more reorg of code structure
enum SAM_ACTION {
    GET_ACHIEVEMENTS,
    STORE_ACHIEVEMENTS,
    QUIT_GAME,
    INVALID
};

// support for the full achievement type shall be added
// for now just implement achieved/not achieved

/* JSON format shall be
messages are sent as plaintext strings
messages are delimited by the usual string NULL terminator

get all achievements for active game
{
    GET_ACHIEVEMENTS_STR
}
response
{
    int NUM_ACH  //maybe not necessary if we have .length function?
    [{
        str ACH_NAME
        bool ACHIEVED
    }]
}

store a list of achievement changes
{
    int NUM_ACH  //maybe not necessary if we have .length function?
    [{
        str ACH_NAME
        bool ACHIEVED
    }]
}

quit active game
{
    SAM_QUIT_STR
}

*/

