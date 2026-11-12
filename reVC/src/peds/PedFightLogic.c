//+ rouz edit (Claude)
#include "PedFightLogic.h"
#include <math.h>
#include "PedStates.h"
#include "PedType.h"
#include "WeaponType.h"
#include "AnimGroups.h"
#include "PedNode.h"
#include "AnimationId.h"

int PedFightLogic_ChooseAttackAI(const PedAttackAIInput *in, PedHeadNotAboveFn headNotAbove, void *context, PedRandomFn random)
{
	bool punchOnly = !!(in->pedFlags & STAT_PUNCH_ONLY);
	bool canRoundhouse = !!(in->pedFlags & STAT_CAN_ROUNDHOUSE);
	bool canKneeHead = !!(in->pedFlags & STAT_CAN_KNEE_HEAD);
	bool canKick = !!(in->pedFlags & STAT_CAN_KICK);
	bool hasShoppingBags = !!(in->pedFlags & STAT_SHOPPING_BAGS);
	float dist = in->dist;

	if (in->fightWithWeapon) {
		// Armed, a target on the ground is only hit when close, with a kick or the weapon ground moves
		if (in->targetOnGroundOrGettingUp) {
			if (in->nastyGame && dist < 1.2f && !in->targetIsPlayer
				&& (in->targetDead || headNotAbove(context))) {
				if (in->weaponFlagGround2nd)
					return FIGHTMOVE_MELEE2;
				if (in->weaponFlagGround3rd)
					return FIGHTMOVE_MELEE3;
				return FIGHTMOVE_GROUNDKICK;
			}
			return FIGHTMOVE_IDLE;
		}

		// Chain the weapon moves when close, otherwise close in
		if (dist < 2.f) {
			if (in->curFightMove == FIGHTMOVE_MELEE1) {
				if (in->hasSecondFireAnim)
					return FIGHTMOVE_MELEE2;
			}
			if (in->curFightMove == FIGHTMOVE_MELEE2) {
				if (in->hasFinishingAttackAnim)
					return FIGHTMOVE_MELEE3;
			}
			return FIGHTMOVE_MELEE1;
		}
		return FIGHTMOVE_SHUFFLE_F;
	}

	if (!hasShoppingBags) {
		if (punchOnly) {
			if (dist < 1.4f)
				return FIGHTMOVE_PUNCH;
		} else {
			// Kick a downed target, otherwise knee, punch or kick by distance
			if (in->targetOnGroundOrGettingUp) {
				if (in->nastyGame && dist < 1.2f && !in->targetIsPlayer
					&& (in->targetDead || headNotAbove(context)))
					return FIGHTMOVE_GROUNDKICK;
				return FIGHTMOVE_IDLE;
			}
			if (dist < 0.95f && canKneeHead)
				return FIGHTMOVE_KNEE;
			if (dist < 1.4f)
				return FIGHTMOVE_PUNCH;
			if (dist < 2.f && canKick) {
				int nextMove = FIGHTMOVE_LONGKICK;
				if (canRoundhouse && random() & 1)
					nextMove = FIGHTMOVE_ROUNDHOUSE;
				return nextMove;
			}
		}
		return FIGHTMOVE_SHUFFLE_F;
	}

	// Peds with shopping bags swing them
	if (dist < 2.f)
		return FIGHTMOVE_ROUNDHOUSE;
	return FIGHTMOVE_SHUFFLE_F;
}
int PedFightLogic_ChooseForwardMovePlayer(bool fightWithWeapon, float distToVictim, bool inFightState, int weaponType, float weaponRange, int lastFightMove, int curFightMove, bool hasFinishingAttackAnim, int randVal)
{
	if (fightWithWeapon) {
		// A knee when very close in a fight
		if (distToVictim < 0.95f - 0.2f && inFightState)
			return FIGHTMOVE_KNEE;
		if (weaponType == WEAPONTYPE_CLEAVER) {
			if (distToVictim < 0.85f * weaponRange)
				return FIGHTMOVE_MELEE1;
			return FIGHTMOVE_SHUFFLE_F;
		}
		if (distToVictim < 0.75f * weaponRange && weaponType != WEAPONTYPE_SCREWDRIVER) {
			// Chain the weapon moves
			if (lastFightMove == FIGHTMOVE_MELEE1 && hasFinishingAttackAnim)
				return FIGHTMOVE_MELEE2;
			if (lastFightMove == FIGHTMOVE_MELEE2 && hasFinishingAttackAnim)
				return FIGHTMOVE_MELEE3;
			return FIGHTMOVE_MELEE1;
		}
		if (distToVictim < weaponRange && hasFinishingAttackAnim)
			return FIGHTMOVE_MELEE3;
		return FIGHTMOVE_SHUFFLE_F;
	}

	// Bare hands by distance, jabs and punches change with the random number
	if (distToVictim < 0.95f && inFightState)
		return FIGHTMOVE_KNEE;
	if (distToVictim < 1.4f) {
		if (curFightMove == FIGHTMOVE_PUNCHJAB)
			return FIGHTMOVE_PUNCH;
		if (curFightMove != FIGHTMOVE_PUNCH || randVal != 1) {
			if (randVal == 2)
				return FIGHTMOVE_PUNCH;
			return FIGHTMOVE_PUNCHJAB;
		}
		return FIGHTMOVE_LONGKICK;
	}
	return FIGHTMOVE_LONGKICK;
}

int PedFightLogic_ChooseNoEnemyWeaponMove(int weaponType, int lastFightMove, bool hasFinishingAttackAnim)
{
	// Fight with the air
	if (weaponType == WEAPONTYPE_SCREWDRIVER)
		return FIGHTMOVE_MELEE3;
	if (lastFightMove == FIGHTMOVE_MELEE1 && hasFinishingAttackAnim)
		return FIGHTMOVE_MELEE2;
	if (lastFightMove == FIGHTMOVE_MELEE2 && hasFinishingAttackAnim)
		return FIGHTMOVE_MELEE3;
	return FIGHTMOVE_MELEE1;
}

int PedFightLogic_ChooseRandomUnarmedMove(int random)
{
	switch (random) {
	case 0: return FIGHTMOVE_PUNCHJAB;
	case 1: return FIGHTMOVE_PUNCH;
	case 2: return FIGHTMOVE_LONGKICK;
	case 3: return FIGHTMOVE_KNEE;
	default: return FIGHTMOVE_IDLE;
	}
}
bool PedFightLogic_FightMoveAnimSpeed(int animGroup, int weaponType, int curFightMove, float *speed)
{
	// Moves with a melee weapon of the knife group are timed by weapon, the others by the move
	if (animGroup == ASSOCGRP_KNIFE && curFightMove >= FIGHTMOVE_MELEE1) {
		switch (weaponType) {
		case WEAPONTYPE_SCREWDRIVER:
		case WEAPONTYPE_KNIFE:
			*speed = 1.05f;
			return true;
		case WEAPONTYPE_GOLFCLUB:
		case WEAPONTYPE_NIGHTSTICK:
		case WEAPONTYPE_BASEBALLBAT:
		case WEAPONTYPE_HAMMER:
		case WEAPONTYPE_KATANA:
			*speed = 0.8f;
			return true;
		case WEAPONTYPE_CLEAVER:
		case WEAPONTYPE_MACHETE:
			*speed = 0.9f;
			return true;
		}
		return false;
	}

	// Back kicks are a little faster
	if (curFightMove == FIGHTMOVE_BACKKICK)
		*speed = 1.15f;
	else
		*speed = 0.8f;
	return true;
}
#define PFL_PI ((float)3.14159265358979323846)
#define PFL_TWOPI (PFL_PI*2)
#define PFL_DEGTORAD(x) ((x) * PFL_PI / 180.0f)

float PedFightLogic_NeededTurn(float targetAngle, float rotationCur)
{
	float neededTurn = fabsf(targetAngle - rotationCur);
	if (neededTurn > PFL_PI)
		neededTurn = PFL_TWOPI - neededTurn;
	return neededTurn;
}

int PedFightLogic_VictimDirection(float victimAngle, float rotationCur, int numPedsWeCanReach, bool fightWithWeapon)
{
	float adjustedAngleDiff = victimAngle - rotationCur + PFL_DEGTORAD(30.0f);
	if (adjustedAngleDiff < 0.0f)
		adjustedAngleDiff += PFL_TWOPI;

	int16_t dir = (int16_t)floorf(adjustedAngleDiff / PFL_DEGTORAD(60.0f));

	// Just focus on who we're fighting with, don't care about peds on the ground
	if (numPedsWeCanReach < 2 || fightWithWeapon) {
		if (PedFightLogic_NeededTurn(victimAngle, rotationCur) < PFL_DEGTORAD(60.0f))
			dir = 0;
	}
	return dir;
}

int PedFightLogic_DirectionMove(int dir)
{
	switch (dir) {
	case 1: return FIGHTMOVE_FWDLEFT;
	case 2: return FIGHTMOVE_BACKLEFT;
	case 3: return FIGHTMOVE_BACKKICK;
	case 4: return FIGHTMOVE_BACKRIGHT;
	default: return FIGHTMOVE_FWDRIGHT;
	}
}

int PedFightLogic_GroundMove(bool fightWithWeapon, bool weaponGround2nd, bool weaponGround3rd)
{
	if (fightWithWeapon && weaponGround2nd)
		return FIGHTMOVE_MELEE2;
	if (fightWithWeapon && weaponGround3rd)
		return FIGHTMOVE_MELEE3;
	return FIGHTMOVE_GROUNDKICK;
}

int PedFightLogic_StrikeNode(int fightMove)
{
	switch (fightMove) {
	case FIGHTMOVE_KNEE:
		return PED_LOWERLEGR;
	case FIGHTMOVE_PUNCHHOOK:
	case FIGHTMOVE_PUNCHJAB:
		return PED_HANDL;
	case FIGHTMOVE_LONGKICK:
	case FIGHTMOVE_ROUNDHOUSE:
	case FIGHTMOVE_FWDLEFT:
	case FIGHTMOVE_BACKRIGHT:
	case FIGHTMOVE_GROUNDKICK:
		return PED_FOOTR;
	case FIGHTMOVE_FWDRIGHT:
		return PED_HEAD;
	case FIGHTMOVE_BACKKICK:
	case FIGHTMOVE_BACKFLIP:
		return PED_FOOTL;
	case FIGHTMOVE_BACKLEFT:
		return PED_UPPERARML;
	default:
		return PED_HANDR;
	}
}

// Values of FightMoveHitLevel, checked by a static_assert in PedFight.cpp
#define PFL_HITLEVEL_GROUND 1
#define PFL_HITLEVEL_LOW 2
#define PFL_HITLEVEL_HIGH 4

bool PedFightLogic_FlinchWithoutFall(int hitLevel, bool isPlayer, PedRandomFn random)
{
	return (hitLevel != PFL_HITLEVEL_HIGH && hitLevel != PFL_HITLEVEL_LOW) || ((isPlayer || (random() & 1)) && (random() & 7));
}

int PedFightLogic_ShotAnim(int direction)
{
	switch (direction) {
	case 1: return ANIM_STD_HITBYGUN_LEFT;
	case 2: return ANIM_STD_HITBYGUN_BACK;
	case 3: return ANIM_STD_HITBYGUN_RIGHT;
	default: return ANIM_STD_HITBYGUN_FRONT;
	}
}

int PedFightLogic_FlinchAnim(int direction, int hitLevel, PedRandomFn random, bool *fall)
{
	*fall = true;
	switch (direction) {
	case 1:
		return ANIM_STD_HIGHIMPACT_LEFT;
	case 2:
		if (random() & 1) {
			*fall = false;
			return ANIM_STD_HIT_BACK;
		}
		return ANIM_STD_HIGHIMPACT_BACK;
	case 3:
		return ANIM_STD_HIGHIMPACT_RIGHT;
	default:
		if (hitLevel == PFL_HITLEVEL_LOW)
			return ANIM_STD_KO_SHOT_STOMACH;
		if (random() & 1) {
			*fall = false;
			return ANIM_STD_HIT_WALK;
		}
		if (random() & 1) {
			*fall = false;
			return ANIM_STD_HIT_HEAD;
		}
		return ANIM_STD_KO_SHOT_FACE;
	}
}

int PedFightLogic_DefendHitMove(bool headHit, int direction, int unk)
{
	switch (direction) {
	case 1: return FIGHTMOVE_HITLEFT;
	case 2: return FIGHTMOVE_HITBACK;
	case 3: return FIGHTMOVE_HITRIGHT;
	default:
		if (unk <= 5)
			return headHit ? FIGHTMOVE_HITHEAD : FIGHTMOVE_HITCHEST;
		return FIGHTMOVE_HITBIGSTEP;
	}
}

void PedFightLogic_MeleeStartTiming(int animGroup, float *delayBetweenAnimAndFire, float *animLoopStart)
{
	switch (animGroup) {
	case ASSOCGRP_UNARMED:
	case ASSOCGRP_SCREWDRIVER:
	case ASSOCGRP_KNIFE:
	case ASSOCGRP_BASEBALLBAT:
	case ASSOCGRP_GOLFCLUB:
	case ASSOCGRP_CHAINSAW:
		*delayBetweenAnimAndFire = 0.2f;
		*animLoopStart = 0.1f;
		break;
	default:
		break;
	}
}

bool PedFightLogic_NotChainsawHit(bool isChainsaw, bool meleeAttackStarted, float delayBetweenAnimAndFire, float currentTime, float timeStep)
{
	return !isChainsaw
		|| (!meleeAttackStarted && delayBetweenAnimAndFire - 0.5f >= currentTime)
		|| currentTime - timeStep > delayBetweenAnimAndFire;
}

float PedFightLogic_LoopEndWithDelay(float animLoopEnd, float delayBetweenAnimAndFire)
{
	float loopEndWithDelay = animLoopEnd;
	if (loopEndWithDelay >= 98.0f)
		loopEndWithDelay = (14.0f / 30.0f) + delayBetweenAnimAndFire;
	return loopEndWithDelay;
}

// Values of ePedPieceTypes, checked by static_asserts in PedFight.cpp
#define PFL_PEDPIECE_MID 1
#define PFL_PEDPIECE_LEFTARM 2
#define PFL_PEDPIECE_RIGHTARM 3
#define PFL_PEDPIECE_HEAD 6

int PedFightLogic_CarHitClassify(bool isTrain, float moveForwardDot, float rightDot, float fwdDot, float boundsMaxX, float boundsMaxY, int randVal, bool *runOver, bool *knockedUp)
{
	*runOver = false;
	*knockedUp = false;

	// Trains always run the ped over
	if (isTrain) {
		*runOver = true;
		return PED_HEAD;
	}

	// A car that is not reversing
	if (moveForwardDot >= 0.0f) {
		// The ped is at the side of the car
		if (0.99f * boundsMaxX < fabsf(rightDot)) {
			if (fabsf(fwdDot) < 0.85f * boundsMaxY)
				*runOver = true;
			return rightDot <= 0.0f ? PED_UPPERARML : PED_UPPERARMR;
		}

		// The car looks at the ped or by chance runs it over
		if ((fwdDot <= 0.1 || randVal <= 1) && randVal != 0) {
			*runOver = true;
			return PED_HEAD;
		}

		*knockedUp = true;
		return PED_MID;
	}

	return PED_TORSO;
}

int PedFightLogic_PieceForNode(int node)
{
	switch (node) {
	case PED_HEAD: return PFL_PEDPIECE_HEAD;
	case PED_UPPERARML: return PFL_PEDPIECE_LEFTARM;
	case PED_UPPERARMR: return PFL_PEDPIECE_RIGHTARM;
	default: return PFL_PEDPIECE_MID;
	}
}

uint8_t PedFightLogic_CarHitShakeFreq(float carSpeed, float carMass)
{
	if (100.0f * carSpeed * 2000.0f / carMass + 80.0f <= 250.0f)
		return (uint8_t)(100.0f * carSpeed * 2000.0f / carMass + 80.0f);
	return 250;
}

int PedFightLogic_ShoveHitAnim(bool otherLooksToUs, bool weAreAtOthersLeft, bool otherIsRunning, int *forceSide)
{
	*forceSide = 0;

	if (otherLooksToUs && weAreAtOthersLeft) {
		*forceSide = 1;
		return otherIsRunning ? ANIM_STD_HITBYGUN_LEFT : ANIM_STD_HIT_LEFT;
	}
	if (otherLooksToUs) {
		*forceSide = -1;
		return otherIsRunning ? ANIM_STD_HITBYGUN_RIGHT : ANIM_STD_HIT_RIGHT;
	}
	return otherIsRunning ? ANIM_STD_HITBYGUN_BACK : ANIM_STD_HIT_BACK;
}

int PedFightLogic_KnockdownAnim(bool weAreAtOthersRight, bool otherLooksToUs, int *forceSide)
{
	if (weAreAtOthersRight) {
		*forceSide = -1;
		return otherLooksToUs ? ANIM_STD_HIGHIMPACT_RIGHT : ANIM_STD_SPINFORWARD_RIGHT;
	}
	*forceSide = 1;
	return otherLooksToUs ? ANIM_STD_HIGHIMPACT_LEFT : ANIM_STD_SPINFORWARD_LEFT;
}

bool PedFightLogic_WalkAroundNotEvade(bool weAreMissionChar, int ourFear, int otherTemper, bool otherIsPlayer, bool otherStill, float otherPlayerMoveSpeed)
{
	return weAreMissionChar || ourFear <= 100 - otherTemper
		|| ((otherIsPlayer || otherStill) && (!otherIsPlayer || otherPlayerMoveSpeed <= 1.0f));
}

bool PedFightLogic_BumpIsPeaceful(int ourTemper, int ourFear, bool weAreArmed, bool weAreMissionChar, bool otherIsCivFemale, bool sameType, bool otherIsArmed)
{
	return ourTemper <= ourFear || weAreArmed || weAreMissionChar || otherIsCivFemale || sameType || otherIsArmed;
}

bool PedFightLogic_ShoveStaggers(float otherDefendWeakness, bool otherIsMissionChar, float ourDefendWeakness)
{
	return otherDefendWeakness <= 1.5f || otherIsMissionChar || ourDefendWeakness <= otherDefendWeakness;
}

// Values of eEventType, checked by static_asserts in PedFight.cpp
#define PFL_EVENT_EXPLOSION 13
#define PFL_EVENT_ICECREAM 20
#define PFL_EVENT_SHOPSTALL 22

int PedFightLogic_InvestigateApproach(int eventType, float distSqr, float distToCountSeekDone)
{
	// Run to events that are far away
	if (eventType < PFL_EVENT_ICECREAM) {
		if ((5.0f + distToCountSeekDone) * (5.0f + distToCountSeekDone) < distSqr)
			return PEDMOVE_RUN;
	}

	if (eventType <= PFL_EVENT_EXPLOSION || eventType >= PFL_EVENT_SHOPSTALL)
		return PEDMOVE_WALK;

	if (distSqr > 1.2f * 1.2f)
		return PEDMOVE_WALK;

	return PEDFIGHT_CHECK_CROWD;
}

//- rouz edit (Claude)
