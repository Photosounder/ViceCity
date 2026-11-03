//+ rouz edit (ChatGPT)
#pragma once
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
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
