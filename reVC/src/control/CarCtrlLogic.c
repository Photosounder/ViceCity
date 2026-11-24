//+ rouz edit (Claude)
#include <math.h>
#include <stddef.h>
#include "CarCtrlLogic.h"
#include "General.h"

CVehicleStruct *CarCtrl_FindClosestRemovableCar(const CPool *vehiclePool, float cx, float cy, float cz, CarCtrlVehicleTest isInteresting, CarCtrlVehicleTest isTargetedByCrane)
{
	int i = CPool_GetSize(vehiclePool);
	float md = 10000000.0f;
	CVehicleStruct *pClosestVehicle = NULL;
	while (i--) {
		CVehicleStruct *pVehicle = (CVehicleStruct *)CPool_GetSlot(vehiclePool, i);
		float dx, dy, dz, distance;
		if (!pVehicle)
			continue;
		if (isInteresting(pVehicle) || pVehicle->vehicle.bIsLocked)
			continue;
		if (!VehicleLogic_CanBeDeleted(&pVehicle->vehicle) || isTargetedByCrane(pVehicle))
			continue;
		// Distance from the point to the vehicle, the first strictly closer one wins
		dx = cx - pVehicle->base.base.placeable.m_matrix.px;
		dy = cy - pVehicle->base.base.placeable.m_matrix.py;
		dz = cz - pVehicle->base.base.placeable.m_matrix.pz;
		distance = sqrtf(dx * dx + dy * dy + dz * dz);
		if (distance < md) {
			md = distance;
			pClosestVehicle = pVehicle;
		}
	}
	return pClosestVehicle;
}

int32_t CarCtrl_CountCarsOfType(const CPool *vehiclePool, int32_t modelIndex)
{
	int32_t total = 0;
	int i;
	for (i = CPool_GetSize(vehiclePool) - 1; i >= 0; i--) {
		CVehicleStruct *pVehicle = (CVehicleStruct *)CPool_GetSlot(vehiclePool, i);
		if (!pVehicle)
			continue;
		if (pVehicle->base.base.entity.m_modelIndex == modelIndex)
			total++;
	}
	return total;
}

void CarCtrl_GetRandomOffsetForVehicle(const CVehicleStruct *vehicle, bool next, float out[3])
{
	int32_t seed = ((next ? vehicle->vehicle.AutoPilot.m_nNextPathNodeInfo : vehicle->vehicle.AutoPilot.m_nCurrentPathNodeInfo) + vehicle->base.base.entity.m_randomSeed) & 7;
	out[0] = (seed - 3) * 0.009f;
	out[1] = ((seed >> 3) - 3) * 0.009f;
	out[2] = 0.0f;
}
float CarCtrl_FindSpeedMultiplier(float limitedAngleChange, float minAngle, float maxAngle, float coef)
{
	float angle = fabsf(limitedAngleChange);
	float n = angle - minAngle;
	float d;
	float mult;

	// Linear between the two angles
	n = n > 0.0f ? n : 0.0f;
	d = maxAngle - minAngle;
	mult = 1.0f - n / d * (1.0f - coef);
	if (n > d)
		return coef;
	return mult;
}

float CarCtrl_FindSpeedMultiplierWithSpeedFromNodes(int type)
{
	// Faster road node types
	switch (type) {
	case 1: return 1.5f;
	case 2: return 2.0f;
	}
	return 1.0f;
}

int CarCtrl_ChooseRatingFromThresholds(int rnd, const int16_t *thresholds, int numClasses, int firstRating, bool returnIndex)
{
	int i;

	// The first class whose threshold is above the random number, the last one otherwise
	for (i = 0; i < numClasses - 1; i++) {
		if (rnd < thresholds[i])
			return returnIndex ? i : firstRating + i;
	}
	return firstRating + numClasses - 1;
}
void CarCtrl_AddToLoadedVehicleArray(int32_t *loadedRow, int32_t *freqRow, int32_t *numLoaded, int32_t mi, int32_t freq)
{
	// Append the model, its sum includes the sum of the model before it
	loadedRow[*numLoaded] = mi;
	freqRow[*numLoaded] = freq;
	if (*numLoaded)
		freqRow[*numLoaded] += freqRow[*numLoaded - 1];
	(*numLoaded)++;
}

void CarCtrl_RemoveFromLoadedVehicleArray(int32_t *loadedRow, int32_t *freqRow, int32_t *numLoaded, int32_t mi)
{
	int index = 0;
	int32_t freq;

	// Find the model
	while (loadedRow[index] != -1) {
		if (loadedRow[index] == mi)
			break;
		index++;
	}

	// Its own frequency is its sum without the one before
	freq = freqRow[index];
	if (index > 0)
		freq -= freqRow[index - 1];

	// Close the gap and lower the sums of the models that followed
	while (loadedRow[index + 1] != -1) {
		loadedRow[index] = loadedRow[index + 1];
		freqRow[index] = freqRow[index + 1] - freq;
		index++;
	}
	--(*numLoaded);
}

int32_t CarCtrl_ChooseCarModel(const int32_t *loadedRow, const int32_t *freqRow, int32_t numLoaded, int32_t (*randomRange)(int32_t, int32_t))
{
	int32_t rnd, index = 0;

	if (numLoaded == 0)
		return -1;

	// The first model whose sum reaches the random number
	rnd = randomRange(0, freqRow[numLoaded - 1]);
	while (rnd > freqRow[index])
		index++;
	return loadedRow[index];
}

bool CarCtrl_IsInPointerList(void *const *list, int count, const void *item)
{
	int i;
	for (i = 0; i < count; i++) {
		if (list[i] == item)
			return true;
	}
	return false;
}

void CarCtrl_RemoveFromPointerList(void **list, int count, const void *item)
{
	int i;
	for (i = 0; i < count; i++) {
		if (list[i] == item)
			list[i] = NULL;
	}
}
bool CarCtrl_AdjustTrafficSpeed(float maxSpeed, float curSpeed, float timeStep, float *newSpeed)
{
	if (maxSpeed >= curSpeed) {
		// Speed up by at most 0.05 per step
		if (maxSpeed > curSpeed) {
			float limit = curSpeed + 0.05f * timeStep;
			*newSpeed = maxSpeed < limit ? maxSpeed : limit;
			return true;
		}
	} else if (curSpeed != 0.0f) {
		// Stop when nearly stopped, otherwise brake by 0.7 per step but not below the speed allowed
		if (curSpeed < 0.1f) {
			*newSpeed = 0.0f;
		} else {
			float limit = curSpeed - 0.7f * timeStep;
			*newSpeed = maxSpeed > limit ? maxSpeed : limit;
		}
		return true;
	}
	return false;
}
void CarCtrl_WrapAngleUp(float *angle)
{
	const float pi = (float)3.14159265358979323846;

	while (*angle < -pi)
		*angle += pi*2;
}

void CarCtrl_WrapAngleDown(float *angle)
{
	const float pi = (float)3.14159265358979323846;

	while (*angle > pi)
		*angle -= pi*2;
}

float CarCtrl_PlaneSteer(float difference, float planeSteer, float timeStep)
{
	float steer = difference > 0.0f ? 0.04f : -0.04f;
	float steerChange, maxChange;

	if (fabsf(difference) < 0.2f)
		steer *= 5.0f * fabsf(difference);
	planeSteer *= powf(0.96f, timeStep);
	steerChange = steer - planeSteer;
	maxChange = 0.003f * timeStep;
	if (fabsf(steerChange) < maxChange)
		planeSteer = steer;
	else if (steerChange < 0.0f)
		planeSteer -= maxChange;
	else
		planeSteer += maxChange;
	return planeSteer;
}

float CarCtrl_PlaneSpeedSplit(float timeStep)
{
	return 1.0f - powf(0.95f, timeStep);
}
float CarCtrl_HeliSpeedScale(float distanceToTarget)
{
	if (distanceToTarget <= 100.0f) {
		if (distanceToTarget > 75.0f)
			return 0.7f;
		if (distanceToTarget > 10.0f)
			return 0.4f;
		return 0.2f;
	}
	return 1.0f;
}

float CarCtrl_HeliZSpeed(float zSpeed, float zSpeedTarget, float timeStep, bool destroyed)
{
	float change = zSpeedTarget - zSpeed;
	float maxChange = 0.001f * timeStep;

	if (destroyed)
		return zSpeed;
	if (fabsf(change) < maxChange)
		return zSpeedTarget;
	if (change < 0.0f)
		return zSpeed + -maxChange;
	return zSpeed + 1.5f * maxChange;
}

float CarCtrl_HeliTurnSpeedTarget(float angleToTurn)
{
	if (fabsf(angleToTurn) <= 0.4f)
		return 0.0f;
	if (angleToTurn < 0.0f)
		return -0.03f;
	return 0.03f;
}

float CarCtrl_ApproachValue(float value, float target, float limit)
{
	float change = target - value;

	if (fabsf(change) < limit)
		return target;
	if (change < 0.0f)
		return value - limit;
	return value + limit;
}
float CarCtrl_BoatAccel(float speedDiff, float currentSpeed)
{
	if (speedDiff <= 0.0f)
		return speedDiff < -5.0f ? -0.2f : -0.1f;
	if (speedDiff / currentSpeed > 0.25f)
		return 1.0f;
	return 1.0f - (0.25f - speedDiff / currentSpeed) * 4.0f;
}
void CarCtrl_BlockForwardBackPedals(float dpPlayerAndForward, float dpDiffAndRight, float dpDiffAndForward, float dpSpeedAndForward, float *accel, float *brake, bool *handbrake)
{
	float multiplier = dpPlayerAndForward * dpDiffAndRight + dpDiffAndForward - dpSpeedAndForward * dpDiffAndRight;

	if (multiplier > 0) {
		float a = 0.1f * multiplier;
		*accel = 1.0f < a ? 1.0f : a;
		*brake = 0.0f;
	} else if (dpSpeedAndForward > 0) {
		float b = -0.1f * multiplier;
		*accel = 0.0f;
		*brake = 1.0f < b ? 1.0f : b;
		if (*brake > 0.95f)
			*handbrake = true;
	} else {
		float a = 0.1f * multiplier;
		*accel = -1.0f > a ? -1.0f : a;
		*brake = 0.0f;
	}
}
void CarCtrl_RoadSlopeOrientation(float midX, float midY, float targetX, float targetY, float aheadZ, float behindZ, float right[3], float forward[3])
{
	float angleZ = atan2f((aheadZ - behindZ) / 3, 1.0f);
	float cosZ = cosf(angleZ);
	float sinZ = sinf(angleZ);

	right[0] = (targetY - midY) / 3;
	right[1] = -(targetX - midX) / 3;
	right[2] = 0.0f / 3;
	forward[0] = -cosZ * right[1];
	forward[1] = cosZ * right[0];
	forward[2] = sinZ;
}
bool CarCtrl_WeaveOutOfReach(float relativeSpeedDotDiff, float otherRadius, float ownRadius, float distance)
{
	return relativeSpeedDotDiff * 110.0f - otherRadius - ownRadius < distance;
}

float CarCtrl_WeaveLengthToEvade(float angleDiff, float otherMaxY, float otherMaxX, float ownMaxX, float distance, float widthCoef)
{
	float lenProjection = fabsf(otherMaxY * sinf(angleDiff));
	float widthProjection = fabsf(otherMaxX * cosf(angleDiff));

	return (2 * (lenProjection + widthProjection) + widthCoef * 2 * ownMaxX) / distance;
}
static float CarCtrl_LimitRadianAngle(float angle)
{
	const float pi = (float)3.14159265358979323846;

	while (angle < -pi)
		angle += pi * 2;
	while (angle > pi)
		angle -= pi * 2;
	return angle;
}

void CarCtrl_ApplyWeaveAngles(float angleToObstacle, float lengthToEvade, float *angleToWeaveLeft, float *angleToWeaveRight)
{
	float diffToLeftAngle = fabsf(CarCtrl_LimitRadianAngle(angleToObstacle - *angleToWeaveLeft));
	float angleToWeave = lengthToEvade / 2;
	float diffToRightAngle;

	if (diffToLeftAngle < angleToWeave) {
		*angleToWeaveLeft = angleToObstacle - angleToWeave;
		CarCtrl_WrapAngleUp(angleToWeaveLeft);
	}
	diffToRightAngle = fabsf(CarCtrl_LimitRadianAngle(angleToObstacle - *angleToWeaveRight));
	if (diffToRightAngle < angleToWeave) {
		*angleToWeaveRight = angleToObstacle + angleToWeave;
		CarCtrl_WrapAngleDown(angleToWeaveRight);
	}
}
float CarCtrl_PickWeaveAngle(float angleForward, float angleToTarget, float angleToWeaveLeft, float angleToWeaveRight)
{
	const float halfPi = (float)3.14159265358979323846 / 2;
	float angleDiffFromActualToTarget = CarCtrl_LimitRadianAngle(angleForward - angleToTarget);
	float angleToBisectActualToTarget = CarCtrl_LimitRadianAngle(angleToTarget + angleDiffFromActualToTarget / 2);
	float angleDiffLeft = fabsf(CarCtrl_LimitRadianAngle(angleToWeaveLeft - angleToBisectActualToTarget));
	float angleDiffRight = fabsf(CarCtrl_LimitRadianAngle(angleToWeaveRight - angleToBisectActualToTarget));

	if (angleDiffLeft > halfPi && angleDiffRight > halfPi)
		return angleToBisectActualToTarget;
	if (fabsf(angleDiffLeft - angleDiffRight) < 0.08f)
		return angleToWeaveRight;
	return angleDiffLeft < angleDiffRight ? angleToWeaveLeft : angleToWeaveRight;
}
void CarCtrl_SlabProximity(float distance, float projection, float positiveHalf, float negativeHalf, float fullSize, float *base, float *full)
{
	*base = 0.0f;
	*full = 1.0f;
	if (distance > positiveHalf) {
		if (projection < 0.0f) {
			float proximity = -(distance - positiveHalf) / projection;
			if (proximity < 1.0f) {
				float f = proximity - fullSize / projection;
				*base = proximity;
				*full = 1.0f < f ? 1.0f : f;
			} else {
				*base = 1.0f;
			}
		} else {
			*base = 1.0f;
			*full = 1.0f;
		}
	} else if (distance < -negativeHalf) {
		if (projection > 0.0f) {
			float proximity = -(distance + negativeHalf) / projection;
			if (proximity < 1.0f) {
				float f = proximity + fullSize / projection;
				*base = proximity;
				*full = 1.0f < f ? 1.0f : f;
			} else {
				*base = 1.0f;
			}
		} else {
			*base = 1.0f;
			*full = 1.0f;
		}
	} else if (projection > 0.0f) {
		*full = (positiveHalf - distance) / projection;
	} else if (projection < 0.0f) {
		*full = -(negativeHalf + distance) / projection;
	}
}
void CarCtrl_CornerOffset(int corner, float offsetX, float offsetY, float width, float length, float backLength, float forwardX, float forwardY, float *x, float *y)
{
	switch (corner) {
	case 0: // Front right corner
		*x = offsetX + width * forwardY + length * forwardX;
		*y = offsetY + length * forwardY - width * forwardX;
		break;
	case 1: // Front left corner
		*x = offsetX + -width * forwardX + length * forwardX;
		*y = offsetY + length * forwardY + width * forwardX;
		break;
	case 2: // Rear right corner
		*x = offsetX + width * forwardY - backLength * forwardX;
		*y = offsetY - backLength * forwardY - width * forwardX;
		break;
	case 3: // Rear left corner
		*x = offsetX - width * forwardY - backLength * forwardX;
		*y = offsetY - backLength * forwardY + width * forwardX;
		break;
	default:
		break;
	}
}
float CarCtrl_SlowForProximity(float minProximity, float curSpeed, float *speed, bool *slowed)
{
	*slowed = false;
	if (minProximity >= 0.0f && minProximity < 1.5f) {
		float limit;

		minProximity = (minProximity - 0.2f) / 1.3f;
		minProximity = 0.0f > minProximity ? 0.0f : minProximity;
		*slowed = true;
		limit = minProximity * curSpeed;
		*speed = *speed < limit ? *speed : limit;
	}
	return minProximity;
}
int CarCtrl_PedEvasion(float dotVelocity, float sideLength, float sidewaysDistance)
{
	if (dotVelocity > 0.3f)
		return sideLength + 0.1f < sidewaysDistance ? CARCTRL_EVADE_STEP : CARCTRL_EVADE_DIVE;
	if (dotVelocity > 0.1f)
		return sideLength - 0.5f < sidewaysDistance ? CARCTRL_EVADE_STEP : CARCTRL_EVADE_DIVE;
	return CARCTRL_EVADE_NONE;
}

float CarCtrl_PedVisibilityAngle(float angleCarToPed, float pedHeading)
{
	const float pi = (float)3.14159265358979323846;
	float d = angleCarToPed - pedHeading;
	float visibilityAngle = d < 0 ? -d : d;

	if (visibilityAngle > pi)
		visibilityAngle = pi * 2 - visibilityAngle;
	return visibilityAngle;
}
void CarCtrl_LanePosition(float linkX, float linkY, float lane, float forwardX, float forwardY, float laneWidth, float pos[2])
{
	pos[0] = linkX + (lane * laneWidth) * forwardY;
	pos[1] = linkY - (lane * laneWidth) * forwardX;
}
int CarCtrl_PathDirection(float prevX, float prevY, float curX, float curY, float nextX, float nextY)
{
	float prevToCurX = curX - prevX;
	float prevToCurY = curY - prevY;
	float curToNextX = nextX - curX;
	float curToNextY = nextY - curY;
	float distPrevToCur = sqrtf(prevToCurX * prevToCurX + prevToCurY * prevToCurY);
	float sin_a, cos_a, distCurToNext, sin_b, cos_b, sin_direction;

	if (distPrevToCur == 0.0f)
		return 0;
	// Angle between the previous and the next direction: a is the angle between the y axis and the first one, b the angle between the x axis and the second one
	sin_a = prevToCurX / distPrevToCur;
	cos_a = prevToCurY / distPrevToCur;
	distCurToNext = sqrtf(curToNextX * curToNextX + curToNextY * curToNextY);
	if (distCurToNext == 0.0f)
		return 0;
	sin_b = curToNextY / distCurToNext;
	cos_b = curToNextX / distCurToNext;
	sin_direction = sin_a * sin_b - cos_a * cos_b;
	if (sin_direction > 0.77f)
		return 4;
	if (sin_direction < -0.77f)
		return 2;
	return 1;
}
float CarCtrl_LinkAngleChange(float angleToTarget, float nodeX, float nodeY, float testNodeX, float testNodeY)
{
	float angle = General_GetATanOfXY(testNodeX - nodeX, testNodeY - nodeY);

	angle = CarCtrl_LimitRadianAngle(angle - angleToTarget);
	return angle < 0 ? -angle : angle;
}
int CarCtrl_AllowedDirections(int lane, int lanes)
{
	int allowed = 0;

	if (lane == 0)
		allowed |= 4;
	if (lane == lanes - 1)
		allowed |= 2;
	if (lanes < 3 || allowed == 0)
		allowed |= 1;
	return allowed;
}

void CarCtrl_LinkLanes(bool linkStartsAtNode, int leftLanes, int rightLanes, int *lanes, bool *isOneWay)
{
	if (linkStartsAtNode) {
		*lanes = leftLanes;
		*isOneWay = rightLanes == 0;
	} else {
		*lanes = rightLanes;
		*isOneWay = leftLanes == 0;
	}
}

void CarCtrl_LinkOneWayFlags(bool linkStartsAtNode, int leftLanes, int rightLanes, bool *goingAgainstOneWay, bool *isOneWayRoad)
{
	*goingAgainstOneWay = linkStartsAtNode ? rightLanes == 0 : leftLanes == 0;
	*isOneWayRoad = linkStartsAtNode ? leftLanes == 0 : rightLanes == 0;
}
bool CarCtrl_NextNodeAcceptable(bool nextDeadEnd, bool prevDeadEnd, bool nextDisabled, bool prevDisabled, bool nextBetweenLevels, bool prevBetweenLevels, bool stayInLevel, bool goingAgainstOneWay, bool onOneWayRoad, bool nextIsOneWayRoad)
{
	return (!nextDeadEnd || prevDeadEnd) &&
		(!nextDisabled || prevDisabled) &&
		(!nextBetweenLevels || prevBetweenLevels || !stayInLevel) &&
		!goingAgainstOneWay && (!onOneWayRoad || !nextIsOneWayRoad);
}

bool CarCtrl_NextNodeAcceptableRelaxed(bool nextDisabled, bool prevDisabled, bool nextBetweenLevels, bool prevBetweenLevels, bool stayInLevel, bool goingAgainstOneWay)
{
	return (!nextDisabled || prevDisabled) &&
		(!nextBetweenLevels || prevBetweenLevels || !stayInLevel) &&
		!goingAgainstOneWay;
}
int CarCtrl_NextLane(int nextLane, int currentLane, int lanesOnNextNode, int laneChange)
{
	if (lanesOnNextNode >= 0) {
		nextLane += laneChange;
		nextLane = lanesOnNextNode - 1 < nextLane ? lanesOnNextNode - 1 : nextLane;
		nextLane = 0 > nextLane ? 0 : nextLane;
		return nextLane;
	}
	return currentLane;
}
int32_t CarCtrl_CurveTime(float speedScaleFactor, float maxTrafficSpeed)
{
	return (int32_t)(speedScaleFactor * (1000.0f / maxTrafficSpeed));
}
float CarCtrl_DespawnThreshold(bool useOnscreenRange, float offscreenRange, float onscreenRange, float generationDistMultiplier, bool checkLookingDown, float cameraForwardZ, bool extendedRange, float extendedRangeMultiplier)
{
	float threshold = offscreenRange;

	if (useOnscreenRange)
		threshold = onscreenRange * generationDistMultiplier;
	if (checkLookingDown && cameraForwardZ < -0.9f)
		threshold = 70.0f;
	if (extendedRange)
		threshold *= extendedRangeMultiplier;
	return threshold;
}
float CarCtrl_AirResistance(float resistance, float timeStep)
{
	return powf(resistance, timeStep);
}

void CarCtrl_OrientationDirection(float orientation, float dir[2])
{
	dir[0] = cosf(orientation);
	dir[1] = sinf(orientation);
}
float CarCtrl_LimitSteer(float steerAngle, float maxAngle)
{
	float lower = -maxAngle > steerAngle ? -maxAngle : steerAngle;

	return maxAngle < lower ? maxAngle : lower;
}

float CarCtrl_SpeedNodesMultiplier(float distanceToNextNode, int cruiseSpeed, float wideMultiplier, float slowingDistance)
{
	if (distanceToNextNode > slowingDistance || cruiseSpeed < 12)
		return 1.0f;
	return 1.0f -
		(1.0f - distanceToNextNode / slowingDistance) *
		(1.0f - wideMultiplier);
}
void CarCtrl_PathAccelBrake(float speed, float speedDifference, float currentForwardSpeed, float *accel, float *brake)
{
	float v;

	if (speed < 0.05f && speedDifference < 0.03f) {
		*brake = 1.0f;
		*accel = 0.0f;
	} else if (speedDifference <= 0.0f) {
		v = -speedDifference * 0.05f;
		*brake = 0.5f < v ? 0.5f : v;
		*accel = 0.0f;
	} else if (currentForwardSpeed < 2.0f) {
		v = speedDifference * 0.25f;
		*brake = 0.0f;
		*accel = 1.0f < v ? 1.0f : v;
	} else {
		v = speedDifference * 0.125f;
		*brake = 0.0f;
		*accel = 1.0f < v ? 1.0f : v;
	}
}

void CarCtrl_TargetAccelBrake(float speedDiff, float currentSpeed, float *accel, float *brake)
{
	float v;

	if (speedDiff <= 0.0f) {
		v = -speedDiff / 20.0f;
		*accel = 0.0f;
		*brake = 0.5f < v ? 0.5f : v;
	} else if (currentSpeed < 25.0f) {
		v = speedDiff * 0.1f;
		*accel = 1.0f < v ? 1.0f : v;
		*brake = 0.0f;
	} else {
		*accel = 1.0f;
		*brake = 0.0f;
	}
}
void CarCtrl_RegisterKept(void **cars, uint32_t *times, int count, void *vehicle, uint32_t now)
{
	uint32_t oldestTime = UINT32_MAX;
	int oldestIndex = 0;
	int i;

	for (i = 0; i < count; i++) {
		if (cars[i] == vehicle) {
			times[i] = now;
			return;
		}
	}
	for (i = 0; i < count; i++) {
		if (!cars[i]) {
			cars[i] = vehicle;
			times[i] = now;
			return;
		}
	}
	for (i = 0; i < count; i++) {
		if (cars[i] && times[i] < oldestTime) {
			oldestTime = times[i];
			oldestIndex = i;
		}
	}
	cars[oldestIndex] = vehicle;
	times[oldestIndex] = now;
}
int32_t CarCtrl_MinCurveTime(int32_t time)
{
	return 10 > time ? 10 : time;
}

int8_t CarCtrl_ClampNextLane(int8_t lane, int lanesOnNextNode)
{
	int v = (lanesOnNextNode - 1) < lane ? (lanesOnNextNode - 1) : lane;

	return (int8_t)(0 > v ? 0 : v);
}
float CarCtrl_CombineProximity(float proximity, float baseWidthProximity, float fullWidthProximity, float baseLengthProximity, float fullLengthProximity)
{
	float baseProximity = baseWidthProximity > baseLengthProximity ? baseWidthProximity : baseLengthProximity;

	if (baseProximity < fullWidthProximity && baseProximity < fullLengthProximity)
		return proximity < baseProximity ? proximity : baseProximity;
	return proximity;
}
float CarCtrl_PositionBetweenNodes(float distanceBetweenNodes, float carLength, float positionBetweenNodes)
{
	float lower, upper;

	if (distanceBetweenNodes / 2 < carLength)
		return 0.5f;
	lower = carLength / distanceBetweenNodes > positionBetweenNodes ? carLength / distanceBetweenNodes : positionBetweenNodes;
	upper = 1.0f - carLength / distanceBetweenNodes;
	return upper < lower ? upper : lower;
}
float CarCtrl_PedSlowdownSpeed(float speed, float distanceUntilHit, float curSpeed)
{
	float v = fabsf(distanceUntilHit - 1.0f) / 10.0f * curSpeed;

	return speed < v ? speed : v;
}

float CarCtrl_ReleasedSpeed(float speed, float curSpeed)
{
	float v = curSpeed / 5;

	return v > speed ? v : speed;
}
bool CarCtrl_TooFarVertically(float heightDifference, float limit)
{
	return fabsf(heightDifference) >= limit;
}

float CarCtrl_SpeedMultiplier(float styleMultiplier, float angleMultiplier, float nodesMultiplier)
{
	float m = angleMultiplier < nodesMultiplier ? angleMultiplier : nodesMultiplier;

	return styleMultiplier < m ? styleMultiplier : m;
}
float CarCtrl_WeaveScanDistance(float speed2D)
{
	float v = speed2D / 0.4f + 1.0f;

	return (2.0f < v ? 2.0f : v) * 12.0f;
}

float CarCtrl_BoatProjection(float distanceToPlayer)
{
	float v = distanceToPlayer / 20.0f;

	return v < 2.0f ? v : 2.0f;
}
float CarCtrl_ForwardAngle(float forwardX, float forwardY)
{
	float x = forwardX;
	float y = forwardY;
	float sq = x * x + y * y;

	if (sq > 0.0f) {
		float invsqrt = 1.0f / sqrtf(sq);
		x *= invsqrt;
		y *= invsqrt;
	} else
		x = 1.0f;
	return General_GetATanOfXY(x, y);
}
//- rouz edit (Claude)
