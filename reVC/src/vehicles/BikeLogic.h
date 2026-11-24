//+ rouz edit (Claude)
#pragma once
#include "BikeLayout.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
// Small bike state helpers
bool Bike_IsComponentPresent(const CBikeData *self, int component);
void Bike_ResetSuspension(CBikeData *self);
void Bike_ClearNodes(CBikeData *self);
void Bike_RemoveRefsToVehicle(CBikeData *self, const CEntity *ent);
// Tilt the lean matrix, which starts as the bike matrix, by the lean angle and put the wheel back on the ground, up is the up vector of the bike and minZ the lowest point of its collision box
void Bike_CalculateLeanMatrix(CMatrixData *leanMatrix, float leanAngle, const float *up, float minZ);
// World position of a door point given in the model space of a bike: p1 and p2 are two points whose sum and difference place it, forward, up and position are those of the bike and maxX and maxZ the upper bounds of its collision box
void Bike_GetCorrectedWorldDoorPosition(const float *forward, const float *up, const float *position, float maxX, float maxZ, const float *p1, const float *p2, float *out);
// Gas and brake pedal of a bike from its speed along its forward direction, the accelerate and brake buttons of the pad (0 to 255) and whether it is a Dodo, a bike that is told to go the other way than it goes releases the gas, burnout is set when both buttons are held while it stands still
void BikeLogic_Pedals(float speed, int16_t accelerate, int16_t brake, bool isDodo, float *gasPedal, float *brakePedal, bool *burnout);
// Lean input after a step towards the wanted lean, leanMul widens the range in the air
float BikeLogic_LeanInput(float leanInput, float updown, float leanMul, float timeStep);
// Pedals and steering of a bike with comedy controls, which rev and wobble with the time in milliseconds
void BikeLogic_ComedyControls(uint32_t timeMs, float *gasPedal, float *brakePedal, float *steerAngle);

// What a ped that got knocked off a bike and gets back on decides to do
enum {
	BIKE_KNOCKOFF_NOTHING,
	BIKE_KNOCKOFF_ENTER_AS_DRIVER,	// goes after the bike
	BIKE_KNOCKOFF_WANDER		// gives up and wanders
};
// Decides what a ped does after it got knocked off a bike, from whether it is braver than it is afraid, whether it and the bike belong to a mission, whether the player goes for the bike, and whether the player is on a mission
int Bike_KnockOffRiderObjective(bool temperAboveFear, bool pedNotMission, bool bikeNotMission, bool playerGoesForBike, bool playerNotOnMission);
// Heading of a ped that got knocked off a bike from the heading of the bike, turned around when the bike is upside down
float Bike_KnockOffHeading(float upZ, float bikeHeading);
// Strength with which a ped that got knocked off by an impact is pushed on the ground, from the impulse of the damage and the masses of the ped and the bike
float Bike_KnockOffForceXY(float damageImpulse, float pedMass, float bikeMass);
// Brake and traction bias of the front and rear wheels of a bike, in this order, from the biases of the handling, 1 for each wheel of a bike that handles neutrally
void Bike_WheelBiases(bool neutralHandling, float brakeBias, float tractionBias, float biases[4]);
// Strength of the turn force that leans a bike back or forward, from the lean input, the turn mass, the lean force of the handling and the speed of the bike
float Bike_LeanForce(float leanInput, float turnMass, float leanForce, float moveSpeed);
// Factor the lean back force is scaled by from the gas pedal, the Sanchez depends less on it than the other bikes
float Bike_LeanBackGasScale(bool isSanchez, float gasPedal);
// Change of the turn speed around the pitch (turn[0]) and roll (turn[1]) axes of a bike in one frame, from the resistances, the axis factors, whether it is the player and which wheels are off the ground, the vertical part of its forward vector, the angles of the wheelie and stoppie, the turn speed in the space of the bike and the time step
void Bike_TurnResistance(bool isPlayer, bool frontOffGround, bool rearOffGround, float forwardZ, float resX, float resY, float dAxisX, float dAxisXExtra, float dAxisY, float inAirXRes, float wheelieAng, float stoppieAng, float localTurnX, float localTurnY, float timeStep, float turn[2]);
// Angle of the front wheel of a bike after a frame, from whether the wheel follows the steering (the player, or a bike that is not standing), the speed along the ground, the steering angle, the current angle, the lean angle, the speed along the bike, whether it is the player, whether the steering is limited by the grip (limitedByGrip), the grip limit of the steering (already scaled by the slip steer when the ground is loose), the steering lock of the handling and the time step
float Bike_WheelAngle(bool followsSteering, float moveSpeedX, float moveSpeedY, float steerAngle, float wheelAngle, float leanAngle, float fwdSpeed, bool isPlayer, bool limitedByGrip, float gripSteer, float steeringLock, float timeStep);
// Adhesion of a wheel of a bike, from the adhesive limit of the surface scaled by the traction, whether the brakes destabilise the bike, the adhesion group of the surface (eAdhesionGroup), whether it is the player, the wet multiplier of the surface (only used for the player) and whether the tyre is burst, destab is set to the factor the destabilised bike loses grip by
float Bike_WheelAdhesion(float adhesiveLimit, float traction, float brakeDestabilization, int adhesionGroup, bool isPlayer, float wetMultiplier, bool burst, float *destab);
// How much the pad shakes from a surface (eSurfaceType) a wheel of a bike rolls on, the bigger of the current value and the one of the surface
float Bike_SurfaceShake(int surface, float currentShake);
// Strength of the force that brings a bike back to its wheelie angle, from the wheelie angle of the handling, the vertical part of the forward vector, the stabilisation multiplier and the speed
float Bike_WheelieStabilisation(float wheelieAng, float forwardZ, float stabMult, float speed);
// Strength of the force that brings a bike back to its stoppie angle, from the stoppie angle of the handling, the vertical part of the forward vector, the stabilisation multiplier and the speed
float Bike_StoppieStabilisation(float stoppieAng, float forwardZ, float stabMult, float speed);
// Side lean angle of a bike after a frame: a bike held by its rider (or picked up) leans into the sideways acceleration (sideAccel, the change of its speed along the right vector of the surface, only used when a wheel touches the ground), a standing bike leans on its stand and any other falls to the side
float Bike_LeanAngle(bool heldByRider, bool isStanding, int wheelsOnGround, float steerAngle, float steeringLock, float sideAccel, bool frontBurst, float maxLean, float desLean, float idleAngle, float rightZ, float leanAngle, float gravity, float timeStep);
// Destabilisation of the steering of a bike that brakes hard after a frame, from the current value, a random number between 0.5 and 1 and the time step
float Bike_BrakeDestabilization(float destabilization, float random, float timeStep);
// Strength of the sideways force at the rear wheel that destabilises a bike that brakes hard, from the turn mass, the destabilization, the speed along the bike and whether the rear wheel touches the ground, without the right vector and the time step
float Bike_BrakeDestabForce(float turnMass, float destabilization, float fwdSpeed, bool rearOnGround);
// What limits the grip of the rear wheel of a bike
enum {
	BIKE_REARGRIP_NORMAL,
	BIKE_REARGRIP_HANDBRAKE,	// the wheel is locked and the tyres reset to cold
	BIKE_REARGRIP_BURNOUT,		// the wheel spins freely and pushes the bike sideways
	BIKE_REARGRIP_COLD_TYRES	// the tyres are still cold, the traction is scaled by their temperature and the bike is pushed sideways
};
// Brake and traction of the rear wheel of a bike, from the brake and the traction (already scaled by the handling), whether the handbrake is on, the brake it applies, whether the bike does a burnout, the temperature of the tyres and the gas pedal, returns what limits the grip (see above)
int Bike_RearWheelGrip(bool handbrakeOn, float handbrakeBrake, bool burnout, float tireTemperature, float gasPedal, float brake, float traction, float *rearBrake, float *rearTraction);
// Speed of the rear wheel of a bike that is off the ground after a frame, from the speed, the acceleration and whether the handbrake is on
float Bike_RearWheelAirSpeed(float wheelSpeed, float acceleration, bool handbrakeOn);
// Whether a bike in the water is drowning: with a big impulse, or a medium one with a wheel fully extended
bool Bike_IsDrowning(float impulseRatio, bool anySuspensionFullyExtended);
// Number of sparks the burst wheel of a bike throws, from the squared speed of the bike and the surface (eSurfaceType) under the wheel, 0 below a speed of 0.1 and on the soft surfaces
int Bike_BurstWheelSparkCount(float speedSqr, int surface);
// Intensity and size of the taillight corona of a bike seen from behind, from how directly behind it the camera is (above 0) and whether it brakes (not when it reverses)
void Bike_TailLightGlow(float behindness, bool braking, float *intensity, float *size);
// Height above the ground of the contact point of a wheel of a bike, from the height of the top of the suspension line, the length of the spring, its ratio and the scale of the wheel
float Bike_WheelContactZ(float lineTopZ, float springLength, float springRatio, float wheelScale);
// Position (y and z) of the front wheel of a bike in the model, from the y of both suspension lines, the position of the wheel and its base position, and the position and slope of the fork
void Bike_FrontWheelPos(float lineY1, float lineY2, float wheelPos, float wheelBasePos, float forkZ, float forkY, float forkSlope, float *y, float *z);
// Angle the rear fork of a bike is rotated by, from the position of the rear wheel, its base position and the length of the fork
float Bike_RearForkAngle(float wheelPos, float wheelBasePos, float forkLength);
// Height the chassis of a bike is lowered by when it leans, from the lean angle and the lowest point of its bounding box
float Bike_ChassisOffsetZ(float leanAngle, float boundingMinZ);
// Height of the centre of a bike above the road when its springs are compressed a bit, from the length of the spring at rest, the force level of the suspension, the height of the top of the first suspension line and the scale of the wheel
float Bike_HeightAboveRoad(float springLength, float suspensionForceLevel, float lineTopZ, float wheelScale);
// Length of the rear fork of a bike from the distance of the rear wheel to the pivot of the fork
float Bike_RearForkLength(float dy, float dz);
// Whether a tyre that bursts at that squared speed throws the rider off or twists the bike, the player needs a higher speed
bool Bike_BurstThrowsRider(float speedSqr, bool isPlayer, float fallSpeed, float fallSpeedPlayer);
// Speed of a bike at a collision as it is felt by the rider, from the impulse, the mass of the bike, whether the player rides it and whether it is a mission vehicle
float Bike_CollisionSpeed(float impulse, float mass, bool isPlayer, bool isMissionVehicle);
// Damage the rider of a bike takes in a collision, from the dot products of the normal of the damage with the forward, right and up vectors of the bike, the vertical part of the normal, whether it is the Sanchez and the speed of the collision
float Bike_RiderDamage(float dotForward, float dotRight, float dotUp, float normalZ, bool isSanchez, float colSpeed);
// Whether the light steam of a slightly damaged bike pauses at that point of the frame counter plus the random seed of the bike
bool Bike_SteamSkipsFrame(int counter);
// Whether the steam of a damaged bike is emitted on that frame (counter plus random seed), which is every other pair of frames
bool Bike_SteamFrame(int counter);
// Scale of the right vector the smoke of a spinning wheel is moved by when the bike leans, from the lean angle and the lowest point of the bounding box
float Bike_LeanSmokeOffset(float leanAngle, float boundingMinZ);
// Angle between the view and the direction of a bike, from the absolute value of how much behind it the camera is
float Bike_ViewAngle(float behindness);
// Wobble of a burst wheel, from the rotation of the wheel and the amplitude of the wobble (0.05 front, 0.07 rear)
float Bike_BurstWheelWobble(float rotation, float amplitude);
// Roll of the chassis of a bike, from its lean angle
float Bike_ChassisRoll(float leanAngle);
// Direction of the front fork of a bike (x is 0), from its steer angle in degrees: y and z
void Bike_ForkAxis(float steerAngleDegrees, float *y, float *z);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
