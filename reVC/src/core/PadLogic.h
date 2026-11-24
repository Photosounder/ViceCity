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
// Value of a stick axis combined with a digital pad: the one with the larger absolute value wins (the pad when equal)
int16_t Pad_AxisOrDPad(int16_t axis, int16_t dpad);
// Position of an XInput stick in game units when it is pushed further than the dead zone of 0.3 on either axis (returns whether it is): x and y are -1 to 1, the game y is inverted
bool Pad_XInputStick(float x, float y, int32_t *outX, int32_t *outY);
// Wanted level set by the cheat: two more than the current one, at most 6
int Pad_CheatWantedLevel(int wantedLevel);
// Time left of the pad shake after a time step in milliseconds, never negative
int Pad_ShakeDurationLeft(int shakeDur, int timeStepMs);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
