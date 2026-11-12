//+ rouz edit (Claude)
#pragma once
#include "HeliLayout.h"
#include "HeliStatus.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Offset of the nth swat rope position relative to the helicopter, zero for an unknown index
void Heli_FindSwatPositionRelativeToHeli(int n, float *x, float *y, float *z);
void Heli_ClearNodes(CHeliData *self);
// Bring a helicopter down, it spins one way or the other and explodes at the given game time
void Heli_ShootDown(CHeliData *self, bool spinPositive, uint32_t explosionTime);
// Add the damage of a bullet and tell whether the helicopter has taken more than it can bear, the Catalina one less than the others
bool Heli_AddBulletDamage(CHeliData *self, int32_t damage);
// Where a bullet that passes a helicopter on a line hits it, a bit before the helicopter along the line
void Heli_BulletPosition(const float *line0, const float *line1, const float *heliPos, float *out);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
