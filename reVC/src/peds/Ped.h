//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#pragma once

#include "RwHelper.h"
#include "AnimManager.h"
#include "Crime.h"
#include "EventList.h"
#include "PedIK.h"
#include "PedType.h"
#include "Physical.h"
#include "Weapon.h"
#include "WeaponInfo.h"
#include "PathFind.h"
#include "Collision.h"

#include "PedConstants.h" // rouz edit (ChatGPT)
#define CHECK_NEARBY_THINGS_MAX_DIST	15.0f
#define ENTER_CAR_MAX_DIST	30.0f
#define CAN_SEE_ENTITY_ANGLE_THRESHOLD	DEGTORAD(60.0f)

struct CAccident;
class CObject;
struct CFire;
struct AnimBlendFrameData;
struct CAnimBlendAssociation;
class CPedAttractor;

#include "PedAudioData.h"

enum
{
	ATTACK_IN_PROGRESS,
	CANT_ATTACK,
	WATCH_UNTIL_HE_DISAPPEARS,
};


enum FightState {
	FIGHTSTATE_MOVE_FINISHED = -2,
	FIGHTSTATE_JUST_ATTACKED,
	FIGHTSTATE_NO_MOVE,
	FIGHTSTATE_1
};

enum
{
	ENDFIGHT_NORMAL,
	ENDFIGHT_WITH_A_STEP,
	ENDFIGHT_FAST
};


enum FightMoveHitLevel
{
	HITLEVEL_NULL,
	HITLEVEL_GROUND,
	HITLEVEL_LOW,
	HITLEVEL_MEDIUM,
	HITLEVEL_HIGH
};

struct FightMove
{
	AnimationId animId;
	float startFireTime;
	float endFireTime;
	float comboFollowOnTime;
	float strikeRadius;
	float extendReachMultiplier;
	uint8 hitLevel; // FightMoveHitLevel
	uint8 damage;
	uint8 flags;
};

// TODO: This is eFightState on mobile.
#include "PedStates.h"

enum ePedPieceTypes
{
	PEDPIECE_TORSO,
	PEDPIECE_MID,
	PEDPIECE_LEFTARM,
	PEDPIECE_RIGHTARM,
	PEDPIECE_LEFTLEG,
	PEDPIECE_RIGHTLEG,
	PEDPIECE_HEAD,
};

#include "PedWaitStates.h"

#include "PedObjectives.h"

enum PedLineUpPhase {
	LINE_UP_TO_CAR_START,
	LINE_UP_TO_CAR_END,
	LINE_UP_TO_CAR_2, // Buggy. Used for cops arresting you from passenger door
	LINE_UP_TO_CAR_FALL
};

enum PedOnGroundState {
	NO_PED,
	PED_IN_FRONT_OF_ATTACKER,
	PED_ON_THE_FLOOR,
	PED_DEAD_ON_THE_FLOOR
};

#include "PointBlank.h"
#include "PedFormation.h"





extern float gfTommyFatness;

class CVehicle;

#include "PedData.h"
#include "PedLogic.h"

class CPed : public CPhysical, public CPedData
{
public:

	CPed(uint32 pedType);
	~CPed(void);

	void DeleteRwObject();
	void SetModelIndex(uint32 mi);
	void ProcessControl(void);
	void Teleport(CVector);
	void PreRender(void);
	void Render(void);
	bool SetupLighting(void);
	void RemoveLighting(bool);
	void FlagToDestroyWhenNextProcessed(void);
	int32 ProcessEntityCollision(CEntity*, CColPoint*);

	virtual void SetMoveAnim(void);

	void AddWeaponModel(int id);
	void AimGun(void);
	void KillPedWithCar(CVehicle *veh, float impulse);
	void Say(uint16 audio);
	void Say(uint16 audio, int32 time);
	void SetLookFlag(CEntity* target, bool keepTryingToLook, bool cancelPrevious = false);
	void SetLookFlag(float direction, bool keepTryingToLook, bool cancelPrevious = false);
	void SetLookTimer(int time);
	void SetDie(AnimationId anim = ANIM_STD_KO_FRONT, float arg1 = 4.0f, float arg2 = 0.0f);
	void SetDead(void);
	void ApplyHeadShot(eWeaponType weaponType, CVector pos, bool evenOnPlayer);
	void RemoveBodyPart(PedNode nodeId, int8 direction);
	bool OurPedCanSeeThisOne(CEntity *target, bool shootablesDoBlock = false);
	void Avoid(void);
	void Attack(void);
	void ClearAimFlag(void);
	void ClearLookFlag(void);
	void RestorePreviousState(void);
	void ClearAttack(void);
	bool IsPedHeadAbovePos(float zOffset);
	void RemoveWeaponModel(int modelId);
	void SetCurrentWeapon(eWeaponType weaponType);
	void SetCurrentWeapon(int weapon);
	void Duck(void);
	void ClearDuck(bool = false);
	void ClearPointGunAt(void);
	void BeingDraggedFromCar(void);
	void RestartNonPartialAnims(void);
	void LineUpPedWithCar(PedLineUpPhase phase);
	void SetPedPositionInCar(void);
	void PlayFootSteps(void);
	void QuitEnteringCar(void);
	void BuildPedLists(void);
	int32 GiveWeapon(eWeaponType weaponType, uint32 ammo, bool unused = true);
	void CalculateNewOrientation(void);
	float WorkOutHeadingForMovingFirstPerson(float);
	void CalculateNewVelocity(void);
	bool CanSeeEntity(CEntity*, float threshold = CAN_SEE_ENTITY_ANGLE_THRESHOLD);
	void RestorePreviousObjective(void);
	void SetIdle(void);
#ifdef _MSC_VER
#if _MSC_VER >= 1920 && _MSC_VER < 1929
	__declspec(noinline) // workaround for a compiler bug, hooray MS :P
#endif
#endif
	void SetObjective(eObjective, void*);
	void SetObjective(eObjective);
	void SetObjective(eObjective, int16, int16);
	void SetObjective(eObjective, CVector);
	void SetObjective(eObjective, float, const CVector&);
	void ClearChat(void);
	void InformMyGangOfAttack(CEntity*);
	void ReactToAttack(CEntity*);
	void SetDuck(uint32, bool = false);
	void RegisterThreatWithGangPeds(CEntity*);
	bool TurnBody(void);
	void Chat(void);
	void CheckAroundForPossibleCollisions(void);
	void SetSeek(CVector, float);
	void SetSeek(CEntity*, float);
	bool MakePhonecall(void);
	bool FacePhone(void);
	CPed *CheckForDeadPeds(void);
	bool CheckForExplosions(CVector2D &area);
	CPed *CheckForGunShots(void);
	uint8 CheckForPointBlankPeds(CPed*);
	bool CheckIfInTheAir(void);
	void ClearAll(void);
	void SetPointGunAt(CEntity*);
	bool Seek(void);
	bool SetWanderPath(int8);
	bool SetFollowPath(CVector dest, float radius, eMoveState state, CEntity*, CEntity*, int);
	bool SetFollowPathStatic(void);
	bool SetFollowPathDynamic(void);
	void ClearAttackByRemovingAnim(void);
	void SetStoredState(void);
	void StopNonPartialAnims(void);
	bool InflictDamage(CEntity*, eWeaponType, float, ePedPieceTypes, uint8);
	void ClearFlee(void);
	void ClearFall(void);
	void SetGetUp(void);
	void ClearInvestigateEvent(void);
	void ClearLeader(void);
	void ClearLook(void);
	void ClearObjective(void);
	void ClearPause(void);
	void ClearSeek(void);
	void ClearWeapons(void);
	void RestoreGunPosition(void);
	void RestoreHeadingRate(void);
	void SetAimFlag(CEntity* to);
	void SetAimFlag(float angle);
	void SetAmmo(eWeaponType weaponType, uint32 ammo);
	void SetEvasiveStep(CPhysical*, uint8);
	void GrantAmmo(eWeaponType, uint32);
	void SetEvasiveDive(CPhysical*, uint8);
	void SetAttack(CEntity*);
	void StartFightAttack(uint8);
	void SetWaitState(eWaitState, void*);
	bool FightStrike(CVector&, bool);
	void FightHitPed(CPed*, CVector&, CVector&, int16);
	int32 ChooseAttackPlayer(uint8, bool);
	int32 ChooseAttackAI(uint8, bool);
	int GetLocalDirection(const CVector2D &);
	void StartFightDefend(uint8, uint8, uint8);
	void PlayHitSound(CPed*);
	void SetFall(int, AnimationId, uint8);
	void SetFlee(CEntity*, int);
	void SetFlee(CVector2D const &, int);
	void RemoveDrivebyAnims(void);
	void RemoveInCarAnims(void);
	void CollideWithPed(CPed*);
	void SetDirectionToWalkAroundObject(CEntity*);
	bool SetDirectionToWalkAroundVehicle(CVehicle*);
	void RemoveWeaponAnims(int, float);
	void CreateDeadPedMoney(void);
	void CreateDeadPedWeaponPickups(void);
	void CreateDeadPedPickupCoors(float *x, float *y, float *z);
	void SetAttackTimer(uint32);
	void SetBeingDraggedFromCar(CVehicle*, uint32, bool);
	void SetRadioStation(void);
	void SetBuyIceCream(void);
	void SetChat(CEntity*, uint32);
	void DeadPedMakesTyresBloody(void);
	void MakeTyresMuddySectorList(CPtrList&);
	bool DuckAndCover(void);
	void EndFight(uint8);
	void EnterCar(void);
	uint8 GetNearestTrainPedPosition(CVehicle*, CVector&);
	uint8 GetNearestTrainDoor(CVehicle*, CVector&);
	void ExitCar(void);
	void Fight(void);
	bool FindBestCoordsFromNodes(CVector, CVector*);
	void Wait(void);
	void ProcessObjective(void);
	CVector *SeekFollowingPath(void);
	void Flee(void);
	void FollowPath(void);
	CVector GetFormationPosition(void);
	void GetNearestDoor(CVehicle*, CVector&);
	bool GetNearestPassengerDoor(CVehicle*, CVector&);
	int GetNextPointOnRoute(void);
	int GetWeaponSlot(eWeaponType);
	bool CanWeRunAndFireWithWeapon(void);
	void GoToNearestDoor(CVehicle*);
	bool HaveReachedNextPointOnRoute(float);
	void Idle(void);
	void InTheAir(void);
	void SetLanding(void);
	void InvestigateEvent(void);
	bool IsPedDoingDriveByShooting(void);
	bool IsRoomToBeCarJacked(void);
	void SetInvestigateEvent(eEventType, CVector2D, float, uint16, float);
	bool LookForInterestingNodes(void);
	void LookForSexyCars(void);
	void LookForSexyPeds(void);
	void Mug(void);
	void MoveHeadToLook(void);
	void Pause(void);
	void ProcessBuoyancy(void);
	void ServiceTalking(void);
	void SetJump(void);
	void WanderPath(void);
	void ReactToPointGun(CEntity*);
	void SeekCar(void);
	bool PositionPedOutOfCollision(void);
	bool PositionAnyPedOutOfCollision(void);
	bool RunToReportCrime(eCrimeType);
	bool PlacePedOnDryLand(void);
	bool PossiblyFindBetterPosToSeekCar(CVector*, CVehicle*);
	void UpdateFromLeader(void);
	uint32 ScanForThreats(void);
	void SetEnterCar(CVehicle*, uint32);
	bool WarpPedToNearEntityOffScreen(CEntity*);
	void SetExitCar(CVehicle*, uint32);
	void SetFormation(eFormation);
	bool WillChat(CPed*);
	void SetEnterCar_AllClear(CVehicle*, uint32, uint32);
	void SetSolicit(uint32 time);
	void ScanForInterestingStuff(void);
	void WarpPedIntoCar(CVehicle*);
	void SetCarJack(CVehicle*);
	bool WarpPedToNearLeaderOffScreen(void);
	void Solicit(void);
	void SetExitBoat(CVehicle*);
	void ClearFollowPath();
	void GiveDelayedWeapon(eWeaponType weapon, uint32 ammo);
	void RequestDelayedWeapon();
	void AddInCarAnims(CVehicle* car, bool isDriver);
	bool CanBeDamagedByThisGangMember(CPed*);
	void AnswerMobile(void);
	void BuyIceCream(void);
	void CheckThreatValidity(void);
	void ClearAnswerMobile(void);
	void SetAnswerMobile(void);
	void AttachPedToEntity(CEntity*, CVector, uint16, float, eWeaponType);
	void DettachPedFromEntity();
	void PedShuffle();
	void DriveVehicle();
	void PositionAttachedPed();
	bool CanUseTorsoWhenLooking();
	void ScanForDelayedResponseThreats();

	// Static methods
	static CVector GetLocalPositionToOpenCarDoor(CVehicle *veh, uint32 component, float offset);
	static CVector GetPositionToOpenCarDoor(CVehicle *veh, uint32 component, float seatPosMult);
	static CVector GetPositionToOpenCarDoor(CVehicle* veh, uint32 component);
	static void Initialise(void);
	static void SetAnimOffsetForEnterOrExitVehicle(void);
	static void LoadFightData(void);

	// Callbacks
	static void PedGetupCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedStaggerCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedEvadeCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishDieAnimCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishedWaitCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishLaunchCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishHitHeadCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimGetInCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimDoorOpenCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimPullPedOutCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimDoorCloseCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetInCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetOutCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimAlignCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetDraggedOutCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimStepOutCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetInTrainCB(CAnimBlendAssociation *assoc, void *arg);
#ifdef GTA_TRAIN
	static void PedSetOutTrainCB(CAnimBlendAssociation *assoc, void *arg);
#endif
	static void FinishedAttackCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishedReloadCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishFightMoveCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimDoorCloseRollingCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishJumpCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedLandCB(CAnimBlendAssociation *assoc, void *arg);
	static void RestoreHeadingRateCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetQuickDraggedOutCarPositionCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetDraggedOutCarPositionCB(CAnimBlendAssociation *assoc, void *arg);
	static void DeleteSunbatheIdleAnimCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetPreviousStateCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimShuffleCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetGetInCarPositionCB(CAnimBlendAssociation* assoc, void* arg);

	bool IsPlayer(void) const;
	bool IsFemale(void) { return m_nPedType == PEDTYPE_CIVFEMALE || m_nPedType == PEDTYPE_PROSTITUTE; }
	bool UseGroundColModel(void);
	bool CanSetPedState(void);
	bool IsPedInControl(void);
	bool CanPedDriveOff(void);
	bool CanBeDeleted(void);
	bool CanBeDeletedEvenInVehicle(void);
	bool CanStrafeOrMouseControl(void);
	bool CanPedReturnToState(void);
	void SetMoveState(eMoveState);
	bool IsTemporaryObjective(eObjective objective);
	void SetObjectiveTimer(int);
	bool SelectGunIfArmed(void);
	bool IsPointerValid(void);
	void SortPeds(CPed**, int, int);
	void ForceStoredObjective(eObjective);
	void SetStoredObjective(void);
	void SetLeader(CEntity* leader);
	void SetPedStats(ePedStats);
	bool IsGangMember(void) const;
	void Die(void);
#ifdef GTA_TRAIN
	void EnterTrain(void);
	void ExitTrain(void);
	void SetExitTrain(CVehicle*);
	void SetPedPositionInTrain(void);
	void LineUpPedWithTrain(void);
	void SetEnterTrain(CVehicle*, uint32);
#endif
	void Fall(void);
	bool IsPedShootable(void);
	void Look(void);
	void SetInTheAir(void);
	void RestoreHeadPosition(void);
	void PointGunAt(void);
	bool ServiceTalkingWhenDead(void);
	void SetShootTimer(uint32);
	void SetSeekCar(CVehicle*, uint32);
	void SetSeekBoatPosition(CVehicle*);
	void WanderRange(void);
	void SetFollowRoute(int16, int16);
	void SeekBoatPosition(void);
	void UpdatePosition(void);
	CObject *SpawnFlyingComponent(int, int8);
	void SetCarJack_AllClear(CVehicle*, uint32, uint32);
	bool CanPedJumpThis(CEntity *unused, CVector *damageNormal = nil);
	void SetNewAttraction(CPedAttractor* pAttractor, const CVector& pos, float, float, int);
	void ClearWaitState(void);
	void Undress(const char*);
	void Dress(void);
	int32 KillCharOnFootMelee(CVector&, CVector&, CVector&);
	int32 KillCharOnFootArmed(CVector&, CVector&, CVector&);
	void SetLook(CEntity* to);
	void SetLook(float direction);

	bool HasWeaponSlot(uint8 slot) { return m_weapons[slot].m_eWeaponType != WEAPONTYPE_UNARMED; }
	CWeapon& GetWeapon(uint8 slot) { return m_weapons[slot]; }
	CWeapon *GetWeapon(void) { return &m_weapons[m_currentWeapon]; }

	PedState GetPedState(void) { return m_nPedState; }
	void SetPedState(PedState state) 
	{
		if (GetPedState() == PED_FOLLOW_PATH && state != PED_FOLLOW_PATH)
			ClearFollowPath();
		m_nPedState = state;
	}
	bool Dead(void) { return m_nPedState == PED_DEAD; }
	bool Dying(void) { return m_nPedState == PED_DIE; }
	bool DyingOrDead(void) { return PedLogic_DyingOrDead(this); }
	bool OnGround(void) { return m_nPedState == PED_FALL || m_nPedState == PED_DIE || m_nPedState == PED_DEAD; }
	bool OnGroundOrGettingUp(void) { return OnGround() || m_nPedState == PED_GETUP; }
	
	bool Driving(void) { return m_nPedState == PED_DRIVING; }
	bool InVehicle(void) { return bInVehicle && m_pMyVehicle; } // True when ped is sitting/standing in vehicle, not in enter/exit state.
	bool EnteringCar(void) { return PedLogic_EnteringCar(this); }
	bool HasAttractor(void);
	bool IsUseAttractorObjective(eObjective obj) {
		return obj == OBJECTIVE_GOTO_ATM_ON_FOOT || obj == OBJECTIVE_GOTO_ICE_CREAM_VAN_ON_FOOT ||
			obj == OBJECTIVE_GOTO_PIZZA_ON_FOOT || obj == OBJECTIVE_GOTO_SEAT_ON_FOOT ||
			obj == OBJECTIVE_GOTO_SHELTER_ON_FOOT || obj == OBJECTIVE_GOTO_BUS_STOP_ON_FOOT;
	}

	void ReplaceWeaponWhenExitingVehicle(void);
	void RemoveWeaponWhenEnteringVehicle(void);
	bool IsNotInWreckedVehicle()
	{
		return m_pMyVehicle != nil && ((CEntity*)m_pMyVehicle)->GetStatus() != STATUS_WRECKED;
	}

	// My names. Inlined in VC
	AnimationId GetFireAnimNotDucking(CWeaponInfo* weapon) {
		if (m_nPedType == PEDTYPE_COP && WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_COP3_RD))
			return Get3rdFireAnim(weapon);
		else
			return GetPrimaryFireAnim(weapon);
	}

	static AnimationId Get3rdFireAnim(CWeaponInfo* weapon) {
		if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_COP3_RD))
			return ANIM_WEAPON_FIRE_3RD;
		else
			return (AnimationId)0;
	}

	static AnimationId GetFireAnimGround(CWeaponInfo* weapon, bool kickFloorIfNone = true) {
		if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_GROUND_2ND))
			return ANIM_WEAPON_CROUCHFIRE;
		else if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_GROUND_3RD))
			return ANIM_WEAPON_FIRE_3RD;
		else if (kickFloorIfNone)
			return ANIM_STD_KICKGROUND;
		else
			return (AnimationId)0;
	}

	static AnimationId GetPrimaryFireAnim(CWeaponInfo* weapon) {
		if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_ANIMDETONATE))
			return ANIM_STD_DETONATE;
		else
			return ANIM_WEAPON_FIRE;
	}

	static AnimationId GetCrouchReloadAnim(CWeaponInfo* weapon) {
		if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_RELOAD))
			return ANIM_WEAPON_CROUCHRELOAD;
		else
			return (AnimationId)0;
	}

	static AnimationId GetCrouchFireAnim(CWeaponInfo* weapon) {
		if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_CROUCHFIRE))
			return ANIM_WEAPON_CROUCHFIRE;
		else
			return (AnimationId)0;
	}

	static AnimationId GetReloadAnim(CWeaponInfo* weapon) {
		if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_RELOAD))
			return ANIM_WEAPON_RELOAD;
		else
			return (AnimationId)0;
	}

	static AnimationId GetFightIdleWithMeleeAnim(CWeaponInfo* weapon) {
		if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_FIGHTMODE))
			return ANIM_MELEE_IDLE_FIGHTMODE;
		else
			return (AnimationId)0;
	}

	static AnimationId GetFinishingAttackAnim(CWeaponInfo* weapon) {
		if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_FINISH_3RD))
			return ANIM_MELEE_ATTACK_FINISH;
		else
			return (AnimationId)0;
	}

	static AnimationId GetSecondFireAnim(CWeaponInfo* weapon) {
		if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_USE_2ND))
			return ANIM_WEAPON_FIRE_2ND;
		else
			return (AnimationId)0;
	}
	
	static AnimationId GetMeleeStartAnim(CWeaponInfo* weapon) {
		if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_PARTIALATTACK))
			return ANIM_MELEE_ATTACK_START;
		else
			return (AnimationId)0;
	}

	static AnimationId GetThrowAnim(CWeaponInfo *weapon)
	{
		if (WeaponInfo_IsFlagSet(weapon,WEAPONFLAG_THROW))
			return ANIM_THROWABLE_START_THROW;
		else
			return (AnimationId)0;
	}
	// --

	// My additions, because there were many, many instances of that.
	inline void SetFindPathAndFlee(CEntity *fleeFrom, int time, bool walk = false)
	{
		SetFlee(fleeFrom, time);
		bUsePedNodeSeek = true;
		m_pNextPathNode = nil;
		if (walk)
			SetMoveState(PEDMOVE_WALK);
	}

	inline void SetFindPathAndFlee(CVector2D const &from, int time, bool walk = false)
	{
		SetFlee(from, time);
		bUsePedNodeSeek = true;
		m_pNextPathNode = nil;
		if (walk)
			SetMoveState(PEDMOVE_WALK);
	}
	// --

	inline void SetWeaponLockOnTarget(CEntity *target)
	{
		if (m_pPointGunAt)
			m_pPointGunAt->CleanUpOldReference(&m_pPointGunAt);

		m_pPointGunAt = (CPed*)target;
		if (target)
			((CEntity*)target)->RegisterReference(&m_pPointGunAt);
	}

	// Using this to abstract nodes of skinned and non-skinned meshes
	CVector GetNodePosition(int32 node)
	{
		RwV3d pos = { 0.0f, 0.0f, 0.0f };
		// Use the C helper and retain the original typed SDK result
		RpHAnimHierarchy *hier = ((RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(GetClump()));
		int32 idx = RpHAnimIDGetIndex(hier, m_pFrames[node]->nodeID);
		RwMatrix *mats = RpHAnimHierarchyGetMatrixArray(hier);
		pos = mats[idx].pos;
		return pos;
	}
	void TransformToNode(CVector &pos, int32 node)
	{
		// Use the C helper and retain the original typed SDK result
		RpHAnimHierarchy *hier = ((RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(GetClump()));
		int32 idx = RpHAnimIDGetIndex(hier, m_pFrames[node]->nodeID);
		RwMatrix *mats = RpHAnimHierarchyGetMatrixArray(hier);
		RwV3dTransformPoints(&pos, &pos, 1, &mats[idx]);
	}

	// set by 0482:set_threat_reaction_range_multiplier opcode
	static uint16 nThreatReactionRangeMultiplier;

	// set by 0481:set_enter_car_range_multiplier opcode
	static uint16 nEnterCarRangeMultiplier;

	static bool bNastyLimbsCheat;
	static bool bFannyMagnetCheat;
	static bool bPedCheat3;
	static CVector2D ms_vec2DFleePosition;

#ifndef MASTER
	// Mobile things
	void DebugDrawPedDestination(CPed *, int, int);
	void DebugDrawPedDesiredHeading(CPed *, int, int);
	void DebugDrawCollisionRadius(float, float, float, float, int);
	void DebugDrawVisionRange(CVector, float);
	void DebugDrawVisionSimple(CVector, float);
	void DebugDrawLook();
	void DebugDrawPedPsyche();
	void DebugDrawDebugLines();

	static void SwitchDebugDisplay(void);
	static int GetDebugDisplay(void);

	void DebugDrawLookAtPoints();
	void DebugRenderOnePedText(void);
	void DebugRenderClosePedText();
#endif

#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif
};

void FinishTalkingOnMobileCB(CAnimBlendAssociation* assoc, void* arg);
void StartTalkingOnMobileCB(CAnimBlendAssociation* assoc, void* arg);
void PlayRandomAnimationsFromAnimBlock(CPed* ped, AssocGroupId animGroup, uint32 first, uint32 amount);

VALIDATE_SIZE(CPed, 0x5F4);

bool IsPedPointerValid(CPed*);
bool IsPedPointerValid_NotInWorld(CPed*);

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
