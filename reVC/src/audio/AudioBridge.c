//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "soundlist.h"
#include "sampman.h"
#include "../control/BridgeStates.h"

#ifdef GTA_BRIDGE
enum {
	BRIDGE_MOTOR_MAX_DIST = 400,
	BRIDGE_MOTOR_VOLUME = MAX_VOLUME,
	BRIDGE_MAX_DIST = BRIDGE_MOTOR_MAX_DIST + 50,

	BRIDGE_WARNING_VOLUME = 100,
};

#ifndef GTA_PS2
#define AUDIO_BRIDGE_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_BRIDGE_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_BRIDGE_RESET_LOOP_OFFSETS
#define AUDIO_BRIDGE_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_BRIDGE_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_BRIDGE_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_BRIDGE_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_BRIDGE_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_BRIDGE_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_BRIDGE_SET_SOUND_REVERB(b)
#endif

void
AudioBridge_Process(cAudioManager *manager)
{
	// Preserve the complete original bridge generator and live query order
	float dist;
	uint8_t distCalculated = 0;

	if (AudioBridgeHost_Road()) {
		// Copy the game position into the C sound record
		AudioReleaseHost_Position(AudioBridgeHost_Road(), &manager->m_sQueueSample.m_vecPos);
		dist = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
		if (dist < (BRIDGE_MAX_DIST * BRIDGE_MAX_DIST)) {
			AudioMath_CalculateDistance(&distCalculated, &manager->m_sQueueSample.m_fDistance, dist);
			switch (AudioBridgeHost_State()) {
			case STATE_BRIDGE_LOCKED:
			case STATE_LIFT_PART_IS_UP:
			case STATE_LIFT_PART_ABOUT_TO_MOVE_UP:
				AudioBridge_Warning(manager);
				break;
			case STATE_LIFT_PART_MOVING_DOWN:
			case STATE_LIFT_PART_MOVING_UP:
				AudioBridge_Warning(manager);
				AudioBridge_Motor(manager);
				break;
			default:
				break;
			}
			AudioBridge_OneShots(manager);
		}
	}
}

void
AudioBridge_Warning(cAudioManager *manager)
{
	// Preserve the complete original bridge generator and live query order
	if (!AudioBridgeHost_CommercialPassed())
		return;

	if (manager->m_sQueueSample.m_fDistance < BRIDGE_MAX_DIST) {
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(BRIDGE_WARNING_VOLUME, BRIDGE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 0;
			manager->m_sQueueSample.m_nSampleIndex = SFX_BRIDGE_OPEN_WARNING;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_GENERIC_EXTRA;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_BRIDGE_OPEN_WARNING);
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_BRIDGE_SET_EMITTING_VOLUME(BRIDGE_WARNING_VOLUME);
			AUDIO_BRIDGE_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
			manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			manager->m_sQueueSample.m_MaxDistance = BRIDGE_MAX_DIST;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 8;
			AUDIO_BRIDGE_SET_SOUND_REVERB(0);
			AUDIO_BRIDGE_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
		}
	}
}

void
AudioBridge_Motor(cAudioManager *manager)
{
	// Preserve the complete original bridge generator and live query order
	if (manager->m_sQueueSample.m_fDistance < BRIDGE_MOTOR_MAX_DIST) {
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(BRIDGE_MOTOR_VOLUME, BRIDGE_MOTOR_MAX_DIST, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_FISHING_BOAT_IDLE; // todo check sfx name
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_nFrequency = 5500;
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_BRIDGE_SET_EMITTING_VOLUME(BRIDGE_MOTOR_VOLUME);
			AUDIO_BRIDGE_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
			manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			manager->m_sQueueSample.m_MaxDistance = BRIDGE_MOTOR_MAX_DIST;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 3;
			AUDIO_BRIDGE_SET_SOUND_REVERB(0);
			AudioRequests_Submit(manager);
		}
	}
}

void
AudioBridge_OneShots(cAudioManager *manager)
{
	// Preserve the complete original bridge generator and live query order
	float maxDist;

	if (AudioBridgeHost_State() == STATE_LIFT_PART_IS_UP && AudioBridgeHost_OldState() == STATE_LIFT_PART_MOVING_UP) {
		maxDist = BRIDGE_MOTOR_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_COL_CONTAINER_1;
	} else if (AudioBridgeHost_State() == STATE_LIFT_PART_IS_DOWN && AudioBridgeHost_OldState() == STATE_LIFT_PART_MOVING_DOWN) {
		maxDist = BRIDGE_MOTOR_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_COL_CONTAINER_1;
	} else if (AudioBridgeHost_State() == STATE_LIFT_PART_MOVING_UP && AudioBridgeHost_OldState() == STATE_LIFT_PART_ABOUT_TO_MOVE_UP) {
		maxDist = BRIDGE_MOTOR_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_COL_CONTAINER_1;
	} else if (AudioBridgeHost_State() == STATE_LIFT_PART_MOVING_DOWN && AudioBridgeHost_OldState() == STATE_LIFT_PART_IS_UP) {
		maxDist = BRIDGE_MOTOR_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_COL_CONTAINER_1;
	} else return;

	if (manager->m_sQueueSample.m_fDistance < maxDist) {
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(BRIDGE_MOTOR_VOLUME, maxDist, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 2;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_BRIDGE_SET_EMITTING_VOLUME(BRIDGE_MOTOR_VOLUME);
			AUDIO_BRIDGE_RESET_LOOP_OFFSETS
			manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			manager->m_sQueueSample.m_MaxDistance = maxDist;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_BRIDGE_SET_SOUND_REVERB(0);
			AUDIO_BRIDGE_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
		}
	}
}

#endif
//- rouz edit (ChatGPT)
