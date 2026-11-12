//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

// What the attack choice of an AI ped depends on, the facts about the target are looked up by the caller
typedef struct PedAttackAIInput {
	bool fightWithWeapon;
	unsigned pedFlags;		// the ped stat flags
	float dist;			// distance to the target
	bool targetOnGroundOrGettingUp;
	bool nastyGame;
	bool targetIsPlayer;
	bool targetDead;
	unsigned weaponFlagGround2nd;	// non zero when the weapon has a second move on the ground
	unsigned weaponFlagGround3rd;
	int curFightMove;
	bool hasSecondFireAnim;
	bool hasFinishingAttackAnim;
} PedAttackAIInput;

typedef bool (*PedHeadNotAboveFn)(void *context);
typedef uint16_t (*PedRandomFn)(void);

#ifdef __cplusplus
extern "C" {
#endif
// The fight move an AI ped chooses once it has a target and a button pressure, headNotAbove tells that the head of the target is not above knee height and is only asked when it matters
int PedFightLogic_ChooseAttackAI(const PedAttackAIInput *in, PedHeadNotAboveFn headNotAbove, void *context, PedRandomFn random);
// Fight move of the player ped against a victim in front of it, randVal is a random number from 0 to 3 that is only used for bare hands
int PedFightLogic_ChooseForwardMovePlayer(bool fightWithWeapon, float distToVictim, bool inFightState, int weaponType, float weaponRange, int lastFightMove, int curFightMove, bool hasFinishingAttackAnim, int randVal);
// Fight move of the player ped when there is nobody to hit and a weapon in hand
int PedFightLogic_ChooseNoEnemyWeaponMove(int weaponType, int lastFightMove, bool hasFinishingAttackAnim);
// Bare handed move picked by a random number, FIGHTMOVE_IDLE for numbers that pick none
int PedFightLogic_ChooseRandomUnarmedMove(int random);
// Playback speed of the animation of a fight move, returns false when the speed is left as it is
bool PedFightLogic_FightMoveAnimSpeed(int animGroup, int weaponType, int curFightMove, float *speed);
// Angle a ped has to turn to face another one, from 0 to PI
float PedFightLogic_NeededTurn(float targetAngle, float rotationCur);
// Direction from 0 (forward) to 5 in steps of 60 degrees of the victim of the player, a victim within 60 degrees of the front is forward when the player isn't surrounded or has a weapon
int PedFightLogic_VictimDirection(float victimAngle, float rotationCur, int numPedsWeCanReach, bool fightWithWeapon);
// Fight move for the directions of the victim other than forward
int PedFightLogic_DirectionMove(int dir);
// Fight move against a ped on the ground, weapons can have a second or third move for it
int PedFightLogic_GroundMove(bool fightWithWeapon, bool weaponGround2nd, bool weaponGround3rd);
// Node of the ped (PedNode) that hits when it performs a fight move
int PedFightLogic_StrikeNode(int fightMove);
// Whether a ped running away or the player running reacts to a hit with a flinch that doesn't make it fall, hitLevel is a FightMoveHitLevel, random is only asked when the original asked for it
bool PedFightLogic_FlinchWithoutFall(int hitLevel, bool isPlayer, PedRandomFn random);
// Animation of a ped that is shot while it runs, from the direction of the hit
int PedFightLogic_ShotAnim(int direction);
// Animation of a ped hit hard while it runs, fall is cleared when the animation is just played, random is only asked when the original asked for it
int PedFightLogic_FlinchAnim(int direction, int hitLevel, PedRandomFn random, bool *fall);
// Fight move of a ped hit in the head or the chest from a direction, unk is the strength of the hit
int PedFightLogic_DefendHitMove(bool headHit, int direction, int unk);
// Timing of the start animation of a melee attack, the values are left as they are for the animation groups that don't change them
void PedFightLogic_MeleeStartTiming(int animGroup, float *delayBetweenAnimAndFire, float *animLoopStart);
// Whether the attack of a ped takes the branch that isn't the chainsaw hitting: not a chainsaw, or an attack not just started and a long time before the fire time, or past the fire time
bool PedFightLogic_NotChainsawHit(bool isChainsaw, bool meleeAttackStarted, float delayBetweenAnimAndFire, float currentTime, float timeStep);
// End of the loop of an attack animation, for animations without one it is a little after the fire time
float PedFightLogic_LoopEndWithDelay(float animLoopEnd, float delayBetweenAnimAndFire);
// How a car that hits a ped that isn't the player hurts it, returns the PedNode that is hit, runOver says that the ped is run over (the car speed is passed on to it) and knockedUp that it is thrown up in the air
int PedFightLogic_CarHitClassify(bool isTrain, float moveForwardDot, float rightDot, float fwdDot, float boundsMaxX, float boundsMaxY, int randVal, bool *runOver, bool *knockedUp);
// ePedPieceTypes of the damaged piece of the ped for the PedNode that is hit
int PedFightLogic_PieceForNode(int node);
// Frequency of the pad shake when the player rams a ped with a car from the speed and mass of the car, from 0 to 250
uint8_t PedFightLogic_CarHitShakeFreq(float carSpeed, float carMass);
// Animation the ped that is bumped into plays, forceSide is the side the bumping ped is pushed to: 1 to its right, -1 to its left, 0 not pushed
int PedFightLogic_ShoveHitAnim(bool otherLooksToUs, bool weAreAtOthersLeft, bool otherIsRunning, int *forceSide);
// Animation the ped that is knocked down plays, forceSide as above
int PedFightLogic_KnockdownAnim(bool weAreAtOthersRight, bool otherLooksToUs, int *forceSide);
// Whether a ped that bumps into another one walks around it instead of getting out of its way
bool PedFightLogic_WalkAroundNotEvade(bool weAreMissionChar, int ourFear, int otherTemper, bool otherIsPlayer, bool otherStill, float otherPlayerMoveSpeed);
// Whether a ped that bumps into another one walks around it instead of fighting
bool PedFightLogic_BumpIsPeaceful(int ourTemper, int ourFear, bool weAreArmed, bool weAreMissionChar, bool otherIsCivFemale, bool sameType, bool otherIsArmed);
// Whether a bump of a ped makes the other one stagger rather than fall
bool PedFightLogic_ShoveStaggers(float otherDefendWeakness, bool otherIsMissionChar, float ourDefendWeakness);
// Returned by PedFightLogic_InvestigateApproach when the move state depends on whether another ped stands on the spot of the event
#define PEDFIGHT_CHECK_CROWD (-1)
// Move state (eMoveState) of a ped that goes to the spot of an event it investigates, from the type of the event, the squared distance to it and the distance that counts as being there
int PedFightLogic_InvestigateApproach(int eventType, float distSqr, float distToCountSeekDone);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
