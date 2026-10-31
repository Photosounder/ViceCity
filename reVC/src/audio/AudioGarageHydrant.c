//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "soundlist.h"
#include "sampman.h"
#include "../control/GarageTypes.h"

enum {
	GARAGES_MAX_DIST = 80,
	GARAGES_VOLUME = 90,
	GARAGES_DOOR_VOLUME = 60,

	FIRE_HYDRANT_MAX_DIST = 35,
	FIRE_HYDRANT_VOLUME = 40,
};

#ifndef GTA_PS2
#define AUDIO_GARAGE_HYDRANT_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_GARAGE_HYDRANT_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_GARAGE_HYDRANT_RESET_LOOP_OFFSETS
#define AUDIO_GARAGE_HYDRANT_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_GARAGE_HYDRANT_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_GARAGE_HYDRANT_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_GARAGE_HYDRANT_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_GARAGE_HYDRANT_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_GARAGE_HYDRANT_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_GARAGE_HYDRANT_SET_SOUND_REVERB(b)
#endif

void
AudioGarageHydrant_Garages(cAudioManager *manager)
{
	// Preserve the complete original generator, shared counter and live query order
	CEntity *entity;
	uint8_t state;
	uint32_t sampleIndex;
	uint8_t j;
	float distSquared;
	uint8_t distCalculated;

	static uint8_t iSound = 32;

#ifdef FIX_BUGS
	for (uint32_t i = 0; i < AudioGarageHydrantHost_Count(); i++) {
#else
	for (uint8_t i = 0; i < AudioGarageHydrantHost_Count(); i++) {
#endif
		if (AudioGarageHydrantHost_Type(i) == GARAGE_NONE)
			continue;
		entity = AudioGarageHydrantHost_Door(i);
		if (entity == NULL)
			continue;
		// Copy the game position into the C sound record
		AudioReleaseHost_Position(entity, &manager->m_sQueueSample.m_vecPos);
		distCalculated = 0;
		distSquared = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
		if (distSquared < (GARAGES_MAX_DIST * GARAGES_MAX_DIST)) {
			state = AudioGarageHydrantHost_State(i);
			if (state == GS_OPENING || state == GS_CLOSING || state == GS_AFTERDROPOFF) {
				AudioMath_CalculateDistance(&distCalculated, &manager->m_sQueueSample.m_fDistance, distSquared);
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(GARAGES_VOLUME, GARAGES_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					if (AudioGarageHydrantHost_Type(i) == GARAGE_CRUSHER) {
						if (AudioGarageHydrantHost_State(i) == GS_AFTERDROPOFF) {
							if (manager->m_FrameCounter & 1) {
								if (manager->m_anRandomTable[1] & 1)
									sampleIndex = manager->m_anRandomTable[2] % 5 + SFX_COL_CAR_1;
								else
									sampleIndex = manager->m_anRandomTable[2] % 6 + SFX_COL_CAR_PANEL_1;
								manager->m_sQueueSample.m_nSampleIndex = sampleIndex;
								manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex) >> 1;
								manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
								manager->m_sQueueSample.m_nLoopCount = 1;
								manager->m_sQueueSample.m_bStatic = 1;
								manager->m_sQueueSample.m_nCounter = iSound++;
								if (iSound < 32)
									iSound = 32;
							} else
								goto CheckGarageEvents; // premature exit to go straight to the for loop
						} else {
							manager->m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE;
							manager->m_sQueueSample.m_nFrequency = 6543;

							manager->m_sQueueSample.m_nCounter = i;
							manager->m_sQueueSample.m_nLoopCount = 0;
							manager->m_sQueueSample.m_nFramesToPlay = 3;
							manager->m_sQueueSample.m_bStatic = 0;
						}
					} else {
						manager->m_sQueueSample.m_nSampleIndex = SFX_GARAGE_DOOR_LOOP;
						manager->m_sQueueSample.m_nFrequency = 13961;

						manager->m_sQueueSample.m_nCounter = i;
						manager->m_sQueueSample.m_nLoopCount = 0;
						manager->m_sQueueSample.m_nFramesToPlay = 3;
						manager->m_sQueueSample.m_bStatic = 0;
					}

					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nPriority = 3;
					AUDIO_GARAGE_HYDRANT_SET_EMITTING_VOLUME(GARAGES_VOLUME);
					AUDIO_GARAGE_HYDRANT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
					manager->m_sQueueSample.m_MaxDistance = GARAGES_MAX_DIST;
					AUDIO_GARAGE_HYDRANT_SET_SOUND_REVERB(1);
					AUDIO_GARAGE_HYDRANT_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
		}
CheckGarageEvents:
		for (j = 0; j < manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_AudioEvents; j++) {
			switch (manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[j]) {
			case SOUND_GARAGE_DOOR_CLOSED:
			case SOUND_GARAGE_DOOR_OPENED:
				if (distSquared < (GARAGES_MAX_DIST * GARAGES_MAX_DIST)) {
					AudioMath_CalculateDistance(&distCalculated, &manager->m_sQueueSample.m_fDistance, distSquared);
					manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(GARAGES_DOOR_VOLUME, GARAGES_MAX_DIST, manager->m_sQueueSample.m_fDistance);
					if (manager->m_sQueueSample.m_nVolume > 0) {
						if (AudioGarageHydrantHost_Type(i) == GARAGE_CRUSHER) {
							manager->m_sQueueSample.m_nSampleIndex = SFX_COL_CAR_PANEL_2;
							manager->m_sQueueSample.m_nFrequency = 6735;
						} else if (manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[j] == SOUND_GARAGE_DOOR_OPENED) {
							manager->m_sQueueSample.m_nSampleIndex = SFX_COL_CAR_PANEL_2;
							manager->m_sQueueSample.m_nFrequency = 22000;
						} else {
							manager->m_sQueueSample.m_nSampleIndex = SFX_COL_GARAGE_DOOR_1;
							manager->m_sQueueSample.m_nFrequency = 18000;
						}
						manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						manager->m_sQueueSample.m_nPriority = 4;
						AUDIO_GARAGE_HYDRANT_SET_EMITTING_VOLUME(GARAGES_DOOR_VOLUME);
						manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
						manager->m_sQueueSample.m_MaxDistance = GARAGES_MAX_DIST;
						AUDIO_GARAGE_HYDRANT_SET_SOUND_REVERB(1); 
						manager->m_sQueueSample.m_bIs2D = 0;
						manager->m_sQueueSample.m_bStatic = 1;
						manager->m_sQueueSample.m_nLoopCount = 1;
						AUDIO_GARAGE_HYDRANT_RESET_LOOP_OFFSETS
						manager->m_sQueueSample.m_nCounter = iSound++;
						if (iSound < 32)
							iSound = 32;
						AUDIO_GARAGE_HYDRANT_SET_SOUND_REFLECTION(1);
						AudioRequests_Submit(manager);
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

void
AudioGarageHydrant_Hydrant(cAudioManager *manager)
{
	// Preserve the complete original generator, shared counter and live query order
	float distSquared;
	uint8_t distCalculated = 0;

	// Copy the game position into the C sound record
	AudioGarageHydrantHost_Position(manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_pEntity, &manager->m_sQueueSample.m_vecPos);
	distSquared = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
	if (distSquared < (FIRE_HYDRANT_MAX_DIST * FIRE_HYDRANT_MAX_DIST)) {
		AudioMath_CalculateDistance(&distCalculated, &manager->m_sQueueSample.m_fDistance, distSquared);
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(FIRE_HYDRANT_VOLUME, FIRE_HYDRANT_MAX_DIST, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 0;
			manager->m_sQueueSample.m_nSampleIndex = SFX_JUMBO_TAXI;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nPriority = 4;
			manager->m_sQueueSample.m_nFrequency = 15591;
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_GARAGE_HYDRANT_SET_EMITTING_VOLUME(FIRE_HYDRANT_VOLUME);
			AUDIO_GARAGE_HYDRANT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
			manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			manager->m_sQueueSample.m_MaxDistance = FIRE_HYDRANT_MAX_DIST;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 3;
			AUDIO_GARAGE_HYDRANT_SET_SOUND_REVERB(1);
			AUDIO_GARAGE_HYDRANT_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
		}
	}
}

//- rouz edit (ChatGPT)
