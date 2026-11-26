//+ rouz edit (ChatGPT)
#pragma once
#include "VectorFwd.h" // rouz edit (Claude)
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#ifdef __cplusplus
extern "C" {
#endif
extern uint8_t Replay_mode;
extern bool Replay_playingBackFromFile;
extern uint32_t Replay_framesActiveLookAroundCam;
extern bool Replay_playerInRCBuggy;
extern bool Replay_allowLookAroundCam;
extern float Replay_distanceLookAroundCam, Replay_alphaAngleLookAroundCam, Replay_betaAngleLookAroundCam;
void ReplayHost_ApplyLookAround(void);
void Replay_ProcessLookAroundCam(void);
bool ReplayHost_HasPlayerVehicle(void);
bool Replay_ShouldStandardCameraBeProcessed(void);
enum { REPLAY_MODE_RECORD = 0, REPLAY_MODE_PLAYBACK = 1 };
static inline bool Replay_IsPlayingBackFromFile(void)
{
    // Preserve the disabled-replay configuration behavior
#ifdef GTA_REPLAY
    return Replay_playingBackFromFile;
#else
    return false;
#endif
}
static inline bool Replay_IsPlayingBack(void)
{
    // Preserve the disabled-replay configuration behavior
#ifdef GTA_REPLAY
    return Replay_mode == REPLAY_MODE_PLAYBACK;
#else
    return false;
#endif
}
//+ rouz edit (Claude)
// Offset of the look-around replay camera from its focus, from the distance and the vertical (alpha) and horizontal (beta) angles
void Replay_LookAroundOffset(float distance, float alpha, float beta, CVector *offset);
// Orientation of the look-around replay camera that looks from a position at a focus point (forward, right and up vectors, all normalised)
void Replay_LookAroundBasis(const CVector *focus, const CVector *cameraPos, CVector *forward, CVector *right, CVector *up);
//- rouz edit (Claude)
//+ rouz edit (Claude)
// Value of a replay packet field: a factor times the value limited to a range
float Replay_ScaledClamp(float factor, float value, float low, float high);
// Velocity stored in a replay packet for a vehicle: 8000 times the speed limited to -4..4
float Replay_VehicleVelocity(float speed);
//- rouz edit (Claude)
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
