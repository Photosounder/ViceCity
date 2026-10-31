//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "AudioVehicleModels.h"
#include "../modelinfo/ModelIds.h"
#include "../entities/EntityStatus.h"
#include "sampman.h"

enum {
	VEHICLE_HORN_MAX_DIST = 40,
	VEHICLE_HORN_VOLUME = 80,

	VEHICLE_SIREN_MAX_DIST = 110,
	VEHICLE_SIREN_VOLUME = 80,
};
#define AudioVehicleHorn_Min(a,b) ((a) < (b) ? (a) : (b))
#ifndef GTA_PS2
#define AUDIO_VEHICLE_HORN_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_HORN_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_HORN_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_HORN_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_HORN_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_HORN_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_HORN_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_HORN_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_HORN_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_HORN_SET_SOUND_REVERB(b)
#endif


static const uint8_t HornPattern[8][44] = {
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
     0, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1,
     1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0,
     0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1,  1,  1,  1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1,  1,  0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1,
     1,  1,  1, 1, 1, 0, 0, 0, 1,  1, 1, 1, 1, 1, 1,  1,  1,  1, 1, 1, 1, 0},
    {0, 0, 1,  1,  1,  1,  1,  1,  1,  1,  1,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1,  1,  1,  0, 0, 0, 1,  1,  1,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 1,  1,  0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1,  1,
     1,  1,  1, 1, 0, 0, 0, 0, 0, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1,  0, 0},
    {0, 0, 1,  1,  1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,  1,  1,  1,  1,
     0, 0, 0, 0, 1, 1, 1,  1,  1, 1, 1, 1, 1, 1,  1,  1,  1, 0, 0, 0, 0, 0},
};

static uint8_t AudioVehicleHorn_Uses(cVehicleParams *params);
static uint8_t AudioVehicleHorn_Switching(cVehicleParams *params);

uint8_t
AudioVehicleHorn_Horn(cAudioManager *manager, cVehicleParams *params)
{
    // Preserve the original horn, siren or switching operation and its query order
	if (params->m_fDistance < (VEHICLE_HORN_MAX_DIST * VEHICLE_HORN_MAX_DIST)) {
		if (AudioVehicleHornHost_Enabled(params->m_pVehicle) && AudioVehicleHorn_Switching(params))
			return 1;

		if (AudioVehicleReverseHost_Model(params->m_pVehicle) == MI_MRWHOOP)
			return 1;

		if (AudioVehicleHornHost_Alarm(params->m_pVehicle))
			return 1;

		if (AudioVehicleHornHost_Timer(params->m_pVehicle) > 0) {
			if (AudioVehicleEngineHost_Status(params->m_pVehicle) != STATUS_PLAYER) {
				// Resolve the original assignment target after computing its clamped right-hand value
                uint32_t clampedTimer = AudioVehicleHorn_Min(44, AudioVehicleHornHost_Timer(params->m_pVehicle));
                AudioVehicleHornHost_SetTimer(params->m_pVehicle, clampedTimer);
				if (AudioVehicleHornHost_Timer(params->m_pVehicle) == 44)
					AudioVehicleHornHost_SetPattern(params->m_pVehicle, (manager->m_FrameCounter + manager->m_sQueueSample.m_nEntityIndex) & 7);

				if (!HornPattern[AudioVehicleHornHost_Pattern(params->m_pVehicle)][44 - AudioVehicleHornHost_Timer(params->m_pVehicle)])
					return 1;
			}

			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(AudioVehicleModelHost_Drowning(params->m_pVehicle) ? VEHICLE_HORN_VOLUME / 4 : VEHICLE_HORN_VOLUME, VEHICLE_HORN_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = 4;
				manager->m_sQueueSample.m_nSampleIndex = AudioVehicleHornHost_HornSample(params->m_nIndex);
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 2;
				manager->m_sQueueSample.m_nFrequency = AudioVehicleHornHost_HornFrequency(params->m_nIndex);
				manager->m_sQueueSample.m_nLoopCount = 0;
#ifdef FIX_BUGS
				AUDIO_VEHICLE_HORN_SET_EMITTING_VOLUME(AudioVehicleModelHost_Drowning(params->m_pVehicle) ? VEHICLE_HORN_VOLUME / 4 : VEHICLE_HORN_VOLUME);
#else
				AUDIO_VEHICLE_HORN_SET_EMITTING_VOLUME(VEHICLE_HORN_VOLUME);
#endif
				AUDIO_VEHICLE_HORN_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				manager->m_sQueueSample.m_fSpeedMultiplier = 5.0f;
				manager->m_sQueueSample.m_MaxDistance = VEHICLE_HORN_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 4;
				AUDIO_VEHICLE_HORN_SET_SOUND_REVERB(1);
				AUDIO_VEHICLE_HORN_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
		return 1;
	}
	return 0;
}

static uint8_t
AudioVehicleHorn_Uses(cVehicleParams *params)
{
    // Preserve the original horn, siren or switching operation and its query order
	return AudioVehicleHornHost_Uses(params->m_pVehicle);
}

static uint8_t
AudioVehicleHorn_Switching(cVehicleParams *params)
{
    // Preserve the original horn, siren or switching operation and its query order
	if (params->m_nIndex == FIRETRUK || params->m_nIndex == MRWHOOP)
		return 0;
	return AudioVehicleHorn_Uses(params);
}

uint8_t
AudioVehicleHorn_Siren(cAudioManager *manager, cVehicleParams *params)
{
    // Preserve the original horn, siren or switching operation and its query order
	uint8_t Vol;

	if (params->m_fDistance < (VEHICLE_SIREN_MAX_DIST * VEHICLE_SIREN_MAX_DIST)) {
		CVehicle *veh = params->m_pVehicle;
		if (AudioVehicleHornHost_Enabled(veh) || AudioVehicleHornHost_Alarm(veh)) {
			if (AudioVehicleHornHost_Alarm(veh)) {
				if (AudioPlayerEngineHost_Time() > AudioVehicleHornHost_Timer(veh))
					AudioVehicleHornHost_SetTimer(veh, AudioPlayerEngineHost_Time() + 750);

				if (AudioVehicleHornHost_Timer(veh) < AudioPlayerEngineHost_Time() + 375)
					return 1;
			}

			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			Vol = AudioVehicleModelHost_Drowning(veh) ? VEHICLE_SIREN_VOLUME / 4 : VEHICLE_SIREN_VOLUME;
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, VEHICLE_SIREN_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = 5;
				if (AudioVehicleHorn_Uses(params)) {
					if (AudioVehicleEngineHost_Status(params->m_pVehicle) == STATUS_ABANDONED)
						return 1;
					if (AudioVehicleHornHost_Timer(veh) > 0 && params->m_nIndex != FIRETRUK && params->m_nIndex != MRWHOOP) {
						manager->m_sQueueSample.m_nSampleIndex = SFX_SIREN_FAST;
						if (params->m_nIndex == FBIRANCH)
							manager->m_sQueueSample.m_nFrequency = 12668;
						else
							manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_SIREN_FAST);
						manager->m_sQueueSample.m_nCounter = 60;
					} else if (params->m_nIndex == VICECHEE) {
						manager->m_sQueueSample.m_nSampleIndex = SFX_POLICE_SIREN_SLOW;
						manager->m_sQueueSample.m_nFrequency = 11440;
					} else {
						manager->m_sQueueSample.m_nSampleIndex = AudioVehicleHornHost_SirenSample(params->m_nIndex);
						manager->m_sQueueSample.m_nFrequency = AudioVehicleHornHost_SirenFrequency(params->m_nIndex);
					}
				} else {
					manager->m_sQueueSample.m_nSampleIndex = AudioVehicleHornHost_HornSample(params->m_nIndex);
					manager->m_sQueueSample.m_nFrequency = AudioVehicleHornHost_HornFrequency(params->m_nIndex);
				}
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 1;
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_VEHICLE_HORN_SET_EMITTING_VOLUME(Vol);
				AUDIO_VEHICLE_HORN_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				manager->m_sQueueSample.m_fSpeedMultiplier = 7.0f;
				manager->m_sQueueSample.m_MaxDistance = VEHICLE_SIREN_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 5;
				AUDIO_VEHICLE_HORN_SET_SOUND_REVERB(1);
				AUDIO_VEHICLE_HORN_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
		return 1;
	}
	return 0;
}

//- rouz edit (ChatGPT)
