//+ rouz edit (ChatGPT)
#pragma once
#include "Sprite2d.h"
#include "Sprite_backend.h"
#include "Shadow_backend.h"
#ifdef __cplusplus
extern "C" {
#endif
void * Sprite2d_HostVertices(void);
void * Sprite2d_HostTempVertices(void);
void * Sprite2d_HostTextureRead(const char *name,const char *mask);
void * Sprite2d_HostTextureRaster(void *texture);
void Sprite2d_HostIndexedDraw(int32_t kind,void *vertices,int32_t count,uint16_t *indices,int32_t indexCount);
enum {SPRITE2D_CONST_rwBLENDINVSRCALPHA,SPRITE2D_CONST_rwBLENDSRCALPHA,SPRITE2D_CONST_rwFILTERLINEAR,SPRITE2D_CONST_rwPRIMTYPETRIFAN,SPRITE2D_CONST_rwPRIMTYPETRILIST,SPRITE2D_CONST_rwRENDERSTATEDESTBLEND,SPRITE2D_CONST_rwRENDERSTATESHADEMODE,SPRITE2D_CONST_rwRENDERSTATESRCBLEND,SPRITE2D_CONST_rwRENDERSTATETEXTUREFILTER,SPRITE2D_CONST_rwRENDERSTATETEXTURERASTER,SPRITE2D_CONST_rwRENDERSTATEVERTEXALPHAENABLE,SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE,SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE,SPRITE2D_CONST_rwSHADEMODEFLAT,SPRITE2D_CONST_rwSHADEMODEGOURAUD};
int32_t Sprite2d_HostConstant(int32_t identity);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
