//+ rouz edit (Claude)
#include <stddef.h>
#include "PedChat.h"
#include "soundlist.h"
#include "PedNode.h"

// Corresponds to ped sounds (from SOUND_PED_DEATH to SOUND_PED_TAXI_CALL)
PedAudioData CommentWaitTime[56] = {
	{ 500, 800, 500, 2 },
	{ 500, 800, 500, 2 },
	{ 500, 800, 500, 2 },
	{ 500, 800, 500, 2 },
	{ 100, 2, 100, 2 },
	{ 500, 500, 2000, 1000 },
	{ 2000, 50, 2050, 1000 },
	{ 5000, 2000, 7000, 3000 },
	{ 5000, 2000, 7000, 3000 },
	{ 300, 200, 500, 200 },
	{ 3000, 1000, 4000, 1000 },
	{ 6000, 6000, 6000, 6000 },
	{ 4000, 1000, 5000, 1000 },
	{ 3000, 1000, 4000, 1000 },
	{ 1000, 1000, 2000, 2000 },
	{ 1000, 500, 2000, 1500 },
	{ 1700, 1000, 3000, 1000 },
	{ 800, 200, 1000, 500 },
	{ 800, 200, 1000, 500 },
	{ 800, 400, 2000, 1000 },
	{ 800, 400, 2000, 1000 },
	{ 2000, 2000, 4000, 4000 },
	{ 2000, 2000, 4000, 1000 },
	{ 4000, 1000, 5000, 1000 },
	{ 800, 400, 1200, 500 },
	{ 5000, 1000, 6000, 2000 },
	{ 5000, 1000, 6000, 2000 },
	{ 5000, 1000, 6000, 2000 },
	{ 5000, 1000, 6000, 2000 },
	{ 5000, 1000, 6000, 2000 },
	{ 5000, 1000, 6000, 2000 },
	{ 5000, 1000, 6000, 2000 },
	{ 4000, 2000, 7000, 2000 },
	{ 1000, 300, 2000, 1000 },
	{ 1500, 1000, 2500, 1000 },
	{ 200, 200, 200, 200 },
	{ 2000, 1000, 4000, 1000 },
	{ 2000, 1000, 4000, 1000 },
	{ 1000, 500, 3000, 1000 },
	{ 1000, 500, 1000, 1000 },
	{ 3000, 2000, 5000, 1000 },
	{ 3000, 2000, 5000, 1000 },
	{ 3000, 2000, 3000, 2000 },
	{ 2000, 1000, 3000, 1000 },
	{ 2500, 1000, 5000, 5000 },
	{ 2000, 1000, 3000, 2000 },
	{ 4000, 1000, 5000, 1000 },
	{ 1000, 500, 2000, 4000 },
	{ 1000, 500, 2000, 5000 },
	{ 2000, 500, 2500, 500 },
	{ 1000, 500, 3000, 2000 },
	{ 1600, 1000, 2000, 2000 },
	{ 2000, 1000, 4000, 2000 },
	{ 1500, 1000, 2500, 1000 },
	{ 1000, 1000, 5000, 5000 },
	{ 0, 0, 0, 0 },
};

bool PedChat_ServiceTalkingWhenDead(const CPedData *self)
{
	// Only the death cry counts
	return self->m_queuedSound == SOUND_PED_DEATH;
}

uint16_t PedChat_ServiceTalking(CPedData *self, bool germanGame, bool isPlayer, uint32_t now, PedChatRandomFn random)
{
	// A head that just came off stays silent
	if (self->bBodyPartJustCameOff && self->m_bodyPartBleeding == PED_HEAD)
		return SOUND_NO_SOUND;

	// Burning peds scream
	if (!germanGame && self->m_pFire)
		self->m_queuedSound = SOUND_PED_BURNING;

	if (self->m_queuedSound != SOUND_NO_SOUND) {
		// The death cry is never delayed
		if (self->m_queuedSound == SOUND_PED_DEATH)
			self->m_soundStart = now - 1;

		if (now > self->m_soundStart) {
			const PedAudioData *d = &CommentWaitTime[self->m_queuedSound - SOUND_PED_DEATH];
			self->m_lastSoundStart = now;
			self->m_soundStart = d->m_nFixedDelayTime + now + random(0, d->m_nOverrideFixedDelayTime);

			if (self->m_queuedSound == SOUND_PED_PLAYER_BEFORESEX && isPlayer)
				self->m_soundStart += 2000;

			{
				uint16_t played = self->m_queuedSound;
				self->m_lastQueuedSound = played;
				self->m_queuedSound = SOUND_NO_SOUND;
				return played;
			}
		}
	}
	return SOUND_NO_SOUND;
}

void PedChat_Say(CPedData *self, uint16_t audio, float camZ, float pedZ, float camSpeed, uint32_t now, PedChatRandomFn random)
{
	// Peds far below the camera are silent
	if (3.0f + camZ < pedZ)
		return;

	// The faster the camera moves the fewer sounds are allowed
	if (camSpeed > 1.65f) {
		if (audio != SOUND_PED_DAMAGE && audio != SOUND_PED_HIT && audio != SOUND_PED_LAND)
			return;

	} else if (camSpeed > 1.25f) {
		if (audio != SOUND_PED_DEATH &&
			audio != SOUND_PED_DAMAGE && audio != SOUND_PED_HIT && audio != SOUND_PED_LAND &&
			audio != SOUND_PED_TAXI_WAIT && audio != SOUND_PED_EVADE)
			return;

	} else if (camSpeed > 0.9f) {
		switch (audio) {
			case SOUND_PED_DEATH:
			case SOUND_PED_DAMAGE:
			case SOUND_PED_HIT:
			case SOUND_PED_LAND:
			case SOUND_PED_BURNING:
			case SOUND_PED_FLEE_SPRINT:
			case SOUND_PED_TAXI_WAIT:
			case SOUND_PED_EVADE:
			case SOUND_PED_CRASH_VEHICLE:
			case SOUND_PED_CRASH_CAR:
			case SOUND_PED_ANNOYED_DRIVER:
				break;
			default:
				return;
		}
	}

	// A more important sound replaces the queued one unless the same sound was said too recently
	if (audio < self->m_queuedSound) {
		if (audio != self->m_lastQueuedSound || audio == SOUND_PED_DEATH
			|| (uint32_t)CommentWaitTime[audio - SOUND_PED_DEATH].m_nOverrideMaxRandomDelayTime
				+ (uint32_t)random(0, CommentWaitTime[audio - SOUND_PED_DEATH].m_nMaxRandomDelayTime)
				+ self->m_lastSoundStart <= now) {
			self->m_queuedSound = audio;
		}
	}
}
//- rouz edit (Claude)
