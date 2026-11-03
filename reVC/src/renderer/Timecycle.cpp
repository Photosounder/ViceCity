//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Camera.h"
#include "Shadows.h"
#include "CutsceneMgr.h"
#include "Timecycle_backend.h"
static_assert(int(TIMECYCLE_MOTION_BLUR_NONE)==int(MOTION_BLUR_NONE)&&int(TIMECYCLE_MOTION_BLUR_LIGHT_SCENE)==int(MOTION_BLUR_LIGHT_SCENE),"Original native blur constants");
CVector TimeCycle_ToNativeVector(TimeCycleVector value)
{
    // Copy the C value explicitly for native vector operations while canonical storage remains in C
    return CVector(value.x,value.y,value.z);
}
void TimeCycle_BackendDebug(const char *message)
{
    // Retain the original formatted debug output without moving any loading decisions into C++
    debug("%s",message);
}
uint8_t (*TimeCycle_BackendWorkBuffer(void))[55000]
{
    // Borrow the complete original shared file buffer with its actual compile-time extent
    return &work_buff;
}
int32_t TimeCycle_BackendBlurType(void)
{
    // Read the live camera blur type separately at each original short-circuit access
    return TheCamera.m_BlurType;
}
void TimeCycle_BackendSetMotionBlur(int32_t red,int32_t green,int32_t blue,int32_t alpha,int32_t type)
{
    // Invoke the original camera primitive after its original argument conversions
    TheCamera.SetMotionBlur(red,green,blue,alpha,type);
}
void TimeCycle_BackendShadowValues(TimeCycleVector direction,float *frontX,float *frontY,float *sideX,float *sideY,float *displacementX,float *displacementY)
{
    // Preserve the native shadow primitive and the six live output addresses selected by the C owner
    CShadows::CalcPedShadowValues(TimeCycle_ToNativeVector(direction),frontX,frontY,sideX,sideY,displacementX,displacementY);
}
float TimeCycle_BackendCameraForwardZ(void)
{
    // Read the live camera forward vector only after the original shadow calculation
    return TheCamera.GetForward().z;
}
bool TimeCycle_BackendCutscene(void)
{
    // Query cutscene playback only after the original force-rain and cull-zone tests
    return Cutscene_IsRunning(); // rouz edit (ChatGPT)
}
//- rouz edit (ChatGPT)
