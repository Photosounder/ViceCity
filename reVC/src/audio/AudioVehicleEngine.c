//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioPoliceGame.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "../modelinfo/VehicleTypes.h"
#include "../entities/EntityStatus.h"
#include "../vehicles/WheelIndices.h"
#include "sampman.h"
#include <math.h>
#include <stddef.h>

#ifndef GTA_PS2
#define CHANNEL_PLAYER_VEHICLE_ENGINE manager->m_nActiveSamples
#endif

static uint8_t AudioVehicleEngine_Decision(uint8_t result)
{
    // Retain repeated transmission reads in floating minimum expressions
    volatile uint8_t comparison = result;
    return comparison;
}
#define AudioVehicleEngine_Min(a,b) (AudioVehicleEngine_Decision((a) < (b)) ? (a) : (b))

enum {
	VEHICLE_ENGINE_MAX_DIST = 50,
	VEHICLE_ENGINE_BASE_VOLUME = 75,
	VEHICLE_ENGINE_FULL_VOLUME = 90,
};

#ifndef GTA_PS2
#define AUDIO_VEHICLE_ENGINE_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_ENGINE_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_ENGINE_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_ENGINE_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_ENGINE_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_ENGINE_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_ENGINE_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_ENGINE_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_ENGINE_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_ENGINE_SET_SOUND_REVERB(b)
#endif


uint8_t
AudioVehicleEngine_Process(cAudioManager *manager, cVehicleParams *params)
{
    // Preserve engine selection, traction arithmetic and queued sound fields
	CVehicle *automobile;
	float relativeGearChange;
#ifdef FIX_BUGS
	uint32_t freq = 0; // uninitialized variable
#else
	uint32_t freq;
#endif
	uint8_t Vol;
	cTransmission *transmission;
	uint8_t currentGear;
	float modificator;
	uint8_t wheelsOnGround;
	uint8_t wheelsOnGroundPrev;
	CVehicle *bike;
	enum tWheelState *wheelState;
	float *gasPedalAudioPtr;

	uint8_t isMoped = 0;
	uint8_t isGolfCart = 0;
	float traction = 0.0f;
	if (params->m_fDistance < (VEHICLE_ENGINE_MAX_DIST * VEHICLE_ENGINE_MAX_DIST)) {
		if (AudioPoliceHost_PlayerVehicle() == params->m_pVehicle && AudioVehicleEngineHost_Status(params->m_pVehicle) == STATUS_WRECKED) {
			SampleManager_StopChannel(&SampleManager, CHANNEL_PLAYER_VEHICLE_ENGINE);
			return 1;
		}
		if (AudioVehicleReverseHost_Engine(params->m_pVehicle)) {
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			if (AudioPoliceHost_PlayerVehicle() == params->m_pVehicle && AudioVehicleReverseHost_Model(params->m_pVehicle) != MI_CADDY)
				AudioPlayerEngine_Process(manager, params, params->m_pVehicle);
			else {
				transmission = params->m_pTransmission;
				if (transmission != NULL) {
					switch (AudioVehicleReverseHost_Model(params->m_pVehicle)) {
					case MI_PIZZABOY:
					case MI_FAGGIO:
						isMoped = 1;
						currentGear = (transmission->nNumberOfGears);
						break;
					case MI_CADDY:
						currentGear = (transmission->nNumberOfGears);
						isGolfCart = 1;
						break;
					default:
						currentGear = AudioVehicleReverseHost_Gear(params->m_pVehicle);
						break;
					}
					switch (params->m_VehicleType) {
					case VEHICLE_TYPE_CAR:
						automobile = params->m_pVehicle;
						wheelsOnGround = AudioVehicleReverseHost_DriveWheels(automobile);
						wheelsOnGroundPrev = AudioVehicleReverseHost_PreviousDriveWheels(automobile);
						wheelState = AudioVehicleEngineHost_CarStates(automobile);
						gasPedalAudioPtr = AudioVehicleReverseHost_GasAudio(automobile);
						break;
					case VEHICLE_TYPE_BIKE:
						bike = params->m_pVehicle;
						wheelsOnGround = AudioVehicleEngineHost_BikeWheels(bike);
						wheelsOnGroundPrev = AudioVehicleEngineHost_BikePreviousWheels(bike);
						wheelState = AudioVehicleEngineHost_BikeStates(bike);
						gasPedalAudioPtr = AudioVehicleEngineHost_BikeGasAudio(bike);
						break;
					default:
						AudioVehicleEngineHost_Unknown(params->m_VehicleType);
						return 1;
					}
					if (wheelsOnGround > 0) {
						if (AudioVehicleEngineHost_Handbrake(params->m_pVehicle) && (!isMoped || !isGolfCart)) { // what a weird check
							if (params->m_fVelocityChange == 0.0f)
								traction = 0.9f;
						} else if (AudioVehicleEngineHost_Status(params->m_pVehicle) == STATUS_SIMPLE || isMoped || isGolfCart) {
							traction = 0.0f;
						} else {
							switch ((transmission->nDriveType)) {
							case '4':
								if (params->m_VehicleType == VEHICLE_TYPE_BIKE) {
									for (uint8_t i = 0; i < 2; i++)
										if (wheelState[i] == WHEEL_STATE_SPINNING)
											traction += 0.1f;
								} else {
									for (uint8_t i = 0; i < 4; i++)
										if (wheelState[i] == WHEEL_STATE_SPINNING)
											traction += 0.05f;
								}
								break;
							case 'F':
								if (params->m_VehicleType == VEHICLE_TYPE_BIKE) {
									if (wheelState[BIKEWHEEL_FRONT] == WHEEL_STATE_SPINNING)
										traction += 0.2f;
								} else {
									if (wheelState[CARWHEEL_FRONT_LEFT] == WHEEL_STATE_SPINNING)
										traction += 0.1f;
									if (wheelState[CARWHEEL_FRONT_RIGHT] == WHEEL_STATE_SPINNING)
										traction += 0.1f;
								}
								break;
							case 'R':
								if (params->m_VehicleType == VEHICLE_TYPE_BIKE) {
									if (wheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SPINNING)
										traction += 0.2f;
								} else {
									if (wheelState[CARWHEEL_REAR_LEFT] == WHEEL_STATE_SPINNING)
										traction += 0.1f;
									if (wheelState[CARWHEEL_REAR_RIGHT] == WHEEL_STATE_SPINNING)
										traction += 0.1f;
								}
								break;
							default:
								break;
							}
						}
						if ((transmission->fMaxVelocity) > 0.0f) {
							if (isMoped || isGolfCart)
								modificator = AudioVehicleEngine_Min(1.0f, fabsf((float)(params->m_fVelocityChange / (transmission->fMaxVelocity) > 1.0f)));
							else {
								if (currentGear > 0) {
									relativeGearChange = AudioVehicleEngine_Min(1.0f,
										params->m_fVelocityChange - (transmission->Gears[currentGear].fShiftDownVelocity)) / (transmission->fMaxVelocity) * 2.5f;
									if (traction == 0.0f && AudioVehicleEngineHost_Status(params->m_pVehicle) != STATUS_SIMPLE &&
										params->m_fVelocityChange < (transmission->Gears[1].fShiftUpVelocity))
										traction = 0.7f;
									modificator = traction * *gasPedalAudioPtr * 0.95f + (1.0f - traction) * relativeGearChange;
								} else {
									modificator = AudioVehicleEngine_Min(1.0f,
										1.0f - fabsf((params->m_fVelocityChange - (transmission->Gears[0].fShiftDownVelocity)) / (transmission->fMaxReverseVelocity)));
								}
							}
						} else
							modificator = 0.0f;
					} else {
						if (wheelsOnGroundPrev > 0)
							*gasPedalAudioPtr *= 0.4f;
						modificator = *gasPedalAudioPtr;
					}
					if (currentGear == 0 || wheelsOnGround > 0) {
						if (params->m_VehicleType == VEHICLE_TYPE_BIKE)
							freq = 22050;
						else
							freq = 13000 * modificator + 14000;
					} else
						freq = 1200 * currentGear + 18000 * modificator + 14000;
					if (modificator < 0.75f)
						Vol = modificator / 0.75f * (VEHICLE_ENGINE_FULL_VOLUME - VEHICLE_ENGINE_BASE_VOLUME) + VEHICLE_ENGINE_BASE_VOLUME;
					else
						Vol = VEHICLE_ENGINE_FULL_VOLUME;
				} else {
					modificator = 0.0f;
					Vol = VEHICLE_ENGINE_BASE_VOLUME;
				}
				if (AudioVehicleModelHost_Drowning(params->m_pVehicle))
					Vol >>= 2;
				if (isGolfCart) {
					Vol = 100 * modificator;
					freq = 2130 * modificator + 4270;
					manager->m_sQueueSample.m_nCounter = 2;
				}
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, VEHICLE_ENGINE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					if (!isGolfCart) {
						if (AudioVehicleEngineHost_Status(params->m_pVehicle) == STATUS_SIMPLE) {
							if (modificator < 0.02f) {
								manager->m_sQueueSample.m_nSampleIndex = AudioVehicleEngineHost_Bank(params->m_nIndex) - CAR_SFX_BANKS_OFFSET + SFX_CAR_IDLE_1;
								freq = 10000 * modificator + 22050;
								manager->m_sQueueSample.m_nCounter = 52;
							} else {
								manager->m_sQueueSample.m_nSampleIndex = AudioVehicleEngineHost_Acceleration(params->m_nIndex);
								manager->m_sQueueSample.m_nCounter = 2;
							}
						} else {
							if (AudioVehicleReverseHost_Gas(params->m_pVehicle) < 0.02f) {
								manager->m_sQueueSample.m_nSampleIndex = AudioVehicleEngineHost_Bank(params->m_nIndex) - CAR_SFX_BANKS_OFFSET + SFX_CAR_IDLE_1;
								freq = 10000 * modificator + 22050;
								manager->m_sQueueSample.m_nCounter = 52;
							} else {
								manager->m_sQueueSample.m_nSampleIndex = AudioVehicleEngineHost_Acceleration(params->m_nIndex);
								manager->m_sQueueSample.m_nCounter = 2;
							}
						}
					}
					if (isGolfCart) {
						if (AudioGame_FindVehicle() == params->m_pVehicle)
							manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_AFTER_ACCEL_12;
						else
							manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_REV_12;
						manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0; // TODO: PS2 bank
						manager->m_sQueueSample.m_nFrequency = freq + 20 * manager->m_sQueueSample.m_nEntityIndex % 100;
					} else {
						manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0; // TODO: PS2 bank
						manager->m_sQueueSample.m_nFrequency = freq + 100 * manager->m_sQueueSample.m_nEntityIndex % 1000;
					}
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nPriority = 3;
					if (manager->m_sQueueSample.m_nSampleIndex == SFX_CAR_IDLE_5 || manager->m_sQueueSample.m_nSampleIndex == SFX_CAR_REV_5)
						manager->m_sQueueSample.m_nFrequency >>= 1;
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_VEHICLE_ENGINE_SET_EMITTING_VOLUME(Vol);
					AUDIO_VEHICLE_ENGINE_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_fSpeedMultiplier = 6.0f;
					manager->m_sQueueSample.m_MaxDistance = VEHICLE_ENGINE_MAX_DIST;
					manager->m_sQueueSample.m_bStatic = 0;
					manager->m_sQueueSample.m_nFramesToPlay = 8;
					AUDIO_VEHICLE_ENGINE_SET_SOUND_REVERB(1);
					AUDIO_VEHICLE_ENGINE_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
		}
		return 1;
	}
	return 0;
}
//- rouz edit (ChatGPT)
