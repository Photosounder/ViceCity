//+ rouz edit (ChatGPT)
#pragma once

#include "Physical.h"
#include "AutoPilot.h"
#include "ModelIndices.h"
#include "AnimationId.h"
#include "WeaponType.h"
#include "Collision.h"
#include "HandlingMgr.h"

class CPed;
class CPlayerPed;
class CCopPed;
struct CFire;

#include "VehicleCreation.h"

#include "CarNodes.h"


#include "CarLock.h"

#include "VehicleBombTypes.h"

#include "VehicleDoors.h"

#include "VehicleDamageParts.h"


enum
{
	CAR_PIECE_BONNET = 1,
	CAR_PIECE_BOOT,
	CAR_PIECE_BUMP_FRONT,
	CAR_PIECE_BUMP_REAR,
	CAR_PIECE_DOOR_LF,
	CAR_PIECE_DOOR_RF,
	CAR_PIECE_DOOR_LR,
	CAR_PIECE_DOOR_RR,
	CAR_PIECE_WING_LF,
	CAR_PIECE_WING_RF,
	CAR_PIECE_WING_LR,
	CAR_PIECE_WING_RR,
	CAR_PIECE_WHEEL_LF,
	CAR_PIECE_WHEEL_RF,
	CAR_PIECE_WHEEL_LR,
	CAR_PIECE_WHEEL_RR,
	CAR_PIECE_WINDSCREEN,
};

#include "WheelState.h"

enum eFlightModel
{
	FLIGHT_MODEL_DODO,
	FLIGHT_MODEL_RCPLANE,
	FLIGHT_MODEL_RCHELI,
	FLIGHT_MODEL_SEAPLANE,
	FLIGHT_MODEL_PLANE_UNUSED,
	FLIGHT_MODEL_PLANE,
	FLIGHT_MODEL_HELI
};

#include "../modelinfo/VehicleAppearance.h"

// TODO: what is this even?
enum eBikeWheelSpecial
{
	BIKE_WHEELSPEC_0,	// both wheels on ground
	BIKE_WHEELSPEC_1,	// rear wheel on ground
	BIKE_WHEELSPEC_2,	// only front wheel on ground
	BIKE_WHEELSPEC_3,	// can't happen
};

enum
{
	ROTOR_TOP = 3,
	ROTOR_FRONT = 4,
	ROTOR_RIGHT = 5,
	ROTOR_LEFT = 7,
	ROTOR_BACK = 8,
	ROTOR_BOTTOM = 9,
};

#include "VehicleData.h"
#include "VehicleLogic.h"

class CVehicle : public CPhysical, public CVehicleData
{
public:

	CVehicle(void) {
        // Preserve the original autopilot initialization in the fake vehicle constructor
        AutoPilot_Init(&AutoPilot);
    }	// FAKE
	CVehicle(uint8 CreatedBy);
	~CVehicle(void);
	// from CEntity
	void SetModelIndex(uint32 id);
	bool SetupLighting(void);
	void RemoveLighting(bool);
	void FlagToDestroyWhenNextProcessed(void) {}

	virtual void ProcessControlInputs(uint8) {}
	virtual void GetComponentWorldPosition(int32 component, CVector &pos) {}
	virtual bool IsComponentPresent(int32 component) { return false; }
	virtual void SetComponentRotation(int32 component, CVector rotation) {}
	virtual void OpenDoor(int32, eDoors door, float) {}
	virtual void ProcessOpenDoor(uint32, uint32, float) {}
	virtual bool IsDoorReady(eDoors door) { return false; }
	virtual bool IsDoorFullyOpen(eDoors door) { return false; }
	virtual bool IsDoorClosed(eDoors door) { return false; }
	virtual bool IsDoorMissing(eDoors door) { return false; }
	virtual bool IsDoorReady(uint32 door) { return false; }
	virtual bool IsDoorMissing(uint32 door) { return false; }
	virtual bool IsOpenTopCar(void) { return false; }
	virtual void RemoveRefsToVehicle(CEntity *ent) {}
	virtual void BlowUpCar(CEntity *ent) {}
	virtual bool SetUpWheelColModel(CColModel *colModel) { return false; }
	virtual void BurstTyre(uint8 tyre, bool applyForces) {}
	virtual bool IsRoomForPedToLeaveCar(uint32 component, CVector *forcedDoorPos) { return false; }
	virtual bool IsClearToDriveAway(void);
	virtual float GetHeightAboveRoad(void);
	virtual void PlayCarHorn(void) {}
#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif

	eVehicleAppearance GetVehicleAppearance(void);
	bool IsCar(void) { return m_vehType == VEHICLE_TYPE_CAR; }
	bool IsBoat(void) { return m_vehType == VEHICLE_TYPE_BOAT; }
	bool IsTrain(void) { return m_vehType == VEHICLE_TYPE_TRAIN; }
	bool IsHeli(void) { return m_vehType == VEHICLE_TYPE_HELI; }
	bool IsPlane(void) { return m_vehType == VEHICLE_TYPE_PLANE; }
	bool IsBike(void) { return m_vehType == VEHICLE_TYPE_BIKE; }

	void FlyingControl(eFlightModel flightModel);
	bool DoBladeCollision(CVector pos, CMatrix &matrix, int16 rotorType, float radius, float damageMult);
	bool BladeColSectorList(CPtrList &list, CColModel &rotorColModel, CMatrix &matrix, int16 rotorType, float damageMult);

	void ProcessWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint,
		int32 wheelsOnGround, float thrust, float brake, float adhesion, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, uint16 wheelStatus);
	void ProcessBikeWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint,
		int32 wheelsOnGround, float thrust, float brake, float adhesion, float destabTraction, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, eBikeWheelSpecial special, uint16 wheelStatus);
	void ExtinguishCarFire(void);
	void ProcessDelayedExplosion(void);
	float ProcessWheelRotation(tWheelState state, const CVector &fwd, const CVector &speed, float radius);
	int FindTyreNearestPoint(float x, float y);
	bool IsLawEnforcementVehicle(void);
	void ChangeLawEnforcerState(uint8 enable);
	bool UsesSiren(void);
	bool IsVehicleNormal(void);
	bool CarHasRoof(void);
	bool IsUpsideDown(void);
	bool IsOnItsSide(void);
	bool CanBeDeleted(void);
	bool CanPedOpenLocks(CPed *ped);
	bool CanDoorsBeDamaged(void);
	bool CanPedEnterCar(void);
	bool CanPedExitCar(bool jumpExit);
	bool CanPedJumpOutCar(void);
	bool CanPedJumpOffBike(void);
	// do these two actually return something?
	CPed *SetUpDriver(void);
	CPed *SetupPassenger(int n);
	void SetDriver(CPed *driver);
	bool AddPassenger(CPed *passenger);
	bool AddPassenger(CPed *passenger, uint8 n);
	void RemovePassenger(CPed *passenger);
	void RemoveDriver(void);
	bool IsDriver(CPed *ped);
	bool IsDriver(int32 model);
	bool IsPassenger(CPed *ped);
	bool IsPassenger(int32 model);
	void UpdatePassengerList(void);
	void ProcessCarAlarm(void);
	bool IsSphereTouchingVehicle(float sx, float sy, float sz, float radius);
	bool ShufflePassengersToMakeSpace(void);
	void MakeNonDraggedPedsLeaveVehicle(CPed *ped1, CPed *ped2, CPlayerPed *&player, CCopPed *&cop);
	void InflictDamage(CEntity *damagedBy, eWeaponType weaponType, float damage, CVector pos = CVector(0.0f, 0.0f, 0.0f));
	void DoFixedMachineGuns(void);
	void FireFixedMachineGuns(void);
	void ActivateBomb(void);
	void ActivateBombWhenEntered(void);
	void KillPedsInVehicle(void);

	void SetComponentAtomicAlpha(RpAtomic *atomic, int32 alpha);
	void UpdateClumpAlpha(void);

	static void HeliDustGenerate(CEntity *heli, float radius, float ground, int rnd);
	void DoSunGlare(void);

	bool IsAlarmOn(void) { return m_nAlarmState != 0 && m_nAlarmState != -1 && GetStatus() != STATUS_WRECKED; }
	CVehicleModelInfo* GetModelInfo() { return (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); } // rouz edit (ChatGPT)
	bool IsTaxi(void) { return GetModelIndex() == MI_TAXI || GetModelIndex() == MI_CABBIE || GetModelIndex() == MI_ZEBRA || GetModelIndex() == MI_KAUFMAN; }
	bool IsLimo(void) { return GetModelIndex() == MI_STRETCH || GetModelIndex() == MI_LOVEFIST; }
	bool IsRealHeli(void) { return !!(pHandling->Flags & HANDLING_IS_HELI); }
	bool IsRealPlane(void) { return !!(pHandling->Flags & HANDLING_IS_PLANE); }

	static bool bWheelsOnlyCheat;
	static bool bAllDodosCheat;
	static bool bCheat3;
	static bool bCheat4;
	static bool bCheat5;
	static bool bCheat8;
	static bool bCheat9;
	static bool bCheat10;
	static bool bHoverCheat;
	static bool bAllTaxisHaveNitro;
	static bool m_bDisableMouseSteering;
	static bool bDisableRemoteDetonation;
	static bool bDisableRemoteDetonationOnContact;
#ifndef MASTER
	static bool m_bDisplayHandlingInfo;
#endif
};

void DestroyVehicleAndDriverAndPassengers(CVehicle* pVehicle);
bool IsVehiclePointerValid(CVehicle* pVehicle);

// Names of functions below are made up by us.

// Used in III and VC.
inline int8 GetCarDoorFlag(int32 carnode) {
	switch (carnode) {
	case CAR_DOOR_LF:
		return CAR_DOOR_FLAG_LF;
	case CAR_DOOR_LR:
		return CAR_DOOR_FLAG_LR;
	case CAR_DOOR_RF:
		return CAR_DOOR_FLAG_RF;
	case CAR_DOOR_RR:
		return CAR_DOOR_FLAG_RR;
	default:
		return CAR_DOOR_FLAG_UNKNOWN;
	}
}

// VC. Accounts the case numMaxPassengers == 0, only for m_nGettingInFlags.
inline int8 GetEnterCarDoorFlag(int32 carnode, uint8 numMaxPassengers) {
	switch (carnode) {
	case CAR_DOOR_RF:
		return CAR_DOOR_FLAG_RF;
	case CAR_DOOR_RR:
		return CAR_DOOR_FLAG_RR;
	case CAR_DOOR_LF:
		if (numMaxPassengers != 0)
			return CAR_DOOR_FLAG_LF;
		else
			return CAR_DOOR_FLAG_LF | CAR_DOOR_FLAG_LR;
	case CAR_DOOR_LR:
		if (numMaxPassengers != 0)
			return CAR_DOOR_FLAG_LR;
		else
			return CAR_DOOR_FLAG_LF | CAR_DOOR_FLAG_LR;
	default:
		return CAR_DOOR_FLAG_UNKNOWN;
	}
}

//- rouz edit (ChatGPT)
