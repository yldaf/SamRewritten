#pragma once

#define MAX_ACHIEVEMENT_ID_LENGTH 256
#define MAX_ACHIEVEMENT_DESC_LENGTH 256
#define MAX_ACHIEVEMENT_NAME_LENGTH 100

/**
 * Achievement structure.
 * It uses basic C types and has a fixed length, because they need 
 * to be piped between different process quickly.
 * The set limits are arbitrary, we will assume that if the name or
 * desription is longer than the given size, the display won't look great,
 * so we trim the end.
 */
struct Achievement_t {
    char name[MAX_ACHIEVEMENT_NAME_LENGTH];
    char desc[MAX_ACHIEVEMENT_DESC_LENGTH];
    char id[MAX_ACHIEVEMENT_ID_LENGTH]; // I have no idea what the length limit of this is. Crossing fingers there's none above 256.
    float global_achieved_rate;  
    int icon_handle; //0 : incorrect, error occurred, RTFM
	bool achieved;
    bool hidden;
};

typedef struct Achievement_t Achievement_t;