//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "../modelinfo/VehicleTypes.h"
#include "../entities/EntityStatus.h"
#include "../vehicles/VehicleBombTypes.h"
#include "sampman.h"

enum {
	ENGINE_DAMAGE_MAX_DIST = 40,
	CAR_BOMB_TICK_MAX_DIST = 40,
	CAR_BOMB_TICK_VOLUME = 60,
};
#ifndef GTA_PS2
#define AUDIO_VEHICLE_CONDITION_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_CONDITION_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_CONDITION_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_CONDITION_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_CONDITION_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_CONDITION_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_CONDITION_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_CONDITION_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_CONDITION_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_CONDITION_SET_SOUND_REVERB(b)
#endif


uint8_t
AudioVehicleCondition_Damage(cAudioManager *manager, cVehicleParams *params)
{
	// Select fire or smoke audio from the live vehicle model, status and health
	uint8_t Vol;

	if (params->m_fDistance < (ENGINE_DAMAGE_MAX_DIST * ENGINE_DAMAGE_MAX_DIST)) {
		if (AudioVehicleReverseHost_Model(params->m_pVehicle) == MI_CADDY)
			return 1;
		if (AudioVehicleEngineHost_Status(params->m_pVehicle) == STATUS_WRECKED || AudioVehicleConditionHost_Health(params->m_pVehicle) >= 390)
			return 1;
		if (AudioVehicleConditionHost_Health(params->m_pVehicle) < 250.0f) {
			Vol = 60;
			manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_ON_FIRE;
			manager->m_sQueueSample.m_nPriority = 7;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_CAR_ON_FIRE);
		} else {
			Vol = 30;
			manager->m_sQueueSample.m_nSampleIndex = SFX_PALM_TREE_LO;
			manager->m_sQueueSample.m_nPriority = 7;
			manager->m_sQueueSample.m_nFrequency = 27000;
		}
		AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
		if (AudioVehicleModelHost_Drowning(params->m_pVehicle))
			Vol >>= 2;
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, ENGINE_DAMAGE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 28;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_VEHICLE_CONDITION_SET_EMITTING_VOLUME(Vol);
			AUDIO_VEHICLE_CONDITION_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
			manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			manager->m_sQueueSample.m_MaxDistance = ENGINE_DAMAGE_MAX_DIST;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 3;
			AUDIO_VEHICLE_CONDITION_SET_SOUND_REVERB(1);
			AUDIO_VEHICLE_CONDITION_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
		}
		return 1;
	}
	return 0;
}

uint8_t
AudioVehicleCondition_BombTick(cAudioManager *manager, cVehicleParams *params)
{
	// Emit the countdown loop for an active timed bomb in a supported vehicle
	uint8_t bombType;

	if (params->m_fDistance < (CAR_BOMB_TICK_MAX_DIST * CAR_BOMB_TICK_MAX_DIST)) {
		if (AudioVehicleReverseHost_Engine(params->m_pVehicle)) {
			switch (params->m_VehicleType) {
			case VEHICLE_TYPE_CAR:
				bombType = AudioVehicleConditionHost_Bomb(params->m_pVehicle);
				break;
			case VEHICLE_TYPE_BIKE:
				bombType = AudioVehicleConditionHost_Bomb(params->m_pVehicle);
				break;
			default:
				AudioVehicleConditionHost_Unknown(params->m_VehicleType);
				return 1;
			}
			if (bombType == CARBOMB_TIMEDACTIVE) {
				AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(CAR_BOMB_TICK_VOLUME, CAR_BOMB_TICK_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_nCounter = 35;
					manager->m_sQueueSample.m_nSampleIndex = SFX_COUNTDOWN;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nPriority = 0;
					manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_COUNTDOWN);
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_VEHICLE_CONDITION_SET_EMITTING_VOLUME(CAR_BOMB_TICK_VOLUME);
					AUDIO_VEHICLE_CONDITION_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
					manager->m_sQueueSample.m_MaxDistance = CAR_BOMB_TICK_MAX_DIST;
					manager->m_sQueueSample.m_bStatic = 0;
					manager->m_sQueueSample.m_nFramesToPlay = 3;
					AUDIO_VEHICLE_CONDITION_SET_SOUND_REVERB(1);
					AUDIO_VEHICLE_CONDITION_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
		}
		return 1;
	}
	return 0;
}

//- rouz edit (ChatGPT)
