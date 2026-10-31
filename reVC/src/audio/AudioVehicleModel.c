//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioPoliceGame.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "sampman.h"
#include <math.h>
#include <stddef.h>

enum {
	MODEL_CAR_ENGINE_MAX_DIST = 35,
	MODEL_CAR_ENGINE_VOLUME = 90,
	MODEL_HELI_ENGINE_VOLUME = 70,
};

static uint8_t AudioVehicleModel_Decision(uint8_t result)
{
    // Preserve conditional repeated pad queries when MSVC recognizes a floating maximum
    volatile uint8_t comparison = result;
    return comparison;
}

#ifdef THIS_IS_STUPID
static float AudioVehicleModel_UpDot(CVehicle *vehicle)
{
    // Retain the original alternate dot-product terms instead of replacing them with Y
    AudioSoundPosition up;
    AudioVehicleModelHost_Up(vehicle, &up);
    return up.x * 0.0f + up.y * 1.0f + up.z * 0.0f;
}
#endif

#define AudioVehicleModel_Min(a,b) ((a) < (b) ? (a) : (b))
#define AudioVehicleModel_Max(a,b) ((a) > (b) ? (a) : (b))
#define AudioVehicleModel_Clamp2(v,center,radius) ((v) > (center) ? AudioVehicleModel_Min(v, (center) + (radius)) : AudioVehicleModel_Max(v, (center) - (radius)))
#define AudioVehicleModel_InputMax(a,b) (AudioVehicleModel_Decision((a) > (b)) ? (a) : (b))
#ifndef GTA_PS2
#define AUDIO_VEHICLE_MODEL_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_MODEL_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_MODEL_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_MODEL_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_MODEL_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_MODEL_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_MODEL_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_MODEL_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_MODEL_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_MODEL_SET_SOUND_REVERB(b)
#endif


void
AudioVehicleModel_Heli(cAudioManager *manager, cVehicleParams *params)
{
    // Retain shared history, player control reads and sound request ordering
	static uint32_t prevFreq = 22050;

	uint32_t freq;
	uint8_t isPlayerVeh;
	int16_t acceletateState;
	int16_t brakeState;

	if (params->m_fDistance < (MODEL_CAR_ENGINE_MAX_DIST * MODEL_CAR_ENGINE_MAX_DIST)) {
		if (AudioPoliceHost_PlayerVehicle() == params->m_pVehicle)
			isPlayerVeh = 1;
		else
	#ifdef FIX_BUGS
			isPlayerVeh = AudioEnvironmentHost_Remote() == params->m_pVehicle;
	#else
			isPlayerVeh = AudioEnvironmentHost_Remote() != NULL;
	#endif
		if (isPlayerVeh) {
			brakeState = AudioVehicleModelHost_Brake();
			acceletateState = AudioVehicleModel_InputMax(AudioVehicleModelHost_Accelerate(), fabsf((float)AudioVehicleModelHost_GunUpDown()) * 2);
		} else {
			acceletateState = 255.0f * AudioVehicleReverseHost_Gas(params->m_pVehicle);
			brakeState = 255.0f * AudioVehicleModelHost_BrakePedal(params->m_pVehicle);
		}
		if (acceletateState < brakeState)
			acceletateState = brakeState;
		freq = AudioVehicleModel_Clamp2(5 * acceletateState + 22050, (int)prevFreq, 30);
		AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(MODEL_HELI_ENGINE_VOLUME, MODEL_CAR_ENGINE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 2;
			manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_RC_HELI;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nPriority = 3;
			manager->m_sQueueSample.m_nFrequency = freq;
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_VEHICLE_MODEL_SET_EMITTING_VOLUME(MODEL_HELI_ENGINE_VOLUME);
			AUDIO_VEHICLE_MODEL_SET_LOOP_OFFSETS(SFX_CAR_RC_HELI)
			manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
			manager->m_sQueueSample.m_MaxDistance = MODEL_CAR_ENGINE_MAX_DIST;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 4;
			AUDIO_VEHICLE_MODEL_SET_SOUND_REVERB(1);
			AUDIO_VEHICLE_MODEL_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
		}
		if (isPlayerVeh)
			prevFreq = freq;
	}
}

void
AudioVehicleModel_Car(cAudioManager *manager, cVehicleParams *params)
{
    // Retain shared history, player control reads and sound request ordering
	static uint32_t prevFreq = 14000;
	static uint8_t LastVol = 0;

	uint32_t freq;
	int16_t acceletateState;
	int16_t brakeState;
	uint8_t Vol;
	uint8_t isPlayerVeh;
	uint8_t vehSlowdown;

	if (params->m_fDistance < (MODEL_CAR_ENGINE_MAX_DIST * MODEL_CAR_ENGINE_MAX_DIST)) {
		if (AudioPoliceHost_PlayerVehicle() == params->m_pVehicle)
			isPlayerVeh = 1;
		else
	#ifdef FIX_BUGS
			isPlayerVeh = AudioEnvironmentHost_Remote() == params->m_pVehicle;
	#else
			isPlayerVeh = AudioEnvironmentHost_Remote() != NULL;
	#endif
		if (AudioVehicleModelHost_Model(params->m_pVehicle) == MI_RCBANDIT) {
			if (AudioVehicleReverseHost_DriveWheels(params->m_pVehicle) != 0) {
				Vol = AudioVehicleModel_Min(127, 127.0f * fabsf(params->m_fVelocityChange) * 3.0f);
				freq = 8000.0f * fabsf(params->m_fVelocityChange) + 14000;
			} else {
				Vol = 127;
				freq = 25000;
			}
			if (isPlayerVeh) {
				Vol = AudioVehicleModel_Clamp2(Vol, LastVol, 7);
				freq = AudioVehicleModel_Clamp2(freq, prevFreq, 800);
			}
			if (Vol > 0) {
				AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, MODEL_CAR_ENGINE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_nCounter = 2;
					manager->m_sQueueSample.m_nSampleIndex = SFX_RC_REV;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nPriority = 1;
					manager->m_sQueueSample.m_nFrequency = freq;
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_VEHICLE_MODEL_SET_EMITTING_VOLUME(Vol);
					AUDIO_VEHICLE_MODEL_SET_LOOP_OFFSETS(SFX_RC_REV)
					manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
					manager->m_sQueueSample.m_MaxDistance = MODEL_CAR_ENGINE_MAX_DIST;
					manager->m_sQueueSample.m_bStatic = 0;
					manager->m_sQueueSample.m_nFramesToPlay = 4;
					AUDIO_VEHICLE_MODEL_SET_SOUND_REVERB(1);
					AUDIO_VEHICLE_MODEL_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
			if (isPlayerVeh) {
				prevFreq = freq;
				LastVol = Vol;
			}
		} else if (params->m_pVehicle != NULL) {
			if (isPlayerVeh) {
				acceletateState = AudioVehicleModelHost_Accelerate();
				brakeState = AudioVehicleModelHost_Brake();
			} else {
				acceletateState = 255.0f * AudioVehicleReverseHost_Gas(params->m_pVehicle);
				brakeState = 255.0f * AudioVehicleModelHost_BrakePedal(params->m_pVehicle);
			}
			if (acceletateState < brakeState)
				acceletateState = brakeState;
			if (acceletateState <= 0) {
				vehSlowdown = 1;
				Vol = 127;
				freq = 18000;
			} else {
				vehSlowdown = 0;
				Vol = AudioVehicleModel_Min(127, (127 * acceletateState / 255) * 3.0f * fabsf(params->m_fVelocityChange));
				freq = AudioVehicleModel_Min(22000, (8000 * acceletateState / 255 + 14000) * 3.0f * fabsf(params->m_fVelocityChange));
			}
			if (isPlayerVeh && !vehSlowdown) {
				Vol = AudioVehicleModel_Clamp2(Vol, LastVol, 7);
				freq = AudioVehicleModel_Clamp2(freq, prevFreq, 800);
			}
			if (!vehSlowdown)
	#ifdef THIS_IS_STUPID
				freq += 8000.0f * fabsf(AudioVehicleModel_UpDot(params->m_pVehicle));
	#else
				freq += 8000.0f * fabsf(AudioVehicleModelHost_UpY(params->m_pVehicle));
	#endif
			if (AudioVehicleModelHost_Drowning(params->m_pVehicle))
				Vol >>= 2;
			if (Vol > 0) {
				AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, MODEL_CAR_ENGINE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					if (vehSlowdown) {
						manager->m_sQueueSample.m_nCounter = 0;
						manager->m_sQueueSample.m_nSampleIndex = SFX_RC_IDLE;
						manager->m_sQueueSample.m_nFramesToPlay = 6;
					} else {
						manager->m_sQueueSample.m_nCounter = 2;
						manager->m_sQueueSample.m_nSampleIndex = SFX_RC_REV;
						manager->m_sQueueSample.m_nFramesToPlay = 4;
					}
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nPriority = 3;
					manager->m_sQueueSample.m_nFrequency = freq;
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_VEHICLE_MODEL_SET_EMITTING_VOLUME(Vol);
					AUDIO_VEHICLE_MODEL_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
					manager->m_sQueueSample.m_MaxDistance = MODEL_CAR_ENGINE_MAX_DIST;
					manager->m_sQueueSample.m_bStatic = 0;
					AUDIO_VEHICLE_MODEL_SET_SOUND_REVERB(1);
					AUDIO_VEHICLE_MODEL_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
			if (isPlayerVeh) {
				if (vehSlowdown) {
					prevFreq = freq;
					LastVol = Vol;
				}
			}
		}
	}
}

//- rouz edit (ChatGPT)
