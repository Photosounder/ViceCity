//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "sampman.h"
#include "../entities/EntityTypes.h"
#include <math.h>

enum {
	FIRE_DEFAULT_MAX_DIST = 80,
	FIRE_DEFAULT_VOLUME = 80,
	FIRE_BUILDING_MAX_DIST = 80,
	FIRE_BUILDING_VOLUME = 100,
	FIRE_PED_MAX_DIST = 25,
	FIRE_PED_VOLUME = 60,
	FIRE_EXTINGUISH_VOLUME = 100,
};

#ifndef GTA_PS2
#define AUDIO_FIRES_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_FIRES_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_FIRES_RESET_LOOP_OFFSETS
#define AUDIO_FIRES_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_FIRES_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_FIRES_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_FIRES_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_FIRES_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_FIRES_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_FIRES_SET_SOUND_REVERB(b)
#endif

void
AudioFires_Process(cAudioManager *manager, int32_t id)
{
	// Retain live fire gates, captured entity selection and both sound paths
	CEntity *entity;
	uint8_t Vol;
	float distSquared;

	for (uint8_t i = 0; i < NUM_FIRES; i++) {
		if (AudioFiresHost_Ongoing(i) && AudioFiresHost_AudioSet(i)) {
			entity = AudioFiresHost_Entity(i);
			if (entity) {
				switch (AudioPedOneShotsHost_EntityType(entity)) {
				case ENTITY_TYPE_BUILDING:
					manager->m_sQueueSample.m_MaxDistance = FIRE_BUILDING_MAX_DIST;
					manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_ON_FIRE;
					Vol = FIRE_BUILDING_VOLUME;
					manager->m_sQueueSample.m_nFrequency = 8 * SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_CAR_ON_FIRE) / 10;
					manager->m_sQueueSample.m_nFrequency += i * (manager->m_sQueueSample.m_nFrequency >> 6);
					manager->m_sQueueSample.m_nPriority = 6;
					break;
				case ENTITY_TYPE_PED:
					manager->m_sQueueSample.m_MaxDistance = FIRE_PED_MAX_DIST;
					manager->m_sQueueSample.m_nSampleIndex = SFX_PED_ON_FIRE;
					manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_PED_ON_FIRE);
					Vol = FIRE_PED_VOLUME;
					manager->m_sQueueSample.m_nFrequency += i * (manager->m_sQueueSample.m_nFrequency >> 6);
					manager->m_sQueueSample.m_nPriority = 10;
					break;
				default:
					manager->m_sQueueSample.m_MaxDistance = FIRE_DEFAULT_MAX_DIST;
					manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_ON_FIRE;
					manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_CAR_ON_FIRE);
					manager->m_sQueueSample.m_nFrequency += i * (manager->m_sQueueSample.m_nFrequency >> 6);
					Vol = FIRE_DEFAULT_VOLUME;
					manager->m_sQueueSample.m_nPriority = 8;
				}
			} else {
				manager->m_sQueueSample.m_MaxDistance = FIRE_DEFAULT_MAX_DIST;
				manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_ON_FIRE;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_CAR_ON_FIRE);
				Vol = FIRE_DEFAULT_VOLUME;
				manager->m_sQueueSample.m_nPriority = 8;
			}
			// Copy the game position into the C sound record
			AudioFiresHost_Position(i, &manager->m_sQueueSample.m_vecPos);
			distSquared = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
			if (distSquared < (manager->m_sQueueSample.m_MaxDistance * manager->m_sQueueSample.m_MaxDistance)) {
				manager->m_sQueueSample.m_fDistance = (distSquared <= 0.0f ? 0.0f : sqrtf(distSquared));
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, manager->m_sQueueSample.m_MaxDistance, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_nCounter = i;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
					manager->m_sQueueSample.m_nFramesToPlay = 10;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nLoopCount = 0;
					manager->m_sQueueSample.m_bStatic = 0;
					AUDIO_FIRES_SET_EMITTING_VOLUME(Vol);
					AUDIO_FIRES_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					AUDIO_FIRES_SET_SOUND_REVERB(1);
					AUDIO_FIRES_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
			// Consume the live water flag and use the current scratch distance
			if (AudioFiresHost_Water(i)) {
				AudioFiresHost_ClearWater(i);
				Vol = FIRE_EXTINGUISH_VOLUME * AudioFiresHost_Countdown(i);
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, manager->m_sQueueSample.m_MaxDistance, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_nSampleIndex = SFX_JUMBO_TAXI;
					manager->m_sQueueSample.m_nFrequency = 19591;
					manager->m_sQueueSample.m_nFrequency += i * (manager->m_sQueueSample.m_nFrequency >> 6);
					manager->m_sQueueSample.m_nPriority = 9;
					manager->m_sQueueSample.m_nCounter = i + 40;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
					manager->m_sQueueSample.m_nFramesToPlay = 10;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nLoopCount = 0;
					manager->m_sQueueSample.m_bStatic = 0;
					AUDIO_FIRES_SET_EMITTING_VOLUME(Vol);
					AUDIO_FIRES_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					AUDIO_FIRES_SET_SOUND_REVERB(1);
					AUDIO_FIRES_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
		}
	}
}
//- rouz edit (ChatGPT)
