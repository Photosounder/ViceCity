//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioPoliceGame.h"
#include "DMAudio.h"
#include <string.h>
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

void
AudioPolice_Service(cAudioManager *manager)
{
	// Preserve report scheduling, game gates, and queue insertion order
	int32_t wantedLevel = 0; // uninitialized variable
	static uint32_t nLastSeen = 300;

	if(!manager->m_bIsInitialised) return;

	if(!manager->m_bIsPaused) {
		uint8_t crimeReport = AudioPolice_SetupCrimeReport(manager);
#ifdef FIX_BUGS // Crash at 0x5fe6ef
		if(AudioPoliceHost_IsReplay() || !AudioPoliceHost_FindPlayer() || !AudioPoliceHost_HasWanted(AudioPoliceHost_FindPlayer()))
			return;
#endif
		CPlayerPed *playerPed = AudioPoliceHost_FindPlayer();
		if (playerPed) {
			wantedLevel = AudioPoliceHost_WantedLevel(playerPed);
			if (!crimeReport) {
				if (wantedLevel != 0) {
					if (nLastSeen != 0)
#ifdef FIX_BUGS
						nLastSeen -= AudioPoliceHost_LogicalFramesPassed();
#else
						nLastSeen--;
#endif
					else {
						nLastSeen = manager->m_anRandomTable[1] % 1000 + 2000;
						AudioPolice_SuspectReport(manager);
					}
				}
			}
		}
	}
	AudioPolice_ServiceChannel(manager, wantedLevel);
}

uint8_t
AudioPolice_SetupCrimeReport(cAudioManager *manager)
{
	// Preserve report scheduling, game gates, and queue insertion order
	int16_t audioZoneId;
	AudioPoliceZoneView zone;
	float rangeX;
	float rangeY;
	float halfX;
	float halfY;
	float quarterX;
	float quarterY;
	int i;
	uint32_t sampleIndex;
	uint8_t processed = 0;

	if (AudioPoliceHost_MusicMode() == MUSICMODE_CUTSCENE) return 0;

	if (POLICE_RADIO_QUEUE_MAX_SAMPLES - manager->m_sPoliceRadioQueue.m_nSamplesInQueue <= 9) {
		AudioCrimes_Age(manager->m_aCrimes);
		return 1;
	}

	for (i = 0; i < ARRAY_SIZE(manager->m_aCrimes); i++) {
		if (manager->m_aCrimes[i].type != CRIME_NONE)
			break;
	}

	if (i == ARRAY_SIZE(manager->m_aCrimes)) return 0;
	// Supply a game vector to the read-only audio-zone lookup
	AudioCrimePosition crimePosition = manager->m_aCrimes[i].position;
	audioZoneId = AudioPoliceHost_FindZone(&crimePosition);
	if (audioZoneId >= 0 && audioZoneId < NUMAUDIOZONES) {
		AudioPoliceHost_GetZone(audioZoneId, &zone);
		for (int j = 0; j < NUMAUDIOZONES; j++) {
			if (strcmp(zone.name, ZoneSfx[j].m_aName) == 0) {
				sampleIndex = ZoneSfx[j].m_nSampleIndex;
				PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_MESSAGE_NOISE_1);
				PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, manager->m_anRandomTable[0] % 3 + SFX_WEVE_GOT);
				PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_A_10);
				switch (manager->m_aCrimes[i].type) {
				case CRIME_PED_BURNED:
				case CRIME_HIT_PED_NASTYWEAPON:
					manager->m_aCrimes[i].type = CRIME_HIT_PED;
					break;
				case CRIME_COP_BURNED:
				case CRIME_HIT_COP_NASTYWEAPON:
					manager->m_aCrimes[i].type = CRIME_HIT_COP;
					break;
				case CRIME_VEHICLE_BURNED: manager->m_aCrimes[i].type = CRIME_STEAL_CAR; break;
				case CRIME_DESTROYED_CESSNA: manager->m_aCrimes[i].type = CRIME_SHOOT_HELI; break;
				case CRIME_EXPLOSION: manager->m_aCrimes[i].type = CRIME_STEAL_CAR; break; // huh?
				default: break;
				}
#ifdef FIX_BUGS
				PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, manager->m_aCrimes[i].type + SFX_CRIME_1 - 1);
#else
				PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, manager->m_aCrimes[i].type + SFX_CRIME_1);
#endif
				PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_IN);
				rangeX = *zone.maxx - *zone.minx;
				rangeY = *zone.maxy - *zone.miny;
				halfX = 0.5f * rangeX + *zone.minx;
				halfY = 0.5f * rangeY + *zone.miny;
				quarterX = 0.25f * rangeX;
				quarterY = 0.25f * rangeY;

				if (manager->m_aCrimes[i].position.y > halfY + quarterY) {
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_NORTH);
					processed = 1;
				} else if (manager->m_aCrimes[i].position.y < halfY - quarterY) {
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_SOUTH);
					processed = 1;
				}

				if (manager->m_aCrimes[i].position.x > halfX + quarterX)
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_EAST);
				else if (manager->m_aCrimes[i].position.x < halfX - quarterX)
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_WEST);
				else if (!processed)
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_CENTRAL);

				PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, sampleIndex);
				PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_MESSAGE_NOISE_1);
				PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, NO_SAMPLE);
				break;
			}
		}
	}
	manager->m_aCrimes[i].type = CRIME_NONE;
	AudioCrimes_Age(manager->m_aCrimes);
	return 1;
}

void
AudioPolice_PlaySuspect(cAudioManager *manager, float x, float y, float z)
{
	// Preserve report scheduling, game gates, and queue insertion order
	int16_t audioZone;
	AudioPoliceZoneView zone;
	float rangeX;
	float rangeY;
	float halfX;
	float halfY;
	float quarterX;
	float quarterY;
	uint32_t sample;
	uint8_t processed = 0;
	AudioCrimePosition vec = { x, y, z };

	if (!manager->m_bIsInitialised) return;

	if (AudioPoliceHost_MusicMode() != MUSICMODE_CUTSCENE && POLICE_RADIO_QUEUE_MAX_SAMPLES - manager->m_sPoliceRadioQueue.m_nSamplesInQueue > 9) {
		audioZone = AudioPoliceHost_FindZone(&vec);
		if (audioZone >= 0 && audioZone < NUMAUDIOZONES) {
			AudioPoliceHost_GetZone(audioZone, &zone);
			for (int i = 0; i < NUMAUDIOZONES; i++) {
				if (strcmp(zone.name, ZoneSfx[i].m_aName) == 0) {
					sample = ZoneSfx[i].m_nSampleIndex;
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_MESSAGE_NOISE_1);
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_SUSPECT);
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_LAST_SEEN);
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_IN);
					rangeX = *zone.maxx - *zone.minx;
					rangeY = *zone.maxy - *zone.miny;
					halfX = 0.5f * rangeX + *zone.minx;
					halfY = 0.5f * rangeY + *zone.miny;
					quarterX = 0.25f * rangeX;
					quarterY = 0.25f * rangeY;

					if (vec.y > halfY + quarterY) {
						PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_NORTH);
						processed = 1;
					} else if (vec.y < halfY - quarterY) {
						PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_SOUTH);
						processed = 1;
					}

					if (vec.x > halfX + quarterX)
						PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_EAST);
					else if (vec.x < halfX - quarterX)
						PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_WEST);
					else if (!processed)
						PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_CENTRAL);
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, sample);
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_MESSAGE_NOISE_1);
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, NO_SAMPLE);
					gSpecialSuspectLastSeenReport = 1;
					break;
				}
			}
		}
	}
}

void
DMAudio_PlaySuspectLastSeen(float x, float y, float z)
{
	// Submit the coordinate-based report through the C interface
	AudioPolice_PlaySuspect(&AudioManager, x, y, z);
}
//- rouz edit (ChatGPT)
