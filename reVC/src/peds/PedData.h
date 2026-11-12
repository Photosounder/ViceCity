//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "PedNode.h"
#include "WeaponType.h"
#include "StoredCollPoly.h"
#include "WeaponData.h"
#include "PedIKData.h"
#include "PathNodeData.h"

// Enums keep their engine type in C++ and int in C, engine classes keep their real type in C++ and plain data in C
#ifdef __cplusplus
#define PEDDATA_ENUM(type) type
#define PEDDATA_PATHNODE CPathNode
#define PEDDATA_WEAPON CWeapon
#define PEDDATA_PEDIK CPedIK
typedef CVector PedVec;
typedef CVector2D PedVec2;
struct Range2D;
#else
#define PEDDATA_ENUM(type) int
#define PEDDATA_PATHNODE CPathNodeData
#define PEDDATA_WEAPON CWeaponData
#define PEDDATA_PEDIK CPedIKData
typedef struct PedVec {float x,y,z;} PedVec;
typedef struct PedVec2 {float x,y;} PedVec2;
typedef struct CEntity CEntity;
typedef struct CPhysical CPhysical;
typedef struct CVehicle CVehicle;
typedef struct CPathNode CPathNode;
typedef struct CAnimBlendAssociation CAnimBlendAssociation;
typedef struct AnimBlendFrameData AnimBlendFrameData;
typedef struct RpAtomic RpAtomic;
typedef struct CPedAttractor CPedAttractor;
typedef struct PedStats PedStats;
typedef struct CAccident CAccident;
typedef struct CFire CFire;
typedef struct Range2D Range2D;
#endif

// The ped state that follows CPhysical, as plain data usable from C
typedef struct CPedData {
#ifdef USE_CUTSCENE_SHADOW_FOR_PED
	struct CutsceneShadow *m_pRTShadow;
#endif
	// 0x128
	CStoredCollPoly m_collPoly;
	float m_fCollisionSpeed;

	// cf. https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/CPed.h from R*
	uint32_t bIsStanding : 1;
	uint32_t bWasStanding : 1;
	uint32_t bIsAttacking : 1;		// doesn't reset after fist fight
	uint32_t bIsPointingGunAt : 1;
	uint32_t bIsLooking : 1;
	uint32_t bKeepTryingToLook : 1; // if we can't look somewhere due to unreachable angles
	uint32_t bIsRestoringLook : 1;
	uint32_t bIsAimingGun : 1;

	uint32_t bIsRestoringGun : 1;
	uint32_t bCanPointGunAtTarget : 1;
	uint32_t bIsTalking : 1;
	uint32_t bIsInTheAir : 1;
	uint32_t bIsLanding : 1;
	uint32_t bIsRunning : 1; // on some conditions
	uint32_t bHitSomethingLastFrame : 1;
	uint32_t bVehEnterDoorIsBlocked : 1; // because someone else enters/exits from there

	uint32_t bCanPedEnterSeekedCar : 1;
	uint32_t bRespondsToThreats : 1;
	uint32_t bRenderPedInCar : 1;
	uint32_t bChangedSeat : 1;
	uint32_t bUpdateAnimHeading : 1;
	uint32_t bBodyPartJustCameOff : 1;
	uint32_t bIsShooting : 1;
	uint32_t bFindNewNodeAfterStateRestore : 1;

	uint32_t bHasACamera : 1; // does ped possess a camera to document accidents involves fire/explosion
	uint32_t bGonnaInvestigateEvent : 1;
	uint32_t bPedIsBleeding : 1;
	uint32_t bStopAndShoot : 1; // Ped cannot reach target to attack with fist, need to use gun
	uint32_t bIsPedDieAnimPlaying : 1;
	uint32_t bUsePedNodeSeek : 1;
	uint32_t bObjectiveCompleted : 1;
	uint32_t bScriptObjectiveCompleted : 1;

	uint32_t bKindaStayInSamePlace : 1;
	uint32_t bBeingChasedByPolice : 1;
	uint32_t bNotAllowedToDuck : 1;
	uint32_t bCrouchWhenShooting : 1;
	uint32_t bIsDucking : 1;
	uint32_t bGetUpAnimStarted : 1;
	uint32_t bDoBloodyFootprints : 1;
	uint32_t bFleeAfterExitingCar : 1;

	uint32_t bWanderPathAfterExitingCar : 1;
	uint32_t bIsLeader : 1;
	uint32_t bDontDragMeOutCar : 1; // unfinished feature
	uint32_t m_ped_flagF8 : 1;
	uint32_t bWillBeQuickJacked : 1;
	uint32_t bCancelEnteringCar : 1; // after door is opened or couldn't be opened due to it's locked
	uint32_t bObstacleShowedUpDuringKillObjective : 1;
	uint32_t bDuckAndCover : 1;

	uint32_t bStillOnValidPoly : 1; // set if the polygon the ped is on is still valid for collision
	uint32_t bAllowMedicsToReviveMe : 1;
	uint32_t bResetWalkAnims : 1;
	uint32_t bStartWanderPathOnFoot : 1; // exits the car if he's in it, reset after path found
	uint32_t bOnBoat : 1; // not just driver, may be just standing
	uint32_t bBusJacked : 1;
	uint32_t bGonnaKillTheCarJacker : 1; // only set when car is jacked from right door and when arrested by police
	uint32_t bFadeOut : 1;

	uint32_t bKnockedUpIntoAir : 1; // has ped been knocked up into the air by a car collision
	uint32_t bHitSteepSlope : 1; // has ped collided/is standing on a steep slope (surface type)
	uint32_t bCullExtraFarAway : 1; // special ped only gets culled if it's extra far away (for roadblocks)
	uint32_t bClearObjective : 1;
	uint32_t bTryingToReachDryLand : 1; // has ped just exited boat and trying to get to dry land
	uint32_t bCollidedWithMyVehicle : 1;
	uint32_t bRichFromMugging : 1; // ped has lots of cash cause they've been mugging people
	uint32_t bChrisCriminal : 1; // Is a criminal as killed during Chris' police mission (should be counted as such)

	uint32_t bShakeFist : 1;  // test shake hand at look entity
	uint32_t bNoCriticalHits : 1; // if set, limbs won't came off
	uint32_t bVehExitWillBeInstant : 1;
	uint32_t bHasAlreadyBeenRecorded : 1;
	uint32_t bFallenDown : 1;
	uint32_t bDontAcceptIKLookAts : 1;
	uint32_t bReachedAttractorHeadingTarget : 1;
	uint32_t bTurnedAroundOnAttractor : 1;

	uint32_t bHasAlreadyUsedAttractor : 1;
	uint32_t bHasAlreadyStoleACar : 1;
	uint32_t bCarPassenger : 1;
	uint32_t bFleeWhenStanding : 1;
	uint32_t bGotUpOfMyOwnAccord : 1;
	uint32_t bMiamiViceCop : 1;
	uint32_t bMoneyHasBeenGivenByScript : 1; //
	uint32_t bHasBeenPhotographed : 1;  //

	uint32_t bIsDrowning : 1;
	uint32_t bDrownsInWater : 1;
	uint32_t bWaitForLeaderToComeCloser : 1;
	uint32_t bHeldHostageInCar : 1;
	uint32_t bIsPlayerFriend : 1;
	uint32_t bHeadStuckInCollision : 1;
	uint32_t bDeadPedInFrontOfCar : 1;
	uint32_t bStayInCarOnJack : 1;

	uint32_t bDontFight : 1;
	uint32_t bDoomAim : 1;
	uint32_t bCanBeShotInVehicle : 1;
	uint32_t bCanGiveUpSunbathing : 1;
	uint32_t bMakeFleeScream : 1;
	uint32_t bPushedAlongByCar : 1;
	uint32_t bRemoveMeWhenIGotIntoCar : 1;
	uint32_t bIgnoreThreatsBehindObjects : 1;

	uint32_t bNeverEverTargetThisPed : 1;
	uint32_t bCrouchWhenScared : 1;
	uint32_t bKnockedOffBike : 1;
	uint32_t b158_8 : 1;
	uint32_t bCollectBusFare : 1;
	uint32_t bBoughtIceCream : 1;
	uint32_t bDonePositionOutOfCollision : 1;
	uint32_t bCanAttackPlayerWithCops : 1;

#ifdef KANGAROO_CHEAT
	// our own flags
	uint32_t m_ped_flagI80 : 1; // KANGAROO_CHEAT define makes use of this as cheat toggle 
#endif

	uint8_t m_gangFlags;
	uint8_t m_unused15D; // these 3 can't be padding but had to actually have been members ...
	uint8_t m_unused15E;
	uint8_t m_unused15F;
	uint8_t CharCreatedBy;
	PEDDATA_ENUM(eObjective) m_objective;
	PEDDATA_ENUM(eObjective) m_prevObjective;
	CPed *m_pedInObjective;
	CVehicle *m_carInObjective;
	PedVec m_nextRoutePointPos;
	float m_attractorHeading;
	CPed *m_leader;
	PEDDATA_ENUM(eFormation) m_pedFormation;
	uint32_t m_fearFlags;
	CEntity *m_threatEntity;
	PedVec2 m_eventOrThreat;
	uint32_t m_eventType;
	CEntity *m_pEventEntity;
	float m_fAngleToEvent;
	AnimBlendFrameData *m_pFrames[PED_NODE_MAX];
	RpAtomic *m_pWeaponModel;
	PEDDATA_ENUM(AssocGroupId) m_animGroup;
	CAnimBlendAssociation *m_pVehicleAnim;
	PedVec2 m_vecAnimMoveDelta;
	PedVec m_vecOffsetSeek;
	PEDDATA_PEDIK m_pedIK; 
	float m_actionX;
	float m_actionY;
	uint32_t m_nPedStateTimer;
	PEDDATA_ENUM(PedState) m_nPedState;
	PEDDATA_ENUM(PedState) m_nLastPedState;
	PEDDATA_ENUM(eMoveState) m_nMoveState;
	int32_t m_nStoredMoveState;
	int32_t m_nPrevMoveState;
	PEDDATA_ENUM(eWaitState) m_nWaitState;
	uint32_t m_nWaitTimer;
	CPathNode *m_pathNodesToGo[8];
	int16_t m_nNumPathNodes;
	int16_t m_nCurPathNodeId;
	CEntity *m_followPathWalkAroundEnt;
	CEntity *m_followPathTargetEnt;
	uint32_t m_pathNodeTimer;
	PEDDATA_PATHNODE m_pathNodeObjPool[8];
	CPathNode *m_pCurPathNode;
	int8_t m_nPathDir;
	CPathNode *m_pLastPathNode;
	CPathNode *m_pNextPathNode;
	PedVec m_followPathDestPos;
	float m_followPathAbortDist;
	PEDDATA_ENUM(eMoveState) m_followPathMoveState;
	float m_fHealth;
	float m_fArmour;
	uint32_t m_nExtendedRangeTimer;
	int16_t m_routeLastPoint;
	uint16_t m_routeStartPoint;
	int16_t m_routePointsPassed;
	int16_t m_routeType;	// See PedRouteType
	int16_t m_routePointsBeingPassed;
	PedVec2 m_moved;
	float m_fRotationCur;
	float m_fRotationDest;
	float m_headingRate;
	uint16_t m_vehDoor;
	int16_t m_walkAroundType;
	CPhysical *m_pCurrentPhysSurface;
	PedVec m_vecOffsetFromPhysSurface;
	CEntity *m_pCurSurface;
	PedVec m_vecSeekPos;
	CEntity *m_pSeekTarget;
	CVehicle *m_pMyVehicle;
	bool bInVehicle;
	float m_distanceToCountSeekDone;
	float m_acceptableHeadingOffset;
	CVehicle *m_vehicleInAccident;
	CPedAttractor* m_attractor;
	int32_t m_positionInQueue;
	bool bRunningToPhone;
	int16_t m_phoneId;
	PEDDATA_ENUM(eCrimeType) m_crimeToReportOnPhone;
	uint32_t m_phoneTalkTimer;
	CAccident *m_lastAccident;
	uint32_t m_nPedType;
	PedStats *m_pedStats;
	PedVec2 m_fleeFromPos;
	CEntity *m_fleeFrom;
	uint32_t m_fleeTimer;
	CEntity *m_threatEx; // TODO(Miami): What is this?
	CEntity *m_collidingEntityWhileFleeing;
	uint32_t m_collidingThingTimer;
	CEntity *m_pCollidingEntity;
	uint8_t m_stateUnused;
	uint32_t m_timerUnused;
	Range2D *m_wanderRangeBounds; // rouz edit (ChatGPT)
	PEDDATA_WEAPON m_weapons[TOTAL_WEAPON_SLOTS];
	PEDDATA_ENUM(eWeaponType) m_storedWeapon;
	PEDDATA_ENUM(eWeaponType) m_delayedWeapon;
	uint32_t m_delayedWeaponAmmo;
	uint8_t m_currentWeapon;			// eWeaponType
	uint8_t m_maxWeaponTypeAllowed;	// eWeaponType
	uint8_t m_wepSkills;
	uint8_t m_wepAccuracy;
	CEntity *m_pPointGunAt;
	PedVec m_vecHitLastPos;
	uint32_t m_curFightMove;
	uint32_t m_lastFightMove;
	uint8_t m_fightButtonPressure;
	int8_t m_fightState;
	bool m_takeAStepAfterAttack;
	uint8_t m_bleedCounter;
	CFire *m_pFire;
	CEntity *m_pLookTarget;
	float m_fLookDirection;
	int32_t m_wepModelID;
	uint32_t m_leaveCarTimer;
	uint32_t m_getUpTimer;
	uint32_t m_lookTimer;
	uint32_t m_chatTimer;
	uint32_t m_attackTimer;
	uint32_t m_shootTimer; // shooting is a part of attack
	uint32_t m_carJackTimer;
	uint32_t m_objectiveTimer;
	uint32_t m_duckTimer;
	uint32_t m_duckAndCoverTimer;
	uint32_t m_bloodyFootprintCountOrDeathTime;	// Death time when bDoBloodyFootprints is false. Weird decision
	uint32_t m_shotTime;
	uint32_t m_ceaseAttackTimer;
	uint8_t m_panicCounter;
	bool m_deadBleeding;
	int8_t m_bodyPartBleeding;		// PedNode, but -1 if there isn't
	CPed *m_nearPeds[10];
	uint16_t m_numNearPeds;
	uint16_t m_nPedMoney;
 	int8_t m_lastWepDam;
	CEntity *m_lastDamEntity;
	CEntity *m_attachedTo;
	PedVec m_vecAttachOffset;
	uint16_t m_attachType;
	float m_attachRotStep;
	uint32_t m_attachWepAmmo;
	uint32_t m_threatFlags;
	uint32_t m_threatCheckTimer;
	uint32_t m_threatCheckInterval;
	int32_t m_delayedSoundID;
	uint32_t m_delayedSoundTimer;
	uint32_t m_lastSoundStart;
	uint32_t m_soundStart;
	uint16_t m_lastQueuedSound;
	uint16_t m_queuedSound;
	bool m_canTalk;
	uint32_t m_lastComment;
	PedVec m_vecSpotToGuard;
	float m_radiusToGuard;

	// rouz edit
	PedVec surf_rel_origin;
} CPedData;
//- rouz edit (Claude)
