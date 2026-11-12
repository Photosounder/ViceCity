//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "MatrixData.h"

enum {
	UNKNOWN_OBJECT = 0,
	GAME_OBJECT = 1,
	MISSION_OBJECT = 2,
	TEMP_OBJECT = 3,
	CUTSCENE_OBJECT = 4,
	CONTROLLED_SUB_OBJECT = 5,
};

enum CollisionSpecialResponseCase
{
	COLLRESPONSE_NONE,
	COLLRESPONSE_LAMPOST,
	COLLRESPONSE_SMALLBOX,
	COLLRESPONSE_BIGBOX,
	COLLRESPONSE_FENCEPART,
	COLLRESPONSE_UNKNOWN5
};

// The matrix keeps its engine type in C++ and the plain data layout in C
#ifdef __cplusplus
#define OBJECTSTATE_MATRIX CMatrix
#else
#define OBJECTSTATE_MATRIX CMatrixData
typedef struct CEntity CEntity;
#endif

// The object state that follows CPhysical, as plain data usable from C
typedef struct CObjectState {
	OBJECTSTATE_MATRIX m_objectMatrix;
	float m_fUprootLimit;
	int8_t ObjectCreatedBy;
	uint8_t bIsPickup : 1;
	uint8_t bAmmoCollected : 1;
	uint8_t bPickupObjWithMessage : 1;
	uint8_t bOutOfStock : 1;
	uint8_t bGlassCracked : 1;
	uint8_t bGlassBroken : 1;
	uint8_t bHasBeenDamaged : 1;
	uint8_t bUseVehicleColours : 1;
	uint8_t bIsWeapon : 1;
	uint8_t bIsStreetLight : 1;
	int8_t m_nBonusValue;
	uint16_t m_nCostValue;
	float m_fCollisionDamageMultiplier;
	uint8_t m_nCollisionDamageEffect;
	uint8_t m_nSpecialCollisionResponseCases;
	bool m_bCameraToAvoidThisObject;
	uint8_t m_nBeachballBounces;
	uint32_t m_obj_unused1;
	uint32_t m_nEndOfLifeTime;
	int16_t m_nRefModelIndex;
	CEntity *m_pCurSurface;
	CEntity *m_pCollidingEntity;
	int8_t m_colour1, m_colour2;
} CObjectState;
//- rouz edit (Claude)
