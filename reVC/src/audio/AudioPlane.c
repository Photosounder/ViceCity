//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "AudioVehicleModels.h"
#include "sampman.h"

enum {
	JUMBO_MAX_DIST = 440,
	JUMBO_RUMBLE_SOUND_MAX_DIST = 240,
	JUMBO_ENGINE_SOUND_MAX_DIST = 180,
	JUMBO_WHINE_SOUND_MAX_DIST = 170,
};
#ifndef GTA_PS2
#define AUDIO_PLANE_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_PLANE_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_PLANE_RESET_LOOP_OFFSETS
#define AUDIO_PLANE_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_PLANE_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_PLANE_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_PLANE_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_PLANE_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_PLANE_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_PLANE_SET_SOUND_REVERB(b)
#endif


static uint8_t gJumboVolOffsetPercentage;

static uint8_t AudioPlane_Less(float a, float b)
{
	// Preserve the repeated original speed read after the comparison
	volatile uint8_t result = a < b;
	return result;
}
#define AUDIO_PLANE_MIN(a,b) (AudioPlane_Less((a),(b)) ? (a) : (b))

static void AudioPlane_Offset(void)
{
	// Update the shared offset from the global owner at the original cadence
	if (!(AudioManager.m_FrameCounter % (AudioManager.m_anRandomTable[0] % 6 + 3)))
		gJumboVolOffsetPercentage = AudioManager.m_anRandomTable[1] % 60;
}

void AudioPlane_Plane(cAudioManager *manager, cVehicleParams *params)
{
	// Route audio-table plane indices to Jumbo or the existing Cessna generator
	switch (params->m_nIndex) {
	case AIRTRAIN:
		AudioPlane_Jumbo(manager, params);
		break;
	case DEADDODO:
		AudioAircraft_Cesna(manager, params);
		break;
	default:
		break;
	}
}

void AudioPlane_Jumbo(cAudioManager *manager, cVehicleParams *params)
{
	// Calculate the cached distance and route the captured plane through its flight state
	CVehicle *plane;
	float position;

	if (params->m_fDistance < (JUMBO_MAX_DIST * JUMBO_MAX_DIST)) {
		AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
		plane = params->m_pVehicle;
		AudioPlane_Offset();
		position = AudioPlaneHost_Position(plane);
		if (position > AudioPlaneHost_TakeOffPoint()) {
			if (300.0f + AudioPlaneHost_TakeOffPoint() < position) {
				if (AudioPlaneHost_LandingPoint() - 350.0f < position) {
					if (position > AudioPlaneHost_LandingPoint()) {
						if (AudioPlaneHost_Speed(plane) > 0.103344f)
							AudioPlane_JumboDecel(manager, plane);
						else
							AudioPlane_JumboTaxi(manager);
					}
					else
						AudioPlane_JumboLanding(manager, plane);
				}
				else
					AudioPlane_JumboFlying(manager);
			} else
				AudioPlane_JumboTakeOff(manager, plane);
		} else {
			if (AudioPlaneHost_Speed(plane) > 0.103344f)
				AudioPlane_JumboAccel(manager, plane);
			else
				AudioPlane_JumboTaxi(manager);
		}
	}
}

void AudioPlane_JumboTaxi(cAudioManager *manager)
{
	// Submit the taxi layers in their original conditional order
	if (AudioPlane_JumboFlySound(manager, 20)) {
		if (AudioPlane_JumboTaxiSound(manager, 75))
			AudioPlane_JumboWhineSound(manager, 18, 29500);
	}
}

void AudioPlane_JumboAccel(cAudioManager *manager, CVehicle *plane)
{
	// Scale acceleration layers from the captured plane speed and retain short-circuit gates
	uint32_t engineFreq;
	uint8_t vol;
	float modificator;
	float freqMult;

	if (AudioPlane_JumboFlySound(manager, 20)) {
		modificator = AUDIO_PLANE_MIN(1.0f, (AudioPlaneHost_Speed(plane) - 0.103344f) * 1.6760077f);
		if (AudioPlane_JumboRumbleSound(manager, MAX_VOLUME * modificator) && AudioPlane_JumboTaxiSound(manager, (1.0f - modificator) * 75)) {
			if (modificator >= 0.2f) {
				freqMult = 1;
				engineFreq = 22050;
				vol = MAX_VOLUME;
			} else {
				freqMult = modificator * 5;
				vol = freqMult * MAX_VOLUME;
				engineFreq = freqMult * 6050 + 16000;
			}
			AudioPlane_JumboEngineSound(manager, vol, engineFreq);
			AudioPlane_JumboWhineSound(manager, 18, 14600 * freqMult + 29500);
		}
	}
}

void AudioPlane_JumboTakeOff(cAudioManager *manager, CVehicle *plane)
{
	// Blend takeoff layers from the current path position and takeoff threshold
	float modificator = (AudioPlaneHost_Position(plane) - AudioPlaneHost_TakeOffPoint()) / 300;
	if (AudioPlane_JumboFlySound(manager, (107 * modificator) + 20) && AudioPlane_JumboRumbleSound(manager, MAX_VOLUME * (1.0f - modificator))) {
		if (AudioPlane_JumboEngineSound(manager, MAX_VOLUME, 22050))
			AudioPlane_JumboWhineSound(manager, 18 * (1.0f - modificator), 44100);
	}
}

void AudioPlane_JumboFlying(cAudioManager *manager)
{
	// Submit the distant fly layer before the cruising engine layer
	if (AudioPlane_JumboFlySound(manager, MAX_VOLUME))
		AudioPlane_JumboEngineSound(manager, 63, 22050);
}

void AudioPlane_JumboLanding(cAudioManager *manager, CVehicle *plane)
{
	// Blend landing layers from the current path position and landing threshold
	float modificator = (AudioPlaneHost_LandingPoint() - AudioPlaneHost_Position(plane)) / 350;
	if (AudioPlane_JumboFlySound(manager, 107 * modificator + 20)) {
		if (AudioPlane_JumboTaxiSound(manager, 75 * (1.0f - modificator))) {
			AudioPlane_JumboEngineSound(manager, MAX_VOLUME, 22050);
			AudioPlane_JumboWhineSound(manager, 18 * (1.0f - modificator), 14600 * modificator + 29500);
		}
	}
}

void AudioPlane_JumboDecel(cAudioManager *manager, CVehicle *plane)
{
	// Scale deceleration layers only after the fly and taxi distance gates pass
	if (AudioPlane_JumboFlySound(manager, 20) && AudioPlane_JumboTaxiSound(manager, 75)) {
		float modificator = AUDIO_PLANE_MIN(1.0f, (AudioPlaneHost_Speed(plane) - 0.103344f) * 1.6760077f);
		AudioPlane_JumboEngineSound(manager, MAX_VOLUME * modificator, 6050* modificator + 16000);
		AudioPlane_JumboWhineSound(manager, 18, 29500);
	}
}

uint8_t AudioPlane_JumboTaxiSound(cAudioManager *manager, uint8_t vol)
{
	// Apply the original integer offset and distance scaling before setting up taxi audio
	if (manager->m_sQueueSample.m_fDistance < JUMBO_ENGINE_SOUND_MAX_DIST) {
		uint8_t Vol = (vol >> 1) + ((vol >> 1) * manager->m_sQueueSample.m_fDistance / JUMBO_ENGINE_SOUND_MAX_DIST);

		if (manager->m_sQueueSample.m_fDistance / JUMBO_ENGINE_SOUND_MAX_DIST < 0.7f)
			Vol -= Vol * gJumboVolOffsetPercentage / 100;
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, JUMBO_ENGINE_SOUND_MAX_DIST, manager->m_sQueueSample.m_fDistance);

		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_JUMBO_TAXI;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_nFrequency = AudioPlane_JumboTaxiFreq(manager);
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_PLANE_SET_EMITTING_VOLUME(Vol);
			AUDIO_PLANE_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
			manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
			manager->m_sQueueSample.m_MaxDistance = JUMBO_ENGINE_SOUND_MAX_DIST;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 4;
			AUDIO_PLANE_SET_SOUND_REVERB(1);
			AUDIO_PLANE_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
		}
		return 1;
	}
	return 0;
}

uint8_t AudioPlane_JumboWhineSound(cAudioManager *manager, uint8_t Vol, uint32_t freq)
{
	// Retain the whine range gate and partial sound record writes
	if (manager->m_sQueueSample.m_fDistance < JUMBO_WHINE_SOUND_MAX_DIST) {
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, JUMBO_WHINE_SOUND_MAX_DIST, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 2;
			manager->m_sQueueSample.m_nSampleIndex = SFX_JUMBO_WHINE;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_nFrequency = freq;
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_PLANE_SET_EMITTING_VOLUME(Vol);
			AUDIO_PLANE_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
			manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
			manager->m_sQueueSample.m_MaxDistance = JUMBO_WHINE_SOUND_MAX_DIST;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 4;
			AUDIO_PLANE_SET_SOUND_REVERB(1);
			AUDIO_PLANE_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
		}
		return 1;
	}
	return 0;
}

uint8_t AudioPlane_JumboEngineSound(cAudioManager *manager, uint8_t Vol, uint32_t freq)
{
	// Retain integer offset division and the engine range gate
	if (manager->m_sQueueSample.m_fDistance < JUMBO_ENGINE_SOUND_MAX_DIST) {
		uint8_t FinalVol = Vol - gJumboVolOffsetPercentage / 100;
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(FinalVol, JUMBO_ENGINE_SOUND_MAX_DIST, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 3;
			manager->m_sQueueSample.m_nSampleIndex = SFX_JUMBO_ENGINE;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_nFrequency = freq;
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_PLANE_SET_EMITTING_VOLUME(FinalVol);
			AUDIO_PLANE_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
			manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
			manager->m_sQueueSample.m_MaxDistance = JUMBO_ENGINE_SOUND_MAX_DIST;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 4;
			AUDIO_PLANE_SET_SOUND_REVERB(1);
			AUDIO_PLANE_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
		}
		return 1;
	}
	return 0;
}

uint8_t AudioPlane_JumboFlySound(cAudioManager *manager, uint8_t Vol)
{
	// Set up the distant fly layer using the original sample base frequency
	if (manager->m_sQueueSample.m_fDistance < JUMBO_MAX_DIST) {
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, JUMBO_MAX_DIST, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 0;
			manager->m_sQueueSample.m_nSampleIndex = SFX_JUMBO_DIST_FLY;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_JUMBO_DIST_FLY);
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_PLANE_SET_EMITTING_VOLUME(Vol);
			AUDIO_PLANE_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
			manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
			manager->m_sQueueSample.m_MaxDistance = JUMBO_MAX_DIST;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 5;
			AUDIO_PLANE_SET_SOUND_REVERB(1);
			AUDIO_PLANE_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
		}
		return 1;
	}
	return 0;
}

uint8_t AudioPlane_JumboRumbleSound(cAudioManager *manager, uint8_t Vol)
{
	// Set up both stereo rumble requests with the second frequency read from live scratch storage
	if (manager->m_sQueueSample.m_fDistance < JUMBO_RUMBLE_SOUND_MAX_DIST) {
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, JUMBO_RUMBLE_SOUND_MAX_DIST, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 5;
			manager->m_sQueueSample.m_nSampleIndex = SFX_JUMBO_RUMBLE;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_bIs2D = 1;
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_JUMBO_RUMBLE);
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_PLANE_SET_EMITTING_VOLUME(Vol);
			AUDIO_PLANE_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
			manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
			manager->m_sQueueSample.m_MaxDistance = JUMBO_RUMBLE_SOUND_MAX_DIST;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 12;
			manager->m_sQueueSample.m_nPan = 0;
			AUDIO_PLANE_SET_SOUND_REVERB(1);
			AUDIO_PLANE_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
			manager->m_sQueueSample.m_nCounter = 6;
			manager->m_sQueueSample.m_nSampleIndex = SFX_JUMBO_RUMBLE;
			manager->m_sQueueSample.m_nFrequency += 200;
			manager->m_sQueueSample.m_nPan = 127;
			AudioRequests_Submit(manager);
		}
		return 1;
	}
	return 0;
}

int32_t AudioPlane_JumboTaxiFreq(cAudioManager *manager)
{
	// Retain the original floating operation order and integer frequency conversion
	return (1.0f / 180 * 10950 * manager->m_sQueueSample.m_fDistance) + 22050; // todo port fix to re3
}

//- rouz edit (ChatGPT)
