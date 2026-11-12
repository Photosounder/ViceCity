//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#include "Vector.h"
#include "Vector2D.h"
typedef CVector CraneVec3;
typedef CVector2D CraneVec2;
class CBuilding;
class CObject;
class CVehicle;
#else
typedef struct CraneVec3 {float x,y,z;} CraneVec3;
typedef struct CraneVec2 {float x,y;} CraneVec2;
typedef struct CBuilding CBuilding;
typedef struct CObject CObject;
typedef struct CVehicle CVehicle;
#endif

// State of one crane, as plain data usable from C
typedef struct CCraneData
{
	CBuilding *m_pCraneEntity;
	CObject *m_pHook;
	float m_fPickupX1;
	float m_fPickupX2;
	float m_fPickupY1;
	float m_fPickupY2;
	CraneVec3 m_vecDropoffTarget;
	float m_fDropoffHeading;
	float m_fPickupAngle;
	float m_fDropoffAngle;
	float m_fPickupDistance;
	float m_fDropoffDistance;
	float m_fPickupHeight;
	float m_fDropoffHeight;
	float m_fHookAngle;
	float m_fHookOffset;
	float m_fHookHeight;
	CraneVec3 m_vecHookInitPos;
	CraneVec3 m_vecHookCurPos;
	CraneVec2 m_vecHookVelocity;
	CVehicle *m_pVehiclePickedUp;
	uint32_t m_nTimeForNextCheck;
	uint8_t m_nCraneStatus;
	uint8_t m_nCraneState;
	uint8_t m_nVehiclesCollected;
	bool m_bIsCrusher;
	bool m_bIsMilitaryCrane;
	bool m_bWasMilitaryCrane;
	bool m_bIsTop;
} CCraneData;
//- rouz edit (Claude)
