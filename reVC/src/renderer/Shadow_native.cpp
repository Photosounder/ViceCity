//+ rouz edit (ChatGPT)
#define WITHD3D
#include "common.h"
#include "Shadow_backend.h"
void * Shadow_HostTextureCreate(void *raster)
{
    // Forward one original SDK operation with its actual typed objects
    return RwTextureCreate((RwRaster*)raster);
}
void Shadow_HostTextureRaster(void *texture,void *raster)
{
    // Forward one original SDK operation with its actual typed objects
    RwTextureSetRaster((RwTexture*)texture,(RwRaster*)raster);
}
void Shadow_HostTextureAddress(void *texture,int32_t mode)
{
    // Forward one original SDK operation with its actual typed objects
    RwTextureSetAddressing((RwTexture*)texture,(RwTextureAddressMode)mode);
}
void Shadow_HostTextureFilter(void *texture,int32_t mode)
{
    // Forward one original SDK operation with its actual typed objects
    RwTextureSetFilterMode((RwTexture*)texture,(RwTextureFilterMode)mode);
}
void Shadow_HostProjection(void *camera,int32_t mode)
{
    // Forward one original SDK operation with its actual typed objects
    RwCameraSetProjection((RwCamera*)camera,(RwCameraProjection)mode);
}
void Shadow_HostFarClip(void *camera,float distance)
{
    // Forward one original SDK operation with its actual typed objects
    RwCameraSetFarClipPlane((RwCamera*)camera,distance);
}
void Shadow_HostNearClip(void *camera,float distance)
{
    // Forward one original SDK operation with its actual typed objects
    RwCameraSetNearClipPlane((RwCamera*)camera,distance);
}
float Shadow_HostGetFarClip(void *camera)
{
    // Forward one original SDK operation with its actual typed objects
    return RwCameraGetFarClipPlane((RwCamera*)camera);
}
void * Shadow_HostLightFrame(void *light)
{
    // Forward one original SDK operation with its actual typed objects
    return RpLightGetFrame((RpLight*)light);
}
void Shadow_HostLightSetFrame(void *light,void *frame)
{
    // Forward one original SDK operation with its actual typed objects
    RpLightSetFrame((RpLight*)light,(RwFrame*)frame);
}
void Shadow_HostLightDestroy(void *light)
{
    // Forward one original SDK operation with its actual typed objects
    RpLightDestroy((RpLight*)light);
}
void Shadow_HostMatrixUpdate(void *matrix)
{
    // Forward one original SDK operation with its actual typed objects
    RwMatrixUpdate((RwMatrix*)matrix);
}
void Shadow_HostFrameUpdate(void *frame)
{
    // Forward one original SDK operation with its actual typed objects
    RwFrameUpdateObjects((RwFrame*)frame);
}
void Shadow_HostFrameOrtho(void *frame)
{
    // Forward one original SDK operation with its actual typed objects
    RwFrameOrthoNormalize((RwFrame*)frame);
}
void Shadow_HostIncrementScaled(void *destination,void *source,float value)
{
    // Forward one original SDK operation with its actual typed objects
    RwV3dIncrementScaled((RwV3d*)destination,(RwV3d*)source,value);
}
void Shadow_HostCopyVector(void *destination,void *source)
{
    // Forward one original SDK operation with its actual typed objects
    *(RwV3d*)destination=*(RwV3d*)source;
}
void Shadow_HostViewWindow(void *camera,const ShadowVector2 *window)
{
    // Access actual SDK storage or construct a real SDK input record at the original operation
    RwV2d native={window->x,window->y};RwCameraSetViewWindow((RwCamera*)camera,&native);
}
void * Shadow_HostMatrixVector(void *matrix,int32_t field)
{
    // Access actual SDK storage or construct a real SDK input record at the original operation
    RwMatrix *m=(RwMatrix*)matrix;switch(field){case SHADOW_MATRIX_RIGHT:return RwMatrixGetRight(m);case SHADOW_MATRIX_UP:return RwMatrixGetUp(m);case SHADOW_MATRIX_AT:return RwMatrixGetAt(m);case SHADOW_MATRIX_POS:return RwMatrixGetPos(m);}return NULL;
}
ShadowVectorOut Shadow_HostVectorOut(void *vector)
{
    // Access actual SDK storage or construct a real SDK input record at the original operation
    RwV3d *v=(RwV3d*)vector;ShadowVectorOut value={&v->x,&v->y,&v->z};return value;
}
void Shadow_HostFrameRotate(void *frame,const ShadowVector *axis,float angle,int32_t mode)
{
    // Access actual SDK storage or construct a real SDK input record at the original operation
    RwV3d native={axis->x,axis->y,axis->z};RwFrameRotate((RwFrame*)frame,&native,angle,(RwOpCombineType)mode);
}
int32_t Shadow_HostConstant(int32_t identity)
{
    // Translate only SDK enum identities while all application selection remains in C
    switch(identity){case SHADOW_CONST_rwRASTERTYPEZBUFFER:return rwRASTERTYPEZBUFFER;case SHADOW_CONST_rwRASTERTYPECAMERATEXTURE:return rwRASTERTYPECAMERATEXTURE;case SHADOW_CONST_rwTEXTUREADDRESSCLAMP:return rwTEXTUREADDRESSCLAMP;case SHADOW_CONST_rwFILTERLINEAR:return rwFILTERLINEAR;case SHADOW_CONST_rwPARALLEL:return rwPARALLEL;case SHADOW_CONST_rwCOMBINEREPLACE:return rwCOMBINEREPLACE;case SHADOW_CONST_rwCOMBINEPOSTCONCAT:return rwCOMBINEPOSTCONCAT;}return 0;
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
#include "SoftwarePolygons.h"
#endif
uint32_t Shadow_HostGeometryFlags(void *geometry)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    return RpGeometryGetFlags((RpGeometry*)geometry);
}
void Shadow_HostGeometrySetFlags(void *geometry,uint32_t flags)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    RpGeometrySetFlags((RpGeometry*)geometry,flags);
}
bool Shadow_HostBeginUpdate(void *camera)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    return RwCameraBeginUpdate((RwCamera*)camera)!=NULL;
}
void Shadow_HostEndUpdate(void *camera)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    RwCameraEndUpdate((RwCamera*)camera);
}
void Shadow_HostClear(void *camera,const ShadowColour *colour,int32_t flags)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    RwRGBA native={colour->red,colour->green,colour->blue,colour->alpha};RwCameraClear((RwCamera*)camera,&native,flags);
}
float Shadow_HostGetNearClip(void *camera)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    return RwCameraGetNearClipPlane((RwCamera*)camera);
}
float Shadow_HostNearScreenZ(void)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    return RwIm2DGetNearScreenZ();
}
ShadowRenderToken Shadow_HostGetRenderCallback(void *atomic)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    return reinterpret_cast<ShadowRenderToken>(RpAtomicGetRenderCallBack((RpAtomic*)atomic));
}
void Shadow_HostSetRenderCallback(void *atomic,ShadowRenderToken callback)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    RpAtomicSetRenderCallBack((RpAtomic*)atomic,reinterpret_cast<RpAtomicCallBackRender>(callback));
}
ShadowRenderToken Shadow_HostDefaultRenderCallback(void)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    return reinterpret_cast<ShadowRenderToken>(AtomicDefaultRenderCallBack);
}
void Shadow_HostAtomicRender(void *atomic)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    RpAtomicRender((RpAtomic*)atomic);
}
void * Shadow_HostVertex(void *vertices,int32_t index)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    return &((RwIm2DVertex*)vertices)[index];
}
void Shadow_HostDrawStrip(void *vertices,int32_t count)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP,(RwIm2DVertex*)vertices,count);
}
void Shadow_HostDrawLine(void *vertices,int32_t count,int32_t first,int32_t second)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    RwIm2DRenderLine((RwIm2DVertex*)vertices,count,first,second);
}
void Shadow_HostDrawPolyline(void *vertices,int32_t count,uint16_t *indices,int32_t indexCount)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    RwIm2DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE,(RwIm2DVertex*)vertices,count,indices,indexCount);
}
void Shadow_HostTwoVertices(RwHelperVertexScope callback,void *data)
{
    // Preserve the exact typed SDK primitive and its borrowed arguments
    RwIm2DVertex vertices[2];callback(data,vertices);
}
void Shadow_HostVertexScreenX(void *vertex,float value)
{
    // Apply one original SDK vertex setter without exposing SDK record layout to C
    RwIm2DVertexSetScreenX((RwIm2DVertex*)vertex,value);
}
void Shadow_HostVertexScreenY(void *vertex,float value)
{
    // Apply one original SDK vertex setter without exposing SDK record layout to C
    RwIm2DVertexSetScreenY((RwIm2DVertex*)vertex,value);
}
void Shadow_HostVertexScreenZ(void *vertex,float value)
{
    // Apply one original SDK vertex setter without exposing SDK record layout to C
    RwIm2DVertexSetScreenZ((RwIm2DVertex*)vertex,value);
}
void Shadow_HostVertexCameraZ(void *vertex,float value)
{
    // Apply one original SDK vertex setter without exposing SDK record layout to C
    RwIm2DVertexSetCameraZ((RwIm2DVertex*)vertex,value);
}
void Shadow_HostVertexRecipCameraZ(void *vertex,float value)
{
    // Apply one original SDK vertex setter without exposing SDK record layout to C
    RwIm2DVertexSetRecipCameraZ((RwIm2DVertex*)vertex,value);
}
void Shadow_HostVertexU(void *vertex,float value,float reciprocal)
{
    // Apply one original SDK vertex setter without exposing SDK record layout to C
    RwIm2DVertexSetU((RwIm2DVertex*)vertex,value,reciprocal);
}
void Shadow_HostVertexV(void *vertex,float value,float reciprocal)
{
    // Apply one original SDK vertex setter without exposing SDK record layout to C
    RwIm2DVertexSetV((RwIm2DVertex*)vertex,value,reciprocal);
}
void Shadow_HostVertexIntRGBA(void *vertex,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha)
{
    // Apply one original SDK vertex setter without exposing SDK record layout to C
    RwIm2DVertexSetIntRGBA((RwIm2DVertex*)vertex,red,green,blue,alpha);
}
#ifdef REVC_SOFTWARE_POLYGONS
bool Shadow_HostSoftwareClear(void *camera,const ShadowColour *colour)
{
    // Preserve the existing software owner's colour overload with a real native RGBA input
    rw::RGBA native={colour->red,colour->green,colour->blue,colour->alpha};return SoftwarePolygons::BeginOffscreenFrame((RwCamera*)camera,native);
}
bool Shadow_HostSoftwareRaster(void *camera,void *raster)
{
    // Preserve the existing software owner's initial-image overload
    return SoftwarePolygons::BeginOffscreenFrame((RwCamera*)camera,(RwRaster*)raster);
}
void Shadow_HostSoftwareEnd(void *raster)
{
    // Upload the C-selected current raster through the unchanged software owner
    SoftwarePolygons::EndOffscreenFrame((RwRaster*)raster);
}
#endif
static_assert(sizeof(ShadowRenderToken)==sizeof(RpAtomicCallBackRender),"Preserve native render callback transport");
int32_t Shadow_HostEffectConstant(int32_t identity)
{
    // Translate SDK enum identities while every render-state choice remains in C
    switch(identity){case SHADOW_EFFECT_CONST_rpGEOMETRYPRELIT:return rpGEOMETRYPRELIT;case SHADOW_EFFECT_CONST_rpGEOMETRYLIGHT:return rpGEOMETRYLIGHT;case SHADOW_EFFECT_CONST_rpGEOMETRYTEXTURED:return rpGEOMETRYTEXTURED;case SHADOW_EFFECT_CONST_rpGEOMETRYTEXTURED2:return rpGEOMETRYTEXTURED2;case SHADOW_EFFECT_CONST_rpGEOMETRYMODULATEMATERIALCOLOR:return rpGEOMETRYMODULATEMATERIALCOLOR;case SHADOW_EFFECT_CONST_rpGEOMETRYNORMALS:return rpGEOMETRYNORMALS;case SHADOW_EFFECT_CONST_rwCAMERACLEARZ:return rwCAMERACLEARZ;case SHADOW_EFFECT_CONST_rwCAMERACLEARIMAGE:return rwCAMERACLEARIMAGE;case SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE:return rwRENDERSTATEZTESTENABLE;case SHADOW_EFFECT_CONST_rwRENDERSTATETEXTURERASTER:return rwRENDERSTATETEXTURERASTER;case SHADOW_EFFECT_CONST_rwRENDERSTATEVERTEXALPHAENABLE:return rwRENDERSTATEVERTEXALPHAENABLE;case SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND:return rwRENDERSTATESRCBLEND;case SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND:return rwRENDERSTATEDESTBLEND;case SHADOW_EFFECT_CONST_rwRENDERSTATETEXTUREFILTER:return rwRENDERSTATETEXTUREFILTER;case SHADOW_EFFECT_CONST_rwRENDERSTATESHADEMODE:return rwRENDERSTATESHADEMODE;case SHADOW_EFFECT_CONST_rwBLENDINVDESTCOLOR:return rwBLENDINVDESTCOLOR;case SHADOW_EFFECT_CONST_rwBLENDZERO:return rwBLENDZERO;case SHADOW_EFFECT_CONST_rwBLENDSRCALPHA:return rwBLENDSRCALPHA;case SHADOW_EFFECT_CONST_rwBLENDINVSRCALPHA:return rwBLENDINVSRCALPHA;case SHADOW_EFFECT_CONST_rwBLENDONE:return rwBLENDONE;case SHADOW_EFFECT_CONST_rwBLENDSRCCOLOR:return rwBLENDSRCCOLOR;case SHADOW_EFFECT_CONST_rwFILTERLINEAR:return rwFILTERLINEAR;case SHADOW_EFFECT_CONST_rwFILTERNAFILTERMODE:return rwFILTERNAFILTERMODE;case SHADOW_EFFECT_CONST_rwSHADEMODEFLAT:return rwSHADEMODEFLAT;case SHADOW_EFFECT_CONST_rwSHADEMODEGOURAUD:return rwSHADEMODEGOURAUD;}return 0;
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#include <string.h>
void *Shadow_HostLightCreate(int32_t kind)
{
    // Preserve the original SDK directional light allocation primitive
    return RpLightCreate(kind);
}
void Shadow_HostLightColour(void *light,const ShadowLightColour *colour)
{
    // Construct the actual SDK colour record without aliasing an unrelated C record type
    RwRGBAReal value={colour->red,colour->green,colour->blue,colour->alpha};RpLightSetColor((RpLight*)light,&value);
}
int32_t Shadow_HostObjectType(void *object)
{
    // Return the original SDK object type code for canonical application storage
    return RwObjectGetType((RwObject*)object);
}
void *Shadow_HostAtomicFrame(void *atomic)
{
    // Preserve the actual SDK atomic frame lookup
    return RpAtomicGetFrame((RpAtomic*)atomic);
}
ShadowSphere Shadow_HostReadSphere(void *sphere)
{
    // Copy only actual SDK sphere field representations into the owned C record
    const RwSphere *source=(RwSphere*)sphere;ShadowSphere value;memcpy(&value.center.x,&source->center.x,sizeof(float));memcpy(&value.center.y,&source->center.y,sizeof(float));memcpy(&value.center.z,&source->center.z,sizeof(float));memcpy(&value.radius,&source->radius,sizeof(float));return value;
}
void Shadow_HostTransformVector(ShadowVector *destination,const ShadowVector *source,void *matrix)
{
    // Invoke one original SDK point transform using real SDK vectors and copy its completed result into C storage
    RwV3d input,output;memcpy(&input.x,&source->x,sizeof(float));memcpy(&input.y,&source->y,sizeof(float));memcpy(&input.z,&source->z,sizeof(float));RwV3dTransformPoints(&output,&input,1,(RwMatrix*)matrix);memcpy(&destination->x,&output.x,sizeof(float));memcpy(&destination->y,&output.y,sizeof(float));memcpy(&destination->z,&output.z,sizeof(float));
}
int32_t Shadow_HostCutsceneConstant(int32_t identity)
{
    // Translate SDK object and light identities without changing canonical type values or application selection
    switch(identity){case SHADOW_CUT_CONST_rpLIGHTDIRECTIONAL:return rpLIGHTDIRECTIONAL;case SHADOW_CUT_CONST_rpCLUMP:return rpCLUMP;case SHADOW_CUT_CONST_rpATOMIC:return rpATOMIC;}return 0;
}
//- rouz edit (ChatGPT)
