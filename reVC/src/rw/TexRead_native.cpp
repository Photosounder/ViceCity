//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#define FORCE_PC_SCALING
#ifndef LIBRW
#define WITHD3D
#endif
#include "common.h"
#ifdef ANISOTROPIC_FILTERING
#include "rpanisot.h"
#endif
#include "crossplatform.h"
#include "platform.h"
#include "TexRead_backend.h"
#ifdef GTA_PC
#include "main.h"
#include "Pad.h"
#include "Streaming.h"
#include "Font.h"
#include "Sprite2d.h"
#include "RwHelper.h"
#include "Frontend.h"
#ifndef LIBRW
extern "C" RwInt32 _rwD3D8FindCorrectRasterFormat(RwRasterType type,RwInt32 flags);
extern "C" RwBool _rwD3D8CheckValidTextureFormat(RwInt32 format);
#endif
#endif
bool TexRead_HostReadNative(void *stream,void **texture,uint32_t size)
{
    // Keep the SDK output slot correctly typed and copy it only after a successful native read
    RwTexture *value;
#ifdef LIBRW
    bool result=rwNativeTextureHackRead((RwStream*)stream,&value,size)!=0;
#else
    bool result=RWSRCGLOBAL(stdFunc[rwSTANDARDNATIVETEXTUREREAD]((RwStream*)stream,&value,size))!=0;
#endif
    if(result)*texture=value;
    return result;
}
struct TexReadVisitorContext {TexReadTextureVisitor visitor;void *data;};
static RwTexture *TexRead_VisitTexture(RwTexture *texture,void *opaque)
{
    // Enter the C callback through the current synchronous enumeration context
    TexReadVisitorContext *context=(TexReadVisitorContext*)opaque;
    return (RwTexture*)context->visitor(texture,context->data);
}
void TexRead_HostForAllTextures(void *dictionary,TexReadTextureVisitor visitor,void *data)
{
    // Keep each callback context local so nested SDK enumerations retain their own visitor and data
    TexReadVisitorContext context={visitor,data};
    RwTexDictionaryForAllTextures((RwTexDictionary*)dictionary,TexRead_VisitTexture,&context);
}
bool TexRead_HostFindChunk(void *stream,bool native,uint32_t *size,uint32_t *version)
{
    // Preserve the original external primitive and its live borrowed arguments
    return RwStreamFindChunk((RwStream*)stream,native?rwID_TEXTURENATIVE:rwID_STRUCT,size,version)!=0;
}
bool TexRead_HostPlayingGame(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return gGameState==GS_INIT_PLAYING_GAME;
}
void TexRead_HostDestroyTexture(void *texture)
{
    // Preserve the original external primitive and its live borrowed arguments
    RwTextureDestroy((RwTexture*)texture);
}
void * TexRead_HostCreateDictionary(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return RwTexDictionaryCreate();
}
void TexRead_HostDestroyDictionary(void *dictionary)
{
    // Preserve the original external primitive and its live borrowed arguments
    RwTexDictionaryDestroy((RwTexDictionary*)dictionary);
}
void TexRead_HostAddTexture(void *dictionary,void *texture)
{
    // Preserve the original external primitive and its live borrowed arguments
    RwTexDictionaryAddTexture((RwTexDictionary*)dictionary,(RwTexture*)texture);
}
uint32_t TexRead_HostReadStream(void *stream,void *buffer,uint32_t size)
{
    // Preserve the original external primitive and its live borrowed arguments
    return RwStreamRead((RwStream*)stream,buffer,size);
}
void TexRead_HostSkipStream(void *stream,uint32_t offset)
{
    // Preserve the original external primitive and its live borrowed arguments
    RwStreamSkip((RwStream*)stream,offset);
}
TexReadStreamPosition TexRead_HostPosition(void *stream)
{
    // Preserve the original external primitive and its live borrowed arguments
    return STREAMPOS((RwStream*)stream);
}
#if defined(ANISOTROPIC_FILTERING)
int32_t TexRead_HostMaxAnisotropy(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return RpAnisotGetMaxSupportedMaxAnisotropy();
}
#endif
#if defined(ANISOTROPIC_FILTERING)
void TexRead_HostSetAnisotropy(void *texture,int32_t level)
{
    // Preserve the original external primitive and its live borrowed arguments
    RpAnisotTextureSetMaxAnisotropy((RwTexture*)texture,level);
}
#endif
#if defined(GTA_PC) && defined(LIBRW)
uint32_t TexRead_HostPlatform(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return rw::platform;
}
#endif
#if defined(GTA_PC) && defined(LIBRW) && defined(RW_GL3)
uint32_t TexRead_HostGLSubplatform(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return rw::gl3::gl3Caps.gles;
}
#endif
#if defined(GTA_PC) && defined(LIBRW) && defined(RW_GL3)
uint32_t TexRead_HostGLDXT(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return rw::gl3::gl3Caps.dxtSupported;
}
#endif
#if defined(GTA_PC) && !defined(LIBRW)
uint32_t TexRead_HostRaster8888(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return _rwD3D8FindCorrectRasterFormat(rwRASTERTYPETEXTURE,rwRASTERFORMAT8888);
}
#endif
#if defined(GTA_PC) && !defined(LIBRW)
uint32_t TexRead_HostRaster888(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return _rwD3D8FindCorrectRasterFormat(rwRASTERTYPETEXTURE,rwRASTERFORMAT888);
}
#endif
#if defined(GTA_PC) && !defined(LIBRW)
uint32_t TexRead_HostRaster1555(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return _rwD3D8FindCorrectRasterFormat(rwRASTERTYPETEXTURE,rwRASTERFORMAT1555);
}
#endif
#if defined(GTA_PC) && !defined(LIBRW)
uint32_t TexRead_HostRasterPAL8(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return _rwD3D8FindCorrectRasterFormat(rwRASTERTYPETEXTURE,rwRASTERFORMATPAL8|rwRASTERFORMAT8888);
}
#endif
#if defined(GTA_PC) && !defined(LIBRW)
bool TexRead_HostDXT1(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return _rwD3D8CheckValidTextureFormat(D3DFMT_DXT1)!=0;
}
#endif
#if defined(GTA_PC) && !defined(LIBRW)
bool TexRead_HostDXT3(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return _rwD3D8CheckValidTextureFormat(D3DFMT_DXT3)!=0;
}
#endif
#if defined(GTA_PC)
void TexRead_HostHandleExit(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    HandleExit();
}
#endif
#if defined(GTA_PC)
void * TexRead_HostLoadSplash(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return LoadSplash(NULL);
}
#endif
#if defined(GTA_PC)
bool TexRead_HostStartFrame(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return DoRWStuffStartOfFrame(0,0,0,0,0,0,255)!=0;
}
#endif
#if defined(GTA_PC)
void TexRead_HostRecipNearClip(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    Sprite2d_SetRecipNearClip();
}
#endif
#if defined(GTA_PC)
void TexRead_HostSpriteInit(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    Sprite2d_InitPerFrame();
}
#endif
#if defined(GTA_PC)
void TexRead_HostFontInit(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::InitPerFrame();
}
#endif
#if defined(GTA_PC)
void TexRead_HostDefinedState(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    DefinedState();
}
#endif
#if defined(GTA_PC)
void TexRead_HostAddressClamp(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS,(void*)rwTEXTUREADDRESSCLAMP);
}
#endif
#if defined(GTA_PC)
void TexRead_HostSplashDraw(void *splash,CRect rect,uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Preserve the original external primitive and its live borrowed arguments
    ([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRect(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(((Sprite2d*)splash),rect,CRGBA(r,g,b,a));
}
#endif
#if defined(GTA_PC)
void TexRead_HostDrawRect(CRect rect,uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Preserve the original external primitive and its live borrowed arguments
    ([](const CRect &r,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawSolidRect(&r,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(rect,CRGBA(r,g,b,a));
}
#endif
#if defined(GTA_PC)
void TexRead_HostFontBackgroundOff(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::SetBackgroundOff();
}
#endif
#if defined(GTA_PC)
void TexRead_HostFontPropOn(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::SetPropOn();
}
#endif
#if defined(GTA_PC)
void TexRead_HostFontScale(float x,float y)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::SetScale(x,y);
}
#endif
#if defined(GTA_PC)
void TexRead_HostFontCentreOff(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::SetCentreOff();
}
#endif
#if defined(GTA_PC)
void TexRead_HostFontWrap(float x)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::SetWrapx(x);
}
#endif
#if defined(GTA_PC)
void TexRead_HostFontJustifyOff(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::SetJustifyOff();
}
#endif
#if defined(GTA_PC)
void TexRead_HostFontColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::SetColor(CRGBA(r,g,b,a));
}
#endif
#if defined(GTA_PC)
void TexRead_HostFontBackgroundTextOff(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::SetBackGroundOnlyTextOff();
}
#endif
#if defined(GTA_PC)
void TexRead_HostFontStyle(int16_t style)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::SetFontStyle(style);
}
#endif
#if defined(GTA_PC)
void TexRead_HostFontPrint(float x,float y,uint16_t *text)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::PrintString(x,y,text);
}
#endif
#if defined(GTA_PC)
void TexRead_HostDrawFonts(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    CFont::DrawFonts();
}
#endif
#if defined(GTA_PC)
void TexRead_HostEndFrame(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    DoRWStuffEndOfFrame();
}
#endif
#if defined(GTA_PC)
float TexRead_HostScreenWidth(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return SCREEN_WIDTH;
}
#endif
#if defined(GTA_PC)
float TexRead_HostScreenHeight(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return SCREEN_HEIGHT;
}
#endif
#if defined(GTA_PC)
bool TexRead_HostQuit(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return RsGlobal.quit!=0;
}
#endif
#if defined(GTA_PC)
void TexRead_HostSetQuit(bool value)
{
    // Preserve the original external primitive and its live borrowed arguments
    RsGlobal.quit=value;
}
#endif
#if defined(GTA_PC)
void TexRead_HostUpdatePads(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    CPad::UpdatePads();
}
#endif
#if defined(GTA_PC)
bool TexRead_HostEscape(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return Input_GetEscapeJustDown()!=0; // rouz edit (ChatGPT)
}
#endif
#if defined(GTA_PC)
void TexRead_HostLoadingScreen(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    LoadingScreen(NULL,NULL,NULL);
}
#endif
#if defined(GTA_PC)
void TexRead_HostFlushRequests(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    Streaming_FlushRequestList(); // rouz edit (ChatGPT)
}
#endif
#if defined(GTA_PC)
void * TexRead_HostOpenImage(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return RwStreamOpen(rwSTREAMFILENAME,rwSTREAMWRITE,"models\\txd.img");
}
#endif
#if defined(GTA_PC)
void TexRead_HostCloseStream(void *stream)
{
    // Preserve the original external primitive and its live borrowed arguments
    RwStreamClose((RwStream*)stream,NULL);
}
#endif
#if defined(GTA_PC)
int64_t TexRead_HostOSVersion(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return _dwOperatingSystemVersion;
}
#endif
#if defined(GTA_PC) && defined(RW_GL3)
void TexRead_HostReadBack(bool value)
{
    // Preserve the original external primitive and its live borrowed arguments
    rw::gl3::needToReadBackTextures=value;
}
#endif
#if defined(GTA_PC) && defined(DISABLE_VSYNC_ON_TEXTURE_CONVERSION)
int8_t TexRead_HostVsync(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return FrontEndMenuManager.m_PrefsVsync;
}
#endif
#if defined(GTA_PC) && defined(DISABLE_VSYNC_ON_TEXTURE_CONVERSION)
int8_t TexRead_HostFrameLimiter(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return FrontEndMenuManager.m_PrefsFrameLimiter;
}
#endif
#if defined(GTA_PC) && defined(DISABLE_VSYNC_ON_TEXTURE_CONVERSION)
void TexRead_HostSetVsync(int8_t value)
{
    // Preserve the original external primitive and its live borrowed arguments
    FrontEndMenuManager.m_PrefsVsync=value;
}
#endif
#if defined(GTA_PC) && defined(DISABLE_VSYNC_ON_TEXTURE_CONVERSION)
void TexRead_HostSetFrameLimiter(int8_t value)
{
    // Preserve the original external primitive and its live borrowed arguments
    FrontEndMenuManager.m_PrefsFrameLimiter=value;
}
#endif
#if defined(GTA_PC)
bool TexRead_HostInImage(int32_t id)
{
    // Preserve the original external primitive and its live borrowed arguments
    return Streaming_IsObjectInCdImage(id); // rouz edit (ChatGPT)
}
#endif
#if defined(GTA_PC)
void TexRead_HostRequestTxd(int32_t slot,int32_t flags)
{
    // Preserve the original external primitive and its live borrowed arguments
    Streaming_RequestTxd(slot,flags); // rouz edit (ChatGPT)
}
#endif
#if defined(GTA_PC)
void TexRead_HostRequestModelStream(int32_t channel)
{
    // Preserve the original external primitive and its live borrowed arguments
    CStreaming::RequestModelStream(channel);
}
#endif
#if defined(GTA_PC)
void TexRead_HostFlushChannels(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    Streaming_FlushChannels(); // rouz edit (ChatGPT)
}
#endif
#if defined(GTA_PC) && defined(LIBRW)
int32_t TexRead_HostTell(void *stream)
{
    // Preserve the original external primitive and its live borrowed arguments
    return ((RwStream*)stream)->tell();
}
#endif
#if defined(GTA_PC) && !defined(LIBRW)
void * TexRead_HostFileInterface(void)
{
    // Preserve the original external primitive and its live borrowed arguments
    return RwOsGetFileInterface();
}
#endif
#if defined(GTA_PC) && !defined(LIBRW)
int32_t TexRead_HostLegacyTell(void *filesys,void *stream)
{
    // Preserve the original external primitive and its live borrowed arguments
    return ((RwFileFunctions*)filesys)->rwftell(((RwStream*)stream)->Type.file.fpFile);
}
#endif
#if defined(GTA_PC)
const void * TexRead_HostWriteDictionary(void *dictionary,void *stream)
{
    // Preserve the original external primitive and its live borrowed arguments
    return RwTexDictionaryStreamWrite((RwTexDictionary*)dictionary,(RwStream*)stream);
}
#endif
#if defined(GTA_PC)
void TexRead_HostWriteStream(void *stream,const void *buffer,uint32_t size)
{
    // Preserve the original external primitive and its live borrowed arguments
    RwStreamWrite((RwStream*)stream,buffer,size);
}
#endif
#if defined(GTA_PC)
void TexRead_HostRemoveTxd(int32_t slot)
{
    // Preserve the original external primitive and its live borrowed arguments
    Streaming_RemoveTxd(slot); // rouz edit (ChatGPT)
}
#endif
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
