//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "soundlist.h"
#include "sampman.h"
#include <math.h>

enum {
	FRONTEND_VOLUME = 127,
};

#define AUDIO_WEATHER_FRONTEND_MAX(a,b) ((a) > (b) ? (a) : (b))
#define AUDIO_WEATHER_FRONTEND_MIN(a,b) ((a) < (b) ? (a) : (b))

#ifndef GTA_PS2
#define AUDIO_WEATHER_FRONTEND_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_WEATHER_FRONTEND_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_WEATHER_FRONTEND_RESET_LOOP_OFFSETS
#define AUDIO_WEATHER_FRONTEND_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_WEATHER_FRONTEND_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_WEATHER_FRONTEND_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_WEATHER_FRONTEND_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_WEATHER_FRONTEND_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_WEATHER_FRONTEND_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_WEATHER_FRONTEND_SET_SOUND_REVERB(b)
#endif

void
AudioWeather_Process(cAudioManager *manager, int32_t id)
{
	// Preserve the original sound decisions, shared statics and live game query order
	uint8_t Vol;
	float x;
	float y;
	float modifier;
	float wind;

	static uint8_t iSound = 0;

	if (manager->m_asAudioEntities[id].m_AudioEvents > 0 && manager->m_asAudioEntities[id].m_awAudioEvent[0] == SOUND_LIGHTNING) {
		if (manager->m_asAudioEntities[id].m_afVolume[0] < 10) {
			manager->m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_2;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 500) + 4000;
			Vol = (manager->m_asAudioEntities[id].m_afVolume[0] * 10.0f * 0.1f);
			Vol += 35;
		} else {
			manager->m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_1;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_GENERIC_EXTRA;
			manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 500) + 4000;
			Vol = ((manager->m_asAudioEntities[id].m_afVolume[0] - 10.0f) * 10.0f * 0.1f);
			Vol += 40;
		}
		manager->m_sQueueSample.m_nVolume = Vol;
		if (AudioWeatherHost_SoundDistUp() < 20.0f)
			manager->m_sQueueSample.m_nVolume >>= 1;
		if (iSound == 4)
			iSound = 0;
		manager->m_sQueueSample.m_nCounter = iSound++;
		manager->m_sQueueSample.m_nPriority = 0;
		manager->m_sQueueSample.m_nPan = (manager->m_anRandomTable[2] % 16) + 55;
		manager->m_sQueueSample.m_bIs2D = 1;
		manager->m_sQueueSample.m_nLoopCount = 1;
		manager->m_sQueueSample.m_bStatic = 1;
		AUDIO_WEATHER_FRONTEND_SET_EMITTING_VOLUME(manager->m_sQueueSample.m_nVolume);
		AUDIO_WEATHER_FRONTEND_RESET_LOOP_OFFSETS
		AUDIO_WEATHER_FRONTEND_SET_SOUND_REVERB(0);
		AUDIO_WEATHER_FRONTEND_SET_SOUND_REFLECTION(0);
		AudioRequests_Submit(manager);
	}
	if (AudioVehicleRainHost_Rain() > 0.0f && (!AudioVehicleRainHost_CameraNoRain() || !AudioVehicleRainHost_PlayerNoRain())) {
		manager->m_sQueueSample.m_nSampleIndex = SFX_RAIN;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_RAIN);
		manager->m_sQueueSample.m_nVolume = (uint8_t)(25.0f * AudioVehicleRainHost_Rain());
		manager->m_sQueueSample.m_nCounter = 4;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nPriority = 0;
		manager->m_sQueueSample.m_nPan = 63;
		manager->m_sQueueSample.m_bIs2D = 1;
		manager->m_sQueueSample.m_nLoopCount = 0;
		manager->m_sQueueSample.m_bStatic = 0;
		manager->m_sQueueSample.m_nFramesToPlay = 30;
		AUDIO_WEATHER_FRONTEND_SET_SOUND_REVERB(0);
		AUDIO_WEATHER_FRONTEND_SET_EMITTING_VOLUME(manager->m_sQueueSample.m_nVolume);
		AUDIO_WEATHER_FRONTEND_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
		AUDIO_WEATHER_FRONTEND_SET_SOUND_REFLECTION(0);
		AudioRequests_Submit(manager);
	}
	x = 0.0f;
	y = 0.0f;
	AudioWeatherHost_WindModifier(&x, &y);
	modifier = AUDIO_WEATHER_FRONTEND_MAX(fabsf(x), fabsf(y)) * 10.0f;
	modifier = AUDIO_WEATHER_FRONTEND_MIN(1.0f, modifier);
	// Keep the original wind comparison read separate from its selected-value read
	volatile float comparedWind = AudioWeatherHost_Wind();
	if (comparedWind > modifier)
		wind = AudioWeatherHost_Wind();
	else
		wind = modifier;
	if (wind > 0.0f && AudioWeatherHost_NearestTree() < 75.0f) {
		manager->m_sQueueSample.m_nSampleIndex = SFX_PALM_TREE_LO;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_PALM_TREE_LO);
		manager->m_sQueueSample.m_nVolume = (manager->m_anRandomTable[1] % 10 + 45.0f) * (75.0f - AudioWeatherHost_NearestTree()) / 75.0f * wind;
		manager->m_sQueueSample.m_nCounter = 5;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nPriority = 1;
		manager->m_sQueueSample.m_nPan = 63;
		manager->m_sQueueSample.m_bIs2D = 1;
		manager->m_sQueueSample.m_nLoopCount = 0;
		manager->m_sQueueSample.m_bStatic = 0;
		manager->m_sQueueSample.m_nFramesToPlay = 7;
		AUDIO_WEATHER_FRONTEND_SET_SOUND_REVERB(0);
		AUDIO_WEATHER_FRONTEND_SET_EMITTING_VOLUME(manager->m_sQueueSample.m_nVolume);
		AUDIO_WEATHER_FRONTEND_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
		AUDIO_WEATHER_FRONTEND_SET_SOUND_REFLECTION(0);
		AudioRequests_Submit(manager);
		AudioWeatherHost_ResetNearestTree();
	}
}

void
AudioFrontEnd_Process(cAudioManager *manager)
{
	// Preserve the original sound decisions, shared statics and live game query order
	uint8_t stereo;
	uint8_t processedPickup;
	uint8_t processedMission;
	uint8_t staticFreq;
	uint8_t center;
	int16_t sample;

	static uint8_t iSound = 0;
	static uint32_t cPickupNextFrame = 0;
	static uint32_t cPartMisComNextFrame = 0;
	static uint32_t radioDial = SFX_RADIO_DIAL_1;

	for (uint32_t i = 0; i < manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_AudioEvents; i++) {
		staticFreq = 0;
		processedPickup = 0;
		center = 0;
		processedMission = 0;
		stereo = 0;
		switch (manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i]) {
		case SOUND_WEAPON_SNIPER_SHOT_NO_ZOOM:
			manager->m_sQueueSample.m_nSampleIndex = SFX_ERROR_FIRE_RIFLE;
			break;
		case SOUND_WEAPON_ROCKET_SHOT_NO_ZOOM:
			manager->m_sQueueSample.m_nSampleIndex = SFX_ERROR_FIRE_ROCKET_LAUNCHER;
			break;
		case SOUND_GARAGE_NO_MONEY:
		case SOUND_GARAGE_BAD_VEHICLE:
		case SOUND_GARAGE_BOMB_ALREADY_SET:
			manager->m_sQueueSample.m_nSampleIndex = SFX_WEAPON_LEFT;
			stereo = 1;
			staticFreq = 1;
			center = 1;
			break;
		case SOUND_GARAGE_OPENING:
		case SOUND_71: //case SOUND_41:
		case SOUND_GARAGE_VEHICLE_DECLINED:
		case SOUND_GARAGE_VEHICLE_ACCEPTED:
		case SOUND_EVIDENCE_PICKUP:
		case SOUND_UNLOAD_GOLD:
			stereo = 1;
			processedPickup = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_MONEY_LEFT;
			break;
		case SOUND_GARAGE_BOMB1_SET:
		case SOUND_GARAGE_BOMB2_SET:
		case SOUND_GARAGE_BOMB3_SET:
		case SOUND_PICKUP_WEAPON_BOUGHT:
		case SOUND_PICKUP_WEAPON:
			center = 1;
			processedPickup = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_WEAPON_LEFT;
			stereo = 1;
			break;
		case SOUND_PICKUP_HEALTH:
		case SOUND_81: //case SOUND_4B:
		case SOUND_PICKUP_ADRENALINE:
		case SOUND_PICKUP_ARMOUR:
			stereo = 1;
			processedPickup = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_MONEY_LEFT;
			break;
		case SOUND_80:
			stereo = 1;
			processedPickup = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_WEAPON_LEFT;
			center = 1;
			staticFreq = 1;
			break;
		case SOUND_PICKUP_BONUS:
		case SOUND_FRONTEND_MENU_STARTING:
		case SOUND_HUD:
			stereo = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_INFO_LEFT;
			center = 1;
			break;
		case SOUND_PICKUP_MONEY:
			stereo = 1;
			processedPickup = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_MONEY_LEFT;
			break;
		case SOUND_PICKUP_HIDDEN_PACKAGE:
		case SOUND_PICKUP_PACMAN_PILL:
		case SOUND_PICKUP_PACMAN_PACKAGE:
		case SOUND_PICKUP_FLOAT_PACKAGE:
			center = 1;
			processedPickup = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_PART_MISSION_COMPLETE_LEFT;
			stereo = 1;
			break;
		case SOUND_RACE_START_3:
		case SOUND_RACE_START_2:
		case SOUND_RACE_START_1:
		case SOUND_PART_MISSION_COMPLETE:
			stereo = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_PART_MISSION_COMPLETE_LEFT;
			processedMission = 1;
			center = 1;
			break;
		case SOUND_RACE_START_GO:
			stereo = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_GO_LEFT;
			center = 1;
			break;
		case SOUND_CLOCK_TICK:
			manager->m_sQueueSample.m_nSampleIndex = SFX_TIMER;
			break;
		case SOUND_FRONTEND_RADIO_TURN_OFF:
		case SOUND_FRONTEND_RADIO_TURN_ON:
			manager->m_sQueueSample.m_nSampleIndex = SFX_RADIO_CLICK;
			break;
		case SOUND_FRONTEND_HURRICANE:
			manager->m_sQueueSample.m_nSampleIndex = SFX_HURRICANE_MA;
			break;
		case SOUND_BULLETTRACE_1:
		case SOUND_BULLETTRACE_2:
			manager->m_sQueueSample.m_nSampleIndex = (manager->m_anRandomTable[0] % 2) + SFX_BULLET_PASS_1;
			break;
		case SOUND_AMMUNATION_IMRAN_ARM_BOMB:
			manager->m_sQueueSample.m_nSampleIndex = SFX_ARM_BOMB;
			break;
		case SOUND_RADIO_CHANGE:
			manager->m_sQueueSample.m_nSampleIndex = (manager->m_anRandomTable[1] % 2) ? radioDial + 1 : radioDial + 2;
			if (manager->m_sQueueSample.m_nSampleIndex > SFX_RADIO_DIAL_12)
				manager->m_sQueueSample.m_nSampleIndex -= 12;
			radioDial = manager->m_sQueueSample.m_nSampleIndex;
			break;
		case SOUND_FRONTEND_HIGHLIGHT_OPTION:
			stereo = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_FE_HIGHLIGHT_LEFT;
			break;
		case SOUND_FRONTEND_ENTER_OR_ADJUST:
			stereo = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_FE_SELECT_LEFT;
			break;
		case SOUND_FRONTEND_BACK:
			stereo = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_FE_BACK_LEFT;
			break;
		case SOUND_FRONTEND_FAIL:
			stereo = 1;
			manager->m_sQueueSample.m_nSampleIndex = SFX_FE_ERROR_LEFT;
			break;
		case SOUND_FRONTEND_AUDIO_TEST:
			manager->m_sQueueSample.m_nSampleIndex = manager->m_anRandomTable[0] % 3 + SFX_FE_NOISE_BURST_1;
			break;
		default:
			continue;
		}

		if (processedPickup) {
			if (manager->m_FrameCounter <= cPickupNextFrame)
				continue;
			cPickupNextFrame = manager->m_FrameCounter + 5;
		} else if (processedMission) {
			if (manager->m_FrameCounter <= cPartMisComNextFrame)
				continue;
			cPartMisComNextFrame = manager->m_FrameCounter + 5;
		}

		sample = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i];

		if (sample == SOUND_FRONTEND_RADIO_TURN_OFF)
			manager->m_sQueueSample.m_nFrequency = 28509;
		else if (sample == SOUND_FRONTEND_RADIO_TURN_ON)
			manager->m_sQueueSample.m_nFrequency = 32000;
		else if (sample == SOUND_BULLETTRACE_1 || sample == SOUND_BULLETTRACE_2) {
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
		} else if (staticFreq)
			manager->m_sQueueSample.m_nFrequency = 5382;
		else
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);

		manager->m_sQueueSample.m_nVolume = FRONTEND_VOLUME;
		if (manager->m_sQueueSample.m_nSampleIndex == SFX_HURRICANE_MA && AudioWeatherHost_Wind() > 1.0f)
			manager->m_sQueueSample.m_nVolume = (AudioWeatherHost_Wind() - 1.0f) * manager->m_sQueueSample.m_nVolume;
		manager->m_sQueueSample.m_nCounter = iSound++;
		manager->m_sQueueSample.m_nLoopCount = 1;
		manager->m_sQueueSample.m_bStatic = 1;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_FRONT_END_MENU;
		manager->m_sQueueSample.m_nPriority = 0;
		manager->m_sQueueSample.m_bIs2D = 1;
		AUDIO_WEATHER_FRONTEND_SET_EMITTING_VOLUME(manager->m_sQueueSample.m_nVolume);
		AUDIO_WEATHER_FRONTEND_RESET_LOOP_OFFSETS
		if (stereo) {
			manager->m_sQueueSample.m_nPan = 0;
			manager->m_sQueueSample.m_fDistance = 1.0f;
		} else {
			sample = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i];
			if (sample == SOUND_BULLETTRACE_1) {
				manager->m_sQueueSample.m_nPan = 20;
				manager->m_sQueueSample.m_nVolume = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i];
				manager->m_sQueueSample.m_nPriority = 10;
				manager->m_sQueueSample.m_fDistance = 100.0f;
			} else if (sample == SOUND_BULLETTRACE_2) {
				manager->m_sQueueSample.m_nPan = 107;
				manager->m_sQueueSample.m_nVolume = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i];
				manager->m_sQueueSample.m_nPriority = 10;
				manager->m_sQueueSample.m_fDistance = 100.0f;
			} else {
				manager->m_sQueueSample.m_nPan = 63;
				manager->m_sQueueSample.m_fDistance = 1.0f;
			}
		}
		AUDIO_WEATHER_FRONTEND_SET_SOUND_REVERB(0);
		AUDIO_WEATHER_FRONTEND_SET_SOUND_REFLECTION(0);
		AudioRequests_Submit(manager);
		if (stereo) {
			manager->m_sQueueSample.m_nSampleIndex++;
			manager->m_sQueueSample.m_nCounter = iSound++;
			manager->m_sQueueSample.m_nPan = 127 - manager->m_sQueueSample.m_nPan;
			AudioRequests_Submit(manager);
		}
		if (center) {
			manager->m_sQueueSample.m_nSampleIndex++;
			manager->m_sQueueSample.m_nCounter = iSound++;
			manager->m_sQueueSample.m_nPan = 63;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
			AudioRequests_Submit(manager);
		}
	}
}

//- rouz edit (ChatGPT)
