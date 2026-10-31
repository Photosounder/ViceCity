"""Describe actual SDK storage and individual native primitives for the C sprite owner."""
HOST=[('int32_t','ScreenWidth','void','return RsGlobal.width;'),('int32_t','ScreenHeight','void','return RsGlobal.height;'),('float','FrontX','void','return TheCamera.CamFrontXNorm;'),('float','FrontY','void','return TheCamera.CamFrontYNorm;'),('float','DefaultFOV','void','return DefaultFOV;'),('void *','SceneCamera','void','return Scene.camera;'),('float','NearClip','void *camera','return RwCameraGetNearClipPlane((RwCamera*)camera);'),('float','NearScreenZ','void','return RwIm2DGetNearScreenZ();'),('float','FarScreenZ','void','return RwIm2DGetFarScreenZ();'),('void *','BufferedVertices','void','return spriteBufferedVertices;'),('void *','ImmediateVertices','void','return spriteImmediateVertices;'),('void','Draw','int32_t kind,void *vertices,int32_t count','RwIm2DRenderPrimitive((RwPrimitiveType)kind,(RwIm2DVertex*)vertices,count);')]
CONSTANTS=['rwPRIMTYPETRIFAN','rwPRIMTYPETRILIST','rwRENDERSTATEZTESTENABLE']


def build():
    # Expose actual SDK buffers and leaf operations while every clipping and rendering decision stays in C
    header='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Sprite.h"\n#include "RwHelper_backend.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    native='//+ rouz edit (ChatGPT)\n#include "common.h"\n#include "main.h"\n#include "Camera.h"\n#include "Sprite_backend.h"\n#include <string.h>\n#ifdef REVC_SOFTWARE_POLYGONS\n#include "SoftwarePolygons.h"\n#endif\nstatic RwIm2DVertex spriteBufferedVertices[64*6];\nstatic RwIm2DVertex spriteImmediateVertices[4];\n'
    for kind,name,args,expression in HOST:
        # Keep one original SDK or independently owned field operation in each typed leaf
        declaration=kind+' Sprite_Host'+name+'('+args+')';header+=declaration+';\n';native+=declaration+'\n{\n    // Forward the original primitive using the actual native object or scalar field\n    '+expression+'\n}\n'
    header+='SpriteVectorRef Sprite_HostCameraPosition(void);\nSpriteVector Sprite_HostViewTransform(const SpriteVector *source);\n'
    native+="""SpriteVectorRef Sprite_HostCameraPosition(void)
{
    // Borrow actual native position fields without copying the camera or treating a C record as its vector class
    const CVector &position=TheCamera.GetPosition();return Sprite_VectorRef(&position.x,&position.y,&position.z);
}
SpriteVector Sprite_HostViewTransform(const SpriteVector *source)
{
    // Invoke the separately owned matrix-vector primitive using actual native vector objects
    CVector input;memcpy(&input.x,&source->x,sizeof(float));memcpy(&input.y,&source->y,sizeof(float));memcpy(&input.z,&source->z,sizeof(float));CVector transformed=TheCamera.m_viewMatrix*input;SpriteVector result;memcpy(&result.x,&transformed.x,sizeof(float));memcpy(&result.y,&transformed.y,sizeof(float));memcpy(&result.z,&transformed.z,sizeof(float));return result;
}
"""
    header+='#ifdef REVC_SOFTWARE_POLYGONS\nbool Sprite_HostCapturingWorldEffects(void);\n#endif\n'
    native+='#ifdef REVC_SOFTWARE_POLYGONS\nbool Sprite_HostCapturingWorldEffects(void)\n{\n    // Query the unchanged software owner at the original projection branch\n    return SoftwarePolygons::CapturingWorldEffects();\n}\n#endif\n'
    header+='enum {'+','.join('SPRITE_CONST_'+v for v in CONSTANTS)+'};\nint32_t Sprite_HostConstant(int32_t identity);\n'
    native+='int32_t Sprite_HostConstant(int32_t identity)\n{\n    // Translate SDK identities while all primitive and render-state choices remain in C\n    switch(identity){'+''.join('case SPRITE_CONST_'+v+':return '+v+';' for v in CONSTANTS)+'}return 0;\n}\n'
    header+='#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n';native+='//- rouz edit (ChatGPT)\n';return header,native
