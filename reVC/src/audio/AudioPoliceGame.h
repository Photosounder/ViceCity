#pragma once
//+ rouz edit (ChatGPT)
#include <stdint.h>
#include "../core/config.h"
#include "AudioCrimes.h"
#ifdef __cplusplus
class CPlayerPed;
class CVehicle;
class CEntity;
class CVector;
extern "C" {
#else
typedef struct CPlayerPed CPlayerPed;
typedef struct CVehicle CVehicle;
typedef struct CEntity CEntity;
typedef struct CVector CVector;
#endif
typedef struct cAudioManager cAudioManager;
typedef struct AudioPoliceZoneView {
	const char *name;
	const float *minx, *miny, *maxx, *maxy;
} AudioPoliceZoneView;
CVehicle *AudioGame_FindVehicle(void);
CVehicle *AudioPoliceHost_PlayerVehicle(void);
CEntity *AudioPoliceHost_AttachedTo(const CPlayerPed *player);
uint8_t AudioPoliceHost_IsVehicle(CEntity *entity);
CVehicle *AudioPoliceHost_AsVehicle(CEntity *entity);
void AudioPoliceHost_CrimePosition(const CVector *position, AudioCrimePosition *plain);
uint8_t AudioPoliceHost_VehicleColor(const CVehicle *vehicle);
int16_t AudioPoliceHost_VehicleModel(const CVehicle *vehicle);
uint8_t AudioPoliceHost_MusicMode(void);
CPlayerPed *AudioPoliceHost_FindPlayer(void);
uint8_t AudioPoliceHost_HasWanted(const CPlayerPed *player);
int32_t AudioPoliceHost_WantedLevel(const CPlayerPed *player);
#ifdef FIX_BUGS
uint8_t AudioPoliceHost_IsReplay(void);
#endif
int16_t AudioPoliceHost_FindZone(const AudioCrimePosition *position);
void AudioPoliceHost_GetZone(int16_t id, AudioPoliceZoneView *view);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
