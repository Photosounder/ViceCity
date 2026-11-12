//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Whether civilians are drawn to an attractor type, shelters only when it rains
bool Civilian_IsAttractedTo(int type, float rain);
// Gate of the attractor search that only runs every few calls, cycleState counts the calls, returns true when the search should go on
bool Civilian_AttractorSearchDue(float rain, bool attractorUnk, bool hasAttractor, int32_t *cycleState, int32_t frequency);
// Whether a model is on the list of cars the ped wants to steal
bool Civilian_IsOnStealWishList(const uint32_t *list, int count, int32_t model);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
