//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "PlayerSkin_backend.h"
#include "main.h"
#include "TxdStore.h"
#include "rtbmp.h"
#include "ClumpModelInfo.h"
#include "VisibilityPlugins.h"
#include "World.h"
#include "PlayerInfo.h"
#include "CdStream.h"
#include "RwHelper.h"
#include "Lights.h"
#include "MemoryMgr_native.h"
int PlayerSkin_BackendCompare(const char *a,const char *b)
{
    // Retain the original typed native operation and its borrowed owner arguments
    return strcasecmp(a,b);
}
int32_t PlayerSkin_BackendImageCount(void)
{
    // Retain the original typed native operation and its borrowed owner arguments
    return CdStreamGetNumImages();
}
void PlayerSkin_BackendAddImage(const char *path)
{
    // Retain the original typed native operation and its borrowed owner arguments
    CdStreamAddImage(path);
}
void PlayerSkin_BackendReadSectors(uint8_t *buffer,uint32_t offset,uint32_t size)
{
    // Retain the original typed native operation and its borrowed owner arguments
    CdStreamRead(0,buffer,offset,size);
}
void PlayerSkin_BackendSync(void)
{
    // Retain the original typed native operation and its borrowed owner arguments
    CdStreamSync(0);
}
void PlayerSkin_BackendRemoveImages(void)
{
    // Retain the original typed native operation and its borrowed owner arguments
    CdStreamRemoveImages();
}
uint8_t * PlayerSkin_BackendAllocate(uint32_t size,uint32_t alignment)
{
    // Retain the original typed native operation and its borrowed owner arguments
    return (uint8_t*)RwMallocAlign(size,alignment);
}
void PlayerSkin_BackendFree(uint8_t *buffer)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RwFreeAlign(buffer);
}
void * PlayerSkin_BackendOpenStream(void *memory)
{
    // Retain the original typed native operation and its borrowed owner arguments
    return RwStreamOpen(rwSTREAMMEMORY,rwSTREAMREAD,memory);
}
bool PlayerSkin_BackendFindChunk(void *stream)
{
    // Retain the original typed native operation and its borrowed owner arguments
    return RwStreamFindChunk((RwStream*)stream,rwID_CLUMP,NULL,NULL)!=0;
}
PlayerSkinClump * PlayerSkin_BackendReadClump(void *stream)
{
    // Retain the original typed native operation and its borrowed owner arguments
    return RpClumpStreamRead((RwStream*)stream);
}
void PlayerSkin_BackendCloseStream(void *stream,void *memory)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RwStreamClose((RwStream*)stream,memory);
}







void * PlayerSkin_BackendReadImage(const char *path)
{
    // Retain the original typed native operation and its borrowed owner arguments
    return RtBMPImageRead(path);
}
void PlayerSkin_BackendRasterFormat(void *image,int32_t *width,int32_t *height,int32_t *depth,int32_t *format)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RwImageFindRasterFormat((RwImage*)image,rwRASTERTYPETEXTURE,width,height,depth,format);
}
void * PlayerSkin_BackendCreateRaster(int32_t width,int32_t height,int32_t depth,int32_t format)
{
    // Retain the original typed native operation and its borrowed owner arguments
    return RwRasterCreate(width,height,depth,format);
}
void PlayerSkin_BackendRasterImage(void *raster,void *image)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RwRasterSetFromImage((RwRaster*)raster,(RwImage*)image);
}
PlayerSkinTexture * PlayerSkin_BackendCreateTexture(void *raster)
{
    // Retain the original typed native operation and its borrowed owner arguments
    return RwTextureCreate((RwRaster*)raster);
}
void PlayerSkin_BackendTextureName(PlayerSkinTexture *texture,const char *name)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RwTextureSetName(texture,name);
}
void PlayerSkin_BackendTextureFilter(PlayerSkinTexture *texture)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RwTextureSetFilterMode(texture,rwFILTERLINEAR);
}
void PlayerSkin_BackendAddTexture(int32_t slot,PlayerSkinTexture *texture)
{
    // Retain the original typed native operation and its borrowed owner arguments
    // Use the complete C texture dictionary owner with the original live arguments
    RwTexDictionaryAddTexture(((RwTexDictionary*)TxdStore_GetSlot(slot)->texDict),texture);
}
void PlayerSkin_BackendDestroyImage(void *image)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RwImageDestroy((RwImage*)image);
}
void PlayerSkin_BackendSetRenderers(PlayerSkinClump *clump)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RpClumpForAllAtomics(clump,CClumpModelInfo::SetAtomicRendererCB,(void*)CVisibilityPlugins::RenderPlayerCB);
}
void PlayerSkin_BackendLoadPlayer(void)
{
    // Retain the original typed native operation and its borrowed owner arguments
    CWorld::Players[0].LoadPlayerSkin();
}
void PlayerSkin_BackendDestroyClump(PlayerSkinClump *clump)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RpClumpDestroy(clump);
}
void * PlayerSkin_BackendFrame(PlayerSkinClump *clump)
{
    // Retain the original typed native operation and its borrowed owner arguments
    return RpClumpGetFrame(clump);
}
void PlayerSkin_BackendTransform(void *frame)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RwFrameTransform((RwFrame*)frame,RwFrameGetMatrix(RwCameraGetFrame(Scene.camera)),rwCOMBINEREPLACE);
}
void PlayerSkin_BackendTranslate(void *frame,PlayerSkinVector value)
{
    // Retain the original typed native operation and its borrowed owner arguments
    const RwV3d position={value.x,value.y,value.z};RwFrameTranslate((RwFrame*)frame,&position,rwCOMBINEPRECONCAT);
}
void PlayerSkin_BackendRotate(void *frame,PlayerSkinVector value,float angle)
{
    // Retain the original typed native operation and its borrowed owner arguments
    const RwV3d axis={value.x,value.y,value.z};RwFrameRotate((RwFrame*)frame,&axis,angle,rwCOMBINEPRECONCAT);
}
void PlayerSkin_BackendUpdateObjects(void *frame)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RwFrameUpdateObjects((RwFrame*)frame);
}
void PlayerSkin_BackendAmbient(PlayerSkinColor value)
{
    // Retain the original typed native operation and its borrowed owner arguments
    // Use the complete C lighting owner and its borrowed SDK handles
    LightsColour color={value.red,value.green,value.blue,value.alpha};Lights_SetAmbientColour(&color);
}
void PlayerSkin_BackendRenderClump(PlayerSkinClump *clump)
{
    // Retain the original typed native operation and its borrowed owner arguments
    RpClumpRender(clump);
}
void PlayerSkin_BackendWithMemory(uint8_t *buffer,uint32_t length,PlayerSkinMemoryCallback callback)
{
    // Keep the same real native memory object alive through stream open, chunk loading and stream close
    RwMemory memory;memory.start=buffer;memory.length=length;callback(&memory);
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
