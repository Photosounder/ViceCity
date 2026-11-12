//+ rouz edit (Claude)
#pragma once
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

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
