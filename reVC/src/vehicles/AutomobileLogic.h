//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "VectorMath.h"
#include "AutomobileLayout.h"
#include "VehicleDoors.h"

#ifdef __cplusplus
extern "C" {
#endif
// Small automobile state helpers, doors are the eDoors values
bool Automobile_IsComponentPresent(const CAutomobileData *self, int comp);
bool Automobile_IsDoorMissing(CAutomobileData *self, int door);
bool Automobile_IsDoorFullyOpen(CAutomobileData *self, int door);
bool Automobile_IsDoorClosed(CAutomobileData *self, int door);
bool Automobile_IsDoorReady(CAutomobileData *self, int door, int gettingInFlags);
bool Automobile_IsDoorReadyByNode(CAutomobileData *self, unsigned node, int gettingInFlags);
bool Automobile_IsDoorMissingByNode(CAutomobileData *self, unsigned node);
bool Automobile_IsOpenTopCar(const CAutomobileStruct *self);
void Automobile_ResetSuspension(CAutomobileData *self);
bool Automobile_GetAllWheelsOffGround(const CAutomobileData *self);
void Automobile_ReduceHornCounter(CVehicleData *vehicle);
// Fly-to-coordinates missions of helicopters and planes and other small state changes
void Automobile_TellHeliToGoToCoors(CAutomobileStruct *self, float x, float y, float z, uint8_t speed);
void Automobile_TellPlaneToGoToCoors(CAutomobileStruct *self, float x, float y, float z, uint8_t speed);
void Automobile_PopBootUsingPhysics(CAutomobileData *self);
void Automobile_ClearCarNodes(CAutomobileData *self);
// Start the doors timer of a bus, type 0 opens and closes and anything else only closes
void Automobile_SetBusDoorTimer(CAutomobileData *self, uint32_t timer, uint8_t type, uint32_t now);
// Compress the suspension of a wheel that is in water, upper and lower are the ends of the suspension line in the world and waterZ the water level at the lower one, returns true when the wheel was changed
bool Automobile_HoverWheelInWater(CAutomobileData *self, int i, const float *upper, const float *lower, float waterZ);
// Opening a door of a bus as asked by the door timer, the component is the car node and the door the eDoors value
typedef struct AutomobileDoorAction {
	int component;
	int door;
	float ratio;
} AutomobileDoorAction;
// Run the doors of a bus by its timer: they close in the last half second before the end and open again afterwards, the front doors that are present and not in use are the ones that move
// doorMissing and doorClosed hold the state of the front left and front right door, the doors to move are written to actions in order and their number is returned
int Automobile_ProcessAutoBusDoors(CAutomobileStruct *self, uint32_t now, const bool doorMissing[2], const bool doorClosed[2], AutomobileDoorAction actions[2]);
// Gas and brake pedal of a car from its speed along its forward direction, the accelerate and brake buttons of the pad (0 to 255) and whether it is a Dodo, burnout is set when both buttons are held while the car stands still
void AutomobileLogic_Pedals(float speed, int16_t accelerate, int16_t brake, bool isDodo, float *gasPedal, float *brakePedal, bool *burnout);
// Steering input after a step towards the one of the pad (from -128 to 127)
float AutomobileLogic_SteerPad(float steerInput, int16_t steeringLeftRight, float timeStep);
// Steering input after the mouse moved, it returns to the center slowly when it is close to it
float AutomobileLogic_SteerMouse(float steerInput, float mouseX, float sensitivity, float centreRange, float centreMult, float timeStep);
// Angle of the front wheels in radians from the steering input (from -1 to 1) and the steering lock in degrees
float AutomobileLogic_WheelAngle(float steerInput, float steeringLockDegrees);
// Next state of the comedy controls of a car, rnd is a random number from 0 to 9, the steer angle gets a nudge in states 1 and 2
int AutomobileLogic_ComedyControls(int state, int rnd, float *steerAngle);

// Rough position of a wheel (0 to 3: front left, rear left, front right, rear right) from the middle of a car, from the extents of its collision box
VectorMath3 AutomobileLogic_ApproxWheelOffset(int wheel, float boxMaxX, float boxMaxY, float boxMinY);
// Rough position of a wheel of a bike (0 front, 1 rear) from the middle of the bike, from the extents of its collision box
VectorMath3 AutomobileLogic_ApproxBikeWheelOffset(int wheel, float boxMaxY, float boxMinY);

// Spring ratio of a suspension after its line got moved, a wheel that is not fully extended moves along it: from the ratio, the height of the top of the line, the height the wheel was at and the length of the line
float Automobile_HydraulicSpringRatio(float springRatio, float lineTopZ, float wheelZ, float lineLength);
// How much each wheel of a car with hydraulics is lifted from the input of the stick (up and down, left and right), in the order of the wheels
void Automobile_HydraulicWheelLift(float upDownInput, float leftRightInput, float lift[4]);
// Loudness of the hydraulics from the biggest movement of a suspension line and the range it can move, returns false when it moved too little to be heard
bool Automobile_HydraulicSoundLevel(float maxDelta, float limitDiff, float *level);
// Spring ratio a wheel had before the last move from the spring ratio, the length of the spring and the length of the line
float Automobile_PrevSpringRatio(float springRatio, float springLength, float lineLength);
// Impulse a car feels from a standing ped that walks into it, the ped pushes with its mass when it walks against the car
float Automobile_PedImpulseReduction(float impulse, float pedMoveDeltaY, float dotForwardNormal, float pedMass);
// How hard the pad shakes when a car hits something with an impulse above 50, from the impulse and the mass of the car, between 0 and 250
unsigned char Automobile_ShakeFrequency(float impulse, float mass);
// Damage a car takes from an impulse above the one it ignores
float Automobile_CollisionDamage(float impulse, float minImpulse, float collisionDamageMultiplier, float damageMultiplier, bool securicaHitByPlayer, bool isRC);
// Health a car has after taking damage, depending on who drives it and who hit it, a car that would be destroyed by this keeps one point when it was healthy
float Automobile_HealthAfterDamage(float health, float damage, bool isPlayerVehicle, bool takeLessDamage, bool hitByPlayerVehicle);
// How much the water pushes a car up compared to its weight, from the vertical buoyancy impulse, the mass and the time step
float Automobile_WaterImpulseRatio(float impulseZ, float mass, float timeStep);
// Factor the speeds of a car in the water are multiplied with in a frame, from the ratio of the impulse and the time step
float Automobile_WaterResistance(float impulseRatio, float timeStep);
// Whether a car in the water is drowning: not for a Sea Sparrow that floats or a car that hovers, yes after a heli hit the water hard, with a big impulse or a medium one with a wheel fully extended
bool Automobile_IsDrowning(bool isSeaSparrow, float impulseRatio, float upZ, bool hoversOnWater, bool heliHitWaterHard, bool anySuspensionFullyExtended);
// Colour channel of the splash of a car that falls into the water from the directional and the ambient light
unsigned char Automobile_WaterSplashChannel(float directional, float ambient);
// Size of the splash a wheel makes in the water from its speed on the ground
float Automobile_WheelSplashSize(float speed);
// Speed a component that flies off a car starts with from the speed of the car, the height of its up vector and whether the component is a bonnet, a boot or a windscreen
void Automobile_FlyingComponentSpeed(const float moveSpeed[3], float upZ, bool isBonnetBootOrWindscreen, float out[3]);
// Number of sparks a burst wheel throws from the squared speed of the car and the surface (eSurfaceType) it grinds on, none on soft ground or water
int Automobile_BurstWheelSparkCount(float speedSqr, int surface);
// Kinds of emergency vehicles with a siren
enum {
	AUTOMOBILE_SIREN_FIRETRUCK,
	AUTOMOBILE_SIREN_AMBULANCE,
	AUTOMOBILE_SIREN_POLICE,
	AUTOMOBILE_SIREN_ENFORCER
};
// Places of the two sirens on a vehicle and the colours they flash in
typedef struct AutomobileSirenSetup {
	float pos1[3], pos2[3];
	unsigned char color1[3], color2[3];
} AutomobileSirenSetup;
AutomobileSirenSetup Automobile_SirenSetup(int kind);
// Colour of the light a siren throws on the road at the time in milliseconds, it flashes in the first colour then the second and fades in and out
void Automobile_SirenLightColor(uint32_t timeMs, const unsigned char color1[3], const unsigned char color2[3], unsigned char out[3]);
// Angle a wheel of a car is tilted by when it presses on the ground, from the height of the wheel and of the car above the road, the scale of the wheel, whether it is the Voodoo and the side of the car
float Automobile_WheelTilt(float wheelZ, float heightAboveRoad, float wheelScale, bool isVoodoo, bool rightSide);
// Amount of dust a helicopter blows up from the speed of its rotor and the time step
float Automobile_HeliDustAmount(float rotorSpeed, float timeStep);
// Alpha of the rotors of a helicopter from the speed they turn with: the sharp ones fade out and the blurred ones fade in as they speed up
void Automobile_HeliRotorAlpha(float rotorSpeed, int *rotorAlpha, int *blurAlpha);
// Direction of the water cannon of a fire truck relative to the car from the angle sideways and the angle up
void Automobile_CannonDirection(float angleLR, float angleUD, float dir[3]);
// Turret angle of a tank kept between 0 and a full turn
float Automobile_TankTurretAngle(float angle);
// Where the end of the gun of a tank is relative to the car after the turret turned by the angle, from the vector from the base of the turret to the end of the gun and the base
void Automobile_TankGunEnd(float angle, const float baseToEnd[3], const float turretBase[3], float out[3]);
// Improved grip for the cars that ram or block the player at a high wanted level, and for the RC bandit that is not remote controlled: from whether the player drives another vehicle, the wanted level, the mission of the car (eCarMission), the speed of the player, the speed of the car and the distance to the player
void Automobile_StrongGrip(bool playerDrivesOtherVehicle, int wantedLevel, int carMission, float playerSpeed, float ownSpeed, float distToPlayer, bool isRCBandit, bool notRemoteControlled, bool *strongGrip1, bool *strongGrip2);
// Height of the centre of mass of a car that drives on two wheels, returns false when it should be left as it is: from the time on two wheels, the stick, the height of the box of the car, the vertical part of its up and right vectors and whether it is a heli or a plane
bool Automobile_TwoWheelCentreOfMassZ(float handlingCentreZ, int twoWheelTime, float steeringLeftRight, float balanceMult, float colMaxZ, float upZ, float rightZ, bool isHeliOrPlane, float *centreZ);
// How much sand slows a car down and by how much its sideways speed is scaled, from the squared speed along the ground and the weather, stuck is set when the car is too slow to get out
float Automobile_SandSlowdown(float parallelSpeedSqr, bool goodInSand, float wetRoads, float sandSlowdown, float *parallelScale, bool *stuck);
// Whether an abandoned or wrecked car moves so little it may be frozen, from the squared averages of its speeds, the time step, the distance it travelled, whether a ped damaged it and whether it was already found to stand still
bool Automobile_IsBarelyMoving(bool isWrecked, float moveSpeedAvgSqr, float turnSpeedAvgSqr, float timeStep, float distanceTravelled, bool damagedByPed, bool makeStatic);
// Spring ratio of a wheel in the sand: the sand lets the wheel sink into it, from the ratio, the speed of the car along its direction (slowed down for the sand), the wetness of the roads and the length of the line and the spring
float Automobile_SandSpringRatio(float springRatio, float fwdSpeed, float wetRoads, float lineLength, float springLength);
// Speed a wheel that is off the ground turns with after a frame: a driven wheel spins up or down with the acceleration, a wheel with the handbrake on stops, any other slows down
float Automobile_FreeWheelSpeed(float wheelSpeed, float acceleration, bool isDriven, bool handbrakeOn);
// Brake and traction bias of the front and rear wheels, in this order, from the biases of the handling, 1 for each wheel of a car that handles neutrally
void Automobile_WheelBiases(bool neutralHandling, float brakeBias, float tractionBias, float biases[4]);
// Temperature of the tyres after a frame: they heat up in a burnout and cool down towards 1 otherwise
float Automobile_TireTemperature(float temperature, bool burnoutWithSpinningWheel, float timeStep);
// How much the pad shakes from a surface (eSurfaceType) a wheel rolls on, the bigger of the current value and the one of the surface
float Automobile_SurfaceShake(int surface, float currentShake);
// How hard the pad shakes (0 to 255, the cap is the limit) when the car shakes by the given amount, from the speed and the mass of the car, the scale of the shake and the base value
unsigned char Automobile_PadShakeFrequency(float shake, float speed, float mass, float base, float cap);
// Height of a wheel of a car after a frame: it follows the compression of its spring, a bit at a time, or straight when the Voodoo uses its own collision model
float Automobile_WheelPosition(float lineTopZ, float springRatio, float springLength, bool followsDirectly, float currentPosition);
// Wobble of a burst wheel of a car as it turns: its tilt wobbles with the rotation of the wheel
float Automobile_WheelWobble(float wheelRotation);
// Vertical angle of the water cannon of a firetruck that fights a fire: it goes up and down a bit with the time in milliseconds
float Automobile_AIFireTruckGunUD(uint32_t timeMs);
// Rotation of the wing propeller of the Phoenix and the Bloodring Banger after a frame: from whether it is the Bloodring Banger (the propeller of the Phoenix tilts up to 1.3 and then wobbles), whether the gas pedal is pressed, the stored rotation (updated), the time step and the time in milliseconds; returns the rotation to render
float Automobile_WingPropeller(bool isBFInjection, bool gasPressed, float *propellerRotation, float timeStep, uint32_t timeMs);
// Forward direction of a steered front wheel in the space of the vehicle, from its steering angle: (-sin, cos), forward[0] is x and forward[1] is y
void Automobile_SteerForward(float steerAngle, float forward[2]);
// Adhesion of a wheel of a car, from the adhesive limit of the surface scaled by the traction, whether the player drives it and the wet multiplier of the surface (only used for the player)
float Automobile_WheelAdhesion(float adhesiveLimit, float traction, bool isPlayer, float wetMultiplier);
// Forward and right directions of a steered front wheel along the surface it stands on, from the forward vector of the car, the normal of the surface and the sine and cosine of the steering angle
void Automobile_SteeredWheelAxes(const float forward[3], const float normal[3], float s, float c, float wheelFwd[3], float wheelRight[3]);
// Forward and right directions of a wheel along the surface it stands on: the forward vector is made to lie on the surface and the right vector is perpendicular to it and to the normal, both normalised
void Automobile_SurfaceWheelAxes(const float forward[3], const float normal[3], float wheelFwd[3], float wheelRight[3]);
// Whether the blades of a helicopter should be checked for the rotor sound: the camera is within 20 units (squared distance) and the propeller is more than 30 degrees away from the blade rotation
bool Automobile_RotorSoundCheck(float distSq, float propellerRotation, float bladeRotation);
// Whether a blade is in line with the camera for the rotor sound, from the dot product of the direction to the camera and the blade direction and the limit
bool Automobile_RotorBladeInLine(float dot, float limit);
// New horizontal angle of the cannon of an AI fire truck that turns towards its target: it snaps to the target when the remaining delta is below the step and otherwise moves by the step in the direction of the delta
float Automobile_CannonTrackStep(float gunAngle, float targetAngle, float cannonDelta, float angleStep);
// Position of the light number i (0 to 3) of a siren bar, interpolated between the two end positions
void Automobile_SirenPosition(const float pos1[3], const float pos2[3], int i, float out[3]);
// How much behind a car the camera is, from the dot product of the direction to the car and its forward vector: clamped to -1..1 (1 if directly behind, -1 if in front), and the matching angle from 0 (behind) to PI (in front)
float Automobile_Behindness(float dot, float *angle);
// Orientation and position of a car lined up with the road under its front and rear points: the right, forward and up vectors and the position, from the x and y of the front and rear points, their heights, the length between them and the height of the car above the road
void Automobile_RoadAlignedMatrix(const float front[2], const float rear[2], float frontZ, float rearZ, float len, float heightAboveRoad, float right[3], float forward[3], float up[3], float pos[3]);
// Whether the absolute value of a speed is above a limit
bool Automobile_SpeedAbove(float speed, float limit);
// Whether the absolute value of a speed is below a limit
bool Automobile_SpeedBelow(float speed, float limit);
// Whether a point is further above or below a collision line than the largest allowed delta (both in absolute value)
bool Automobile_LineDeltaExceeds(float posZ, float lineZ, float maxDelta);
// Upward force a car applies to another that touches its wheel: 50 times the mass times a factor, at most 50
float Automobile_WheelNudgeForce(float mass, float factor);
// How far ahead a rocket of a Hunter starts: the forward speed of the vehicle (never negative) along its forward vector over the time step
void Automobile_RocketLead(float forwardSpeed, const float forward[3], float timeStep, float lead[3]);
// Distance used to normalise the direction to the camera: the square root of the squared distance but at least 0.01
float Automobile_SafeDistance(float distSq);
// Upward push of the exhaust smoke of a slow car, from its speed: stronger when it is slower, never below 0
float Automobile_SmokeLift(float speed);
// Divisor of the impulse of a helicopter that hits the water, from the impulse ratio: 8 times the ratio but at least 1
float Automobile_WaterImpactDivisor(float impulseRatio);
// Direction on the ground the gun of a tank points in, from the angle of its turret: (sin(-angle), cos(-angle))
void Automobile_TurretDirection(float turretAngle, float dir[2]);
// Whether the camera is further than a limit away from a vehicle, measured as the sum of the distances on x and y
bool Automobile_CameraIsFarAway(float dx, float dy, float limit);
// Whether wheels on both sides of a car are compressed: one of the left wheels (0, 2) and one of the right wheels (1, 3) have a spring ratio below 1
bool Automobile_WheelsOnBothSides(const float springRatio[4]);
// Force that keeps a car from rolling over: gravity times the turn mass limited to 2500, as a magnitude
float Automobile_AntiRollStrength(float gravity, float turnMass);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
