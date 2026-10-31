//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../core/SurfaceTypes.h"
#include "../modelinfo/VehicleTypes.h"
#include "../vehicles/WheelIndices.h"
#include "sampman.h"
#include <math.h>

enum {
	VEHICLE_SKIDDING_MAX_DIST = 40,
	VEHICLE_SKIDDING_VOLUME = 50,
};

static uint8_t AudioVehicleSkid_Decision(uint8_t result)
{
    // Retain conditional repeated transmission and turn-speed reads in floating extrema
    volatile uint8_t comparison = result;
    return comparison;
}
#define AudioVehicleSkid_Min(a,b) (AudioVehicleSkid_Decision((a) < (b)) ? (a) : (b))
#define AudioVehicleSkid_Max(a,b) (AudioVehicleSkid_Decision((a) > (b)) ? (a) : (b))
#ifndef GTA_PS2
#define AUDIO_VEHICLE_SKID_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_SKID_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_SKID_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_SKID_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_SKID_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_SKID_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_SKID_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_SKID_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_SKID_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_SKID_SET_SOUND_REVERB(b)
#endif


static float AudioVehicleSkid_Drive(CVehicle *veh, enum tWheelState wheelState, float gasPedalAudio, cTransmission *transmission, float velocityChange);
static float AudioVehicleSkid_NonDrive(CVehicle *veh, enum tWheelState wheelState, cTransmission *transmission, float velocityChange);

uint8_t
AudioVehicleSkid_Process(cAudioManager *manager, cVehicleParams *params)
{
    // Preserve the original wheel aggregation or skid-value arithmetic
	CVehicle *automobile;
	CVehicle *bike;
	uint8_t numWheels;
	uint8_t wheelsOnGround;
	float gasPedalAudio;
	enum tWheelState* wheelStateArr;


	cTransmission *transmission;
	uint8_t Vol;
	float newSkidVal = 0.0f;
	float skidVal = 0.0f;

	if (params->m_fDistance < (VEHICLE_SKIDDING_MAX_DIST * VEHICLE_SKIDDING_MAX_DIST)) {
		switch (params->m_VehicleType) {
		case VEHICLE_TYPE_CAR:
			automobile = params->m_pVehicle;
			numWheels = 4;
			wheelStateArr = AudioVehicleEngineHost_CarStates(automobile);
			wheelsOnGround = AudioVehicleRoadHost_WheelsOnGround(automobile, VEHICLE_TYPE_CAR);
			gasPedalAudio = AudioVehicleSkidHost_CarGas(automobile);
			break;
		case VEHICLE_TYPE_BIKE:
			bike = params->m_pVehicle;
			numWheels = 2;
			wheelStateArr = AudioVehicleEngineHost_BikeStates(bike);
			wheelsOnGround = AudioVehicleRoadHost_WheelsOnGround(bike, VEHICLE_TYPE_BIKE);
			gasPedalAudio = AudioVehicleSkidHost_BikeGas(bike);
			break;
		default:
			AudioVehicleSkidHost_Unknown(params->m_VehicleType);
			return 1;
		}
		if (wheelsOnGround > 0) {
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);

			for (uint8_t i = 0; i < numWheels; i++) {
				if (wheelStateArr[i] == WHEEL_STATE_NORMAL)
					continue;
				transmission = params->m_pTransmission;
				switch ((transmission->nDriveType)) {
				case '4':
					newSkidVal = AudioVehicleSkid_Drive(params->m_pVehicle, wheelStateArr[i], gasPedalAudio, transmission, params->m_fVelocityChange);
					break;
				case 'F':
					if (i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_FRONT_RIGHT)
						newSkidVal = AudioVehicleSkid_Drive(params->m_pVehicle, wheelStateArr[i], gasPedalAudio, transmission, params->m_fVelocityChange);
					else
						newSkidVal = AudioVehicleSkid_NonDrive(params->m_pVehicle, wheelStateArr[i], transmission, params->m_fVelocityChange);
					break;
				case 'R':
					if (i == CARWHEEL_REAR_LEFT || i == CARWHEEL_REAR_RIGHT)
						newSkidVal = AudioVehicleSkid_Drive(params->m_pVehicle, wheelStateArr[i], gasPedalAudio, transmission, params->m_fVelocityChange);
					else
						newSkidVal = AudioVehicleSkid_NonDrive(params->m_pVehicle, wheelStateArr[i], transmission, params->m_fVelocityChange);
					break;
				default:
					break;
				}
				skidVal = AudioVehicleSkid_Max(skidVal, newSkidVal);
			}

			if (skidVal > 0.0f) {
				Vol = VEHICLE_SKIDDING_VOLUME * skidVal;
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, VEHICLE_SKIDDING_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_nCounter = 3;
					switch (AudioVehicleRoadHost_Surface(params->m_pVehicle)) {
					case SURFACE_GRASS:
					case SURFACE_HEDGE:
						manager->m_sQueueSample.m_nSampleIndex = SFX_RAIN;
						Vol >>= 2;
						manager->m_sQueueSample.m_nFrequency = 13000 * skidVal + 35000;
						manager->m_sQueueSample.m_nVolume >>= 2;
						if (manager->m_sQueueSample.m_nVolume == 0)
							return 1;
						break;
					case SURFACE_GRAVEL:
					case SURFACE_MUD_DRY:
					case SURFACE_SAND:
					case SURFACE_WATER:
					case SURFACE_SAND_BEACH:
						manager->m_sQueueSample.m_nSampleIndex = SFX_GRAVEL_SKID;
						manager->m_sQueueSample.m_nFrequency = 6000 * skidVal + 10000;
						break;

					default:
						manager->m_sQueueSample.m_nSampleIndex = SFX_SKID;
						manager->m_sQueueSample.m_nFrequency = 5000 * skidVal + 11000;
						if (params->m_VehicleType == VEHICLE_TYPE_BIKE)
							manager->m_sQueueSample.m_nFrequency += 2000;
						break;
					}

					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nPriority = 8;
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_VEHICLE_SKID_SET_EMITTING_VOLUME(Vol);
					AUDIO_VEHICLE_SKID_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
					manager->m_sQueueSample.m_MaxDistance = VEHICLE_SKIDDING_MAX_DIST;
					manager->m_sQueueSample.m_bStatic = 0;
					manager->m_sQueueSample.m_nFramesToPlay = 3;
					AUDIO_VEHICLE_SKID_SET_SOUND_REVERB(1);
					AUDIO_VEHICLE_SKID_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
		}
		return 1;
	}
	return 0;
}

static float
AudioVehicleSkid_Drive(CVehicle *veh, enum tWheelState wheelState, float gasPedalAudio, cTransmission *transmission, float velocityChange)
{
    // Preserve the original wheel aggregation or skid-value arithmetic
	float relativeVelChange = 0.0f;
	float velChange;
	float relativeVel;

	switch (wheelState)
	{
	case WHEEL_STATE_SPINNING:
		if (gasPedalAudio > 0.4f)
			relativeVelChange = (gasPedalAudio - 0.4f) * (5.0f / 3.0f) * 0.75f;
		break;
	case WHEEL_STATE_SKIDDING:
		relativeVelChange = AudioVehicleSkid_Min(1.0f, fabsf(velocityChange) / (transmission->fMaxVelocity));
		break;
	case WHEEL_STATE_FIXED:
		relativeVel = gasPedalAudio;
		if (relativeVel > 0.4f)
			relativeVel = (gasPedalAudio - 0.4f) * (5.0f / 3.0f);

		velChange = fabsf(velocityChange);
		if (velChange > 0.04f)
			relativeVelChange = AudioVehicleSkid_Min(1.0f, velChange / (transmission->fMaxVelocity));
		if (relativeVel > relativeVelChange)
			relativeVelChange = relativeVel;

		break;
	default:
		break;
	}

	return AudioVehicleSkid_Max(relativeVelChange, AudioVehicleSkid_Min(1.0f, fabsf(AudioVehicleSkidHost_TurnZ(veh)) * 20.0f));
}

static float
AudioVehicleSkid_NonDrive(CVehicle *veh, enum tWheelState wheelState, cTransmission *transmission, float velocityChange)
{
    // Preserve the original wheel aggregation or skid-value arithmetic
	float relativeVelChange = 0.0f;

	if (wheelState == WHEEL_STATE_SKIDDING)
		relativeVelChange = AudioVehicleSkid_Min(1.0f, fabsf(velocityChange) / (transmission->fMaxVelocity));

	return AudioVehicleSkid_Max(relativeVelChange, AudioVehicleSkid_Min(1.0f, fabsf(AudioVehicleSkidHost_TurnZ(veh)) * 20.0f));
}

//- rouz edit (ChatGPT)
