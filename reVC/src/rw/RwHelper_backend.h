//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#if defined(LIBRW) && (defined(RW_GL3) || defined(RW_WDGL)) && !defined(RW_OPENGL)
#define RW_OPENGL
#endif
#include "Debug.h"
typedef struct RwHelperColour {uint8_t red,green,blue,alpha;} RwHelperColour;
typedef struct RwHelperRect {int32_t x,y,w,h;} RwHelperRect;
typedef struct RwHelperRectRef {int32_t *x,*y,*w,*h;} RwHelperRectRef;
typedef struct RwHelperVector2 {float x,y;} RwHelperVector2;
typedef struct RwHelperVideoMode {int32_t width,height;uint32_t flags;} RwHelperVideoMode;
typedef void *(*RwHelperVisit)(void *object,void *data);
typedef void (*RwHelperPairScope)(void *data,void *first,void *second);
typedef void (*RwHelperVertexScope)(void *data,void *vertices);
#ifdef __cplusplus
extern "C" {
#endif
#if !defined(FINAL) || defined(DEBUGMENU)
void RwHelper_HostCharsetOpen(void);
#endif
#if !defined(FINAL) || defined(DEBUGMENU)
void RwHelper_HostCharsetClose(void);
#endif
#if !defined(FINAL) || defined(DEBUGMENU)
void RwHelper_HostCharsetDestroy(void *charset);
#endif
#if !defined(FINAL) || defined(DEBUGMENU)
void RwHelper_HostCharsetPrint(void *charset,const char *text,int32_t x,int32_t y,bool hide);
#endif
#if !defined(FINAL) || defined(DEBUGMENU)
void RwHelper_HostCharsetFlush(void);
#endif
void * RwHelper_HostAtomicGeometry(void *atomic);
void * RwHelper_HostGeometrySkin(void *geometry);
void * RwHelper_HostAtomicHierarchy(void *atomic);
void * RwHelper_HostFrameHierarchy(void *frame);
void * RwHelper_HostClumpFrame(void *clump);
int32_t RwHelper_HostNumBones(void *skin);
const void * RwHelper_HostSkinMatrices(void *skin);
const void * RwHelper_HostMatrixAt(const void *matrices,int32_t index);
void * RwHelper_HostVectorAt(void *vectors,int32_t index);
const void * RwHelper_HostMatrixPosition(const void *matrix);
void RwHelper_HostCopyMatrix(void *destination,const void *source);
void RwHelper_HostInvertMatrix(void *destination,const void *source);
void RwHelper_HostTransformPoints(void *destination,const void *source,int32_t count,const void *matrix);
uint32_t RwHelper_HostNodeFlags(void *hierarchy,int32_t index);
int32_t RwHelper_HostNumNodes(void *hierarchy);
void * RwHelper_HostCreateAnimation(int32_t type,int32_t count,int32_t flags,float duration);
void * RwHelper_HostAnimationFrame(void *animation,int32_t index);
void RwHelper_HostPreviousFrame(void *frame,void *previous);
void * RwHelper_HostHierarchyMatrices(void *hierarchy);
void * RwHelper_HostAtomicSphere(void *atomic);
void * RwHelper_HostAtomicClump(void *atomic);
void * RwHelper_HostFrameLTM(void *frame);
void * RwHelper_HostFrameMatrix(void *frame);
void * RwHelper_HostSphereCenter(void *sphere);
float * RwHelper_HostSphereRadius(void *sphere);
void RwHelper_HostVectorAdd(void *destination,void *a,void *b);
void RwHelper_HostVectorSub(void *destination,void *a,void *b);
void RwHelper_HostVectorScale(void *destination,void *source,float scale);
float RwHelper_HostVectorLength(void *vector);
int32_t RwHelper_HostNumAtomics(void *clump);
void RwHelper_HostCopySphere(void *destination,void *source);
void * RwHelper_HostCameraRaster(void *camera);
void * RwHelper_HostCameraZRaster(void *camera);
int32_t RwHelper_HostRasterWidth(void *raster);
int32_t RwHelper_HostRasterHeight(void *raster);
void RwHelper_HostDestroyRaster(void *raster);
void RwHelper_HostStoreCameraRaster(void *camera,void *raster);
void RwHelper_HostStoreCameraZRaster(void *camera,void *raster);
void * RwHelper_HostCreateRaster(int32_t width,int32_t height,int32_t depth,int32_t type);
void RwHelper_HostSetCameraRaster(void *camera,void *raster);
void RwHelper_HostSetCameraZRaster(void *camera,void *raster);
int32_t * RwHelper_HostRasterWidthSlot(void *raster);
int32_t * RwHelper_HostRasterHeightSlot(void *raster);
void RwHelper_HostScreenWidth(int32_t width);
void RwHelper_HostScreenHeight(int32_t height);
void * RwHelper_HostCameraFrame(void *camera);
void RwHelper_HostDestroyFrame(void *frame);
void * RwHelper_HostRasterParent(void *raster);
void RwHelper_HostDestroyCamera(void *camera);
void * RwHelper_HostCreateCamera(void);
void * RwHelper_HostCreateFrame(void);
void RwHelper_HostSetCameraFrame(void *camera,void *frame);
#ifdef LIBRW
void * RwHelper_HostAtomicGeometryField(void *atomic);
void * RwHelper_HostInstanceData(void *geometry);
int32_t RwHelper_HostInstancePlatform(void *instance);
void * RwHelper_HostPipeline(void *atomic);
void RwHelper_HostSetPipeline(void *atomic,void *pipeline);
int32_t RwHelper_HostPipelinePlatform(void *pipeline);
uint32_t RwHelper_HostPipelinePlugin(void *pipeline);
void * RwHelper_HostSkinPipeline(int32_t platform);
void * RwHelper_HostMatFXPipeline(int32_t platform);
int32_t RwHelper_HostPlatform(void);
void RwHelper_HostStorePlatform(int32_t platform);
void RwHelper_HostMaterialEffects(void *atomic);
void RwHelper_HostForMaterials(void *geometry,int32_t *effects);
uint32_t RwHelper_HostGeometryFlags(void *geometry);
void RwHelper_HostUninstance(void *atomic);
void RwHelper_HostLibSetState(uint32_t state,uint32_t value);
uint32_t RwHelper_HostLibGetState(uint32_t state);
#endif
void RwHelper_HostChildren(void *parent,RwHelperVisit callback,void *data);
void RwHelper_HostObjects(void *parent,RwHelperVisit callback,void *data);
void RwHelper_HostAtomics(void *parent,RwHelperVisit callback,void *data);
void RwHelper_HostTextures(void *parent,RwHelperVisit callback,void *data);
void RwHelper_HostBoneMatrices(RwHelperPairScope callback,void *data);
void RwHelper_HostAtomicVectors(RwHelperPairScope callback,void *data);
void RwHelper_HostBoundingSphere(RwHelperPairScope callback,void *data);
void RwHelper_HostQuadVertices(RwHelperVertexScope callback,void *data);
float * RwHelper_HostVectorField(void *vector,int32_t index);
Debug_Vector RwHelper_HostVectorValue(const void *vector);
float * RwHelper_HostKeyFrameField(void *frame,int32_t index);
void RwHelper_HostStoreSphere(void *sphere,float x,float y,float z,float radius);
RwHelperRectRef RwHelper_HostRect(void *rectangle);
int32_t RwHelper_HostCurrentVideoMode(void);
void RwHelper_HostVideoMode(RwHelperVideoMode *mode,int32_t index);
void RwHelper_HostViewWindow(void *camera,const RwHelperVector2 *window);
#if !defined(FINAL) || defined(DEBUGMENU)
void * RwHelper_HostCharsetCreate(const RwHelperColour *foreground,const RwHelperColour *background);
#endif
#if defined(RW_D3D9)
void * RwHelper_HostWideBuffer(void);
#endif
#if defined(RW_D3D9)
void RwHelper_HostToWide(const char *name,void *buffer,uint32_t count);
#endif
#if defined(RW_D3D9)
void RwHelper_HostPerfBegin(uint32_t colour,void *buffer);
#endif
#if defined(RW_D3D9)
void RwHelper_HostPerfEnd(void);
#endif
#if defined(RW_OPENGL)
bool RwHelper_HostGLDebugAvailable(void);
#endif
#if defined(RW_OPENGL)
void RwHelper_HostGLDebugPush(const char *name);
#endif
#if defined(RW_OPENGL)
void RwHelper_HostGLDebugPop(void);
#endif
void * RwHelper_HostVertexAt(void *vertices,int32_t index);
void RwHelper_HostDrawStrip(void *vertices,int32_t count);
void RwHelper_HostRenderState(uint32_t state,void *value);
#if !defined(LIBRW)
void RwHelper_HostD3DSetState(uint32_t state,uint32_t value);
#endif
#if !defined(LIBRW)
void RwHelper_HostD3DGetState(uint32_t state,uint32_t *value);
#endif
void RwHelper_HostVertexScreenX(void *vertex,float value);
void RwHelper_HostVertexScreenY(void *vertex,float value);
void RwHelper_HostVertexScreenZ(void *vertex,float value);
void RwHelper_HostVertexCameraZ(void *vertex,float value);
void RwHelper_HostVertexRecipCameraZ(void *vertex,float value);
void RwHelper_HostVertexU(void *vertex,float value,float reciprocal);
void RwHelper_HostVertexV(void *vertex,float value,float reciprocal);
void RwHelper_HostVertexIntRGBA(void *vertex,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha);
enum {RWH_CONST_D3DCMP_GREATER,RWH_CONST_D3DCMP_GREATEREQUAL,RWH_CONST_D3DRS_ALPHAFUNC,RWH_CONST_D3DRS_ALPHAREF,RWH_CONST_LIB_ALPHAGREATEREQUAL,RWH_CONST_LIB_ALPHATESTFUNC,RWH_CONST_LIB_ALPHATESTREF,RWH_CONST_LIB_GSALPHATEST,RWH_CONST_LIB_Geometry_NATIVE,RWH_CONST_LIB_ID_MATFX,RWH_CONST_LIB_ID_SKIN,RWH_CONST_rpHANIMPOPPARENTMATRIX,RWH_CONST_rpHANIMPUSHPARENTMATRIX,RWH_CONST_rpHANIMSTDKEYFRAMETYPEID,RWH_CONST_rpMATFXEFFECTNULL,RWH_CONST_rwBLENDINVSRCALPHA,RWH_CONST_rwBLENDSRCALPHA,RWH_CONST_rwCULLMODECULLNONE,RWH_CONST_rwFILTERLINEAR,RWH_CONST_rwFOGTYPELINEAR,RWH_CONST_rwRASTERTYPECAMERA,RWH_CONST_rwRASTERTYPEZBUFFER,RWH_CONST_rwRENDERSTATEBORDERCOLOR,RWH_CONST_rwRENDERSTATECULLMODE,RWH_CONST_rwRENDERSTATEDESTBLEND,RWH_CONST_rwRENDERSTATEFOGCOLOR,RWH_CONST_rwRENDERSTATEFOGENABLE,RWH_CONST_rwRENDERSTATEFOGTYPE,RWH_CONST_rwRENDERSTATESHADEMODE,RWH_CONST_rwRENDERSTATESRCBLEND,RWH_CONST_rwRENDERSTATETEXTUREADDRESS,RWH_CONST_rwRENDERSTATETEXTUREFILTER,RWH_CONST_rwRENDERSTATETEXTUREPERSPECTIVE,RWH_CONST_rwRENDERSTATEVERTEXALPHAENABLE,RWH_CONST_rwRENDERSTATEZTESTENABLE,RWH_CONST_rwRENDERSTATEZWRITEENABLE,RWH_CONST_rwSHADEMODEGOURAUD,RWH_CONST_rwTEXTUREADDRESSWRAP,RWH_CONST_rwVIDEOMODEEXCLUSIVE};
uint32_t RwHelper_HostConstant(int32_t identity);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
