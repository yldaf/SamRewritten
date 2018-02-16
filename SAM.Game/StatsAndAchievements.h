//========= Copyright � 1996-2008, Valve LLC, All rights reserved. ============
//
// Purpose: Class for tracking stats and achievements
//
// $NoKeywords: $
//=============================================================================

#ifndef STATS_AND_ACHIEVEMENTS_H
#define STATS_AND_ACHIEVEMENTS_H

#define ARRAYSIZE(A) ( sizeof(A)/sizeof(A[0]) )

#include <string>
#include <vector>
#include "Achievement_t.h"
#include "AchievementsDAO.h"
#include "../common/c_processes.h"
#include "../common/lodepng.h"
#include "../steam/steam_api.h"

class ISteamUser;

class CStatsAndAchievements
{
public:
	// Constructor
	CStatsAndAchievements();

	// Try to download the game's schema
	void AskForSchema();

	//Print achievements to console
	void PrintAchievements() const;


	STEAM_CALLBACK( CStatsAndAchievements, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived );
	STEAM_CALLBACK( CStatsAndAchievements, OnUserStatsStored, UserStatsStored_t, m_CallbackUserStatsStored );
	STEAM_CALLBACK( CStatsAndAchievements, OnAchievementStored, UserAchievementStored_t, m_CallbackAchievementStored );
	STEAM_CALLBACK( CStatsAndAchievements, OnAchievementIconFetched, UserAchievementIconFetched_t, m_CallbackAchievementIconFetched );

private:

	// Determine if we get this achievement now
	//void EvaluateAchievement( Achievement_t &achievement );
	//void UnlockAchievement( Achievement_t &achievement );

	bool ExportIconToFile( int& handle ) const;

	// The list of all the achievements
	std::vector<struct Achievement_t> m_all_achievements;

	// Store stats
	void StoreStatsIfNecessary();

	// our GameID
	CGameID m_GameID;

	// Steam User interface
	ISteamUser *m_pSteamUser;

	// Steam UserStats interface
	ISteamUserStats *m_pSteamUserStats;

	// The total number of achievements known from Steam
	unsigned m_num_achievements;

	// Did we get the stats from Steam?
	bool m_bRequestedStats;
	bool m_bStatsValid;

	// Should we store stats this frame?
	bool m_bStoreStats;
};

#endif // STATS_AND_ACHIEVEMENTS_H