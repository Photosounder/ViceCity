//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "../modelinfo/VehicleTypes.h"
#include "AudioPoliceGame.h"
#include <stddef.h>
#include "sampman.h"

enum {
	CAR_HELI_MAX_DIST = 250,
	CAR_HELI_ENGINE_MAX_DIST = 140,
	CAR_HELI_ENGINE_START_MAX_DIST = 30,
	CAR_HELI_ENGINE_START_VOLUME = 70,
	CAR_HELI_SEAPLANE_MAX_DIST = 20,
	CAR_HELI_SEAPLANE_VOLUME = 100,
	CAR_HELI_REAR_MAX_DIST = 27,
	CAR_HELI_REAR_VOLUME = 25,
	CESNA_IDLE_MAX_DIST = 200,
	CESNA_REV_MAX_DIST = 90,
	CESNA_VOLUME = 80,
};
#ifndef GTA_PS2
#define AUDIO_AIRCRAFT_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_AIRCRAFT_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_AIRCRAFT_RESET_LOOP_OFFSETS
#define AUDIO_AIRCRAFT_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_AIRCRAFT_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_AIRCRAFT_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_AIRCRAFT_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_AIRCRAFT_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_AIRCRAFT_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_AIRCRAFT_SET_SOUND_REVERB(b)
#endif


static float AudioAircraft_UpDot(const AudioCameraMatrix *matrix)
{
	// Preserve all original dot-product terms including the zero multipliers
	return matrix->ux * 0.0f + matrix->uy * 1.0f + matrix->uz * 0.0f;
}
static float AudioAircraft_ForwardDot(CVehicle *vehicle, const AudioCameraMatrix *matrix)
{
	// Copy the live forward vector before applying the original dot product
	AudioSoundPosition forward;
	AudioAircraftHost_Forward(vehicle, &forward);
	return forward.x * matrix->fx + forward.y * matrix->fy + forward.z * matrix->fz;
}
static void AudioAircraft_Rear(CVehicle *vehicle, AudioSoundPosition *position)
{
	// Preserve the original complete matrix-vector product and vanilla discarded result
	AudioCameraMatrix matrix;
	AudioSoundPosition result;
	AudioAircraftHost_Matrix(vehicle, &matrix);
	result.x = matrix.rx * 0.0f + matrix.fx * -10.0f + matrix.ux * 0.0f + matrix.px;
	result.y = matrix.ry * 0.0f + matrix.fy * -10.0f + matrix.uy * 0.0f + matrix.py;
	result.z = matrix.rz * 0.0f + matrix.fz * -10.0f + matrix.uz * 0.0f + matrix.pz;
	if (position) *position = result;
}

#define AUDIO_AIRCRAFT_MIN(a,b) ((a) < (b) ? (a) : (b))
#define AUDIO_AIRCRAFT_MAX(a,b) ((a) > (b) ? (a) : (b))
#define AUDIO_AIRCRAFT_ABS(a) (((a) < 0) ? (-(a)) : (a))
#define AUDIO_AIRCRAFT_CLAMP(v,c,r) ((v) > (c) ? AUDIO_AIRCRAFT_MIN(v,c+r) : AUDIO_AIRCRAFT_MAX(v,c-r))

uint8_t
AudioAircraft_Heli(cAudioManager *manager, cVehicleParams *params)
{
	// Generate every rotor layer with the original shared frequency histories
	CVehicle* automobile;
	CVehicle* boat;

	uint8_t Vol;
	int16_t brakeState;
	int16_t accelerateState;
	uint32_t freq;
	float propellerSpeed;
	float freqModifier; //may be relate to angle with horison
	float cameraAngle;
	uint8_t distanceCalculatedOld;
	float distanceOld;
	AudioSoundPosition vecPosOld;

	float volumeModifier;//TODO find better name
	uint8_t hunterBool;
	AudioCameraMatrix cameraMatrix;
#ifdef GTA_PS2
    // Store the original matrix assignment cleanup state until this generator returns
    void *cameraAttachment = NULL;
    uint8_t cameraOwned = 0;
#endif

	boat = NULL;
	automobile = NULL;
	hunterBool = 0;

	static uint32_t freqFrontPrev = 14287;
	static uint32_t freqPropellerPrev = 7143;
	static uint32_t freqSkimmerPrev = 14287;


	if (params->m_fDistance < (CAR_HELI_MAX_DIST * CAR_HELI_MAX_DIST)) {
		if (AudioPoliceHost_PlayerVehicle() == params->m_pVehicle) {
			accelerateState = AudioVehicleModelHost_Accelerate();
			brakeState = AudioVehicleModelHost_Brake();
		} else {
			accelerateState = AudioVehicleReverseHost_Gas(params->m_pVehicle) * 255.0f;
			brakeState = AudioVehicleModelHost_BrakePedal(params->m_pVehicle) * 255.0f;
		}
		#ifdef GTA_PS2
    AudioAircraftHost_CameraBegin(&cameraMatrix, &cameraAttachment, &cameraOwned);
#else
    AudioGeometryHost_CameraMatrix(&cameraMatrix);
#endif
		freqModifier = AudioAircraft_UpDot(&cameraMatrix);
		freqModifier = AUDIO_AIRCRAFT_ABS(freqModifier);
		cameraAngle = (AudioAircraft_ForwardDot(params->m_pVehicle, &cameraMatrix) + 1.0f) / 2.0f;
		if (AudioVehicleReverseHost_Model(params->m_pVehicle) == MI_SKIMMER) {
			boat = params->m_pVehicle;
			propellerSpeed = AudioAircraftHost_MovingSpeed(boat) * 50.0f / 11.0f;
		} else if (params->m_VehicleType == VEHICLE_TYPE_HELI) {
			propellerSpeed = 1.0f;
		} else {
			automobile = params->m_pVehicle;
			propellerSpeed = AudioAircraftHost_WheelSpeed(automobile) * 50.0f / 11.0f;
		}

		if (propellerSpeed == 0.0f)
			{
    // Complete the original camera snapshot lifecycle before returning
#ifdef GTA_PS2
    AudioAircraftHost_CameraEnd(cameraAttachment, cameraOwned);
#endif
    return 1;
}

		propellerSpeed = AUDIO_AIRCRAFT_MIN(1.0f, propellerSpeed);
		AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);


		//sound on long distances
		if (manager->m_sQueueSample.m_fDistance < 40.0f) {
			if (manager->m_sQueueSample.m_fDistance < 25.0f)
				Vol = 0;
			else
				Vol = (manager->m_sQueueSample.m_fDistance - 25.0f) * (75.0f * propellerSpeed) / 15.0f;
		} else
			Vol = propellerSpeed * 75.0f;
		if (Vol > 0) {
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, CAR_HELI_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = 88;
				if (boat != NULL) {
					manager->m_sQueueSample.m_nSampleIndex = SFX_SEAPLANE_PRO3;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					if (accelerateState > 0 || brakeState > 0)
						manager->m_sQueueSample.m_nFrequency = 4600 + AUDIO_AIRCRAFT_MIN(1.0f, (AUDIO_AIRCRAFT_MAX(accelerateState, brakeState) / 255.0f) * freqModifier) * 563;
					else
						manager->m_sQueueSample.m_nFrequency = 3651 + AUDIO_AIRCRAFT_MIN(1.0f, freqModifier) * 949;
				} else {
					manager->m_sQueueSample.m_nSampleIndex = SFX_HELI_1;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
				}
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 2;
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_AIRCRAFT_SET_EMITTING_VOLUME(Vol);
				AUDIO_AIRCRAFT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				manager->m_sQueueSample.m_fSpeedMultiplier = 6.0f;
				manager->m_sQueueSample.m_MaxDistance = CAR_HELI_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 5;
				AUDIO_AIRCRAFT_SET_SOUND_REVERB(1);
				AUDIO_AIRCRAFT_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
	
		AudioSoundPosition backPropellerPos;
		if (automobile != NULL)
			AudioAircraftHost_Boot(automobile, &backPropellerPos);
		else if (params->m_VehicleType == VEHICLE_TYPE_HELI) 
#ifdef FIX_BUGS
			AudioAircraft_Rear(params->m_pVehicle, &backPropellerPos);
#else
			AudioAircraft_Rear(params->m_pVehicle, NULL);
#endif
		else
			// Copy the sound coordinates for the game vector operation
			backPropellerPos = manager->m_sQueueSample.m_vecPos;

		if (params->m_fDistance < (CAR_HELI_ENGINE_MAX_DIST * CAR_HELI_ENGINE_MAX_DIST)) {
			if (propellerSpeed < 0.4f)
				volumeModifier = 0.0f;
			else
				volumeModifier = (propellerSpeed - 0.4f) * 5.0f / 3.0f;
			if (!boat) {
				freq = AUDIO_AIRCRAFT_MIN(1300, 7000 * freqModifier);
				if (AudioPoliceHost_PlayerVehicle() == params->m_pVehicle && (accelerateState > 0 || brakeState > 0) && freq < 1300)//unnesesary freqModifier alredy <= 1300
					freq = 1300;
				if (AudioVehicleReverseHost_Model(params->m_pVehicle) == MI_HUNTER)
					hunterBool = 1;
			}

			//sound from front of helicopter
			Vol = (1.0f - cameraAngle) * volumeModifier * MAX_VOLUME;
			if (!boat) {
				if (accelerateState > 0 || brakeState > 0)
					manager->m_sQueueSample.m_nFrequency = 18000 + AUDIO_AIRCRAFT_MIN(1.0f, freqModifier * (AUDIO_AIRCRAFT_MAX(accelerateState, brakeState) / 255.0f)) * 2204;
				else
					manager->m_sQueueSample.m_nFrequency = 14287 + AUDIO_AIRCRAFT_MIN(1.0f, freqModifier) * 3713;
				if (propellerSpeed < 1.0f)
					manager->m_sQueueSample.m_nFrequency = (propellerSpeed + 1.0f) * (manager->m_sQueueSample.m_nFrequency >> 1);
				manager->m_sQueueSample.m_nFrequency = AUDIO_AIRCRAFT_CLAMP(manager->m_sQueueSample.m_nFrequency, freqFrontPrev, 197);
				freqFrontPrev = manager->m_sQueueSample.m_nFrequency;
			}

			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, CAR_HELI_ENGINE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = 3;
				if (hunterBool) {
					manager->m_sQueueSample.m_nSampleIndex = SFX_HELI_APACHE_1;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_nFrequency = (volumeModifier + 1.0f) * 16000 + freq;
				} else if (boat) {
					manager->m_sQueueSample.m_nSampleIndex = SFX_SEAPLANE_PRO1;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				} else {
					manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_HELI_MAI;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_nFrequency = (volumeModifier + 1.0f) * 16000 + freq;
				}
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 1;
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_AIRCRAFT_SET_EMITTING_VOLUME(Vol);
				AUDIO_AIRCRAFT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_fSpeedMultiplier = 6.0f;
				manager->m_sQueueSample.m_MaxDistance = CAR_HELI_ENGINE_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 5;
				AUDIO_AIRCRAFT_SET_SOUND_REVERB(1);
				AUDIO_AIRCRAFT_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}


			//engine starting sound 
			if (boat == NULL && params->m_VehicleType != VEHICLE_TYPE_HELI && manager->m_sQueueSample.m_fDistance < CAR_HELI_ENGINE_START_MAX_DIST && AudioVehicleReverseHost_Engine(automobile) && propellerSpeed < 1.0f) { //strange way to check if automobile != NULL
				Vol = (1.0f - propellerSpeed / 2.0f) * CAR_HELI_ENGINE_START_VOLUME;
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, CAR_HELI_ENGINE_START_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					if (hunterBool) {
						manager->m_sQueueSample.m_nSampleIndex = SFX_HELI_APACHE_4;
						manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						manager->m_sQueueSample.m_nFrequency = 3000 * propellerSpeed + 30000;
					} else {
						manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_HELI_STA;
						manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						manager->m_sQueueSample.m_nFrequency = 3000 * propellerSpeed + 6000;
					}
					manager->m_sQueueSample.m_nCounter = 12;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nPriority = 1;
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_AIRCRAFT_SET_EMITTING_VOLUME(Vol);
					AUDIO_AIRCRAFT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_fSpeedMultiplier = 6.0f;
					manager->m_sQueueSample.m_MaxDistance = CAR_HELI_ENGINE_START_MAX_DIST;
					manager->m_sQueueSample.m_bStatic = 0;
					manager->m_sQueueSample.m_nFramesToPlay = 30;
					AUDIO_AIRCRAFT_SET_SOUND_REVERB(1);
					AUDIO_AIRCRAFT_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}

			//after accel rotor sound

			Vol = ((cameraAngle + 1.0f) * volumeModifier * MAX_VOLUME) / 2.0f;
			if (boat) {
				if (accelerateState > 0 || brakeState > 0)
					manager->m_sQueueSample.m_nFrequency = 9000 + AUDIO_AIRCRAFT_MIN(1.0f, (AUDIO_AIRCRAFT_MAX(accelerateState, brakeState) / 255) * freqModifier) * 1102;
				else
					manager->m_sQueueSample.m_nFrequency = 7143 + AUDIO_AIRCRAFT_MIN(1.0f, freqModifier) * 1857;

				if (propellerSpeed < 1.0f)
					manager->m_sQueueSample.m_nFrequency = (propellerSpeed + 1) * (manager->m_sQueueSample.m_nFrequency >> 1);

				manager->m_sQueueSample.m_nFrequency = AUDIO_AIRCRAFT_CLAMP(manager->m_sQueueSample.m_nFrequency, freqPropellerPrev, 98);
				freqPropellerPrev = manager->m_sQueueSample.m_nFrequency;
			}
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, CAR_HELI_ENGINE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = 1;
				if (hunterBool) {
					manager->m_sQueueSample.m_nSampleIndex = SFX_HELI_APACHE_2;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_nFrequency = (volumeModifier + 1) * 16000 + freq;
				} else if (boat) {
					manager->m_sQueueSample.m_nSampleIndex = SFX_SEAPLANE_PRO2;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				} else {
					manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_HELI_MAI2;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_nFrequency = (volumeModifier + 1) * 16000 + freq;
				}
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 1;
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_AIRCRAFT_SET_EMITTING_VOLUME(Vol);
				AUDIO_AIRCRAFT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				manager->m_sQueueSample.m_fSpeedMultiplier = 6.0f;
				manager->m_sQueueSample.m_MaxDistance = CAR_HELI_ENGINE_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 5;
				AUDIO_AIRCRAFT_SET_SOUND_REVERB(1);
				AUDIO_AIRCRAFT_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}

			if (boat) {
				if (AudioAircraftHost_FixedCamera() && manager->m_sQueueSample.m_fDistance < CAR_HELI_SEAPLANE_MAX_DIST && propellerSpeed > 0.0f) {
					manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(propellerSpeed * CAR_HELI_SEAPLANE_VOLUME, CAR_HELI_SEAPLANE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
					if (accelerateState > 0 || brakeState > 0)
						manager->m_sQueueSample.m_nFrequency = 18000 + AUDIO_AIRCRAFT_MIN(1.0f, (AUDIO_AIRCRAFT_MAX(accelerateState, brakeState) / 255.0f) * freqModifier) * 2204;
					else
						manager->m_sQueueSample.m_nFrequency = 14287 + AUDIO_AIRCRAFT_MIN(1.0f, freqModifier) * 3713;
					if (propellerSpeed < 1.0f)
						manager->m_sQueueSample.m_nFrequency = (propellerSpeed + 1) * (manager->m_sQueueSample.m_nFrequency >> 1);
					manager->m_sQueueSample.m_nFrequency = AUDIO_AIRCRAFT_CLAMP(manager->m_sQueueSample.m_nFrequency, freqSkimmerPrev, 197);
					freqSkimmerPrev = manager->m_sQueueSample.m_nFrequency;
					if (manager->m_sQueueSample.m_nVolume > 0) {
						manager->m_sQueueSample.m_nSampleIndex = SFX_SEAPLANE_PRO4;
						manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						manager->m_sQueueSample.m_nCounter = 12;
						manager->m_sQueueSample.m_bIs2D = 0;
						manager->m_sQueueSample.m_nPriority = 3;
						manager->m_sQueueSample.m_nLoopCount = 0;
						AUDIO_AIRCRAFT_SET_EMITTING_VOLUME(propellerSpeed * CAR_HELI_SEAPLANE_VOLUME);
						AUDIO_AIRCRAFT_SET_LOOP_OFFSETS(SFX_SEAPLANE_PRO4)
						manager->m_sQueueSample.m_fSpeedMultiplier = 5.0f;
						manager->m_sQueueSample.m_MaxDistance = CAR_HELI_SEAPLANE_MAX_DIST;
						manager->m_sQueueSample.m_bStatic = 0;
						manager->m_sQueueSample.m_nFramesToPlay = 7;
						AUDIO_AIRCRAFT_SET_SOUND_REVERB(1);
						AUDIO_AIRCRAFT_SET_SOUND_REFLECTION(0);
						AudioRequests_Submit(manager);
					}
				}
			} else {
				//vacuum cleaner sound
				// Copy the sound coordinates for the game vector operation
				vecPosOld = manager->m_sQueueSample.m_vecPos;
				distanceCalculatedOld = params->m_bDistanceCalculated;
				distanceOld = params->m_fDistance;

				// Copy the game position into the C sound record
				manager->m_sQueueSample.m_vecPos = backPropellerPos;
				params->m_bDistanceCalculated = 0;
				params->m_fDistance = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
				if (params->m_fDistance < (CAR_HELI_REAR_MAX_DIST * CAR_HELI_REAR_MAX_DIST)) {
					AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
					manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(volumeModifier * CAR_HELI_REAR_VOLUME, CAR_HELI_REAR_MAX_DIST, manager->m_sQueueSample.m_fDistance);
					if (manager->m_sQueueSample.m_nVolume > 0) {
						manager->m_sQueueSample.m_nCounter = 2;
						manager->m_sQueueSample.m_nSampleIndex = hunterBool ? SFX_HELI_APACHE_3 : SFX_CAR_HELI_REA;
						manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						manager->m_sQueueSample.m_bIs2D = 0;
						manager->m_sQueueSample.m_nPriority = 1;
						manager->m_sQueueSample.m_nFrequency = (volumeModifier + 1.0f) * 16000;
						manager->m_sQueueSample.m_nLoopCount = 0;
						AUDIO_AIRCRAFT_SET_EMITTING_VOLUME(volumeModifier * CAR_HELI_REAR_VOLUME);
						AUDIO_AIRCRAFT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
						manager->m_sQueueSample.m_fSpeedMultiplier = 6.0f;
						manager->m_sQueueSample.m_MaxDistance = CAR_HELI_REAR_MAX_DIST;
						manager->m_sQueueSample.m_bStatic = 0;
						manager->m_sQueueSample.m_nFramesToPlay = 5;
						AUDIO_AIRCRAFT_SET_SOUND_REVERB(1);
						AUDIO_AIRCRAFT_SET_SOUND_REFLECTION(0);
						AudioRequests_Submit(manager);
					}
				}
		
				// Copy the game position into the C sound record
				manager->m_sQueueSample.m_vecPos = vecPosOld;
				params->m_bDistanceCalculated = distanceCalculatedOld;
				params->m_fDistance = distanceOld;
			}
		}
		{
    // Complete the original camera snapshot lifecycle before returning
#ifdef GTA_PS2
    AudioAircraftHost_CameraEnd(cameraAttachment, cameraOwned);
#endif
    return 1;
}
	}
	{
    // Complete the original camera snapshot lifecycle before returning
#ifdef GTA_PS2
    AudioAircraftHost_CameraEnd(cameraAttachment, cameraOwned);
#endif
    return 0;
}
}

void
AudioAircraft_Cesna(cAudioManager *manager, cVehicleParams *params)
{
	// Generate the independent Cessna idle and rev loops at their original ranges
	if(params->m_fDistance < (CESNA_IDLE_MAX_DIST * CESNA_IDLE_MAX_DIST)) {
		AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(CESNA_VOLUME, CESNA_IDLE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 52;
			manager->m_sQueueSample.m_nSampleIndex = SFX_CESNA_IDLE;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nPriority = 0;
			manager->m_sQueueSample.m_nFrequency = 12500;
			manager->m_sQueueSample.m_nLoopCount = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 8;
			AUDIO_AIRCRAFT_SET_EMITTING_VOLUME(CESNA_VOLUME);
			AUDIO_AIRCRAFT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
			manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
			manager->m_sQueueSample.m_MaxDistance = CESNA_IDLE_MAX_DIST;
			manager->m_sQueueSample.m_bStatic = 0;
			AUDIO_AIRCRAFT_SET_SOUND_REVERB(1);
			AUDIO_AIRCRAFT_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
		}
		if(params->m_fDistance < (CESNA_REV_MAX_DIST * CESNA_REV_MAX_DIST)) {
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(CESNA_VOLUME, CESNA_REV_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = 2;
				manager->m_sQueueSample.m_nSampleIndex = SFX_CESNA_REV;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_nFrequency = 25000;
				manager->m_sQueueSample.m_nLoopCount = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 4;
				AUDIO_AIRCRAFT_SET_EMITTING_VOLUME(CESNA_VOLUME);
				AUDIO_AIRCRAFT_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
				manager->m_sQueueSample.m_MaxDistance = CESNA_REV_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				AUDIO_AIRCRAFT_SET_SOUND_REVERB(1);
				AUDIO_AIRCRAFT_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
	}
}

//- rouz edit (ChatGPT)
