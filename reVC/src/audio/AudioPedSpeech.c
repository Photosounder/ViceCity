//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "DMAudio.h"
#include "soundlist.h"

enum { PED_COMMENT_MAX_DIST = 40, PED_COMMENT_POLICE_HELI_MAX_DIST = 400 };

void
AudioPedSpeech_SetTalking(CPed *ped, uint8_t status)
{
    // Preserve the original operation and game-query order
	if (ped != NULL)
		AudioPedSpeechHost_SetTalking(ped, status);
}

void
AudioPedSpeech_SetMood(cAudioManager *manager, uint8_t mood, uint32_t time)
{
    // Preserve the original operation and game-query order
	if (!manager->m_bIsInitialised) return;

	if (mood < MAX_PLAYER_MOODS) {
		manager->m_nPlayerMood = mood;
		manager->m_nPlayerMoodTimer = AudioPlayerEngineHost_Time() + time;
	}

}

void
AudioPedSpeech_Mood(cAudioManager *manager)
{
    // Preserve the original operation and game-query order
	CPlayerPed *playerPed;
	uint32_t curTime = AudioPlayerEngineHost_Time();

	if (manager->m_nPlayerMoodTimer > curTime)
		return;

	playerPed = AudioPoliceHost_FindPlayer();
	if (!playerPed)
		return;

	if (AudioPoliceHost_WantedLevel(playerPed) > 3)
		manager->m_nPlayerMood = PLAYER_MOOD_ANGRY;
	else if (AudioPoliceHost_WantedLevel(playerPed) > 1)
		manager->m_nPlayerMood = PLAYER_MOOD_PISSED_OFF;
	else if (AudioPedSpeechHost_MissionTime() != -1) {
		if (curTime >= AudioPedSpeechHost_MissionTime()) {
			if (curTime < AudioPedSpeechHost_MissionTime() + 180000)
				manager->m_nPlayerMood = PLAYER_MOOD_WISECRACKING;
			else
				manager->m_nPlayerMood = PLAYER_MOOD_CALM;
		} else
			AudioPedSpeechHost_SetMissionTime(curTime);
	} else
		manager->m_nPlayerMood = PLAYER_MOOD_CALM;
}

void
AudioPedSpeech_Setup(cAudioManager *manager, cPedParams *params, uint16_t sound)
{
    // Preserve the original operation and game-query order
	CPed *ped = params->m_pPed;
	uint8_t Vol;
	float maxDist;
	tPedComment pedComment;

	if(ped != NULL) {
		if(!AudioPedSpeechHost_CanTalk(ped)) return;
		manager->m_bGenericSfx = 0;
		pedComment.m_nSampleIndex = AudioPedTalk_Select(manager, ped, sound);
		if(pedComment.m_nSampleIndex == NO_SAMPLE)
			return;
		maxDist = PED_COMMENT_MAX_DIST;
	} else {
		manager->m_bGenericSfx = 1;
		switch(sound) {
		case SOUND_PED_HELI_PLAYER_FOUND:
			maxDist = PED_COMMENT_POLICE_HELI_MAX_DIST;
			pedComment.m_nSampleIndex = manager->m_anRandomTable[manager->m_sQueueSample.m_nEntityIndex % 4] % 20 + SFX_POLICE_HELI_1;
			break;
		case SOUND_PED_VCPA_PLAYER_FOUND:
			maxDist = PED_COMMENT_POLICE_HELI_MAX_DIST;
#ifdef FIX_BUGS
			pedComment.m_nSampleIndex = manager->m_anRandomTable[manager->m_sQueueSample.m_nEntityIndex % 4] % 23 + SFX_POLICE_BOAT_1;
#else
			pedComment.m_nSampleIndex = manager->m_anRandomTable[manager->m_sQueueSample.m_nEntityIndex % 4] % 29 + SFX_POLICE_BOAT_1;
#endif
			break;
		case SOUND_INJURED_PED_MALE_OUCH:
			maxDist = PED_COMMENT_MAX_DIST;
			pedComment.m_nSampleIndex = manager->m_anRandomTable[manager->m_sQueueSample.m_nEntityIndex % 4] % 41 + SFX_GENERIC_MALE_GRUNT_1;
			break;
		case SOUND_INJURED_PED_FEMALE:
			maxDist = PED_COMMENT_MAX_DIST;
			pedComment.m_nSampleIndex = manager->m_anRandomTable[manager->m_sQueueSample.m_nEntityIndex % 4] % 33 + SFX_GENERIC_FEMALE_GRUNT_1;
			break;
		default:
			return;
		}
	}

	if(params->m_fDistance < (maxDist * maxDist)) {
		AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
		if(AudioPedSpeechHost_LineClear(&manager->m_sQueueSample.m_vecPos))
			Vol = PED_COMMENT_VOLUME;
		else
			Vol = PED_COMMENT_VOLUME_BEHIND_WALL;
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, maxDist, manager->m_sQueueSample.m_fDistance);
		pedComment.m_nLoadingTimeout = 10;
		if (manager->m_sQueueSample.m_nVolume > 0) {
			pedComment.m_nEntityIndex = manager->m_sQueueSample.m_nEntityIndex;
			// Copy the game position into the C comment record
			// Copy the sound coordinates for the game vector operation
			const AudioSoundPosition commentPosition = manager->m_sQueueSample.m_vecPos;
			pedComment.m_vecPos.x = commentPosition.x;
			pedComment.m_vecPos.y = commentPosition.y;
			pedComment.m_vecPos.z = commentPosition.z;
			pedComment.m_fDistance = manager->m_sQueueSample.m_fDistance;
			pedComment.m_nVolume = manager->m_sQueueSample.m_nVolume;
#if defined(EXTERNAL_3D_SOUND) && defined(FIX_BUGS)
			pedComment.m_nEmittingVolume = Vol;
#endif
			PedComments_Add(&manager->m_sPedComments, &pedComment);
		}
	}
}

void
DMAudio_SetPedTalkingStatus(CPed *ped, uint8_t status)
{
    // Apply talking permission through the C operation
    AudioPedSpeech_SetTalking(ped, status);
}
void
DMAudio_SetPlayersMood(uint8_t mood, uint32_t time)
{
    // Apply player mood to the live global audio owner
    AudioPedSpeech_SetMood(&AudioManager, mood, time);
}
//- rouz edit (ChatGPT)
