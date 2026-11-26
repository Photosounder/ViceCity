//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "EntityLayout.h"
#include "VehicleLogic.h"
#include "Pool.h"

#ifdef __cplusplus
extern "C" {
#endif
// Vehicle pool helpers of the traffic code, the tests that need other systems are supplied as callbacks
typedef bool (*CarCtrlVehicleTest)(void *vehicle);
// Closest vehicle to a point that may be removed to make room in the pool, or NULL
CVehicleStruct *CarCtrl_FindClosestRemovableCar(const CPool *vehiclePool, float cx, float cy, float cz, CarCtrlVehicleTest isInteresting, CarCtrlVehicleTest isTargetedByCrane);
// Number of vehicles of a model in the pool
int32_t CarCtrlLogic_CountCarsOfType(const CPool *vehiclePool, int32_t modelIndex);
// Small deterministic lateral offset applied to a vehicle on rails, derived from its path node info and random seed
void CarCtrl_GetRandomOffsetForVehicle(const CVehicleStruct *vehicle, bool next, float out[3]);

// Speed factor from a heading change, 1 below minAngle falling to coef at maxAngle
float CarCtrlLogic_FindSpeedMultiplier(float limitedAngleChange, float minAngle, float maxAngle, float coef);
// Speed factor of a road node type
float CarCtrlLogic_FindSpeedMultiplierWithSpeedFromNodes(int type);
// Pick a car or boat class from a random number 0..1000 and the zone thresholds
int CarCtrl_ChooseRatingFromThresholds(int rnd, const int16_t *thresholds, int numClasses, int firstRating, bool returnIndex);

// The loaded vehicle models of one car class with their running frequency sums, each row ends with -1 after the last model
// Add a model with its frequency, the sum keeps adding the frequencies of the models before it
void CarCtrlLogic_AddToLoadedVehicleArray(int32_t *loadedRow, int32_t *freqRow, int32_t *numLoaded, int32_t mi, int32_t freq);
// Remove a model and take its frequency out of the sums after it
void CarCtrlLogic_RemoveFromLoadedVehicleArray(int32_t *loadedRow, int32_t *freqRow, int32_t *numLoaded, int32_t mi);
// Pick a loaded model in proportion to its frequency, -1 when none is loaded, the random number is asked for only then
int32_t CarCtrlLogic_ChooseCarModel(const int32_t *loadedRow, const int32_t *freqRow, int32_t numLoaded, int32_t (*randomRange)(int32_t, int32_t));
// Whether a pointer is in a list of kept vehicles, and forget it there
bool CarCtrl_IsInPointerList(void *const *list, int count, const void *item);
void CarCtrl_RemoveFromPointerList(void **list, int count, const void *item);
// Move the traffic speed of a car on rails towards the speed it may drive, speeding up slowly and braking harder, returns true and the new speed when the speed has to be changed
bool CarCtrl_AdjustTrafficSpeed(float maxSpeed, float curSpeed, float timeStep, float *newSpeed);
// Turns an angle in radians that is below -PI up by full turns until it is not, and the other way for an angle above PI
void CarCtrl_WrapAngleUp(float *angle);
void CarCtrl_WrapAngleDown(float *angle);
// Steering of a plane that flies on its own after a frame, from the difference between the heading it wants and the one it has, its current steering and the time step
float CarCtrl_PlaneSteer(float difference, float planeSteer, float timeStep);
// Fraction of the speed of a plane or helicopter that comes from the new direction after a frame (0.05 per frame at the normal time step)
float CarCtrl_PlaneSpeedSplit(float timeStep);
// Factor the cruise speed of a helicopter that flies on its own is scaled by when it is close to its target, from the distance to it
float CarCtrl_HeliSpeedScale(float distanceToTarget);
// Vertical speed of a helicopter that flies on its own after a frame, from its vertical speed, the speed it aims at, the time step and whether it is destroyed: it follows the target at a limited rate and climbs 1.5 times faster than it sinks
float CarCtrl_HeliZSpeed(float zSpeed, float zSpeedTarget, float timeStep, bool destroyed);
// Turn speed a helicopter that flies on its own aims at, from the angle it still has to turn
float CarCtrl_HeliTurnSpeedTarget(float angleToTurn);
// Moves a value towards a target by at most the limit
float CarCtrl_ApproachValue(float value, float target, float limit);
// Acceleration of a boat that is steered by the AI, from the difference between the speed it wants and the one it has and the current speed: it reverses when too fast, full speed when far from the target speed and eases off near it
float CarCtrl_BoatAccel(float speedDiff, float currentSpeed);
// Pedals of a car that blocks the player by going forward and back: from how the speed of the player along the car, the time until the player crosses its path (diffAndRight), the distance along the car to the player and the speed of the car along itself compare, the handbrake is pulled for a hard brake
void CarCtrl_BlockForwardBackPedals(float dpPlayerAndForward, float dpDiffAndRight, float dpDiffAndForward, float dpSpeedAndForward, float *accel, float *brake, bool *handbrake);
// Right and forward vectors of a car that drives on rails along a slope, from the x and y of the middle of the path and of its target and the heights of the road ahead and behind
void CarCtrl_RoadSlopeOrientation(float midX, float midY, float targetX, float targetY, float aheadZ, float behindZ, float right[3], float forward[3]);
// Whether another car is too far away or moves away too fast for a car to weave around it: from the dot product of their relative speed and the distance vector, the radii of both bounding spheres and the distance
bool CarCtrl_WeaveOutOfReach(float relativeSpeedDotDiff, float otherRadius, float ownRadius, float distance);
// Angle a car has to evade another one by, from the angle between the cars and their direction, the half sizes of the bounding boxes, the distance and the safety coefficient of the width
float CarCtrl_WeaveLengthToEvade(float angleDiff, float otherMaxY, float otherMaxX, float ownMaxX, float distance, float widthCoef);
// Widens the free corridor of a car that weaves around an obstacle: the angles it weaves to on the left and on the right move away from the angle of the obstacle by half the length to evade when it is in the way
void CarCtrl_ApplyWeaveAngles(float angleToObstacle, float lengthToEvade, float *angleToWeaveLeft, float *angleToWeaveRight);
// Angle a car steers to when it weaves through traffic, from the angle it is heading, the angle to its target and the free angles on the left and on the right of the obstacles: the one that is closer to the middle of the way, or the bisector when both are far
float CarCtrl_PickWeaveAngle(float angleForward, float angleToTarget, float angleToWeaveLeft, float angleToWeaveRight);
// How far along its movement (0 to 1) a point reaches the edges of a slab, for the test of the collision of two cars: base is where it touches the near face and full where it has crossed the whole slab, from the distance of the point to the centre of the slab along an axis, the movement along that axis, the half sizes of the slab on the positive and on the negative side and its full size
void CarCtrl_SlabProximity(float distance, float projection, float positiveHalf, float negativeHalf, float fullSize, float *base, float *full);
// Offset of a corner of a car from the centre of another one, for the test of their collision: corner 0 is the front right one, 1 the front left, 2 the rear right and 3 the rear left, from the offset between the cars, the half width, the front and back length of the car and the forward vector of the other car
void CarCtrl_CornerOffset(int corner, float offsetX, float offsetY, float width, float length, float backLength, float forwardX, float forwardY, float *x, float *y);
// Slows a car down for another car that is close ahead: from the proximity (the fraction of the way until they hit), the speed of the car and the speed limit that is lowered, slowed is set when it did; returns the proximity rescaled from the 0.2 to 1.5 range
float CarCtrl_SlowForProximity(float minProximity, float curSpeed, float *speed, bool *slowed);
// How a ped that is about to be hit by a car evades it
enum {
	CARCTRL_EVADE_NONE,
	CARCTRL_EVADE_STEP,
	CARCTRL_EVADE_DIVE
};
// Evasion of a ped that a car (that is not the RC Bandit) is about to hit, from the speed of the car towards the ped, the half width of the car and the sideways distance of the ped
int CarCtrl_PedEvasion(float dotVelocity, float sideLength, float sidewaysDistance);
// Angle between where a ped looks and the direction of a car, between 0 and half a turn, from the angle of the car to the ped and the heading of the ped
float CarCtrl_PedVisibilityAngle(float angleCarToPed, float pedHeading);
// Position of a car in its lane on a path link, from the position of the link, the lane (counted with the offset of one way links), the direction of the link and the width of a lane, pos[0] is x and pos[1] is y
void CarCtrl_LanePosition(float linkX, float linkY, float lane, float forwardX, float forwardY, float laneWidth, float pos[2]);
// Direction of a turn on a path of three nodes: 0 when two of the nodes are at the same place, 1 for straight, 2 for a turn to the right and 4 for a turn to the left, from the positions of the previous, current and next nodes
int CarCtrl_PathDirection(float prevX, float prevY, float curX, float curY, float nextX, float nextY);
// How much a car has to turn to follow a link from a node towards a point, between 0 and half a turn, from the angle to the point and the positions of the node and of the node the link leads to
float CarCtrl_LinkAngleChange(float angleToTarget, float nodeX, float nodeY, float testNodeX, float testNodeY);
// Directions a car may take at a junction (bits: 1 straight, 2 right, 4 left), from the lane it is in and the number of lanes it has: left from the leftmost lane, right from the rightmost one and straight on roads of one or two lanes or from a middle lane
int CarCtrl_AllowedDirections(int lane, int lanes);
// Number of lanes a car has on a link in its direction of travel and whether the link is a one way road, from whether the link starts at the node of the car and the numbers of its left and right lanes
void CarCtrl_LinkLanes(bool linkStartsAtNode, int leftLanes, int rightLanes, int *lanes, bool *isOneWay);
// Whether a link leads against the traffic and whether it leads on a one way road, from whether the link starts at the node of the car and the numbers of its left and right lanes
void CarCtrl_LinkOneWayFlags(bool linkStartsAtNode, int leftLanes, int rightLanes, bool *goingAgainstOneWay, bool *isOneWayRoad);
// Whether a node is acceptable as the next one of a car that picks its way randomly, from the flags of the node and of the previous one (a car may leave a dead end, a disabled area or the level below only when it is in one already), whether the car stays in its level, whether the link leads against the traffic and whether both the road of the car and the next one are one way
bool CarCtrl_NextNodeAcceptable(bool nextDeadEnd, bool prevDeadEnd, bool nextDisabled, bool prevDisabled, bool nextBetweenLevels, bool prevBetweenLevels, bool stayInLevel, bool goingAgainstOneWay, bool onOneWayRoad, bool nextIsOneWayRoad);
// Same as above for the second try, that ignores the dead ends and the one way roads the car is on
bool CarCtrl_NextNodeAcceptableRelaxed(bool nextDisabled, bool prevDisabled, bool nextBetweenLevels, bool prevBetweenLevels, bool stayInLevel, bool goingAgainstOneWay);
// Lane a car takes on the next road, from the lane it is in now, the lane it is in on its way to the next one (nextLane), the number of lanes of the next road (negative when it has none) and the change of lane it wants (-1, 0 or 1): it stays within the lanes of the road, and keeps its lane when the road has none
int CarCtrl_NextLane(int nextLane, int currentLane, int lanesOnNextNode, int laneChange);
// Time in milliseconds a car takes to drive a curve between two links, from the speed scale factor of the curve and the maximum speed of the traffic
int32_t CarCtrl_CurveTime(float speedScaleFactor, float maxTrafficSpeed);
// Distance from the player beyond which a vehicle is removed: the range for vehicles that are seen or that matter when useOnscreenRange is set (scaled by the generation distance of the camera) and the one for vehicles that are not otherwise, 70 when the camera looks down (only when checkLookingDown is set), longer for vehicles of extended range
float CarCtrl_DespawnThreshold(bool useOnscreenRange, float offscreenRange, float onscreenRange, float generationDistMultiplier, bool checkLookingDown, float cameraForwardZ, bool extendedRange, float extendedRangeMultiplier);
// Factor a helicopter slows down by in a frame: a constant resistance raised to the power of the time step
float CarCtrl_AirResistance(float resistance, float timeStep);
// Direction on the ground a flying AI vehicle points to from its orientation (cos, sin)
void CarCtrl_OrientationDirection(float orientation, float dir[2]);
// Steering angle limited to the maximum steering angle in both directions
float CarCtrl_LimitSteer(float steerAngle, float maxAngle);
// Multiplier of the speed of an AI car from the distance to the next node, its cruise speed, the multiplier of the angle between the nodes and the distance below which it slows down: it slows down more as the node gets closer
float CarCtrl_SpeedNodesMultiplier(float distanceToNextNode, int cruiseSpeed, float wideMultiplier, float slowingDistance);
// Accelerator and brake of an AI car that follows a path, from the speed it wants, the speed difference to the current one and the current forward speed
void CarCtrl_PathAccelBrake(float speed, float speedDifference, float currentForwardSpeed, float *accel, float *brake);
// Accelerator and brake of an AI car that heads for a target, from the speed difference to the wanted speed and the current speed
void CarCtrl_TargetAccelBrake(float speedDiff, float currentSpeed, float *accel, float *brake);
// Registers a vehicle in the list of vehicles to keep: refreshes its time when it is already there, takes the first free slot, or replaces the vehicle that was kept the longest ago
void CarCtrl_RegisterKept(void **cars, uint32_t *times, int count, void *vehicle, uint32_t now);
// Time an AI car spends on a curve, at least 10
int32_t CarCtrl_MinCurveTime(int32_t time);
// Lane an AI car takes on the next node, kept between 0 and the last lane of the node
int8_t CarCtrl_ClampNextLane(int8_t lane, int lanesOnNextNode);
// Proximity of a car to another after a test of its width and length: the closest of the current one and the proximity where the base of both slabs collides, only when it is within the full width and length proximities
float CarCtrl_CombineProximity(float proximity, float baseWidthProximity, float fullWidthProximity, float baseLengthProximity, float fullLengthProximity);
// Position of a new car between two nodes, from the distance between the nodes, the length of the car and the wanted position: in the middle when the car does not fit, otherwise limited so that the whole car fits
float CarCtrl_PositionBetweenNodes(float distanceBetweenNodes, float carLength, float positionBetweenNodes);
// Speed of an AI car that slows down for a ped ahead: at most the speed scaled by how far the ped is beyond 1 unit (up to 10)
float CarCtrl_PedSlowdownSpeed(float speed, float distanceUntilHit, float curSpeed);
// Speed of an AI car that was standing still and is let go: at least a fifth of its current speed
float CarCtrl_ReleasedSpeed(float speed, float curSpeed);
// Whether an entity is too far above or below an AI car to be considered for weaving around (the height difference is at least the limit)
bool CarCtrl_TooFarVertically(float heightDifference, float limit);
// Multiplier of the speed of an AI car: the smallest of the style, angle and nodes multipliers, taken as Min(style, Min(angle, nodes))
float CarCtrl_SpeedMultiplier(float styleMultiplier, float angleMultiplier, float nodesMultiplier);
// Distance around an AI car that is scanned for things to weave around, growing with its horizontal speed (twice as far at most)
float CarCtrl_WeaveScanDistance(float speed2D);
// How far ahead a boat that attacks the player aims at where the player will be, from the distance to the player (at most 2)
float CarCtrl_BoatProjection(float distanceToPlayer);
// Angle of the horizontal part of a forward direction, which is normalised first like CVector2D::Normalise does (a zero vector becomes (1, 0))
float CarCtrl_ForwardAngle(float forwardX, float forwardY);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
