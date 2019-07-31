#pragma once

// Message format shall be
// all GET format
#define GET_ACHIEVEMENTS_STR "GET_ACHIEVEMENTS"
#define STORE_ACHIEVEMENTS_STR "STORE_ACHIEVEMENTS"
#define QUIT_GAME_STR "QUIT_GAME"

// Mirroring structure of Achievement_t, should be combined with that
#define SAM_ACTION_STR "SAM_ACTION"
#define ACHIEVEMENT_LIST_STR "ACHIEVEMENT_LIST"
#define NAME_STR "NAME"
#define DESC_STR "DESC"
#define ID_STR "ID"
#define RATE_STR "RATE"
#define ICON_STR "ICON"
#define ACHIEVED_STR "ACHIEVED"
#define HIDDEN_STR "HIDDEN"

#define SAM_ACK_STR "SAM_ACK"

// TODO: Add SAM_START as an action to this too?
// Would require more reorg of code structure
enum SAM_ACTION {
    GET_ACHIEVEMENTS,
    STORE_ACHIEVEMENTS,
    QUIT_GAME,
    INVALID
};

/* JSON format shall be
messages are sent as plaintext strings
messages are delimited by the usual string NULL terminator

get all achievements for active game
{
    SAM_ACTION_STR: GET_ACHIEVEMENTS_STR
}
response
{
    SAM_ACK: SAM_ACK,
    ACHIEVEMENT_LIST_STR:
        [
            {
            NAME_STR: "name"
            DESC_STR: "desc"
            ID_STR: "ID"
            RATE_STR: global_achieved_rate
            ICON_STR: icon
            ACHIEVED_STR: true/fase
            HIDDEN_STR: true/false
            },
            .
            .
            .
        ]
}

store a list of achievement changes
can be reduced to not encode the whole achievement,
only id and achieved status / stats changes
{
    SAM_ACTION_STR: STORE_ACHIEVEMENTS_STR
    ACHIEVEMENT_LIST_STR:
        [
            {
            NAME_STR: ""
            DESC_STR: ""
            ID_STR: "ID"
            RATE_STR: 0
            ICON_STR: 0
            ACHIEVED_STR: true/fase
            HIDDEN_STR: false
            },
            .
            .
            .
        ]
}

response
{
    SAM_ACK: SAM_ACK,
}

quit active game
{
    SAM_ACTION_STR: SAM_QUIT_STR
}
response
{
    SAM_ACK: SAM_ACK,
}

*/

