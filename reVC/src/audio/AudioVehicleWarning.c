//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "AudioVehicleModels.h"
#include "sampman.h"

enum {
	VEHICLE_REVERSE_WARNING_MAX_DIST = 50,
	VEHICLE_REVERSE_WARNING_VOLUME = 60,
	AIR_BRAKES_MAX_DIST = 30,
	AIR_BRAKES_VOLUME = 70,
};
#ifndef GTA_PS2
#define AUDIO_VEHICLE_WARNING_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_WARNING_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_WARNING_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_WARNING_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_WARNING_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_WARNING_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_WARNING_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_WARNING_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_WARNING_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_WARNING_SET_SOUND_REVERB(b)
#endif


uint8_t
AudioVehicleWarning_UsesReverse(uint32_t model)
{
	// Select the original set of vehicles with reversing warnings
	return model == LINERUN || model == FIRETRUK || model == BUS || model == COACH || model == PACKER || model == FLATBED;
}

uint8_t
AudioVehicleWarning_Reverse(cAudioManager *manager, cVehicleParams *params)
{
	// Capture the warning vehicle before testing distance and engine state
	CVehicle *veh = params->m_pVehicle;

	if (params->m_fDistance < (VEHICLE_REVERSE_WARNING_MAX_DIST * VEHICLE_REVERSE_WARNING_MAX_DIST)) {
		if (AudioVehicleReverseHost_Engine(veh) && AudioVehicleReverseHost_Gas(veh) < 0.0f) {
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(AudioVehicleModelHost_Drowning(veh) ? VEHICLE_REVERSE_WARNING_VOLUME / 4 : VEHICLE_REVERSE_WARNING_VOLUME, VEHICLE_REVERSE_WARNING_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = 12;
				manager->m_sQueueSample.m_nSampleIndex = SFX_REVERSE_WARNING;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 2;
				manager->m_sQueueSample.m_nFrequency = (100 * manager->m_sQueueSample.m_nEntityIndex % 1024) + SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_REVERSE_WARNING);
				manager->m_sQueueSample.m_nLoopCount = 0;
#ifdef FIX_BUGS
				AUDIO_VEHICLE_WARNING_SET_EMITTING_VOLUME(AudioVehicleModelHost_Drowning(veh) ? VEHICLE_REVERSE_WARNING_VOLUME / 4 : VEHICLE_REVERSE_WARNING_VOLUME);
#else
				AUDIO_VEHICLE_WARNING_SET_EMITTING_VOLUME(VEHICLE_REVERSE_WARNING_VOLUME);
#endif
				AUDIO_VEHICLE_WARNING_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
				manager->m_sQueueSample.m_MaxDistance = VEHICLE_REVERSE_WARNING_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 3;
				AUDIO_VEHICLE_WARNING_SET_SOUND_REVERB(1);
				AUDIO_VEHICLE_WARNING_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
		return 1;
	}
	return 0;
}

uint8_t
AudioVehicleWarning_AirBrakes(cAudioManager *manager, cVehicleParams *params)
{
	// Emit the braking sample when the original velocity thresholds are crossed
	CVehicle *automobile;
	uint8_t Vol;

	if (params->m_fDistance < (AIR_BRAKES_MAX_DIST * AIR_BRAKES_MAX_DIST)) {
		automobile = params->m_pVehicle;
		if (AudioVehicleReverseHost_Engine(automobile) && (AudioPlayerEngineHost_CarVelocity(automobile) >= 0.025f && params->m_fVelocityChange < 0.025f ||
			AudioPlayerEngineHost_CarVelocity(automobile) <= -0.025f && params->m_fVelocityChange > 0.025f)) {
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			Vol = manager->m_anRandomTable[0] % 10 + AIR_BRAKES_VOLUME;
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, AIR_BRAKES_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = 13;
				manager->m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_AIR_BRAKES);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 10;
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_VEHICLE_WARNING_SET_EMITTING_VOLUME(Vol);
				AUDIO_VEHICLE_WARNING_RESET_LOOP_OFFSETS
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = AIR_BRAKES_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 1;
				AUDIO_VEHICLE_WARNING_SET_SOUND_REVERB(1);
				AUDIO_VEHICLE_WARNING_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
		return 1;
	}
	return 0;
}

uint8_t
AudioVehicleWarning_HasAirBrakes(uint32_t model)
{
	// Select the original set of vehicles with air brakes
	return model == LINERUN || model == FIRETRUK || model == TRASH || model == BUS || model == BARRACKS 
		|| model == COACH || model == PACKER || model == FLATBED;
}

//- rouz edit (ChatGPT)
