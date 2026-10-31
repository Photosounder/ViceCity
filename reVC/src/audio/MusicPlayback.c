//+ rouz edit (ChatGPT)
#include "MusicManager.h"
#include "AudioManager.h"
#include "DMAudio.h"
#include "sampman.h"

void
MusicPlayback_Terminate(cMusicManager *manager)
{
	// Preserve the original music gates, provider calls and subsequent state writes

	if (!MusicState_IsInitialised(manager)) return;

	if (SampleManager_IsStreamPlaying(&SampleManager, 0)) {
		SampleManager_StopStreamedFile(&SampleManager, 0);
		manager->m_nPlayingTrack = NO_TRACK;
	}
	manager->m_bIsInitialised = 0;
}

void
MusicPlayback_PreloadCutSceneMusic(cMusicManager *manager, uint32_t track)
{
	// Preserve the original music gates, provider calls and subsequent state writes

	if (MusicState_IsInitialised(manager) && !manager->m_bDisabled && track < TOTAL_STREAMED_SOUNDS && manager->m_nMusicMode == MUSICMODE_CUTSCENE) {
		AudioPolice_Reset(&AudioManager);
		while (SampleManager_IsStreamPlaying(&SampleManager, 0))
			SampleManager_StopStreamedFile(&SampleManager, 0);
		SampleManager_PreloadStreamedFile(&SampleManager, track, 0);
		SampleManager_SetStreamedVolumeAndPan(&SampleManager, MAX_VOLUME, 63, 1, 0);
		manager->m_nPlayingTrack = track;
	}
}

void
MusicPlayback_PlayPreloadedCutSceneMusic(cMusicManager *manager)
{
	// Preserve the original music gates, provider calls and subsequent state writes

	if (MusicState_IsInitialised(manager) && !manager->m_bDisabled && manager->m_nMusicMode == MUSICMODE_CUTSCENE)
		SampleManager_StartPreloadedStreamedFile(&SampleManager, 0);
}

void
MusicPlayback_StopCutSceneMusic(cMusicManager *manager)
{
	// Preserve the original music gates, provider calls and subsequent state writes

	if (MusicState_IsInitialised(manager) && !manager->m_bDisabled && manager->m_nMusicMode == MUSICMODE_CUTSCENE) {
		SampleManager_StopStreamedFile(&SampleManager, 0);
		manager->m_nPlayingTrack = NO_TRACK;
	}
}

void
DMAudio_PreloadCutSceneMusic(uint32_t track)
{
	// Preserve the original game audio operation through the C interface
	MusicPlayback_PreloadCutSceneMusic(&MusicManager, track);
}

void
DMAudio_PlayPreloadedCutSceneMusic(void)
{
	// Preserve the original game audio operation through the C interface
	MusicPlayback_PlayPreloadedCutSceneMusic(&MusicManager);
}

void
DMAudio_StopCutSceneMusic(void)
{
	// Preserve the original game audio operation through the C interface
	MusicPlayback_StopCutSceneMusic(&MusicManager);
}

//- rouz edit (ChatGPT)
