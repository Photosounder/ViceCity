//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../core/SurfaceTypes.h"
#include "../modelinfo/VehicleTypes.h"
#include "../vehicles/WheelStatus.h"
#include "sampman.h"
#include <math.h>
#include <stddef.h>

enum {
	FLAT_TYRE_MAX_DIST = 60,
	FLAT_TYRE_VOLUME = 100,
	VEHICLE_ROAD_NOISE_MAX_DIST = 95,
	VEHICLE_ROAD_NOISE_VOLUME = 30,
	WET_ROAD_NOISE_MAX_DIST = 30,
	WET_ROAD_NOISE_VOLUME = 23,
};

static uint8_t AudioVehicleRoad_Less(float left, float right)
{
    // Retain the conditional second transmission read when MSVC optimizes float minimum expressions
    volatile uint8_t result = left < right;
    return result;
}

#define AudioVehicleRoad_Min(a,b) (AudioVehicleRoad_Less((a), (b)) ? (a) : (b))
#ifndef GTA_PS2
#define AUDIO_VEHICLE_ROAD_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_ROAD_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_ROAD_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_ROAD_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_ROAD_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_ROAD_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_ROAD_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_ROAD_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_ROAD_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_ROAD_SET_SOUND_REVERB(b)
#endif


uint8_t
AudioVehicleRoad_FlatTyre(cAudioManager *manager, cVehicleParams *params)
{
    // Retain range gates, live vehicle reads and sound request ordering
	CVehicle *automobile;
	CVehicle *bike;
	uint8_t wheelBurst;
	uint8_t Vol;

	float modifier;

	if (params->m_fDistance < (FLAT_TYRE_MAX_DIST * FLAT_TYRE_MAX_DIST)) {
		switch (params->m_VehicleType) {
		case VEHICLE_TYPE_CAR:
			automobile = params->m_pVehicle;
			wheelBurst = 0;
			for (int i = 0; i < 4; i++)
				if (AudioVehicleRoadHost_WheelStatus(automobile, VEHICLE_TYPE_CAR, i) == WHEEL_STATUS_BURST && AudioVehicleRoadHost_WheelTimer(automobile, VEHICLE_TYPE_CAR, i) > 0.0f)
					wheelBurst = 1;
			if (!wheelBurst)
				return 1;
			break;
		case VEHICLE_TYPE_BIKE:
			bike = params->m_pVehicle;
			wheelBurst = 0;
			for(int i = 0; i < 2; i++)
				if (AudioVehicleRoadHost_WheelStatus(bike, VEHICLE_TYPE_BIKE, i) == WHEEL_STATUS_BURST && AudioVehicleRoadHost_WheelTimer(bike, VEHICLE_TYPE_BIKE, i) > 0.0f)
					wheelBurst = 1;
			if (!wheelBurst)
				return 1;
			break;
		default:
			return 1;
		}
		modifier = AudioVehicleRoad_Min(1.0f, fabsf(params->m_fVelocityChange) / (0.3f * AudioVehicleRoadHost_MaxVelocity(params->m_pTransmission)));
		if (modifier > 0.01f) {
			Vol = (FLAT_TYRE_VOLUME * modifier);
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, FLAT_TYRE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = 95;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 5;
				manager->m_sQueueSample.m_nSampleIndex = SFX_TYRE_BURST_L;
				manager->m_sQueueSample.m_nFrequency = (5500.0f * modifier) + 8000;
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_VEHICLE_ROAD_SET_EMITTING_VOLUME(Vol);
				AUDIO_VEHICLE_ROAD_SET_LOOP_OFFSETS(SFX_TYRE_BURST_L)
				manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
				manager->m_sQueueSample.m_MaxDistance = FLAT_TYRE_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 3;
				AUDIO_VEHICLE_ROAD_SET_SOUND_REVERB(1);
				AUDIO_VEHICLE_ROAD_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
		return 1;
	}
	return 0;
}

uint8_t
AudioVehicleRoad_Dry(cAudioManager *manager, cVehicleParams *params)
{
    // Retain range gates, live vehicle reads and sound request ordering
	uint8_t Vol;
	uint32_t freq;
	float multiplier;
	int sampleFreq;
	float velocity;
	uint8_t wheelsOnGround;

	if (params->m_fDistance < (VEHICLE_ROAD_NOISE_MAX_DIST * VEHICLE_ROAD_NOISE_MAX_DIST)) {
		switch (params->m_VehicleType) {
		case VEHICLE_TYPE_CAR:
			wheelsOnGround = AudioVehicleRoadHost_WheelsOnGround(params->m_pVehicle, VEHICLE_TYPE_CAR);
			break;
		case VEHICLE_TYPE_BIKE:
			wheelsOnGround = AudioVehicleRoadHost_WheelsOnGround(params->m_pVehicle, VEHICLE_TYPE_BIKE);
			break;
		default:
			wheelsOnGround = 4;
			break;
		}
		if ((params->m_pTransmission != NULL) && (wheelsOnGround > 0)) {
			velocity = fabsf(params->m_fVelocityChange);
			if (velocity > 0.0f) {
				AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
				Vol = VEHICLE_ROAD_NOISE_VOLUME * AudioVehicleRoad_Min(1.0f, velocity / (0.5f * AudioVehicleRoadHost_MaxVelocity(params->m_pTransmission)));
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, VEHICLE_ROAD_NOISE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_nCounter = 0;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nPriority = 3;
					if (AudioVehicleRoadHost_Surface(params->m_pVehicle) == SURFACE_WATER) {
						manager->m_sQueueSample.m_nSampleIndex = SFX_BOAT_WATER_LOOP;
						freq = 6050 * Vol / VEHICLE_ROAD_NOISE_VOLUME + 16000;
					} else {
						manager->m_sQueueSample.m_nSampleIndex = SFX_ROAD_NOISE;
						multiplier = (manager->m_sQueueSample.m_fDistance / VEHICLE_ROAD_NOISE_MAX_DIST) * 0.5f;
						sampleFreq = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_ROAD_NOISE);
						freq = (sampleFreq * multiplier) + ((3 * sampleFreq) >> 2);
					}
					manager->m_sQueueSample.m_nFrequency = freq;
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_VEHICLE_ROAD_SET_EMITTING_VOLUME(Vol);
					AUDIO_VEHICLE_ROAD_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_fSpeedMultiplier = 6.0f;
					manager->m_sQueueSample.m_MaxDistance = VEHICLE_ROAD_NOISE_MAX_DIST;
					manager->m_sQueueSample.m_bStatic = 0;
					manager->m_sQueueSample.m_nFramesToPlay = 4;
					AUDIO_VEHICLE_ROAD_SET_SOUND_REVERB(1);
					AUDIO_VEHICLE_ROAD_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
		}
		return 1;
	}
	return 0;
}

uint8_t
AudioVehicleRoad_Wet(cAudioManager *manager, cVehicleParams *params)
{
    // Retain range gates, live vehicle reads and sound request ordering
	float relativeVelocity;
	uint8_t Vol;
	float multiplier;
	int freq;
	float velChange;
	uint8_t wheelsOnGround;

	if (params->m_fDistance < (WET_ROAD_NOISE_MAX_DIST * WET_ROAD_NOISE_MAX_DIST)) {
		switch (params->m_VehicleType) {
		case VEHICLE_TYPE_CAR:
			wheelsOnGround = AudioVehicleRoadHost_WheelsOnGround(params->m_pVehicle, VEHICLE_TYPE_CAR);
			break;
		case VEHICLE_TYPE_BIKE:
			wheelsOnGround = AudioVehicleRoadHost_WheelsOnGround(params->m_pVehicle, VEHICLE_TYPE_BIKE);
			break;
		default:
			wheelsOnGround = 4;
			break;
		}
		if ((params->m_pTransmission != NULL) && (wheelsOnGround > 0)) {
			velChange = fabsf(params->m_fVelocityChange);
			if (velChange > 0.0f) {
				AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
				relativeVelocity = AudioVehicleRoad_Min(1.0f, velChange / (0.5f * AudioVehicleRoadHost_MaxVelocity(params->m_pTransmission)));
				Vol = WET_ROAD_NOISE_VOLUME * relativeVelocity * AudioVehicleRoadHost_WetRoads();
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, WET_ROAD_NOISE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_nCounter = 1;
					manager->m_sQueueSample.m_nSampleIndex = SFX_ROAD_NOISE;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nPriority = 3;
					multiplier = (manager->m_sQueueSample.m_fDistance / WET_ROAD_NOISE_MAX_DIST) * 0.5f;
					freq = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_ROAD_NOISE);
					manager->m_sQueueSample.m_nFrequency = freq + freq * multiplier;
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_VEHICLE_ROAD_SET_EMITTING_VOLUME(Vol);
					AUDIO_VEHICLE_ROAD_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_fSpeedMultiplier = 6.0f;
					manager->m_sQueueSample.m_MaxDistance = WET_ROAD_NOISE_MAX_DIST;
					manager->m_sQueueSample.m_bStatic = 0;
					manager->m_sQueueSample.m_nFramesToPlay = 4;
					AUDIO_VEHICLE_ROAD_SET_SOUND_REVERB(1);
					AUDIO_VEHICLE_ROAD_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
		}
		return 1;
	}
	return 0;
}

//- rouz edit (ChatGPT)
