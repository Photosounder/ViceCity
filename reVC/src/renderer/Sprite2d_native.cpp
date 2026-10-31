//+ rouz edit (ChatGPT)
#include "common.h"
#include "Sprite2d_backend.h"
#include "RenderBuffer.h"
static RwIm2DVertex sprite2dVertices[8];
void * Sprite2d_HostVertices(void)
{
    // Forward the original SDK primitive using its actual native storage
    return sprite2dVertices;
}
void * Sprite2d_HostTempVertices(void)
{
    // Forward the original SDK primitive using its actual native storage
    return TempVertexBuffer.im2d;
}
void * Sprite2d_HostTextureRead(const char *name,const char *mask)
{
    // Forward the original SDK primitive using its actual native storage
    return RwTextureRead(name,mask);
}
void * Sprite2d_HostTextureRaster(void *texture)
{
    // Forward the original SDK primitive using its actual native storage
    return RwTextureGetRaster((RwTexture*)texture);
}
void Sprite2d_HostIndexedDraw(int32_t kind,void *vertices,int32_t count,uint16_t *indices,int32_t indexCount)
{
    // Forward the original SDK primitive using its actual native storage
    RwIm2DRenderIndexedPrimitive((RwPrimitiveType)kind,(RwIm2DVertex*)vertices,count,indices,indexCount);
}
int32_t Sprite2d_HostConstant(int32_t identity)
{
    // Translate SDK identities while retaining every draw and state choice in C
    switch(identity){case SPRITE2D_CONST_rwBLENDINVSRCALPHA:return rwBLENDINVSRCALPHA;case SPRITE2D_CONST_rwBLENDSRCALPHA:return rwBLENDSRCALPHA;case SPRITE2D_CONST_rwFILTERLINEAR:return rwFILTERLINEAR;case SPRITE2D_CONST_rwPRIMTYPETRIFAN:return rwPRIMTYPETRIFAN;case SPRITE2D_CONST_rwPRIMTYPETRILIST:return rwPRIMTYPETRILIST;case SPRITE2D_CONST_rwRENDERSTATEDESTBLEND:return rwRENDERSTATEDESTBLEND;case SPRITE2D_CONST_rwRENDERSTATESHADEMODE:return rwRENDERSTATESHADEMODE;case SPRITE2D_CONST_rwRENDERSTATESRCBLEND:return rwRENDERSTATESRCBLEND;case SPRITE2D_CONST_rwRENDERSTATETEXTUREFILTER:return rwRENDERSTATETEXTUREFILTER;case SPRITE2D_CONST_rwRENDERSTATETEXTURERASTER:return rwRENDERSTATETEXTURERASTER;case SPRITE2D_CONST_rwRENDERSTATEVERTEXALPHAENABLE:return rwRENDERSTATEVERTEXALPHAENABLE;case SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE:return rwRENDERSTATEZTESTENABLE;case SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE:return rwRENDERSTATEZWRITEENABLE;case SPRITE2D_CONST_rwSHADEMODEFLAT:return rwSHADEMODEFLAT;case SPRITE2D_CONST_rwSHADEMODEGOURAUD:return rwSHADEMODEGOURAUD;}return 0;
}
//- rouz edit (ChatGPT)
