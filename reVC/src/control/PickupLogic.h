//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "PickupData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Pickup table helpers, the table has count entries
bool PickupLogic_TestForPickupsInBubble(const CPickupData *pickups, int count, float x, float y, float z, float range);
bool Pickups_TryToMerge(CPickupData *pickups, int count, float x, float y, float z, uint8_t type, int16_t modelIndex, uint32_t quantity);
bool PickupLogic_IsPickUpPickedUp(int32_t *collected, int count, int32_t pickupId);
void PickupLogic_PassTime(CPickupData *pickups, int count, uint32_t time);
int32_t PickupLogic_GetActualPickupIndex(const CPickupData *pickups, int32_t index);
// Pickups to get rid of around a point: money and timed pickups within the radius, in table order
int Pickups_SelectUnnecessary(const CPickupData *pickups, int count, float x, float y, float z, float radius, int *out, int maxOut);
// Sine of the phase of the pulse of a pickup, from the time in milliseconds plus the address of the pickup and the mask that gives the period (0x7FF is about 2 seconds)
float Pickups_PulseSin(uint32_t timePlusEntity, uint32_t mask);
// Angle a pickup is rotated by, from the time in milliseconds (plus the address of the pickup when it is not synchronised) and the mask that gives the period
float Pickups_SpinAngle(uint32_t time, uint32_t mask);
// Strength of the glow of a mine, a money bag or a collectable that fades with the distance, as the integer the original code computed from the maximum distance, the distance, the pulse and the weights of the colour (scale and offset on the pulse, multiplier of the channel)
int32_t Pickups_GlowChannel(float maxDist, float dist, float pulse, float scale, float offset, float multiplier);
// State of a flickering corona of a pickup from a phase between 0 and 7 of the timer: 0 off, 1 flickers (on 3 times out of 4, decided by a random number) and 2 on
int Pickups_CoronaPhase(int phase);
// Radius of the light of a pickup from a random number
float Pickups_LightRadius(int random);
// Scale of the model of a pickup from its biggest dimension, models that are smaller than 1.2 are enlarged by 60% of what is needed to reach it
float Pickups_ModelScale(float maxDimension);
// Alpha of the message of a pickup from its distance and the maximum distance
int Pickups_MessageAlpha(float dist, float maxDist);
// Number of pickups the money of a dead ped or a spilled bag is split into, at most 7
int Pickups_MoneyPickupCount(int money);
// Offset of a pickup of money around its origin on a ring of radius 1.5, from a random number (x with the sine, y with the cosine)
float Pickups_MoneyRingOffset(int random, bool isY);
// Offset of the pickups of a dead ped around the ped on a ring of radius 1.5, from a random number (x with the sine, y with the cosine)
float Pickups_DeadPedRingOffset(int random, bool isY);
// Scale of the text of a message of a pickup, from the width or the height of the sprite and the largest scale
float Pickups_MessageScale(float spriteSize, float maxScale);
// Cosine and sine of the spin angle of a pickup, both scaled by the scale of the model
void Pickups_SpinCosSin(float angle, float scale, float *c, float *s);
// Whether a player on foot touches a pickup: less than 2 units apart in height and with a squared distance on the ground below 1.8
bool Pickups_OnFootTouches(float playerX, float playerY, float playerZ, float objectX, float objectY, float objectZ);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
