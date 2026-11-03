//+ rouz edit (ChatGPT)
#pragma once
#include "ShadowCamera.h"
#include "RwHelper.h"
#include "RwHelper_backend.h"
typedef struct ShadowVector2 {float x,y;} ShadowVector2;
typedef struct ShadowVectorOut {float *x,*y,*z;} ShadowVectorOut;
enum {SHADOW_MATRIX_RIGHT,SHADOW_MATRIX_UP,SHADOW_MATRIX_AT,SHADOW_MATRIX_POS};
#ifdef __cplusplus
extern "C" {
#endif
void re3_assert(const char*,const char*,unsigned int,const char*);
void * Shadow_HostTextureCreate(void *raster);
void Shadow_HostTextureRaster(void *texture,void *raster);
void Shadow_HostTextureAddress(void *texture,int32_t mode);
void Shadow_HostTextureFilter(void *texture,int32_t mode);
void Shadow_HostProjection(void *camera,int32_t mode);
void Shadow_HostFarClip(void *camera,float distance);
void Shadow_HostNearClip(void *camera,float distance);
float Shadow_HostGetFarClip(void *camera);
void * Shadow_HostLightFrame(void *light);
void Shadow_HostLightSetFrame(void *light,void *frame);
void Shadow_HostLightDestroy(void *light);
void Shadow_HostMatrixUpdate(void *matrix);
void Shadow_HostFrameUpdate(void *frame);
void Shadow_HostFrameOrtho(void *frame);
void Shadow_HostIncrementScaled(void *destination,void *source,float value);
void Shadow_HostCopyVector(void *destination,void *source);
void Shadow_HostViewWindow(void *camera,const ShadowVector2 *window);
void * Shadow_HostMatrixVector(void *matrix,int32_t field);
ShadowVectorOut Shadow_HostVectorOut(void *vector);
void Shadow_HostFrameRotate(void *frame,const ShadowVector *axis,float angle,int32_t mode);
enum {SHADOW_CONST_rwRASTERTYPEZBUFFER,SHADOW_CONST_rwRASTERTYPECAMERATEXTURE,SHADOW_CONST_rwTEXTUREADDRESSCLAMP,SHADOW_CONST_rwFILTERLINEAR,SHADOW_CONST_rwPARALLEL,SHADOW_CONST_rwCOMBINEREPLACE,SHADOW_CONST_rwCOMBINEPOSTCONCAT};
int32_t Shadow_HostConstant(int32_t identity);
typedef void (*ShadowRenderToken)(void);
bool Shadow_BeginSoftwareClear(void *camera,const ShadowColour *colour);
bool Shadow_BeginSoftwareRaster(void *camera,void *raster);
void Shadow_EndSoftware(void *camera,bool active);
void *Shadow_RenderAtomic(void *atomic,void *data);
uint32_t Shadow_HostGeometryFlags(void *geometry);
void Shadow_HostGeometrySetFlags(void *geometry,uint32_t flags);
bool Shadow_HostBeginUpdate(void *camera);
void Shadow_HostEndUpdate(void *camera);
void Shadow_HostClear(void *camera,const ShadowColour *colour,int32_t flags);
float Shadow_HostGetNearClip(void *camera);
float Shadow_HostNearScreenZ(void);
ShadowRenderToken Shadow_HostGetRenderCallback(void *atomic);
void Shadow_HostSetRenderCallback(void *atomic,ShadowRenderToken callback);
ShadowRenderToken Shadow_HostDefaultRenderCallback(void);
void Shadow_HostAtomicRender(void *atomic);
void * Shadow_HostVertex(void *vertices,int32_t index);
void Shadow_HostDrawStrip(void *vertices,int32_t count);
void Shadow_HostDrawLine(void *vertices,int32_t count,int32_t first,int32_t second);
void Shadow_HostDrawPolyline(void *vertices,int32_t count,uint16_t *indices,int32_t indexCount);
void Shadow_HostTwoVertices(RwHelperVertexScope callback,void *data);
void Shadow_HostVertexScreenX(void *vertex,float value);
void Shadow_HostVertexScreenY(void *vertex,float value);
void Shadow_HostVertexScreenZ(void *vertex,float value);
void Shadow_HostVertexCameraZ(void *vertex,float value);
void Shadow_HostVertexRecipCameraZ(void *vertex,float value);
void Shadow_HostVertexU(void *vertex,float value,float reciprocal);
void Shadow_HostVertexV(void *vertex,float value,float reciprocal);
void Shadow_HostVertexIntRGBA(void *vertex,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha);
#ifdef REVC_SOFTWARE_POLYGONS
bool Shadow_HostSoftwareClear(void *camera,const ShadowColour *colour);
bool Shadow_HostSoftwareRaster(void *camera,void *raster);
void Shadow_HostSoftwareEnd(void *raster);
#endif
enum {SHADOW_EFFECT_CONST_rpGEOMETRYPRELIT,SHADOW_EFFECT_CONST_rpGEOMETRYLIGHT,SHADOW_EFFECT_CONST_rpGEOMETRYTEXTURED,SHADOW_EFFECT_CONST_rpGEOMETRYTEXTURED2,SHADOW_EFFECT_CONST_rpGEOMETRYMODULATEMATERIALCOLOR,SHADOW_EFFECT_CONST_rpGEOMETRYNORMALS,SHADOW_EFFECT_CONST_rwCAMERACLEARZ,SHADOW_EFFECT_CONST_rwCAMERACLEARIMAGE,SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE,SHADOW_EFFECT_CONST_rwRENDERSTATETEXTURERASTER,SHADOW_EFFECT_CONST_rwRENDERSTATEVERTEXALPHAENABLE,SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND,SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND,SHADOW_EFFECT_CONST_rwRENDERSTATETEXTUREFILTER,SHADOW_EFFECT_CONST_rwRENDERSTATESHADEMODE,SHADOW_EFFECT_CONST_rwBLENDINVDESTCOLOR,SHADOW_EFFECT_CONST_rwBLENDZERO,SHADOW_EFFECT_CONST_rwBLENDSRCALPHA,SHADOW_EFFECT_CONST_rwBLENDINVSRCALPHA,SHADOW_EFFECT_CONST_rwBLENDONE,SHADOW_EFFECT_CONST_rwBLENDSRCCOLOR,SHADOW_EFFECT_CONST_rwFILTERLINEAR,SHADOW_EFFECT_CONST_rwFILTERNAFILTERMODE,SHADOW_EFFECT_CONST_rwSHADEMODEFLAT,SHADOW_EFFECT_CONST_rwSHADEMODEGOURAUD};
int32_t Shadow_HostEffectConstant(int32_t identity);
typedef struct ShadowLightColour {float red,green,blue,alpha;} ShadowLightColour;
void *Shadow_HostLightCreate(int32_t kind);
void Shadow_HostLightColour(void *light,const ShadowLightColour *colour);
int32_t Shadow_HostObjectType(void *object);
void *Shadow_HostAtomicFrame(void *atomic);
ShadowSphere Shadow_HostReadSphere(void *sphere);
void Shadow_HostTransformVector(ShadowVector *destination,const ShadowVector *source,void *matrix);
enum {SHADOW_CUT_CONST_rpLIGHTDIRECTIONAL,SHADOW_CUT_CONST_rpCLUMP,SHADOW_CUT_CONST_rpATOMIC};
int32_t Shadow_HostCutsceneConstant(int32_t identity);
static inline RwHelperSphereRef Shadow_SphereRef(ShadowSphere *sphere)
{
    // Borrow actual C scalar receiver fields while leaving all SDK scratch storage inside the helper peer
    RwHelperSphereRef value={NULL,NULL,NULL,NULL};if(sphere){value.x=&sphere->center.x;value.y=&sphere->center.y;value.z=&sphere->center.z;value.radius=&sphere->radius;}return value;
}
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
