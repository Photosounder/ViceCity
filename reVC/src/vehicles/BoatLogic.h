//+ rouz edit (Claude)
#pragma once
#include "BoatLayout.h"

#ifdef __cplusplus
extern "C" {
#endif
// Wake trail upkeep, the time step is in game frames
void Boat_PruneWakeTrail(CBoatData *self, float timeStep);
void Boat_AddWakePoint(CBoatStruct *self, float x, float y);
void Boat_ClearNodes(CBoatData *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
