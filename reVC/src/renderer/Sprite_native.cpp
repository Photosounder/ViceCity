//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Camera.h"
#include "Sprite_backend.h"
#include <string.h>
#ifdef REVC_SOFTWARE_POLYGONS
#include "SoftwarePolygons.h"
#endif
static RwIm2DVertex spriteBufferedVertices[64*6];
static RwIm2DVertex spriteImmediateVertices[4];
int32_t Sprite_HostScreenWidth(void)
{
    // Forward the original primitive using the actual native object or scalar field
    return RsGlobal.width;
}
int32_t Sprite_HostScreenHeight(void)
{
    // Forward the original primitive using the actual native object or scalar field
    return RsGlobal.height;
}
float Sprite_HostFrontX(void)
{
    // Forward the original primitive using the actual native object or scalar field
    return TheCamera.CamFrontXNorm;
}
float Sprite_HostFrontY(void)
{
    // Forward the original primitive using the actual native object or scalar field
    return TheCamera.CamFrontYNorm;
}
float Sprite_HostDefaultFOV(void)
{
    // Forward the original primitive using the actual native object or scalar field
    return DefaultFOV;
}
void * Sprite_HostSceneCamera(void)
{
    // Forward the original primitive using the actual native object or scalar field
    return Scene.camera;
}
float Sprite_HostNearClip(void *camera)
{
    // Forward the original primitive using the actual native object or scalar field
    return RwCameraGetNearClipPlane((RwCamera*)camera);
}
float Sprite_HostNearScreenZ(void)
{
    // Forward the original primitive using the actual native object or scalar field
    return RwIm2DGetNearScreenZ();
}
float Sprite_HostFarScreenZ(void)
{
    // Forward the original primitive using the actual native object or scalar field
    return RwIm2DGetFarScreenZ();
}
void * Sprite_HostBufferedVertices(void)
{
    // Forward the original primitive using the actual native object or scalar field
    return spriteBufferedVertices;
}
void * Sprite_HostImmediateVertices(void)
{
    // Forward the original primitive using the actual native object or scalar field
    return spriteImmediateVertices;
}
void Sprite_HostDraw(int32_t kind,void *vertices,int32_t count)
{
    // Forward the original primitive using the actual native object or scalar field
    RwIm2DRenderPrimitive((RwPrimitiveType)kind,(RwIm2DVertex*)vertices,count);
}
SpriteVectorRef Sprite_HostCameraPosition(void)
{
    // Borrow actual native position fields without copying the camera or treating a C record as its vector class
    const CVector &position=TheCamera.GetPosition();return Sprite_VectorRef(&position.x,&position.y,&position.z);
}
SpriteVector Sprite_HostViewTransform(const SpriteVector *source)
{
    // Invoke the separately owned matrix-vector primitive using actual native vector objects
    CVector input;memcpy(&input.x,&source->x,sizeof(float));memcpy(&input.y,&source->y,sizeof(float));memcpy(&input.z,&source->z,sizeof(float));CVector transformed=TheCamera.m_viewMatrix*input;SpriteVector result;memcpy(&result.x,&transformed.x,sizeof(float));memcpy(&result.y,&transformed.y,sizeof(float));memcpy(&result.z,&transformed.z,sizeof(float));return result;
}
#ifdef REVC_SOFTWARE_POLYGONS
bool Sprite_HostCapturingWorldEffects(void)
{
    // Query the unchanged software owner at the original projection branch
    return SoftwarePolygons::CapturingWorldEffects();
}
#endif
int32_t Sprite_HostConstant(int32_t identity)
{
    // Translate SDK identities while all primitive and render-state choices remain in C
    switch(identity){case SPRITE_CONST_rwPRIMTYPETRIFAN:return rwPRIMTYPETRIFAN;case SPRITE_CONST_rwPRIMTYPETRILIST:return rwPRIMTYPETRILIST;case SPRITE_CONST_rwRENDERSTATEZTESTENABLE:return rwRENDERSTATEZTESTENABLE;}return 0;
}
//- rouz edit (ChatGPT)
