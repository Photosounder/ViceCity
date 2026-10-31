//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "soundlist.h"
#include "../peds/PedStates.h"
#include "../modelinfo/ModelIds.h"

#define AUDIO_PED_TALK_COOLDOWN(count)		/* Retain one shared cooldown at each original expansion */ static uint8_t cooldown = 0;\
if (cooldown != 0) {\
	if (++cooldown == count) cooldown = 0;\
	return NO_SAMPLE;\
}\
cooldown = 1;

uint32_t
AudioPedTalk_Select(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	if(AudioPedOneShotsHost_State(ped) != PED_FALL || sound == SOUND_PED_DAMAGE || sound == SOUND_PED_HIT || sound == SOUND_PED_LAND) {
		if(AudioPedTalkHost_GetUpTimer(ped) == UINT32_MAX || AudioPedTalkHost_GetUpTimer(ped) > AudioPlayerEngineHost_Time()) {
			if(sound != SOUND_PED_DAMAGE && sound != SOUND_PED_HIT && sound != SOUND_PED_LAND) return NO_SAMPLE;
		}
		if(AudioPedOneShotsHost_IsPlayer(ped)) return AudioPedTalk_Player(manager, ped, sound);
		switch(AudioPedTalkHost_Model(ped)) {
		case MI_PLAYER: return AudioPedTalk_Player(manager, ped, sound);
		case MI_COP: return AudioPedTalk_Cop(manager, ped, sound);
		case MI_SWAT: return AudioPedTalk_Swat(manager, ped, sound);
		case MI_FBI: return AudioPedTalk_FBI(manager, ped, sound);
		case MI_ARMY: return AudioPedTalk_Army(manager, ped, sound);
		case MI_MEDIC: return AudioPedTalk_Medic(manager, ped, sound);
		case MI_FIREMAN: return AudioPedTalk_Fireman(manager, ped, sound);
		case MI_MALE01: return AudioPedTalk_Default(manager, ped, sound);
		case MI_HFYST: return AudioPedTalk_HFYST(manager, ped, sound);
		case MI_HFOST: return AudioPedTalk_HFOST(manager, ped, sound);
		case MI_HMYST: return AudioPedTalk_HMYST(manager, ped, sound);
		case MI_HMOST: return AudioPedTalk_HMOST(manager, ped, sound);
		case MI_HFYRI: return AudioPedTalk_HFYRI(manager, ped, sound);
		case MI_HFORI: return AudioPedTalk_HFORI(manager, ped, sound);
		case MI_HMYRI: return AudioPedTalk_HMYRI(manager, ped, sound);
		case MI_HMORI: return AudioPedTalk_HMORI(manager, ped, sound);
		case MI_HFYBE: return AudioPedTalk_HFYBE(manager, ped, sound);
		case MI_HFOBE: return AudioPedTalk_HFOBE(manager, ped, sound);
		case MI_HMYBE: return AudioPedTalk_HMYBE(manager, ped, sound);
		case MI_HMOBE: return AudioPedTalk_HMOBE(manager, ped, sound);
		case MI_HFYBU: return AudioPedTalk_HFYBU(manager, ped, sound);
		case MI_HFYMD: return AudioPedTalk_HFYMD(manager, ped, sound);
		case MI_HFYCG: return AudioPedTalk_HFYCG(manager, ped, sound);
		case MI_HFYPR: return AudioPedTalk_HFYPR(manager, ped, sound);
		case MI_HFOTR: return AudioPedTalk_HFOTR(manager, ped, sound);
		case MI_HMOTR: return AudioPedTalk_HMOTR(manager, ped, sound);
		case MI_HMYAP: return AudioPedTalk_HMYAP(manager, ped, sound);
		case MI_HMOCA: return AudioPedTalk_HMOCA(manager, ped, sound);
		case MI_BMODK: return AudioPedTalk_BMODK(manager, ped, sound);
		case MI_BMYKR: return AudioPedTalk_BMYCR(manager, ped, sound);
		case MI_BFYST: return AudioPedTalk_BFYST(manager, ped, sound);
		case MI_BFOST: return AudioPedTalk_BFOST(manager, ped, sound);
		case MI_BMYST: return AudioPedTalk_BMYST(manager, ped, sound);
		case MI_BMOST: return AudioPedTalk_BMOST(manager, ped, sound);
		case MI_BFYRI: return AudioPedTalk_BFYRI(manager, ped, sound);
		case MI_BFORI: return AudioPedTalk_BFORI(manager, ped, sound);
		case MI_BMYRI: return AudioPedTalk_BMYRI(manager, ped, sound);
		case MI_BFYBE: return AudioPedTalk_BFYBE(manager, ped, sound);
		case MI_BMYBE: return AudioPedTalk_BMYBE(manager, ped, sound);
		case MI_BFOBE: return AudioPedTalk_BFOBE(manager, ped, sound);
		case MI_BMOBE: return AudioPedTalk_BMOBE(manager, ped, sound);
		case MI_BMYBU: return AudioPedTalk_BMYBU(manager, ped, sound);
		case MI_BFYPR: return AudioPedTalk_BFYPR(manager, ped, sound);
		case MI_BFOTR: return AudioPedTalk_BFOTR(manager, ped, sound);
		case MI_BMOTR: return AudioPedTalk_BMOTR(manager, ped, sound);
		case MI_BMYPI: return AudioPedTalk_BMYPI(manager, ped, sound);
		case MI_BMYBB: return AudioPedTalk_BMYBB(manager, ped, sound);
		case MI_WMYCR: return AudioPedTalk_WMYCR(manager, ped, sound);
		case MI_WFYST: return AudioPedTalk_WFYST(manager, ped, sound);
		case MI_WFOST: return AudioPedTalk_WFOST(manager, ped, sound);
		case MI_WMYST: return AudioPedTalk_WMYST(manager, ped, sound);
		case MI_WMOST: return AudioPedTalk_WMOST(manager, ped, sound);
		case MI_WFYRI: return AudioPedTalk_WFYRI(manager, ped, sound);
		case MI_WFORI: return AudioPedTalk_WFORI(manager, ped, sound);
		case MI_WMYRI: return AudioPedTalk_WMYRI(manager, ped, sound);
		case MI_WMORI: return AudioPedTalk_WMORI(manager, ped, sound);
		case MI_WFYBE: return AudioPedTalk_WFYBE(manager, ped, sound);
		case MI_WMYBE: return AudioPedTalk_WMYBE(manager, ped, sound);
		case MI_WFOBE: return AudioPedTalk_WFOBE(manager, ped, sound);
		case MI_WMOBE: return AudioPedTalk_WMOBE(manager, ped, sound);
		case MI_WMYCW: return AudioPedTalk_WMYCW(manager, ped, sound);
		case MI_WMYGO: return AudioPedTalk_WMYGO(manager, ped, sound);
		case MI_WFOGO: return AudioPedTalk_WFOGO(manager, ped, sound);
		case MI_WMOGO: return AudioPedTalk_WMOGO(manager, ped, sound);
		case MI_WFYLG: return AudioPedTalk_WFYLG(manager, ped, sound);
		case MI_WMYLG: return AudioPedTalk_WMYLG(manager, ped, sound);
		case MI_WFYBU: return AudioPedTalk_WFYBU(manager, ped, sound);
		case MI_WMYBU: return AudioPedTalk_WMYBU(manager, ped, sound);
		case MI_WMOBU: return AudioPedTalk_WMOBU(manager, ped, sound);
		case MI_WFYPR: return AudioPedTalk_WFYPR(manager, ped, sound);
		case MI_WFOTR: return AudioPedTalk_WFOTR(manager, ped, sound);
		case MI_WMOTR: return AudioPedTalk_WMOTR(manager, ped, sound);
		case MI_WMYPI: return AudioPedTalk_WMYPI(manager, ped, sound);
		case MI_WMOCA: return AudioPedTalk_WMOCA(manager, ped, sound);
		case MI_WFYJG: return AudioPedTalk_WFYJG(manager, ped, sound);
		case MI_WMYJG: return AudioPedTalk_WMYJG(manager, ped, sound);
		case MI_WFYSK: return AudioPedTalk_WFYSK(manager, ped, sound);
		case MI_WMYSK: return AudioPedTalk_WMYSK(manager, ped, sound);
		case MI_WFYSH: return AudioPedTalk_WFYSH(manager, ped, sound);
		case MI_WFOSH: return AudioPedTalk_WFOSH(manager, ped, sound);
		case MI_JFOTO: return AudioPedTalk_JFOTO(manager, ped, sound);
		case MI_JMOTO: return AudioPedTalk_JMOTO(manager, ped, sound);
		case MI_CBA:
		case MI_CBB: return AudioPedTalk_CB(manager, ped, sound);
		case MI_HNA:
		case MI_HNB: return AudioPedTalk_HN(manager, ped, sound);
		case MI_SGA:
		case MI_SGB: return AudioPedTalk_SG(manager, ped, sound);
		case MI_CLA:
		case MI_CLB: return AudioPedTalk_CL(manager, ped, sound);
		case MI_GDA:
		case MI_GDB: return AudioPedTalk_GD(manager, ped, sound);
		case MI_BKA:
		case MI_BKB: return AudioPedTalk_BK(manager, ped, sound);
		case MI_PGA:
		case MI_PGB: return AudioPedTalk_PG(manager, ped, sound);
		case MI_VICE1:
		case MI_VICE2:
		case MI_VICE3:
		case MI_VICE4:
		case MI_VICE5:
		case MI_VICE7:
		case MI_VICE8: return AudioPedTalk_ViceWhite(manager, ped, sound);
		case MI_VICE6: return AudioPedTalk_ViceBlack(manager, ped, sound);
		case MI_WFYG1: return AudioPedTalk_WFYG1(manager, ped, sound);
		case MI_WFYG2: return AudioPedTalk_WFYG2(manager, ped, sound);
		case MI_SPECIAL01:
		case MI_SPECIAL02:
		case MI_SPECIAL03:
		case MI_SPECIAL04:
		case MI_SPECIAL05:
		case MI_SPECIAL06:
		case MI_SPECIAL07:
		case MI_SPECIAL08:
		case MI_SPECIAL09:
		case MI_SPECIAL10:
		case MI_SPECIAL11:
		case MI_SPECIAL12:
		case MI_SPECIAL13:
		case MI_SPECIAL14:
		case MI_SPECIAL15:
		case MI_SPECIAL16:
		case MI_SPECIAL17:
		case MI_SPECIAL18:
		case MI_SPECIAL19:
		case MI_SPECIAL20:
		case MI_SPECIAL21: return AudioPedTalk_SpecialCharacter(manager, ped, AudioPedTalkHost_Model(ped), sound);
		default: return AudioPedTalk_GenericMale(manager, ped, sound);
		}
	}

	return NO_SAMPLE;
}

void
AudioPedTalk_Phrase(cAudioManager *manager, uint32_t *phrase, uint32_t *prevPhrase, uint32_t sample, uint32_t maxOffset)
{
    // Preserve the original character selection and shared phrase history
	(*phrase) = sample + manager->m_anRandomTable[manager->m_sQueueSample.m_nEntityIndex & 3] % maxOffset;

	// check if the same sfx like last time, if yes, then try use next one,
	// if exceeded range, then choose first available sample
	if ((*phrase) == (*prevPhrase) && ++(*phrase) >= sample + maxOffset)
		(*phrase) = sample;
	(*prevPhrase) = (*phrase);
}

uint32_t
AudioPedTalk_Player(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	if(manager->m_bIsPlayerShutUp) return NO_SAMPLE;
	switch(sound) {
	case SOUND_PED_DEATH: return SFX_PLAYER_DEATH;
	case SOUND_PED_DAMAGE:
	case SOUND_PED_BULLET_HIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_HIT_BULLET_1, 33); break;
	case SOUND_PED_HIT:
	case SOUND_PED_DEFEND: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_HIT_FIST_1, 42); break;
	case SOUND_PED_LAND: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_HIT_GROUND_1, 35); break;
	case SOUND_PED_BURNING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ON_FIRE_1, 16); break;
	case SOUND_PED_PLAYER_REACTTOCOP:
		switch(manager->m_nPlayerMood) {
		case PLAYER_MOOD_ANGRY:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ANGRY_BUSTED_1, 38);
			break;
		case PLAYER_MOOD_WISECRACKING:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_WISECRACKING_BUSTED_1, 20);
			break;
		default:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_CALM_BUSTED_1, 22);
			break;
		}
		break;
	case SOUND_PED_ON_FIRE: {
		AUDIO_PED_TALK_COOLDOWN(8);
		switch(manager->m_nPlayerMood) {
		case PLAYER_MOOD_PISSED_OFF:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_PISSED_OFF_SHOOT_1, 29);
			break;
		case PLAYER_MOOD_ANGRY:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ANGRY_SHOOT_1, 39);
			break;
		case PLAYER_MOOD_WISECRACKING:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_WISECRACKING_SHOOT_1, 9);
			break;
		default:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_CALM_SHOOT_1, 35);
			break;
		}
		break;
	}
	case SOUND_PED_AIMING: {
		AUDIO_PED_TALK_COOLDOWN(8);
		switch(manager->m_nPlayerMood) {
		case PLAYER_MOOD_PISSED_OFF:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_PISSED_OFF_PULL_GUN_1, 25);
			break;
		case PLAYER_MOOD_ANGRY:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ANGRY_PULL_GUN_1, 52);
			break;
		case PLAYER_MOOD_WISECRACKING:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_WISECRACKING_PULL_GUN_1, 19);
			break;
		default:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_CALM_PULL_GUN_1, 39);
			break;
		}
		break;
	}
	case SOUND_PED_CAR_JACKING: {
		AUDIO_PED_TALK_COOLDOWN(4);
		switch(manager->m_nPlayerMood) {
		case PLAYER_MOOD_PISSED_OFF:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_PISSED_OFF_JACKING_1, 36);
			break;
		case PLAYER_MOOD_ANGRY:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ANGRY_JACKING_1, 43);
			break;
		case PLAYER_MOOD_WISECRACKING:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_WISECRACKING_JACKING_1, 18);
			break;
		default:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_CALM_JACKING_1, 40);
			break;
		}
		break;
	}
	case SOUND_PED_MUGGING: {
		AUDIO_PED_TALK_COOLDOWN(8);
		switch(manager->m_nPlayerMood) {
		case PLAYER_MOOD_PISSED_OFF:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_PISSED_OFF_PICK_UP_CASH_1, 25);
			break;
		case PLAYER_MOOD_ANGRY:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ANGRY_PICK_UP_CASH_1, 12);
			break;
		case PLAYER_MOOD_WISECRACKING:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_WISECRACKING_PICK_UP_CASH_1, 23);
			break;
		default:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_CALM_PICK_UP_CASH_1, 11);
			break;
		}
		break;
	}
	case SOUND_PED_CAR_JACKED: {
		AUDIO_PED_TALK_COOLDOWN(4);
		switch(manager->m_nPlayerMood) {
		case PLAYER_MOOD_PISSED_OFF:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_PISSED_OFF_JACKED_1, 21);
			break;
		case PLAYER_MOOD_ANGRY:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ANGRY_JACKED_1, 33);
			break;
		case PLAYER_MOOD_WISECRACKING:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_WISECRACKING_JACKED_1, 18);
			break;
		default:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_CALM_JACKED_1, 24);
			break;
		}
		break;
	}
	case SOUND_PED_PLAYER_AFTERSEX: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_AFTERSEX_1, 18); break;
	case SOUND_PED_PLAYER_BEFORESEX:
		switch(manager->m_nPlayerMood) {
		case PLAYER_MOOD_ANGRY:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ANGRY_SEX_1, 18);
			break;
		case PLAYER_MOOD_WISECRACKING:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_WISECRACKING_SEX_1, 10);
			break;
		default:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_CALM_SEX_1, 8);
			break;
		}
		break;
	case SOUND_PED_PLAYER_FARFROMCOPS: {
		AUDIO_PED_TALK_COOLDOWN(4);
		switch(manager->m_nPlayerMood) {
		case PLAYER_MOOD_ANGRY:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ANGRY_CHASED_1, 9);
			break;
		case PLAYER_MOOD_WISECRACKING:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_WISECRACKING_CHASED_1, 7);
			break;
		default:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_CALM_CHASED_1, 20);
			break;
		}
		break;
	}
	case SOUND_PED_ATTACK: {
		AUDIO_PED_TALK_COOLDOWN(4);
		switch(manager->m_nPlayerMood) {
		case PLAYER_MOOD_PISSED_OFF:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_PISSED_OFF_FIGHT_1, 61);
			break;
		case PLAYER_MOOD_ANGRY:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ANGRY_FIGHT_1, 61);
			break;
		case PLAYER_MOOD_WISECRACKING:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_WISECRACKING_FIGHT_1, 27);
			break;
		default:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_CALM_FIGHT_1, 47);
			break;
		}
		break;
	}
	case SOUND_PED_CRASH_VEHICLE:
	case SOUND_PED_CRASH_CAR:
	case SOUND_PED_ANNOYED_DRIVER: {
		AUDIO_PED_TALK_COOLDOWN(4);
		switch(manager->m_nPlayerMood) {
		case PLAYER_MOOD_PISSED_OFF:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_PISSED_OFF_CRASH_1, 44);
			break;
		case PLAYER_MOOD_ANGRY:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ANGRY_CRASH_1, 41);
			break;
		case PLAYER_MOOD_WISECRACKING:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_WISECRACKING_CRASH_1, 19);
			break;
		default:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_CALM_CRASH_1, 43);
			break;
		}
		break;
	}
	case SOUND_PED_SOLICIT: {
		AUDIO_PED_TALK_COOLDOWN(4);
		switch(manager->m_nPlayerMood) {
		case PLAYER_MOOD_PISSED_OFF:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_PISSED_OFF_PICK_UP_HOOKER_1, 17);
			break;
		case PLAYER_MOOD_ANGRY:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_ANGRY_PICK_UP_HOOKER_1, 6);
			break;
		case PLAYER_MOOD_WISECRACKING:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_WISECRACKING_PICK_UP_HOOKER_1, 11);
			break;
		default:
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_CALM_PICK_UP_HOOKER_1, 22);
			break;
		}
		break;
	}
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_GenericMale(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	manager->m_bGenericSfx = 1;
	switch(sound) {
	case SOUND_PED_DEATH: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GENERIC_MALE_DEATH_1, 41); break;
	case SOUND_PED_BULLET_HIT:
	case SOUND_PED_DEFEND: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GENERIC_MALE_GRUNT_1, 41); break;
	case SOUND_PED_BURNING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GENERIC_MALE_FIRE_1, 32); break;
	case SOUND_PED_FLEE_SPRINT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GENERIC_MALE_PANIC_1, 35); break;
	default: return NO_SAMPLE;
	}
	return sfx;
}

uint32_t
AudioPedTalk_GenericFemale(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;
	manager->m_bGenericSfx = 1;
	switch(sound) {
	case SOUND_PED_DEATH: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GENERIC_FEMALE_DEATH_1, 22); break;
	case SOUND_PED_BULLET_HIT:
	case SOUND_PED_DEFEND: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GENERIC_FEMALE_GRUNT_1, 33); break;
	case SOUND_PED_BURNING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GENERIC_FEMALE_FIRE_1, 17); break;
	case SOUND_PED_FLEE_SPRINT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GENERIC_FEMALE_PANIC_1, 27); break;
	default: return NO_SAMPLE;
	}
	return sfx;
}

uint32_t
AudioPedTalk_Default(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_GUN_PANIC_1, 12); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_JACKED_1, 12); break;
#ifdef FIX_BUGS
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_JACKING_1, 13); break;
#endif
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_MUGGED_1, 4); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_SAVED_1, 4); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_TAXI_1, 5); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_FIGHT_1, 16); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_DODGE_1, 19); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_RUN_1, 19); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_GENERIC_CRASH_1, 13); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_CAR_CRASH_1, 15); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_BLOCKED_1, 16); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_LOST_1, 5); break;
#ifdef FIX_BUGS
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_EYEING_1, 6); break;
#else
	case SOUND_PED_CHAT_SEXY_FEMALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_EYEING_1, 6); break;
#endif
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_SHOCKED_1, 6); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_BUMP_1, 25); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_DEFAULT_VOICE_CHAT_1, 25); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_Cop(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;
	enum PedState objective;
	switch(sound) {
	case SOUND_PED_ARREST_COP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_ARREST_1, 4); break;
	case SOUND_PED_PULLOUTWEAPON: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_PULLOUTWEAPON_1, 3); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_SAVED_1, 2); break;
	case SOUND_PED_COP_TARGETING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_COP_TARGETING_1, 4); break;
	case SOUND_PED_COP_MANYCOPSAROUND: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_COP_MANYCOPSAROUND_1, 2); break;
	case SOUND_PED_GUNAIMEDAT2: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_GUNAIMEDAT2_1, 2); break;
	case SOUND_PED_COP_ALONE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_COP_ALONE_1, 4); break;
	case SOUND_PED_GUNAIMEDAT3: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_GUNAIMEDAT2_1, 2); break;
	case SOUND_PED_COP_ASK_FOR_ID: {
		AUDIO_PED_TALK_COOLDOWN(4);
		AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_COP_ASK_FOR_ID_1, 2);
		break;
	}
	case SOUND_PED_COP_LITTLECOPSAROUND:
		objective = AudioPedTalkHost_PlayerState();
		if(objective == PED_ARRESTED || objective == PED_DEAD || objective == PED_DIE) return NO_SAMPLE;
		AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_COP_LITTLECOPSAROUND_1, 4);
		break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_FIGHT_1, 4); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_DODGE_1, 3); break;
#ifdef FIX_BUGS
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_CAR_CRASH_1, 4); break;
#endif
	case SOUND_PED_PED_COLLISION:
		if(AudioPoliceHost_WantedLevel(AudioPoliceHost_FindPlayer()) > 0)
			AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_COP_VOICE_1_BUMP_1, 5);
		else
			return NO_SAMPLE;
		break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return (SFX_COP_VOICE_2_ARREST_1 - SFX_COP_VOICE_1_ARREST_1) * (manager->m_sQueueSample.m_nEntityIndex % 5) + sfx;
}

uint32_t
AudioPedTalk_Swat(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;
	switch(sound) {
	case SOUND_PED_COP_HELIPILOTPHRASE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_SWAT_VOICE_1_COP_HELIPILOTPHRASE_1, 7); break;
	case SOUND_PED_COP_TARGETING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_SWAT_VOICE_1_COP_TARGETING_1, 4); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_SWAT_VOICE_1_DODGE_1, 3); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	sfx += (SFX_SWAT_VOICE_2_DODGE_1 - SFX_SWAT_VOICE_1_DODGE_1) * (manager->m_sQueueSample.m_nEntityIndex % 3);
	return sfx;
}

uint32_t
AudioPedTalk_FBI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;
	switch(sound) {
#ifdef FIX_BUGS
	case SOUND_PED_COP_TARGETING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_FBI_VOICE_1_COP_TARGETING_1, 6); break;
#else
	case SOUND_PED_COP_TARGETING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_FBI_VOICE_1_COP_TARGETING_1, 4); break;
#endif
	case SOUND_PED_COP_MANYCOPSAROUND: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_FBI_VOICE_1_COP_MANYCOPSAROUND_1, 3); break;
	case SOUND_PED_GUNAIMEDAT2: sfx = SFX_FBI_VOICE_1_GUNAIMEDAT2_1; break;
	case SOUND_PED_GUNAIMEDAT3: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_FBI_VOICE_1_GUNAIMEDAT3_1, 4); break;
	case SOUND_PED_CRASH_VEHICLE:
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_FBI_VOICE_1_CAR_CRASH_1, 4); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
#ifdef FIX_BUGS
	sfx += (SFX_FBI_VOICE_2_GUNAIMEDAT3_1 - SFX_FBI_VOICE_1_GUNAIMEDAT3_1) * (manager->m_sQueueSample.m_nEntityIndex % 3);
#else
	sfx += 16 * (manager->m_sQueueSample.m_nEntityIndex % 3);
#endif
	return sfx;
}

uint32_t
AudioPedTalk_Army(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	return AudioPedTalk_GenericMale(manager, ped, sound);
}

uint32_t
AudioPedTalk_Medic(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;
	switch(sound) {
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_MEDIC_VOICE_1_FIGHT_1, 6); break;
	case SOUND_PED_HEALING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_MEDIC_VOICE_1_AT_VICTIM_1, 17); break;
	case SOUND_PED_LEAVE_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_MEDIC_VOICE_1_GET_OUT_VAN_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	sfx += (SFX_MEDIC_VOICE_2_FIGHT_1 - SFX_MEDIC_VOICE_1_FIGHT_1) * (manager->m_sQueueSample.m_nEntityIndex % 2);
	return sfx;
}

uint32_t
AudioPedTalk_Fireman(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	return AudioPedTalk_GenericMale(manager, ped, sound);
}

uint32_t
AudioPedTalk_WFYG1(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_GUN_COOL_1, 6); break;
	case SOUND_PED_MUGGING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_MUGGING_1, 2); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_JACKED_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WFYG1_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WFYG1_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_FIGHT_1, 4); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_DODGE_1, 9); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_RUN_1, 2); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_CAR_CRASH_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_BLOCKED_1, 7); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_LOST_1, 3); break;
	case SOUND_PED_CHAT_SEXY_FEMALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: return SFX_WFYG1_SHOCKED_1;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG1_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFYG2(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_GUN_COOL_1, 3); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_JACKED_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_MUGGED_1, 2); break;
#ifdef FIX_BUGS
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_TAXI_1, 2); break;
#else
	case SOUND_PED_TAXI_WAIT: return SFX_WFYG2_TAXI_1;
#endif
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_FIGHT_1, 5); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_DODGE_1, 8); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_CAR_CRASH_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_BLOCKED_1, 5); break;
	case SOUND_PED_WAIT_DOUBLEBACK: return SFX_WFYG2_LOST_1;
	case SOUND_PED_CHAT_SEXY_FEMALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_EYEING_1, 4); break;
	case SOUND_PED_CHAT_EVENT: return SFX_WFYG2_SHOCKED_1;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYG2_CHAT_1, 9); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}

	return sfx;
}

uint32_t
AudioPedTalk_HFYST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;
	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_JACKING_1, 4); break;
	case SOUND_PED_MUGGING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_MUGGING_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_MUGGED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: return SFX_HFYST_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_FIGHT_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_DODGE_1, 10); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_BLOCKED_1, 7); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_LOST_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYST_CHAT_1, 9); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HFOST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_GUN_COOL_1, 6); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_JACKED_1, 8); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_MUGGED_1, 3); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_TAXI_1, 2); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_FIGHT_1, 8); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_DODGE_1, 10); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_GENERIC_CRASH_1, 11); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_BLOCKED_1, 9); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_LOST_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_BUMP_1, 12); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOST_CHAT_1, 11); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HMYST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYST_GUN_PANIC_1, 6); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_HMYST_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_HMYST_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYST_DODGE_1, 6); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYST_RUN_1, 4); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYST_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYST_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYST_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYST_BUMP_1, 13); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYST_CHAT_1, 11); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HMOST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_JACKING_1, 3); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_MUGGED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: return SFX_HMOST_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_FIGHT_1, 8); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_DODGE_1, 9); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_CAR_CRASH_1, 7); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_BLOCKED_1, 7); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: return SFX_HMOST_EYEING_1;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOST_CHAT_1, 11); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HFYRI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_MUGGED_1, 4); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_SAVED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: return SFX_HFYRI_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_DODGE_1, 10); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_RUN_1, 4); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_LOST_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYRI_BUMP_1, 9); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HFORI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_GUN_PANIC_1, 6); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_JACKED_1, 9); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_HFORI_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_HFORI_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_DODGE_1, 6); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_RUN_1, 4); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_CAR_CRASH_1, 7); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_BLOCKED_1, 6); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_FEMALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFORI_BUMP_1, 10); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HMYRI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYRI_GUN_PANIC_1, 7); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYRI_JACKING_1, 3); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYRI_JACKED_1, 8); break;
	case SOUND_PED_ROBBED: return SFX_HMYRI_MUGGED_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYRI_FIGHT_1, 5); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYRI_DODGE_1, 9); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYRI_GENERIC_CRASH_1, 12); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYRI_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYRI_BLOCKED_1, 7); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYRI_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYRI_BUMP_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HMORI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history

	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_JACKED_1, 8); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_MUGGED_1, 3); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_TAXI_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_DODGE_1, 7); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_RUN_1, 6); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_GENERIC_CRASH_1, 11); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_CAR_CRASH_1, 6); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_LOST_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMORI_CHAT_1, 8); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HFYBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBE_GUN_PANIC_1, 7); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBE_JACKED_1, 7); break;
	case SOUND_PED_TAXI_WAIT: return SFX_HFYBE_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBE_DODGE_1, 11); break;
#ifdef FIX_BUGS // assumption
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBE_RUN_1, 7); break;
#endif
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBE_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBE_CAR_CRASH_1, 6); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBE_LOST_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBE_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBE_BUMP_1, 8); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBE_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HFOBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_JACKED_1, 6); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_HFOBE_SAVED_1;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_TAXI_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_DODGE_1, 7); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_RUN_1, 4); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_GENERIC_CRASH_1, 5); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_CAR_CRASH_1, 6); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_BLOCKED_1, 6); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_LOST_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOBE_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HMYBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_GUN_PANIC_1, 6); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_JACKED_1, 12); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_HMYBE_SAVED_1;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_INNOCENT_1, 4); break;
	case SOUND_PED_TAXI_WAIT: return SFX_HMYBE_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_FIGHT_1, 8); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_DODGE_1, 7); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_GENERIC_CRASH_1, 10); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_CAR_CRASH_1, 7); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_LOST_1, 3); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_EYEING_1, 5); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYBE_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HMOBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOBE_GUN_PANIC_1, 3); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOBE_JACKED_1, 6); break;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOBE_INNOCENT_1, 3); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOBE_DODGE_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOBE_BLOCKED_1, 10); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOBE_EYEING_1, 4); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOBE_BUMP_1, 8); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}

	return sfx;
}

uint32_t
AudioPedTalk_HFYBU(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history

	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBU_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBU_JACKING_1, 3); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBU_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBU_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_HFYBU_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_HFYBU_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBU_FIGHT_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBU_DODGE_1, 10); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBU_GENERIC_CRASH_1, 12); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBU_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBU_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBU_LOST_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYBU_BUMP_1, 11); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HFYMD(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYMD_GUN_PANIC_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYMD_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYMD_SAVED_1, 3); break;
#ifdef FIX_BUGS
	case SOUND_PED_TAXI_WAIT: return SFX_HFYMD_TAXI_1;
#else
	case SOUND_PED_TAXI_WAIT: return SFX_BFOBE_TAXI_1;
#endif
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYMD_FIGHT_1, 9); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYMD_DODGE_1, 8); break;
	case SOUND_PED_SOLICIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYMD_SOLICIT_1, 15); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYMD_BUMP_1, 9); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HFYCG(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYCG_GUN_PANIC_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYCG_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_HFYCG_SAVED_1;
#ifdef FIX_BUGS
	case SOUND_PED_TAXI_WAIT: return SFX_HFYCG_TAXI_1;
#else
	case SOUND_PED_TAXI_WAIT: return SFX_BFOBE_TAXI_1;
#endif
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYCG_DODGE_1, 8); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYCG_RUN_1, 4); break;
	case SOUND_PED_SOLICIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYCG_SOLICIT_1, 14); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYCG_BUMP_1, 9); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HFYPR(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYPR_GUN_COOL_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYPR_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_HFYPR_SAVED_1;
	case SOUND_PED_PLAYER_BEFORESEX: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYPR_FUCKING_1, 8); break;
	case SOUND_PED_TAXI_WAIT: return SFX_HFYPR_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYPR_FIGHT_1, 10); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYPR_DODGE_1, 9); break;
	case SOUND_PED_SOLICIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYPR_SOLICIT_1, 14); break;
	case SOUND_PED_CHAT_SEXY_FEMALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYPR_EYEING_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYPR_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFYPR_CHAT_1, 12); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HFOTR(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOTR_GUN_COOL_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOTR_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_HFOTR_SAVED_1;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOTR_TAXI_1, 2); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOTR_FIGHT_1, 6); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOTR_DODGE_1, 8); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOTR_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOTR_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HFOTR_CHAT_1, 12); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HMOTR(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOTR_GUN_COOL_1, 6); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOTR_SAVED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: return SFX_HMOTR_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOTR_FIGHT_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOTR_DODGE_1, 11); break;
	case SOUND_PED_SOLICIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOTR_SOLICIT_1, 8); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOTR_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOTR_BUMP_1, 8); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOTR_CHAT_1, 9); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HMOCA(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOCA_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOCA_JACKING_1, 11); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOCA_JACKED_1, 10); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOCA_MUGGED_1, 7); break;
	case SOUND_PED_TAXI_WAIT: return SFX_HMOCA_TAXI_1;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOCA_RUN_1, 2); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOCA_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOCA_BLOCKED_1, 8); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOCA_EYEING_1, 2); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMOCA_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BMYCR(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_GUN_COOL_1, 6); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_JACKING_1, 12); break;
	case SOUND_PED_MUGGING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_MUGGING_1, 6); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_MUGGED_1, 3); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_SAVED_1, 2); break;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_INNOCENT_1, 4); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_FIGHT_1, 8); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_DODGE_1, 8); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_CAR_CRASH_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_BLOCKED_1, 12); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_EYEING_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYCR_BUMP_1, 11); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BFYST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_GUN_PANIC_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_JACKED_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_SAVED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: return SFX_BFYST_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_DODGE_1, 9); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_RUN_1, 6); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_CAR_CRASH_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_LOST_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_BUMP_1, 9); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYST_CHAT_1, 9); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BFOST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_JACKED_1, 8); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_SAVED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: return SFX_BFOST_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_DODGE_1, 11); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_RUN_1, 4); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_BLOCKED_1, 7); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_LOST_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOST_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BMYST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_GUN_COOL_1, 6); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_JACKING_1, 4); break;
	case SOUND_PED_MUGGING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_MUGGING_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_JACKED_1, 8); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_MUGGED_1, 2); break;
#ifdef FIX_BUGS
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_TAXI_1, 2); break;
#else
	case SOUND_PED_TAXI_WAIT: return SFX_BMYST_TAXI_1;
#endif
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_FIGHT_1, 6); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_DODGE_1, 8); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_CAR_CRASH_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_BLOCKED_1, 8); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYST_CHAT_1, 12); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BMOST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOST_GUN_PANIC_1, 9); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOST_MUGGED_1, 4); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_BMOST_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_BMOST_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOST_FIGHT_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOST_DODGE_1, 8); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOST_GENERIC_CRASH_1, 13); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOST_CAR_CRASH_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOST_LOST_1, 6); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOST_EYEING_1, 6); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOST_BUMP_1, 17); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOST_CHAT_1, 18); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BFYRI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_GUN_PANIC_1, 4); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_JACKING_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_JACKED_1, 8); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_MUGGED_1, 3); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_SAVED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: return SFX_BFYRI_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_DODGE_1, 8); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_RUN_1, 6); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_BLOCKED_1, 9); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_FEMALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_EYEING_1, 3); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_SHOCKED_1, 4); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYRI_BUMP_1, 9); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BFORI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_JACKED_1, 4); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_BFORI_SAVED_1;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_TAXI_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_DODGE_1, 9); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_RUN_1, 4); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_CAR_CRASH_1, 7); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_LOST_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFORI_BUMP_1, 9); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BMYRI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history

	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYRI_GUN_PANIC_1, 7); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYRI_JACKED_1, 4); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYRI_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_BMYRI_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_BMYRI_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYRI_DODGE_1, 8); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYRI_RUN_1, 4); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYRI_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYRI_CAR_CRASH_1, 7); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYRI_BLOCKED_1, 6); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYRI_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: return SFX_BMYRI_EYEING_1;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYRI_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYRI_BUMP_1, 7); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BFYBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_GUN_COOL_1, 6); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_JACKED_1, 8); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_MUGGED_1, 5); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_SAVED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_TAXI_1, 3); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_DODGE_1, 10); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_RUN_1, 6); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_GENERIC_CRASH_1, 8); break;
#ifdef FIX_BUGS
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_CAR_CRASH_1, 10); break;
#else
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_CAR_CRASH_1, 8); break;
#endif
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_BLOCKED_1, 12); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_LOST_1, 4); break;
#ifdef FIX_BUGS
	case SOUND_PED_CHAT_SEXY_FEMALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_EYEING_1, 4); break;
#else
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_EYEING_1, 4); break;
#endif
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_SHOCKED_1, 4); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYBE_CHAT_1, 16); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BMYBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBE_GUN_COOL_1, 4); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBE_JACKING_1, 3); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBE_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: return SFX_BMYBE_MUGGED_1;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_BMYBE_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_BMYBE_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBE_FIGHT_1, 8); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBE_DODGE_1, 10); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBE_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBE_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBE_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: return SFX_BMYBE_LOST_1;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBE_EYEING_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBE_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBE_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BFOBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOBE_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOBE_JACKING_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOBE_JACKED_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOBE_MUGGED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: return SFX_BFOBE_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOBE_DODGE_1, 9); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOBE_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOBE_CAR_CRASH_1, 7); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOBE_BLOCKED_1, 8); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOBE_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOBE_BUMP_1, 8); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOBE_CHAT_1, 8); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BMOBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOBE_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOBE_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOBE_MUGGED_1, 4); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOBE_SAVED_1, 3); break;
	case SOUND_PED_TAXI_WAIT: return SFX_BMOBE_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOBE_FIGHT_1, 10); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOBE_DODGE_1, 11); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOBE_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOBE_CAR_CRASH_1, 9); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOBE_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOBE_BUMP_1, 5); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOBE_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BMYBU(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_BMYBU_SAVED_1;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_INNOCENT_1, 2); break;
	case SOUND_PED_TAXI_WAIT: return SFX_BMYBU_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_FIGHT_1, 5); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_DODGE_1, 10); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_CAR_CRASH_1, 7); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_BLOCKED_1, 8); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBU_BUMP_1, 7); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BFYPR(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYPR_GUN_COOL_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYPR_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_BFYPR_SAVED_1;
	case SOUND_PED_PLAYER_BEFORESEX: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYPR_FUCKING_1, 7); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYPR_TAXI_1, 2); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYPR_FIGHT_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYPR_DODGE_1, 7); break;
	case SOUND_PED_SOLICIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYPR_SOLICIT_1, 13); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYPR_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYPR_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFYPR_CHAT_1, 13); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BFOTR(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOTR_GUN_COOL_1, 6); break;
	case SOUND_PED_MUGGING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOTR_MUGGING_1, 3); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOTR_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_BFOTR_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_BFOTR_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOTR_FIGHT_1, 6); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOTR_DODGE_1, 9); break;
	case SOUND_PED_SOLICIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOTR_SOLICIT_1, 5); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOTR_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOTR_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BFOTR_CHAT_1, 15); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BMOTR(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOTR_GUN_COOL_1, 5); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOTR_SAVED_1, 1); break;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOTR_INNOCENT_1, 4); break;
	case SOUND_PED_TAXI_WAIT: return SFX_BMOTR_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOTR_DODGE_1, 11); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOTR_RUN_1, 7); break;
	case SOUND_PED_SOLICIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOTR_SOLICIT_1, 7); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOTR_EYEING_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOTR_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMOTR_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BMYPI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYPI_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYPI_JACKING_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYPI_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: return SFX_BMYPI_MUGGED_1;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_BMYPI_SAVED_1;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYPI_TAXI_1, 2); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYPI_FIGHT_1, 8); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYPI_DODGE_1, 10); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYPI_GENERIC_CRASH_1, 13); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYPI_CAR_CRASH_1, 5); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYPI_BLOCKED_1, 6); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYPI_EYEING_1, 4); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYPI_BUMP_1, 9); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BMYBB(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_JACKING_1, 9); break;
	case SOUND_PED_MUGGING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_MUGGING_1, 8); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_JACKED_1, 11); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_MUGGED_1, 5); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_SAVED_1, 6); break;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_INNOCENT_1, 4); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_TAXI_1, 3); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_FIGHT_1, 12); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_DODGE_1, 18); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_GENERIC_CRASH_1, 9); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_CAR_CRASH_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_BLOCKED_1, 13); break;
	case SOUND_PED_JEER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_JEER_1, 16); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_EYEING_1, 16); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_SHOCKED_1, 6); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_BUMP_1, 17); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMYBB_CHAT_1, 21); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMYCR(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCR_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCR_JACKING_1, 6); break;
	case SOUND_PED_MUGGING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCR_MUGGING_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCR_MUGGED_1, 3); break;
	case SOUND_PED_TAXI_WAIT: return SFX_WMYCR_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCR_FIGHT_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCR_DODGE_1, 10); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCR_GENERIC_CRASH_1, 9); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCR_CAR_CRASH_1, 9); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCR_BUMP_1, 18); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFYST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_JACKING_1, 4); break;
	case SOUND_PED_MUGGING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_MUGGING_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WFYST_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WFYST_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_FIGHT_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_DODGE_1, 10); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_BLOCKED_1, 6); break;
	case SOUND_PED_WAIT_DOUBLEBACK: return SFX_WFYST_LOST_1;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYST_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFYSK(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSK_GUN_PANIC_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSK_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSK_SAVED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: return SFX_WFYSK_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSK_FIGHT_1, 11); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSK_DODGE_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSK_BLOCKED_1, 11); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSK_BUMP_1, 18); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMYSK(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYSK_GUN_PANIC_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYSK_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYSK_SAVED_1, 2); break;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYSK_INNOCENT_1, 3); break;
	case SOUND_PED_TAXI_WAIT: return SFX_WMYSK_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYSK_FIGHT_1, 5); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYSK_DODGE_1, 10); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYSK_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYSK_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYSK_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYSK_BUMP_1, 14); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYSK_CHAT_1, 13); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFOST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_GUN_PANIC_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_JACKED_1, 8); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_MUGGED_1, 5); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_SAVED_1, 4); break;
	case SOUND_PED_TAXI_WAIT: return SFX_WFOST_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_DODGE_1, 12); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_RUN_1, 7); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_GENERIC_CRASH_1, 10); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_CAR_CRASH_1, 11); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_BLOCKED_1, 12); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_LOST_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_BUMP_1, 19); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOST_CHAT_1, 16); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMYST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_GUN_PANIC_1, 5); break;
	case SOUND_PED_MUGGING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_MUGGING_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_JACKED_1, 5); break;
	case SOUND_PED_ROBBED: return SFX_WMYST_MUGGED_1;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WMYST_SAVED_1;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_INNOCENT_1, 3); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_TAXI_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_DODGE_1, 10); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_RUN_1, 7); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_GENERIC_CRASH_1, 5); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_EYEING_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYST_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMOST(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_JACKED_1, 4); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WMOST_SAVED_1;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_INNOCENT_1, 3); break;
	case SOUND_PED_TAXI_WAIT: return SFX_WMOST_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_FIGHT_1, 8); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_DODGE_1, 8); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_CAR_CRASH_1, 7); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_BLOCKED_1, 8); break;
	case SOUND_PED_JEER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_JEER_1, 4); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_EYEING_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOST_CHAT_1, 9); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFYRI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_JACKED_1, 7); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WFYRI_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WFYRI_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_DODGE_1, 9); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_RUN_1, 5); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_CAR_CRASH_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_FEMALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYRI_BUMP_1, 10); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFORI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFORI_GUN_PANIC_1, 6); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFORI_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFORI_MUGGED_1, 3); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WFORI_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WFORI_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFORI_FIGHT_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFORI_DODGE_1, 11); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFORI_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFORI_CAR_CRASH_1, 10); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFORI_BLOCKED_1, 7); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFORI_LOST_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFORI_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFORI_BUMP_1, 11); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMYRI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_GUN_PANIC_1, 8); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_JACKED_1, 8); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WMYRI_SAVED_1;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_TAXI_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_DODGE_1, 9); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_RUN_1, 5); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_GENERIC_CRASH_1, 11); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_CAR_CRASH_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_BLOCKED_1, 10); break;
	case SOUND_PED_WAIT_DOUBLEBACK: return SFX_WMYRI_LOST_1;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_EYEING_1, 3); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_SHOCKED_1, 4); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_BUMP_1, 8); break;
#ifdef FIX_BUGS
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYRI_CHAT_1, 10); break;
#endif
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMORI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_GUN_PANIC_1, 9); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_MUGGED_1, 4); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_SAVED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_TAXI_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_DODGE_1, 10); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_RUN_1, 12); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_CAR_CRASH_1, 6); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_BLOCKED_1, 10); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_EYEING_1, 3); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_SHOCKED_1, 4); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMORI_BUMP_1, 14); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFYBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBE_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBE_JACKED_1, 4); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WFYBE_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WFYBE_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBE_DODGE_1, 8); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBE_RUN_1, 5); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBE_GENERIC_CRASH_1, 6); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBE_CAR_CRASH_1, 6); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBE_BLOCKED_1, 7); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBE_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBE_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBE_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMYBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_GUN_PANIC_1, 8); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_JACKING_1, 3); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_JACKED_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_DODGE_1, 12); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_RUN_1, 5); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_BLOCKED_1, 9); break;
	case SOUND_PED_JEER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_JEER_1, 7); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_LOST_1, 3); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_SHOCKED_1, 6); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_BUMP_1, 14); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBE_CHAT_1, 11); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFOBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_JACKED_1, 4); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_SAVED_1, 3); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_TAXI_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_DODGE_1, 8); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_RUN_1, 7); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_GENERIC_CRASH_1, 10); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_CAR_CRASH_1, 7); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_BLOCKED_1, 8); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOBE_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMOBE(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_JACKING_1, 4); break;
	case SOUND_PED_MUGGING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_MUGGING_1, 6); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_JACKED_1, 8); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_SAVED_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_DODGE_1, 8); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_RUN_1, 4); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_BLOCKED_1, 6); break;
	case SOUND_PED_JEER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_JEER_1, 16); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_BUMP_1, 12); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMYCW(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_GUN_PANIC_1, 6); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_JACKING_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: return SFX_WMYCW_MUGGED_1;
#ifdef FIX_BUGS
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_INNOCENT_1, 3); break;
#endif
	case SOUND_PED_TAXI_WAIT: return SFX_WMYCW_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_FIGHT_1, 8); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_DODGE_1, 10); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_CAR_CRASH_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_BLOCKED_1, 9); break;
	case SOUND_PED_JEER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_JEER_1, 5); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_EYEING_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_BUMP_1, 9); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYCW_CHAT_1, 15); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMYGO(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WMYGO_SAVED_1;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_TAXI_1, 3); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_DODGE_1, 11); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_RUN_1, 6); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_CAR_CRASH_1, 7); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_BUMP_1, 9); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYGO_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFOGO(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOGO_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOGO_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOGO_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WFOGO_SAVED_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOGO_FIGHT_1, 14); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOGO_DODGE_1, 9); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOGO_RUN_1, 2); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOGO_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOGO_CAR_CRASH_1, 8); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOGO_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOGO_BUMP_1, 8); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOGO_CHAT_1, 11); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMOGO(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOGO_GUN_PANIC_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOGO_JACKED_1, 6); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WMOGO_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WMOGO_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOGO_FIGHT_1, 13); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOGO_DODGE_1, 12); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOGO_RUN_1, 5); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOGO_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOGO_CAR_CRASH_1, 9); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOGO_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOGO_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOGO_BUMP_1, 8); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOGO_CHAT_1, 9); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFYLG(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYLG_GUN_COOL_1, 5); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WFYLG_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WFYLG_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYLG_FIGHT_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYLG_DODGE_1, 8); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYLG_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYLG_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMYLG(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYLG_GUN_COOL_1, 6); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WMYLG_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WMYLG_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYLG_FIGHT_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYLG_DODGE_1, 9); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYLG_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYLG_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFYBU(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBU_GUN_PANIC_1, 8); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBU_JACKED_1, 8); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBU_MUGGED_1, 4); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBU_TAXI_1, 2); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBU_RUN_1, 8); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBU_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBU_CAR_CRASH_1, 9); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBU_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYBU_BUMP_1, 21); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMYBU(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_GUN_PANIC_1, 6); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_JACKED_1, 5); break;
	case SOUND_PED_ROBBED: return SFX_WMYBU_MUGGED_1;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_SAVED_1, 2); break;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_INNOCENT_1, 2); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_TAXI_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_DODGE_1, 10); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_RUN_1, 3); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_GENERIC_CRASH_1, 5); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_CAR_CRASH_1, 9); break;
#ifdef FIX_BUGS
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_BLOCKED_1, 9); break;
#else
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBE_BLOCKED_1, 6); break;
#endif
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_LOST_1, 5); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_SHOCKED_1, 5); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYBU_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMOBU(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_GUN_PANIC_1, 6); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_JACKED_1, 7); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_SAVED_1, 3); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_TAXI_1, 2); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_FIGHT_1, 3); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_DODGE_1, 8); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_CAR_CRASH_1, 7); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_BLOCKED_1, 7); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_LOST_1, 3); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_EYEING_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOBU_BUMP_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFYPR(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYPR_GUN_COOL_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYPR_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WFYPR_SAVED_1;
	case SOUND_PED_PLAYER_BEFORESEX: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYPR_FUCKING_1, 5); break;
	case SOUND_PED_TAXI_WAIT: return SFX_WFYPR_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYPR_FIGHT_1, 9); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYPR_DODGE_1, 10); break;
	case SOUND_PED_SOLICIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYPR_SOLICIT_1, 15); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYPR_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYPR_CHAT_1, 14); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFOTR(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOTR_GUN_COOL_1, 6); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WFOTR_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WFOTR_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOTR_DODGE_1, 9); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOTR_RUN_1, 6); break;
	case SOUND_PED_SOLICIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOTR_SOLICIT_1, 9); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOTR_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOTR_CHAT_1, 9); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMOTR(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOTR_GUN_COOL_1, 5); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WMOTR_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WMOTR_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOTR_FIGHT_1, 6); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOTR_DODGE_1, 17); break;
	case SOUND_PED_SOLICIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOTR_SOLICIT_1, 7); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOTR_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOTR_SHOCKED_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOTR_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOTR_CHAT_1, 13); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMYPI(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_JACKING_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_SAVED_1, 2); break;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_INNOCENT_1, 2); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_TAXI_1, 4); break;
#ifdef FIX_BUGS
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_FIGHT_1, 9); break;
#else
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_FIGHT_1, 7); break;
#endif
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_DODGE_1, 8); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_BLOCKED_1, 8); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_EYEING_1, 6); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYPI_BUMP_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMOCA(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOCA_GUN_PANIC_1, 6); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOCA_JACKING_1, 11); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOCA_JACKED_1, 10); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOCA_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WMOCA_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WMOCA_TAXI_1;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOCA_FIGHT_1, 8); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOCA_DODGE_1, 10); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOCA_GENERIC_CRASH_1, 9); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOCA_CAR_CRASH_1, 10); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOCA_BLOCKED_1, 12); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOCA_EYEING_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMOCA_BUMP_1, 6); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFYSH(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSH_GUN_COOL_1, 9); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSH_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSH_SAVED_1, 4); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSH_TAXI_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSH_DODGE_1, 11); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSH_RUN_1, 11); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSH_LOST_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSH_SHOCKED_1, 5); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSH_BUMP_1, 12); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYSH_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFOSH(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOSH_GUN_COOL_1, 10); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOSH_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOSH_SAVED_1, 3); break;
	case SOUND_PED_TAXI_WAIT: return SFX_WFOSH_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOSH_DODGE_1, 10); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOSH_RUN_1, 9); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOSH_LOST_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOSH_SHOCKED_1, 5); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOSH_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFOSH_CHAT_1, 9); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_JFOTO(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_GUN_PANIC_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_JACKED_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_SAVED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_TAXI_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_DODGE_1, 9); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_RUN_1, 5); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_GENERIC_CRASH_1, 6); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: return SFX_JFOTO_LOST_1;
	case SOUND_PED_CHAT_EVENT: return SFX_JFOTO_SHOCKED_1;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JFOTO_CHAT_1, 13); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_JMOTO(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JMOTO_GUN_PANIC_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JMOTO_JACKED_1, 4); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JMOTO_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_JMOTO_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_JMOTO_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JMOTO_DODGE_1, 6); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JMOTO_RUN_1, 4); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JMOTO_GENERIC_CRASH_1, 6); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JMOTO_CAR_CRASH_1, 6); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JMOTO_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: return SFX_JMOTO_LOST_1;
	case SOUND_PED_CHAT_EVENT: return SFX_JMOTO_SHOCKED_1;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JMOTO_BUMP_1, 8); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_JMOTO_CHAT_1, 7); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HN(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_JACKING_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_MUGGED_1, 3); break;
	case SOUND_PED_ACCIDENTREACTION1: sfx = SFX_HAITIAN_GANG_1_SAVED_1; break;
	case SOUND_PED_TAXI_WAIT: sfx = SFX_HAITIAN_GANG_1_TAXI_1; break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_FIGHT_1, 10); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_DODGE_1, 10); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_GENERIC_CRASH_1, 9); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_CAR_CRASH_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_BLOCKED_1, 9); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_LOST_1, 4); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_EYEING_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_BUMP_1, 12); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HAITIAN_GANG_1_CHAT_1, 14); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return (SFX_HAITIAN_GANG_2_BLOCKED_1 - SFX_HAITIAN_GANG_1_BLOCKED_1) * (manager->m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32_t
AudioPedTalk_BK(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_JACKING_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_JACKED_1, 8); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: sfx = SFX_BIKER_GANG_1_SAVED_1; break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_TAXI_1, 2); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_FIGHT_1, 9); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_DODGE_1, 9); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_BLOCKED_1, 10); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_LOST_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BIKER_GANG_1_CHAT_1, 12); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return (SFX_BIKER_GANG_2_BLOCKED_1 - SFX_BIKER_GANG_1_BLOCKED_1) * (manager->m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32_t
AudioPedTalk_CB(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_JACKING_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_JACKED_1, 4); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: sfx = SFX_CUBAN_GANG_1_SAVED_1; break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_TAXI_1, 2); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_FIGHT_1, 9); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_DODGE_1, 9); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_CAR_CRASH_1, 8); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_EYEING_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_GANG_1_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return (SFX_CUBAN_GANG_2_BLOCKED_1 - SFX_CUBAN_GANG_1_BLOCKED_1) * (manager->m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32_t
AudioPedTalk_SG(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_JACKING_1, 5); break;
	case SOUND_PED_MUGGING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_MUGGING_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_JACKED_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_MUGGED_1, 3); break;
	case SOUND_PED_ACCIDENTREACTION1: sfx = SFX_STREET_GANG_1_SAVED_1; break;
	case SOUND_PED_TAXI_WAIT: sfx = SFX_STREET_GANG_1_TAXI_1; break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_FIGHT_1, 10); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_DODGE_1, 9); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_GENERIC_CRASH_1, 6); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_CAR_CRASH_1, 6); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_BLOCKED_1, 8); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_EYEING_1, 3); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_STREET_GANG_1_CHAT_1, 12); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	if(AudioPedTalkHost_Model(ped) == MI_SGB) sfx += (SFX_STREET_GANG_2_BLOCKED_1 - SFX_STREET_GANG_1_BLOCKED_1);
	return sfx;
}

uint32_t
AudioPedTalk_CL(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_GUN_COOL_1, 5); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_JACKING_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_JACKED_1, 6); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: sfx = SFX_CUBAN_LORD_GANG_1_SAVED_1; break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_TAXI_1, 2); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_FIGHT_1, 10); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_DODGE_1, 13); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_GENERIC_CRASH_1, 8); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_CAR_CRASH_1, 10); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_BLOCKED_1, 10); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_EYEING_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_CUBAN_LORD_GANG_1_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return (SFX_CUBAN_LORD_GANG_2_BLOCKED_1 - SFX_CUBAN_LORD_GANG_1_BLOCKED_1) * (manager->m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32_t
AudioPedTalk_GD(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GUARD_DUTY_1_GUN_COOL_1, 6); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GUARD_DUTY_1_SAVED_1, 2); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GUARD_DUTY_1_FIGHT_1, 7); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GUARD_DUTY_1_DODGE_1, 9); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GUARD_DUTY_1_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GUARD_DUTY_1_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GUARD_DUTY_1_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GUARD_DUTY_1_BUMP_1, 10); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_GUARD_DUTY_1_CHAT_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return (SFX_GUARD_DUTY_2_BUMP_1 - SFX_GUARD_DUTY_1_BUMP_1) * (manager->m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32_t
AudioPedTalk_PG(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_UP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_GUN_COOL_1, 4); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_JACKING_1, 5); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_JACKED_1, 5); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: sfx = SFX_PLAYER_GANG_1_SAVED_1; break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_TAXI_1, 2); break;
	case SOUND_PED_ATTACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_FIGHT_1, 5); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_DODGE_1, 7); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_GENERIC_CRASH_1, 5); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_CAR_CRASH_1, 5); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_BLOCKED_1, 10); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_EYEING_1, 2); break;
	case SOUND_PED_CHAT_EVENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_SHOCKED_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_BUMP_1, 5); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_PLAYER_GANG_1_CHAT_1, 8); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return (SFX_PLAYER_GANG_2_BLOCKED_1 - SFX_PLAYER_GANG_1_BLOCKED_1) * (manager->m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32_t
AudioPedTalk_ViceWhite(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;
	switch(sound) {
	case SOUND_PED_ARREST_COP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_VICE_VOICE_1_ARREST_1, 3); break;
	case SOUND_PED_MIAMIVICE_EXITING_CAR: sfx = SFX_VICE_VOICE_1_MIAMIVICE_EXITING_CAR_1; break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	sfx += (SFX_VICE_VOICE_2_ARREST_1-SFX_VICE_VOICE_1_ARREST_1) * (manager->m_sQueueSample.m_nEntityIndex % 5);
	return sfx;
}

uint32_t
AudioPedTalk_ViceBlack(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;
	switch(sound) {
	case SOUND_PED_ARREST_COP: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_VICE_VOICE_6_ARREST_1, 3); break;
	case SOUND_PED_MIAMIVICE_EXITING_CAR: return SFX_VICE_VOICE_6_MIAMIVICE_EXITING_CAR_1;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_BMODK(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_GUN_PANIC_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_JACKED_1, 9); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_MUGGED_1, 2); break;
	case SOUND_PED_INNOCENT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_INNOCENT_1, 3); break;
	case SOUND_PED_TAXI_WAIT: return SFX_BMODK_TAXI_1;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_DODGE_1, 7); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_RUN_1, 4); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_GENERIC_CRASH_1, 7); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_CAR_CRASH_1, 10); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_BLOCKED_1, 8); break;
	case SOUND_PED_147: // this is some cut behaviour, the guy was selling something
		AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_UNK_147_1, 11);
		// what is this? some sort of censorship?
		switch(sfx) {
		case SFX_BMODK_UNK_147_5:
		case SFX_BMODK_UNK_147_6:
		case SFX_BMODK_UNK_147_7: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_UNK_147_1, 4); break;
		default: break;
		}
		break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_BMODK_BUMP_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_HMYAP(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_GUN_PANIC_1, 7); break;
	case SOUND_PED_CAR_JACKING: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_JACKING_1, 4); break;
	case SOUND_PED_CAR_JACKED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_JACKED_1, 7); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_SAVED_1, 2); break;
	case SOUND_PED_TAXI_WAIT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_TAXI_1, 2); break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_DODGE_1, 9); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_RUN_1, 6); break;
	case SOUND_PED_CRASH_VEHICLE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_GENERIC_CRASH_1, 6); break;
	case SOUND_PED_CRASH_CAR: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_CAR_CRASH_1, 9); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_BLOCKED_1, 9); break;
	case SOUND_PED_WAIT_DOUBLEBACK: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_LOST_1, 2); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_EYEING_1, 3); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_BUMP_1, 11); break;
	case SOUND_PED_CHAT: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_HMYAP_CHAT_1, 9); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WFYJG(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;
	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYJG_GUN_PANIC_1, 4); break;
	case SOUND_PED_ACCIDENTREACTION1: sfx = SFX_WFYJG_SAVED_1; break;
	case SOUND_PED_TAXI_WAIT: sfx = SFX_WFYJG_TAXI_1; break;
	case SOUND_PED_EVADE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYJG_DODGE_1, 8); break;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYJG_RUN_1, 6); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WFYJG_BUMP_1, 12); break;
	default: return AudioPedTalk_GenericFemale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_WMYJG(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	uint32_t sfx;

	switch(sound) {
	case SOUND_PED_HANDS_COWER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYJG_GUN_PANIC_1, 4); break;
	case SOUND_PED_ROBBED: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYJG_MUGGED_1, 2); break;
	case SOUND_PED_ACCIDENTREACTION1: return SFX_WMYJG_SAVED_1;
	case SOUND_PED_TAXI_WAIT: return SFX_WMYJG_TAXI_1;
	case SOUND_PED_FLEE_RUN: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYJG_RUN_1, 5); break;
	case SOUND_PED_ANNOYED_DRIVER: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYJG_BLOCKED_1, 10); break;
	case SOUND_PED_CHAT_SEXY_MALE: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYJG_EYEING_1, 2); break;
	case SOUND_PED_PED_COLLISION: AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped), SFX_WMYJG_BUMP_1, 10); break;
	default: return AudioPedTalk_GenericMale(manager, ped, sound);
	}
	return sfx;
}

uint32_t
AudioPedTalk_SpecialCharacter(cAudioManager *manager, CPed *ped, int32_t model, uint16_t sound)
{
    // Preserve the original character selection and shared phrase history
	return NO_SAMPLE;
}

#undef AUDIO_PED_TALK_COOLDOWN
//- rouz edit (ChatGPT)
