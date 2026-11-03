//+ rouz edit (ChatGPT)
#include "common.h"
#include "Messages.h"
#include "Hud.h"
#include "Camera.h"
#include "Pager_backend.h"

bool Pager_BackendWidescreen(void)
{
    // Read the live camera flag at the original short-circuit position
    return TheCamera.m_WideScreenOn;
}
bool Pager_BackendDrawHud(void)
{
    // Read the live HUD visibility flag after the original camera test
    return CHud::m_Wants_To_Draw_Hud;
}
uint16_t Pager_BackendBigMessage0(void)
{
    // Read the original first message channel only when the preceding gates permit it
    return CHud::m_BigMessage[0][0];
}
uint16_t Pager_BackendBigMessage2(void)
{
    // Read the original third message channel only when the preceding gates permit it
    return CHud::m_BigMessage[2][0];
}
void Pager_BackendSetMessage(uint16_t *message)
{
    // Publish the original scroll slice through the existing synchronous HUD copy
    CHud::SetPagerMessage(message);
}
//- rouz edit (ChatGPT)
