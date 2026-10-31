//+ rouz edit (ChatGPT)
#define WITHD3D
#include "common.h"
#include <rpskin.h>
#include "skeleton.h"
#include "RwHelper_backend.h"
#if !defined(FINAL) || defined(DEBUGMENU)
#include "rtcharse.h"
#endif
#ifdef LIBRW
#include <rpmatfx.h>
#include "VehicleModelInfo.h"
#endif
#if !defined(FINAL) || defined(DEBUGMENU)
void RwHelper_HostCharsetOpen(void)
{
    // Forward one original SDK operation or access its actual typed metadata
    RtCharsetOpen();
}
#endif
#if !defined(FINAL) || defined(DEBUGMENU)
void RwHelper_HostCharsetClose(void)
{
    // Forward one original SDK operation or access its actual typed metadata
    RtCharsetClose();
}
#endif
#if !defined(FINAL) || defined(DEBUGMENU)
void RwHelper_HostCharsetDestroy(void *charset)
{
    // Forward one original SDK operation or access its actual typed metadata
    RtCharsetDestroy((RtCharset*)charset);
}
#endif
#if !defined(FINAL) || defined(DEBUGMENU)
void RwHelper_HostCharsetPrint(void *charset,const char *text,int32_t x,int32_t y,bool hide)
{
    // Forward one original SDK operation or access its actual typed metadata
    RtCharsetPrintBuffered((RtCharset*)charset,text,x,y,hide);
}
#endif
#if !defined(FINAL) || defined(DEBUGMENU)
void RwHelper_HostCharsetFlush(void)
{
    // Forward one original SDK operation or access its actual typed metadata
    RtCharsetBufferFlush();
}
#endif
void * RwHelper_HostAtomicGeometry(void *atomic)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RpAtomicGetGeometry((RpAtomic*)atomic);
}
void * RwHelper_HostGeometrySkin(void *geometry)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RpSkinGeometryGetSkin((RpGeometry*)geometry);
}
void * RwHelper_HostAtomicHierarchy(void *atomic)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RpSkinAtomicGetHAnimHierarchy((RpAtomic*)atomic);
}
void * RwHelper_HostFrameHierarchy(void *frame)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RpHAnimFrameGetHierarchy((RwFrame*)frame);
}
void * RwHelper_HostClumpFrame(void *clump)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RpClumpGetFrame((RpClump*)clump);
}
int32_t RwHelper_HostNumBones(void *skin)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RpSkinGetNumBones((RpSkin*)skin);
}
const void * RwHelper_HostSkinMatrices(void *skin)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RpSkinGetSkinToBoneMatrices((RpSkin*)skin);
}
const void * RwHelper_HostMatrixAt(const void *matrices,int32_t index)
{
    // Forward one original SDK operation or access its actual typed metadata
    return &((const RwMatrix*)matrices)[index];
}
void * RwHelper_HostVectorAt(void *vectors,int32_t index)
{
    // Forward one original SDK operation or access its actual typed metadata
    return &((RwV3d*)vectors)[index];
}
const void * RwHelper_HostMatrixPosition(const void *matrix)
{
    // Forward one original SDK operation or access its actual typed metadata
    return &((const RwMatrix*)matrix)->pos;
}
void RwHelper_HostCopyMatrix(void *destination,const void *source)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwMatrixCopy((RwMatrix*)destination,(const RwMatrix*)source);
}
void RwHelper_HostInvertMatrix(void *destination,const void *source)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwMatrixInvert((RwMatrix*)destination,(const RwMatrix*)source);
}
void RwHelper_HostTransformPoints(void *destination,const void *source,int32_t count,const void *matrix)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwV3dTransformPoints((RwV3d*)destination,(const RwV3d*)source,count,(const RwMatrix*)matrix);
}
uint32_t RwHelper_HostNodeFlags(void *hierarchy,int32_t index)
{
    // Forward one original SDK operation or access its actual typed metadata
    return HIERNODEINFO((RpHAnimHierarchy*)hierarchy)[index].flags;
}
int32_t RwHelper_HostNumNodes(void *hierarchy)
{
    // Forward one original SDK operation or access its actual typed metadata
    return ((RpHAnimHierarchy*)hierarchy)->numNodes;
}
void * RwHelper_HostCreateAnimation(int32_t type,int32_t count,int32_t flags,float duration)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RpHAnimAnimationCreate(type,count,flags,duration);
}
void * RwHelper_HostAnimationFrame(void *animation,int32_t index)
{
    // Forward one original SDK operation or access its actual typed metadata
    return HANIMFRAME((RpHAnimAnimation*)animation,index);
}
void RwHelper_HostPreviousFrame(void *frame,void *previous)
{
    // Forward one original SDK operation or access its actual typed metadata
    ((RpHAnimStdKeyFrame*)frame)->prevFrame=(RpHAnimStdKeyFrame*)previous;
}
void * RwHelper_HostHierarchyMatrices(void *hierarchy)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RpHAnimHierarchyGetMatrixArray((RpHAnimHierarchy*)hierarchy);
}
void * RwHelper_HostAtomicSphere(void *atomic)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RpAtomicGetBoundingSphere((RpAtomic*)atomic);
}
void * RwHelper_HostAtomicClump(void *atomic)
{
    // Forward one original SDK operation or access its actual typed metadata
    return ((RpAtomic*)atomic)->clump;
}
void * RwHelper_HostFrameLTM(void *frame)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwFrameGetLTM((RwFrame*)frame);
}
void * RwHelper_HostFrameMatrix(void *frame)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwFrameGetMatrix((RwFrame*)frame);
}
void * RwHelper_HostSphereCenter(void *sphere)
{
    // Forward one original SDK operation or access its actual typed metadata
    return &((RwSphere*)sphere)->center;
}
float * RwHelper_HostSphereRadius(void *sphere)
{
    // Forward one original SDK operation or access its actual typed metadata
    return &((RwSphere*)sphere)->radius;
}
void RwHelper_HostVectorAdd(void *destination,void *a,void *b)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwV3dAdd((RwV3d*)destination,(RwV3d*)a,(RwV3d*)b);
}
void RwHelper_HostVectorSub(void *destination,void *a,void *b)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwV3dSub((RwV3d*)destination,(RwV3d*)a,(RwV3d*)b);
}
void RwHelper_HostVectorScale(void *destination,void *source,float scale)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwV3dScale((RwV3d*)destination,(RwV3d*)source,scale);
}
float RwHelper_HostVectorLength(void *vector)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwV3dLength((RwV3d*)vector);
}
int32_t RwHelper_HostNumAtomics(void *clump)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RpClumpGetNumAtomics((RpClump*)clump);
}
void RwHelper_HostCopySphere(void *destination,void *source)
{
    // Forward one original SDK operation or access its actual typed metadata
    *(RwSphere*)destination=*(RwSphere*)source;
}
void * RwHelper_HostCameraRaster(void *camera)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwCameraGetRaster((RwCamera*)camera);
}
void * RwHelper_HostCameraZRaster(void *camera)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwCameraGetZRaster((RwCamera*)camera);
}
int32_t RwHelper_HostRasterWidth(void *raster)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwRasterGetWidth((RwRaster*)raster);
}
int32_t RwHelper_HostRasterHeight(void *raster)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwRasterGetHeight((RwRaster*)raster);
}
void RwHelper_HostDestroyRaster(void *raster)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwRasterDestroy((RwRaster*)raster);
}
void RwHelper_HostStoreCameraRaster(void *camera,void *raster)
{
    // Forward one original SDK operation or access its actual typed metadata
    ((RwCamera*)camera)->frameBuffer=(RwRaster*)raster;
}
void RwHelper_HostStoreCameraZRaster(void *camera,void *raster)
{
    // Forward one original SDK operation or access its actual typed metadata
    ((RwCamera*)camera)->zBuffer=(RwRaster*)raster;
}
void * RwHelper_HostCreateRaster(int32_t width,int32_t height,int32_t depth,int32_t type)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwRasterCreate(width,height,depth,type);
}
void RwHelper_HostSetCameraRaster(void *camera,void *raster)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwCameraSetRaster((RwCamera*)camera,(RwRaster*)raster);
}
void RwHelper_HostSetCameraZRaster(void *camera,void *raster)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwCameraSetZRaster((RwCamera*)camera,(RwRaster*)raster);
}
int32_t * RwHelper_HostRasterWidthSlot(void *raster)
{
    // Forward one original SDK operation or access its actual typed metadata
    return &((RwRaster*)raster)->width;
}
int32_t * RwHelper_HostRasterHeightSlot(void *raster)
{
    // Forward one original SDK operation or access its actual typed metadata
    return &((RwRaster*)raster)->height;
}
void RwHelper_HostScreenWidth(int32_t width)
{
    // Forward one original SDK operation or access its actual typed metadata
    RsGlobal.width=width;
}
void RwHelper_HostScreenHeight(int32_t height)
{
    // Forward one original SDK operation or access its actual typed metadata
    RsGlobal.height=height;
}
void * RwHelper_HostCameraFrame(void *camera)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwCameraGetFrame((RwCamera*)camera);
}
void RwHelper_HostDestroyFrame(void *frame)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwFrameDestroy((RwFrame*)frame);
}
void * RwHelper_HostRasterParent(void *raster)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwRasterGetParent((RwRaster*)raster);
}
void RwHelper_HostDestroyCamera(void *camera)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwCameraDestroy((RwCamera*)camera);
}
void * RwHelper_HostCreateCamera(void)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwCameraCreate();
}
void * RwHelper_HostCreateFrame(void)
{
    // Forward one original SDK operation or access its actual typed metadata
    return RwFrameCreate();
}
void RwHelper_HostSetCameraFrame(void *camera,void *frame)
{
    // Forward one original SDK operation or access its actual typed metadata
    RwCameraSetFrame((RwCamera*)camera,(RwFrame*)frame);
}
#ifdef LIBRW
void * RwHelper_HostAtomicGeometryField(void *atomic)
{
    // Forward one original SDK operation or access its actual typed metadata
    return ((RpAtomic*)atomic)->geometry;
}
void * RwHelper_HostInstanceData(void *geometry)
{
    // Forward one original SDK operation or access its actual typed metadata
    return ((rw::Geometry*)geometry)->instData;
}
int32_t RwHelper_HostInstancePlatform(void *instance)
{
    // Forward one original SDK operation or access its actual typed metadata
    return ((rw::InstanceDataHeader*)instance)->platform;
}
void * RwHelper_HostPipeline(void *atomic)
{
    // Forward one original SDK operation or access its actual typed metadata
    return ((RpAtomic*)atomic)->pipeline;
}
void RwHelper_HostSetPipeline(void *atomic,void *pipeline)
{
    // Forward one original SDK operation or access its actual typed metadata
    ((RpAtomic*)atomic)->pipeline=(rw::ObjPipeline*)pipeline;
}
int32_t RwHelper_HostPipelinePlatform(void *pipeline)
{
    // Forward one original SDK operation or access its actual typed metadata
    return ((rw::ObjPipeline*)pipeline)->platform;
}
uint32_t RwHelper_HostPipelinePlugin(void *pipeline)
{
    // Forward one original SDK operation or access its actual typed metadata
    return ((rw::ObjPipeline*)pipeline)->pluginID;
}
void * RwHelper_HostSkinPipeline(int32_t platform)
{
    // Forward one original SDK operation or access its actual typed metadata
    return rw::skinGlobals.pipelines[platform];
}
void * RwHelper_HostMatFXPipeline(int32_t platform)
{
    // Forward one original SDK operation or access its actual typed metadata
    return rw::matFXGlobals.pipelines[platform];
}
int32_t RwHelper_HostPlatform(void)
{
    // Forward one original SDK operation or access its actual typed metadata
    return rw::platform;
}
void RwHelper_HostStorePlatform(int32_t platform)
{
    // Forward one original SDK operation or access its actual typed metadata
    rw::platform=platform;
}
void RwHelper_HostMaterialEffects(void *atomic)
{
    // Forward one original SDK operation or access its actual typed metadata
    RpMatFXAtomicEnableEffects((RpAtomic*)atomic);
}
void RwHelper_HostForMaterials(void *geometry,int32_t *effects)
{
    // Forward one original SDK operation or access its actual typed metadata
    RpGeometryForAllMaterials((RpGeometry*)geometry,CVehicleModelInfo::GetMatFXEffectMaterialCB,effects);
}
uint32_t RwHelper_HostGeometryFlags(void *geometry)
{
    // Forward one original SDK operation or access its actual typed metadata
    return ((rw::Geometry*)geometry)->flags;
}
void RwHelper_HostUninstance(void *atomic)
{
    // Forward one original SDK operation or access its actual typed metadata
    ((RpAtomic*)atomic)->uninstance();
}
void RwHelper_HostLibSetState(uint32_t state,uint32_t value)
{
    // Forward one original SDK operation or access its actual typed metadata
    rw::SetRenderState((rw::RenderState)state,value);
}
uint32_t RwHelper_HostLibGetState(uint32_t state)
{
    // Forward one original SDK operation or access its actual typed metadata
    return rw::GetRenderState((rw::RenderState)state);
}
#endif
struct RwHelperVisitContext {RwHelperVisit callback;void *data;};
static RwFrame *RwHelper_VisitChildren(RwFrame *object,void *data)
{
    // Return the C callback result through the exact SDK callback signature
    RwHelperVisitContext *scope=(RwHelperVisitContext*)data;return (RwFrame*)scope->callback(object,scope->data);
}
void RwHelper_HostChildren(void *parent,RwHelperVisit callback,void *data)
{
    // Preserve independent borrowed callback state for synchronous traversal and reentry
    RwHelperVisitContext scope={callback,data};RwFrameForAllChildren((RwFrame*)parent,RwHelper_VisitChildren,&scope);
}
static RwObject *RwHelper_VisitObjects(RwObject *object,void *data)
{
    // Return the C callback result through the exact SDK callback signature
    RwHelperVisitContext *scope=(RwHelperVisitContext*)data;return (RwObject*)scope->callback(object,scope->data);
}
void RwHelper_HostObjects(void *parent,RwHelperVisit callback,void *data)
{
    // Preserve independent borrowed callback state for synchronous traversal and reentry
    RwHelperVisitContext scope={callback,data};RwFrameForAllObjects((RwFrame*)parent,RwHelper_VisitObjects,&scope);
}
static RpAtomic *RwHelper_VisitAtomics(RpAtomic *object,void *data)
{
    // Return the C callback result through the exact SDK callback signature
    RwHelperVisitContext *scope=(RwHelperVisitContext*)data;return (RpAtomic*)scope->callback(object,scope->data);
}
void RwHelper_HostAtomics(void *parent,RwHelperVisit callback,void *data)
{
    // Preserve independent borrowed callback state for synchronous traversal and reentry
    RwHelperVisitContext scope={callback,data};RpClumpForAllAtomics((RpClump*)parent,RwHelper_VisitAtomics,&scope);
}
static RwTexture *RwHelper_VisitTextures(RwTexture *object,void *data)
{
    // Return the C callback result through the exact SDK callback signature
    RwHelperVisitContext *scope=(RwHelperVisitContext*)data;return (RwTexture*)scope->callback(object,scope->data);
}
void RwHelper_HostTextures(void *parent,RwHelperVisit callback,void *data)
{
    // Preserve independent borrowed callback state for synchronous traversal and reentry
    RwHelperVisitContext scope={callback,data};RwTexDictionaryForAllTextures((RwTexDictionary*)parent,RwHelper_VisitTextures,&scope);
}
void RwHelper_HostBoneMatrices(RwHelperPairScope callback,void *data)
{
    // Borrow distinct native stack objects for the complete synchronous C operation
    RwMatrix first;RwMatrix second;callback(data,&first,&second);
}
void RwHelper_HostAtomicVectors(RwHelperPairScope callback,void *data)
{
    // Borrow distinct native stack objects for the complete synchronous C operation
    RwV3d first;RwV3d second;callback(data,&first,&second);
}
void RwHelper_HostBoundingSphere(RwHelperPairScope callback,void *data)
{
    // Borrow distinct native stack objects for the complete synchronous C operation
    RwMatrix first;RwSphere second;callback(data,&first,&second);
}
void RwHelper_HostQuadVertices(RwHelperVertexScope callback,void *data)
{
    // Preserve the original four uninitialized SDK vertices and their per-invocation lifetime
    RwIm2DVertex vertices[4];callback(data,vertices);
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
float * RwHelper_HostVectorField(void *vector,int32_t index)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    RwV3d *v=(RwV3d*)vector;float *fields[]={&v->x,&v->y,&v->z};return fields[index];
}
Debug_Vector RwHelper_HostVectorValue(const void *vector)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    const RwV3d *v=(const RwV3d*)vector;Debug_Vector value={v->x,v->y,v->z};return value;
}
float * RwHelper_HostKeyFrameField(void *frame,int32_t index)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    RpHAnimStdKeyFrame *f=(RpHAnimStdKeyFrame*)frame;float *fields[]={&f->q.real,&f->q.imag.x,&f->q.imag.y,&f->q.imag.z,&f->t.x,&f->t.y,&f->t.z,&f->time};return fields[index];
}
void RwHelper_HostStoreSphere(void *sphere,float x,float y,float z,float radius)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    RwSphere *s=(RwSphere*)sphere;s->center.x=x;s->center.y=y;s->center.z=z;s->radius=radius;
}
RwHelperRectRef RwHelper_HostRect(void *rectangle)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    RwRect *r=(RwRect*)rectangle;RwHelperRectRef value={NULL,NULL,NULL,NULL};if(r){value.x=&r->x;value.y=&r->y;value.w=&r->w;value.h=&r->h;}return value;
}
int32_t RwHelper_HostCurrentVideoMode(void)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    return RwEngineGetCurrentVideoMode();
}
void RwHelper_HostVideoMode(RwHelperVideoMode *mode,int32_t index)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    RwVideoMode native;RwEngineGetVideoModeInfo(&native,index);mode->width=native.width;mode->height=native.height;mode->flags=native.flags;
}
void RwHelper_HostViewWindow(void *camera,const RwHelperVector2 *window)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    RwV2d native={window->x,window->y};RwCameraSetViewWindow((RwCamera*)camera,&native);
}
#if !defined(FINAL) || defined(DEBUGMENU)
void * RwHelper_HostCharsetCreate(const RwHelperColour *foreground,const RwHelperColour *background)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    RwRGBA fg={foreground->red,foreground->green,foreground->blue,foreground->alpha};RwRGBA bg={background->red,background->green,background->blue,background->alpha};return RtCharsetCreate(&fg,&bg);
}
#endif
#if defined(RW_D3D9)
void * RwHelper_HostWideBuffer(void)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    static_assert(sizeof(WCHAR)==sizeof(uint16_t),"Preserve original wide storage element size");static WCHAR buffer[256];return buffer;
}
#endif
#if defined(RW_D3D9)
void RwHelper_HostToWide(const char *name,void *buffer,uint32_t count)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,name,-1,(WCHAR*)buffer,count);
}
#endif
#if defined(RW_D3D9)
void RwHelper_HostPerfBegin(uint32_t colour,void *buffer)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    D3DPERF_BeginEvent(colour,(WCHAR*)buffer);
}
#endif
#if defined(RW_D3D9)
void RwHelper_HostPerfEnd(void)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    D3DPERF_EndEvent();
}
#endif
#if defined(RW_OPENGL)
bool RwHelper_HostGLDebugAvailable(void)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    return GLAD_GL_KHR_debug!=0;
}
#endif
#if defined(RW_OPENGL)
void RwHelper_HostGLDebugPush(const char *name)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION,0,-1,name);
}
#endif
#if defined(RW_OPENGL)
void RwHelper_HostGLDebugPop(void)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    glPopDebugGroup();
}
#endif
void * RwHelper_HostVertexAt(void *vertices,int32_t index)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    return &((RwIm2DVertex*)vertices)[index];
}
void RwHelper_HostDrawStrip(void *vertices,int32_t count)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP,(RwIm2DVertex*)vertices,count);
}
void RwHelper_HostRenderState(uint32_t state,void *value)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    RwRenderStateSet((RwRenderState)state,value);
}
#if !defined(LIBRW)
void RwHelper_HostD3DSetState(uint32_t state,uint32_t value)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    RwD3D8SetRenderState(state,value);
}
#endif
#if !defined(LIBRW)
void RwHelper_HostD3DGetState(uint32_t state,uint32_t *value)
{
    // Access the actual SDK primitive or typed record at the C-selected point
    RwD3D8GetRenderState(state,value);
}
#endif
void RwHelper_HostVertexScreenX(void *vertex,float value)
{
    // Forward the original SDK vertex setter and its exact C arguments
    RwIm2DVertexSetScreenX((RwIm2DVertex*)vertex,value);
}
void RwHelper_HostVertexScreenY(void *vertex,float value)
{
    // Forward the original SDK vertex setter and its exact C arguments
    RwIm2DVertexSetScreenY((RwIm2DVertex*)vertex,value);
}
void RwHelper_HostVertexScreenZ(void *vertex,float value)
{
    // Forward the original SDK vertex setter and its exact C arguments
    RwIm2DVertexSetScreenZ((RwIm2DVertex*)vertex,value);
}
void RwHelper_HostVertexCameraZ(void *vertex,float value)
{
    // Forward the original SDK vertex setter and its exact C arguments
    RwIm2DVertexSetCameraZ((RwIm2DVertex*)vertex,value);
}
void RwHelper_HostVertexRecipCameraZ(void *vertex,float value)
{
    // Forward the original SDK vertex setter and its exact C arguments
    RwIm2DVertexSetRecipCameraZ((RwIm2DVertex*)vertex,value);
}
void RwHelper_HostVertexU(void *vertex,float value,float reciprocal)
{
    // Forward the original SDK vertex setter and its exact C arguments
    RwIm2DVertexSetU((RwIm2DVertex*)vertex,value,reciprocal);
}
void RwHelper_HostVertexV(void *vertex,float value,float reciprocal)
{
    // Forward the original SDK vertex setter and its exact C arguments
    RwIm2DVertexSetV((RwIm2DVertex*)vertex,value,reciprocal);
}
void RwHelper_HostVertexIntRGBA(void *vertex,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha)
{
    // Forward the original SDK vertex setter and its exact C arguments
    RwIm2DVertexSetIntRGBA((RwIm2DVertex*)vertex,red,green,blue,alpha);
}
uint32_t RwHelper_HostConstant(int32_t identity)
{
    // Translate SDK constant identities without embedding backend enum values in C policy
    switch(identity){
#if !defined(LIBRW)
case RWH_CONST_D3DCMP_GREATER:return D3DCMP_GREATER;
#endif
#if !defined(LIBRW)
case RWH_CONST_D3DCMP_GREATEREQUAL:return D3DCMP_GREATEREQUAL;
#endif
#if !defined(LIBRW)
case RWH_CONST_D3DRS_ALPHAFUNC:return D3DRS_ALPHAFUNC;
#endif
#if !defined(LIBRW)
case RWH_CONST_D3DRS_ALPHAREF:return D3DRS_ALPHAREF;
#endif
#if defined(LIBRW)
case RWH_CONST_LIB_ALPHAGREATEREQUAL:return rw::ALPHAGREATEREQUAL;
#endif
#if defined(LIBRW)
case RWH_CONST_LIB_ALPHATESTFUNC:return rw::ALPHATESTFUNC;
#endif
#if defined(LIBRW)
case RWH_CONST_LIB_ALPHATESTREF:return rw::ALPHATESTREF;
#endif
#if defined(LIBRW)
case RWH_CONST_LIB_GSALPHATEST:return rw::GSALPHATEST;
#endif
#if defined(LIBRW)
case RWH_CONST_LIB_Geometry_NATIVE:return rw::Geometry::NATIVE;
#endif
#if defined(LIBRW)
case RWH_CONST_LIB_ID_MATFX:return rw::ID_MATFX;
#endif
#if defined(LIBRW)
case RWH_CONST_LIB_ID_SKIN:return rw::ID_SKIN;
#endif
case RWH_CONST_rpHANIMPOPPARENTMATRIX:return rpHANIMPOPPARENTMATRIX;
case RWH_CONST_rpHANIMPUSHPARENTMATRIX:return rpHANIMPUSHPARENTMATRIX;
case RWH_CONST_rpHANIMSTDKEYFRAMETYPEID:return rpHANIMSTDKEYFRAMETYPEID;
#if defined(LIBRW)
case RWH_CONST_rpMATFXEFFECTNULL:return rpMATFXEFFECTNULL;
#endif
case RWH_CONST_rwBLENDINVSRCALPHA:return rwBLENDINVSRCALPHA;
case RWH_CONST_rwBLENDSRCALPHA:return rwBLENDSRCALPHA;
case RWH_CONST_rwCULLMODECULLNONE:return rwCULLMODECULLNONE;
case RWH_CONST_rwFILTERLINEAR:return rwFILTERLINEAR;
case RWH_CONST_rwFOGTYPELINEAR:return rwFOGTYPELINEAR;
case RWH_CONST_rwRASTERTYPECAMERA:return rwRASTERTYPECAMERA;
case RWH_CONST_rwRASTERTYPEZBUFFER:return rwRASTERTYPEZBUFFER;
case RWH_CONST_rwRENDERSTATEBORDERCOLOR:return rwRENDERSTATEBORDERCOLOR;
case RWH_CONST_rwRENDERSTATECULLMODE:return rwRENDERSTATECULLMODE;
case RWH_CONST_rwRENDERSTATEDESTBLEND:return rwRENDERSTATEDESTBLEND;
case RWH_CONST_rwRENDERSTATEFOGCOLOR:return rwRENDERSTATEFOGCOLOR;
case RWH_CONST_rwRENDERSTATEFOGENABLE:return rwRENDERSTATEFOGENABLE;
case RWH_CONST_rwRENDERSTATEFOGTYPE:return rwRENDERSTATEFOGTYPE;
case RWH_CONST_rwRENDERSTATESHADEMODE:return rwRENDERSTATESHADEMODE;
case RWH_CONST_rwRENDERSTATESRCBLEND:return rwRENDERSTATESRCBLEND;
case RWH_CONST_rwRENDERSTATETEXTUREADDRESS:return rwRENDERSTATETEXTUREADDRESS;
case RWH_CONST_rwRENDERSTATETEXTUREFILTER:return rwRENDERSTATETEXTUREFILTER;
case RWH_CONST_rwRENDERSTATETEXTUREPERSPECTIVE:return rwRENDERSTATETEXTUREPERSPECTIVE;
case RWH_CONST_rwRENDERSTATEVERTEXALPHAENABLE:return rwRENDERSTATEVERTEXALPHAENABLE;
case RWH_CONST_rwRENDERSTATEZTESTENABLE:return rwRENDERSTATEZTESTENABLE;
case RWH_CONST_rwRENDERSTATEZWRITEENABLE:return rwRENDERSTATEZWRITEENABLE;
case RWH_CONST_rwSHADEMODEGOURAUD:return rwSHADEMODEGOURAUD;
case RWH_CONST_rwTEXTUREADDRESSWRAP:return rwTEXTUREADDRESSWRAP;
case RWH_CONST_rwVIDEOMODEEXCLUSIVE:return rwVIDEOMODEEXCLUSIVE;
}return 0;
}
#include "RwHelper.h"
#ifdef LIBRW
RpAtomic *ConvertPlatformAtomic(RpAtomic *atomic,void *data)
{
    // Enter the complete C policy through the original typed SDK iterator callback ABI
    return (RpAtomic*)RwHelper_ConvertPlatformAtomic(atomic,data);
}
#endif
//- rouz edit (ChatGPT)
