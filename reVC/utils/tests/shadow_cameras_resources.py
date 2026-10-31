"""Project original shadow camera resource and light operations into C with typed SDK primitives."""
import re
from memory_heap_migration import body

ALIASES={
'RwCameraCreate':'RwHelper_HostCreateCamera','RwCameraGetFrame':'RwHelper_HostCameraFrame','RwCameraSetFrame':'RwHelper_HostSetCameraFrame','RwFrameCreate':'RwHelper_HostCreateFrame','RwFrameDestroy':'RwHelper_HostDestroyFrame','RwCameraGetRaster':'RwHelper_HostCameraRaster','RwCameraGetZRaster':'RwHelper_HostCameraZRaster','RwCameraSetRaster':'RwHelper_HostSetCameraRaster','RwCameraSetZRaster':'RwHelper_HostSetCameraZRaster','RwRasterCreate':'RwHelper_HostCreateRaster','RwRasterDestroy':'RwHelper_HostDestroyRaster','RwCameraDestroy':'RwHelper_HostDestroyCamera','RwFrameGetMatrix':'RwHelper_HostFrameMatrix',
'RwTextureCreate':'Shadow_HostTextureCreate','RwTextureSetRaster':'Shadow_HostTextureRaster','RwTextureDestroy':'Shadow_HostTextureDestroy','RwTextureSetAddressing':'Shadow_HostTextureAddress','RwTextureSetFilterMode':'Shadow_HostTextureFilter','RwCameraSetProjection':'Shadow_HostProjection','RwCameraSetFarClipPlane':'Shadow_HostFarClip','RwCameraSetNearClipPlane':'Shadow_HostNearClip','RwCameraGetFarClipPlane':'Shadow_HostGetFarClip','RwCameraSetViewWindow':'Shadow_HostViewWindow','RpLightGetFrame':'Shadow_HostLightFrame','RpLightSetFrame':'Shadow_HostLightSetFrame','RpLightDestroy':'Shadow_HostLightDestroy','RwMatrixUpdate':'Shadow_HostMatrixUpdate','RwFrameUpdateObjects':'Shadow_HostFrameUpdate','RwFrameOrthoNormalize':'Shadow_HostFrameOrtho','RwV3dIncrementScaled':'Shadow_HostIncrementScaled','RwFrameRotate':'Shadow_HostFrameRotate'}
CONSTANTS=['rwRASTERTYPEZBUFFER','rwRASTERTYPECAMERATEXTURE','rwTEXTUREADDRESSCLAMP','rwFILTERLINEAR','rwPARALLEL','rwCOMBINEREPLACE','rwCOMBINEPOSTCONCAT']
HOST=[
('void *','TextureCreate','void *raster','RwTextureCreate((RwRaster*)raster)'),
('void','TextureRaster','void *texture,void *raster','RwTextureSetRaster((RwTexture*)texture,(RwRaster*)raster)'),
('void','TextureDestroy','void *texture','RwTextureDestroy((RwTexture*)texture)'),
('void','TextureAddress','void *texture,int32_t mode','RwTextureSetAddressing((RwTexture*)texture,(RwTextureAddressMode)mode)'),
('void','TextureFilter','void *texture,int32_t mode','RwTextureSetFilterMode((RwTexture*)texture,(RwTextureFilterMode)mode)'),
('void','Projection','void *camera,int32_t mode','RwCameraSetProjection((RwCamera*)camera,(RwCameraProjection)mode)'),
('void','FarClip','void *camera,float distance','RwCameraSetFarClipPlane((RwCamera*)camera,distance)'),
('void','NearClip','void *camera,float distance','RwCameraSetNearClipPlane((RwCamera*)camera,distance)'),
('float','GetFarClip','void *camera','RwCameraGetFarClipPlane((RwCamera*)camera)'),
('void *','LightFrame','void *light','RpLightGetFrame((RpLight*)light)'),
('void','LightSetFrame','void *light,void *frame','RpLightSetFrame((RpLight*)light,(RwFrame*)frame)'),
('void','LightDestroy','void *light','RpLightDestroy((RpLight*)light)'),
('void','MatrixUpdate','void *matrix','RwMatrixUpdate((RwMatrix*)matrix)'),
('void','FrameUpdate','void *frame','RwFrameUpdateObjects((RwFrame*)frame)'),
('void','FrameOrtho','void *frame','RwFrameOrthoNormalize((RwFrame*)frame)'),
('void','IncrementScaled','void *destination,void *source,float value','RwV3dIncrementScaled((RwV3d*)destination,(RwV3d*)source,value)'),
('void','CopyVector','void *destination,void *source','*(RwV3d*)destination=*(RwV3d*)source'),
]

def project_body(source,signature,fields):
    # Preserve the complete original method body while changing only instance fields and native representations
    value=body(source,signature)
    for name in fields:value=re.sub(r'\b'+name+r'\b','self->'+name,value)
    for old,new in ALIASES.items():value=re.sub(r'\b'+old+r'\b',new,value)
    for old in ['RwCamera','RwRaster','RwFrame','RwTexture','RwMatrix']:value=re.sub(r'\b'+old+r'\b','void',value)
    value=re.sub(r'\bint32\b','int32_t',value)
    for name in CONSTANTS:value=re.sub(r'\b'+name+r'\b','Shadow_HostConstant(SHADOW_CONST_'+name+')',value)
    return value

def resource_methods(camera,cutscene):
    # Move every original allocation and teardown branch into C without adding recovery or resource resets
    outputs=[]
    fields=['m_pCamera','m_pTexture']
    for name,result,parameters in [('Destroy','void',''),('Create','void *',',int32_t rasterSize'),('SetFrustum','void *',',float objectRadius'),('SetLight','void *',',void *light'),('SetCenter','void *',',ShadowVectorRef center')]:
        value=project_body(camera,'CShadowCamera::'+name+'(',fields)
        value=re.sub(r'(?<!\w)Destroy\(\)','ShadowCamera_Destroy(self)',value)
        if name=='SetFrustum':value=value.replace('RwV2d vw;','ShadowVector2 vw;')
        if name=='SetLight':
            for field in ['Right','Up','At']:
                old=r'\*RwMatrixGet'+field+r'\(camMatrix\)\s*=\s*\*RwMatrixGet'+field+r'\(lightMatrix\);'
                assert re.search(old,value)
                value=re.sub(old,'Shadow_HostCopyVector(Shadow_HostMatrixVector(camMatrix,SHADOW_MATRIX_'+field.upper()+'),Shadow_HostMatrixVector(lightMatrix,SHADOW_MATRIX_'+field.upper()+'));',value)
        if name=='SetCenter':
            value=value.replace('ASSERT(center != nil);','ASSERT(center.x != nil);')
            old='*RwMatrixGetPos(camMatrix) = *center;';assert old in value
            value=value.replace(old,'ShadowVectorOut position=Shadow_HostVectorOut(Shadow_HostMatrixVector(camMatrix,SHADOW_MATRIX_POS));\n    memcpy(position.x,center.x,sizeof(float));memcpy(position.y,center.y,sizeof(float));memcpy(position.z,center.z,sizeof(float));')
            for field in ['Pos','At']:value=value.replace('RwMatrixGet'+field+'(camMatrix)','Shadow_HostMatrixVector(camMatrix,SHADOW_MATRIX_'+field.upper()+')')
        outputs.append(result+' ShadowCamera_'+name+'(ShadowCamera *self'+parameters+')\n{\n    // Retain the complete original resource or frame operation and its canonical instance fields\n'+value+'\n}\n')
    cameraCode='\n'.join(outputs)
    fields=['m_pAtomic','m_nRwObjectType','m_pLight','m_bResample']
    outputs=[]
    for name,result,parameters in [('Destroy','void',''),('SetLightProperties','void *',',float angleY,float angleX,bool setLight'),('DrawBorderAroundTexture','void',',const ShadowColour *color')]:
        value=project_body(cutscene,'CCutsceneShadow::'+name+'(',fields)
        for field in ['m_Camera','m_ResampleCamera','m_BlurCamera','m_GradientCamera']:
            value=value.replace(field+'.Destroy()','ShadowCamera_Destroy(&self->'+field+')').replace(field+'.SetLight(self->m_pLight)','ShadowCamera_SetLight(&self->'+field+',self->m_pLight)')
            value=value.replace(field+'.DrawOutlineBorder(color)','ShadowCamera_DrawOutlineBorder(&self->'+field+',color)')
        value=value.replace('static RwV3d','static ShadowVector')
        outputs.append(result+' CutsceneShadow_'+name+'(CutsceneShadow *self'+parameters+')\n{\n    // Preserve original light lifetime, rotation order and camera propagation\n'+value+'\n}\n')
    return cameraCode,'\n'.join(outputs)

def boundary():
    # Expose only SDK primitives and real scalar subobject references to the C producer
    header='''//+ rouz edit (ChatGPT)
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
'''
    native='//+ rouz edit (ChatGPT)\n#define WITHD3D\n#include "common.h"\n#include "Shadow_backend.h"\n'
    for result,name,args,expression in HOST:
        # Keep opaque handle forwarding separate from all allocation and rendering decisions
        declaration=result+' Shadow_Host'+name+'('+args+')';header+=declaration+';\n';native+=declaration+'\n{\n    // Forward one original SDK operation with its actual typed objects\n    '+('return ' if result!='void' else '')+expression+';\n}\n'
    extra=[
('void','ViewWindow','void *camera,const ShadowVector2 *window','RwV2d native={window->x,window->y};RwCameraSetViewWindow((RwCamera*)camera,&native);'),
('void *','MatrixVector','void *matrix,int32_t field','RwMatrix *m=(RwMatrix*)matrix;switch(field){case SHADOW_MATRIX_RIGHT:return RwMatrixGetRight(m);case SHADOW_MATRIX_UP:return RwMatrixGetUp(m);case SHADOW_MATRIX_AT:return RwMatrixGetAt(m);case SHADOW_MATRIX_POS:return RwMatrixGetPos(m);}return NULL;'),
('ShadowVectorOut','VectorOut','void *vector','RwV3d *v=(RwV3d*)vector;ShadowVectorOut value={&v->x,&v->y,&v->z};return value;'),
('void','FrameRotate','void *frame,const ShadowVector *axis,float angle,int32_t mode','RwV3d native={axis->x,axis->y,axis->z};RwFrameRotate((RwFrame*)frame,&native,angle,(RwOpCombineType)mode);')]
    for result,name,args,value in extra:
        # Translate C values and borrow SDK scalar fields without casting C records to SDK classes
        declaration=result+' Shadow_Host'+name+'('+args+')';header+=declaration+';\n';native+=declaration+'\n{\n    // Access actual SDK storage or construct a real SDK input record at the original operation\n    '+value+'\n}\n'
    header+='enum {'+','.join('SHADOW_CONST_'+name for name in CONSTANTS)+'};\nint32_t Shadow_HostConstant(int32_t identity);\n'
    native+='int32_t Shadow_HostConstant(int32_t identity)\n{\n    // Translate only SDK enum identities while all application selection remains in C\n    switch(identity){'+''.join('case SHADOW_CONST_'+name+':return '+name+';' for name in CONSTANTS)+'}return 0;\n}\n'
    header+='#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n';native+='//- rouz edit (ChatGPT)\n'
    return header,native

def prelude():
    # Retain the original assertion evaluation while keeping C policy independent of common.h
    return '''#include "Shadow_backend.h"
#include <string.h>
#define nil NULL
#ifndef MASTER
#define ASSERT(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define ASSERT(expression) (expression)
#endif
'''
