//+ rouz edit (Claude)
#include "PedLogic.h"
#include "PedStates.h"
#include "PedNode.h"
#include "PedType.h"
#include "PedWaitStates.h"
#include "VehicleDoors.h"
#include "CarNodes.h"
#include "EntityStatus.h"
#include "SurfaceTypes.h"
#include "PedRoutes.h"
#include "PedObjectives.h"
#include "AnimGroups.h"
#include "AnimationId.h"
#include "maths.h"
#include "config.h"
#include <math.h>
#include "General.h"
#include "EntityLayout.h"
#include <stddef.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw_constants.h"

bool PedLogic_CanUseTorsoWhenLooking(const CPedData *self)
{
	// Driving, being dragged out of a car and ducking peds cannot, nor can women with their own animation group
	if (self->m_nPedState != PED_DRIVING && self->m_nPedState != PED_DRAG_FROM_CAR && !self->bIsDucking) {
		if (self->m_animGroup != ASSOCGRP_SEXYWOMAN && self->m_animGroup != ASSOCGRP_WOMAN)
			return true;
	}
	return false;
}

bool PedLogic_IsPedInControl(const CPedData *self)
{
	// In an AI-capable state, on the ground and alive
	return self->m_nPedState <= PED_STATES_NO_AI
		&& !self->bIsInTheAir && !self->bIsLanding
		&& self->m_fHealth > 0.0f;
}

bool PedLogic_IsPedShootable(const CPedData *self)
{
	// Only some states can be shot
	return self->m_nPedState <= PED_STATES_NO_ST;
}

bool PedLogic_CanPedReturnToState(const CPedData *self)
{
	// AI-capable states except the ones that are always interrupted
	return self->m_nPedState <= PED_STATES_NO_AI && self->m_nPedState != PED_AIM_GUN && self->m_nPedState != PED_ATTACK &&
		self->m_nPedState != PED_FIGHT && self->m_nPedState != PED_STEP_AWAY && self->m_nPedState != PED_SNIPER_MODE && self->m_nPedState != PED_LOOK_ENTITY;
}

bool PedLogic_DyingOrDead(const CPedData *self)
{
	// Dying or already dead
	return self->m_nPedState == PED_DIE || self->m_nPedState == PED_DEAD;
}

bool PedLogic_EnteringCar(const CPedData *self)
{
	// Getting into a car or carjacking one
	return self->m_nPedState == PED_ENTER_CAR || self->m_nPedState == PED_CARJACK;
}

bool PedLogic_CanSetPedState(const CPedData *self)
{
	// Not dying, arrested, entering a car or stealing one
	return !PedLogic_DyingOrDead(self) && self->m_nPedState != PED_ARRESTED && !PedLogic_EnteringCar(self) && self->m_nPedState != PED_STEAL_CAR;
}

bool PedLogic_CanStrafeOrMouseControl(const CPedData *self, bool freeCam)
{
	// The free camera takes over the controls
	if (freeCam)
		return false;
	return self->m_nPedState == PED_NONE || self->m_nPedState == PED_IDLE || self->m_nPedState == PED_FLEE_POS || self->m_nPedState == PED_FLEE_ENTITY ||
		self->m_nPedState == PED_ATTACK || self->m_nPedState == PED_FIGHT || self->m_nPedState == PED_AIM_GUN || self->m_nPedState == PED_JUMP || self->m_nPedState == PED_ANSWER_MOBILE;
}

static bool CreatedByAllowsDeletion(const CPedData *self)
{
	// Mission and unknown peds are never deleted automatically
	switch (self->CharCreatedBy) {
		case RANDOM_CHAR:
			return true;
		case MISSION_CHAR:
			return false;
		case UNK_CHAR:
			return false;
		default:
			return true;
	}
}

bool PedLogic_CanBeDeleted(const CPedData *self)
{
	// Peds in vehicles are kept
	if (self->bInVehicle)
		return false;

	return CreatedByAllowsDeletion(self);
}

bool PedLogic_CanBeDeletedEvenInVehicle(const CPedData *self)
{
	// Same rule without the vehicle check
	return CreatedByAllowsDeletion(self);
}

bool PedLogic_IsPlayer(const CPedData *self)
{
	// The unused player types are still in the enum so they are checked as well
	return self->m_nPedType == PEDTYPE_PLAYER1 || self->m_nPedType == PEDTYPE_PLAYER2 ||
		self->m_nPedType == PEDTYPE_PLAYER3 || self->m_nPedType == PEDTYPE_PLAYER4;
}

bool PedLogic_IsGangMember(const CPedData *self)
{
	// Any of the nine gang types
	return self->m_nPedType >= PEDTYPE_GANG1 && self->m_nPedType <= PEDTYPE_GANG9;
}

void PedLogic_SetLookTimer(CPedData *self, uint32_t now, int time)
{
	// Only extend the timer once the previous one ran out
	if (now > self->m_lookTimer) {
		self->m_lookTimer = now + time;
	}
}

void PedLogic_SetAttackTimer(CPedData *self, uint32_t now, uint32_t time)
{
	// The attack starts after the shoot timer too
	if (now > self->m_attackTimer) {
		uint32_t base = self->m_shootTimer > now ? self->m_shootTimer : now;
		self->m_attackTimer = base + time;
	}
}

void PedLogic_SetShootTimer(CPedData *self, uint32_t now, uint32_t time)
{
	// Only extend the timer once the previous one ran out
	if (now > self->m_shootTimer) {
		self->m_shootTimer = now + time;
	}
}

void PedLogic_ClearFollowPath(CPedData *self)
{
	// Forget the path nodes to go to
	int i;
	for (i = 0; i < 8; i++) {
		self->m_pathNodesToGo[i] = NULL;
	}
	self->m_nNumPathNodes = 0;
	self->m_nCurPathNodeId = 0;
}

void PedLogic_RestoreHeadingRate(CPedData *self)
{
	// Back to the heading change rate of the ped stats
	self->m_headingRate = self->m_pedStats->m_headingChangeRate;
}

bool PedLogic_HasAttractor(const CPedData *self)
{
	// An attractor is assigned
	return self->m_attractor != NULL;
}

int PedLogic_GetLocalDirection(const CPedData *self, float offsetX, float offsetY)
{
	int direction;
	float angle;

	// Heading of the offset minus the ped rotation, shifted by 45 degrees and wrapped to a positive angle
	for (angle = atan2f(-offsetX, offsetY) - self->m_fRotationCur + DEGTORAD(45.0f); angle < 0.0f; angle += TWOPI);

	for (direction = RADTODEG(angle) / 90.0f; direction > 3; direction -= 4);

	// 0-forward, 1-left, 2-backward, 3-right.
	return direction;
}

void PedLogic_SetStoredState(CPedData *self)
{
	if (self->m_nLastPedState != PED_NONE || !PedLogic_CanPedReturnToState(self))
		return;

	if (self->m_nPedState == PED_WANDER_PATH) {
		self->bFindNewNodeAfterStateRestore = true;
		if (self->m_nMoveState == PEDMOVE_NONE || self->m_nMoveState == PEDMOVE_STILL)
			self->m_nMoveState = PEDMOVE_WALK;
	}
	if (self->m_nPedState != PED_IDLE) {
		self->m_nLastPedState = self->m_nPedState;
		if (self->m_nMoveState >= self->m_nPrevMoveState)
			self->m_nPrevMoveState = self->m_nMoveState;
	}
}

void PedLogic_SetLookFlagDirection(CPedData *self, uint32_t now, float direction, bool keepTryingToLook, bool cancelPrevious)
{
	if (self->m_lookTimer < now || cancelPrevious) {
		self->bIsLooking = true;
		self->bIsRestoringLook = false;
		self->m_fLookDirection = direction;
		self->m_pLookTarget = NULL;
		self->m_lookTimer = 0;
		self->bKeepTryingToLook = keepTryingToLook;
		if (PedLogic_CanUseTorsoWhenLooking(self)) {
			self->m_pedIK.m_flags &= ~PEDIK_LOOKAROUND_HEAD_ONLY;
		}
	}
}

bool PedLogic_TurnBody(CPedData *self, float posX, float posY)
{
	bool turnDone = true;
	float limitedLookDir, currentRot, neededTurn;

	// A look target overrides the look direction
	if (self->m_pLookTarget) {
		const CMatrixData *t = &((const CEntityStruct *)self->m_pLookTarget)->placeable.m_matrix;
		self->m_fLookDirection = General_GetRadianAngleBetweenPoints(t->px, t->py, posX, posY);
	}

	limitedLookDir = General_LimitRadianAngle(self->m_fLookDirection);
	currentRot = self->m_fRotationCur;

	if (currentRot - PI > limitedLookDir)
		limitedLookDir += 2 * PI;
	else if (PI + currentRot < limitedLookDir)
		limitedLookDir -= 2 * PI;

	neededTurn = currentRot - limitedLookDir;
	self->m_fRotationDest = limitedLookDir;

	if (fabsf(neededTurn) > 0.05f) {
		turnDone = false;
		currentRot -= neededTurn * 0.2f;
	}

	self->m_fRotationCur = currentRot;
	self->m_fLookDirection = limitedLookDir;
	return turnDone;
}

static float DistTo(float px, float py, float pz, const void *ped)
{
	// Distance from the point to the ped position
	const CMatrixData *m = &((const CEntityStruct *)ped)->placeable.m_matrix;
	float dx = px - m->px;
	float dy = py - m->py;
	float dz = pz - m->pz;
	return sqrtf(dx * dx + dy * dy + dz * dz);
}

void PedLogic_SortPedsByDistance(float px, float py, float pz, void **list, int min, int max)
{
	int left, right;
	float middleDist;

	if (min >= max)
		return;

	middleDist = DistTo(px, py, pz, list[(max + min) / 2]);

	left = max;
	right = min;
	while (right <= left) {
		float rightDist, leftDist;
		do {
			rightDist = DistTo(px, py, pz, list[right]);
		} while (middleDist > rightDist && ++right);

		do {
			leftDist = DistTo(px, py, pz, list[left]);
		} while (middleDist < leftDist && left--);

		if (right <= left) {
			void *ped = list[right];
			list[right] = list[left];
			list[left] = ped;
			right++;
			left--;
		}
	}
	PedLogic_SortPedsByDistance(px, py, pz, list, min, left);
	PedLogic_SortPedsByDistance(px, py, pz, list, right, max);
}
int PedLogic_CheckForPointBlankPeds(CPedStruct *self, bool twoHanded, const void *pedToVerify)
{
	const CMatrixData *myMat = &self->base.base.placeable.m_matrix;
	CPedData *me = &self->ped;
	float pbDistance = twoHanded ? 1.6f : 1.1f;
	int i;

	for (i = 0; i < me->m_numNearPeds; i++) {
		const CPedStruct *nearPed = (const CPedStruct*)me->m_nearPeds[i];

		if (!pedToVerify || pedToVerify == (const void*)nearPed) {
			const CMatrixData *nearMat = &nearPed->base.base.placeable.m_matrix;
			float dx = nearMat->px - myMat->px;
			float dy = nearMat->py - myMat->py;
			float dz = nearMat->pz - myMat->pz;

			// Close enough to count
			if (dx*dx + dy*dy + dz*dz < pbDistance * pbDistance) {
				float neededAngle = General_GetRadianAngleBetweenPoints(nearMat->px, nearMat->py, myMat->px, myMat->py);
				float neededTurn;
				int nearPedState = nearPed->ped.m_nPedState;

				neededAngle = General_LimitRadianAngle(neededAngle);
				me->m_fRotationCur = General_LimitRadianAngle(me->m_fRotationCur);

				neededTurn = fabsf(neededAngle - me->m_fRotationCur);
				if (neededTurn > PI)
					neededTurn = 2*PI - neededTurn;

				// A ped that is down or dying is no threat
				if (nearPedState == PED_FALL || nearPedState == PED_GETUP || nearPedState == PED_DIE || nearPedState == PED_DEAD || nearPedState == PED_DIVE_AWAY)
					return NO_POINT_BLANK_PED;

				// In front of the ped within 60 degrees
				if (neededTurn < DEGTORAD(60.0f)) {
					if (pedToVerify == (const void*)nearPed)
						return POINT_BLANK_FOR_WANTED_PED;
					else
						return POINT_BLANK_FOR_SOMEONE_ELSE;
				}
			}
		}
	}
	return NO_POINT_BLANK_PED;
}
void PedLogic_SetObjectiveTimer(CPedData *self, uint32_t now, int time)
{
	// Zero clears the timer, otherwise only extend an expired one
	if (time == 0) {
		self->m_objectiveTimer = 0;
	} else if (now > self->m_objectiveTimer) {
		self->m_objectiveTimer = now + time;
	}
}

bool PedLogic_IsTemporaryObjective(int objective)
{
	// Objectives that are dropped once done
	return objective == OBJECTIVE_LEAVE_CAR || objective == OBJECTIVE_SET_LEADER ||
		objective == OBJECTIVE_LEAVE_CAR_AND_DIE || objective == OBJECTIVE_ENTER_CAR_AS_DRIVER ||
		objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER;
}

bool PedLogic_RestorePreviousObjective(CPedData *self)
{
	if (self->m_objective == OBJECTIVE_NONE)
		return false;

	// Forget the target ped unless the objective is about cars
	if (self->m_objective != OBJECTIVE_LEAVE_CAR && self->m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER && self->m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER
		&& self->m_nPedState != PED_CARJACK)
		self->m_pedInObjective = NULL;

	// Waiting in the car ends with leaving it
	if (self->m_objective == OBJECTIVE_WAIT_IN_CAR_THEN_GET_OUT) {
		self->m_objective = OBJECTIVE_NONE;
		return self->m_pMyVehicle != NULL;
	}

	self->m_objective = self->m_prevObjective;
	self->m_prevObjective = OBJECTIVE_NONE;
	return false;
}

bool PedLogic_HaveReachedNextPointOnRoute(CPedData *self, float posX, float posY, float distToCountReached)
{
	float dx = self->m_nextRoutePointPos.x - posX;
	float dy = self->m_nextRoutePointPos.y - posY;

	// Count the point as passed when close enough
	if (sqrtf(dx*dx + dy*dy) < distToCountReached) {
		self->m_routePointsPassed += self->m_routePointsBeingPassed;
		return true;
	}
	return false;
}

bool PedLogic_CanBeDamagedByThisGangMember(const CPedData *self, uint32_t whoPedType)
{
	// One flag per gang
	return (self->m_gangFlags & (1 << (whoPedType - PEDTYPE_GANG1))) != 0;
}
bool PedLogic_UseGroundColModel(const CPedData *self)
{
	// Lying down or on the way there
	return self->m_nPedState == PED_FALL ||
		self->m_nPedState == PED_DIVE_AWAY ||
		self->m_nPedState == PED_DIE ||
		self->m_nPedState == PED_DEAD;
}

void PedLogic_CheckThreatValidity(CPedData *self, bool threatInvalid, bool eventInvalid)
{
	// Forget entities that no longer exist and the threat flags with them
	if (self->m_threatEntity && threatInvalid) {
		self->m_threatFlags = 0;
		self->m_threatEntity = NULL;
	}
	if (self->m_pEventEntity && eventInvalid) {
		self->m_threatFlags = 0;
		self->m_pEventEntity = NULL;
	}
	if (!self->m_threatEntity && !self->m_pEventEntity)
		self->m_threatFlags = 0;
}

bool PedLogic_CanPedDriveOff(CPedData *self, uint32_t now)
{
	int i;

	// Only drivers that are not busy looking around
	if (self->m_nPedState != PED_DRIVING || self->m_lookTimer > now)
		return false;

	// Wait a second for a ped of the same type that is about to get in
	for (i = 0; i < self->m_numNearPeds; i++) {
		const CPedData *nearPed = &((const CPedStruct*)self->m_nearPeds[i])->ped;
		if (nearPed->m_nPedType == self->m_nPedType && nearPed->m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER && nearPed->m_carInObjective == self->m_carInObjective) {
			self->m_lookTimer = now + 1000;
			return false;
		}
	}
	return true;
}
int PedLogic_GetNextPointOnRoute(CPedData *self)
{
	int16_t nextPoint = self->m_routePointsBeingPassed + self->m_routePointsPassed + self->m_routeStartPoint;

	// Route is complete
	if (nextPoint < 0 || nextPoint > NUMPEDROUTES || self->m_routeLastPoint != PedRoutes_GetRouteThisPointIsOn(nextPoint)) {
		switch (self->m_routeType) {
			case PEDROUTE_STOP_WHEN_DONE:
				nextPoint = -1;
				break;
			case PEDROUTE_GO_BACKWARD_WHEN_DONE:
				self->m_routePointsBeingPassed = -self->m_routePointsBeingPassed;
				nextPoint = self->m_routePointsBeingPassed + self->m_routePointsPassed + self->m_routeStartPoint;
				break;
			case PEDROUTE_GO_TO_START_WHEN_DONE:
				self->m_routePointsPassed = -1;
				nextPoint = self->m_routePointsBeingPassed + self->m_routePointsPassed + self->m_routeStartPoint;
				break;
			default:
				break;
		}
	}
	return nextPoint;
}

void PedLogic_FormationOffset(int formation, float offset, float *out)
{
	// Rear is behind the leader, front in front of it, left and right beside it
	out[0] = 0.0f;
	out[1] = 0.0f;
	out[2] = 0.0f;
	switch (formation) {
		case FORMATION_REAR:
			out[1] = -offset;
			break;
		case FORMATION_REAR_LEFT:
			out[0] = -offset; out[1] = -offset;
			break;
		case FORMATION_REAR_RIGHT:
			out[0] = offset; out[1] = -offset;
			break;
		case FORMATION_FRONT_LEFT:
			out[0] = -offset; out[1] = offset;
			break;
		case FORMATION_FRONT_RIGHT:
			out[0] = offset; out[1] = offset;
			break;
		case FORMATION_LEFT:
			out[0] = -offset;
			break;
		case FORMATION_RIGHT:
			out[0] = offset;
			break;
		case FORMATION_FRONT:
			out[1] = offset;
			break;
		default:
			break;
	}
}
bool PedLogic_CanSeeEntity(float rotation, float targetX, float targetY, float pedX, float pedY, float threshold)
{
	// The direction the thing is in, from 0 to a full turn
	float neededAngle = General_GetRadianAngleBetweenPoints(targetX, targetY, pedX, pedY);
	float ourAngle, neededTurn;

	if (neededAngle < 0.0f)
		neededAngle += TWOPI;
	else if (neededAngle > TWOPI)
		neededAngle -= TWOPI;

	// The direction the ped faces
	ourAngle = rotation;
	if (ourAngle < 0.0f)
		ourAngle += TWOPI;
	else if (ourAngle > TWOPI)
		ourAngle -= TWOPI;

	// Within the threshold on either side of straight ahead
	neededTurn = fabsf(neededAngle - ourAngle);
	return neededTurn < threshold || TWOPI - threshold < neededTurn;
}
bool PedLogic_WillChat(bool roadBetween, int surfaceTouched, bool strangerIsSelf, uint32_t myType, uint32_t strangerType)
{
	bool strangerIsPlayer = strangerType == PEDTYPE_PLAYER1 || strangerType == PEDTYPE_PLAYER2 ||
		strangerType == PEDTYPE_PLAYER3 || strangerType == PEDTYPE_PLAYER4;
	bool iAmGang = myType >= PEDTYPE_GANG1 && myType <= PEDTYPE_GANG9;
	bool strangerGang = strangerType >= PEDTYPE_GANG1 && strangerType <= PEDTYPE_GANG9;

	// No chatting across a road, on the road or with oneself
	if (roadBetween)
		return false;
	if (surfaceTouched == SURFACE_TARMAC)
		return false;
	if (strangerIsSelf)
		return false;

	// Peds of the same kind always chat
	if (myType == strangerType)
		return true;

	// Criminals do not, nor does anybody chat with cops, the player or another gang
	if (myType == PEDTYPE_CRIMINAL)
		return false;
	if (strangerType == PEDTYPE_COP)
		return false;
	if (strangerIsPlayer)
		return false;
	if ((iAmGang || strangerGang) && myType != strangerType)
		return false;
	return true;
}
float PedLogic_WorkOutHeadingFirstPerson(float offset, float leftRight, float upDown, float *angle)
{
	// Walk angle from the stick, or a quarter turn when only sideways is pressed
	if (upDown != 0.0f) {
		*angle = General_GetRadianAngleBetweenPoints(0.0f, 0.0f, -leftRight, upDown);
	} else {
		if (leftRight < 0.0f)
			*angle = HALFPI;
		else if (leftRight > 0.0f)
			*angle = -HALFPI;
	}

	return General_LimitRadianAngle(offset + *angle);
}

int PedLogic_ChooseFistShakeAnim(bool isPlayer, bool handFree, bool targetIsPed, int temper, bool targetIsCop, bool targetIsPolice)
{
	int animToPlay = ANIM_STD_NUM;

	// The player picks by temper, other peds with a free hand shake their fist half of the time
	if (isPlayer && handFree) {
		if (targetIsPed) {
#ifdef FIX_BUGS
			if (temper > 49 || targetIsCop)
#else
			if (temper < 49 || targetIsCop)
#endif
				animToPlay = ANIM_STD_PARTIAL_FUCKU;
			else if (temper < 47)
				animToPlay = ANIM_STD_PARTIAL_PUNCH;
		} else {
			if (temper > 49 || targetIsPolice)
				animToPlay = ANIM_STD_PARTIAL_FUCKU;
		}
	} else if (handFree && (General_GetRandomNumber() & 1)) {
		animToPlay = ANIM_STD_PARTIAL_FUCKU;
	}

	return animToPlay;
}

int PedLogic_PickMostValuableVehicle(const int *values, int count, int *bestValue)
{
	int best = -1;
	int bestMonetaryValue = 0;

	// Keep the first vehicle with the highest value
	for (int i = 0; i < count; i++)
		if (bestMonetaryValue < values[i]) {
			best = i;
			bestMonetaryValue = values[i];
		}

	*bestValue = bestMonetaryValue;
	return best;
}

bool PedLogic_IsMugVictimType(uint32_t pedType)
{
	return pedType == PEDTYPE_CIVFEMALE || pedType == PEDTYPE_CIVMALE || pedType == PEDTYPE_CRIMINAL ||
		pedType == PEDTYPE_UNUSED1 || pedType == PEDTYPE_PROSTITUTE;
}

float PedLogic_TurnTowardsDest(float rotCur, float rotDest, float headAmount)
{
	// Limit both angles then take the shortest way around
	rotCur = General_LimitRadianAngle(rotCur);
	float limitedRotDest = General_LimitRadianAngle(rotDest);

	if (rotCur - PI > limitedRotDest) {
		limitedRotDest += 2 * PI;
	} else if (PI + rotCur < limitedRotDest) {
		limitedRotDest -= 2 * PI;
	}

	// Turn the whole way when it is within reach, otherwise as far as allowed
	float neededTurn = limitedRotDest - rotCur;
	if (neededTurn <= headAmount) {
		if (neededTurn > (-headAmount))
			rotCur += neededTurn;
		else
			rotCur -= headAmount;
	} else {
		rotCur += headAmount;
	}

	return rotCur;
}

float PedLogic_FoldWalkAngle(float localWalkAngle)
{
	// Walking backwards is the same as walking forwards with the legs turned half a turn
	if (localWalkAngle < -0.5f * PI) {
		localWalkAngle += PI;
	} else if (localWalkAngle > 0.5f * PI) {
		localWalkAngle -= PI;
	}

	return localWalkAngle;
}

void PedLogic_SteepCliffVelocityChange(float normalX, float normalY, float movedX, float movedY, float *outX, float *outY)
{
	// Direction of the push, a unit vector along x when there is none
	float rx = normalX, ry = normalY;
	float sq = rx * rx + ry * ry;
	if (sq > 0.0f) {
		float invsqrt = RecipSqrt(sq);
		rx *= invsqrt;
		ry *= invsqrt;
	} else
		rx = 1.0f;

	float vx = 0.02f * rx + movedX;
	float vy = 0.02f * ry + movedY;

	// Remove the part of the movement that goes against the push
	float dot = rx * vx + ry * vy;
	if (dot < 0.0f) {
		vx -= dot * rx;
		vy -= dot * ry;
	}

	*outX = vx;
	*outY = vy;
}

void PedLogic_LimitSurfaceVelocityChange(float *vx, float *vy, bool dieOnVehicle, bool notBoat, float timeStep)
{
	// A dying ped on a vehicle is limited the most, a boat does not limit at all
	float speedChange = Sqrt(*vx * *vx + *vy * *vy);
	float changeMult = speedChange;
	if (dieOnVehicle) {
		changeMult = 0.002f * timeStep;
	} else if (notBoat) {
		changeMult = 0.01f * timeStep;
	}

	if (speedChange > changeMult) {
		*vx = *vx * (changeMult / speedChange);
		*vy = *vy * (changeMult / speedChange);
	}
}

#ifndef GRAVITY
#define GRAVITY (0.008f)
#endif

bool PedLogic_IsBuoyantlySubmerged(float impulseZ, float mass, float timeStep, float waterLevel, float posZ)
{
	return impulseZ / mass > GRAVITY * timeStep || waterLevel > posZ + 0.6f;
}

bool PedLogic_IsBuoyancySlowing(float impulseZ, float mass, float timeStep)
{
	return impulseZ / mass > GRAVITY * 0.25f * timeStep;
}

float PedLogic_WaterSpeedMult(float timeStep)
{
	return powf(0.9f, timeStep);
}

float PedLogic_WaterVerticalSpeed(float speedZ, bool *splash)
{
	*splash = false;

	// Slow fall is damped, a fast one stops and splashes
	if (speedZ >= -0.1f) {
		if (speedZ < -0.04f)
			speedZ = -0.02f;
	} else {
		speedZ = -0.01f;
		*splash = true;
	}

	return speedZ;
}

int PedLogic_NextClumpAlpha(int alpha, bool fadeOut)
{
	if (!fadeOut) {
		if (alpha < 255) {
			alpha += 16;
			if (alpha > 255)
				alpha = 255;
		}
	} else {
		alpha -= 8;
		if (alpha < 0)
			alpha = 0;
	}

	return alpha;
}

float PedLogic_DeadBloodRadius(uint32_t timeSinceDeath)
{
	if (timeSinceDeath >= 2000) {
		if (timeSinceDeath <= 7000)
			return (timeSinceDeath - 2000) / 5000.0f * 0.75f;
		return 0.75f;
	}

	return 0.0f;
}

int PedLogic_NextPanicCounter(int panicCounter, bool resets)
{
	if (resets)
		return 0;
	if (panicCounter < 50)
		return panicCounter + 1;
	return panicCounter;
}

float PedLogic_TurnAwayFromDamage(float damageAngle, float *rotCur, float headingChangeRate)
{
	float angleToFace;

	if (damageAngle < *rotCur)
		damageAngle += TWOPI;

	// Turn the shorter way around towards the damage angle
	float neededTurn = damageAngle - *rotCur;

	if (neededTurn <= PI) {
		angleToFace = 0.5f * neededTurn + *rotCur;
		*rotCur += DEGTORAD(headingChangeRate) * 2.0f;
	} else {
		angleToFace = *rotCur - (TWOPI - neededTurn) * 0.5f;
		*rotCur -= DEGTORAD(headingChangeRate) * 2.0f;
	}

	return angleToFace;
}

float PedLogic_WrapDestRotation(float rotDest, float rotCur)
{
	if (rotCur - PI > rotDest) {
		rotDest += TWOPI;
	} else if (PI + rotCur < rotDest) {
		rotDest -= TWOPI;
	}

	return rotDest;
}

VectorMath3 PedLogic_CarDoorLocalPosition(uint32_t component, bool isVan, bool lowVehicle, float seatOffsetDistance, float seatPosMult, VectorMath3 frontSeat, VectorMath3 backSeat, VectorMath3 vanRearOffset, VectorMath3 lowOffset, VectorMath3 normalOffset)
{
	VectorMath3 doorPos, doorOffset, out;
	float seatOffset;

	// Rear doors of vans have their own animation offset and no seat offset
	if (isVan && (component == CAR_DOOR_LR || component == CAR_DOOR_RR)) {
		seatOffset = 0.0f;
		doorOffset = vanRearOffset;
	} else {
		seatOffset = seatOffsetDistance * seatPosMult;
		if (lowVehicle)
			doorOffset = lowOffset;
		else
			doorOffset = normalOffset;
	}

	// Doors on the left mirror the position of the seat
	switch (component) {
	case CAR_DOOR_RF:
		doorPos = frontSeat;
		doorPos.x += seatOffset;
		doorOffset.x = -doorOffset.x;
		break;

	case CAR_DOOR_RR:
		doorPos = backSeat;
		doorPos.x += seatOffset;
		doorOffset.x = -doorOffset.x;
		break;

	case CAR_DOOR_LF:
		doorPos = frontSeat;
		doorPos.x = -(doorPos.x + seatOffset);
		break;

	case CAR_DOOR_LR:
		doorPos = backSeat;
		doorPos.x = -(doorPos.x + seatOffset);
		break;

	default:
		doorPos = frontSeat;
		doorOffset.x = 0.0f;
		doorOffset.y = 0.0f;
		doorOffset.z = 0.0f;
	}

	out.x = doorPos.x - doorOffset.x;
	out.y = doorPos.y - doorOffset.y;
	out.z = doorPos.z - doorOffset.z;
	return out;
}

int PedLogic_NearestDoor(float rfX, float rfY, float lrX, float lrY, float rrX, float rrY)
{
	int door = 0;
	float next = rfX * rfX + rfY * rfY;
	float lr = lrX * lrX + lrY * lrY;
	float rr = rrX * rrX + rrY * rrY;

	// The front right door wins ties
	if (lr < next) {
		door = 1;
		next = lr;
	}

	if (rr < next)
		door = 2;

	return door;
}

/* a * v + b, with the multiplication done first as in the vector operators */
static VectorMath3 ExitMulAdd(float a, VectorMath3 v, VectorMath3 b)
{
	VectorMath3 r;

	r.x = a * v.x + b.x;
	r.y = a * v.y + b.y;
	r.z = a * v.z + b.z;
	return r;
}

bool PedLogic_ExitCandidate(int index, const PedExitInput *in, VectorMath3 *out)
{
	VectorMath3 p;

	switch (index) {
	case 0:
		// The top of a vehicle on its side or the usual way out of the door
		if (in->onItsSide) {
			p = in->vehPos;
			p.z = in->feetOffset + in->boundsMax.x + in->vehPos.z;
		} else if (in->doorValid) {
			p = in->doorPos;
		} else
			return false;
		break;
	case 1:
	{
		// The side of the vehicle, in line with the seat
		VectorMath3 d = { in->ourPos.x - in->vehPos.x, in->ourPos.y - in->vehPos.y, in->ourPos.z - in->vehPos.z };
		float exitOffset = in->exitX - (d.x * in->right.x + d.y * in->right.y + d.z * in->right.z);

		p = ExitMulAdd(exitOffset, in->right, in->ourPos);
		break;
	}
	case 2: case 3: case 4: case 5:
	{
		// Sections along the length of the vehicle
		float minY = in->boundsMin.y;
		float ySection = (in->boundsMax.y - minY) / 3.f;
		float fwdMult = (index - 2) * ySection + minY;
		VectorMath3 a = { in->exitX * in->right.x, in->exitX * in->right.y, in->exitX * in->right.z };

		p.x = (a.x + fwdMult * in->forward.x) + in->vehPos.x;
		p.y = (a.y + fwdMult * in->forward.y) + in->vehPos.y;
		p.z = (a.z + fwdMult * in->forward.z) + in->vehPos.z;
		break;
	}
	case 6:
		// The back
		p = ExitMulAdd(in->boundsMin.y - 0.355f, in->forward, in->vehPos);
		break;
	case 7:
		// The front
		p = ExitMulAdd(0.355f + in->boundsMax.y, in->forward, in->vehPos);
		break;
	case 8:
	case 9:
	{
		// The opposite side at the back or the front
		float y = index == 8 ? in->boundsMin.y : in->boundsMax.y;

		p = ExitMulAdd(y, in->forward, in->vehPos);
		p.x -= in->exitX * in->right.x;
		p.y -= in->exitX * in->right.y;
		p.z -= in->exitX * in->right.z;
		break;
	}
	case 10:
		// The top, only for cars
		if (!in->isCarType)
			return false;
		p = ExitMulAdd(in->boundsMax.z, in->up, in->vehPos);
		p.z += in->feetOffset;
		break;
	default:
		return false;
	}

	*out = p;
	return true;
}

float PedLogic_CarEntryRotation(uint32_t vehDoor, bool vehIsUpsideDown, bool vehIsBus, float forwardHeading)
{
	if (vehDoor == CAR_DOOR_RF || vehDoor == CAR_DOOR_RR) {
		if (vehIsUpsideDown)
			return -PI + forwardHeading;
		if (vehIsBus)
			return 0.5f * PI + forwardHeading;
		return forwardHeading;
	} else if (vehDoor == CAR_DOOR_LF || vehDoor == CAR_DOOR_LR) {
		if (vehIsUpsideDown)
			return forwardHeading;
		if (vehIsBus)
			return -0.5f * PI + forwardHeading;
		return forwardHeading;
	}

	// Other doors only happen for things like boats
	if (vehIsUpsideDown)
		return forwardHeading;
	if (vehIsBus)
		return 0.5f * PI + forwardHeading;
	if (vehDoor == CAR_WINDSCREEN)
		return forwardHeading + PI;
	return forwardHeading;
}

void PedLogic_CarSeatPosMult(const PedSeatPosInput *in, PedSeatPosResult *out)
{
	float zBlend = 0.0f;
	bool multExtractedFromAnim = false;
	bool multExtractedFromAnimBus = false;
	float seatPosMult = 0.0f;

	if (in->hasAnim) {
		switch (in->animId) {
		case ANIM_STD_JACKEDCAR_RHS:
		case ANIM_STD_JACKEDCAR_LO_RHS:
		case ANIM_STD_JACKEDCAR_LHS:
		case ANIM_STD_JACKEDCAR_LO_LHS:
		case ANIM_STD_VAN_GET_IN_REAR_LHS:
		case ANIM_STD_VAN_GET_IN_REAR_RHS:
			multExtractedFromAnim = true;
			zBlend = ((in->progress - 0.3f) > 0.0f ? (in->progress - 0.3f) : 0.0f) / (1.0f - 0.3f);
			/* fall through */

		case ANIM_STD_QUICKJACKED:
		case ANIM_STD_GETOUT_LHS:
		case ANIM_STD_GETOUT_LO_LHS:
		case ANIM_STD_GETOUT_RHS:
		case ANIM_STD_GETOUT_LO_RHS:
			if (!multExtractedFromAnim) {
				multExtractedFromAnim = true;
				zBlend = ((in->progress - 0.5f) > 0.0f ? (in->progress - 0.5f) : 0.0f) / (1.0f - 0.5f);
			}
			/* fall through */

		case ANIM_STD_CRAWLOUT_LHS:
		case ANIM_STD_CRAWLOUT_RHS:
		case ANIM_STD_VAN_GET_OUT_REAR_LHS:
		case ANIM_STD_VAN_GET_OUT_REAR_RHS:
		case ANIM_BIKE_GETOFF_LHS:
		case ANIM_BIKE_GETOFF_RHS:
			seatPosMult = in->progress;
			break;
		case ANIM_STD_CAR_GET_IN_RHS:
		case ANIM_STD_CAR_GET_IN_LHS:
			if (in->vehIsBus) {
				multExtractedFromAnimBus = true;
				zBlend = (in->progress < 0.5f ? in->progress : 0.5f) / 0.5f;
			}
			/* fall through */

		case ANIM_STD_QUICKJACK:
		case ANIM_STD_CAR_GET_IN_LO_LHS:
		case ANIM_STD_CAR_GET_IN_LO_RHS:
		case ANIM_STD_BOAT_DRIVE:
			seatPosMult = in->timeLeft / in->totalLength;
			break;
		case ANIM_STD_CAR_CLOSE_DOOR_LHS:
		case ANIM_STD_CAR_CLOSE_DOOR_LO_LHS:
		case ANIM_STD_CAR_CLOSE_DOOR_RHS:
		case ANIM_STD_CAR_CLOSE_DOOR_LO_RHS:
		case ANIM_STD_CAR_SHUFFLE_RHS:
		case ANIM_STD_CAR_SHUFFLE_LO_RHS:
			seatPosMult = 0.0f;
			break;
		case ANIM_STD_CAR_JUMP_IN_LO_LHS:
		{
			float animLength = in->totalLength;
			float left = 0.5f * animLength - in->currentTime;

			seatPosMult = (0.0f > left ? 0.0f : left) / animLength;
			break;
		}
		case ANIM_STD_CAR_CLOSE_LHS:
		case ANIM_STD_CAR_CLOSE_RHS:
		case ANIM_STD_COACH_OPEN_LHS:
		case ANIM_STD_COACH_OPEN_RHS:
		case ANIM_STD_COACH_GET_IN_LHS:
		case ANIM_STD_COACH_GET_IN_RHS:
		case ANIM_STD_COACH_GET_OUT_LHS:
			seatPosMult = 1.0f;
			break;
		default:
			if (in->vehIsBike)
				seatPosMult = 0.0f;
			else if (in->inVehicle)
				seatPosMult = 0.0f;
			else
				seatPosMult = 1.0f;
			break;
		}
	} else {
		if (in->vehIsBike)
			seatPosMult = 0.0f;
		else if (in->inVehicle)
			seatPosMult = 0.0f;
		else
			seatPosMult = 1.0f;
	}

	out->seatPosMult = seatPosMult;
	out->zBlend = zBlend;
	out->multExtractedFromAnim = multExtractedFromAnim;
	out->multExtractedFromAnimBus = multExtractedFromAnimBus;
}

uint32_t PedLogic_DefaultExitDoor(bool isBike, bool canJumpOut, bool isBus, bool isDriver, int slot)
{
	uint32_t optedDoorNode = CAR_DOOR_LF;

	if (isBike) {
		if (canJumpOut)
			optedDoorNode = slot == 0 ? CAR_BUMP_REAR : CAR_BOOT;
		else if (slot == 0)
			optedDoorNode = CAR_DOOR_LR;
		else
			optedDoorNode = CAR_DOOR_LF;
	} else if (isBus) {
		optedDoorNode = CAR_DOOR_LF;
	} else if (isDriver) {
		optedDoorNode = CAR_DOOR_LF;
	} else if (slot == 0) {
		optedDoorNode = CAR_DOOR_RF;
	} else if (slot == 1) {
		optedDoorNode = CAR_DOOR_LR;
	} else if (slot == 2) {
		optedDoorNode = CAR_DOOR_RR;
	} else if (slot >= 3) {
		// Further seats alternate between the right and left rear doors
		if (slot & 1)
			optedDoorNode = CAR_DOOR_RR;
		else
			optedDoorNode = CAR_DOOR_LR;
	}

	return optedDoorNode;
}

void PedLogic_ExitDoorBusy(bool isBike, uint32_t doorNode, uint32_t gettingInFlags, uint32_t gettingOutFlags, bool *entersFromDoor, bool *exitsFromDoor)
{
	*entersFromDoor = false;
	*exitsFromDoor = false;

	if (isBike) {
		switch (doorNode) {
		case CAR_BUMP_REAR:
		case CAR_DOOR_RR:
		case CAR_DOOR_LR:
			if (gettingInFlags & (CAR_DOOR_FLAG_LR | CAR_DOOR_FLAG_RR))
				*entersFromDoor = true;
			break;
		case CAR_DOOR_RF:
		case CAR_DOOR_LF:
		case CAR_BOOT:
			if (gettingInFlags & (CAR_DOOR_FLAG_LF | CAR_DOOR_FLAG_RF))
				*entersFromDoor = true;
			break;
		default:
			break;
		}
	} else {
		uint32_t flag;

		switch (doorNode) {
		case CAR_DOOR_RF: flag = CAR_DOOR_FLAG_RF; break;
		case CAR_DOOR_RR: flag = CAR_DOOR_FLAG_RR; break;
		case CAR_DOOR_LF: flag = CAR_DOOR_FLAG_LF; break;
		case CAR_DOOR_LR: flag = CAR_DOOR_FLAG_LR; break;
		default: return;
		}

		if (gettingInFlags & flag)
			*entersFromDoor = true;
		if (gettingOutFlags & flag)
			*exitsFromDoor = true;
	}
}

bool PedLogic_CarExitAnim(uint32_t door, bool canJumpOut, bool isBus, bool isVan, bool isLow, int32_t *group, int32_t *anim)
{
	switch (door) {
	case CAR_DOOR_RF:
		if (canJumpOut) {
			*group = ASSOCGRP_STD; *anim = ANIM_STD_ROLLOUT_RHS;
		} else if (isBus) {
			*group = ASSOCGRP_COACH; *anim = ANIM_STD_COACH_GET_OUT_LHS;
		} else {
			*group = ASSOCGRP_STD; *anim = isLow ? ANIM_STD_GETOUT_LO_RHS : ANIM_STD_GETOUT_RHS;
		}
		return true;
	case CAR_DOOR_RR:
		if (canJumpOut) {
			*group = ASSOCGRP_STD; *anim = ANIM_STD_ROLLOUT_RHS;
		} else if (isVan) {
			*group = ASSOCGRP_VAN; *anim = ANIM_STD_VAN_GET_OUT_REAR_RHS;
		} else if (isLow) {
			*group = ASSOCGRP_STD; *anim = ANIM_STD_GETOUT_LO_RHS;
		} else {
			*group = ASSOCGRP_STD; *anim = ANIM_STD_GETOUT_RHS;
		}
		return true;
	case CAR_DOOR_LF:
		if (canJumpOut) {
			*group = ASSOCGRP_STD; *anim = ANIM_STD_ROLLOUT_LHS;
		} else if (isBus) {
			*group = ASSOCGRP_COACH; *anim = ANIM_STD_COACH_GET_OUT_LHS;
		} else {
			*group = ASSOCGRP_STD; *anim = isLow ? ANIM_STD_GETOUT_LO_LHS : ANIM_STD_GETOUT_LHS;
		}
		return true;
	case CAR_DOOR_LR:
		if (canJumpOut) {
			*group = ASSOCGRP_STD; *anim = ANIM_STD_ROLLOUT_LHS;
		} else if (isVan) {
			*group = ASSOCGRP_VAN; *anim = ANIM_STD_VAN_GET_OUT_REAR_LHS;
		} else if (isLow) {
			*group = ASSOCGRP_STD; *anim = ANIM_STD_GETOUT_LO_LHS;
		} else {
			*group = ASSOCGRP_STD; *anim = ANIM_STD_GETOUT_LHS;
		}
		return true;
	default:
		return false;
	}
}

bool PedLogic_BikeExitAnim(uint32_t door, int32_t *anim)
{
	switch (door) {
	case CAR_BUMP_REAR:
	case CAR_BOOT:
		*anim = ANIM_BIKE_GETOFF_BACK;
		return true;
	case CAR_DOOR_RF:
	case CAR_DOOR_RR:
		*anim = ANIM_BIKE_GETOFF_RHS;
		return true;
	case CAR_DOOR_LF:
	case CAR_DOOR_LR:
		*anim = ANIM_BIKE_GETOFF_LHS;
		return true;
	default:
		return false;
	}
}

int PedLogic_BikeExitFlags(uint32_t door)
{
	switch (door) {
	case CAR_BUMP_REAR:
	case CAR_DOOR_RR:
	case CAR_DOOR_LR:
		return CAR_DOOR_FLAG_RR | CAR_DOOR_FLAG_LR;
	case CAR_DOOR_RF:
	case CAR_DOOR_LF:
	case CAR_BOOT:
		return CAR_DOOR_FLAG_RF | CAR_DOOR_FLAG_LF;
	default:
		return 0;
	}
}

uint32_t PedLogic_MirrorDoor(uint32_t door)
{
	switch (door) {
	case CAR_DOOR_RF: return CAR_DOOR_LF;
	case CAR_DOOR_RR: return CAR_DOOR_LR;
	case CAR_DOOR_LF: return CAR_DOOR_RF;
	case CAR_DOOR_LR: return CAR_DOOR_RR;
	default: return door;
	}
}

int PedLogic_PadIndexForType(uint32_t pedType)
{
	switch (pedType) {
	case PEDTYPE_PLAYER1:
		return 0;
	case PEDTYPE_PLAYER2:
		return 1;
	case PEDTYPE_PLAYER3:
		return 2;
	case PEDTYPE_PLAYER4:
		return 3;
	default:
		return 0;
	}
}

uint32_t PedLogic_TargetCarDoor(bool isDriverOrBus, int slot)
{
	if (isDriverOrBus)
		return CAR_DOOR_LF;
	if (slot == 0)
		return CAR_DOOR_RF;
	if (slot == 1)
		return CAR_DOOR_LR;
	if (slot == 2)
		return CAR_DOOR_RR;
	return 0;
}

void PedLogic_CarJackDoor(bool isBike, uint32_t vehDoor, bool hasFrontPassenger, bool isCop, PedCarJackDoor *out)
{
	out->doorFlag = CAR_DOOR_FLAG_UNKNOWN;
	out->door = 0;
	out->seat = 0;

	if (isBike) {
		// Both doors of a side of a bike are used together
		switch (vehDoor) {
		case CAR_DOOR_RF:
			out->doorFlag = CAR_DOOR_FLAG_LF | CAR_DOOR_FLAG_RF;
			out->door = DOOR_FRONT_RIGHT;
			out->seat = 1;
			break;
		case CAR_DOOR_RR:
			out->doorFlag = CAR_DOOR_FLAG_LR | CAR_DOOR_FLAG_RR;
			out->door = DOOR_REAR_RIGHT;
			out->seat = 2;
			break;
		case CAR_DOOR_LF:
		case CAR_WINDSCREEN:
			out->doorFlag = CAR_DOOR_FLAG_LF | CAR_DOOR_FLAG_RF;
			out->door = DOOR_FRONT_LEFT;
			out->seat = 1;
			break;
		case CAR_DOOR_LR:
			out->doorFlag = CAR_DOOR_FLAG_LR | CAR_DOOR_FLAG_RR;
			out->door = DOOR_REAR_LEFT;
			out->seat = 2;
			break;
		default:
			break;
		}
	} else {
		switch (vehDoor) {
		case CAR_DOOR_RF:
			out->doorFlag = CAR_DOOR_FLAG_RF;
			out->door = DOOR_FRONT_RIGHT;
			if (hasFrontPassenger)
				out->seat = 2;
			else if (isCop)
				out->seat = 1;
			break;
		case CAR_DOOR_RR:
			out->doorFlag = CAR_DOOR_FLAG_RR;
			out->door = DOOR_REAR_RIGHT;
			out->seat = 4;
			break;
		case CAR_DOOR_LF:
			out->doorFlag = CAR_DOOR_FLAG_LF;
			out->door = DOOR_FRONT_LEFT;
			out->seat = 1;
			break;
		case CAR_DOOR_LR:
			out->doorFlag = CAR_DOOR_FLAG_LR;
			out->door = DOOR_REAR_LEFT;
			out->seat = 3;
			break;
		default:
			break;
		}
	}
}

void PedLogic_EnterCarDoor(bool isBike, uint32_t vehDoor, int numMaxPassengers, int *doorFlag, int *door)
{
	*doorFlag = CAR_DOOR_FLAG_UNKNOWN;
	*door = 0;

	if (isBike) {
		switch (vehDoor) {
		case CAR_DOOR_RF:
			*doorFlag = CAR_DOOR_FLAG_RF | CAR_DOOR_FLAG_LF;
			*door = DOOR_FRONT_RIGHT;
			break;
		case CAR_DOOR_RR:
			*doorFlag = CAR_DOOR_FLAG_RR | CAR_DOOR_FLAG_LR;
			*door = DOOR_REAR_RIGHT;
			break;
		case CAR_DOOR_LF:
		case CAR_WINDSCREEN:
			*doorFlag = CAR_DOOR_FLAG_RF | CAR_DOOR_FLAG_LF;
			*door = DOOR_FRONT_LEFT;
			break;
		case CAR_DOOR_LR:
			*doorFlag = CAR_DOOR_FLAG_RR | CAR_DOOR_FLAG_LR;
			*door = DOOR_REAR_LEFT;
			break;
		default:
			break;
		}
	} else {
		switch (vehDoor) {
		case CAR_DOOR_RF:
			*doorFlag = CAR_DOOR_FLAG_RF;
			*door = DOOR_FRONT_RIGHT;
			break;
		case CAR_DOOR_RR:
			*doorFlag = CAR_DOOR_FLAG_RR;
			*door = DOOR_REAR_RIGHT;
			break;
		case CAR_DOOR_LF:
			if (numMaxPassengers != 0)
				*doorFlag = CAR_DOOR_FLAG_LF;
			else
				*doorFlag = CAR_DOOR_FLAG_LF | CAR_DOOR_FLAG_LR;
			*door = DOOR_FRONT_LEFT;
			break;
		case CAR_DOOR_LR:
			if (numMaxPassengers != 0)
				*doorFlag = CAR_DOOR_FLAG_LR;
			else
				*doorFlag = CAR_DOOR_FLAG_LF | CAR_DOOR_FLAG_LR;
			*door = DOOR_REAR_LEFT;
			break;
		default:
			break;
		}
	}
}

int PedLogic_DoorFromNode(uint32_t node)
{
	switch (node) {
	case CAR_DOOR_RF: return DOOR_FRONT_RIGHT;
	case CAR_DOOR_RR: return DOOR_REAR_RIGHT;
	case CAR_DOOR_LF: return DOOR_FRONT_LEFT;
	case CAR_DOOR_LR: return DOOR_REAR_LEFT;
	default: return 0;
	}
}

void PedLogic_MuggingPickups(int money, int *pickupCount, int *moneyPerPickup)
{
	// One pickup for every 40 and a bit, sharing the money out evenly
	*pickupCount = money / 40 + 1;
	*moneyPerPickup = money / *pickupCount;
}

bool PedLogic_IgnoresPointGun(bool isPlayer, bool inControl, bool missionCharNotCrouching, bool leaderIsGunman, bool alreadyHandsUpOrCowering, float distSq2D, bool gang7AndPlayerGunman)
{
	if (isPlayer || !inControl || missionCharNotCrouching)
		return true;

	if (leaderIsGunman)
		return true;

	return alreadyHandsUpOrCowering || distSq2D > 225.0f || gang7AndPlayerGunman;
}

int PedLogic_ReactToAttack(const PedReactToAttackInput *in, bool *registerThreat, bool *sayFleeRun)
{
	*registerThreat = false;
	*sayFleeRun = false;

	// A player that is attacked by a ped tells its gang and looks at the attacker
	if (in->isPlayer && in->attackerIsPed)
		return PED_REACT_INFORM_GANG_AND_LOOK;

	if (in->isGang7 && in->attackerIsPed && in->attackerIsPlayer) {
		if (!in->pedStateIsFall)
			return PED_REACT_GANG7_FALL;
		return PED_REACT_NONE;
	} else if (in->driverBranch) {
		return in->driverSwitchApplies ? PED_REACT_DRIVER_SWITCH_PHYSICS : PED_REACT_NONE;
	} else if (in->controllable && in->respondsToThreats) {
		if (in->notFollowingLeader && in->attackerIsPed) {
			if (in->notAllowedToDuck) {
				if (!in->attackerMelee)
					return PED_REACT_COVER_TIMER;
			} else if (in->crouchOrStay) {
				return PED_REACT_DUCK;
			}

			if (in->stripper)
				return PED_REACT_STRIPPER_KILL;

			if (in->fearOk) {
				if (in->statsDiffer) {
					*registerThreat = in->gangOrEmergency;
					if (!in->attackerMelee && in->selfMelee)
						return PED_REACT_FLEE_FROM_RANGED;
					return PED_REACT_KILL;
				}
				return PED_REACT_NONE;
			}

			*sayFleeRun = in->attackerMelee;
			return PED_REACT_FLEE_SCARED;
		}
	}

	return PED_REACT_NONE;
}

bool PedLogic_ThreatIgnored(bool missionFriendAttackedByPlayer, bool attackerIsLeader, bool gang7AttackedByPlayer)
{
	return missionFriendAttackedByPlayer || attackerIsLeader || gang7AttackedByPlayer;
}

bool PedLogic_DriverRamsPlayer(bool notMissionVehicle, bool hasDriver, bool driverIsOther, bool sameType, bool driverIsRandom, bool normalCar)
{
	return notMissionVehicle && hasDriver && driverIsOther && sameType && driverIsRandom && normalCar;
}

int PedLogic_FollowLeaderAction(int leaderObjective)
{
	switch (leaderObjective) {
	case OBJECTIVE_WAIT_ON_FOOT:
	case OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE:
	case OBJECTIVE_WAIT_IN_CAR:
	case OBJECTIVE_FOLLOW_ROUTE:
		return LEADER_FOLLOW_COPY_OBJECTIVE;
	case OBJECTIVE_GUARD_SPOT:
		return LEADER_FOLLOW_GUARD_SPOT;
	case OBJECTIVE_KILL_CHAR_ON_FOOT:
	case OBJECTIVE_KILL_CHAR_ANY_MEANS:
	case OBJECTIVE_GOTO_CHAR_ON_FOOT:
	case OBJECTIVE_GOTO_CHAR_ON_FOOT_WALKING:
	case OBJECTIVE_HASSLE_CHAR:
		return LEADER_FOLLOW_COPY_TARGET_PED;
	case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
	case OBJECTIVE_ENTER_CAR_AS_DRIVER:
		return LEADER_FOLLOW_ENTER_CAR;
	case OBJECTIVE_GUARD_ATTACK:
		return LEADER_FOLLOW_RETURN;
	case OBJECTIVE_HAIL_TAXI:
		return LEADER_FOLLOW_DROP_LEADER;
	default:
		return LEADER_FOLLOW_GOTO_LEADER;
	}
}

bool PedLogic_FollowerLeavesCar(int leaderObjective, bool carIsLeadersOrTarget)
{
	if ((leaderObjective == OBJECTIVE_ENTER_CAR_AS_PASSENGER || leaderObjective == OBJECTIVE_ENTER_CAR_AS_DRIVER) && carIsLeadersOrTarget)
		return false;
	return true;
}

bool PedLogic_SetObjectiveIgnored(const PedSetObjectiveInput *in)
{
	const int newObj = in->newObjective;

	if (in->dyingOrDead)
		return true;

	if (in->prevObjective == newObj && in->prevObjective != OBJECTIVE_NONE)
		return true;

	if (in->entityIsSelf)
		return true;

	if (in->attached && newObj != OBJECTIVE_KILL_CHAR_ON_FOOT && newObj != OBJECTIVE_KILL_CHAR_ANY_MEANS && newObj != OBJECTIVE_DESTROY_OBJECT && newObj != OBJECTIVE_DESTROY_CAR)
		return true;

	if (in->objective == newObj) {
		switch (newObj) {
		case OBJECTIVE_KILL_CHAR_ON_FOOT:
		case OBJECTIVE_KILL_CHAR_ANY_MEANS:
		case OBJECTIVE_GOTO_CHAR_ON_FOOT:
		case OBJECTIVE_GOTO_CHAR_ON_FOOT_WALKING:
		case OBJECTIVE_HASSLE_CHAR:
		case OBJECTIVE_FOLLOW_CHAR_IN_FORMATION:
		case OBJECTIVE_GOTO_AREA_ANY_MEANS:
		case OBJECTIVE_GUARD_ATTACK:
		case OBJECTIVE_KILL_CHAR_ON_BOAT:
		case OBJECTIVE_SOLICIT_FOOT:
			return in->pedInObjectiveIsEntity;
		case OBJECTIVE_LEAVE_CAR:
		case OBJECTIVE_FLEE_CAR:
		case OBJECTIVE_LEAVE_CAR_AND_DIE:
			return true;
		case OBJECTIVE_ENTER_CAR_AS_PASSENGER:
		case OBJECTIVE_ENTER_CAR_AS_DRIVER:
		case OBJECTIVE_DESTROY_CAR:
		case OBJECTIVE_SOLICIT_VEHICLE:
		case OBJECTIVE_BUY_ICE_CREAM:
			if (in->carInObjectiveIsEntity)
				return true;
			return newObj == OBJECTIVE_BUY_ICE_CREAM && in->boughtIceCream;
		case OBJECTIVE_SET_LEADER:
			return in->leaderIsEntity;
		case OBJECTIVE_AIM_GUN_AT:
			return in->pedInObjectiveIsEntity;
		default:
			break;
		}
	} else {
		if (newObj != OBJECTIVE_WAIT_IN_CAR_THEN_GET_OUT && (newObj == OBJECTIVE_LEAVE_CAR || newObj == OBJECTIVE_LEAVE_CAR_AND_DIE)
			&& !in->inVehicle)
			return true;
	}

	return false;
}

void PedLogic_CoverOffsets(int numPedsUsingCover, VectorMath3 *right, VectorMath3 *left)
{
	// Without a known count there is no offset
	right->x = right->y = right->z = 0.0f;
	left->x = left->y = left->z = 0.0f;

	// 3 peds can use the car as cover, each one gets a different spot
	if (numPedsUsingCover == 2) {
		right->x = 1.5f; right->y = -0.5f;
		left->x = -1.5f; left->y = -0.5f;
	} else if (numPedsUsingCover == 1) {
		right->x = 1.5f; right->y = 0.5f;
		left->x = -1.5f; left->y = 0.5f;
	} else if (numPedsUsingCover == 0) {
		right->x = 1.5f; right->y = 0.0f;
		left->x = -1.5f; left->y = 0.0f;
	}
}

bool PedLogic_CoverCarEligible(bool isCar, float speed, bool isBus, bool isVan, bool isBig, int numPedsUsingCover)
{
	return isCar && speed <= 0.02f && !isBus && !isVan && !isBig && numPedsUsingCover < 3;
}

bool PedLogic_SeekNotAtSafePos(VectorMath3 seekPos, VectorMath3 safePos)
{
	return seekPos.x != safePos.x && seekPos.y != safePos.y && seekPos.z != safePos.z;
}

bool PedLogic_CoverPrefersLeft(float distSqRight, float distSqLeft)
{
	return distSqRight <= distSqLeft;
}

bool PedLogic_NoPoliceZoneKill(bool noPoliceZone, bool victimOnOtherSurface, float distToVictim)
{
	return noPoliceZone || (victimOnOtherSurface && distToVictim < 5.0f);
}

bool PedLogic_MeleeVictimIgnored(bool isCop, bool ignoredByCops, bool ignoredByEveryone, bool victimInWater, bool victimArrested)
{
	return (isCop && ignoredByCops) || ignoredByEveryone || victimInWater || victimArrested;
}

float PedLogic_MeleeWeaponRange(float weaponRange, bool unarmed, bool isPlayer, bool canKick)
{
	float wepRange = weaponRange / 2.0f;

	if (unarmed && !isPlayer && !canKick)
		wepRange -= 0.3f;

	return wepRange;
}

bool PedLogic_MeleeGoesForward(VectorMath3 seekPos, VectorMath3 victimPos, VectorMath3 ourPos, float wepRange)
{
	// How far the attack position is from the victim in units of weapon range
	VectorMath3 victimFarness;
	victimFarness.x = (seekPos.x - victimPos.x) / wepRange;
	victimFarness.y = (seekPos.y - victimPos.y) / wepRange;
	victimFarness.z = (seekPos.z - victimPos.z) / wepRange;

	// Direction from the victim to the ped
	VectorMath3 distVec;
	distVec.x = ourPos.x - victimPos.x;
	distVec.y = ourPos.y - victimPos.y;
	distVec.z = ourPos.z - victimPos.z;
	float distSqr = distVec.x*distVec.x + distVec.y*distVec.y + distVec.z*distVec.z;

	if (wepRange*wepRange > distSqr && distSqr > 0.05f) {
		VectorMath_Normalise(&distVec);
		return victimFarness.x*distVec.x + victimFarness.y*distVec.y > cosf(30.0f * (float)3.14159265358979323846 / 180.0f);
	}
	return false;
}

float PedLogic_CopMaxArrestDist(bool dragsPlayer, bool stateFall, bool stateDragFromCar, bool *clearDrag)
{
	float maxArrestDist = 1.5f;
	*clearDrag = false;

	if (dragsPlayer) {
		if (stateFall)
			maxArrestDist = 3.5f;
		else if (!stateDragFromCar)
			*clearDrag = true;
	}
	return maxArrestDist;
}

bool PedLogic_MeleeCanStrikeNow(uint32_t attackTimer, uint32_t now, float distToVictim, float wepRange, bool victimGettingUp, bool victimDraggedFromCar)
{
	return attackTimer < now && distToVictim < wepRange && !victimGettingUp && !victimDraggedFromCar;
}

float PedLogic_ObstacleSeekRange(bool isCop, bool strongWeapon, bool fleeFromObject, bool fleeFromVehicle)
{
	if (!isCop)
		return 2.0f;

	if (strongWeapon || fleeFromObject)
		return 6.0f;
	if (fleeFromVehicle)
		return 4.0f;
	return 2.0f;
}

bool PedLogic_ArmedMustApproach(float distToVictim, float wepRange, uint32_t victimGetUpTimer, uint32_t now, bool victimArrested, bool victimEnteringCar)
{
	return distToVictim > wepRange || victimGetUpTimer > now || victimArrested || (victimEnteringCar && distToVictim < 3.0f);
}

bool PedLogic_ArmedCanShootVehicle(uint32_t attackTimer, uint32_t now, float distToVictim, float wepRange)
{
	return attackTimer < now && distToVictim < wepRange && distToVictim > 3.0f;
}

bool PedLogic_FleesFromPoliceHeat(bool victimIsPlayer, bool isCop, bool isMissionChar, int currentCops, bool staysInPlace)
{
	return victimIsPlayer && !isMissionChar && !isCop && currentCops != 0 && !staysInPlace;
}

float PedLogic_PoliceChaseSpeedFactor(int wantedLevel)
{
	return wantedLevel * 0.1f + 0.6f;
}

float PedLogic_BikeLeanUD(bool isPlayer, float targetUDLean, float targetLRLean, float leanInput, const float wheelTimer[4], float forwardZ, float stoppieAng, float wheelieAng,
	float brakePedal, float gasPedal, float velocityFwd, float maxCruiseVelocity, bool *setWheelieCam, bool *wheelieCam)
{
	*setWheelieCam = false;

	if (!isPlayer)
		return 0.0f;

	// A lean of -1 is kept as it is
	if (!(targetUDLean > -1.0f))
		return targetUDLean;

	targetUDLean = leanInput;
	*setWheelieCam = true;
	*wheelieCam = false;

	// Find out whether the bike is in a wheelie or a stoppie
	float neededAngForWheelie = 1.0f;
	if (wheelTimer[0] != 0.0f || wheelTimer[1] != 0.0f || forwardZ <= 0.0f || (0.0f == wheelTimer[2] && 0.0f == wheelTimer[3])) {
		if (0.0f == wheelTimer[2] && 0.0f == wheelTimer[3] && (forwardZ < 0.0f && (wheelTimer[0] != 0.0f || wheelTimer[1] != 0.0f))) {
			if (stoppieAng - forwardZ > 0.6f * stoppieAng)
				*wheelieCam = true;
		}
	} else {
		neededAngForWheelie = wheelieAng - forwardZ;
		if (neededAngForWheelie < wheelieAng / 2.0f)
			*wheelieCam = true;
	}

	// Lean according to the pedals and the angle of the bike
	if (neededAngForWheelie >= 0.15f) {
		if (brakePedal <= 0.5f || velocityFwd <= 0.01f) {
			if (gasPedal > 0.5f && targetUDLean <= 0.0f && 0.3f * maxCruiseVelocity > velocityFwd)
				targetUDLean = targetUDLean < 0.1f ? targetUDLean : 0.1f;
		} else {
			targetUDLean = targetUDLean > 0.1f ? targetUDLean : 0.1f;
		}
	} else {
		targetUDLean = targetUDLean > 0.25f ? targetUDLean : 0.25f;
	}

	// Lean forward less when leaning sideways
	float targetLRLeanABS = fabsf(targetLRLean);
	if (targetLRLeanABS > 0.3f) {
		float factor = 1.0f - (targetLRLeanABS - 0.3f) * 50.0f / 13.0f;
		targetUDLean *= 0.0f > factor ? 0.0f : factor;
	}
	return targetUDLean;
}

void PedLogic_BikeLeanAmounts(float leanLR, float leanUD, bool isPlayer, float *fwdBackLeanAmount, float *leftRightLeanAmount)
{
	if (fabsf(leanLR) <= 0.56f && isPlayer) {
		if (fabsf(leanUD) <= 0.56f) {
			float smoothLeanMag = sqrtf(leanLR*leanLR + leanUD*leanUD);
			if (smoothLeanMag <= 0.01f) {
				*fwdBackLeanAmount = fabsf(leanUD);
				*leftRightLeanAmount = fabsf(leanLR);
			} else {
				*fwdBackLeanAmount = fabsf(leanUD / smoothLeanMag);
				*leftRightLeanAmount = fabsf(leanLR / smoothLeanMag);
			}
		} else {
			*fwdBackLeanAmount = 1.0f;
			*leftRightLeanAmount = 0.0f;
		}
	} else {
		*fwdBackLeanAmount = 0.0f;
		*leftRightLeanAmount = 1.0f;
	}
}

int PedLogic_HighImpactAnim(int direction, int current)
{
	switch (direction) {
	case 0: return ANIM_STD_HIGHIMPACT_FRONT;
	case 1: return ANIM_STD_HIGHIMPACT_LEFT;
	case 2: return ANIM_STD_HIGHIMPACT_BACK;
	case 3: return ANIM_STD_HIGHIMPACT_RIGHT;
	default: return current;
	}
}

int PedLogic_CarHitDieAnim(int random, bool pieceIsLeftArm, bool pieceIsRightArm, bool pieceIsMid, bool divingAway, int current)
{
	switch (random) {
	case 0:
		if ((!pieceIsLeftArm || random <= 1) && (!pieceIsMid || random != 1)) {
			if ((pieceIsRightArm && random > 1) || (pieceIsMid && random == 2))
				return ANIM_STD_HIGHIMPACT_RIGHT;
			return ANIM_STD_HIGHIMPACT_FRONT;
		}
		return ANIM_STD_HIGHIMPACT_LEFT;
	case 1:
		return divingAway ? ANIM_STD_SPINFORWARD_LEFT : ANIM_STD_HIGHIMPACT_LEFT;
	case 2:
		if ((!pieceIsLeftArm || random <= 1) && (!pieceIsMid || random != 1)) {
			if ((!pieceIsRightArm || random <= 1) && (!pieceIsMid || random != 2))
				return ANIM_STD_HIGHIMPACT_BACK;
			return ANIM_STD_SPINFORWARD_RIGHT;
		}
		return ANIM_STD_SPINFORWARD_LEFT;
	case 3:
		return divingAway ? ANIM_STD_SPINFORWARD_RIGHT : ANIM_STD_HIGHIMPACT_RIGHT;
	default:
		return current;
	}
}

int PedLogic_WalkAroundType(int currentType, float angleDiffToCenter, float objTopRightHeading, bool objUpsideDown, bool goingToEnterCar, int vehDoor, float rotDestRel,
	int onTopLeft, int onTopRight, int onBottomRight, int onBottomLeft)
{
	const float absDiff = fabsf(angleDiffToCenter);
	const int TL = onTopLeft, TR = onTopRight, BR = onBottomRight, BL = onBottomLeft;
	int type = currentType;
	const float pi = (float)3.14159265358979323846;

	if (absDiff >= objTopRightHeading) {
		if (pi - objTopRightHeading >= absDiff) {
			if ((angleDiffToCenter <= 0.0f || objUpsideDown) && (angleDiffToCenter < 0.0f || !objUpsideDown)) {
				if (goingToEnterCar && (vehDoor == CAR_DOOR_RF || vehDoor == CAR_DOOR_RR)) {
					type = 0;
				} else {
					if (rotDestRel >= 0.0f) {
						if (BR == 1 || (BR && !TL && !TR))
							type = 1;
						else if (BL == 1 || (BL && !TL && !TR))
							type = 1;
					} else {
						if (TR == 1 || (TR && !BR && !BL))
							type = 4;
						else if (TL == 1 || (TL && !BR && !BL))
							type = 4;
					}
				}
			} else {
				if (goingToEnterCar && (vehDoor == CAR_DOOR_LF || vehDoor == CAR_DOOR_LR)) {
					type = 0;
				} else {
					if (rotDestRel <= 0.0f) {
						if (BL == 1 || (BL && !TL && !TR))
							type = 2;
						else if (BR == 1 || (BR && !TL && !TR))
							type = 2;
					} else {
						if (TL == 1 || (TL && !BR && !BL))
							type = 3;
						else if (TR == 1 || (TR && !BR && !BL))
							type = 3;
					}
				}
			}
		} else if ((goingToEnterCar && (vehDoor == CAR_DOOR_LF || vehDoor == CAR_DOOR_LR)) || rotDestRel < 0.0f) {
			if (TL == 1 || (TL && !TR && !BR))
				type = 3;
		} else if (TR == 1 || (TR && !TL && !BL)) {
			type = 4;
		}
	} else if ((goingToEnterCar && (vehDoor == CAR_DOOR_LF || vehDoor == CAR_DOOR_LR)) || rotDestRel > 0.0f) {
		if (BL == 1 || (BL && !TR && !BR))
			type = 2;
	} else if (BR == 1 || (BR && !TL && !BL)) {
		type = 1;
	} else {
		type = 0;
	}
	return type;
}

bool PedLogic_WalkAroundTarget(int walkAroundType, float angleDiffToCenter, float objTopRightHeading, bool goingToEnterCar, bool carIsVan, int vehDoor, float rotDestRel,
	int onTopLeft, int onTopRight, int onBottomRight, int onBottomLeft, bool *maxX, bool *maxY)
{
	const float absDiff = fabsf(angleDiffToCenter);
	const int type = walkAroundType;
	const float pi = (float)3.14159265358979323846;

	if (absDiff < objTopRightHeading) {
		if (goingToEnterCar) {
			if (carIsVan) {
				if (vehDoor == CAR_DOOR_LR || vehDoor == CAR_DOOR_RR)
					return true;
			}
			if (vehDoor != CAR_DOOR_LF && vehDoor != CAR_DOOR_LR && (!onBottomRight || onBottomLeft)) {
				*maxX = true; *maxY = false;
			} else {
				*maxX = false; *maxY = false;
			}
		} else {
			if (type != 1 && type != 4 && (type || rotDestRel <= 0.0f)) {
				*maxX = true; *maxY = false;
			} else {
				*maxX = false; *maxY = false;
			}
		}
	} else {
		if (pi - objTopRightHeading >= absDiff) {
			if (angleDiffToCenter <= 0.0f) {
				if (!goingToEnterCar || !carIsVan || (vehDoor != CAR_DOOR_LR && vehDoor != CAR_DOOR_RR)) {
					if (goingToEnterCar) {
						if (vehDoor == CAR_DOOR_RF || (vehDoor == CAR_DOOR_RR && !carIsVan))
							return true;
					}
					if (type == 4 || type == 3 || (!type && rotDestRel > 0.0f)) {
						*maxX = true; *maxY = false;
					} else {
						*maxX = true; *maxY = true;
					}
				} else {
					*maxX = true; *maxY = false;
				}
			} else if (goingToEnterCar && carIsVan && (vehDoor == CAR_DOOR_LR || vehDoor == CAR_DOOR_RR)) {
				*maxX = false; *maxY = false;
			} else {
				if (goingToEnterCar) {
					if (vehDoor == CAR_DOOR_LF || (vehDoor == CAR_DOOR_LR && !carIsVan))
						return true;
				}
				if (type == 1 || type == 2 || (!type && rotDestRel > 0.0f)) {
					*maxX = false; *maxY = true;
				} else {
					*maxX = false; *maxY = false;
				}
			}
		} else {
			if (goingToEnterCar && (!carIsVan || (vehDoor != CAR_DOOR_LR && vehDoor != CAR_DOOR_RR))) {
				if (vehDoor != CAR_DOOR_LF && vehDoor != CAR_DOOR_LR && (!onTopRight || onTopLeft)) {
					*maxX = true; *maxY = true;
				} else {
					*maxX = false; *maxY = true;
				}
			} else {
				if (type == 2 || type == 3 || (!type && rotDestRel > 0.0f)) {
					*maxX = true; *maxY = true;
				} else {
					*maxX = false; *maxY = true;
				}
			}
		}
	}
	return false;
}

int PedLogic_SeekInitialMove(int objective, float seekPosDist, bool isRunning, int followedMoveState)
{
	int nextMove = PEDMOVE_NONE;

	if (seekPosDist < 2.0f || objective == OBJECTIVE_GOTO_AREA_ON_FOOT) {
		if (objective == OBJECTIVE_FOLLOW_CHAR_IN_FORMATION) {
			if (followedMoveState != PEDMOVE_STILL)
				nextMove = followedMoveState;
		} else
			nextMove = PEDMOVE_WALK;

	} else if (objective != OBJECTIVE_FOLLOW_CHAR_IN_FORMATION) {
		if (objective == OBJECTIVE_SPRINT_TO_AREA)
			nextMove = PEDMOVE_SPRINT;
		else if (objective == OBJECTIVE_KILL_CHAR_ON_FOOT || objective == OBJECTIVE_KILL_CHAR_ANY_MEANS || objective == OBJECTIVE_RUN_TO_AREA || isRunning)
			nextMove = PEDMOVE_RUN;
		else
			nextMove = PEDMOVE_WALK;

	} else if (seekPosDist <= 2.0f) {
		if (followedMoveState != PEDMOVE_STILL)
			nextMove = followedMoveState;

	} else {
		nextMove = PEDMOVE_RUN;
	}
	return nextMove;
}

int PedLogic_SeekTurnAdjustMove(int nextMove, float rotDest, float rotCur, float seekPosDist, bool isRunning)
{
	const float pi = (float)3.14159265358979323846;

	float neededTurn = fabsf(rotDest - rotCur);
	if (neededTurn > pi)
		neededTurn = (pi*2) - neededTurn;

	if (neededTurn > pi/2) {
		if (seekPosDist >= 1.0f) {
			if (seekPosDist < 2.0f) {
				if (isRunning)
					nextMove = PEDMOVE_RUN;
				else
					nextMove = PEDMOVE_WALK;
			}
		} else {
			nextMove = PEDMOVE_STILL;
		}
	}
	return nextMove;
}

int PedLogic_SeekPathTurnAdjustMove(int nextMove, float rotDest, float rotCur, float seekPosDist)
{
	const float pi = (float)3.14159265358979323846;

	float neededTurn = fabsf(rotDest - rotCur);
	if (neededTurn > pi)
		neededTurn = (pi*2) - neededTurn;

	if (neededTurn > pi/2) {
		if (seekPosDist >= 1.0f && neededTurn <= 135.0f * pi / 180.0f) {
			if (seekPosDist < 2.0f)
				nextMove = PEDMOVE_WALK;
		} else {
			nextMove = PEDMOVE_STILL;
		}
	}
	return nextMove;
}

bool PedLogic_SeekMayChangeMoveState(int pedState, int moveState, int nextMove, int objective, bool noWaitState)
{
	return ((pedState == PED_FLEE_POS || pedState == PED_FLEE_ENTITY) && moveState < nextMove)
		|| (pedState != PED_FLEE_POS && pedState != PED_FLEE_ENTITY && pedState != PED_FOLLOW_PATH && objective != OBJECTIVE_GOTO_CHAR_ON_FOOT && noWaitState);
}

void PedLogic_FleeCollisionHeading(uint32_t collidingTimer, uint32_t now, float posX, float posY, float entityX, float entityY, float normalX, float normalY, float *rotDest, float *rotCur)
{
	double collidingThingPriorityMult = (double)(collidingTimer - now) * 2.0 / 2500;

	if (collidingThingPriorityMult <= 1.5) {
		// Heading away from the thing the ped ran into and away from the surface it hit
		double angleToFleeEntity = General_GetRadianAngleBetweenPoints(posX, posY, entityX, entityY);
		angleToFleeEntity = General_LimitRadianAngle(angleToFleeEntity);

		double angleToFleeCollidingThing = General_GetRadianAngleBetweenPoints(normalX, normalY, 0.0f, 0.0f);
		angleToFleeCollidingThing = General_LimitRadianAngle(angleToFleeCollidingThing);
		angleToFleeCollidingThing = PedLogic_WrapDestRotation(angleToFleeCollidingThing, angleToFleeEntity);

		if (collidingThingPriorityMult <= 1.0f) {
			// Range [0.0, 1.0]
			float angleToFleeBoth = (angleToFleeCollidingThing + angleToFleeEntity) * 0.5f;
			angleToFleeBoth = PedLogic_WrapDestRotation(angleToFleeBoth, *rotDest);
			*rotDest = (1.0f - collidingThingPriorityMult) * *rotDest + collidingThingPriorityMult * angleToFleeBoth;
		} else {
			// Range (1.0, 1.5]
			double adjustedMult = (collidingThingPriorityMult - 1.0f) * 2.0f;
			*rotDest = angleToFleeEntity * (1.0 - adjustedMult) + adjustedMult * angleToFleeCollidingThing;
		}
	} else {
		*rotDest = General_GetRadianAngleBetweenPoints(normalX, normalY, 0.0f, 0.0f);
		*rotDest = General_LimitRadianAngle(*rotDest);
	}

	*rotCur = General_LimitRadianAngle(*rotCur);
	*rotDest = PedLogic_WrapDestRotation(*rotDest, *rotCur);
}

uint8_t PedLogic_OppositePathDir(int8_t pathDir)
{
	uint8_t dir = pathDir;
	if (dir <= 3)
		dir += 4;
	else
		dir -= 4;
	return dir;
}

int8_t PedLogic_WanderPathDirAdjust(int8_t pathDir, uint8_t randVal)
{
	if (randVal <= 90) {
		if (randVal > 80) {
			pathDir += 2;
			pathDir %= 8;
		}
	} else {
		pathDir -= 2;
		if (pathDir < 0)
			pathDir += 8;
	}
	return pathDir;
}

int PedLogic_WaitStateTimerMs(int waitState, bool hasTime, int timeValue)
{
	switch (waitState) {
	case WAITSTATE_TRAFFIC_LIGHTS:
		return 500;
	case WAITSTATE_CROSS_ROAD:
		return 1000;
	case WAITSTATE_CROSS_ROAD_LOOK:
		return hasTime ? timeValue : General_GetRandomNumberInRangeInt(2000, 5000);
	case WAITSTATE_DOUBLEBACK:
		return 3500;
	case WAITSTATE_HITWALL:
	case WAITSTATE_TURN180:
	case WAITSTATE_STUCK:
	case WAITSTATE_LOOK_ABOUT:
	case WAITSTATE_GROUND_ATTACK:
		return 5000;
	case WAITSTATE_SURPRISE:
		return 2000;
	case WAITSTATE_PLAYANIM_COWER:
	case WAITSTATE_PLAYANIM_HANDSUP:
	case WAITSTATE_PLAYANIM_HANDSCOWER:
	case WAITSTATE_PLAYANIM_DUCK:
	case WAITSTATE_PLAYANIM_TAXI:
	case WAITSTATE_PLAYANIM_CHAT:
		return hasTime ? timeValue : 3000;
	case WAITSTATE_FINISH_FLEE:
		return 2500;
	case WAITSTATE_SIT_DOWN:
	case WAITSTATE_SIT_UP:
		return 100000;
	case WAITSTATE_SIT_IDLE:
		return hasTime ? timeValue : General_GetRandomNumberInRangeInt(25000, 30000);
	case WAITSTATE_USE_ATM:
		return hasTime ? timeValue : 100000;
	case WAITSTATE_SUN_BATHE_IDLE:
		return General_GetRandomNumberInRangeInt(50000, 100000);
	case WAITSTATE_BOMBER:
	case WAITSTATE_PLAYANIM_HANDSUP_SIMPLE:
		return timeValue;
	default:
		return 0;
	}
}

int PedLogic_HandsUpReaction(bool lookTargetIsPed, bool fleeingOrAttacking, bool crouchWhenScared, bool fearOk, bool selfMelee, bool gunPanic)
{
	if (!lookTargetIsPed || fleeingOrAttacking)
		return HANDSUP_REACT_NONE;

	if (crouchWhenScared)
		return HANDSUP_REACT_CROUCH;

	if (fearOk) {
		if (selfMelee)
			return gunPanic ? HANDSUP_REACT_MELEE_PANIC : HANDSUP_REACT_MELEE_WANDER;
		return HANDSUP_REACT_KILL;
	}
	return HANDSUP_REACT_FLEE;
}

bool PedLogic_HandsUpKeepsWaiting(bool noPedToLookAt, bool lookTargetIsAimedAt, bool fleeingOrAttacking, bool waitRunning, bool animPlays)
{
	return (noPedToLookAt || lookTargetIsAimedAt) && !fleeingOrAttacking && waitRunning && animPlays;
}

int PedLogic_WaitStateMustHaveAnim(int waitState)
{
	switch (waitState) {
	case WAITSTATE_PLAYANIM_HANDSUP: return ANIM_STD_HANDSUP;
	case WAITSTATE_PLAYANIM_HANDSCOWER:
	case WAITSTATE_PLAYANIM_COWER: return ANIM_STD_HANDSCOWER;
	case WAITSTATE_PLAYANIM_DUCK: return ANIM_STD_DUCK_DOWN;
	case WAITSTATE_PLAYANIM_TAXI: return ANIM_STD_HAILTAXI;
	case WAITSTATE_PLAYANIM_CHAT: return ANIM_STD_CHAT;
	default: return ANIM_STD_NUM;
	}
}

bool PedLogic_DoubleBackScratch(uint32_t timeLeft)
{
	return timeLeft < 2500 && timeLeft > 2000;
}

int PedLogic_StuckWaitAnim(int randVal)
{
	switch (randVal & 3) {
	case 0: return ANIM_STD_ROADCROSS;
	case 1: return ANIM_STD_IDLE_TIRED;
	case 2: return ANIM_STD_XPRESS_SCRATCH;
	default: return ANIM_STD_TURN180;
	}
}

bool PedLogic_MoveAnimForState(int moveState, bool fleeingFromEntity, int *anim, float *blendRate)
{
	switch (moveState) {
	case PEDMOVE_STILL:
		*anim = ANIM_STD_IDLE;
		*blendRate = 4.0f;
		return true;
	case PEDMOVE_WALK:
		*anim = ANIM_STD_WALK;
		*blendRate = 1.0f;
		return true;
	case PEDMOVE_RUN:
		*anim = ANIM_STD_RUN;
		*blendRate = fleeingFromEntity ? 3.0f : 1.0f;
		return true;
	case PEDMOVE_SPRINT:
		*anim = ANIM_STD_RUNFAST;
		*blendRate = 1.0f;
		return true;
	default:
		return false;
	}
}

float PedLogic_MoveAnimSpeed(bool isMissionChar, uint16_t randomSeed, int myrandMax)
{
	if (isMissionChar)
		return 1.0f;
	return 1.2f - randomSeed * 0.4f / myrandMax;
}

bool PedLogic_ShooterIsThreat(int ourPedType, int shooterPedType)
{
	return ourPedType != shooterPedType || ourPedType == PEDTYPE_CIVMALE || ourPedType == PEDTYPE_CIVFEMALE;
}

bool PedLogic_GangFearsShooter(bool shooterTypeIsFeared, bool weAreGang5)
{
	return shooterTypeIsFeared || weAreGang5;
}

bool PedLogic_WaitStateEffects(int waitState, PedWaitStateEffects *fx)
{
	fx->moveStill = false;
	fx->moveAnim = false;
	fx->headingRate = -1.0f;
	fx->hasTimer = false;
	fx->animGroup = -1;
	fx->anim = 0;
	fx->blendRate = 0.0f;
	fx->flagsMode = PEDWAIT_FLAGS_NONE;
	fx->callback = PEDWAIT_CB_NONE;
	fx->carJackCheck = false;

#define WAIT_ANIM(group, animId, blend) do { fx->animGroup = (group); fx->anim = (animId); fx->blendRate = (blend); } while (0)

	switch (waitState) {
	case WAITSTATE_TRAFFIC_LIGHTS:
		fx->hasTimer = true;
		fx->moveStill = true;
		return true;
	case WAITSTATE_CROSS_ROAD:
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_IDLE_HBHB, 4.0f);
		return true;
	case WAITSTATE_CROSS_ROAD_LOOK:
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_ROADCROSS, 8.0f);
		return true;
	case WAITSTATE_LOOK_PED:
	case WAITSTATE_LOOK_SHOP:
	case WAITSTATE_LOOK_ACCIDENT:
	case WAITSTATE_FACEOFF_GANG:
	case WAITSTATE_RIOT:
	case WAITSTATE_STRIPPER:
		return true;
	case WAITSTATE_DOUBLEBACK:
		fx->headingRate = 0.0f;
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_IDLE_HBHB, 4.0f);
		fx->callback = PEDWAIT_CB_FINISH_RESTORE_HEADING;
		return true;
	case WAITSTATE_HITWALL:
		fx->headingRate = 2.0f;
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_HIT_WALL, 16.0f);
		fx->flagsMode = PEDWAIT_FLAGS_DELETE_AND_FADE_OUT;
		fx->callback = PEDWAIT_CB_DELETE_FINISHED;
		fx->carJackCheck = true;
		return true;
	case WAITSTATE_TURN180:
		fx->headingRate = 0.0f;
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_TURN180, 4.0f);
		fx->callback = PEDWAIT_CB_FINISH_FINISHED;
		return true;
	case WAITSTATE_SURPRISE:
		fx->headingRate = 0.0f;
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_HIT_WALL, 4.0f);
		fx->callback = PEDWAIT_CB_FINISH_FINISHED;
		return true;
	case WAITSTATE_STUCK:
		fx->moveStill = true;
		fx->moveAnim = true;
		fx->headingRate = 0.0f;
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_IDLE_TIRED, 4.0f);
		fx->callback = PEDWAIT_CB_FINISH_RESTORE_HEADING;
		fx->carJackCheck = true;
		return true;
	case WAITSTATE_LOOK_ABOUT:
		fx->moveStill = true;
		fx->moveAnim = true;
		fx->headingRate = 0.0f;
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_IDLE_HBHB, 4.0f);
		fx->callback = PEDWAIT_CB_FINISH_RESTORE_HEADING;
		return true;
	case WAITSTATE_PLAYANIM_COWER:
	case WAITSTATE_PLAYANIM_HANDSUP:
	case WAITSTATE_PLAYANIM_HANDSCOWER:
		fx->headingRate = 0.0f;
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, PedLogic_WaitStateMustHaveAnim(waitState), 4.0f);
		fx->callback = PEDWAIT_CB_DELETE_FINISHED;
		return true;
	case WAITSTATE_PLAYANIM_DUCK:
	case WAITSTATE_PLAYANIM_TAXI:
	case WAITSTATE_PLAYANIM_CHAT:
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, PedLogic_WaitStateMustHaveAnim(waitState), 4.0f);
		fx->flagsMode = PEDWAIT_FLAGS_DELETE_NO_FADE_OUT;
		fx->callback = PEDWAIT_CB_DELETE_FINISHED;
		return true;
	case WAITSTATE_FINISH_FLEE:
		fx->moveStill = true;
		fx->moveAnim = true;
		fx->headingRate = 0.0f;
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_IDLE_TIRED, 4.0f);
		fx->callback = PEDWAIT_CB_FINISH_RESTORE_HEADING;
		return true;
	case WAITSTATE_SIT_DOWN:
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_SEAT_DOWN, 4.0f);
		fx->callback = PEDWAIT_CB_FINISH_FINISHED;
		return true;
	case WAITSTATE_SIT_UP:
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_SEAT_UP, 4.0f);
		fx->callback = PEDWAIT_CB_FINISH_FINISHED;
		return true;
	case WAITSTATE_SIT_IDLE:
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_SEAT_IDLE, 128.f);
		fx->callback = PEDWAIT_CB_FINISH_FINISHED;
		return true;
	case WAITSTATE_USE_ATM:
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_ATM, 4.0f);
		fx->callback = PEDWAIT_CB_FINISH_FINISHED;
		return true;
	case WAITSTATE_SUN_BATHE_IDLE:
		fx->headingRate = 0.0f;
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_SUNBATHE, ANIM_SUNBATHE_IDLE, 4.0f);
		fx->callback = PEDWAIT_CB_DELETE_SUNBATHE;
		return true;
	case WAITSTATE_BOMBER:
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_DETONATE, 4.0f);
		return true;
	case WAITSTATE_LANCESITTING:
		WAIT_ANIM(ASSOCGRP_LANCE, ANIM_SUNBATHE_IDLE, 4.0f);
		return true;
	case WAITSTATE_PLAYANIM_HANDSUP_SIMPLE:
		fx->hasTimer = true;
		WAIT_ANIM(ASSOCGRP_STD, ANIM_STD_HANDSUP, 4.0f);
		fx->flagsMode = PEDWAIT_FLAGS_DELETE_NO_FADE_OUT;
		fx->callback = PEDWAIT_CB_DELETE_FINISHED;
		return true;
	default:
		return false;
	}
#undef WAIT_ANIM
}

uint32_t PedLogic_FallGetUpTimer(int extraTime, bool hasAnim, bool isPlayer, bool animIsRollOut, float animTotalLength, float animCurrentTime, uint32_t now, uint16_t randomSeed, uint32_t frameCounter)
{
	if (extraTime == -1)
		return UINT32_MAX;

	if (hasAnim) {
		if (isPlayer) {
			if (animIsRollOut) {
				return (uint32_t)(1000.0f * animTotalLength
					+ now
					- 1000.0f * animCurrentTime
					+ 100.0f);
			}
			return (uint32_t)(1000.0f * animTotalLength
				+ now
				+ 500.0f);
		}
		return (uint32_t)(1000.0f * animTotalLength
			+ now
			+ extraTime
			+ ((randomSeed + frameCounter) % 1000));
	}

	return extraTime
		+ now
		+ 1000
		+ ((randomSeed + frameCounter) % 1000);
}

bool PedLogic_DuckCannotStart(bool isDucking, uint32_t now, uint32_t duckTimer, bool sth, uint32_t time, uint32_t *newDuckTimer)
{
	*newDuckTimer = duckTimer;

	if (isDucking || (now <= duckTimer && !sth)) {
		if (sth && now + time > duckTimer)
			*newDuckTimer = now + time;
		return true;
	}
	return false;
}

float PedLogic_BloodPoolFade(uint32_t timeSinceDeath)
{
	return (timeSinceDeath - 2000) / 5000.0f;
}

bool PedLogic_ShakesFistAtDriver(int weaponType, uint32_t weaponSlot)
{
	return weaponType == WEAPONTYPE_UNARMED
		|| weaponSlot == 3
		|| weaponSlot == 5
		|| weaponSlot == 1;
}

bool PedLogic_WalksAroundBumpingVehicle(bool inControl, bool isPlayer, bool objectiveIsGoingSomewhere)
{
	return inControl && (!isPlayer || objectiveIsGoingSomewhere);
}

float PedLogic_CarHitAdjustedImpulse(bool isPlayer, bool isStanding, float dotProd, float damageImpulse, float mass)
{
	float adjustedImpulse = damageImpulse;
	if (isPlayer) {
		if (isStanding) {
			if (dotProd < 0.0f) {
				adjustedImpulse = dotProd * mass + damageImpulse;
				if (adjustedImpulse < 0.0f)
					adjustedImpulse = 0.0f;
			}
		}
	}
	return adjustedImpulse;
}

float PedLogic_CarHitSideCheck(float angleDiffFromFront, float vehTopRightHeading, float vehSpeedSqr, float rightDotSpeed, float rightDotPedForward, float distDotSpeed, bool *evasive)
{
	const float pi = (float)3.14159265358979323846;
	float vehRightVecAndSpeedDotProd;
	*evasive = false;

	if (fabsf(angleDiffFromFront) >= vehTopRightHeading && fabsf(angleDiffFromFront) < pi - vehTopRightHeading) {
		if (angleDiffFromFront <= 0.0f) {
			vehRightVecAndSpeedDotProd = rightDotSpeed;

			// The vehicle is being overturned or spinning to its right
			if (vehSpeedSqr > 1.0f / 100.0f && vehRightVecAndSpeedDotProd < 0.1f) {
				// The right of the car faces towards us and it isn't coming directly to us
				if (rightDotPedForward < 0.0f && distDotSpeed > 0.0f)
					*evasive = true;
			}
		} else {
			vehRightVecAndSpeedDotProd = -rightDotSpeed;

			if (vehSpeedSqr > 1.0f / 100.0f && vehRightVecAndSpeedDotProd < 0.1f) {
				if (rightDotPedForward > 0.0f && distDotSpeed > 0.0f)
					*evasive = true;
			}
		}
	} else {
		vehRightVecAndSpeedDotProd = distDotSpeed;
	}
	return vehRightVecAndSpeedDotProd;
}

bool PedLogic_InAirDrag(bool isStanding, float moveSpeedZ)
{
	return !isStanding && moveSpeedZ > 0.25;
}

bool PedLogic_NeedsPhysicsProcess(bool isPlayer, bool isStanding, float moveSpeedX, float moveSpeedY, float moveSpeedZ, int moveState, float animMoveDeltaX, float animMoveDeltaY,
	bool isJumping, bool isInTheAir, bool hasPhysSurface)
{
	return isPlayer || !isStanding || moveSpeedX != 0.0f || moveSpeedY != 0.0f || moveSpeedZ != 0.0f
		|| (moveState != PEDMOVE_NONE && moveState != PEDMOVE_STILL)
		|| animMoveDeltaX != 0.0f || animMoveDeltaY != 0.0f
		|| isJumping
		|| isInTheAir
		|| hasPhysSurface;
}

bool PedLogic_IsOpenDoorAnim(int anim)
{
	return anim == ANIM_STD_CAR_OPEN_DOOR_LHS || anim == ANIM_STD_CAR_OPEN_DOOR_RHS || anim == ANIM_STD_COACH_OPEN_LHS || anim == ANIM_STD_COACH_OPEN_RHS ||
		anim == ANIM_STD_VAN_OPEN_DOOR_REAR_LHS || anim == ANIM_STD_VAN_OPEN_DOOR_REAR_RHS;
}

bool PedLogic_IsPullOutAnim(int anim)
{
	return anim == ANIM_STD_CAR_PULL_OUT_PED_LHS || anim == ANIM_STD_CAR_PULL_OUT_PED_LO_LHS || anim == ANIM_STD_CAR_PULL_OUT_PED_LO_RHS || anim == ANIM_STD_CAR_PULL_OUT_PED_RHS;
}

bool PedLogic_ShouldBeSetDead(bool isStanding, bool hasPhysSurface, bool isPlayer, bool isInWater, bool usesCollision)
{
	return (isStanding && (!hasPhysSurface || isPlayer)) || isInWater || !usesCollision;
}

bool PedLogic_FannyMagnetPullsPed(bool cheatOn, int pedType, int sexiness, bool hasLeader)
{
	return cheatOn && pedType == PEDTYPE_CIVFEMALE && sexiness > 40 && !hasLeader;
}

bool PedLogic_ForgetsStoredPoly(bool isPlayer, float collisionSpeed, int pedState)
{
	return isPlayer || (collisionSpeed >= 1.0f && (collisionSpeed >= 2.0f || pedState != PED_WANDER_PATH));
}

void PedLogic_LandingLimits(bool isIdle, float *upperSpeedLimit, float *lowerSpeedLimit)
{
	if (isIdle) {
		*upperSpeedLimit *= 2.0f;
		*lowerSpeedLimit *= 1.5f;
	}
}

bool PedLogic_LandingHurts(bool wasStanding, float speed, float upperSpeedLimit, bool pushedAlongByCar, float speedZ, float lowerSpeedLimit, bool collidedWithSameEntity)
{
	return !wasStanding && ((speed > upperSpeedLimit && !pushedAlongByCar) || (speedZ < lowerSpeedLimit))
		&& !collidedWithSameEntity;
}

float PedLogic_LandingDamage(float speed, float speedZ, float *damage2)
{
	float over = speed - 0.25f;
	float damage = 100.0f * (over > 0.0f ? over : 0.0f);
	*damage2 = damage;
	if (speedZ < -0.25f)
		damage += (-0.25f - speedZ) * 150.0f;
	return damage;
}

bool PedLogic_LandingBumps(bool wasStanding, bool hasFallAnim, float timeStep, float speedZ)
{
	return !wasStanding && hasFallAnim && -0.016f * timeStep > speedZ;
}

int PedLogic_FootStepPart(float currentTime, float timeStep, float stepStart, float stepEnd)
{
	int stepPart = 0;

	if (currentTime >= stepStart && currentTime - timeStep < stepStart)
		stepPart = 1;
	else if (currentTime >= stepEnd && currentTime - timeStep < stepEnd)
		stepPart = 2;
	return stepPart;
}

bool PedLogic_IsSandSurface(int surface)
{
	return surface == SURFACE_SAND || surface == SURFACE_SAND_BEACH;
}

bool PedLogic_BloodyFootprintStep(uint32_t *count)
{
	if (*count <= 20) {
		*count = 0;
		return true;
	}
	*count -= 20;
	return false;
}

bool PedLogic_WaterRunSplashes(float pedSpeed, uint32_t frameCounter)
{
	return pedSpeed > 0.03f && frameCounter % 2 == 0 && pedSpeed > 0.13f;
}

float PedLogic_WaterRunParticleSize(float pedSpeed)
{
	float particleSize = pedSpeed * 2.0f;

	if (particleSize < 0.25f)
		particleSize = 0.25f;

	if (particleSize > 0.75f)
		particleSize = 0.75f;
	return particleSize;
}

float PedLogic_EvasiveStepAngle(float angleToFace, float dangerDirection)
{
	const float pi = (float)3.14159265358979323846;

	// Let's turn our back to the "reason"
	angleToFace += pi;

	if (angleToFace > pi)
		angleToFace -= pi*2;

	// We don't want to run towards car's direction
	float dangerZone = angleToFace - dangerDirection;
	dangerZone = General_LimitRadianAngle(dangerZone);

	// So, add or subtract 90deg (jump to left/right) according to that
	if (dangerZone > 0.0f)
		angleToFace = dangerDirection - pi/2;
	else
		angleToFace = dangerDirection + pi/2;
	return angleToFace;
}

float PedLogic_EvasiveDiveAngle(float angleToFace, float vehDirection)
{
	const float pi = (float)3.14159265358979323846;

	// Let's turn our back to the "reason"
	angleToFace += pi;

	if (angleToFace > pi)
		angleToFace -= 2 * pi;

	// We don't want to dive towards car's direction
	float dangerZone = angleToFace - vehDirection;
	dangerZone = General_LimitRadianAngle(dangerZone);

	// So, add or subtract 90deg (jump to left/right) according to that
	if (dangerZone > 0.0f)
		angleToFace = 0.5f * pi + vehDirection;
	else
		angleToFace = vehDirection - 0.5f * pi;
	return angleToFace;
}

int PedLogic_EvasiveStepAnim(int animType)
{
	int stepAnim = ANIM_STD_NUM;
	if (animType == 0 || animType == 1)
		stepAnim = ANIM_STD_EVADE_STEP;
	else if (animType == 2)
		stepAnim = ANIM_STD_HANDSCOWER;
	return stepAnim;
}

int PedLogic_SeekCarEnterAction(int status, bool isBike, bool isBus, int door, bool leaderIsDriver, bool hasDriver, bool hasPassenger0, bool hasPassenger1, bool hasPassenger2,
	bool isPlayer, bool passenger0DontDrag, bool objectiveIsEnterAsDriver)
{
	switch (status) {
	case STATUS_PLAYER:
	case STATUS_SIMPLE:
	case STATUS_PHYSICS:
	case STATUS_PLAYER_DISABLED:
		if (isBike) {
			if (!leaderIsDriver &&
				(((door == CAR_DOOR_LF || door == CAR_DOOR_RF || door == CAR_WINDSCREEN) && hasDriver) ||
				((door == CAR_DOOR_LR || door == CAR_DOOR_RR) && hasPassenger0)))
				return PED_SEEKCAR_CARJACK;
			return PED_SEEKCAR_ENTER;
		} else if (!isBus && !leaderIsDriver &&
			((door == CAR_DOOR_LF && hasDriver) || (door == CAR_DOOR_RF && hasPassenger0) || (door == CAR_DOOR_LR && hasPassenger1) || (door == CAR_DOOR_RR && hasPassenger2))) {
			if (objectiveIsEnterAsDriver && door != CAR_DOOR_LF)
				return PED_SEEKCAR_CARJACK_FLEE_DRIVER;
			return PED_SEEKCAR_CARJACK;
		}
		return PED_SEEKCAR_ENTER;
	case STATUS_ABANDONED:
		if (isBike) {
			if ((door == CAR_DOOR_LR || door == CAR_DOOR_RR) && hasPassenger0) {
				if (passenger0DontDrag)
					return isPlayer ? PED_SEEKCAR_ENTER : PED_SEEKCAR_NONE;
				return PED_SEEKCAR_CARJACK;
			}
			return PED_SEEKCAR_ENTER;
		} else if (door == CAR_DOOR_RF && hasPassenger0) {
			if (passenger0DontDrag)
				return isPlayer ? PED_SEEKCAR_ENTER : PED_SEEKCAR_NONE;
			return PED_SEEKCAR_CARJACK;
		}
		return PED_SEEKCAR_ENTER;
	case STATUS_WRECKED:
		return PED_SEEKCAR_IDLE;
	default:
		return PED_SEEKCAR_RETURN;
	}
}

int PedLogic_SeekCarMoveState(bool isRunning, bool carHasDriver, float distToDestSqr, float carSpeedX, float carSpeedY)
{
	if (isRunning ||
		(carHasDriver && distToDestSqr > 2.0f*2.0f && (fabsf(carSpeedX) > 0.01f || fabsf(carSpeedY) > 0.01f)))
		return PEDMOVE_RUN;
	else if (distToDestSqr < 2.0f*2.0f)
		return PEDMOVE_WALK;
	return PEDMOVE_NONE;
}

int PedLogic_CanEnterSeekedCar(float distToDestSqr, float carBoxMaxX)
{
	if (distToDestSqr >= 1.0f)
		return 0;
	else if (2.0f * carBoxMaxX > distToDestSqr)
		return 1;
	return 2;
}

bool PedLogic_IsDiagonalWalk(float localWalkAngle)
{
	return localWalkAngle > -(50.0f * PI / 180.0f) && localWalkAngle < (50.0f * PI / 180.0f);
}

bool PedLogic_UpperLegsYaw(float localWalkAngle, float *yaw)
{
	float y = localWalkAngle;

	// Fold the angle to half a turn when it is beyond 100 degrees
	if (y < -(100.0f * PI / 180.0f)) {
		y += PI;
	} else if (y > (100.0f * PI / 180.0f)) {
		y -= PI;
	}

	*yaw = y;
	return y > -(50.0f * PI / 180.0f) && y < (50.0f * PI / 180.0f);
}

bool PedLogic_TargetInViewRange(float dx, float dy, float forwardX, float forwardY)
{
	// Target is behind the ped
	if (dx * forwardX + dy * forwardY < 0.0f)
		return false;

	// Target is too far away
	if (sqrtf(dx * dx + dy * dy) >= 40.0f)
		return false;

	return true;
}

PedFiremanResult PedLogic_FiremanDecide(int state, bool hasNearestFire, bool nearestIsAttended, float fireDist, bool attendedIsOngoing, bool nearestWithin30)
{
	PedFiremanResult r;
	r.newState = state;
	r.chaseNearestFire = false;
	r.standIdle = false;
	r.sayExtinguishing = false;
	r.stop = false;

	switch (state) {
	case PED_EMERGENCY_READY:
		if (hasNearestFire) {
			r.chaseNearestFire = true;
			r.newState = PED_EMERGENCY_DETERMINE_NEXT_STATE;
		}
		break;
	case PED_EMERGENCY_DETERMINE_NEXT_STATE:
		if (hasNearestFire && !nearestIsAttended) {
			r.chaseNearestFire = true;
		} else if (!hasNearestFire) {
			r.newState = PED_EMERGENCY_STOP;
		}

		// "Extinguish" the fire, it overwrites the stop decision above
		if (fireDist < 5.0f) {
			r.standIdle = true;
			r.newState = PED_EMERGENCY_STAND_STILL;
		}
		break;
	case PED_EMERGENCY_STAND_STILL:
		if (!attendedIsOngoing)
			r.newState = PED_EMERGENCY_STOP;

		if (hasNearestFire && !nearestIsAttended && nearestWithin30)
			r.newState = PED_EMERGENCY_DETERMINE_NEXT_STATE;
		r.sayExtinguishing = true;
		break;
	case PED_EMERGENCY_STOP:
		r.stop = true;
		r.newState = PED_EMERGENCY_READY;
		break;
	default:
		break;
	}
	return r;
}

PedMedicArrival PedLogic_MedicArrival(float distToEmergency, bool victimOnFire, bool cprInProgress)
{
	PedMedicArrival r;
	r.stopRunning = false;
	r.moveState = PEDMOVE_NONE;
	r.startCPR = false;

	if (distToEmergency < 5.0f) {
		if (victimOnFire) {
			r.stopRunning = true;
			r.moveState = PEDMOVE_STILL;
		} else if (distToEmergency < 4.5f) {
			r.stopRunning = true;
			r.moveState = PEDMOVE_WALK;
			if (distToEmergency < 1.0f || (distToEmergency < 4.5f && cprInProgress))
				r.startCPR = true;
		}
	}
	return r;
}

bool PedLogic_MedicStandStillResumes(bool noRevivedPed, bool revivedPedHealthy, bool noVictim, bool noCPRInProgress, bool unattendedAccidents)
{
	if (noRevivedPed || revivedPedHealthy)
		return true;
	return noVictim || noCPRInProgress || unattendedAccidents;
}

bool PedLogic_MedicFacesPatient(float rotationCur, float rotationDest, float *newRotation)
{
	// Close enough to the destination when within 45 degrees
	if (fabsf(rotationCur - rotationDest) < (45.0f * (float)3.14159265358979323846 / 180.0f))
		return true;
	*newRotation = (rotationCur + rotationDest) * 0.5f;
	return false;
}

int PedLogic_MedicIdleAction(bool hasVehicle, bool inVehicle, bool objectiveIsEnterAsDriver, bool objectiveIsEnterAsPassenger, bool objectiveIsKillOnFoot,
	bool vehicleHasDriver, bool driverIsEmergency, bool someoneIsGettingIn, bool isWandering)
{
	if (hasVehicle) {
		if (!inVehicle) {
			if (objectiveIsEnterAsDriver || vehicleHasDriver || someoneIsGettingIn) {
				if (vehicleHasDriver && !driverIsEmergency && !objectiveIsKillOnFoot)
					return PED_MEDICIDLE_KILL_DRIVER;
				else if (!objectiveIsEnterAsDriver && !objectiveIsEnterAsPassenger && !objectiveIsKillOnFoot)
					return PED_MEDICIDLE_ENTER_AS_PASSENGER;
			} else {
				return PED_MEDICIDLE_ENTER_AS_DRIVER;
			}
		}
	} else if (!isWandering) {
		return PED_MEDICIDLE_WANDER;
	}
	return PED_MEDICIDLE_NONE;
}

void PedLogic_RemoveCopFromPursuit(void **cops, uint8_t maxCops, uint8_t *currentCops, void *self)
{
	int ourCopId = 0;
	bool foundMyself = false;
	int biggestCopId = 0;

	// The bound follows the number of cops that changes in the loop
	for (int i = 0; i < (maxCops > *currentCops ? maxCops : *currentCops); ++i) {
		if (!foundMyself && cops[i] == self) {
			cops[i] = NULL;
			--*currentCops;
			foundMyself = true;
			ourCopId = i;
			biggestCopId = i;
		} else {
			if (cops[i])
				biggestCopId = i;
		}
	}

	// Move the last cop in the slot that got free
	if (foundMyself && biggestCopId > ourCopId) {
		cops[ourCopId] = cops[biggestCopId];
		cops[biggestCopId] = NULL;
	}
}

bool PedLogic_AddCopToPursuit(void **cops, uint8_t maxCops, uint8_t *currentCops, void *self, bool ignoreCopLimit)
{
	bool inPursuit = false;

	if (*currentCops < maxCops || ignoreCopLimit) {
		for (int i = 0; i < maxCops; ++i) {
			if (!cops[i]) {
				inPursuit = true;
				++*currentCops;
				cops[i] = self;
				break;
			}
		}
	}
	return inPursuit;
}

int PedLogic_CopPursuitAction(bool inPursuit, void **cops, int oldCopNum, int maxCops, void *self, float distanceToTarget, float (*getDistance)(void *cop), void **farthestCop)
{
	void *copFarthestToTarget = NULL;
	float copFarthestToTargetDist = distanceToTarget;

	// Look for the cop farthest from the target
	for (int i = 0; i < (maxCops > oldCopNum ? maxCops : oldCopNum); i++) {
		void *cop = cops[i];
		if (cop && getDistance(cop) > copFarthestToTargetDist) {
			copFarthestToTargetDist = getDistance(cop);
			copFarthestToTarget = cops[i];
		}
	}
	*farthestCop = copFarthestToTarget;

	if (inPursuit) {
		if (copFarthestToTarget && oldCopNum > maxCops) {
			if (copFarthestToTarget == self && distanceToTarget > 10.0f)
				return PED_COPPURSUIT_CLEAR_SELF;
			else if (copFarthestToTargetDist > 10.0f)
				return PED_COPPURSUIT_CLEAR_FARTHEST;
		}
	} else {
		if (oldCopNum < maxCops) {
			return PED_COPPURSUIT_JOIN;
		} else {
			if (distanceToTarget <= 10.0f || (copFarthestToTarget && distanceToTarget < copFarthestToTargetDist)) {
				if (copFarthestToTarget && copFarthestToTargetDist > 10.0f)
					return PED_COPPURSUIT_CLEAR_FARTHEST_AND_JOIN;
				return PED_COPPURSUIT_JOIN;
			}
		}
	}
	return PED_COPPURSUIT_NONE;
}

int PedLogic_CopSeekEntityArrival(bool objectiveIsKillOnFoot, float distanceToTarget, int playerState)
{
	if (objectiveIsKillOnFoot && distanceToTarget < 2.5f && playerState != -1) {
		if (playerState == PED_ARRESTED || playerState == PED_ENTER_CAR ||
			(playerState == PED_CARJACK && distanceToTarget < 1.3f))
			return PED_COPARRIVAL_ARREST;
		return PED_COPARRIVAL_RESTORE;
	}
	return PED_COPARRIVAL_RESTORE;
}

int PedLogic_CopSeekPosArrival(bool playerHasVehicle, bool playerEntersCar, float copZ, float seekZ, bool vehicleCanBeEntered)
{
	if (!playerHasVehicle && playerEntersCar)
		return PED_COPSEEK_ARREST;
	else if (1.5f + copZ <= seekZ || copZ - 0.3f >= seekZ)
		return PED_COPSEEK_STILL;
	else if (playerHasVehicle && vehicleCanBeEntered)
		return PED_COPSEEK_CARJACK;
	return PED_COPSEEK_NONE;
}

static VectorMath3 FpSub(VectorMath3 a, VectorMath3 b)
{
	VectorMath3 r = { a.x - b.x, a.y - b.y, a.z - b.z };
	return r;
}

static float FpDot(VectorMath3 a, VectorMath3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Point where the line from ourPos along dirToGo crosses a plane
static VectorMath3 FpCrossPlane(VectorMath3 ourPos, VectorMath3 dirToGo, VectorMath3 normal, float dotCorrection)
{
	float f = -(FpDot(ourPos, normal) + dotCorrection) / FpDot(dirToGo, normal);
	VectorMath3 r = { f * dirToGo.x + ourPos.x, f * dirToGo.y + ourPos.y, f * dirToGo.z + ourPos.z };
	return r;
}

void PedLogic_FollowPathPlanes(const VectorMath3 corners[4], VectorMath3 normals[4], float dotCorrection[4])
{
	VectorMath3 up = { 0.0f, 0.0f, 1.0f };
	VectorMath3 prev = corners[3];
	int i;

	for (i = 0; i < 4; i++) {
		VectorMath3 fwdToNextCorner = FpSub(corners[i], prev);
		VectorMath_Normalise(&fwdToNextCorner);
		normals[i] = VectorMath_Cross(fwdToNextCorner, up);

		// The dot product is with the global position, as in a distance to a plane
		dotCorrection[i] = -FpDot(prev, normals[i]);
		prev = corners[i];
	}
}

bool PedLogic_FollowPathClip(const VectorMath3 normals[4], const float dotCorrection[4], VectorMath3 *startCandidate, VectorMath3 *targetCandidate, VectorMath3 ourPos, VectorMath3 dirToGo)
{
	bool weReGoingGreat = false;
	int i;

	for (i = 0; i < 4; i++) {
		VectorMath3 curPlaneNormal = normals[i];
		float minusGlobalCornerPos = dotCorrection[i];
		float startVecDistToPlane = FpDot(curPlaneNormal, *startCandidate) + minusGlobalCornerPos;
		int startVecStatus, targetVecStatus;

		if (startVecDistToPlane > 0.1f)
			startVecStatus = 1;
		else if (startVecDistToPlane < -0.1f)
			startVecStatus = -1;
		else
			startVecStatus = 0;

		float targetVecDistToPlane = FpDot(curPlaneNormal, *targetCandidate) + minusGlobalCornerPos;
		if (targetVecDistToPlane > 0.1f)
			targetVecStatus = 1;
		else if (targetVecDistToPlane < -0.1f)
			targetVecStatus = -1;
		else
			targetVecStatus = 0;

		if (startVecStatus == -1 || targetVecStatus == -1) {
			if (startVecStatus == -1 && targetVecStatus == 1) {
				*targetCandidate = FpCrossPlane(ourPos, dirToGo, curPlaneNormal, minusGlobalCornerPos);
			} else if (startVecStatus == 1 && targetVecStatus == -1) {
				*startCandidate = FpCrossPlane(ourPos, dirToGo, curPlaneNormal, minusGlobalCornerPos);
			}
		} else {
			weReGoingGreat = true;
			if (startVecStatus == 0) {
				float f = 0.1f - startVecDistToPlane;
				startCandidate->x += f * curPlaneNormal.x;
				startCandidate->y += f * curPlaneNormal.y;
				startCandidate->z += f * curPlaneNormal.z;
			}
			if (targetVecStatus == 0) {
				float f = 0.1f - targetVecDistToPlane;
				targetCandidate->x += f * curPlaneNormal.x;
				targetCandidate->y += f * curPlaneNormal.y;
				targetCandidate->z += f * curPlaneNormal.z;
			}
		}
	}
	return weReGoingGreat;
}

void PedLogic_FollowPathFindPlanes(const VectorMath3 normals[4], const float dotCorrection[4], VectorMath3 *startCandidate, VectorMath3 *targetCandidate, int *lastPlaneBehindUs, int *lastPlaneInFrontOfUs)
{
	VectorMath3 oldstartVecCandidate = *startCandidate;
	VectorMath3 oldDirToGo = FpSub(*targetCandidate, *startCandidate);
	int i;

	*lastPlaneBehindUs = -1;
	*lastPlaneInFrontOfUs = -1;
	VectorMath_Normalise(&oldDirToGo);

	// At least one plane should be between the target and us
	for (i = 0; i < 4; i++) {
		VectorMath3 curPlaneNormal = normals[i];
		float minusGlobalCornerPos = dotCorrection[i];
		float startVecDistToPlane = FpDot(curPlaneNormal, *startCandidate) + minusGlobalCornerPos;
		float targetVecDistToPlane = FpDot(curPlaneNormal, *targetCandidate) + minusGlobalCornerPos;

		if (startVecDistToPlane > 0.0f && targetVecDistToPlane < 0.0f) {
			*lastPlaneInFrontOfUs = i;
			*startCandidate = FpCrossPlane(oldstartVecCandidate, oldDirToGo, curPlaneNormal, minusGlobalCornerPos);
		} else if (startVecDistToPlane < 0.0f && targetVecDistToPlane > 0.0f) {
			*lastPlaneBehindUs = i;
			*targetCandidate = FpCrossPlane(oldstartVecCandidate, oldDirToGo, curPlaneNormal, minusGlobalCornerPos);
		}
	}
}

int PedLogic_FollowPathPickVariant(bool route1Ok, bool route2Ok, int nodeToStop1, int nodeToStop2, float length1, float length2, bool sameObstacle, bool obstacle1AtLeastAsBig)
{
	if (route1Ok && route2Ok)
		return length1 < length2 ? 1 : 2;
	else if (route1Ok)
		return 1;
	else if (route2Ok)
		return 2;
	else if (nodeToStop1 == 1 && nodeToStop2 > 1)
		return 2;
	else if (nodeToStop1 > 1 && nodeToStop2 == 1)
		return 1;
	else if (sameObstacle)
		return length1 < length2 ? 1 : 2;
	else if (obstacle1AtLeastAsBig)
		return 2;
	return 1;
}

float PedLogic_WalkAroundCheckDist(float colMinY, float colMaxY, float colMinZ, float colMaxZ, float adjustedMinX, float adjustedMaxX)
{
	float checkIntervalInDist = (colMaxY - colMinY) * 0.1f;

	checkIntervalInDist = checkIntervalInDist > 0.5f ? checkIntervalInDist : 0.5f;
	checkIntervalInDist = checkIntervalInDist < (colMaxZ - colMinZ) / 2.0f ? checkIntervalInDist : (colMaxZ - colMinZ) / 2.0f;
	checkIntervalInDist = checkIntervalInDist < (adjustedMaxX - adjustedMinX) / 2.0f ? checkIntervalInDist : (adjustedMaxX - adjustedMinX) / 2.0f;
	return checkIntervalInDist;
}

float PedLogic_WalkAroundCheckTime(bool isPlayer, float playerMoveSpeed, int moveState)
{
	if (isPlayer) {
		if (playerMoveSpeed <= 0.0f)
			return 0.0f;
		return 2.0f / playerMoveSpeed;
	}
	switch (moveState) {
	case PEDMOVE_WALK:
		return 2.0f;
	case PEDMOVE_RUN:
		return 0.5f;
	case PEDMOVE_SPRINT:
		return 0.5f;
	default:
		return 0.0f;
	}
}

float PedLogic_WalkAroundFinalRotation(float rotationDest, float rotationCur, bool hitSomethingLastFrame, float oldRotationDest, float dirToSet)
{
	const float pi = (float)3.14159265358979323846;

	if (rotationDest != rotationCur && hitSomethingLastFrame) {
		if (rotationDest == oldRotationDest)
			rotationDest = oldRotationDest;
		else
			rotationDest = General_LimitRadianAngle(pi + dirToSet);
	}
	return rotationDest;
}

float PedLogic_WalkAroundTimer(float dist, float distLimit, float checkIntervalInTime)
{
	if (dist < 0.5f)
		dist = 0.5f;
	if (dist > distLimit)
		dist = distLimit;
	return 280.0f * dist * checkIntervalInTime;
}

int PedLogic_PickSeekCarCorner(int door, float helperX, float helperY, const float corners[8], float vehForwardHeading)
{
	const float pi = (float)3.14159265358979323846;
	const float halfpi = pi / 2;

	// The corners are the left rear, the right rear, the left front and the right front
	float potentialLrHeading = atan2f(corners[0] - helperX, corners[1] - helperY);
	float NTVF_LR = General_LimitRadianAngle(potentialLrHeading - vehForwardHeading);

	float potentialRrHeading = atan2f(corners[2] - helperX, corners[3] - helperY);
	float NTVF_RR = General_LimitRadianAngle(potentialRrHeading - vehForwardHeading);

	float potentialLfHeading = atan2f(corners[4] - helperX, corners[5] - helperY);
	float NTVF_LF = General_LimitRadianAngle(potentialLfHeading - vehForwardHeading);

	float potentialRfHeading = atan2f(corners[6] - helperX, corners[7] - helperY);
	float NTVF_RF = General_LimitRadianAngle(potentialRfHeading - vehForwardHeading);

	bool canHeadToLr = NTVF_LR <= -pi || NTVF_LR >= -halfpi;
	bool canHeadToRr = NTVF_RR <= halfpi || NTVF_RR >= pi;
	bool canHeadToLf = NTVF_LF >= 0.0f || NTVF_LF <= -halfpi;
	bool canHeadToRf = NTVF_RF <= 0.0f || NTVF_RF >= halfpi;

	// Only the order of the conditions differs among the doors
	if (door == CAR_DOOR_RR) {
		if (canHeadToRr) return PED_CARCORNER_RIGHT_REAR;
		else if (canHeadToRf) return PED_CARCORNER_RIGHT_FRONT;
		else if (canHeadToLr) return PED_CARCORNER_LEFT_REAR;
		else if (canHeadToLf) return PED_CARCORNER_LEFT_FRONT;
	} else if (door == CAR_DOOR_RF) {
		if (canHeadToRf) return PED_CARCORNER_RIGHT_FRONT;
		else if (canHeadToRr) return PED_CARCORNER_RIGHT_REAR;
		else if (canHeadToLf) return PED_CARCORNER_LEFT_FRONT;
		else if (canHeadToLr) return PED_CARCORNER_LEFT_REAR;
	} else if (door == CAR_DOOR_LF) {
		if (canHeadToLf) return PED_CARCORNER_LEFT_FRONT;
		else if (canHeadToLr) return PED_CARCORNER_LEFT_REAR;
		else if (canHeadToRf) return PED_CARCORNER_RIGHT_FRONT;
		else if (canHeadToRr) return PED_CARCORNER_RIGHT_REAR;
	} else if (door == CAR_DOOR_LR) {
		if (canHeadToLr) return PED_CARCORNER_LEFT_REAR;
		else if (canHeadToLf) return PED_CARCORNER_LEFT_FRONT;
		else if (canHeadToRr) return PED_CARCORNER_RIGHT_REAR;
		else if (canHeadToRf) return PED_CARCORNER_RIGHT_FRONT;
	}
	return PED_CARCORNER_NONE;
}

float PedLogic_TommyFatnessScale(int bone, float fatness)
{
	float part;

	switch (bone) {
	case PED_HEAD:
		part = 0.7f;
		break;
	case PED_NECK:
	case PED_FOREARML:
	case PED_FOREARMR:
		part = 0.2f;
		break;
	case PED_MID:
	case PED_LOWERLEGL:
	case PED_LOWERLEGR:
		part = 0.5f;
		break;
	case PED_UPPERLEGL:
	case PED_UPPERLEGR:
		part = 1.0f;
		break;
	case PED_FOOTL:
	case PED_FOOTR:
	case PED_UPPERARML:
	case PED_UPPERARMR:
		part = 0.23f;
		break;
	default:
		part = 0.0f;
		break;
	}
	return 1.0f + fatness * part;
}

void PedLogic_BloodSpurtDirection(int bodyPart, const float up[3], const float right[3], const float forward[3], float out[3])
{
	int i;

	switch (bodyPart) {
	case PED_HEAD:
		for (i = 0; i < 3; i++)
			out[i] = 0.1f * up[i];
		break;
	case PED_UPPERARML:
	case PED_UPPERARMR:
		for (i = 0; i < 3; i++)
			out[i] = 0.04f * up[i] - 0.04f * right[i];
		break;
	case PED_UPPERLEGL:
	case PED_UPPERLEGR:
		for (i = 0; i < 3; i++)
			out[i] = 0.04f * up[i] + 0.05f * forward[i];
		break;
	default:
		for (i = 0; i < 3; i++)
			out[i] = 0.0f;
		break;
	}
}

int PedLogic_ExitCarFollowUp(bool fleeAfterExiting, bool hasVehicle, bool wanderAfterExiting, bool goingToKillCarJacker, bool noObjective, bool notMissionChar, bool isIdle, bool isPlayer)
{
	if (fleeAfterExiting && hasVehicle)
		return PED_EXITCAR_FLEE;
	else if (wanderAfterExiting)
		return PED_EXITCAR_WANDER;
	else if (goingToKillCarJacker)
		return PED_EXITCAR_KILL_CARJACKER;
	else if (noObjective && notMissionChar && isIdle && !isPlayer)
		return PED_EXITCAR_WANDER_RANDOM;
	return PED_EXITCAR_NOTHING;
}

float PedLogic_BusExitHeading(float busHeading, bool frontLeftDoor)
{
	const float pi = (float)3.14159265358979323846;
	const float halfpi = pi / 2;

	if (frontLeftDoor)
		return halfpi + busHeading;
	return busHeading - halfpi;
}

bool PedLogic_IsGoOnFootObjective(int objective)
{
	return objective == OBJECTIVE_RUN_TO_AREA || objective == OBJECTIVE_GOTO_CHAR_ON_FOOT ||
		objective == OBJECTIVE_SPRINT_TO_AREA || objective == OBJECTIVE_KILL_CHAR_ON_FOOT;
}

bool PedLogic_KeepsObjectiveInCar(int objective)
{
	switch (objective) {
	case OBJECTIVE_KILL_CHAR_ON_FOOT:
	case OBJECTIVE_KILL_CHAR_ANY_MEANS:
	case OBJECTIVE_LEAVE_CAR:
	case OBJECTIVE_FOLLOW_CAR_IN_CAR:
	case OBJECTIVE_GOTO_AREA_ANY_MEANS:
	case OBJECTIVE_GOTO_AREA_ON_FOOT:
	case OBJECTIVE_RUN_TO_AREA:
	case OBJECTIVE_GOTO_SEAT_ON_FOOT:
	case OBJECTIVE_GOTO_ATM_ON_FOOT:
	case OBJECTIVE_GOTO_BUS_STOP_ON_FOOT:
	case OBJECTIVE_GOTO_PIZZA_ON_FOOT:
	case OBJECTIVE_GOTO_SHELTER_ON_FOOT:
	case OBJECTIVE_SPRINT_TO_AREA:
	case OBJECTIVE_GOTO_ICE_CREAM_VAN_ON_FOOT:
		return true;
	default:
		return false;
	}
}

int PedLogic_PassengerSeat(bool isBike, bool isBus, int door)
{
	if (isBike)
		return 0;
	if (isBus)
		return -1;
	switch (door) {
	case CAR_DOOR_RF:
		return 0;
	case CAR_DOOR_RR:
		return 2;
	case CAR_DOOR_LR:
		return 1;
	default:
		return -1;
	}
}

void PedLogic_AnimMoved(float deltaX, float deltaY, float rotation, float timeStep, float moved[2])
{
	float fx = sinf(rotation);
	float fy = cosf(rotation);
	float x = deltaX * fy - deltaY * fx;
	float y = deltaX * fx + deltaY * fy;
	float scale = timeStep >= 0.01f ? 1 / timeStep : 1 / 100.0f;

	moved[0] = x * scale;
	moved[1] = y * scale;
}

void PedLogic_SpeedFromAngle(float velocity, float angle, float speed[2])
{
	speed[0] = -velocity * sinf(angle);
	speed[1] = velocity * cosf(angle);
}
float PedLogic_AttractorHeading(float rotationCur, float attractorHeading)
{
	float headingDistance = rotationCur - attractorHeading;
	float s = sinf(headingDistance);
	float c = cosf(headingDistance);

	if (s > 0.0f) {
		if (c > 0.0f)
			return rotationCur - asinf(s);
		return rotationCur - acosf(c);
	}
	if (c > 0.0f)
		return rotationCur - asinf(s);
	return rotationCur + acosf(c);
}
float PedLogic_BikeLeanLRBlend(float amount, float target, float timeStep)
{
	float timeBlend = powf(0.86f, timeStep);

	return amount * timeBlend + (1.0 - timeBlend) * target;
}

float PedLogic_BikeLeanUDBlend(float amount, float target, float timeStep)
{
	float timeBlend = powf(0.89f, timeStep);

	return (timeBlend * amount) + ((1.0f - timeBlend) * target);
}
float PedLogic_AirResistanceFactor(float timeStep)
{
	return powf(0.95f, timeStep);
}

void PedLogic_StuckPushDirection(float moveSpeedX, float moveSpeedY, float groundSpeed, float *x, float *y)
{
	float d = 0.001f > groundSpeed ? 0.001f : groundSpeed;

	*x = -moveSpeedX / d;
	*y = -moveSpeedY / d;
}
bool PedLogic_FacesAttractor(float rotationCur, float attractorHeading, float acceptableOffset)
{
	return fabsf(rotationCur - attractorHeading) < acceptableOffset;
}

bool PedLogic_NotFacingAttractor(float rotationCur, float attractorHeading, float acceptableOffset)
{
	const float twopi = (float)3.14159265358979323846 * 2;

	return fabsf(rotationCur - attractorHeading) >= acceptableOffset &&
		fabsf(rotationCur - attractorHeading + twopi) >= acceptableOffset &&
		fabsf(rotationCur - attractorHeading - twopi) >= acceptableOffset;
}

float PedLogic_ClimbHeight(float targetZ, float currentZ)
{
	float v = targetZ - currentZ;

	return 0.0f > v ? 0.0f : v;
}
float PedLogic_AnimTimeStep(float timeStep)
{
	return timeStep > 0.1f ? timeStep : 0.1f;
}

float PedLogic_SmoothZDown(float currentZ, float neededZ, float timeLeft, float adjustedTimeStep)
{
	return currentZ - (currentZ - neededZ) / (timeLeft / adjustedTimeStep);
}

float PedLogic_SmoothZUp(float currentZ, float neededZ, float timeLeft, float adjustedTimeStep)
{
	return (neededZ - currentZ) / (timeLeft / adjustedTimeStep) + currentZ;
}
float PedLogic_FightMoveMult(float damageMult, bool fightingWithWeapon, bool isGroundKick, bool victimIsDying)
{
	float v;

	if (fightingWithWeapon) {
		v = damageMult * 0.02f;
		return v < 1.0f ? v : 1.0f;
	} else if (isGroundKick) {
		v = damageMult * 0.6f;
		return v < 4.0f ? v : 4.0f;
	}
	if (!victimIsDying || damageMult >= 20) {
		return damageMult;
	}
	v = damageMult * 2.0f;
	return v < 14.0f ? v : 14.0f;
}

float PedLogic_DriveBlend(float steerAngle, float scale)
{
	float v = steerAngle * scale / 61.0f;

	return v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v;
}
bool PedLogic_PadIsWalking(float padWalkX, float padWalkY)
{
	return fabsf(padWalkX) > 0.0f || fabsf(padWalkY) > 0.0f;
}

void PedLogic_FacingDirection(float angle, float dir[2])
{
	dir[0] = -sinf(angle);
	dir[1] = cosf(angle);
}
bool PedLogic_KeepsPushing(uint32_t stateTimer, float adjustedTimeStep, float forceDirMagSqr, int collisionRecords, float forceDirZ)
{
	return (stateTimer <= 50.0f / (4.0f * adjustedTimeStep) || stateTimer * 0.01f <= forceDirMagSqr)
		&& (collisionRecords <= 1 || stateTimer <= 50.0f / (2.0f * adjustedTimeStep) || stateTimer * 1.0f / 250.0f <= fabsf(forceDirZ));
}

bool PedLogic_PushedAgainstWallLong(uint32_t stateTimer, float adjustedTimeStep, float forceDirZ)
{
	return stateTimer > 50.0f / (2.0f * adjustedTimeStep) && stateTimer * 1.0f / 250.0f > fabsf(forceDirZ);
}
float PedLogic_CarHitMassScale(float carMass)
{
	float v = carMass / 1400.0f;

	return v < 1.0f ? v : 1.0f;
}

float PedLogic_BlendRemaining(float blendDelta, float limit, float timeStep, float assocBlendDelta, float assocBlendAmount)
{
	float part = timeStep * 0.02f * assocBlendDelta + assocBlendAmount;

	return blendDelta - (limit < part ? limit : part);
}
bool PedLogic_WarpHeightAcceptable(float correctedZ, float warpToZ, float testedZ)
{
	return fabsf(correctedZ - warpToZ) < 3.0f || fabsf(correctedZ - testedZ) < 3.0f;
}

int PedLogic_WarpTryCount(float halfOfDist)
{
	int d = (int)halfOfDist;

	return 10 < d ? 10 : d;
}

float PedLogic_AngleDifference(float angleA, float angleB)
{
	const float pi = (float)3.14159265358979323846;
	float angleDiff = fabsf(angleA - angleB);

	if (angleDiff > pi)
		angleDiff = 2 * pi - angleDiff;
	return angleDiff;
}
bool PedLogic_IsInFrontOfBike(float angleDiff)
{
	const float pi = (float)3.14159265358979323846;

	if (angleDiff > pi)
		angleDiff -= pi * 2;
	else if (angleDiff < -pi)
		angleDiff += pi * 2;
	return fabsf(angleDiff) < 30.f * pi / 180.0f;
}
int PedLogic_ObstacleKind(bool hasObstacle, bool isBuilding, bool isVehicle)
{
	if (!hasObstacle)
		return 0;
	if (isBuilding)
		return 1;
	if (isVehicle)
		return 2;
	return 3;
}
int PedLogic_SexFrequencyAfterService(int frequency)
{
	int reduced = frequency >= 350 ? frequency - 30 : frequency - 10;
	return 250 > reduced ? 250 : reduced;
}
float PedLogic_DriverHealthAfterService(float maxHealth, float health)
{
	float cap = maxHealth + 25.0f;
	float raised = 1.0f + health;
	return cap < raised ? cap : raised;
}
float PedLogic_ReloadLoopRestartTime(float animLoopStart)
{
	float t = animLoopStart - 0.04f;
	return t > 0.0f ? t : 0.0f;
}
float PedLogic_EnterCarZBlend(float currentTime, float animLength)
{
	float v = 2.0f * currentTime / animLength;
	return 1.0f < v ? 1.0f : v;
}
float PedLogic_MaxDistToKeep(float wepRange)
{
	float v = 0.9f * wepRange;
	return 0.8f > v ? 0.8f : v;
}
float PedLogic_AdjustedTimeStep(float timeStep)
{
	return timeStep > 0.01f ? timeStep : 0.01f;
}
float PedLogic_LimitedWindMult(float windMult, float anyDirectionShift)
{
	float limit = fabsf(anyDirectionShift - 1.0f);

	return windMult < limit ? windMult : limit;
}
float PedLogic_JumpOffsetScale(float collPower)
{
	float v = 2.0f / collPower;

	return v < 4.0f ? v : 4.0f;
}
//- rouz edit (Claude)
