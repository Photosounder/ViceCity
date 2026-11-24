//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "VectorMath.h"
#include "PedData.h"
#include "PointBlank.h"
#include "PedFormation.h"

// How a ped came to exist, stored in CPedData::CharCreatedBy
enum {
	RANDOM_CHAR = 1,
	MISSION_CHAR,
	UNK_CHAR,
};

#ifdef __cplusplus
extern "C" {
#endif
// State predicates of a ped that only need its own data
bool PedLogic_CanUseTorsoWhenLooking(const CPedData *self);
bool PedLogic_IsPedInControl(const CPedData *self);
bool PedLogic_IsPedShootable(const CPedData *self);
bool PedLogic_CanPedReturnToState(const CPedData *self);
bool PedLogic_DyingOrDead(const CPedData *self);
bool PedLogic_EnteringCar(const CPedData *self);
bool PedLogic_CanSetPedState(const CPedData *self);
bool PedLogic_CanStrafeOrMouseControl(const CPedData *self, bool freeCam);
bool PedLogic_CanBeDeleted(const CPedData *self);
bool PedLogic_CanBeDeletedEvenInVehicle(const CPedData *self);
bool PedLogic_IsPlayer(const CPedData *self);
bool PedLogic_IsGangMember(const CPedData *self);
// Timers and small state changes of a ped, the time is the game time in milliseconds
void PedLogic_SetLookTimer(CPedData *self, uint32_t now, int time);
void PedLogic_SetAttackTimer(CPedData *self, uint32_t now, uint32_t time);
void PedLogic_SetShootTimer(CPedData *self, uint32_t now, uint32_t time);
void PedLogic_ClearFollowPath(CPedData *self);
void PedLogic_RestoreHeadingRate(CPedData *self);
bool PedLogic_HasAttractor(const CPedData *self);
// Direction of an offset relative to the ped heading: 0 forward, 1 left, 2 backward, 3 right
int PedLogic_GetLocalDirection(const CPedData *self, float offsetX, float offsetY);

// Remember the current state so it can be restored later, unless a state is already stored
void PedLogic_SetStoredState(CPedData *self);
// Look in a direction: now is the game time in milliseconds
void PedLogic_SetLookFlagDirection(CPedData *self, uint32_t now, float direction, bool keepTryingToLook, bool cancelPrevious);

// Turn the ped body towards its look target or look direction, returns true when the turn is done
bool PedLogic_TurnBody(CPedData *self, float posX, float posY);
// Sort a list of peds by distance to a point, closest first, the range is inclusive
void PedLogic_SortPedsByDistance(float px, float py, float pz, void **list, int min, int max);

// Whether a ped near this one is in point blank range and in front, the result is a PointBlankNecessity, pedToVerify limits the test to that ped
struct CPedStruct;
int PedLogic_CheckForPointBlankPeds(struct CPedStruct *self, bool twoHanded, const void *pedToVerify);

// Objectives, the time is the game time in milliseconds
void PedLogic_SetObjectiveTimer(CPedData *self, uint32_t now, int time);
bool PedLogic_IsTemporaryObjective(int objective);
// Go back to the previous objective, returns true when the ped has to be told to leave its car next, bObjectiveCompleted is for the caller to clear afterwards
bool PedLogic_RestorePreviousObjective(CPedData *self);
// Whether the ped is close enough to the next route point, on arrival the count of passed points goes up
bool PedLogic_HaveReachedNextPointOnRoute(CPedData *self, float posX, float posY, float distToCountReached);
// Whether the gang member of the given ped type can hurt this ped
bool PedLogic_CanBeDamagedByThisGangMember(const CPedData *self, uint32_t whoPedType);

// Fallen, diving and dying peds use the ground collision model
bool PedLogic_UseGroundColModel(const CPedData *self);
// Drop threat and event entities that are gone, the flags tell whether the entity pointer is set but no longer valid
void PedLogic_CheckThreatValidity(CPedData *self, bool threatInvalid, bool eventInvalid);
// A driver waits for a ped that is about to get in, now is the game time in milliseconds
bool PedLogic_CanPedDriveOff(CPedData *self, uint32_t now);

// The next point of the route of a ped, -1 when it is complete and has to stop, the ped turns around or restarts as its route type says
int PedLogic_GetNextPointOnRoute(CPedData *self);
// Offset from the ped it follows to the place a ped in a formation stands at, offset is the distance to the leader
void PedLogic_FormationOffset(int formation, float offset, float *out);

// Whether a ped facing rotation can see a thing at a position, that is within threshold radians of straight ahead
bool PedLogic_CanSeeEntity(float rotation, float targetX, float targetY, float pedX, float pedY, float threshold);

// Whether a ped is willing to chat with another one, roadBetween is true when the path nodes of the ped say a road is in the way, the types are ped types
bool PedLogic_WillChat(bool roadBetween, int surfaceTouched, bool strangerIsSelf, uint32_t myType, uint32_t strangerType);

// Heading a player ped moves towards in first person from the walk stick values, angle is the last walk angle which is kept when the stick is centred sideways
float PedLogic_WorkOutHeadingFirstPerson(float offset, float leftRight, float upDown, float *angle);

// Animation a ped shakes its fist with while looking at a target, ANIM_STD_NUM when none, isPed says the target is a ped, targetIsCop is only meaningful for a ped and targetIsPolice for a vehicle model
int PedLogic_ChooseFistShakeAnim(bool isPlayer, bool handFree, bool targetIsPed, int temper, bool targetIsCop, bool targetIsPolice);

// Index of the vehicle with the highest value, -1 when none is above zero, values of vehicles that can not be stolen are given as zero
int PedLogic_PickMostValuableVehicle(const int *values, int count, int *bestValue);
// Whether a ped of this type is a possible victim of a mugging
bool PedLogic_IsMugVictimType(uint32_t pedType);

// New current rotation of a ped turning towards its destination rotation by at most headAmount radians, also limits the current rotation in rotCur
float PedLogic_TurnTowardsDest(float rotCur, float rotDest, float headAmount);
// Walk angle of a player relative to its facing folded into a half turn, the result is -PI/2 to PI/2
float PedLogic_FoldWalkAngle(float localWalkAngle);

// Velocity change of a ped pushed away from a steep cliff by its damage normal, moved is the movement it wanted
void PedLogic_SteepCliffVelocityChange(float normalX, float normalY, float movedX, float movedY, float *outX, float *outY);
// Limits the velocity change of a ped standing on a moving surface, dieOnVehicle is for a dying ped on a vehicle, notBoat is true unless the surface is a boat
void PedLogic_LimitSurfaceVelocityChange(float *vx, float *vy, bool dieOnVehicle, bool notBoat, float timeStep);

// Whether the water pushes a ped up so hard or is high enough over it that it is drowning
bool PedLogic_IsBuoyantlySubmerged(float impulseZ, float mass, float timeStep, float waterLevel, float posZ);
// Whether the water pushes a ped up hard enough to slow it down horizontally
bool PedLogic_IsBuoyancySlowing(float impulseZ, float mass, float timeStep);
// Speed multiplier that slows a ped down in water over a time step
float PedLogic_WaterSpeedMult(float timeStep);
// Vertical speed of a ped slowed down by water, splash is set when it hit the water fast enough to splash
float PedLogic_WaterVerticalSpeed(float speedZ, bool *splash);

// Alpha of a ped for the next frame, it fades in by 16 or out by 8 and stays within 0 to 255
int PedLogic_NextClumpAlpha(int alpha, bool fadeOut);
// Distance up to which a dead ped's blood reaches nearby peds, it grows over the 5 seconds that start 2 seconds after death
float PedLogic_DeadBloodRadius(uint32_t timeSinceDeath);
// New panic counter of a ped, it counts up to 50 frames while the ped keeps being stuck when it wants to move and resets otherwise
int PedLogic_NextPanicCounter(int panicCounter, bool resets);

// Angle a ped hit by a damage normal should face to turn away from it, it also turns the current rotation by twice its heading change rate the short way around
float PedLogic_TurnAwayFromDamage(float damageAngle, float *rotCur, float headingChangeRate);
// Wraps a destination rotation to the half turn around the current rotation
float PedLogic_WrapDestRotation(float rotDest, float rotCur);

// Position in the local space of a car from which a ped opens a door: the seat position of the door side minus the offset the animation needs, offsets are those of vans, low cars and normal cars
VectorMath3 PedLogic_CarDoorLocalPosition(uint32_t component, bool isVan, bool lowVehicle, float seatOffsetDistance, float seatPosMult, VectorMath3 frontSeat, VectorMath3 backSeat, VectorMath3 vanRearOffset, VectorMath3 lowOffset, VectorMath3 normalOffset);

// Which door of a vehicle is the nearest from the distances to its front right, rear left and rear right doors: 0 for the first, 1 for the second and 2 for the third, a nearer door has to be strictly nearer
int PedLogic_NearestDoor(float rfX, float rfY, float lrX, float lrY, float rrX, float rrY);

// Positions around a vehicle a ped that is stuck in it is tried to be placed at, in the order they are tried
#define PEDEXIT_NUM_CANDIDATES 11
typedef struct PedExitInput {
	VectorMath3 vehPos, ourPos, right, forward, up;
	VectorMath3 boundsMin, boundsMax;
	VectorMath3 doorPos;
	bool onItsSide, doorValid, isCarType;
	float exitX;
	float feetOffset;
} PedExitInput;
// The candidate with the given index, false when there is none: 0 is the top of a vehicle on its side or the position to open the door, 1 the side of the vehicle, 2 to 5 sections along it, 6 the back, 7 the front, 8 and 9 the opposite side at the back and front and 10 the top
bool PedLogic_ExitCandidate(int index, const PedExitInput *in, VectorMath3 *out);

// Heading a ped faces while getting into or out of a vehicle from the door it uses, forwardHeading is the heading of the vehicle
float PedLogic_CarEntryRotation(uint32_t vehDoor, bool vehIsUpsideDown, bool vehIsBus, float forwardHeading);

// How far a ped has moved towards the seat while getting into or out of a vehicle, from the animation it plays, the values of the animation are given when there is one
typedef struct PedSeatPosInput {
	bool hasAnim;
	int32_t animId;
	float progress, timeLeft, totalLength, currentTime;
	bool vehIsBus, vehIsBike, inVehicle;
} PedSeatPosInput;
typedef struct PedSeatPosResult {
	float seatPosMult;
	float zBlend;
	bool multExtractedFromAnim, multExtractedFromAnimBus;
} PedSeatPosResult;
void PedLogic_CarSeatPosMult(const PedSeatPosInput *in, PedSeatPosResult *out);

// Door a ped leaves a vehicle by when it is not told which, slot is the place of the ped among the passengers or -1 for none, isDriver says it is the driver
uint32_t PedLogic_DefaultExitDoor(bool isBike, bool canJumpOut, bool isBus, bool isDriver, int slot);
// Whether someone enters or leaves a vehicle by the door a ped wants to leave by, from the doors in use for getting in and out
void PedLogic_ExitDoorBusy(bool isBike, uint32_t doorNode, uint32_t gettingInFlags, uint32_t gettingOutFlags, bool *entersFromDoor, bool *exitsFromDoor);

// Animation a ped plays when it gets out of a vehicle by a door: false when the door has none, the group is an animation group, rolling out is for jumping out of a moving car
bool PedLogic_CarExitAnim(uint32_t door, bool canJumpOut, bool isBus, bool isVan, bool isLow, int32_t *group, int32_t *anim);
// Same for a bike, whose animation group depends on the bike
bool PedLogic_BikeExitAnim(uint32_t door, int32_t *anim);
// Doors of a bike that are in use while a ped gets off by a door
int PedLogic_BikeExitFlags(uint32_t door);
// The door on the other side of a vehicle, the same door when it has no other side
uint32_t PedLogic_MirrorDoor(uint32_t door);

// Number of the pad of a player ped from its ped type: 0 to 3 for the four player types, 0 for anything else
int PedLogic_PadIndexForType(uint32_t pedType);

// Door of a vehicle a ped goes to to get at an occupant: the front left one for the driver of anyone in a bus, slot is the passenger seat of the occupant, 0 to 2, or -1, and 0 is returned when none of that applies
uint32_t PedLogic_TargetCarDoor(bool isDriverOrBus, int slot);

// Door, door flags and seat of the occupant a ped jacks when it uses a door of a vehicle, seat is 0 for none, 1 for the driver and 2 to 4 for the passengers 0 to 2
typedef struct PedCarJackDoor {
	int doorFlag;
	int door;
	int seat;
} PedCarJackDoor;
void PedLogic_CarJackDoor(bool isBike, uint32_t vehDoor, bool hasFrontPassenger, bool isCop, PedCarJackDoor *out);

// Door flags and door of a vehicle a ped uses when it gets in by a door, a vehicle without passenger seats has both left doors in use together, the door is 0 when no flag is set
void PedLogic_EnterCarDoor(bool isBike, uint32_t vehDoor, int numMaxPassengers, int *doorFlag, int *door);

// Door of the damage model of a vehicle from the node of a door, 0 for a node that is no door
int PedLogic_DoorFromNode(uint32_t node);

// Number of money pickups a mugged ped drops and the amount in each, before the random extra, for a total amount of money
void PedLogic_MuggingPickups(int money, int *pickupCount, int *moneyPerPickup);
// Whether a ped ignores someone pointing a gun at it: the first two conditions are about the ped itself, then about the gunman and the distance
bool PedLogic_IgnoresPointGun(bool isPlayer, bool inControl, bool missionCharNotCrouching, bool leaderIsGunman, bool alreadyHandsUpOrCowering, float distSq2D, bool gang7AndPlayerGunman);

// How a ped reacts to being attacked: the decision is made here from facts about the ped, the attacker and the vehicle, the effects are applied by the caller
typedef struct PedReactToAttackInput {
	bool isPlayer, attackerIsPed, isGang7, attackerIsPlayer, pedStateIsFall;
	bool driverBranch, driverSwitchApplies;
	bool controllable, respondsToThreats, notFollowingLeader;
	bool notAllowedToDuck, attackerMelee, crouchOrStay, stripper;
	bool fearOk, statsDiffer, gangOrEmergency, selfMelee;
} PedReactToAttackInput;
enum {
	PED_REACT_NONE,
	PED_REACT_INFORM_GANG_AND_LOOK,
	PED_REACT_GANG7_FALL,
	PED_REACT_DRIVER_SWITCH_PHYSICS,
	PED_REACT_COVER_TIMER,
	PED_REACT_DUCK,
	PED_REACT_STRIPPER_KILL,
	PED_REACT_KILL,
	PED_REACT_FLEE_FROM_RANGED,
	PED_REACT_FLEE_SCARED
};
// Returns the reaction, registerThreat says the threat is to be told to the gang and sayFleeRun that a scared ped shouts
int PedLogic_ReactToAttack(const PedReactToAttackInput *in, bool *registerThreat, bool *sayFleeRun);

// Whether a ped does not register an attacker as a threat to its gang: a mission friend of the player attacked by the player, a ped attacked by its leader or a GANG7 ped attacked by the player
bool PedLogic_ThreatIgnored(bool missionFriendAttackedByPlayer, bool attackerIsLeader, bool gang7AttackedByPlayer);
// Whether a driver near a car attacked by the player goes to ram the player: not a mission vehicle, driven by another random ped of the same type, and a normal car
bool PedLogic_DriverRamsPlayer(bool notMissionVehicle, bool hasDriver, bool driverIsOther, bool sameType, bool driverIsRandom, bool normalCar);

// What a follower does about the objective of its leader when it neither idles with it nor goes to it
enum {
	LEADER_FOLLOW_COPY_OBJECTIVE,
	LEADER_FOLLOW_GUARD_SPOT,
	LEADER_FOLLOW_COPY_TARGET_PED,
	LEADER_FOLLOW_ENTER_CAR,
	LEADER_FOLLOW_RETURN,
	LEADER_FOLLOW_DROP_LEADER,
	LEADER_FOLLOW_GOTO_LEADER
};
int PedLogic_FollowLeaderAction(int leaderObjective);
// Whether a follower in a car leaves it when its leader is out of a car: not when the leader is entering that same car
bool PedLogic_FollowerLeavesCar(int leaderObjective, bool carIsLeadersOrTarget);

// Facts about a ped and the entity of a new objective that decide whether setting the objective is ignored
typedef struct PedSetObjectiveInput {
	int newObjective, objective, prevObjective;
	bool dyingOrDead, entityIsSelf, attached, inVehicle;
	bool pedInObjectiveIsEntity, carInObjectiveIsEntity, leaderIsEntity, boughtIceCream;
} PedSetObjectiveInput;
// Whether setting an objective with an entity is a no-op because the ped is dead, the objective is already stored or being followed on that entity, or it cannot apply
bool PedLogic_SetObjectiveIgnored(const PedSetObjectiveInput *in);

// Offsets from a car on its right and left side where a ped can duck, depending on how many peds already use the car as cover
void PedLogic_CoverOffsets(int numPedsUsingCover, VectorMath3 *right, VectorMath3 *left);
// Whether a car can be used as cover: a car that is almost still, not a bus, a van or a big vehicle, and used by less than 3 peds
bool PedLogic_CoverCarEligible(bool isCar, float speed, bool isBus, bool isVan, bool isBig, int numPedsUsingCover);
// Whether the ped seeks a position other than the safe position (all three coordinates differ)
bool PedLogic_SeekNotAtSafePos(VectorMath3 seekPos, VectorMath3 safePos);
// Whether the left side is chosen to duck at, from the squared distances of the ped to defend to each side
bool PedLogic_CoverPrefersLeft(float distSqRight, float distSqLeft);

// Whether a ped attacking in melee targets a player who is on another surface, or in a zone with no police
bool PedLogic_NoPoliceZoneKill(bool noPoliceZone, bool victimOnOtherSurface, float distToVictim);
// Whether a ped gives up attacking a player: a cop that is ignored by cops, a player ignored by everyone, in water or arrested
bool PedLogic_MeleeVictimIgnored(bool isCop, bool ignoredByCops, bool ignoredByEveryone, bool victimInWater, bool victimArrested);
// Reach of a melee attack from the range of the weapon, unarmed peds that can't kick reach less
float PedLogic_MeleeWeaponRange(float weaponRange, bool unarmed, bool isPlayer, bool canKick);
// Whether a fighting ped steps forward: it is within reach of a victim who is in front of the attack position
bool PedLogic_MeleeGoesForward(VectorMath3 seekPos, VectorMath3 victimPos, VectorMath3 ourPos, float wepRange);
// Distance within which a cop arrests a player, clearDrag is set when the cop is no longer dragging the player from a car
float PedLogic_CopMaxArrestDist(bool dragsPlayer, bool stateFall, bool stateDragFromCar, bool *clearDrag);
// Whether a ped can strike now: its attack timer is over, the victim is in reach and neither getting up nor dragged from a car
bool PedLogic_MeleeCanStrikeNow(uint32_t attackTimer, uint32_t now, float distToVictim, float wepRange, bool victimGettingUp, bool victimDraggedFromCar);

// Distance at which an armed ped stops to aim when something showed up in its way: cops use more for strong weapons and objects, less for vehicles
float PedLogic_ObstacleSeekRange(bool isCop, bool strongWeapon, bool fleeFromObject, bool fleeFromVehicle);
// Whether an armed ped has to get closer to its victim: out of weapon range, the victim is getting up or arrested, or is entering a car close by
bool PedLogic_ArmedMustApproach(float distToVictim, float wepRange, uint32_t victimGetUpTimer, uint32_t now, bool victimArrested, bool victimEnteringCar);
// Whether an armed ped shoots at the vehicle of its victim: the attack timer is over and the vehicle is between 3 units and the weapon range away
bool PedLogic_ArmedCanShootVehicle(uint32_t attackTimer, uint32_t now, float distToVictim, float wepRange);

// Whether a ped that was to attack the player flees instead because cops are after the player: not for cops, mission peds or peds that stay in place
bool PedLogic_FleesFromPoliceHeat(bool victimIsPlayer, bool isCop, bool isMissionChar, int currentCops, bool staysInPlace);
// Factor of the cruise speed of a car chasing the player for a wanted level
float PedLogic_PoliceChaseSpeedFactor(int wantedLevel);

// Forward/backward lean a player on a bike aims for from the lean input, the wheelie and stoppie state and the pedals, setWheelieCam says that wheelieCam is to be written
float PedLogic_BikeLeanUD(bool isPlayer, float targetUDLean, float targetLRLean, float leanInput, const float wheelTimer[4], float forwardZ, float stoppieAng, float wheelieAng,
	float brakePedal, float gasPedal, float velocityFwd, float maxCruiseVelocity, bool *setWheelieCam, bool *wheelieCam);
// Weights of the forward/backward and left/right lean animations for the lean amounts of a bike rider
void PedLogic_BikeLeanAmounts(float leanLR, float leanUD, bool isPlayer, float *fwdBackLeanAmount, float *leftRightLeanAmount);

// Animation of a ped hit hard from a direction (0 front, 1 left, 2 back, 3 right), the current animation is kept for other directions
int PedLogic_HighImpactAnim(int direction, int current);
// Animation of a ped hit by a car from a random number from 0 to 3, the part of the ped that was hit and whether it was diving away, the current animation is kept for other numbers
int PedLogic_CarHitDieAnim(int random, bool pieceIsLeftArm, bool pieceIsRightArm, bool pieceIsMid, bool divingAway, int current);

// Side a ped walks around an object on (0 undecided, 1 to 4 for the four corners), from what stands at the corners of the object (0 nothing, 1 building, 2 vehicle, 3 other), the angle between the object and the ped and whether it is going to a car door, rotDestRel is the ped's heading relative to the direction to the object center
int PedLogic_WalkAroundType(int currentType, float angleDiffToCenter, float objTopRightHeading, bool objUpsideDown, bool goingToEnterCar, int vehDoor, float rotDestRel,
	int onTopLeft, int onTopRight, int onBottomRight, int onBottomLeft);
// Corner of an object a ped heads for when walking around it: maxX and maxY select the large or the small coordinate of the box, returns true when the ped is already at the door it goes to and stays
bool PedLogic_WalkAroundTarget(int walkAroundType, float angleDiffToCenter, float objTopRightHeading, bool goingToEnterCar, bool carIsVan, int vehDoor, float rotDestRel,
	int onTopLeft, int onTopRight, int onBottomRight, int onBottomLeft, bool *maxX, bool *maxY);

// Move state (eMoveState) a ped seeking a spot starts with: followedMoveState is the move state of the ped it follows in a formation, PEDMOVE_NONE is returned when the ped keeps its move state
int PedLogic_SeekInitialMove(int objective, float seekPosDist, bool isRunning, int followedMoveState);
// Move state after the ped has to turn back to the spot it seeks while following an action position, the turn is worked out from the heading it wants and the one it has
int PedLogic_SeekTurnAdjustMove(int nextMove, float rotDest, float rotCur, float seekPosDist, bool isRunning);
// Move state after the ped has to turn back to the spot it seeks or the next path node, turns beyond 135 degrees stop the ped
int PedLogic_SeekPathTurnAdjustMove(int nextMove, float rotDest, float rotCur, float seekPosDist);
// Whether a seeking ped takes the move state it worked out: fleeing peds only to move faster, others when not following a path, going to a character or waiting
bool PedLogic_SeekMayChangeMoveState(int pedState, int moveState, int nextMove, int objective, bool noWaitState);

// Heading a fleeing ped wants after running into something: it blends the way away from the thing it hit with the way away from the point it flees from, the more the closer the collision timer is to its end, rotDest and rotCur are updated
void PedLogic_FleeCollisionHeading(uint32_t collidingTimer, uint32_t now, float posX, float posY, float entityX, float entityY, float normalX, float normalY, float *rotDest, float *rotCur);

// Direction (0 to 7) opposite to a path direction, the one a wandering ped doesn't prefer to take
uint8_t PedLogic_OppositePathDir(int8_t pathDir);
// Direction a wandering ped looks for its next path node in: randVal from 0 to 99 sometimes turns it a quarter to the right or to the left
int8_t PedLogic_WanderPathDirAdjust(int8_t pathDir, uint8_t randVal);

// Milliseconds a ped waits in a wait state (eWaitState), states that take a time use it when hasTime is set and a default otherwise, random ones draw it here
int PedLogic_WaitStateTimerMs(int waitState, bool hasTime, int timeValue);

// What a ped that stops putting its hands up or ducking and looks at another ped does about it
enum {
	HANDSUP_REACT_NONE,
	HANDSUP_REACT_CROUCH,
	HANDSUP_REACT_MELEE_PANIC,
	HANDSUP_REACT_MELEE_WANDER,
	HANDSUP_REACT_KILL,
	HANDSUP_REACT_FLEE
};
int PedLogic_HandsUpReaction(bool lookTargetIsPed, bool fleeingOrAttacking, bool crouchWhenScared, bool fearOk, bool selfMelee, bool gunPanic);
// Whether a ped keeps its hands up: it doesn't look at an armed ped, isn't fleeing or attacking, the wait isn't over and the animation still plays
bool PedLogic_HandsUpKeepsWaiting(bool noPedToLookAt, bool lookTargetIsAimedAt, bool fleeingOrAttacking, bool waitRunning, bool animPlays);
// Animation (AnimationId) that has to play during a wait state of the ped that plays one
int PedLogic_WaitStateMustHaveAnim(int waitState);
// Whether a ped that doubles back scratches itself now: between 2000 and 2500 ms are left
bool PedLogic_DoubleBackScratch(uint32_t timeLeft);
// Animation (AnimationId) a stuck ped plays from a random number
int PedLogic_StuckWaitAnim(int randVal);

// Animation (AnimationId) and blend speed a ped plays for a move state (eMoveState), false for move states that play none
bool PedLogic_MoveAnimForState(int moveState, bool fleeingFromEntity, int *anim, float *blendRate);
// Playback speed of the walking and running animations: mission peds play them at normal speed, the others slightly differently per ped from their random seed
float PedLogic_MoveAnimSpeed(bool isMissionChar, uint16_t randomSeed, int myrandMax);

// Whether a ped that heard a shot considers the shooter a threat: peds of the same type don't, except civilians
bool PedLogic_ShooterIsThreat(int ourPedType, int shooterPedType);
// Whether a gang member that heard a shot flees from the shooter: its gang fears the shooter's type or it is a GANG5 member
bool PedLogic_GangFearsShooter(bool shooterTypeIsFeared, bool weAreGang5);

// Callbacks that a wait state hangs on its animation
enum {
	PEDWAIT_CB_NONE,
	PEDWAIT_CB_FINISH_FINISHED,		// FinishedWaitCB when the animation finishes
	PEDWAIT_CB_DELETE_FINISHED,		// FinishedWaitCB when the animation is deleted
	PEDWAIT_CB_FINISH_RESTORE_HEADING,	// RestoreHeadingRateCB when the animation finishes
	PEDWAIT_CB_DELETE_SUNBATHE		// DeleteSunbatheIdleAnimCB when the animation is deleted
};
// What the animation of a wait state does to its flags
enum {
	PEDWAIT_FLAGS_NONE,
	PEDWAIT_FLAGS_DELETE_AND_FADE_OUT,	// delete when faded out and fade out when done
	PEDWAIT_FLAGS_DELETE_NO_FADE_OUT	// delete when faded out but don't fade out when done
};
// The effects of putting a ped into a wait state, in the order they are applied: moveStill, moveAnim, headingRate, the timer, the animation with its flags and callback, and the check of a passenger that was about to enter a car
typedef struct PedWaitStateEffects {
	bool moveStill;		// the move state becomes still
	bool moveAnim;		// the move animation is updated afterwards
	float headingRate;	// the heading rate, negative to leave it
	bool hasTimer;		// the wait timer is set from PedLogic_WaitStateTimerMs
	int animGroup;		// AssocGroupId of the animation, negative for none
	int anim;		// AnimationId
	float blendRate;
	int flagsMode;		// PEDWAIT_FLAGS_*
	int callback;		// PEDWAIT_CB_*
	bool carJackCheck;	// a random ped that wanted to enter a car as passenger gives up and can't be carjacked for 30 seconds
} PedWaitStateEffects;
// Fills the effects of a wait state (eWaitState), returns false for states that do something else, which the caller handles
bool PedLogic_WaitStateEffects(int waitState, PedWaitStateEffects *fx);

// Time at which a ped that falls may get up again: never for extraTime -1, the length of the fall animation plus a delay otherwise, for the player the end of the animation (a roll out ends earlier by its current time)
uint32_t PedLogic_FallGetUpTimer(int extraTime, bool hasAnim, bool isPlayer, bool animIsRollOut, float animTotalLength, float animCurrentTime, uint32_t now, uint16_t randomSeed, uint32_t frameCounter);

// Whether a ped can't start ducking now: it is ducking already, or its duck timer is still running and the duck is not forced (sth), a forced duck extends the timer, newDuckTimer receives the timer to keep
bool PedLogic_DuckCannotStart(bool isDucking, uint32_t now, uint32_t duckTimer, bool sth, uint32_t time, uint32_t *newDuckTimer);

// How far a blood pool has grown, from 0 to 1, from the time since the ped died minus the 2 seconds it takes to start (the pool is full after 5 more seconds)
float PedLogic_BloodPoolFade(uint32_t timeSinceDeath);

// Whether a ped shakes a fist at a car driver that bumped into it: it is unarmed or holds a weapon of the slot 1, 3 or 5
bool PedLogic_ShakesFistAtDriver(int weaponType, uint32_t weaponSlot);
// Whether a ped that is hit by a slow vehicle walks around it: it is in control and either not the player or going somewhere on its own
bool PedLogic_WalksAroundBumpingVehicle(bool inControl, bool isPlayer, bool objectiveIsGoingSomewhere);
// Impulse a car hit gives a ped for the sound and the damage, the player standing and walking into the car doesn't count the part it makes itself, dotProd is the dot product of its forward movement and the damage direction
float PedLogic_CarHitAdjustedImpulse(bool isPlayer, bool isStanding, float dotProd, float damageImpulse, float mass);
// A moving car that hits the player from the side: returns the dot product that decides whether it knocks the player over, evasive says the player steps aside, the dot products are those of the right vector of the car with its speed and with the forward vector of the player, and of the direction from the car to the player with the speed of the car
float PedLogic_CarHitSideCheck(float angleDiffFromFront, float vehTopRightHeading, float vehSpeedSqr, float rightDotSpeed, float rightDotPedForward, float distDotSpeed, bool *evasive);

// Whether a ped slows down in the air: it is not standing and moves up fast
bool PedLogic_InAirDrag(bool isStanding, float moveSpeedZ);
// Whether a ped needs its physics to be processed: the player and every ped that is not standing, moving, animated, jumping, in the air or carried by something, a still ped that stands somewhere skips it
bool PedLogic_NeedsPhysicsProcess(bool isPlayer, bool isStanding, float moveSpeedX, float moveSpeedY, float moveSpeedZ, int moveState, float animMoveDeltaX, float animMoveDeltaY,
	bool isJumping, bool isInTheAir, bool hasPhysSurface);
// Whether an animation (AnimationId) of a ped entering a car is one of those that open a door
bool PedLogic_IsOpenDoorAnim(int anim);
// Whether an animation (AnimationId) is one that pulls a ped out of a car
bool PedLogic_IsPullOutAnim(int anim);

// Whether a ped that is dying is set dead: it landed on something that doesn't carry it (the player always counts), it is in water or it has no collision
bool PedLogic_ShouldBeSetDead(bool isStanding, bool hasPhysSurface, bool isPlayer, bool isInWater, bool usesCollision);
// Whether the fanny magnet cheat makes a female civilian follow the player: it isn't led by someone yet and is sexy enough
bool PedLogic_FannyMagnetPullsPed(bool cheatOn, int pedType, int sexiness, bool hasLeader);
// Whether the stored polygon a ped stands on is forgotten: the player always forgets it, others when they moved far enough and are not just wandering
bool PedLogic_ForgetsStoredPoly(bool isPlayer, float collisionSpeed, int pedState);

// Speed limits above which a landing hurts a ped, an idle ped tolerates more, upper is the horizontal speed and lower the vertical one, the caller sets the standard values first
void PedLogic_LandingLimits(bool isIdle, float *upperSpeedLimit, float *lowerSpeedLimit);
// Whether a landing hurts the ped: it was not standing, it landed fast enough sideways (unless it was pushed by a car) or fell fast enough, and it didn't collide with the same entity before
bool PedLogic_LandingHurts(bool wasStanding, float speed, float upperSpeedLimit, bool pushedAlongByCar, float speedZ, float lowerSpeedLimit, bool collidedWithSameEntity);
// Damage of a landing from the horizontal and the vertical speed, damage2 is the part that comes from the horizontal speed only
float PedLogic_LandingDamage(float speed, float speedZ, float *damage2);
// Whether a ped that was not standing and plays its fall animation takes a small hit for landing while it still goes down
bool PedLogic_LandingBumps(bool wasStanding, bool hasFallAnim, float timeStep, float speedZ);

// Which footstep a walking or running ped makes in this frame: 1 when the animation passed the start of the step, 2 when it passed the second half, 0 for none
int PedLogic_FootStepPart(float currentTime, float timeStep, float stepStart, float stepEnd);
// Whether the surface (SurfaceType) is sand that keeps footprints
bool PedLogic_IsSandSurface(int surface);
// A step of a ped with bloody feet: its footprints get used up, returns true when no blood is left, count is the number of footprints left
bool PedLogic_BloodyFootprintStep(uint32_t *count);
// Whether a ped running through water throws up splashes in this frame
bool PedLogic_WaterRunSplashes(float pedSpeed, uint32_t frameCounter);
// Size of the splashes of a ped running through water
float PedLogic_WaterRunParticleSize(float pedSpeed);

// Heading a ped steps away from a vehicle with: its back to the vehicle, then a quarter turn away from the direction the vehicle moves in
float PedLogic_EvasiveStepAngle(float angleToFace, float dangerDirection);
// Heading a ped dives away from a vehicle with, the same but to the other side
float PedLogic_EvasiveDiveAngle(float angleToFace, float vehDirection);
// Animation (AnimationId) of an evasive step, ANIM_STD_NUM for step types without one
int PedLogic_EvasiveStepAnim(int animType);

// What a ped that arrived at the door of a car does
enum {
	PED_SEEKCAR_NONE,		// nothing
	PED_SEEKCAR_ENTER,		// enters the car
	PED_SEEKCAR_CARJACK,		// pulls the occupant out
	PED_SEEKCAR_CARJACK_FLEE_DRIVER,// pulls the occupant out and the driver flees after it got out
	PED_SEEKCAR_IDLE,		// the car is wrecked
	PED_SEEKCAR_RETURN		// nothing more is done this frame
};
// Chooses what a ped does at the door of a car from the status of the car (eEntityStatus), the door it is at (eDoors) and who sits in the car, leaderIsDriver says that the ped follows the driver
int PedLogic_SeekCarEnterAction(int status, bool isBike, bool isBus, int door, bool leaderIsDriver, bool hasDriver, bool hasPassenger0, bool hasPassenger1, bool hasPassenger2,
	bool isPlayer, bool passenger0DontDrag, bool objectiveIsEnterAsDriver);
// Move state a ped seeking a car takes: PEDMOVE_RUN when it runs already or the car with a driver moves away from it, PEDMOVE_WALK when it is close, PEDMOVE_NONE to leave it
int PedLogic_SeekCarMoveState(bool isRunning, bool carHasDriver, float distToDestSqr, float carSpeedX, float carSpeedY);
// Whether a ped may enter the car it seeks: 0 not, 1 yes, 2 leave the flag as it is, from the squared distance to the door and the half width of the car
int PedLogic_CanEnterSeekedCar(float distToDestSqr, float carBoxMaxX);

// Whether the walk angle relative to the ped is close enough to straight ahead for the first person diagonal walking
bool PedLogic_IsDiagonalWalk(float localWalkAngle);
// Yaw of the upper legs for a walk angle relative to the ped, folded to half a turn when it is beyond 100 degrees, returns whether the legs get turned (yaw within 50 degrees)
bool PedLogic_UpperLegsYaw(float localWalkAngle, float *yaw);

// Whether a target at the given 2D offset is in front of a ped facing the given 2D direction and closer than 40 units, line of sight is checked separately
bool PedLogic_TargetInViewRange(float dx, float dy, float forwardX, float forwardY);

// States of the emergency ped as eEmergencyPedState has them
enum {
	PED_EMERGENCY_READY = 0x0,
	PED_EMERGENCY_DETERMINE_NEXT_STATE = 0x1,
	PED_EMERGENCY_STAND_STILL = 0x80,
	PED_EMERGENCY_STOP = 0x100
};
// What a fireman does in a state: the state it ends in, whether it runs to the nearest fire (stops what it does, seeks the fire, runs and attends it) and whether it stands still, says something or stops the whole thing
typedef struct PedFiremanResult {
	int newState;
	bool chaseNearestFire;
	bool standIdle;
	bool sayExtinguishing;
	bool stop;
} PedFiremanResult;
// Decides the step of a fireman from its state, the nearest fire (if any), whether it is the fire attended, the distance to it, whether the attended fire still burns and whether the nearest fire is closer than 30
PedFiremanResult PedLogic_FiremanDecide(int state, bool hasNearestFire, bool nearestIsAttended, float fireDist, bool attendedIsOngoing, bool nearestWithin30);

// What a medic does when it gets close to the ped it revives: whether it stops running, the move state it takes (PEDMOVE_NONE for keeping it) and whether it starts the CPR
typedef struct PedMedicArrival {
	bool stopRunning;
	int moveState;
	bool startCPR;
} PedMedicArrival;
// Decides the arrival of a medic from the distance to the accident, whether the victim burns and whether another medic performs the CPR already
PedMedicArrival PedLogic_MedicArrival(float distToEmergency, bool victimOnFire, bool cprInProgress);
// Whether a medic that stands still goes back to look for accidents
bool PedLogic_MedicStandStillResumes(bool noRevivedPed, bool revivedPedHealthy, bool noVictim, bool noCPRInProgress, bool unattendedAccidents);
// Turn of a medic towards its patient, returns true when it faces it close enough to start the CPR, otherwise newRotation is half way to the destination
bool PedLogic_MedicFacesPatient(float rotationCur, float rotationDest, float *newRotation);

// What a medic without an accident to attend does
enum {
	PED_MEDICIDLE_NONE,
	PED_MEDICIDLE_KILL_DRIVER,		// attacks the driver of its ambulance when it is not a medic
	PED_MEDICIDLE_ENTER_AS_PASSENGER,	// gets in its ambulance as passenger
	PED_MEDICIDLE_ENTER_AS_DRIVER,		// gets in its ambulance as driver
	PED_MEDICIDLE_WANDER			// wanders along the paths
};
// Chooses what a medic does when there is no accident from whether it has an ambulance and sits in it, its current objective, who drives the ambulance and whether the ambulance is being entered
int PedLogic_MedicIdleAction(bool hasVehicle, bool inVehicle, bool objectiveIsEnterAsDriver, bool objectiveIsEnterAsPassenger, bool objectiveIsKillOnFoot,
	bool vehicleHasDriver, bool driverIsEmergency, bool someoneIsGettingIn, bool isWandering);

// Takes a cop out of the list of the cops in pursuit and fills its slot with the last cop of the list
void PedLogic_RemoveCopFromPursuit(void **cops, uint8_t maxCops, uint8_t *currentCops, void *self);
// Puts a cop in the first free slot of the list of the cops in pursuit when the limit allows it, returns whether it got a slot
bool PedLogic_AddCopToPursuit(void **cops, uint8_t maxCops, uint8_t *currentCops, void *self, bool ignoreCopLimit);

// What a cop does about its place in the pursuit
enum {
	PED_COPPURSUIT_NONE,			// nothing
	PED_COPPURSUIT_CLEAR_SELF,		// leaves the pursuit
	PED_COPPURSUIT_CLEAR_FARTHEST,		// makes the cop farthest from the target leave the pursuit
	PED_COPPURSUIT_JOIN,			// joins the pursuit
	PED_COPPURSUIT_CLEAR_FARTHEST_AND_JOIN	// makes the cop farthest from the target leave the pursuit and joins it
};
// Decides what a cop does about the pursuit from the cops in it, farthestCop gets the cop farthest from the target when it is farther than the cop itself, getDistance gives the distance of a cop to the target
int PedLogic_CopPursuitAction(bool inPursuit, void **cops, int oldCopNum, int maxCops, void *self, float distanceToTarget, float (*getDistance)(void *cop), void **farthestCop);

// What a cop that follows an entity does when it got there
enum {
	PED_COPARRIVAL_RESTORE,		// goes back to what it did before
	PED_COPARRIVAL_ARREST		// arrests the player
};
// Decides the arrival of a cop that follows an entity from whether it hunts a character on foot, its distance to the target and the state of the player (ped state), a missing player is -1
int PedLogic_CopSeekEntityArrival(bool objectiveIsKillOnFoot, float distanceToTarget, int playerState);
// What a cop that seeks a position does when it got there
enum {
	PED_COPSEEK_NONE,		// nothing
	PED_COPSEEK_ARREST,		// arrests the player who is getting in a car
	PED_COPSEEK_STILL,		// stands still as the target is on another floor
	PED_COPSEEK_CARJACK		// pulls the player out of the car
};
// Decides the arrival of a cop that seeks a position from whether the player has a vehicle, whether the player is entering a car, the height of the cop and of the position, and whether the car of the player can be entered
int PedLogic_CopSeekPosArrival(bool playerHasVehicle, bool playerEntersCar, float copZ, float seekZ, bool vehicleCanBeEntered);

// Planes of the box of the entity a ped walks around, from its corners in order: normals face the outside and dotCorrection is the offset of each plane
void PedLogic_FollowPathPlanes(const VectorMath3 corners[4], VectorMath3 normals[4], float dotCorrection[4]);
// Clips the line from the start to the target against the planes of the box, the points that are inside the box move to the planes, returns true when the line does not cross the box
bool PedLogic_FollowPathClip(const VectorMath3 normals[4], const float dotCorrection[4], VectorMath3 *startCandidate, VectorMath3 *targetCandidate, VectorMath3 ourPos, VectorMath3 dirToGo);
// Finds the planes of the box the line from the start to the target crosses and moves the ends of the line onto them, lastPlaneBehindUs and lastPlaneInFrontOfUs are -1 when there is none
void PedLogic_FollowPathFindPlanes(const VectorMath3 normals[4], const float dotCorrection[4], VectorMath3 *startCandidate, VectorMath3 *targetCandidate, int *lastPlaneBehindUs, int *lastPlaneInFrontOfUs);
// Which of the two routes around the box a ped takes (1 or 2) from whether each is free, the node each one stops at, their length, whether the obstacles are the same entity and whether the first obstacle is at least as big as the second
int PedLogic_FollowPathPickVariant(bool route1Ok, bool route2Ok, int nodeToStop1, int nodeToStop2, float length1, float length2, bool sameObstacle, bool obstacle1AtLeastAsBig);
// Distance between the spheres a ped tests around an object it walks around, from the size of the box of the object and the box with the margin
float PedLogic_WalkAroundCheckDist(float colMinY, float colMaxY, float colMinZ, float colMaxZ, float adjustedMinX, float adjustedMaxX);
// Time between the checks of a ped that walks around an object, from whether it is the player, the speed of the player and its move state (PEDMOVE_*)
float PedLogic_WalkAroundCheckTime(bool isPlayer, float playerMoveSpeed, int moveState);
// Heading a ped that walks around an object ends up facing from the heading it wants, the one it has, whether it hit something and the one it wanted before
float PedLogic_WalkAroundFinalRotation(float rotationDest, float rotationCur, bool hitSomethingLastFrame, float oldRotationDest, float dirToSet);
// Time in milliseconds a ped walks towards the corner of an object before it looks for the way again, from the distance to the corner, the limit for the timer and the time between checks
float PedLogic_WalkAroundTimer(float dist, float distLimit, float checkIntervalInTime);
// Corners of a car a ped can head for
enum {
	PED_CARCORNER_LEFT_REAR,
	PED_CARCORNER_RIGHT_REAR,
	PED_CARCORNER_LEFT_FRONT,
	PED_CARCORNER_RIGHT_FRONT,
	PED_CARCORNER_NONE
};
// Corner of a car a ped heads for to get around another car, from the door it wants to enter (eDoors), where it stands relative to the car, the world positions of the corners (x then y of left rear, right rear, left front and right front) and the heading of the car
int PedLogic_PickSeekCarCorner(int door, float helperX, float helperY, const float corners[8], float vehForwardHeading);
// Scale of a bone (PedNode) of the player ped on the axes it grows on, from the fatness: 1 plus a part of the fatness that depends on the bone
float PedLogic_TommyFatnessScale(int bone, float fatness);
// Direction the blood spurts from the part of the body (PedNode) that came off, from the up, right and forward vectors of the ped, zero for the other parts
void PedLogic_BloodSpurtDirection(int bodyPart, const float up[3], const float right[3], const float forward[3], float out[3]);
// What a ped does after it got out of a car
enum {
	PED_EXITCAR_NOTHING,
	PED_EXITCAR_FLEE,		// flees from the car
	PED_EXITCAR_WANDER,		// wanders away on its own
	PED_EXITCAR_KILL_CARJACKER,	// goes after the one who took the car
	PED_EXITCAR_WANDER_RANDOM	// wanders as it has nothing to do
};
// Chooses what a ped does after it got out of a car from the flags it has and its objective and state
int PedLogic_ExitCarFollowUp(bool fleeAfterExiting, bool hasVehicle, bool wanderAfterExiting, bool goingToKillCarJacker, bool noObjective, bool notMissionChar, bool isIdle, bool isPlayer);
// Heading a ped has after it got out of a bus, from the heading of the bus and whether it used the front left door
float PedLogic_BusExitHeading(float busHeading, bool frontLeftDoor);
// Whether an objective (eObjective) is one that makes a ped go somewhere or after somebody on foot, which a ped forgets once it sat in a car
bool PedLogic_IsGoOnFootObjective(int objective);
// Whether a ped keeps its objective (eObjective) after it got in a car, true for the ones that go on in the car or on foot
bool PedLogic_KeepsObjectiveInCar(int objective);
// Passenger seat a ped takes from the door it used (eDoors) and the kind of vehicle, -1 for the first free seat
int PedLogic_PassengerSeat(bool isBike, bool isBus, int door);
// Movement of a ped per time step from the movement of its animation (in the space of the ped), its heading and the time step: moved[0] is sideways and moved[1] forwards
void PedLogic_AnimMoved(float deltaX, float deltaY, float rotation, float timeStep, float moved[2]);
// Speed of a ped that moves at a speed in the direction of an angle, speed[0] is x and speed[1] is y
void PedLogic_SpeedFromAngle(float velocity, float angle, float speed[2]);
// Heading a ped turns to when it lines up with the heading of its attractor, from its current heading and the heading of the attractor, the new heading of the attractor stays within a half turn of the heading of the ped
float PedLogic_AttractorHeading(float rotationCur, float attractorHeading);
// Sideways lean of a ped on a bike after a frame: it blends towards the target (the lean of the bike) at a rate that depends on the time step, from the current amount, the target and the time step
float PedLogic_BikeLeanLRBlend(float amount, float target, float timeStep);
// Forward lean of a ped on a bike after a frame, from the current amount, the target and the time step
float PedLogic_BikeLeanUDBlend(float amount, float target, float timeStep);
// Factor the move speed of a ped is multiplied by each frame while it falls
float PedLogic_AirResistanceFactor(float timeStep);
// Direction on the ground a ped that has its head stuck in the collision is pushed away at, opposite to its move speed (x, y) normalised by the ground speed (at least 0.001)
void PedLogic_StuckPushDirection(float moveSpeedX, float moveSpeedY, float groundSpeed, float *x, float *y);
// Whether a ped faces the heading of its attractor closely enough, from its rotation, the heading and the acceptable offset
bool PedLogic_FacesAttractor(float rotationCur, float attractorHeading, float acceptableOffset);
// Whether a ped is not facing the heading of its attractor even when the heading wraps around by a full turn either way
bool PedLogic_NotFacingAttractor(float rotationCur, float attractorHeading, float acceptableOffset);
// Height a ped still has to climb to the position, never negative
float PedLogic_ClimbHeight(float targetZ, float currentZ);
// Time step of an animation used to smooth the position of a ped entering a vehicle, at least 0.1
float PedLogic_AnimTimeStep(float timeStep);
// Height of a ped that is moved down towards the needed height over the time left of its animation
float PedLogic_SmoothZDown(float currentZ, float neededZ, float timeLeft, float adjustedTimeStep);
// Height of a ped that is moved up towards the needed height over the time left of its animation
float PedLogic_SmoothZUp(float currentZ, float neededZ, float timeLeft, float adjustedTimeStep);
// Strength a victim is pushed by a fight move, from the damage multiplier, whether a weapon is used, whether the move is a ground kick and whether the victim is dying
float PedLogic_FightMoveMult(float damageMult, bool fightingWithWeapon, bool isGroundKick, bool victimIsDying);
// Blend amount of a steering animation of a driver, from the steer angle and the scale (-100 for the right animation, 100 for the left one), limited to 0..1
float PedLogic_DriveBlend(float steerAngle, float scale);
// Whether the pad asks the ped to walk: the stick is pushed on either axis
bool PedLogic_PadIsWalking(float padWalkX, float padWalkY);
// Direction on the ground a ped faces from its rotation angle: (-sin, cos)
void PedLogic_FacingDirection(float angle, float dir[2]);
// Whether a ped that is pushed against something keeps being pushed this frame, from the state timer, the time step (at least 0.01), the squared length of the force direction, the number of collision records and the vertical part of the force direction
bool PedLogic_KeepsPushing(uint32_t stateTimer, float adjustedTimeStep, float forceDirMagSqr, int collisionRecords, float forceDirZ);
// Whether a ped that is pushed against a building has been doing it long enough and is not pushed up: checked after the ped keeps pushing
bool PedLogic_PushedAgainstWallLong(uint32_t stateTimer, float adjustedTimeStep, float forceDirZ);
// Scale of the push a ped takes when hit by a car: the mass of the car over 1400, at most 1
float PedLogic_CarHitMassScale(float carMass);
// Blend delta that remains after an animation takes its part: the blend delta minus the smaller of the limit and the part the animation takes (0.02 of the time step times its blend delta plus its blend amount)
float PedLogic_BlendRemaining(float blendDelta, float limit, float timeStep, float assocBlendDelta, float assocBlendAmount);
// Whether a corrected ground height is close enough (less than 3 units) to the height of the target or of the tested position for a ped to be teleported there
bool PedLogic_WarpHeightAcceptable(float correctedZ, float warpToZ, float testedZ);
// Number of tries when looking for a position to teleport a ped to: the distance to cover halved, at most 10
int PedLogic_WarpTryCount(float halfOfDist);
// Angle between two directions, from 0 to PI (the angles must already be limited to a turn)
float PedLogic_AngleDifference(float angleA, float angleB);
// Whether a ped stands in front of a bike, so it can climb over the windscreen: the angle between the direction to the ped and the bike forward direction, wrapped to a turn, is within 30 degrees
bool PedLogic_IsInFrontOfBike(float angleDiff);
// Kind of obstacle found next to an object a ped walks around: 0 none, 1 a building, 2 a vehicle, 3 anything else
int PedLogic_ObstacleKind(bool hasObstacle, bool isBuilding, bool isVehicle);
// New delay setting between services of a prostitute: 30 shorter from 350 and above, 10 shorter below, never under 250
int PedLogic_SexFrequencyAfterService(int frequency);
// Health of the driver after a service: one more, capped at 25 above the maximum health
float PedLogic_DriverHealthAfterService(float maxHealth, float health);
// Time a firing animation restarts at after a reload: a little before the start of the loop, never before zero
float PedLogic_ReloadLoopRestartTime(float animLoopStart);
// Blend of the height of a ped entering a car: twice the time of the animation over its length, limited to 1
float PedLogic_EnterCarZBlend(float currentTime, float animLength);
// Distance a ped keeps from the target it attacks: 0.9 times the weapon range, at least 0.8
float PedLogic_MaxDistToKeep(float wepRange);
// Time step with a minimum of 0.01 so ped timers never stall
float PedLogic_AdjustedTimeStep(float timeStep);
// Wind of the clothes of a ped on a bike when the wind modifier is on: limited by the distance of the shift of the direction from 1
float PedLogic_LimitedWindMult(float windMult, float anyDirectionShift);
// Scale of the jump offset of a ped hit from below: 2 over the horizontal collision power, at most 4
float PedLogic_JumpOffsetScale(float collPower);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
