//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "GarageTypes.h"
#include "StoredCar.h"

// The 2D vectors keep the engine type in C++ and plain floats in C
#ifdef __cplusplus
typedef CVector2D GarageVec2;
class CEntity;
#else
typedef struct GarageVec2 {float x,y;} GarageVec2;
typedef struct CEntity CEntity;
#endif

// The plain data of one garage, usable from C
typedef struct CGarageData {
	uint8_t m_eGarageType;
	uint8_t m_eGarageState;
	uint8_t m_nMaxStoredCars;
	bool field_2; // unused
	bool m_bClosingWithoutTargetCar;
	bool m_bDeactivated;
	bool m_bResprayHappened;
	int32_t m_nTargetModelIndex;
	CEntity *m_pDoor1;
	CEntity *m_pDoor2;
	uint8_t m_bDoor1PoolIndex;
	uint8_t m_bDoor2PoolIndex;
	bool m_bDoor1IsDummy;
	bool m_bDoor2IsDummy;
	bool m_bRecreateDoorOnNextRefresh;
	bool m_bRotatedDoor;
	bool m_bCameraFollowsPlayer;
	GarageVec2 m_vecCorner1;
	float m_fInfZ;
	GarageVec2 m_vDir1;
	GarageVec2 m_vDir2;
	float m_fSupZ;
	float m_fDir1Len;
	float m_fDir2Len;
	float m_fInfX;
	float m_fSupX;
	float m_fInfY;
	float m_fSupY;
	float m_fDoorPos;
	float m_fDoorHeight;
	float m_fDoor1X;
	float m_fDoor1Y;
	float m_fDoor2X;
	float m_fDoor2Y;
	float m_fDoor1Z;
	float m_fDoor2Z;
	uint32_t m_nTimeToStartAction;
	uint8_t m_bCollectedCarsState;
	CVehicle *m_pTarget;
	CStoredCar m_sStoredCar; // not needed
} CGarageData;
//- rouz edit (Claude)
