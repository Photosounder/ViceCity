//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "sampman.h"

enum {
	REVERSE_GEAR_MAX_DIST = 30,
	REVERSE_GEAR_VOLUME = 24,
};

#define AudioVehicleReverse_Abs(a) (((a) < 0) ? (-(a)) : (a))
#ifndef GTA_PS2
#define AUDIO_VEHICLE_REVERSE_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_REVERSE_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_REVERSE_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_REVERSE_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_REVERSE_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_REVERSE_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_REVERSE_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_REVERSE_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_REVERSE_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_REVERSE_SET_SOUND_REVERB(b)
#endif


uint8_t
AudioVehicleReverse_Process(cAudioManager *manager, cVehicleParams *params)
{
    // Retain reverse eligibility, vehicle capture timing and sound request ordering
	CVehicle *veh;
	CVehicle *automobile;
	uint8_t Vol;
	float modificator;

	if (params->m_fDistance < (REVERSE_GEAR_MAX_DIST * REVERSE_GEAR_MAX_DIST)) {
		veh = params->m_pVehicle;
		if (AudioVehicleReverseHost_Model(veh) == MI_CADDY)
			return 1;
		if (AudioVehicleReverseHost_Engine(veh) && (AudioVehicleReverseHost_Gas(veh) < 0.0f || AudioVehicleReverseHost_Gear(veh) == 0)) {
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			automobile = params->m_pVehicle;
			if (AudioVehicleReverseHost_DriveWheels(automobile) > 0)
				modificator = params->m_fVelocityChange / (params->m_pTransmission->fMaxReverseVelocity);
			else {
				if (AudioVehicleReverseHost_PreviousDriveWheels(automobile) > 0)
					(*AudioVehicleReverseHost_GasAudio(automobile)) *= 0.4f;
				modificator = (*AudioVehicleReverseHost_GasAudio(automobile));
			}
			modificator = AudioVehicleReverse_Abs(modificator);
			Vol = (REVERSE_GEAR_VOLUME * modificator);
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, REVERSE_GEAR_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				if (AudioVehicleReverseHost_Gas(params->m_pVehicle) < 0.0f) {
					manager->m_sQueueSample.m_nCounter = 61;
					manager->m_sQueueSample.m_nSampleIndex = SFX_REVERSE_GEAR;
				} else {
					manager->m_sQueueSample.m_nCounter = 62;
					manager->m_sQueueSample.m_nSampleIndex = SFX_REVERSE_GEAR_2;
				}
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_nFrequency = (6000 * modificator) + 7000;
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_VEHICLE_REVERSE_SET_EMITTING_VOLUME(Vol);
				AUDIO_VEHICLE_REVERSE_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
				manager->m_sQueueSample.m_MaxDistance = REVERSE_GEAR_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 5;
				AUDIO_VEHICLE_REVERSE_SET_SOUND_REVERB(1);
				AUDIO_VEHICLE_REVERSE_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
		return 1;
	}
	return 0;
}
//- rouz edit (ChatGPT)
