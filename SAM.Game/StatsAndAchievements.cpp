//========= Copyright � 1996-2008, Valve LLC, All rights reserved. ============
//
// Purpose: Class for tracking stats and achievements
//
// $NoKeywords: $
//=============================================================================

#include "StatsAndAchievements.h"
#include <math.h>

void OutputDebugString( const char *pchMsg )
{
	fprintf( stderr, "%s", pchMsg );
}


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
#pragma warning( push )
//  warning C4355: 'this' : used in base member initializer list
//  This is OK because it's warning on setting up the Steam callbacks, they won't use this until after construction is done
#pragma warning( disable : 4355 ) 
CStatsAndAchievements::CStatsAndAchievements()
	: 
	m_pSteamUser( NULL ),
	m_pSteamUserStats( NULL ),
	m_GameID( SteamUtils()->GetAppID() ),
	m_CallbackUserStatsReceived( this, &CStatsAndAchievements::OnUserStatsReceived ),
	m_CallbackUserStatsStored( this, &CStatsAndAchievements::OnUserStatsStored ),
	m_CallbackAchievementStored( this, &CStatsAndAchievements::OnAchievementStored ),
	m_CallbackAchievementIconFetched( this, &CStatsAndAchievements::OnAchievementIconFetched )
{
	m_pSteamUser = SteamUser();
	m_pSteamUserStats = SteamUserStats();

	m_bRequestedStats = false;
	m_bStatsValid = false;
	m_bStoreStats = false;
	m_num_achievements = 0;
}
#pragma warning( pop )

//-----------------------------------------------------------------------------
// Purpose: Run a frame for the CStatsAndAchievements
//-----------------------------------------------------------------------------
void CStatsAndAchievements::AskForSchema()
{
	if ( !m_bRequestedStats )
	{
		OutputDebugString("Requesting stats\n");

		// Is Steam Loaded? if no, can't get stats, done
		if ( NULL == m_pSteamUserStats || NULL == m_pSteamUser )
		{
			OutputDebugString("Steam is not loaded, can't request stats.");
			m_bRequestedStats = true;
			return;
		}

		// If yes, request our stats
		bool bSuccess = m_pSteamUserStats->RequestCurrentStats();
		
		// This function should only return false if we weren't logged in, and we already checked that.
		// But handle it being false again anyway, just ask again later.
		m_bRequestedStats = bSuccess;
	}

	if ( !m_bStatsValid )
		return;

	// Get info from sources

	// Evaluate achievements
	/**
	 * For each achievement
	 * 	If it is marked as "modified value"
	 * 		If it is marked as "To be unlocked"
	 * 			m_pSteamUserStats->SetAchievement( achievement.m_pchAchievementID );
	 * 		else
	 * 			Find equivalent to relock an achievement
	 */

	// Store stats
	//StoreStatsIfNecessary();
}



//-----------------------------------------------------------------------------
// Purpose: Store stats in the Steam database
//-----------------------------------------------------------------------------
void CStatsAndAchievements::StoreStatsIfNecessary()
{
	if ( m_bStoreStats )
	{
		// already set any achievements in UnlockAchievement

		// set stats
		//m_pSteamUserStats->SetStat( "NumGames", m_nTotalGamesPlayed );
		
		// Update average feet / second stat
		//m_pSteamUserStats->UpdateAvgRateStat( "AverageSpeed", m_flGameFeetTraveled, m_flGameDurationSeconds );
		
		// The averaged result is calculated for us
		//m_pSteamUserStats->GetStat( "AverageSpeed", &m_flAverageSpeed );

		bool bSuccess = m_pSteamUserStats->StoreStats();
		// If this failed, we never sent anything to the server, try
		// again later.
		m_bStoreStats = !bSuccess;
	}
}


//-----------------------------------------------------------------------------
// Purpose: We have stats data from Steam. It is authoritative, so update
//			our data with those results now.
//-----------------------------------------------------------------------------
void CStatsAndAchievements::OnUserStatsReceived( UserStatsReceived_t *pCallback )
{	
	if ( !m_pSteamUserStats )
		return;

	// we may get callbacks for other games' stats arriving, ignore them
	if ( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
			OutputDebugString( "Received stats and achievements from Steam\n" );

			//Let's admit the steam files are in ~/.local/share/Steam
			std::string steam_directory = getenv("HOME");
			steam_directory += "/.local/share/Steam";
			std::string app_schema_file_name = steam_directory + "/appcache/stats/UserGameStatsSchema_" + std::string(getenv("SteamAppId")) + ".bin";

			//Methods that may come in handy
			// m_pSteamUserStats->GetStat( "NumGames", &m_nTotalGamesPlayed );
			// bool GetStat( const char *pchName, int32 *pData );
			// bool GetStat( const char *pchName, float *pData );
			
			
			//Methods that may come in handy when I will be implementing storing
			// bool ResetAllStats( bool bAchievementsToo );
			// SetAchievement /SetStat obviously
			// bool ClearAchievement( const char *pchName );

			
			// First, let's retrieve all achievement ids
			CAchievementsDAO ach_id_dao;
			ach_id_dao.load_binary_file(app_schema_file_name);
			std::vector<std::string> all_ids = ach_id_dao.get_all_achievements_ids();

			// Then we will retrieve all data about all the retreived ids
			m_all_achievements.clear();
			struct Achievement_t temp_achievement;

			//There was an error fetching achievements
			//If the game didn't have achievements we wouldn't be here but why not check
			//one more time anyway
			if(all_ids.empty() && ach_id_dao.app_has_achievements())
				return;

			for(std::string &i : all_ids) {
				temp_achievement.achievementID = i;
				temp_achievement.achievementName = 	m_pSteamUserStats->GetAchievementDisplayAttribute( i.c_str(), "name" );
				temp_achievement.achievementDesc = 	m_pSteamUserStats->GetAchievementDisplayAttribute( i.c_str(), "desc" );
				
				m_pSteamUserStats->GetAchievementAchievedPercent( i.c_str(), &temp_achievement.global_achieved_rate );
				m_pSteamUserStats->GetAchievement( i.c_str(), &temp_achievement.achieved );
				temp_achievement.hidden = (bool)strcmp(m_pSteamUserStats->GetAchievementDisplayAttribute( i.c_str(), "hidden" ), "0");
				temp_achievement.icon_handle = m_pSteamUserStats->GetAchievementIcon( i.c_str() );

				ExportIconToFile(temp_achievement.icon_handle);
				
				m_all_achievements.push_back(temp_achievement);
			}

			m_num_achievements = m_pSteamUserStats->GetNumAchievements();

			PrintAchievements();
		}
		else
		{
			char buffer[128];
			sprintf( buffer, "RequestStats - failed, this app probably doesn't have achievements or stats (Error %d)\n", pCallback->m_eResult );
			buffer[ sizeof(buffer) - 1 ] = 0;
			OutputDebugString( buffer );
		}
	}
}

/**
 * A simple method to display all the retrieved achievement data in stderr
 */
void CStatsAndAchievements::PrintAchievements() const {
	for(int i = 0; i < m_all_achievements.size(); i++) {
		std::cerr << "==============" << std::endl;
		std::cerr << "Achievement ID: \t" << m_all_achievements[i].achievementID << std::endl;
		std::cerr << "Achievement Name: \t" << m_all_achievements[i].achievementName << std::endl;
		std::cerr << "Achievement Desc: \t" << m_all_achievements[i].achievementDesc << std::endl;
		std::cerr << "Achievement rate: \t" << m_all_achievements[i].global_achieved_rate << std::endl;
		std::cerr << "Achievement icon id: \t" << m_all_achievements[i].icon_handle << std::endl;
		std::cerr << "Achievement hidden: \t" << std::string(m_all_achievements[i].hidden ? "yes" : "no") << std::endl;
		std::cerr << "Achievement achieved: \t" << std::string(m_all_achievements[i].achieved ? "yes" : "no") << std::endl;
	}

	std::cerr << "Retrieved " << m_all_achievements.size() << " achievements out of " << m_num_achievements << " total." << std::endl;
}

//-----------------------------------------------------------------------------
// Purpose: Our stats data was stored!
//-----------------------------------------------------------------------------
void CStatsAndAchievements::OnUserStatsStored( UserStatsStored_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if ( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
			OutputDebugString( "StoreStats - success\n" );
		}
		else if ( k_EResultInvalidParam == pCallback->m_eResult )
		{
			// One or more stats we set broke a constraint. They've been reverted,
			// and we should re-iterate the values now to keep in sync.
			OutputDebugString( "StoreStats - some failed to validate\n" );
			// Fake up a callback here so that we re-load the values.
			UserStatsReceived_t callback;
			callback.m_eResult = k_EResultOK;
			callback.m_nGameID = m_GameID.ToUint64();
			OnUserStatsReceived( &callback );
		}
		else
		{
			char buffer[128];
			sprintf( buffer, "StoreStats - failed, %d\n", pCallback->m_eResult );
			buffer[ sizeof(buffer) - 1 ] = 0;
			OutputDebugString( buffer );
		}
	}
}

/**
 * This method only triggers when icons are fetched for the first time
 * For now, SamRewritten exports the picture both when the fetch event is triggered
 * and when it's not. So it's doing an action twice when only one is needed, but 
 * for now it works so I'd better not touch anything
 */
void CStatsAndAchievements::OnAchievementIconFetched( UserAchievementIconFetched_t *pCallback ) {
	// This hurts me to write that, but it's the way it's meant to be, I guess
	if ( m_GameID.ToUint64() == pCallback->m_nGameID.ToUint64() ) 
	{	
		//We find the appropriate achievement and assign it to the handle for the image
		for(Achievement_t &i : m_all_achievements) {
			if(strcmp(i.achievementID.c_str(), pCallback->m_rgchAchievementName) == 0) {
				i.icon_handle = pCallback->m_nIconHandle;
				ExportIconToFile(i.icon_handle);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: An achievement was stored
//-----------------------------------------------------------------------------
void CStatsAndAchievements::OnAchievementStored( UserAchievementStored_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if ( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if ( 0 == pCallback->m_nMaxProgress )
		{
			char buffer[128];
			sprintf( buffer, "Achievement '%s' unlocked!", pCallback->m_rgchAchievementName );
			buffer[ sizeof(buffer) - 1 ] = 0;
			OutputDebugString( buffer );
		}
		else
		{
			char buffer[128];
			sprintf( buffer, "Achievement '%s' progress callback, (%d,%d)\n", 
				pCallback->m_rgchAchievementName, pCallback->m_nCurProgress, pCallback->m_nMaxProgress );
			buffer[ sizeof(buffer) - 1 ] = 0;
			OutputDebugString( buffer );
		}
	}
}

/**
 * Feed it a image handle
 * It will fetch the associated image, and save it as a PNG in the cache folder
 */
bool CStatsAndAchievements::ExportIconToFile( int& handle ) const {
	// Get the image dimensions
	uint32 uAvatarWidth, uAvatarHeight;
	bool success = SteamUtils()->GetImageSize( handle, &uAvatarWidth, &uAvatarHeight );
	if(!success) {
		// The parent process will use the default "missing icon"
		return false;
	}
	
	// Fill the image buffer
	const unsigned uImageSizeInBytes = uAvatarWidth * uAvatarHeight * 4;
	uint8 *pAvatarRGBA = new uint8[ uImageSizeInBytes ];
	success = SteamUtils()->GetImageRGBA( handle, pAvatarRGBA, uImageSizeInBytes );
	if(!success) {
		//Maybe the icon wasn't preloaded and this method will be called again in OnAchievementIconFetched
		//Or maybe the format just isn't recognised
		std::cerr << "Unable to retrieve achievement icon. A retry may occur. Handle: " << handle << std::endl;
		return false;
	}

	// Write the buffer to a PNG image in the cache folder
	const std::string icon_path("/tmp/SamRewritten/" + std::string(getenv("SteamAppId")) + "/" + std::to_string(handle));
	const unsigned error = lodepng::encode(icon_path.c_str(), pAvatarRGBA, uAvatarWidth, uAvatarHeight);
	if(error) {
		std::cerr << "Error while converting to png " << error << ": " << lodepng_error_text(error) << std::endl;
	}

	// Free the memory and exit
	delete[] pAvatarRGBA;
	return true;
}