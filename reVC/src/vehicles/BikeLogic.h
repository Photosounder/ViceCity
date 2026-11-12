//+ rouz edit (Claude)
#pragma once
#include "BikeLayout.h"

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
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
