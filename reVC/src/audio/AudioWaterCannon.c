//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "sampman.h"
#include <math.h>

enum {
	WATER_CANNON_MAX_DIST = 30,
	WATER_CANNON_VOLUME = 50,
};

#ifndef GTA_PS2
#define AUDIO_WATER_CANNON_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_WATER_CANNON_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_WATER_CANNON_RESET_LOOP_OFFSETS
#define AUDIO_WATER_CANNON_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_WATER_CANNON_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_WATER_CANNON_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_WATER_CANNON_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_WATER_CANNON_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_WATER_CANNON_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_WATER_CANNON_SET_SOUND_REVERB(b)
#endif

void
AudioWaterCannon_Process(cAudioManager *manager, int32_t id)
{
	// Retain each cannon gate, the original shared position source and audible-loop setup
	for (uint32_t i = 0; i < NUM_WATERCANNONS; i++) {
		if (AudioWaterCannonHost_Id(i)) {
			// Copy the game position into the C sound record
			AudioWaterCannonHost_Position(i, &manager->m_sQueueSample.m_vecPos);
			float distSquared = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
			if (distSquared < (WATER_CANNON_MAX_DIST * WATER_CANNON_MAX_DIST)) {
				manager->m_sQueueSample.m_fDistance = (distSquared <= 0.0f ? 0.0f : sqrtf(distSquared));
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(WATER_CANNON_VOLUME, WATER_CANNON_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_MaxDistance = WATER_CANNON_MAX_DIST;
					manager->m_sQueueSample.m_nSampleIndex = SFX_JUMBO_TAXI;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_nFrequency = 15591;
					manager->m_sQueueSample.m_nPriority = 5;
					manager->m_sQueueSample.m_nCounter = i;
					manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
					manager->m_sQueueSample.m_nFramesToPlay = 8;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nLoopCount = 0;
					manager->m_sQueueSample.m_bStatic = 0;
					AUDIO_WATER_CANNON_SET_EMITTING_VOLUME(WATER_CANNON_VOLUME);
					AUDIO_WATER_CANNON_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					AUDIO_WATER_CANNON_SET_SOUND_REVERB(1);
					AUDIO_WATER_CANNON_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
		}
	}
}
//- rouz edit (ChatGPT)
