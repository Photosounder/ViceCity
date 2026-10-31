//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "config.h"
#include "ZoneRecords.h"
typedef struct ZonesPosition { float x, y, z; } ZonesPosition;
#ifdef __cplusplus
#include "Vector.h"
static inline ZonesPosition
ZonesPosition_FromVector(const CVector &position)
{
    // Copy the original vector coordinates into the C value argument
    ZonesPosition result = { position.x, position.y, position.z };
    return result;
}
extern "C" {
#endif
uint16_t *Zone_GetTranslatedName(CZone *zone);
ZonesPosition ZonesHost_Player(void);
uint16_t *ZonesHost_Text(const char *name);
extern int16_t Zones_findIndex;
extern uint16_t Zones_audioCount;
extern int16_t Zones_audioIndices[NUMAUDIOZONES];
extern uint16_t Zones_mapCount;
extern uint16_t Zones_infoCount;
extern uint16_t Zones_navigationCount;
extern CZone Zones_info[NUMINFOZONES];
extern CZone Zones_map[NUMMAPZONES];
extern CZone Zones_navigation[NUMNAVIGZONES];
extern uint16_t Zones_zoneInfoCount;
extern CZoneInfo Zones_zoneInfo[2*NUMINFOZONES];
extern eLevelName Zones_currentLevel;
void Zones_Init(void);
void Zones_Update(void);
void Zones_CreateZone(char *name, eZoneType type,
	                       float minx, float miny, float minz,
	                       float maxx, float maxy, float maxz,
	                       eLevelName level);
static inline CZone *Zones_GetInfoZone(uint16_t i)
{
    // Preserve the original live record address or navigation pointer index
    return &Zones_info[i];
}
static inline CZone *Zones_GetNavigationZone(uint16_t i)
{
    // Preserve the original live record address or navigation pointer index
    return &Zones_navigation[i];
}
static inline CZone *Zones_GetMapZone(uint16_t i)
{
    // Preserve the original live record address or navigation pointer index
    return &Zones_map[i];
}
static inline CZone *Zones_GetAudioZone(uint16_t i)
{
    // Preserve the original live record address or navigation pointer index
    return &Zones_navigation[Zones_audioIndices[i]];
}
void Zones_PostZoneCreation(void);
void Zones_CheckZonesForOverlap(void);
void Zones_InsertZoneIntoZoneHierarchy(CZone *zone);
bool Zones_InsertZoneIntoZoneHierRecursive(CZone *z1, CZone *z2);
bool Zones_ZoneIsEntirelyContainedWithinOtherZone(CZone *z1, CZone *z2);
bool Zones_PointLiesWithinZone(ZonesPosition v, CZone *zone);
eLevelName Zones_GetLevelFromPosition(ZonesPosition v);
CZone *Zones_FindInformationZoneForPosition(ZonesPosition v);
CZone *Zones_FindSmallestNavigationZoneForPosition(ZonesPosition v, bool findDefault, bool findNavig);
int16_t Zones_FindZoneByLabelAndReturnIndex(char *name, eZoneType type);
int16_t Zones_FindNextZoneByLabelAndReturnIndex(char *name, eZoneType type);
CZoneInfo *Zones_GetZoneInfo(ZonesPosition v, uint8_t day);
void Zones_GetZoneInfoForTimeOfDay(ZonesPosition pos, CZoneInfo *info);
void Zones_SetZoneCarInfo(uint16_t zoneid, uint8_t day, int16_t carDensity,
		int16_t copCarDensity, const int16_t *gangCarDensities /*[NUMGANGS]*/);
void Zones_SetZoneCivilianCarInfo(uint16_t zoneid, uint8_t day,
		const int16_t* carDensities, const int16_t* boatDensities);
void Zones_SetZonePedInfo(uint16_t zoneid, uint8_t day, int16_t pedDensity,
		int16_t gang0Density, int16_t gang1Density, int16_t gang2Density, int16_t gang3Density,
		int16_t gang4Density, int16_t gang5Density, int16_t gang6Density, int16_t gang7Density,
		int16_t gang8Density, int16_t copDensity);
void Zones_SetCarDensity(uint16_t zoneid, uint8_t day, uint16_t cardensity);
void Zones_SetPedDensity(uint16_t zoneid, uint8_t day, uint16_t peddensity);
void Zones_SetPedGroup(uint16_t zoneid, uint8_t day, uint16_t pedgroup);
int16_t Zones_FindAudioZone(ZonesPosition pos);
static inline CZone *Zones_GetPointerForNavigationZoneIndex(ptrdiff_t i)
{
    // Preserve the original live record address or navigation pointer index
    return i == -1 ? NULL : &Zones_navigation[i];
}
static inline ptrdiff_t Zones_GetIndexForNavigationZonePointer(CZone *zone)
{
    // Preserve the original live record address or navigation pointer index
    return zone == NULL ? -1 : zone - Zones_navigation;
}
void Zones_AddZoneToAudioZoneArray(CZone *zone);
void Zones_InitialiseAudioZoneArray(void);
void Zones_SaveAllZones(uint8_t *buffer, uint32_t *length);
void Zones_SaveOneZone(CZone *zone, uint8_t **buffer, uint32_t *length, eZoneType zoneType);
void Zones_LoadAllZones(uint8_t *buffer, uint32_t length);
void Zones_LoadOneZone(CZone *zone, uint8_t **buffer, uint32_t *length, eZoneType zoneType);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
