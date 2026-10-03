//+ rouz edit (ChatGPT)
#include "common.h"

#include "DMAudio.h"

#include "AudioManager.h"

#include "AudioSamples.h"
#include "MusicManager.h"
#include "PlayerPed.h"
#include "PolRadio.h"
#include "AudioPoliceGame.h"
#include "Replay.h"
#include "Vehicle.h"
#include "World.h"
#include "Zones.h"
#include "sampman.h"
#include "Wanted.h"








//+ rouz edit (ChatGPT)

#ifdef FIX_BUGS
uint32_t
AudioPoliceHost_LogicalFramesPassed(void)
{
	// Read the existing game timer at the original wait-counter update point
	return CTimer::GetLogicalFramesPassed();
}
#endif

#ifdef USE_TIME_SCALE_FOR_AUDIO
float
AudioPoliceHost_TimeScale(void)
{
	// Read the existing game time scale at the original frequency update point
	return CTimer::GetTimeScale();
}
#endif
//- rouz edit (ChatGPT)





//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
uint8_t
AudioPoliceHost_MusicMode(void)
{
	// Read the existing music-mode gate at the report decision point
	return MusicManager.m_nMusicMode;
}

CPlayerPed *
AudioPoliceHost_FindPlayer(void)
{
	// Preserve each individual player lookup and its short-circuit position
	return FindPlayerPed();
}

uint8_t
AudioPoliceHost_HasWanted(const CPlayerPed *player)
{
	// Read the original wanted pointer without changing the player lookup sequence
	return player->m_pWanted != NULL;
}

int32_t
AudioPoliceHost_WantedLevel(const CPlayerPed *player)
{
	// Read the existing wanted level after the original player gate
	return player->m_pWanted->GetWantedLevel();
}

#ifdef FIX_BUGS
uint8_t
AudioPoliceHost_IsReplay(void)
{
	// Retain the original replay gate and short-circuit order
	return CReplay::IsPlayingBack();
}
#endif

int16_t
AudioPoliceHost_FindZone(const AudioCrimePosition *position)
{
	// Supply the same three coordinates to the existing audio-zone lookup
	CVector gamePosition(position->x, position->y, position->z);
	return CTheZones::FindAudioZone(&gamePosition);
}

void
AudioPoliceHost_GetZone(int16_t id, AudioPoliceZoneView *view)
{
	// Expose pointers to the existing zone fields without copying their values
	const CZone *zone = CTheZones::GetAudioZone(id);
	view->name = zone->name;
	view->minx = &zone->minx;
	view->miny = &zone->miny;
	view->maxx = &zone->maxx;
	view->maxy = &zone->maxy;
}

//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)

uint8_t
AudioPoliceHost_VehicleColor(const CVehicle *vehicle)
{
	// Read the original primary vehicle color after the report capacity gate
	return vehicle->m_currentColour1;
}

int16_t
AudioPoliceHost_VehicleModel(const CVehicle *vehicle)
{
	// Read the original model index after validating the color table index
	return vehicle->GetModelIndex();
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
CVehicle *
AudioPoliceHost_PlayerVehicle(void)
{
	// Preserve the original player vehicle lookup before the separate player lookup
	return FindPlayerVehicle();
}

CEntity *
AudioPoliceHost_AttachedTo(const CPlayerPed *player)
{
	// Read the existing attachment only when no player vehicle was found
	return player->m_attachedTo;
}

uint8_t
AudioPoliceHost_IsVehicle(CEntity *entity)
{
	// Preserve the original game entity type test before downcasting
	return entity->IsVehicle();
}

CVehicle *
AudioPoliceHost_AsVehicle(CEntity *entity)
{
	// Retain the original C++ base-to-derived conversion for the game object layout
	return (CVehicle*)entity;
}

void
AudioPoliceHost_CrimePosition(const CVector *position, AudioCrimePosition *plain)
{
	// Read the game coordinates only after all original crime eligibility checks pass
	plain->x = position->x;
	plain->y = position->y;
	plain->z = position->z;
}
//- rouz edit (ChatGPT)
