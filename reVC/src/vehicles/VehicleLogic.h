//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "PlaceableData.h"
#include "PhysicalData.h"
#include "VehicleData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Vehicle state helpers that only need the entity data
bool VehicleLogic_CarHasRoof(const CVehicleData *vehicle);
bool VehicleLogic_IsUpsideDown(const CPlaceableData *placeable);
bool VehicleLogic_IsOnItsSide(const CPlaceableData *placeable);
bool VehicleLogic_CanPedOpenLocks(const CVehicleData *vehicle, bool pedIsPlayer);
bool VehicleLogic_CanDoorsBeDamaged(const CVehicleData *vehicle);
bool VehicleLogic_CanPedEnterCar(const CPlaceableData *placeable, const CPhysicalData *physical, const CVehicleData *vehicle);
bool VehicleLogic_CanPedExitCar(const CPlaceableData *placeable, const CPhysicalData *physical, const CVehicleData *vehicle, bool jumpExit);
// Occupant bookkeeping, peds are only compared by address
bool VehicleLogic_IsDriver(const CVehicleData *vehicle, const CPed *ped);
bool VehicleLogic_IsPassenger(const CVehicleData *vehicle, const CPed *ped);
void VehicleLogic_UpdatePassengerList(CVehicleData *vehicle);
// Count the car alarm down by the time step in milliseconds
void VehicleLogic_ProcessCarAlarm(CVehicleData *vehicle, uint32_t stepMs);

// Put a passenger in the first free seat, or in seat n, false when there is no room
bool VehicleLogic_AddPassengerToFirstFreeSeat(CVehicleData *vehicle, CPed *passenger);
bool VehicleLogic_AddPassengerToSeat(CVehicleData *vehicle, CPed *passenger, uint8_t n);
// Take a passenger out of the seats, trains have all eight seats usable
void VehicleLogic_RemovePassenger(CVehicleData *vehicle, const CPed *passenger, bool isTrain);

// Move a passenger to another free seat so that a new one can get in, isRoom tells whether a ped may leave through a door and is only asked when the move depends on it
typedef bool (*VehicleRoomFn)(void *context, uint32_t door);
bool VehicleLogic_ShufflePassengersToMakeSpace(CVehicleData *vehicle, VehicleRoomFn isRoom, void *context);

// Honk for a driver who is held up, with a random delay between honks, returns true when the driver has to complain out loud as well, random is only called when the delay ran out
typedef uint16_t (*VehicleRandomFn)(void);
bool VehicleLogic_PlayCarHorn(CVehicleData *vehicle, bool alarmOn, VehicleRandomFn random);

// Whether the vehicle can be deleted, this looks at the creation mode and at the state of its occupants
bool VehicleLogic_CanBeDeleted(const CVehicleData *vehicle);

// Factor that stretches the input of the stick of a plane from a circle to a square, from the angle of the stick
float VehicleLogic_PlaneSteerMultiplier(float steerUpDown, float steerLeftRight);
// Acceleration of the engine of a plane from the state of the pedals, its speed forward and the thrust it has, the thrust is scaled down for the unused and the normal plane
float VehicleLogic_PlaneThrustAccel(float pedalState, float forwardSpeed, float thrust, float thrustFallOff, float thrustFallOffBack, bool isUnusedPlane, bool isPlane);
// Thrust of a helicopter from the pedals, whether it is a radio controlled one, its thrust, how much it falls off with the speed upwards, the speed upwards and the height, as a factor of the gravity
float VehicleLogic_HeliThrust(float pedalState, bool isRCHeli, float flyingThrust, float thrustFallOff, float upSpeed, float height);
// How much a helicopter leans to the side and to the front from its orientation, to be pushed back: the heights of the up, right and forward vectors and the limit of the lean
void VehicleLogic_HeliStabilise(float upZ, float rightZ, float forwardZ, float formLift, float *upRight, float *upFwd);
// What the friction of a wheel with the ground needs to know
typedef struct VehicleWheelInput {
	float contactSpeedFwd, contactSpeedRight;	// speed of the contact point along the wheel and sideways
	int wheelsOnGround;
	float thrust, brake, adhesion;
	float timeStep;
	float tractionLoss;			// of the handling of the vehicle
	bool slippery;				// the slippery cheat
	bool isFrontWheel;
	bool isBurst;
	float burstSpeedMax, burstTyreMod;
	float gasPedal;
	bool isBike;
	bool isRCBandit;
	float mass;				// mass of the handling
	float wheelFriction;
	bool useTimeStepFix;
	float timeStepFix;
	int wheelState;				// state of the wheel before
} VehicleWheelInput;
// What the friction of a wheel gives
typedef struct VehicleWheelOutput {
	float fwd, right;			// forces along the wheel and sideways
	int wheelState;
	bool braking, driving;
} VehicleWheelOutput;
// Friction of a wheel with the ground: the forces that slow it down or push it, and the state it ends in, alreadySkidding remembers that a wheel skidded
void VehicleLogic_ProcessWheel(const VehicleWheelInput *in, bool *alreadySkidding, VehicleWheelOutput *out);
// What the friction of the wheel of a bike with the ground needs to know
typedef struct VehicleBikeWheelInput {
	float contactSpeedFwd, contactSpeedRight;	// speed of the contact point along the wheel and sideways
	int wheelsOnGround;
	float thrust, brake, adhesion, destabTraction;
	float timeStep;
	float tractionLoss;			// of the handling of the vehicle
	bool isBurst;
	float burstSpeedMax, burstTyreMod;
	float gasPedal;
	bool isBike;
	bool isRCBandit;
	float handlingMass;			// mass of the handling
	float mass;				// mass of the vehicle
	float wheelFriction;
	bool useTimeStepFix;
	float timeStepFix;
	int wheelState;				// state of the wheel before
} VehicleBikeWheelInput;
// What the friction of the wheel of a bike gives
typedef struct VehicleBikeWheelOutput {
	float fwd, right;			// forces along the wheel and sideways
	int wheelState;
	bool braking, driving, reversing;
} VehicleBikeWheelOutput;
// Friction of the wheel of a bike with the ground, alreadySkidding remembers that a wheel skidded
void VehicleLogic_ProcessBikeWheel(const VehicleBikeWheelInput *in, bool *alreadySkidding, VehicleBikeWheelOutput *out);
// Strength of the glare of the sun on a car from how much the car faces the camera and the distance to it, returns false when there is no glare
bool VehicleLogic_SunGlareStrength(float fwdness, float dist, float *strength);
// Colour channel of the star of a glare from its intensity and the red, green or blue of the core of the sun
int VehicleLogic_SunGlareChannel(float intensity, float sunCore);
// Finds the vertex of the second triangle of a quad that is not in the first triangle, returns how many vertices of the second triangle are not in the first one, vertex gets the last one
int VehicleLogic_FindQuadVertex(int a1, int b1, int c1, int a2, int b2, int c2, int *vertex);
// Colour of the dust a helicopter blows up from a surface (eSurfaceType) that is not water
void VehicleLogic_HeliDustColor(int surface, unsigned char *r, unsigned char *g, unsigned char *b);
// Colour channel of the spray a helicopter blows up from water, from the directional and the ambient light
unsigned char VehicleLogic_HeliWaterChannel(float directional, float ambient);
// The kind of damage of a weapon (eWeaponType) a vehicle can be proof against
enum {
	VEHICLE_DAMAGE_OTHER,		// no proof against it
	VEHICLE_DAMAGE_MELEE,
	VEHICLE_DAMAGE_BULLET,
	VEHICLE_DAMAGE_EXPLOSION,
	VEHICLE_DAMAGE_FIRE,
	VEHICLE_DAMAGE_COLLISION
};
// Kind of damage a weapon does, bullets and explosions also frighten the driver
int VehicleLogic_DamageProofKind(int weaponType);
// Chance out of 128 that a bullet of a weapon bursts a tyre, depending on whether the shooter is the player
int VehicleLogic_TyreBurstAccuracy(int weaponType, bool shooterIsPlayer);
// Whether the lights of a vehicle should be on, from the time of the day, the random seed of the vehicle (each one has its own sunset and sunrise) and the fog and the wet roads
bool VehicleLogic_ShouldLightsBeOn(int hours, int minutes, uint16_t randomSeed, float foggyness, float wetRoads);
// State the lights of a vehicle take when they should change: wrecked vehicles keep them, abandoned ones only switch them off when they are far from the camera
bool VehicleLogic_NextLightsState(bool shouldBeOn, bool lightsOn, bool isWrecked, bool isAbandoned, bool farFromCamera);
// Phase of the flashing lights of an alarm that sounds: on or off, from the time in milliseconds, both false when no alarm sounds
void VehicleLogic_AlarmFlash(bool alarmIsOn, uint32_t timeMs, bool *lightsOn, bool *lightsOff);
// Brightness and size of the corona of a headlight seen from in front of the vehicle, from how far behind it the viewer is (negative when in front)
void VehicleLogic_HeadlightGlow(float behindness, float *intensity, float *size);
// Factor the vector from the seat to the door is scaled by to find where a ped that leaves a vehicle ends up, from the length of that vector
float VehicleLogic_ExitCheckScale(float length);
// Whether the ceiling above the door of a vehicle blocks a ped that leaves it, from the height of the ceiling and of the door
bool VehicleLogic_ExitBlockedAbove(float ceilingZ, float doorZ);
// Whether the ground below the door of a vehicle is higher than the ceiling above it, so that there is no room for a ped that leaves it, from the height of the ceiling (0 when there is none) and of the ground
bool VehicleLogic_ExitBlockedBelow(float ceilingZ, float groundZ);
// Factor a speed vector is multiplied by to be limited to a given length, from its squared length and the limit, 1 when it is below it
float VehicleLogic_SpeedLimitScale(float speedSqr, float limit);
// Resistance of a plane to rolling: returns the change of the turn speed around its y axis in one frame, from the turn speed, the resistance of the handling of that axis (turnRes), the speed resistance and the time step
float VehicleLogic_PlaneRollResistance(float turnSpeedY, float turnResY, float speedResY, float timeStep);
// Resistance of a helicopter to yawing: returns the factor its turn speed around the z axis is multiplied by in one frame and in resistance the change of that turn speed, from the turn speed, the resistance of the handling of that axis (turnRes), the speed resistance and the time step
float VehicleLogic_HeliYawResistance(float turnSpeedZ, float turnResZ, float speedResZ, float timeStep, float *resistance);
// Acceleration of a plane around its yaw or pitch axis, from the stabilisation of the handling, the speed of the tail across that axis, the gain of the handling, the stick and the speed along the plane
float VehicleLogic_PlaneAxisAccel(float stab, float tailSpeed, float gain, float steer, float fwdSpeed);
// Acceleration of a plane around its roll axis, from whether it is an RC plane, the direction the player looks in (RC plane only), the stick and the roll gain of the handling
float VehicleLogic_PlaneRollAccel(bool isRCPlane, float lookDirection, float steerLR, float roll);
// Speed of the roll stabilisation of a plane, from the stabilisation of the handling, the vertical parts of its right and forward vectors and the dot product of its right vector with the one of the stabilisation
float VehicleLogic_PlaneRollStabilise(float rollStab, float rightZ, float rightDotStabilise, float forwardZ);
// Lift acceleration of a plane, from the lift of the form and the attack of the handling, the part of its speed that goes up and the speed along the plane
float VehicleLogic_PlaneLiftAccel(float formLift, float attackLift, float liftRatio, float fwdSpeed);
// Ratio of the speed of the Dodo along one of its axes to its total speed, negated: how much it is pushed back towards its direction
float VehicleLogic_DodoImpulse(float axisSpeed, float moveSpeed);
// Magic factor the forces of the Dodo are scaled by, from its squared speed and its speed along its direction
float VehicleLogic_DodoMagic(float moveSpeedSqr, float fwdSpeed);
// Turn force that keeps the Dodo straight and steers it, from the impulse (see above) and the steering angle, or the stick (pitch) for the other axis, with the weights of both (sideWeight 0.003 and steerWeight 0.001 for the sideways axis, 0.002 and 0.001 for the up axis)
float VehicleLogic_DodoTurnForce(float impulse, float impulseWeight, float steer, float steerWeight, float magic, float turnMass, float timeStep);
// Sideways force that pushes the Dodo back to its direction, from the impulse, the magic factor, the mass and the time step
float VehicleLogic_DodoSideForce(float impulse, float magic, float mass, float timeStep);
// Lift force of the Dodo, from the up impulse, the magic factor, the mass, the time step, the gravity and the height of the plane, limited above a height of 100
float VehicleLogic_DodoLiftForce(float upImpulse, float magic, float mass, float timeStep, float gravity, float height);
// Horizontal direction the fixed machine guns of a vehicle shoot in, from the forward vector of the vehicle, normalised with a length of at least 0.1
void VehicleLogic_FixedGunDirection(float forwardX, float forwardY, float *dirX, float *dirY);
// Whether every component of a speed vector is smaller than the limit in absolute value
bool VehicleLogic_AllSpeedsBelow(float x, float y, float z, float limit);
// Whether any component of a speed vector is larger than the limit in absolute value
bool VehicleLogic_AnySpeedAbove(float x, float y, float z, float limit);
// Position of the left light of a vehicle from the position of the right one, by moving it along the right vector of the vehicle by twice the x offset of the light
void VehicleLogic_MirrorLight(const float lightR[3], const float right[3], float offsetX, float lightL[3]);
// Whether two positions are within a limit of each other on both the x and y axes (a square, not a circle)
bool VehicleLogic_WithinSquare(float ax, float ay, float bx, float by, float limit);
// Factor a speed is multiplied by in a frame for a resistance defined per time unit: the resistance raised to the power of the time step
float VehicleLogic_ResistancePerStep(float resistance, float timeStep);
// Value that counts down by an amount per frame but never goes below 0
float VehicleLogic_DecayToZero(float value, float amount);
// Sideways acceleration of a flying vehicle from its side slip factor and its speed to the side: it grows with the square of the speed in the direction of the speed
float VehicleLogic_SideSlipAccel(float sideSlip, float sideSpeed);
// Stabilising pitch or roll of a helicopter: the stability factor times the speed along an axis, limited to -200..1.3
float VehicleLogic_StabilityClamp(float stability, float speedAlongAxis);
// Whether a collision point is too far from the rotor of a helicopter to count: further along the rotor axis than twice its half thickness and further along the axis than the disregard multiple of its distance along the collision normal
bool VehicleLogic_RotorMissesCollision(float axisDir, float colDir, float semiThickness, float disregardMult);
// Distance of the glare on a window from the middle of the quad, 1.4 times the smaller of the distances on x and y
float VehicleLogic_GlareDistance(float dx, float dy);
// Health of the player after the freebie of an ambulance: 20 more but at most the maximum health, and never lower than before
float VehicleLogic_AmbulanceFreebieHealth(float health, float maxHealth);
// Armour of the driver after the freebie of an enforcer: at least the maximum armour
float VehicleLogic_EnforcerFreebieArmour(float armour, float maxArmour);
// Radius of a vehicle around its origin from the corners of its bounding box: the larger of the distances of the two corners
float VehicleLogic_BoundingRadius(const float boxMin[3], const float boxMax[3]);
// Whether a bike has tipped too far to stay standing: the right vector points up more than 0.35 or the forward vector more than 0.5
bool VehicleLogic_BikeTippedOver(float rightZ, float forwardZ);
// Value limited between a low and a high bound, as used for the inputs and speeds of vehicles
float VehicleLogic_ClampValue(float value, float low, float high);
// Whether any component of a speed vector is at least the limit in absolute value
bool VehicleLogic_AnySpeedAtLeast(float x, float y, float z, float limit);
// Whether two heights differ by less than a limit
bool VehicleLogic_HeightDifferenceBelow(float zA, float zB, float limit);
// Whether a distance is not smaller than a limit, so it is beyond it
bool VehicleLogic_IsBeyond(float distance, float limit);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
