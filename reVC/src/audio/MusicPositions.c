//+ rouz edit (ChatGPT)
#include "MusicManager.h"
#include "DMAudio.h"

#define MUSIC_POSITION_MIN(a,b) ((a) < (b) ? (a) : (b))

void
MusicPositions_RecordRadioStats(cMusicManager *manager)
{
	// Preserve original timer reads, arithmetic and live owner state
	if (manager->m_nPlayingTrack < NUM_RADIOS) {
		double time /*Rusty*/ = MusicInitHost_PauseTime();
		if (time > manager->m_nLastTrackServiceTime)
			manager->aListenTimeArray[manager->m_nPlayingTrack] += time - manager->m_nLastTrackServiceTime;
	}
}

uint32_t
MusicPositions_GetTrackStartPos(cMusicManager *manager, uint32_t track)
{
	// Preserve original timer reads, arithmetic and live owner state
	if (!MusicState_IsInitialised(manager)) return 0;

	uint32_t pos = manager->m_aTracks[track].m_nPosition;
	if (MusicInitHost_PauseTime() > manager->m_aTracks[track].m_nLastPosCheckTimer)
		pos += MUSIC_POSITION_MIN(MusicInitHost_PauseTime() - manager->m_aTracks[track].m_nLastPosCheckTimer, 270000);
	else
		manager->m_aTracks[track].m_nLastPosCheckTimer = MusicInitHost_PauseTime();

	if (pos > manager->m_aTracks[track].m_nLength)
		pos %= manager->m_aTracks[track].m_nLength;
	return pos;
}

uint32_t
MusicPositions_GetRadioPosition(cMusicManager *manager, uint32_t station)
{
	// Preserve original timer reads, arithmetic and live owner state
	if (station < NUM_RADIOS)
		return MusicPositions_GetTrackStartPos(manager, station);
	return 0;
}

int32_t
DMAudio_GetRadioPosition(uint32_t station)
{
	// Return the original unsigned position through the signed public interface
	return MusicPositions_GetRadioPosition(&MusicManager, station);
}

//- rouz edit (ChatGPT)
