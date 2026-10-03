//+ rouz edit (ChatGPT)
#include "AudioSound.h"
#include "sampman.h"

void
AudioSoundQueue_ClearRequested(uint8_t *order, uint8_t *count, uint8_t activeSamples)
{
	// Clear the selected order list and count without changing requested records
	for (uint8_t i = 0; i < activeSamples; i++)
		order[i] = activeSamples;
	*count = 0;
}

void
AudioSoundQueue_ClearActive(tSound *active, uint8_t activeSamples)
{
	// Reset only the active-record fields written by the original method
	for (uint8_t i = 0; i < activeSamples; i++) {
		active[i].m_nEntityIndex = AEHANDLE_NONE;
		active[i].m_nCounter = 0;
		active[i].m_nSampleIndex = NO_SAMPLE;
		active[i].m_nBankIndex = INVALID_SFX_BANK;
		active[i].m_bIs2D = 0;
		active[i].m_nPriority = 5;
		active[i].m_nFrequency = 0;
		active[i].m_nVolume = 0;
#ifdef EXTERNAL_3D_SOUND
		active[i].m_nEmittingVolume = 0;
#endif
		active[i].m_fDistance = 0.0f;
		active[i].m_bIsBeingPlayed = 0;
		active[i].m_bIsPlayingFinished = 0;
		active[i].m_nLoopCount = 1;
#ifndef GTA_PS2
		active[i].m_nLoopStart = 0;
		active[i].m_nLoopEnd = -1;
#endif
		active[i].m_fSpeedMultiplier = 0.0f;
		active[i].m_MaxDistance = 200.0f;
		active[i].m_nPan = 63;
		active[i].m_bStatic = 0;
		active[i].m_nFinalPriority = 0;
		active[i].m_nFramesToPlay = 0;
		active[i].m_nVolumeChange = -1;
		// Reset the three position coordinates to the original positive-zero values
		active[i].m_vecPos.x = 0.0f;
		active[i].m_vecPos.y = 0.0f;
		active[i].m_vecPos.z = 0.0f;
#ifdef AUDIO_REVERB
		active[i].m_bReverb = 0;
#endif // AUDIO_REVERB
#ifdef AUDIO_REFLECTIONS
		active[i].m_nReflectionDelay = 0;
		active[i].m_bReflections = 0;
#endif // AUDIO_REFLECTIONS
	}
}
//- rouz edit (ChatGPT)
