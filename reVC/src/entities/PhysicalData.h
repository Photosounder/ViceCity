//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include "Lists.h"

enum {
	PHYSICAL_MAX_COLLISIONRECORDS = 6
};

// The vector members keep the engine CVector type in C++ and the same three floats in C
#ifdef __cplusplus
typedef CVector PhysicalVec;
#else
typedef struct PhysicalVec {float x,y,z;} PhysicalVec;
typedef struct CEntity CEntity;
#endif

// The physical state that follows CEntity, as plain data usable from C
typedef struct CPhysicalData {
	int32_t m_audioEntityId;
	float m_phys_unused1;
	uint32_t m_nLastTimeCollided;
	PhysicalVec m_vecMoveSpeed;		// velocity
	PhysicalVec m_vecTurnSpeed;		// angular velocity
	PhysicalVec m_vecMoveFriction;
	PhysicalVec m_vecTurnFriction;
	PhysicalVec m_vecMoveSpeedAvg;
	PhysicalVec m_vecTurnSpeedAvg;
	float m_fMass;
	float m_fTurnMass;	// moment of inertia
	float m_fForceMultiplier;
	float m_fAirResistance;
	float m_fElasticity;
	float m_fBuoyancy;
	PhysicalVec m_vecCentreOfMass;
	CEntryInfoList m_entryInfoList;
	CPtrNode *m_movingListNode;

	int8_t m_phys_unused2;
	uint8_t m_nStaticFrames;
	uint8_t m_nCollisionRecords;
	CEntity *m_aCollisionRecords[PHYSICAL_MAX_COLLISIONRECORDS];

	float m_fDistanceTravelled;

	// damaged piece
	float m_fDamageImpulse;
	CEntity *m_pDamageEntity;
	PhysicalVec m_vecDamageNormal;
	int16_t m_nDamagePieceType;

	uint8_t bIsHeavy : 1;
	uint8_t bAffectedByGravity : 1;
	uint8_t bInfiniteMass : 1;
	uint8_t m_phy_flagA08 : 1;
	uint8_t bIsInWater : 1;
	uint8_t m_phy_flagA20 : 1; // unused
	uint8_t bHitByTrain : 1;
	uint8_t bSkipLineCol : 1;

	uint8_t bIsFrozen : 1;
	uint8_t bDontLoadCollision : 1;
	uint8_t m_bIsVehicleBeingShifted : 1;	// wrong name - also used on but never set for peds
	uint8_t bJustCheckCollision : 1;	// just see if there is a collision

	uint8_t m_nSurfaceTouched;
	int8_t m_nZoneLevel;
} CPhysicalData;
//- rouz edit (Claude)
