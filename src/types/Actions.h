#pragma once

// Message format shall be
// all GET format
#define GET_ACHIEVEMENTS_STR "GET_ACHIEVEMENTS"
#define COMMIT_CHANGES_STR "COMMIT_CHANGES"
#define QUIT_GAME_STR "QUIT_GAME"

// Mirroring structure of Achievement_t, should be combined with that
#define SAM_ACTION_STR "SAM_ACTION"
#define ACHIEVEMENT_LIST_STR "ACHIEVEMENT_LIST"
#define STAT_LIST_STR "STAT_LIST"
#define NAME_STR "NAME"
#define DESC_STR "DESC"
#define ICON_STR "ICON"
#define ID_STR "ID"
#define RATE_STR "RATE"
#define ACHIEVED_STR "ACHIEVED"
#define HIDDEN_STR "HIDDEN"

// Mirroring structure of StatValue_t, should be combined with that
#define STAT_TYPE_STR "STAT_TYPE"
#define STAT_ID_STR "STAT_ID"
#define STAT_DISPLAY_STR "STAT_DISPLAY"
#define STAT_VALUE_STR "STAT_VALUE"
#define STAT_INCREMENTONLY_STR "STAT_INCREMENTONLY"
#define STAT_PERMISSION_STR "STAT_PERMISSION"

// Misc
#define SAM_ACK_STR "SAM_ACK"

// TODO: Add SAM_START as an action to this too?
// Would require more reorg of code structure
enum SAM_ACTION {
    GET_ACHIEVEMENTS,
    COMMIT_CHANGES,
    QUIT_GAME,
    INVALID
};

/* 

JSON format shall be
* messages are sent as plaintext strings
* messages are delimited by the usual string NULL terminator

For all the examples given, you can provide additional keys to the request, only the ones 
shown are taken in account.
The server can also answer with more keys, but if so their value will likely be garbage value.


# 1. Get all achievements and stats for active game

Request
Client --> Server

{
    SAM_ACTION_STR: GET_ACHIEVEMENTS_STR
}

Response
Server --> Client

{
    SAM_ACK: SAM_ACK,
    ACHIEVEMENT_LIST_STR:
        [
            {
                NAME_STR: "name"
                DESC_STR: "desc"
                ID_STR: "ID"
                ACHIEVED_STR: true/fase
                HIDDEN_STR: true/false,
                RATE_STR: float value (0 to 100)
            },
            .
            .
            .
        ],
    STAT_LIST_STR:
        [
            {
                STAT_DISPLAY_STR: "desc",
                STAT_ID_STR: "ID",
                STAT_INCREMENTONLY_STR: true/false,
                STAT_PERMISSION_STR: <int value>,
                STAT_TYPE_STR: <int cast of UserStatType>,
                STAT_VALUE_STR: <int or float>
            },
            .
            .
            .
        ]
}

# 2. Lock or unlock an achievement for active game

Client --> Server

{
    SAM_ACTION_STR: COMMIT_CHANGES_STR,
    ACHIEVEMENT_LIST_STR:
        [
            {
                ID_STR: "ID"
                ACHIEVED_STR: true/fase
            },
            .
            .
            .
        ],
    STAT_LIST_STR:
        [
            {
                STAT_ID_STR: "ID",
                STAT_TYPE_STR: <int cast of UserStatType>,
                STAT_VALUE_STR: <int or float>
            },
            .
            .
            .
        ]
}

Server --> Client

{
    SAM_ACK: SAM_ACK
}

# 3. Stop the server

Client --> Server

{
    SAM_ACTION_STR: SAM_QUIT_STR
}

Server --> Client

{
    SAM_ACK: SAM_ACK
}

*/

