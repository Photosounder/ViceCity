//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "../entities/EntityStatus.h"
#include "AudioPoliceGame.h"
#include <math.h>
#include "sampman.h"

enum {
	BOAT_ENGINE_MAX_DIST = 90,
	BOAT_ENGINE_REEFER_IDLE_VOLUME = 80,
	BOAT_MOVING_OVER_WATER_MAX_DIST = 50,
	BOAT_MOVING_OVER_WATER_VOLUME = 30,
};
#ifndef GTA_PS2
#define AUDIO_BOAT_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_BOAT_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_BOAT_RESET_LOOP_OFFSETS
#define AUDIO_BOAT_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_BOAT_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_BOAT_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_BOAT_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_BOAT_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_BOAT_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_BOAT_SET_SOUND_REVERB(b)
#endif


static uint8_t AudioBoat_Decision(uint8_t result)
{
	// Preserve conditional repeated throttle queries across compiler optimizations
	volatile uint8_t comparison = result;
	return comparison;
}

#define AUDIO_BOAT_MIN(a,b) ((a) < (b) ? (a) : (b))
#define AUDIO_BOAT_MAX(a,b) (AudioBoat_Decision((a) > (b)) ? (a) : (b))

uint8_t
AudioBoat_Engine(cAudioManager *manager, cVehicleParams *params)
{
	// Generate engine and Reefer idle loops with shared player smoothing history
	CVehicle *boat;
	float padRelativeAccerate;

	uint8_t isV12 = 0;
	static uint32_t LastFreq = 2000;
	static uint8_t LastVol = 0;

	if (params->m_fDistance < (BOAT_ENGINE_MAX_DIST * BOAT_ENGINE_MAX_DIST)) {
		boat = params->m_pVehicle;
		if(AudioVehicleEngineHost_Status(boat) == STATUS_WRECKED)
			return 1;

		float freqModificator;
		float volModificator;
		uint8_t BaseVol;
		uint32_t BaseFreq;

		switch(AudioVehicleReverseHost_Model(boat)) {
		case MI_RIO:
			freqModificator = 490;
			volModificator = 60;
			BaseVol = 20;
			BaseFreq = 1888;
			break;
		case MI_PREDATOR:
		case MI_SQUALO:
		case MI_SPEEDER:
		case MI_COASTG:
		case MI_DINGHY:
		case MI_JETMAX:
			freqModificator = 6000;
			volModificator = 60;
			isV12 = 1;
			BaseFreq = 9000;
			BaseVol = 20;
			break;
		case MI_REEFER:
			freqModificator = 715;
			volModificator = 80;
			BaseVol = 0;
			BaseFreq = 3775;
			break;
		case MI_TROPIC:
		case MI_MARQUIS:
			freqModificator = 463;
			volModificator = 60;
			BaseVol = 20;
			BaseFreq = 1782;
			break;
		default:
			return 1;
		}

		uint8_t bIsPlayerVeh;

		if(AudioPoliceHost_PlayerVehicle() == params->m_pVehicle) {
			padRelativeAccerate = AUDIO_BOAT_MAX(AudioVehicleModelHost_Accelerate(), AudioVehicleModelHost_Brake()) / 255.0f;
			bIsPlayerVeh = 1;
		} else {
			padRelativeAccerate = AUDIO_BOAT_MAX(AudioVehicleReverseHost_Gas(params->m_pVehicle), AudioVehicleModelHost_BrakePedal(params->m_pVehicle));
			bIsPlayerVeh = 0;
		}

		uint32_t Freq = BaseFreq + (padRelativeAccerate * freqModificator);
		uint8_t Vol = BaseVol + (padRelativeAccerate * volModificator);

		if(!AudioBoatHost_Propeller(boat))
			Freq = (9 * Freq) / 8;

		if(bIsPlayerVeh) {
			if(Freq > LastFreq) {
				if(isV12)
					Freq = AUDIO_BOAT_MIN(Freq, LastFreq + 100);
				else
					Freq = AUDIO_BOAT_MIN(Freq, LastFreq + 15);
			} else {
				if(isV12)
					Freq = AUDIO_BOAT_MAX(Freq, LastFreq - 100);
				else
					Freq = AUDIO_BOAT_MAX(Freq, LastFreq - 15);
			}
			if(Vol > LastVol)
				Vol = AUDIO_BOAT_MIN(Vol, LastVol + 3);
			else
				Vol = AUDIO_BOAT_MAX(Vol, LastVol - 3);
		}

		if (Vol > 0) {
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, BOAT_ENGINE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nFrequency = Freq;
				manager->m_sQueueSample.m_nCounter = 40;
				if (isV12)
					manager->m_sQueueSample.m_nSampleIndex = SFX_BOAT_V12_LOOP;
				else
					manager->m_sQueueSample.m_nSampleIndex = SFX_BOAT_CRUISER_LOOP;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_BOAT_SET_EMITTING_VOLUME(Vol);
				AUDIO_BOAT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
				manager->m_sQueueSample.m_MaxDistance = BOAT_ENGINE_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 7;
				AUDIO_BOAT_SET_SOUND_REVERB(1);
				AUDIO_BOAT_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}

		if(AudioVehicleReverseHost_Model(boat) == MI_REEFER) {
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(BOAT_ENGINE_REEFER_IDLE_VOLUME, BOAT_ENGINE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nFrequency = 6000;
				manager->m_sQueueSample.m_nCounter = 39;
				manager->m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE;
				manager->m_sQueueSample.m_nFrequency += (manager->m_sQueueSample.m_nEntityIndex * 65536) % 1000;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_BOAT_SET_EMITTING_VOLUME(BOAT_ENGINE_REEFER_IDLE_VOLUME);
				AUDIO_BOAT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
				manager->m_sQueueSample.m_MaxDistance = BOAT_ENGINE_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 7;
				AUDIO_BOAT_SET_SOUND_REVERB(1);
				AUDIO_BOAT_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
		if(bIsPlayerVeh) {
			LastFreq = Freq;
			LastVol = Vol;
		}
		return 1;
	}
	return 0;
}

uint8_t
AudioBoat_Water(cAudioManager *manager, cVehicleParams *params)
{
	// Generate the water loop from the live speed and boat-in-water flag
	float velocityChange;
	uint8_t Vol;
	float multiplier;

	if (params->m_fDistance < (BOAT_MOVING_OVER_WATER_MAX_DIST * BOAT_MOVING_OVER_WATER_MAX_DIST)) {
		velocityChange = fabsf(params->m_fVelocityChange);
		if (velocityChange > 0.0005f && AudioBoatHost_InWater(params->m_pVehicle)) {
			velocityChange = AUDIO_BOAT_MIN(0.75f, velocityChange);
			multiplier = (velocityChange - 0.0005f) / (1499.0f / 2000.0f);
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			Vol = (BOAT_MOVING_OVER_WATER_VOLUME * multiplier);
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, BOAT_MOVING_OVER_WATER_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = 38;
				manager->m_sQueueSample.m_nSampleIndex = SFX_BOAT_WATER_LOOP;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_nFrequency = (6050 * multiplier) + 16000;
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_BOAT_SET_EMITTING_VOLUME(Vol);
				AUDIO_BOAT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
				manager->m_sQueueSample.m_MaxDistance = BOAT_MOVING_OVER_WATER_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 6;
				AUDIO_BOAT_SET_SOUND_REVERB(1);
				AUDIO_BOAT_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
		return 1;
	}
	return 0;
}

//- rouz edit (ChatGPT)
