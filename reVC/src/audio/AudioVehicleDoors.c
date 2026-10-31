//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../vehicles/DoorTypes.h"
#include "sampman.h"
#include <math.h>

#define AUDIO_VEHICLE_DOORS_MIN(a,b) ((a) < (b) ? (a) : (b))

enum {
	VEHICLE_DOORS_MAX_DIST = 40,
	VEHICLE_DOORS_VOLUME = 100,
};
#ifndef GTA_PS2
#define AUDIO_VEHICLE_DOORS_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_DOORS_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_DOORS_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_DOORS_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_DOORS_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_DOORS_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_DOORS_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_DOORS_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_DOORS_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_DOORS_SET_SOUND_REVERB(b)
#endif


uint8_t
AudioVehicleDoors_Process(cAudioManager *manager, cVehicleParams *params)
{
	// Preserve the complete per-door sound operation and reflection submission
	CVehicle *automobile;
	int8_t doorState;
	uint8_t Vol;
	float velocity;

	if (params->m_fDistance < (VEHICLE_DOORS_MAX_DIST * VEHICLE_DOORS_MAX_DIST)) {
		automobile = params->m_pVehicle;
		AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
		for (uint8_t i = 0; i < VEHICLE_DOOR_COUNT; i++) {
			if (AudioVehicleDoorsHost_Status(automobile, i) == DOOR_STATUS_SWINGING) {
				doorState = AudioVehicleDoorsHost_State(automobile, i);
				if (doorState == DOORST_OPEN || doorState == DOORST_CLOSED) {
					velocity = AUDIO_VEHICLE_DOORS_MIN(0.3f, fabsf(AudioVehicleDoorsHost_Velocity(automobile, i)));
					if (velocity > 0.0035f) {
						Vol = (VEHICLE_DOORS_VOLUME * velocity / 0.3f);
						manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, VEHICLE_DOORS_MAX_DIST, manager->m_sQueueSample.m_fDistance);
						if (manager->m_sQueueSample.m_nVolume > 0) {
							manager->m_sQueueSample.m_nCounter = i + 6;
							manager->m_sQueueSample.m_nSampleIndex = manager->m_anRandomTable[1] % 6 + SFX_COL_CAR_PANEL_1;
							manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex) + AudioMath_RandomDisplacement(manager->m_anRandomTable, 1000);
							manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
							manager->m_sQueueSample.m_bIs2D = 0;
							manager->m_sQueueSample.m_nPriority = 10;
							manager->m_sQueueSample.m_nLoopCount = 1;
							AUDIO_VEHICLE_DOORS_SET_EMITTING_VOLUME(Vol);
							AUDIO_VEHICLE_DOORS_RESET_LOOP_OFFSETS
							manager->m_sQueueSample.m_fSpeedMultiplier = 1.0f;
							manager->m_sQueueSample.m_MaxDistance = VEHICLE_DOORS_MAX_DIST;
							manager->m_sQueueSample.m_bStatic = 1;
							AUDIO_VEHICLE_DOORS_SET_SOUND_REVERB(1);
							AUDIO_VEHICLE_DOORS_SET_SOUND_REFLECTION(1);
							AudioRequests_Submit(manager);
						}
					}
				}
			}
		}
		return 1;
	}
	return 0;
}

//- rouz edit (ChatGPT)
