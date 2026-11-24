//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "GarageData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Garage geometry and door state helpers, a position is a 3D point
bool Garage_IsPointInside(const CGarageData *garage, float x, float y, float z);
bool Garage_IsPointInsideWithMargin(const CGarageData *garage, float x, float y, float z, float margin);
float Garage_CalcDistToRectangleSquared(const CGarageData *garage, float x, float y);
float Garage_CalcSmallestDistToDoorSquared(const CGarageData *garage, float x, float y);
void Garage_OpenThis(CGarageData *garage);
void Garage_CloseThis(CGarageData *garage);
// Up vector of a garage door that is rotated open to a position between 0 and 1, from the position and the x and y of the forward vector of the door
void Garage_RotatedDoorUp(float position, float forwardX, float forwardY, float up[3]);
// Position of a garage door after it closed for a frame, going down at a speed per time step but not below 0
float Garage_DoorClosedPos(float doorPos, float speed, float timeStep);
// Position of a garage door after it opened for a frame, going up at a speed per time step but not above its height
float Garage_DoorOpenedPos(float doorPos, float doorHeight, float speed, float timeStep);
// Lowest and highest coordinate on an axis of a garage given by three of its corners (the fourth is the opposite of the first)
void Garage_AxisBounds(float c1, float c2, float c3, float *inf, float *sup);
// Whether the player is too far from the centre of a garage on either axis (more than the limit)
bool Garage_PlayerIsFar(float playerX, float playerY, float centerX, float centerY, float limit);
// Money of the player after paying a price at a garage, never below 0
int32_t Garage_MoneyAfterPayment(int32_t money, int32_t price);
// Health of a car that is saved from blowing up: at least 300
float Garage_SavedHealth(float health);
// Engine status of a car that is saved from blowing up: at least 275
int Garage_SavedEngineStatus(int status);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
