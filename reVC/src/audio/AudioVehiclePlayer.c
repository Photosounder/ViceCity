//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../modelinfo/VehicleTypes.h"
#include "sampman.h"
#include <math.h>

enum { VEHICLE_ENGINE_MAX_DIST = 50 };
#define AudioVehiclePlayer_Min(a,b) ((a) < (b) ? (a) : (b))
#define AudioVehiclePlayer_Max(a,b) ((a) > (b) ? (a) : (b))
#ifndef GTA_PS2
#define AUDIO_VEHICLE_PLAYER_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_PLAYER_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_PLAYER_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_PLAYER_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_PLAYER_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_PLAYER_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_PLAYER_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_PLAYER_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_PLAYER_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_PLAYER_SET_SOUND_REVERB(b)
#endif


void
AudioVehiclePlayer_UpdateGas(CVehicle *veh, int32_t vehType)
{
    // Preserve the original smoothing or player sample operation with explicit C access
	float gasPedal = fabsf(AudioVehicleReverseHost_Gas(veh));
	float* gasPealAudioPtr;

	switch(vehType) {
	case VEHICLE_TYPE_CAR: gasPealAudioPtr = AudioVehicleReverseHost_GasAudio(veh); break;
	case VEHICLE_TYPE_BIKE: gasPealAudioPtr = AudioVehicleEngineHost_BikeGasAudio(veh); break;
	default: return;
	}
	if (*gasPealAudioPtr < gasPedal)
		*gasPealAudioPtr = AudioVehiclePlayer_Min(*gasPealAudioPtr + 0.09f, gasPedal);
	else
		*gasPealAudioPtr = AudioVehiclePlayer_Max(*gasPealAudioPtr - 0.07f, gasPedal);
}

void
AudioVehiclePlayer_AddSample(cAudioManager *manager, uint8_t Vol, uint32_t freq, uint32_t sample, uint8_t bank, uint8_t counter, uint8_t bLooping)
{
    // Preserve the original smoothing or player sample operation with explicit C access
	manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, VEHICLE_ENGINE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
	if (manager->m_sQueueSample.m_nVolume > 0) {
		manager->m_sQueueSample.m_nCounter = counter;
		manager->m_sQueueSample.m_nSampleIndex = sample;
#ifdef GTA_PS2
		manager->m_sQueueSample.m_nBankIndex = bank;
#else
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#endif // GTA_PS2
		manager->m_sQueueSample.m_bIs2D = 0;
		manager->m_sQueueSample.m_nPriority = 0;
		manager->m_sQueueSample.m_nFrequency = freq;
		if (bLooping) {
			manager->m_sQueueSample.m_nLoopCount = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 8;
		} else
			manager->m_sQueueSample.m_nLoopCount = 1;
		AUDIO_VEHICLE_PLAYER_SET_EMITTING_VOLUME(Vol);
		AUDIO_VEHICLE_PLAYER_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
		manager->m_sQueueSample.m_fSpeedMultiplier = 6.0f;
		manager->m_sQueueSample.m_MaxDistance = VEHICLE_ENGINE_MAX_DIST;
		manager->m_sQueueSample.m_bStatic = 0;
		AUDIO_VEHICLE_PLAYER_SET_SOUND_REVERB(1);
		AUDIO_VEHICLE_PLAYER_SET_SOUND_REFLECTION(0);
		AudioRequests_Submit(manager);
	}
}

//- rouz edit (ChatGPT)
