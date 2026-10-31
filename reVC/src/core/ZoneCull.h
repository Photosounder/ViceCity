//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "Pool.h"
enum eZoneAttribs
{
	ATTRZONE_CAMCLOSEIN		= 1,
	ATTRZONE_STAIRS			= 2,
	ATTRZONE_1STPERSON		= 4,
	ATTRZONE_NORAIN			= 8,
	ATTRZONE_NOPOLICE		= 0x10,
	ATTRZONE_NOTCULLZONE		= 0x20,
	ATTRZONE_DOINEEDCOLLISION	= 0x40,
	ATTRZONE_SUBWAYVISIBLE		= 0x80,
	ATTRZONE_POLICEABANDONCARS	= 0x100,
	ATTRZONE_ROOMFORAUDIO		= 0x200,
	ATTRZONE_WATERFUDGE		= 0x400,
};

typedef struct CAttributeZone
{
	int16_t minx;
	int16_t maxx;
	int16_t miny;
	int16_t maxy;
	int16_t minz;
	int16_t maxz;
	int16_t attributes;
	int16_t wantedLevel;
} CAttributeZone;

typedef struct CullZonesPosition { float x, y, z; } CullZonesPosition;
#ifdef __cplusplus
#include "Vector.h"
class CEntity;
class CVehicle;
static inline CullZonesPosition
CullZonesPosition_FromVector(const CVector &position)
{
    // Copy the original vector coordinates into the C value argument
    CullZonesPosition result = { position.x, position.y, position.z };
    return result;
}
extern "C" {
#else
typedef struct CEntity CEntity;
typedef struct CVehicle CVehicle;
#endif
extern int32_t CullZones_numAttributeZones;
extern CAttributeZone CullZones_attributeZones[NUMATTRIBZONES];
extern int32_t CullZones_playerWantedLevelDrop;
extern int32_t CullZones_cameraFlags;
extern int32_t CullZones_playerFlags;
extern bool CullZones_subwayInvisible;
extern bool CullZones_atBeachForAudio;
void CullZones_Init(void);
void CullZones_Update(void);
void CullZones_UpdateAtBeachForAudio(void);
void CullZones_ForceCullZoneCoors(CullZonesPosition coors);
int32_t CullZones_FindAttributesForCoors(CullZonesPosition coors, int32_t *wantedLevel);
CAttributeZone *CullZones_FindZoneWithStairsAttributeForPlayer(void);
void CullZones_MarkSubwayAsInvisible(bool visible);
void CullZones_AddCullZone(const void *position, float minx, float maxx, float miny, float maxy, float minz, float maxz, uint16_t flag, int16_t wantedLevel);
bool IsPointWithinArbitraryArea(float px, float py, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
CullZonesPosition CullZonesHost_GameCamera(void);
CullZonesPosition CullZonesHost_Player(void);
float CullZonesHost_CameraX(void);
float CullZonesHost_CameraY(void);
CPool * CullZonesHost_BuildingPool(void);
CPool * CullZonesHost_TreadablePool(void);
CPool * CullZonesHost_VehiclePool(void);
CEntity * CullZonesHost_Building(void *slot);
CEntity * CullZonesHost_Treadable(void *slot);
CVehicle * CullZonesHost_Vehicle(void *slot);
bool CullZonesHost_IsSubway(CEntity *entity);
bool CullZonesHost_IsTrain(CVehicle *vehicle);
uint8_t CullZonesHost_Track(CVehicle *vehicle);
void CullZonesHost_SetEntityVisible(CEntity *entity, bool visible);
void CullZonesHost_SetVehicleVisible(CVehicle *entity, bool visible);
static inline bool CullZones_CamCloseInForPlayer(void)
{
    // Read the original live zone flags or wanted-level drop
    return (CullZones_playerFlags & ATTRZONE_CAMCLOSEIN) != 0;
}
static inline bool CullZones_CamStairsForPlayer(void)
{
    // Read the original live zone flags or wanted-level drop
    return (CullZones_playerFlags & ATTRZONE_STAIRS) != 0;
}
static inline bool CullZones_Cam1stPersonForPlayer(void)
{
    // Read the original live zone flags or wanted-level drop
    return (CullZones_playerFlags & ATTRZONE_1STPERSON) != 0;
}
static inline bool CullZones_NoPolice(void)
{
    // Read the original live zone flags or wanted-level drop
    return (CullZones_playerFlags & ATTRZONE_NOPOLICE) != 0;
}
static inline bool CullZones_DoINeedToLoadCollision(void)
{
    // Read the original live zone flags or wanted-level drop
    return (CullZones_playerFlags & ATTRZONE_DOINEEDCOLLISION) != 0;
}
static inline bool CullZones_PlayerNoRain(void)
{
    // Read the original live zone flags or wanted-level drop
    return (CullZones_playerFlags & ATTRZONE_NORAIN) != 0;
}
static inline bool CullZones_CamNoRain(void)
{
    // Read the original live zone flags or wanted-level drop
    return (CullZones_cameraFlags & ATTRZONE_NORAIN) != 0;
}
static inline bool CullZones_PoliceAbandonCars(void)
{
    // Read the original live zone flags or wanted-level drop
    return (CullZones_cameraFlags & ATTRZONE_POLICEABANDONCARS) != 0;
}
static inline bool CullZones_InRoomForAudio(void)
{
    // Read the original live zone flags or wanted-level drop
    return (CullZones_cameraFlags & ATTRZONE_ROOMFORAUDIO) != 0;
}
static inline bool CullZones_WaterFudge(void)
{
    // Read the original live zone flags or wanted-level drop
    return (CullZones_cameraFlags & ATTRZONE_WATERFUDGE) != 0;
}
static inline int32_t CullZones_GetWantedLevelDrop(void)
{
    // Read the original live zone flags or wanted-level drop
    return CullZones_playerWantedLevelDrop;
}
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
