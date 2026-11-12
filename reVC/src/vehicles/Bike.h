//+ rouz edit (ChatGPT)
#pragma once

#include "Vehicle.h"
#include "Skidmarks.h"
#include "AnimManager.h"

#include "BikeNodes.h"
#include "BikeData.h"

#include "WheelIndices.h"

enum {
	BIKESUSP_F1,
	BIKESUSP_F2,
	BIKESUSP_R1,
	BIKESUSP_R2,
};

class CBike : public CVehicle, public CBikeData
{
public:

	CBike(int32 id, uint8 CreatedBy);

	// from CEntity
	void SetModelIndex(uint32 id);
	void ProcessControl(void);
	void Teleport(CVector v);
	void PreRender(void);
	void Render(void);

	// from CPhysical
	int32 ProcessEntityCollision(CEntity *ent, CColPoint *colpoints);

	// from CVehicle
	void ProcessControlInputs(uint8);
	void GetComponentWorldPosition(int32 component, CVector &pos);
	bool IsComponentPresent(int32 component);
	void SetComponentRotation(int32 component, CVector rotation);
	bool IsDoorReady(eDoors door);
	bool IsDoorFullyOpen(eDoors door);
	bool IsDoorClosed(eDoors door);
	bool IsDoorMissing(eDoors door);
	void RemoveRefsToVehicle(CEntity *ent);
	void BlowUpCar(CEntity *ent);
	bool SetUpWheelColModel(CColModel *colModel);
	void BurstTyre(uint8 tyre, bool applyForces);
	bool IsRoomForPedToLeaveCar(uint32 component, CVector *doorOffset);
	float GetHeightAboveRoad(void);
	void PlayCarHorn(void);

	void KnockOffRider(eWeaponType weapon, uint8 direction, CPed *ped, bool bGetBackOn);
	void VehicleDamage(void);
	void ProcessBuoyancy(void);
	void DoDriveByShootings(void);
	void AddDamagedVehicleParticles(void);
	int32 AddWheelDirtAndWater(CColPoint *colpoint, uint32 belowEffectSpeed);
	void PlayHornIfNecessary(void);
	void ResetSuspension(void);
	void SetupSuspensionLines(void);
	void CalculateLeanMatrix(void);
	void GetCorrectedWorldDoorPosition(CVector &pos, CVector p1, CVector p2);

	void Fix(void);
	void SetupModelNodes(void);
	void ReduceHornCounter(void);

#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif
	static const uint32 nSaveStructSize;
};

// These functions and function names are made up

inline int8 GetBikeDoorFlag(int32 carnode) {
	switch (carnode) {
	case CAR_DOOR_RR:
	case CAR_DOOR_LR:
		return CAR_DOOR_FLAG_RR | CAR_DOOR_FLAG_LR;
	case CAR_DOOR_RF:
	case CAR_DOOR_LF:
		return CAR_DOOR_FLAG_RF | CAR_DOOR_FLAG_LF;
	default:
		return CAR_DOOR_FLAG_UNKNOWN;
	}
}

// for m_nGettingOutFlags
inline int8 GetBikeDoorFlagInclJumpInFromFront(int32 carnode) {
	switch (carnode) {
		case CAR_DOOR_RR:
		case CAR_DOOR_LR:
			return CAR_DOOR_FLAG_RR | CAR_DOOR_FLAG_LR;
		case CAR_DOOR_RF:
		case CAR_DOOR_LF:
		case CAR_WINDSCREEN:
			return CAR_DOOR_FLAG_RF | CAR_DOOR_FLAG_LF;
		default:
			return CAR_DOOR_FLAG_UNKNOWN;
	}
}
//- rouz edit (ChatGPT)
