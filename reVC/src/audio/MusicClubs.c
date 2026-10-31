//+ rouz edit (ChatGPT)
#include "MusicManager.h"
#include "AudioManager.h"
#include "soundlist.h"

void
MusicClubs_SetMalibuClubTrackPos(cMusicManager *manager, uint8_t scriptObject)
{
	// Preserve default positions, track guards and the timer update after position selection
	if (!MusicState_IsInitialised(manager))
		manager->m_aTracks[STREAMED_SOUND_MALIBU_AMBIENT].m_nPosition = 8640;
	if (manager->m_nNextTrack != STREAMED_SOUND_MALIBU_AMBIENT && manager->m_nPlayingTrack != STREAMED_SOUND_MALIBU_AMBIENT) {
		switch (scriptObject)
		{
		case SCRIPT_SOUND_NEW_BUILDING_MALIBU_1:
			manager->m_aTracks[STREAMED_SOUND_MALIBU_AMBIENT].m_nPosition = (AudioManager.m_anRandomTable[0] % 128) + 8640;
			break;
		case SCRIPT_SOUND_NEW_BUILDING_MALIBU_2:
			manager->m_aTracks[STREAMED_SOUND_MALIBU_AMBIENT].m_nPosition = (AudioManager.m_anRandomTable[0] % 128) + 286720;
			break;
		case SCRIPT_SOUND_NEW_BUILDING_MALIBU_3:
			manager->m_aTracks[STREAMED_SOUND_MALIBU_AMBIENT].m_nPosition = (AudioManager.m_anRandomTable[0] % 128) + 509120;
			break;
		}
		manager->m_aTracks[STREAMED_SOUND_MALIBU_AMBIENT].m_nLastPosCheckTimer = MusicInitHost_PauseTime();
	}
}

void
MusicClubs_SetStripClubTrackPos(cMusicManager *manager, uint8_t scriptObject)
{
	// Preserve default positions, track guards and the timer update after position selection
	if (!MusicState_IsInitialised(manager))
		manager->m_aTracks[STREAMED_SOUND_STRIPCLUB_AMBIENT].m_nPosition = 0;
	if (manager->m_nNextTrack != STREAMED_SOUND_STRIPCLUB_AMBIENT && manager->m_nPlayingTrack != STREAMED_SOUND_STRIPCLUB_AMBIENT)
	{
		switch (scriptObject)
		{
		case SCRIPT_SOUND_NEW_BUILDING_STRIP_1:
			manager->m_aTracks[STREAMED_SOUND_STRIPCLUB_AMBIENT].m_nPosition = AudioManager.m_anRandomTable[0] % 128;
			break;
		case SCRIPT_SOUND_NEW_BUILDING_STRIP_2:
			manager->m_aTracks[STREAMED_SOUND_STRIPCLUB_AMBIENT].m_nPosition = (AudioManager.m_anRandomTable[0] % 128) + 320200;
			break;
		case SCRIPT_SOUND_NEW_BUILDING_STRIP_3:
			manager->m_aTracks[STREAMED_SOUND_STRIPCLUB_AMBIENT].m_nPosition = (AudioManager.m_anRandomTable[0] % 128) + 672000;
			break;
		}
		manager->m_aTracks[STREAMED_SOUND_STRIPCLUB_AMBIENT].m_nLastPosCheckTimer = MusicInitHost_PauseTime();
	}
}

//- rouz edit (ChatGPT)
