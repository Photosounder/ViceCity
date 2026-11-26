#pragma once
#include "common.h"

#include "World.h"
#include "CraneData.h"

class CVehicle;
class CEntity;
class CObject;
class CBuilding;

class CCrane : public CCraneData
{
public:
	enum CraneState {
		IDLE = 0,
		GOING_TOWARDS_TARGET = 1,
		LIFTING_TARGET = 2,
		GOING_TOWARDS_TARGET_ONLY_HEIGHT = 3,
		ROTATING_TARGET = 4,
		DROPPING_TARGET = 5
	};
	enum CraneStatus {
		NONE = 0,
		ACTIVATED = 1,
		DEACTIVATED = 2
	};

	void Init(void) { memset(this, 0, sizeof(*this)); }
	void Update(void);
	bool RotateCarriedCarProperly(void);
	void FindCarInSectorList(CPtrList* pList);
	bool DoesCranePickUpThisCarType(uint32 mi);
	bool GoTowardsTarget(float fAngleToTarget, float fDistanceToTarget, float fTargetHeight, float fSpeedMultiplier = 1.0f);
	bool GoTowardsHeightTarget(float fTargetHeight, float fSpeedMultiplier = 1.0f);
	void FindParametersForTarget(float X, float Y, float Z, float* pAngle, float* pDistance, float* pHeight);
	void CalcHookCoordinates(float* pX, float* pY, float* pZ);
	void SetHookMatrix(void);

	float GetHeightToPickup() { return 4.0f + m_fPickupHeight + (m_bIsCrusher ? 4.5f : 0.0f); };
	float GetHeightToDropoff() { return m_bIsCrusher ? (2.0f + m_fDropoffHeight + 3.0f) : (2.0f + m_fDropoffHeight); }
	float GetHeightToPickupHeight() { return m_fPickupHeight + (m_bIsCrusher ? 7.0f : 4.0f); }
	float GetHeightToDropoffHeight() { return m_fDropoffHeight + (m_bIsCrusher ? 7.0f : 2.0f); }
};

VALIDATE_SIZE(CCrane, 128);

//+ rouz edit (Claude)
// Formerly the static class CCranes, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
void Cranes_InitCranes(void);
void Cranes_AddThisOneCrane(CEntity* pCraneEntity);
void Cranes_ActivateCrane(float fInfX, float fSupX, float fInfY, float fSupY, float fDropOffX, float fDropOffY, float fDropOffZ, float fHeading, bool bIsCrusher, bool bIsMilitary, float fPosX, float fPosY);
void Cranes_DeActivateCrane(float fX, float fY);
bool Cranes_IsThisCarPickedUp(float fX, float fY, CVehicle* pVehicle);
void Cranes_UpdateCranes(void);
bool Cranes_DoesMilitaryCraneHaveThisOneAlready(uint32 mi);
void Cranes_RegisterCarForMilitaryCrane(uint32 mi);
bool Cranes_HaveAllCarsBeenCollectedByMilitaryCrane(void);
bool Cranes_IsThisCarBeingCarriedByAnyCrane(CVehicle* pVehicle);
bool Cranes_IsThisCarBeingTargettedByAnyCrane(CVehicle* pVehicle);
void Cranes_Save(uint8* buf, uint32* size);
void Cranes_Load(uint8* buf, uint32 size); // out of class in III PC and later because of SecuROM
extern uint32 Cranes_CarsCollectedMilitaryCrane;
extern int32 Cranes_NumCranes;
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
extern CCrane Cranes_aCranes[NUM_CRANES];
#endif
//- rouz edit (Claude)
