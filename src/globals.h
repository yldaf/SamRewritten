#pragma once

class MySteam;
class MySteamClient;
class PerfMon;

// Reason for the globals is that it's easier to access them in GTK callbacks,
// or the data they hold may be needed both by the parent process and its children.
// All those variables are initialised in main.cpp

/**
 * A custom Steam Client interface, because the Steamworks lib won't let me get 
 * the SteamClient() accessor before a SteamAPI_Init(). It replaces the accessor
 * without having to initialize the whole SDK.
 */
extern MySteamClient *g_steamclient;

/**
 * g_steam is the backend of all of this. This is used to retrieve achievements,
 * stats, achievement icons, etc.
 */
extern MySteam *g_steam;

/**
 * A basic performance monitor. Used to log stuff along with process info.
 */
extern PerfMon *g_perfmon;