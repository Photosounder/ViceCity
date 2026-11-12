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

class CCranes
{
public:
	static void InitCranes(void);
	static void AddThisOneCrane(CEntity* pCraneEntity);
	static void ActivateCrane(float fInfX, float fSupX, float fInfY, float fSupY, float fDropOffX, float fDropOffY, float fDropOffZ, float fHeading, bool bIsCrusher, bool bIsMilitary, float fPosX, float fPosY);
	static void DeActivateCrane(float fX, float fY);
	static bool IsThisCarPickedUp(float fX, float fY, CVehicle* pVehicle);
	static void UpdateCranes(void);
	static bool DoesMilitaryCraneHaveThisOneAlready(uint32 mi);
	static void RegisterCarForMilitaryCrane(uint32 mi);
	static bool HaveAllCarsBeenCollectedByMilitaryCrane(void);
	static bool IsThisCarBeingCarriedByAnyCrane(CVehicle* pVehicle);
	static bool IsThisCarBeingTargettedByAnyCrane(CVehicle* pVehicle);
	static void Save(uint8* buf, uint32* size);
	static void Load(uint8* buf, uint32 size); // out of class in III PC and later because of SecuROM

	static uint32 CarsCollectedMilitaryCrane;
	static int32 NumCranes;
	static CCrane aCranes[NUM_CRANES];
};
