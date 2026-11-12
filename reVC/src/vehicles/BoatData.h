//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "BoatNodes.h"
#include "Door.h"
#include "HandlingMgr.h"

// Types keep their engine form in C++ and plain data in C
#ifdef __cplusplus
#define BOATDATA_FRAME RwFrame
typedef CVector BoatVec3;
typedef CVector2D BoatVec2;
class CEntity;
#else
#define BOATDATA_FRAME void
typedef struct BoatVec3 {float x,y,z;} BoatVec3;
typedef struct BoatVec2 {float x,y;} BoatVec2;
typedef struct CEntity CEntity;
#endif

// The boat state that follows CVehicle, as plain data usable from C
typedef struct CBoatData {
	float m_fMovingRotation;
	float m_fMovingSpeed;
	int32_t m_boat_unused1;
	BOATDATA_FRAME *m_aBoatNodes[NUM_BOAT_NODES];
	CDoor m_boom;
	tBoatHandlingData *pBoatHandling;
	uint8_t bBoatInWater : 1;
	uint8_t bPropellerInWater : 1;
	bool m_bIsAnchored;
	float m_fOrientation;
	uint32_t m_nPoliceShoutTimer;
	int32_t m_boat_unused2;
	float m_fDamage;
	CEntity *m_pSetOnFireEntity;
	float m_skimmerThingTimer;
	bool m_boat_unused3;
	float m_fAccelerate;
	float m_fBrake;
	float m_fSteeringLeftRight;
	uint8_t m_nPadID;
	int32_t m_boat_unused4;
	float m_fVolumeUnderWater;
	BoatVec3 m_vecBuoyancePoint;
	float m_fPrevVolumeUnderWater;
	int16_t m_nDeltaVolumeUnderWater;
	uint16_t m_nNumWakePoints;
	BoatVec2 m_avec2dWakePoints[32];
	float m_afWakePointLifeTime[32];
} CBoatData;
//- rouz edit (Claude)
