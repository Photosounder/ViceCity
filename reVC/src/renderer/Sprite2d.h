//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include "Sprite.h"
typedef struct Sprite2d {void *m_pTexture;} Sprite2d;
#ifdef __cplusplus
extern "C" {
#endif
void Sprite2d_Init(Sprite2d *self);
void Sprite2d_Deinit(Sprite2d *self);
void *Sprite2d_GetVertices(void);
void Sprite2d_InitArray(Sprite2d *objects,size_t count);
void Sprite2d_DeinitArray(Sprite2d *objects,size_t count);
void Sprite2d_SetRecipNearClip(void);
void Sprite2d_InitPerFrame(void);
void Sprite2d_Delete(Sprite2d *self);
void Sprite2d_SetTexture(Sprite2d *self,const char *name);
void Sprite2d_SetTextureMasked(Sprite2d *self,const char *name, const char *mask);
void Sprite2d_SetAddressing(Sprite2d *self,int32_t addr);
void Sprite2d_SetRenderState(Sprite2d *self);
void Sprite2d_DrawXYWH(Sprite2d *self,float x, float y, float w, float h, SpriteColourRef col);
void Sprite2d_DrawRect(Sprite2d *self,const CRect *rect, SpriteColourRef col);
void Sprite2d_DrawRectUV(Sprite2d *self,const CRect *rect, SpriteColourRef col, float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2);
void Sprite2d_DrawRectColours(Sprite2d *self,const CRect *rect, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3);
void Sprite2d_DrawQuad(Sprite2d *self,float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, SpriteColourRef col);
void Sprite2d_SetRectVertices(const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3);
void Sprite2d_SetRectUVVertices(const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3, float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2);
void Sprite2d_SetQuadVertices(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3);
void Sprite2d_SetArrayVertices(int n, float *positions, float *uvs, SpriteColourRef col);
void Sprite2d_SetMaskVertices(int n, float *positions);
void Sprite2d_SetExternalVertices(void *verts, const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3, float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2);
void Sprite2d_DrawSolidRect(const CRect *r, SpriteColourRef col);
void Sprite2d_DrawFourColourRect(const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3);
void Sprite2d_DrawRectXLU(const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3);
void Sprite2d_DrawAnyRect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3);
void Sprite2d_Draw2DPolygon(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, SpriteColourRef color);
void Sprite2d_AddToBuffer(const CRect *r, SpriteColourRef c, float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2);
bool Sprite2d_IsVertexBufferFull(void);
void Sprite2d_RenderVertexBuffer(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
