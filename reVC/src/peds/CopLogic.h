//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Whether a cop with a stinger should throw it in front of the vehicle of the player, and the heading the cop has to face when so
// vehDist is the offset from the vehicle to the cop and dirVehGoing the move speed of the vehicle, both on the ground plane, forwardHeading is the heading of the vehicle
bool CopLogic_ShouldDeployStinger(float vehDistX, float vehDistY, float dirVehGoingX, float dirVehGoingY, float forwardHeading, float *angle);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
