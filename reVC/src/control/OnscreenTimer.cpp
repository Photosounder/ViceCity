//+ rouz edit (ChatGPT)
#include "common.h"
#include "Script.h"
#include "Replay.h"
#include "Hud.h"
#include "Camera.h"
#include "OnscreenTimer_backend.h"

int32_t *OnscreenTimer_BackendVariable(int32_t offset)
{
    // Return the actual script variable pointer at the original lookup point
    return CTheScripts::GetPointerToScriptVariable(offset);
}

bool OnscreenTimer_BackendDrawHud(void)
{
    // Read the live HUD visibility flag without resetting retained display state
    return CHud::m_Wants_To_Draw_Hud;
}
bool OnscreenTimer_BackendWidescreen(void)
{
    // Read the camera flag only when the original tick condition reaches it
    return TheCamera.m_WideScreenOn;
}
//- rouz edit (ChatGPT)
