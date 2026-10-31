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
void Pager_BackendBrief(uint16_t *text,int32_t n1,int32_t n2,int32_t n3,int32_t n4,int32_t n5,int32_t n6,uint16_t *string)
{
    // Forward the captured original message payload to the native previous-brief owner
    Messages_AddToPreviousBriefArray(text,n1,n2,n3,n4,n5,n6,string);
}
void Pager_BackendInsertNumbers(uint16_t *str,int32_t n1,int32_t n2,int32_t n3,int32_t n4,int32_t n5,int32_t n6,uint16_t *output)
{
    // Expand the borrowed original text with the native UTF-16 substitution operation
    Messages_InsertNumberInString(str,n1,n2,n3,n4,n5,n6,output);
}
uint16_t Pager_BackendLength(uint16_t *str)
{
    // Query the native UTF-16 length at the original call site
    return Messages_GetWideStringLength(str);
}
void Pager_BackendSetMessage(uint16_t *message)
{
    // Publish the original scroll slice through the existing synchronous HUD copy
    CHud::SetPagerMessage(message);
}
//- rouz edit (ChatGPT)
