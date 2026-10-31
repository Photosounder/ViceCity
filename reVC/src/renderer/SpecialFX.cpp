//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "SpecialFX.h"
#include "Markers3D_backend.h"
#include "RenderBuffer.h"
#include "Timer.h"
#include "Sprite.h"
#include "Font.h"
#include "Text.h"
#include "TxdStore.h"
#include "FileMgr.h"
#include "FileLoader.h"
#include "Timecycle.h"
#include "Lights.h"
#include "ModelIndices.h"
#include "VisibilityPlugins.h"
#include "World.h"
#include "PlayerPed.h"
#include "Particle.h"
#include "Shadows.h"
#include "General.h"
#include "Camera.h"
#include "Shadows.h"
#include "main.h"
#include "ColStore.h"
#include "Coronas.h"
#include "Script.h"
#include "DMAudio.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif











//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#include "SpecialFX_backend.h"
void * SpecialFX_BackendWeaponMatrix(void)
{
    // Retain the original lazy weapon matrix and its C++ lifetime
    static CMatrix matrix;return &matrix;
}
void * SpecialFX_BackendCallbackMatrix(void)
{
    // Retain the independent original lazy callback matrix
    static CMatrix matrix;return &matrix;
}
void SpecialFX_BackendSetMatrix(void *target,void *source)
{
    // Preserve original typed temporary construction and matrix assignment
    *(CMatrix *)target=CMatrix((RwMatrix *)source);
}
SpecialFXMatrix SpecialFX_BackendReadMatrix(void *matrix)
{
    // Copy actual named matrix coefficients without assuming its binary layout
    const CMatrix &m=*(CMatrix *)matrix;
    SpecialFXMatrix result={m.rx,m.ry,m.rz,m.fx,m.fy,m.fz,m.ux,m.uy,m.uz,m.px,m.py,m.pz};return result;
}
void * SpecialFX_BackendPlayerPed(void)
{
    // Preserve every original live player-ped query
    return FindPlayerPed();
}
void * SpecialFX_BackendWeapon(void *ped)
{
    // Read the actual player weapon-model field at its original use
    return ((CPlayerPed *)ped)->m_pWeaponModel;
}
void * SpecialFX_BackendAtomicFrame(void *atomic)
{
    // Read the original typed atomic frame
    return RpAtomicGetFrame((RpAtomic *)atomic);
}
void * SpecialFX_BackendFrameLTM(void *frame)
{
    // Enter the original typed frame local-to-world operation
    return RwFrameGetLTM((RwFrame *)frame);
}
void * SpecialFX_BackendModelInfo(void *object)
{
    // Read the original typed atomic model-info pointer
    return CVisibilityPlugins::GetAtomicModelInfo((RpAtomic *)object);
}
float SpecialFX_BackendScreenWidth(void)
{
    // Read the original live screen-width macro
    return SCREEN_WIDTH;
}
float SpecialFX_BackendScreenHeight(void)
{
    // Read the original live screen-height macro
    return SCREEN_HEIGHT;
}
char * SpecialFX_BackendString(void)
{
    // Borrow the actual shared ASCII buffer
    return gString;
}
uint16_t * SpecialFX_BackendUnicode(void)
{
    // Borrow the actual shared game Unicode buffer
    return gUString;
}
void SpecialFX_BackendAsciiToUnicode(const char *source,uint16_t *target)
{
    // Enter the original typed game text converter
    AsciiToUnicode(source,target);
}
void SpecialFX_BackendFontScale(float x,float y)
{
    // Set the original font scale with C-computed values
    CFont::SetScale(x,y);
}
void SpecialFX_BackendFontCentreSize(float value)
{
    // Retain the original nominally unused centre-size write
    CFont::SetCentreSize(value);
}
void SpecialFX_BackendFontColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Construct the original typed font color after original narrowing
    CFont::SetColor(CRGBA(r,g,b,a));
}
void SpecialFX_BackendFontStyle(int16_t style)
{
    // Set the original typed font style selected by C
    CFont::SetFontStyle(style);
}
bool SpecialFX_BackendJapanese(void)
{
    // Read the original locale predicate only when the original macro uses it
    #ifdef MORE_LANGUAGES
    return CFont::IsJapanese();
#else
    return false;
#endif
}
void SpecialFX_BackendPrintString(float x,float y,uint16_t *text)
{
    // Print through the exact original typed Unicode overload
    CFont::PrintString(x,y,text);
}
void SpecialFX_BackendPolygon(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Construct the original typed polygon color at its use point
    ([](float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,const CRGBA &color){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Draw2DPolygon(x1,y1,x2,y2,x3,y3,x4,y4,Sprite_ColourRef(&color.r,&color.g,&color.b,&color.a));
})(x1,y1,x2,y2,x3,y3,x4,y4,CRGBA(r,g,b,a));
}
void SpecialFX_BackendRect(float left,float top,float right,float bottom,uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Retain the exact original rectangle constructor argument order
    ([](const CRect &r,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawSolidRect(&r,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(Rect_Make(left,top,right,bottom),CRGBA(r,g,b,a));
}
void SpecialFX_BackendPush(const char *name)
{
    // Preserve the original debug label and FINAL configuration
    PUSH_RENDERGROUP(name);
}
void SpecialFX_BackendPop(void)
{
    // Close the original debug group
    POP_RENDERGROUP();
}
bool SpecialFX_BackendNewRenderer(void)
{
    // Read the original optional renderer flag at its short-circuit operand
    #ifdef NEW_RENDERER
    return gbNewRenderer;
#else
    return false;
#endif
}
bool SpecialFX_BackendFirstPerson(void)
{
    // Enter the original optional first-person camera query
    #ifdef NEW_RENDERER
    return FredIsInFirstPersonCam();
#else
    return false;
#endif
}
void SpecialFX_BackendFontJustifyOff(void)
{
    // Preserve the original typed font JustifyOff operation
    CFont::SetJustifyOff();
}
void SpecialFX_BackendFontBackgroundOff(void)
{
    // Preserve the original typed font BackgroundOff operation
    CFont::SetBackgroundOff();
}
void SpecialFX_BackendFontCentreOff(void)
{
    // Preserve the original typed font CentreOff operation
    CFont::SetCentreOff();
}
void SpecialFX_BackendFontPropOn(void)
{
    // Preserve the original typed font PropOn operation
    CFont::SetPropOn();
}
void SpecialFX_BackendSrcBlend(bool additive)
{
    // Set the original typed SrcBlend factor
    RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)(uintptr_t)(additive?rwBLENDONE:rwBLENDSRCALPHA));
}
void SpecialFX_BackendDestBlend(bool additive)
{
    // Set the original typed DestBlend factor
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)(uintptr_t)(additive?rwBLENDONE:rwBLENDINVSRCALPHA));
}

RwObject *LookForBatCB(RwObject *object,void *data)
{
    // Keep the original typed callback entry while C controls every decision and computation
    return (RwObject *)SpecialFX_LookForBat(object,data);
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
