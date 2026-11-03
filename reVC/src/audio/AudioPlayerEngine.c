//+ rouz edit (ChatGPT)
#include "ReplayState.h"
#include "AudioManager.h"
#include "AudioGeometry.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "../modelinfo/VehicleTypes.h"
#include "../vehicles/WheelIndices.h"
#include "sampman.h"

enum {
	VEHICLE_ENGINE_MAX_DIST = 50,
	PLAYER_VEHICLE_ENGINE_VOLUME = 120,
};
#ifndef GTA_PS2
#define CHANNEL_PLAYER_VEHICLE_ENGINE manager->m_nActiveSamples
#endif

static uint8_t AudioPlayerEngine_Decision(uint8_t result)
{
    // Retain conditional repeated game queries in floating clamps and minima
    volatile uint8_t comparison = result;
    return comparison;
}
#define AudioPlayerEngine_Min(a,b) (AudioPlayerEngine_Decision((a) < (b)) ? (a) : (b))
#define AudioPlayerEngine_Max(a,b) (AudioPlayerEngine_Decision((a) > (b)) ? (a) : (b))
#define AudioPlayerEngine_Clamp(v,low,high) (AudioPlayerEngine_Decision((v) < (low)) ? (low) : AudioPlayerEngine_Decision((v) > (high)) ? (high) : (v))

uint8_t bPlayerJustEnteredCar;

void
AudioPlayerEngine_Entered(cAudioManager *manager)
{
    // Retain the original initialized-owner gate before marking the next entry reset
	if (manager->m_bIsInitialised)
		bPlayerJustEnteredCar = 1;
}

void
AudioPlayerEngine_Left(cAudioManager *manager)
{
    // Retain the original empty exit notification
    (void)manager;
}

void
AudioPlayerEngine_Process(cAudioManager *manager, cVehicleParams *params, CVehicle *veh)
{
    // Preserve shared engine history and all original player channel transitions
    (void)veh;
	static int32_t GearFreqAdj[] = { 6000, 6000, 3400, 1200, 0, -1000 };

	float relativeVelocityChange;
	float accelerationMultipler;
	uint8_t wheelInUseCounter;
	float time;
	int baseFreq;
	uint8_t vol;
	int32_t freq;

	int freqModifier;
	uint32_t soundOffset;
	uint8_t engineSoundType;
	int16_t accelerateState;
	int16_t brakeState;
	uint8_t channelUsed;

	uint8_t currentGear;
	float gasPedalAudio;
	AudioSoundPosition pos;
	uint8_t slowingDown;

	enum tWheelState *wheelState;
	CVehicle *automobile;
	CVehicle *bike;
	float *gasPedalAudioPtr;

	uint32_t gearSoundLength;
	uint8_t wheelsOnGround;
	uint8_t wheelsOnGroundPrev;

	float velocityChangeForAudio;
	uint8_t noGearBox;
	uint8_t stuckInSand;

	static int16_t LastAccel = 0;
	static uint8_t CurrentPretendGear = 1;
	static uint8_t bLostTractionLastFrame = 0;
	static uint8_t bHandbrakeOnLastFrame = 0;
	static uint32_t nCruising = 0;
	static uint8_t bAccelSampleStopped = 1;

	uint8_t lostTraction = 0;
	uint8_t isMoped = 0;
	uint8_t processedAccelSampleStopped = 0;
	static uint32_t gearSoundStartTime;
    static uint8_t gearSoundTimeInitialized;
    // Initialize the original shared timer at the first engine call
    if (!gearSoundTimeInitialized) {
        gearSoundStartTime = AudioPlayerEngineHost_Time();
        gearSoundTimeInitialized = 1;
    }
#ifdef GTA_PS2
	uint8_t nChannel = manager->m_bIsSurround ? CHANNEL_DTS_PLAYER_VEHICLE_ENGINE : CHANNEL_PLAYER_VEHICLE_ENGINE;
#else
	uint8_t nChannel = CHANNEL_PLAYER_VEHICLE_ENGINE;
#endif
	if (bPlayerJustEnteredCar) {
		bAccelSampleStopped = 1;
		bPlayerJustEnteredCar = 0;
		nCruising = 0;
		LastAccel = 0;
		bLostTractionLastFrame = 0;
		CurrentPretendGear = 1;
		bHandbrakeOnLastFrame = 0;
	}
#ifdef FIX_BUGS // fix acceleration sound on exiting the vehicle
	if (Replay_IsPlayingBack() || AudioPlayerEngineHost_Exiting()) {
#else
	if (Replay_IsPlayingBack()) {
#endif
		accelerateState = (255 * AudioPlayerEngine_Clamp(AudioVehicleReverseHost_Gas(params->m_pVehicle), 0.0f, 1.0f));
		brakeState = (255 * AudioPlayerEngine_Clamp(AudioVehicleModelHost_BrakePedal(params->m_pVehicle), 0.0f, 1.0f));
	} else {
		accelerateState = AudioVehicleModelHost_Accelerate();
		brakeState = AudioVehicleModelHost_Brake();
	}
	slowingDown = params->m_fVelocityChange < -0.001f;
	channelUsed = SampleManager_GetChannelUsedFlag(&SampleManager, nChannel);
	if (AudioVehicleReverseHost_Model(params->m_pVehicle) == MI_PIZZABOY || AudioVehicleReverseHost_Model(params->m_pVehicle) == MI_FAGGIO) {
		CurrentPretendGear = (params->m_pTransmission->nNumberOfGears);
		currentGear = CurrentPretendGear;
		if (AudioVehicleEngineHost_Handbrake(params->m_pVehicle)) {
			brakeState = 0;
			nCruising = 0;
			LastAccel = 0;
			accelerateState = 0;
		} else
			nCruising = 1;
		isMoped = 1;
	} else
		currentGear = AudioVehicleReverseHost_Gear(params->m_pVehicle);

	switch (params->m_VehicleType) {
	case VEHICLE_TYPE_CAR:
		automobile = params->m_pVehicle;
		wheelsOnGround = AudioVehicleReverseHost_DriveWheels(automobile);
		wheelsOnGroundPrev = AudioVehicleReverseHost_PreviousDriveWheels(automobile);
		gasPedalAudioPtr = AudioVehicleReverseHost_GasAudio(automobile);
		wheelState = AudioVehicleEngineHost_CarStates(automobile);
		velocityChangeForAudio = AudioPlayerEngineHost_CarVelocity(automobile);
		break;
	case VEHICLE_TYPE_BIKE:
		bike = params->m_pVehicle;
		wheelsOnGround = AudioVehicleEngineHost_BikeWheels(bike);
		wheelsOnGroundPrev = AudioVehicleEngineHost_BikePreviousWheels(bike);
		gasPedalAudioPtr = AudioVehicleEngineHost_BikeGasAudio(bike);
		wheelState = AudioVehicleEngineHost_BikeStates(bike);
		velocityChangeForAudio = AudioPlayerEngineHost_BikeVelocity(bike);
		break;
	default:
		AudioVehicleEngineHost_Unknown(params->m_VehicleType);
		return;
	}
	if (!isMoped) {
		switch ((params->m_pTransmission->nDriveType)) {
		case '4':
			if (params->m_VehicleType != VEHICLE_TYPE_BIKE) {
				wheelInUseCounter = 0;
				for (uint8_t i = 0; i < 4; i++) {
					if (wheelState[i] != WHEEL_STATE_NORMAL)
						wheelInUseCounter++;
				}
				if (wheelInUseCounter > 2)
					lostTraction = 1;
			}
			break;
		case 'F':
			if (params->m_VehicleType == VEHICLE_TYPE_BIKE) {
				if (wheelState[BIKEWHEEL_FRONT] != WHEEL_STATE_NORMAL)
					lostTraction = 1;
			} else {
				if ((wheelState[CARWHEEL_FRONT_LEFT] != WHEEL_STATE_NORMAL || wheelState[CARWHEEL_FRONT_RIGHT] != WHEEL_STATE_NORMAL) &&
					(wheelState[CARWHEEL_REAR_LEFT] != WHEEL_STATE_NORMAL || wheelState[CARWHEEL_REAR_RIGHT] != WHEEL_STATE_NORMAL))
					lostTraction = 1;
			}
			break;
		case 'R':
			if (params->m_VehicleType == VEHICLE_TYPE_BIKE) {
				if (wheelState[BIKEWHEEL_REAR] != WHEEL_STATE_NORMAL)
					lostTraction = 1;
			} else {
				if (wheelState[CARWHEEL_REAR_LEFT] != WHEEL_STATE_NORMAL || wheelState[CARWHEEL_REAR_RIGHT] != WHEEL_STATE_NORMAL)
					lostTraction = 1;
			}
			break;
		default:
			break;
		}
	}
	if (params->m_fVelocityChange != 0.0f) {
		time = AudioPlayerEngineHost_SpeedZ(params->m_pVehicle) / params->m_fVelocityChange;
		if (time > 0.0f)
			freqModifier = -(AudioPlayerEngine_Min(0.2f, time) * 3000 * 5);
		else
			freqModifier = -(AudioPlayerEngine_Max(-0.2f, time) * 3000 * 5);
		if (slowingDown)
			freqModifier = -freqModifier;
	} else
		freqModifier = 0;

	if (params->m_VehicleType == VEHICLE_TYPE_BIKE && AudioPlayerEngineHost_ExtraSpeed(bike))
		freqModifier += 1400;

	gearSoundLength = 0;
	engineSoundType = AudioVehicleEngineHost_Bank(params->m_nIndex);
	soundOffset = 3 * (engineSoundType - CAR_SFX_BANKS_OFFSET);
	noGearBox = 0;
	switch (engineSoundType) {
	case SFX_BANK_PONTIAC:
		gearSoundLength = 2526;
		break;
	case SFX_BANK_PORSCHE:
		gearSoundLength = 3587;
		break;
	case SFX_BANK_SPIDER:
		gearSoundLength = 4898;
		break;
	case SFX_BANK_MERC:
		gearSoundLength = 4003;
		break;
	case SFX_BANK_TRUCK:
		gearSoundLength = 6289;
		break;
	case SFX_BANK_HOTROD:
		gearSoundLength = 2766;
		break;
	case SFX_BANK_COBRA:
		gearSoundLength = 3523;
		break;
	case SFX_BANK_PONTIAC_SLOW:
		gearSoundLength = 2773;
		break;
	case SFX_BANK_CADILLAC:
		gearSoundLength = 2560;
		break;
	case SFX_BANK_PATHFINDER:
		gearSoundLength = 4228;
		break;
	case SFX_BANK_PACARD:
		gearSoundLength = 4648;
		break;
	case SFX_BANK_VTWIN:
		gearSoundLength = 3480;
		break;
	case SFX_BANK_HONDA250:
		gearSoundLength = 2380;
		break;
	case SFX_BANK_SPORTS_BIKE:
		gearSoundLength = 2410;
		break;
	default:
		noGearBox = 1;
		break;
	}
	if (channelUsed && nCruising == 0 && !noGearBox) {
		gearSoundLength -= 1000;
		if (AudioPlayerEngineHost_Time() - gearSoundStartTime > gearSoundLength) {
			channelUsed = 0;
			gearSoundStartTime = AudioPlayerEngineHost_Time();
		}
	} else
		gearSoundStartTime = AudioPlayerEngineHost_Time();
	relativeVelocityChange = 2.0f * params->m_fVelocityChange / (params->m_pTransmission->fMaxVelocity);
	accelerationMultipler = AudioPlayerEngine_Clamp(relativeVelocityChange, 0.0f, 1.0f);
	gasPedalAudio = accelerationMultipler;
	switch (engineSoundType) {
	case SFX_BANK_MOPED:
		soundOffset++;
		break;
	case SFX_BANK_HONDA250:
		soundOffset += 2;
		break;
	case SFX_BANK_SPORTS_BIKE:
		soundOffset += 3;
		break;
	default:
		break;
	}
	if (accelerateState > 0) {
		if (nCruising > 0) {
PlayCruising:
			bAccelSampleStopped = 1;
			SampleManager_StopChannel(&SampleManager, nChannel);
			if (!isMoped && (accelerateState < 150 || wheelsOnGround == 0 || brakeState > 0 || AudioVehicleEngineHost_Handbrake(params->m_pVehicle)
				|| lostTraction || currentGear < (params->m_pTransmission->nNumberOfGears) - 1)) {
				nCruising = 0;
			} else {
				if (accelerateState < 220 || params->m_fVelocityChange + 0.001f < velocityChangeForAudio) {
					if (nCruising > 3)
						nCruising--; 
				} else
					if (nCruising < 800)
						nCruising++;
				freq = 27 * nCruising + freqModifier + 22050;
				if (engineSoundType == SFX_BANK_TRUCK)
					freq >>= 1;
				AudioVehiclePlayer_AddSample(manager, PLAYER_VEHICLE_ENGINE_VOLUME, freq, soundOffset + SFX_CAR_AFTER_ACCEL_1, engineSoundType, 64, 1);
			}
		} else {
			stuckInSand = params->m_VehicleType == VEHICLE_TYPE_CAR && AudioPlayerEngineHost_Sand(params->m_pVehicle);
			if (accelerateState < 150 || wheelsOnGround == 0 || AudioVehicleEngineHost_Handbrake(params->m_pVehicle) || lostTraction
				|| (currentGear < 2 && params->m_fVelocityChange - velocityChangeForAudio < 0.01f) || brakeState > 0) {
				if (((wheelsOnGround == 0 || AudioVehicleEngineHost_Handbrake(params->m_pVehicle) || lostTraction) && !stuckInSand) || brakeState > 0) {
					if (wheelsOnGround == 0 && wheelsOnGroundPrev != 0 || (AudioVehicleEngineHost_Handbrake(params->m_pVehicle) && !bHandbrakeOnLastFrame || lostTraction && !bLostTractionLastFrame)
						&& wheelsOnGround != 0) {
						*gasPedalAudioPtr *= 0.6f;
					}
					freqModifier = 0;
					if (engineSoundType == SFX_BANK_GOLF_CART || engineSoundType == SFX_BANK_CAR_CHAINSAW)
						baseFreq = (15000 * *gasPedalAudioPtr) + 14000;
					else
						baseFreq = (25000 * *gasPedalAudioPtr) + 14000;
					vol = (25 * *gasPedalAudioPtr) + 60;
				} else {
					baseFreq = (8000 * accelerationMultipler) + 16000;
					vol = (25 * accelerationMultipler) + 60;
					*gasPedalAudioPtr = accelerationMultipler;
				}
				freq = freqModifier + baseFreq;
				if (engineSoundType == SFX_BANK_TRUCK)
					freq >>= 1;
				if (channelUsed) {
					SampleManager_StopChannel(&SampleManager, nChannel);
					bAccelSampleStopped = 1;
				}
				if (AudioVehicleModelHost_Drowning(params->m_pVehicle))
					vol >>= 2;
				AudioVehiclePlayer_AddSample(manager, vol, freq, engineSoundType - CAR_SFX_BANKS_OFFSET + SFX_CAR_REV_1, SFX_BANK_0, 2, 1);
			} else {
				// Translate a copied game vector through the existing camera matrix
				const AudioSoundPosition soundPosition = manager->m_sQueueSample.m_vecPos;
				AudioGeometry_Translate(&soundPosition, &pos);
#ifndef EXTERNAL_3D_SOUND
				manager->m_sQueueSample.m_nPan = AudioMath_ComputePan(manager->m_sQueueSample.m_fDistance, pos.x);
#endif
				if (bAccelSampleStopped) {
					if (CurrentPretendGear != 1 || currentGear != 2)
						CurrentPretendGear = AudioPlayerEngine_Max(1, currentGear - 1);
					processedAccelSampleStopped = 1;
					bAccelSampleStopped = 0;
				}

				if (!channelUsed) {
					if (!processedAccelSampleStopped) {
						if (CurrentPretendGear < (params->m_pTransmission->nNumberOfGears) - 1)
							CurrentPretendGear++;
						else {
							nCruising = 1;
							AudioPlayerEngineHost_ChangingGear(params->m_pVehicle);
							goto PlayCruising;
						}
					}

					gearSoundStartTime = AudioPlayerEngineHost_Time();
					AudioPlayerEngineHost_ChangingGear(params->m_pVehicle);
#ifdef GTA_PS2
					SampleManager_InitialiseChannel(&SampleManager, nChannel, soundOffset + SFX_CAR_ACCEL_1, SFX_BANK_0);
#else
					if (!SampleManager_InitialiseChannel(&SampleManager, CHANNEL_PLAYER_VEHICLE_ENGINE, soundOffset + SFX_CAR_ACCEL_1, SFX_BANK_0))
						return;
#endif
					SampleManager_SetChannelLoopCount(&SampleManager, CHANNEL_PLAYER_VEHICLE_ENGINE, 1);
#ifndef GTA_PS2
					SampleManager_SetChannelLoopPoints(&SampleManager, CHANNEL_PLAYER_VEHICLE_ENGINE, 0, -1);
#endif
				}

#ifdef EXTERNAL_3D_SOUND
				SampleManager_SetChannelEmittingVolume(&SampleManager, CHANNEL_PLAYER_VEHICLE_ENGINE, PLAYER_VEHICLE_ENGINE_VOLUME);
				SampleManager_SetChannel3DPosition(&SampleManager, CHANNEL_PLAYER_VEHICLE_ENGINE, pos.x, pos.y, pos.z);
				SampleManager_SetChannel3DDistances(&SampleManager, CHANNEL_PLAYER_VEHICLE_ENGINE, VEHICLE_ENGINE_MAX_DIST, VEHICLE_ENGINE_MAX_DIST / 4.0f);
#else
				SampleManager_SetChannelVolume(&SampleManager, nChannel, AudioMath_ComputeVolume(PLAYER_VEHICLE_ENGINE_VOLUME, VEHICLE_ENGINE_MAX_DIST, manager->m_sQueueSample.m_fDistance));
				SampleManager_SetChannelPan(&SampleManager, nChannel, manager->m_sQueueSample.m_nPan);
#endif
				freq = GearFreqAdj[CurrentPretendGear] + freqModifier + 22050;
				if (engineSoundType == SFX_BANK_TRUCK)
					freq >>= 1;
#ifdef USE_TIME_SCALE_FOR_AUDIO
				SampleManager_SetChannelFrequency(&SampleManager, nChannel, freq * AudioPoliceHost_TimeScale());
#else
				SampleManager_SetChannelFrequency(&SampleManager, nChannel, freq);
#endif
				if (!channelUsed) {
#ifdef AUDIO_REVERB
					SampleManager_SetChannelReverbFlag(&SampleManager, nChannel, manager->m_bDynamicAcousticModelingStatus != 0);
#endif
					SampleManager_StartChannel(&SampleManager, nChannel);
				}
			}
		}
	} else {
		if (slowingDown) {
			if (channelUsed) {
				SampleManager_StopChannel(&SampleManager, nChannel);
				bAccelSampleStopped = 1;
			}
			if (wheelsOnGround == 0 || AudioVehicleEngineHost_Handbrake(params->m_pVehicle) || lostTraction)
				gasPedalAudio = *gasPedalAudioPtr;
			else if (params->m_VehicleType == VEHICLE_TYPE_BIKE)
				gasPedalAudio = 0.0f;
			else
				gasPedalAudio = AudioPlayerEngine_Min(1.0f, params->m_fVelocityChange / (params->m_pTransmission->fMaxReverseVelocity));
			*gasPedalAudioPtr = AudioPlayerEngine_Max(0.0f, gasPedalAudio);
		} else if (LastAccel > 0) {
			if (channelUsed) {
				SampleManager_StopChannel(&SampleManager, nChannel);
				bAccelSampleStopped = 1;
			}
			nCruising = 0;
			if (wheelsOnGround == 0
				|| AudioVehicleEngineHost_Handbrake(params->m_pVehicle)
				|| lostTraction
				|| params->m_fVelocityChange < 0.01f && *gasPedalAudioPtr > 0.2f) {
				if (AudioVehicleReverseHost_Model(params->m_pVehicle) == MI_PIZZABOY || AudioVehicleReverseHost_Model(params->m_pVehicle) == MI_FAGGIO) {
					gasPedalAudio = 0.0f;
				} else {
					*gasPedalAudioPtr *= 0.6f;
					gasPedalAudio = *gasPedalAudioPtr;
				}
			}
			if (gasPedalAudio > 0.05f) {
				freq = (5000 * ((gasPedalAudio - 0.05f) / 0.95f)) + 19000;
				vol = (25 * ((gasPedalAudio - 0.05f) / 0.95f)) + 40;
				if (AudioVehicleModelHost_Drowning(params->m_pVehicle))
					vol >>= 2;
				if (engineSoundType == SFX_BANK_TRUCK)
					freq >>= 1;
				AudioVehiclePlayer_AddSample(manager, vol, freq, soundOffset + SFX_CAR_FINGER_OFF_ACCEL_1, engineSoundType, 63, 0);
			}
		}
		freq = (10000 * gasPedalAudio) + 22050;
		vol = 110 - (40 * gasPedalAudio);
		if (engineSoundType == SFX_BANK_TRUCK)
			freq >>= 1;
		if (AudioVehicleModelHost_Drowning(params->m_pVehicle))
			vol >>= 2;
		AudioVehiclePlayer_AddSample(manager, vol, freq, engineSoundType - CAR_SFX_BANKS_OFFSET + SFX_CAR_IDLE_1, SFX_BANK_0, 52, 1);

		CurrentPretendGear = AudioPlayerEngine_Max(1, currentGear);
	}
	LastAccel = accelerateState;

	bHandbrakeOnLastFrame = !!AudioVehicleEngineHost_Handbrake(params->m_pVehicle);
	bLostTractionLastFrame = lostTraction;
}
//- rouz edit (ChatGPT)
