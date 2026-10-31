//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "Rect.h"
typedef struct SpriteVector {float x,y,z;} SpriteVector;
typedef struct SpriteVectorRef {const float *x,*y,*z;} SpriteVectorRef;
typedef struct SpriteVectorOut {float *x,*y,*z;} SpriteVectorOut;
typedef struct SpriteColour {uint8_t r,g,b,a;} SpriteColour;
typedef struct SpriteColourRef {const uint8_t *r,*g,*b,*a;} SpriteColourRef;
static inline SpriteVectorRef Sprite_VectorRef(const float *x,const float *y,const float *z)
{
    // Borrow actual live vector scalar subobjects for the complete synchronous C operation
    SpriteVectorRef value={x,y,z};return value;
}
static inline SpriteVectorOut Sprite_VectorOut(float *x,float *y,float *z)
{
    // Publish directly into the caller's actual scalar receiver fields
    SpriteVectorOut value={x,y,z};return value;
}
static inline SpriteColourRef Sprite_ColourRef(const uint8_t *r,const uint8_t *g,const uint8_t *b,const uint8_t *a)
{
    // Retain the original live colour component reads across intervening SDK vertex setters
    SpriteColourRef value={r,g,b,a};return value;
}
#ifdef __cplusplus
extern "C" {
#endif
float Sprite_GetNearScreenZ(void);
float Sprite_GetFarScreenZ(void);
float Sprite_CalcHorizonCoors(void);
bool Sprite_CalcScreenCoors(SpriteVectorRef in, SpriteVectorOut out, float *outw, float *outh, bool farclip);
void Sprite_InitSpriteBuffer(void);
void Sprite_InitSpriteBuffer2D(void);
void Sprite_FlushSpriteBuffer(void);
void Sprite_RenderOneXLUSprite(float x, float y, float z, float w, float h, uint8_t r, uint8_t g, uint8_t b, int16_t intens, float recipz, uint8_t a);
void Sprite_RenderOneXLUSprite_Rotate_Aspect(float x, float y, float z, float w, float h, uint8_t r, uint8_t g, uint8_t b, int16_t intens, float recipz, float rotation, uint8_t a);
void Sprite_RenderBufferedOneXLUSprite(float x, float y, float z, float w, float h, uint8_t r, uint8_t g, uint8_t b, int16_t intens, float recipz, uint8_t a);
void Sprite_RenderBufferedOneXLUSprite_Rotate_Dimension(float x, float y, float z, float w, float h, uint8_t r, uint8_t g, uint8_t b, int16_t intens, float recipz, float rotation, uint8_t a);
void Sprite_RenderBufferedOneXLUSprite_Rotate_Aspect(float x, float y, float z, float w, float h, uint8_t r, uint8_t g, uint8_t b, int16_t intens, float recipz, float rotation, uint8_t a);
void Sprite_RenderBufferedOneXLUSprite_Rotate_2Colours(float x, float y, float z, float w, float h, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, float cx, float cy, float recipz, float rotation, uint8_t a);
void Sprite_Set6Vertices2DRect(void *verts, const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3);
void Sprite_Set6Vertices2DCoords(void *verts, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3);
void Sprite_RenderBufferedOneXLUSprite2D(float x, float y, float w, float h, SpriteColourRef colour, int16_t intens, uint8_t alpha);
void Sprite_RenderBufferedOneXLUSprite2D_Rotate_Dimension(float x, float y, float w, float h, SpriteColourRef colour, int16_t intens, float rotation, uint8_t alpha);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
