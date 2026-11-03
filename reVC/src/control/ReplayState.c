//+ rouz edit (ChatGPT)
#include "ReplayState.h"
#include "InputState.h"
uint8_t Replay_mode;
bool Replay_playingBackFromFile;
uint32_t Replay_framesActiveLookAroundCam;
bool Replay_playerInRCBuggy;
bool Replay_allowLookAroundCam;
float Replay_distanceLookAroundCam, Replay_alphaAngleLookAroundCam, Replay_betaAngleLookAroundCam;
#ifdef GTA_REPLAY
void Replay_ProcessLookAroundCam(void)
{
    // Preserve the original input gate and ordered mouse reads
    float x_moved, y_moved, value;
    if (!Replay_allowLookAroundCam)
        return;
    x_moved = Input_newMouseState.x / 200.0f;
    y_moved = Input_newMouseState.y / 200.0f;
    if (x_moved > 0.01f || y_moved > 0.01f) {
        // Start or extend the original look-around timer
        if (Replay_framesActiveLookAroundCam == 0)
            Replay_distanceLookAroundCam = 9.0f;
        Replay_framesActiveLookAroundCam = 60;
    }
    if (Replay_playerInRCBuggy)
        Replay_framesActiveLookAroundCam = 0;
    if (!Replay_framesActiveLookAroundCam)
        return;
    // Update angles or distance before the native camera geometry
    --Replay_framesActiveLookAroundCam;
    Replay_betaAngleLookAroundCam += x_moved;
    if (Input_newMouseState.LMB && Input_newMouseState.RMB) {
        // Preserve the original nested minimum and maximum for zoom
        value = Replay_distanceLookAroundCam + 2.0f * y_moved;
        value = 15.0f < value ? 15.0f : value;
        Replay_distanceLookAroundCam = 3.0f > value ? 3.0f : value;
    } else {
        // Preserve the original nested minimum and maximum for elevation
        value = Replay_alphaAngleLookAroundCam + y_moved;
        value = 1.5f < value ? 1.5f : value;
        Replay_alphaAngleLookAroundCam = 0.1f > value ? 0.1f : value;
    }
    ReplayHost_ApplyLookAround();
}
#else
void Replay_ProcessLookAroundCam(void)
{
    // Keep replay-disabled builds independent of native replay callbacks
}
#endif

bool Replay_ShouldStandardCameraBeProcessed(void)
{
    // Preserve the original replay and camera-state short circuits
#ifdef GTA_REPLAY
    if (Replay_mode != REPLAY_MODE_PLAYBACK)
        return true;
    if (Replay_framesActiveLookAroundCam || Replay_playerInRCBuggy)
        return false;
    return ReplayHost_HasPlayerVehicle();
#else
    return true;
#endif
}
//- rouz edit (ChatGPT)
