//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "sampman.h"

enum {
	RAIN_ON_VEHICLE_MAX_DIST = 22,
	RAIN_ON_VEHICLE_VOLUME = 30,
};

#ifndef GTA_PS2
#define AUDIO_VEHICLE_RAIN_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_RAIN_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_RAIN_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_RAIN_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_RAIN_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_RAIN_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_RAIN_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_RAIN_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_RAIN_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_RAIN_SET_SOUND_REVERB(b)
#endif


void
AudioVehicleRain_Process(cAudioManager *manager, cVehicleParams *params)
{
    // Retain weather eligibility, counter updates and request field ordering
	if (params->m_fDistance < (RAIN_ON_VEHICLE_MAX_DIST * RAIN_ON_VEHICLE_MAX_DIST) && AudioVehicleRainHost_Rain() > 0.01f && (!AudioVehicleRainHost_CameraNoRain() || !AudioVehicleRainHost_PlayerNoRain())) {
		// Capture the vehicle counters after all original eligibility checks
		CVehicle *veh = params->m_pVehicle;
		uint8_t *audioCounter, *sampleCounter;
		AudioVehicleRainHost_Counters(veh, &audioCounter, &sampleCounter);
		(*audioCounter)++;
		if ((*audioCounter) >= 2) {
			(*audioCounter) = 0;
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			uint8_t Vol = RAIN_ON_VEHICLE_VOLUME * AudioVehicleRainHost_Rain();
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, RAIN_ON_VEHICLE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = (*sampleCounter)++;
				if ((*sampleCounter) > 4)
					(*sampleCounter) = 68;
				manager->m_sQueueSample.m_nSampleIndex = (manager->m_anRandomTable[1] & 3) + SFX_CAR_RAIN_1;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 9;
				manager->m_sQueueSample.m_nFrequency = manager->m_anRandomTable[1] % 4000 + 28000;
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_VEHICLE_RAIN_SET_EMITTING_VOLUME(Vol);
				AUDIO_VEHICLE_RAIN_RESET_LOOP_OFFSETS
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = RAIN_ON_VEHICLE_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 1;
				AUDIO_VEHICLE_RAIN_SET_SOUND_REVERB(0);
				AUDIO_VEHICLE_RAIN_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
	}
}
//- rouz edit (ChatGPT)
