//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Edge detection of an analogue stick pushed past a threshold, old holds the previous value of the stick and is updated
bool PadLogic_AnalogueEdge(int16_t stick, int16_t *old, bool negative);
// Look axis of the sniper mode from the stick and the digital pad
int16_t PadLogic_SniperAxis(int16_t axis, int16_t dpad);
// Look axis around the player from the right stick, the mouse camera uses a smaller dead zone
int16_t PadLogic_LookAroundAxis(float axis, bool lookBehind, bool mouseCam, float mouseDeadZone);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
