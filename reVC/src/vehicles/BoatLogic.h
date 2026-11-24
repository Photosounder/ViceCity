//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "BoatLayout.h"

#ifdef __cplusplus
extern "C" {
#endif
// Wake trail upkeep, the time step is in game frames
void Boat_PruneWakeTrail(CBoatData *self, float timeStep);
void Boat_AddWakePoint(CBoatStruct *self, float x, float y);
void Boat_ClearNodes(CBoatData *self);
// Factors the speed of a boat in local space is multiplied with by the water, from the volume of the boat under water, its mass, the suspension force level of its handling, whether it is a skimmer with its resistance, the speed forward, the resistance of its handling and the time step
void Boat_WaterResistanceFactors(float volumeUnderWater, float mass, float suspensionForceLevel, bool isSkimmer, float seaPlaneWaterResistance, float forwardSpeed,
	float moveResX, float moveResY, float moveResZ, float timeStep, float *fx, float *fy, float *fz);
// Vertical speed of a boat after the water resistance, which is stronger on the way up
float Boat_WaterResistanceVertical(float speedZ, float fz);

// Whether a boat at an offset from the camera is within the range its wake gets drawn from, camera direction is normalised, distSq gets the squared distance on the ground
bool Boat_WakeInViewRange(float camToBoatX, float camToBoatY, float camForwardX, float camForwardY, float *distSq);
// Picks the slot of the wake list a boat takes when the list is full, from the squared distances of the boats in it to the camera, returns -1 for none
int Boat_PickWakeSlotToReplace(const float *slotDistSq, int numSlots, float distSq, bool isPlayerBoat);

// Alpha of the rotor of a seaplane from the speed it turns with, the faster it turns the more transparent it gets
int Boat_SeaplaneRotorAlpha(float movingSpeed);
// Speed of the little wind propeller of a Rio after the wind pushes it, from the heading of the propeller and of the boat, the wind, its speed and the time step
float Boat_RioPropellerSpeed(float flapHeading, float boatHeading, float wind, float movingSpeed, float timeStep);

// Colour channel of the spray from the time cycle: scale * (ambient + directionalFactor * directional) truncated and kept between 0 and 255
int Boat_SprayColorChannel(float scale, float ambient, float directionalFactor, float directional);
// What a collision does to a boat that was damaged enough
typedef struct BoatCollisionResult {
	float health;			// health after the collision
	bool driverAnnoyed;		// the driver has something to say
	bool setOnFire;			// the boat is set on fire by whoever damaged it last
} BoatCollisionResult;
// Applies the damage of a collision above 25 to the health of a boat, for the vehicle of the player or another one, driven or not, with a damage reduction
BoatCollisionResult Boat_CollisionDamage(float health, float collisionDamage, bool isPlayerVehicle, bool takeLessDamage, bool hasDriver);
// How deep the propeller is in the water from the water level and its height, from 0 to 1
float Boat_PropellerDepth(float waterLevel, float propellerZ);
// Force of a boat whose propeller is slowing it down from the traction loss, its speed forward, the turn mass, the gas pedal and whether it is the boat of the player
float Boat_SlowAheadForce(float tractionLoss, float speedForward, float turnMass, float gasPedal, bool isPlayer);

// Speed on an axis after it is limited so that the boat slows down and is pushed back when it nears the edge of the world, from the position, the end of the world on both sides each one with a margin
float Boat_WorldEdgeSpeed(float speed, float position, float maxEdge, float minEdge);
// Drag on the turn speed of a boat in its local space: turnY and turnZ get the drag of the resistance, returns the force up that comes from the turn around X
float Boat_TurnDrag(float turnResX, float turnResY, float turnResZ, float timeStep, float turnX, float *turnY, float *turnZ, float turnMass);
// Speeds a boat that falls into the water gets, speedUp pushes it up and speedForward slows it, from the change of volume under water, its speed, the brake deceleration and the brake bias
void Boat_SplashImpulse(int deltaVolume, float speedSqr, float speedZ, float speedAlongForward, float brakeDeceleration, float brakeBias, float *speedUp, float *speedForward);
// Speed the rotor of a seaplane turns with after a frame
float Boat_SkimmerMovingSpeed(float movingSpeed, float timeStep, bool isPlayer);

// Kinds of boats that splash in their own way
enum {
	BOAT_SPLASH_RIO,
	BOAT_SPLASH_SQUALO,
	BOAT_SPLASH_REEFER,
	BOAT_SPLASH_COASTG,
	BOAT_SPLASH_DINGHY,
	BOAT_SPLASH_DEFAULT
};
// Where and how a boat splashes on the sides of its bow, all of them are factors of the speed or the length of the boat except for the offsets
typedef struct BoatSplashParams {
	float sizeFactor;		// size of the splash from the speed
	float frontFactor;		// distance of the bow from the middle from the length of the boat
	float moveFactor;		// direction of the splash from the speed vector
	float upFactor;			// speed of the splash upwards from the speed
	float rightFactor;		// speed of the splash sideways from the speed
	float rightOffset;		// distance of the splash to the side
	float upOffset;			// height of the splash above the position, 0 for none
	bool addBuoyancePoint;		// whether the splash starts from the buoyance point
} BoatSplashParams;
BoatSplashParams Boat_SplashParams(int kind);

// Sine and cosine of the angle the propeller thrust of a boat is turned by, from the steering angle and the steer factor
void Boat_SteerSinCos(float steerAngle, float steerFactor, float *steerSin, float *steerCos);

// Thrust force of the propeller of a boat in the world, from the depth of the propeller, the gas pedal, the engine acceleration, the mass and the world direction of the thrust: a rising force is turned into a falling one, and on land reversing multiplies the horizontal force by 5 and the force never points down
void Boat_PropellerThrust(float propellerDepth, float gasPedal, float engineAcceleration, float mass, const float dir[3], bool onLand, float force[3]);

// Value of the up and down steering of a seaplane after a frame: it moves towards the target by a part of the difference that grows with the time step (at most 1)
float Boat_SmoothSteeringUD(float current, float target, float timeStep);
// Alpha of the windscreen of a seaplane, from the alpha of the plane: it is more transparent the more opaque the plane is, never below 0
int Boat_WindscreenAlpha(int alpha);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
