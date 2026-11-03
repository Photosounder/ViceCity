//+ rouz edit (ChatGPT)
#include "RwHelper.h"
#include "Sprite2d.h"
#include "Sprite2d_backend.h"
#include "RenderBuffer.h"
#include <stdint.h>
#define nil NULL
#define TRUE 1
#define FALSE 0
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
static float Sprite2d_RecipNearClip;
static float Sprite2d_NearScreenZ;
static float Sprite2d_NearCamZ;
static int32_t Sprite2d_nextBufferVertex;
static int32_t Sprite2d_nextBufferIndex;
void Sprite2d_Init(Sprite2d *self)
{
    // Initialize the sole instance field at the former constructor boundary
    self->m_pTexture=NULL;
}
void Sprite2d_Deinit(Sprite2d *self)
{
    // Retain the complete original destructor behavior through the C resource operation
    Sprite2d_Delete(self);
}
void *Sprite2d_GetVertices(void)
{
    // Return the same actual SDK array rather than a copied C vertex buffer
    return Sprite2d_HostVertices();
}
void Sprite2d_InitArray(Sprite2d *objects,size_t count)
{
    // Retain the original ascending array-construction order
    for(size_t i=0;i<count;++i)Sprite2d_Init(&objects[i]);
}
void Sprite2d_DeinitArray(Sprite2d *objects,size_t count)
{
    // Retain the original reverse array-destruction order
    while(count!=0)Sprite2d_Deinit(&objects[--count]);
}
void Sprite2d_SetRecipNearClip(void)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	// Used but empty in VC, instead they set in InitPerFrame. Isn't that great?

}
void Sprite2d_InitPerFrame(void)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_nextBufferVertex = 0;
	Sprite2d_nextBufferIndex = 0;
	Sprite2d_RecipNearClip = 1.0f / Sprite_HostNearClip(Sprite_HostSceneCamera());
	Sprite2d_NearScreenZ = Sprite_HostNearScreenZ();
	// not original but you're supposed to set camera z too
	// wrapping all this in FIX_BUGS is too ugly
	Sprite2d_NearCamZ = Sprite_HostNearClip(Sprite_HostSceneCamera());

}
void Sprite2d_Delete(Sprite2d *self)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	if(self->m_pTexture){
		RwHelper_DestroyTexture(self->m_pTexture);
		self->m_pTexture = nil;
	}

}
void Sprite2d_SetTexture(Sprite2d *self,const char *name)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_Delete(self);
	if(name)
		self->m_pTexture = RwHelper_ReadTexture(name, nil);

}
void Sprite2d_SetTextureMasked(Sprite2d *self,const char *name, const char *mask)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_Delete(self);
	if(name)
		self->m_pTexture = RwHelper_ReadTexture(name, mask);

}
void Sprite2d_SetAddressing(Sprite2d *self,int32_t addr)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	if(self->m_pTexture)
		Shadow_HostTextureAddress(self->m_pTexture, addr);

}
void Sprite2d_SetRenderState(Sprite2d *self)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	if(self->m_pTexture)
		RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATETEXTURERASTER), RwHelper_TextureRaster(self->m_pTexture));
	else
		RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATETEXTURERASTER), nil);

}
void Sprite2d_DrawXYWH(Sprite2d *self,float x, float y, float w, float h, SpriteColourRef col)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	CRect drawRect=Rect_Make(x, y, x + w, y + h);
	Sprite2d_SetRectVertices(&drawRect, col, col, col, col);
	Sprite2d_SetRenderState(self);
	Sprite_HostDraw(Sprite2d_HostConstant(SPRITE2D_CONST_rwPRIMTYPETRIFAN), Sprite2d_HostVertices(), 4);

}
void Sprite2d_DrawRect(Sprite2d *self,const CRect *rect, SpriteColourRef col)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_SetRectVertices(rect, col, col, col, col);
	Sprite2d_SetRenderState(self);
	Sprite_HostDraw(Sprite2d_HostConstant(SPRITE2D_CONST_rwPRIMTYPETRIFAN), Sprite2d_HostVertices(), 4);

}
void Sprite2d_DrawRectUV(Sprite2d *self,const CRect *rect, SpriteColourRef col, float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_SetRectUVVertices(rect, col, col, col, col, u0, v0, u1, v1, u3, v3, u2, v2);
	Sprite2d_SetRenderState(self);
	Sprite_HostDraw(Sprite2d_HostConstant(SPRITE2D_CONST_rwPRIMTYPETRIFAN), Sprite2d_HostVertices(), 4);

}
void Sprite2d_DrawRectColours(Sprite2d *self,const CRect *rect, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_SetRectVertices(rect, c0, c1, c2, c3);
	Sprite2d_SetRenderState(self);
	Sprite_HostDraw(Sprite2d_HostConstant(SPRITE2D_CONST_rwPRIMTYPETRIFAN), Sprite2d_HostVertices(), 4);

}
void Sprite2d_DrawQuad(Sprite2d *self,float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, SpriteColourRef col)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_SetQuadVertices(x1, y1, x2, y2, x3, y3, x4, y4, col, col, col, col);
	Sprite2d_SetRenderState(self);
	Sprite_HostDraw(Sprite2d_HostConstant(SPRITE2D_CONST_rwPRIMTYPETRIFAN), Sprite2d_HostVertices(), 4);

}
void Sprite2d_SetRectVertices(const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	float offset = 1.0f/1024.0f;

	// This is what we draw:
	// 0---1
	// | / |
	// 3---2
	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), r->left);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), r->top);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), (*c2.r), (*c2.g), (*c2.b), (*c2.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), 0.0f+offset, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), 0.0f+offset, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), r->right);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), r->top);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), (*c3.r), (*c3.g), (*c3.b), (*c3.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), 1.0f+offset, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), 0.0f+offset, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), r->right);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), r->bottom);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), (*c1.r), (*c1.g), (*c1.b), (*c1.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), 1.0f+offset, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), 1.0f+offset, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), r->left);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), r->bottom);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), (*c0.r), (*c0.g), (*c0.b), (*c0.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), 0.0f+offset, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), 1.0f+offset, Sprite2d_RecipNearClip);

}
void Sprite2d_SetRectUVVertices(const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3, float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	// This is what we draw:
	// 0---1
	// | / |
	// 3---2
	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), r->left);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), r->top);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), (*c2.r), (*c2.g), (*c2.b), (*c2.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), u0, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), v0, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), r->right);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), r->top);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), (*c3.r), (*c3.g), (*c3.b), (*c3.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), u1, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), v1, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), r->right);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), r->bottom);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), (*c1.r), (*c1.g), (*c1.b), (*c1.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), u2, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), v2, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), r->left);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), r->bottom);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), (*c0.r), (*c0.g), (*c0.b), (*c0.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), u3, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), v3, Sprite2d_RecipNearClip);

}
void Sprite2d_SetQuadVertices(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), x3);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), y3);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), (*c2.r), (*c2.g), (*c2.b), (*c2.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), 0.0f, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),0), 0.0f, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), x4);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), y4);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), (*c3.r), (*c3.g), (*c3.b), (*c3.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), 1.0f, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),1), 0.0f, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), x2);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), y2);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), (*c1.r), (*c1.g), (*c1.b), (*c1.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), 1.0f, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),2), 1.0f, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), x1);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), y1);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), (*c0.r), (*c0.g), (*c0.b), (*c0.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), 0.0f, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),3), 1.0f, Sprite2d_RecipNearClip);

}
void Sprite2d_SetArrayVertices(int n, float *positions, float *uvs, SpriteColourRef col)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	int i;

	for(i = 0; i < n; i++){
		RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), positions[i*2 + 0]);
		RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), positions[i*2 + 1]);
		RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), Sprite2d_NearScreenZ + 0.0001f);
		RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), Sprite2d_NearCamZ);
		RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), Sprite2d_RecipNearClip);
		RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), (*col.r), (*col.g), (*col.b), (*col.a));
		RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), uvs[i*2 + 0], Sprite2d_RecipNearClip);
		RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), uvs[i*2 + 1], Sprite2d_RecipNearClip);
	}

}
void Sprite2d_SetMaskVertices(int n, float *positions)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	int i;

	for(i = 0; i < n; i++){
		RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), positions[i*2 + 0]);
		RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), positions[i*2 + 1]);
		RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), Sprite2d_NearScreenZ);
		RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), Sprite2d_NearCamZ);
		RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), Sprite2d_RecipNearClip);
#if !defined(GTA_PS2_STUFF) && defined(RWLIBS)
		RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), 0, 0, 0, 0);
#else
		RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite2d_HostVertices(),i), 255, 255, 255, 255);
#endif
	}

}
void Sprite2d_SetExternalVertices(void *verts, const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3, float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,0), r->left);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,0), r->top);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,0), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,0), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,0), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,0), (*c2.r), (*c2.g), (*c2.b), (*c2.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,0), u0, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,0), v0, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,1), r->right);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,1), r->top);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,1), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,1), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,1), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,1), (*c3.r), (*c3.g), (*c3.b), (*c3.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,1), u1, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,1), v1, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,2), r->right);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,2), r->bottom);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,2), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,2), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,2), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,2), (*c1.r), (*c1.g), (*c1.b), (*c1.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,2), u2, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,2), v2, Sprite2d_RecipNearClip);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,3), r->left);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,3), r->bottom);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,3), Sprite2d_NearScreenZ);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,3), Sprite2d_NearCamZ);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,3), Sprite2d_RecipNearClip);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,3), (*c0.r), (*c0.g), (*c0.b), (*c0.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,3), u3, Sprite2d_RecipNearClip);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,3), v3, Sprite2d_RecipNearClip);

}
void Sprite2d_DrawSolidRect(const CRect *r, SpriteColourRef col)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_SetRectVertices(r, col, col, col, col);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATETEXTURERASTER), nil);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATESHADEMODE), (void*)(uintptr_t)Sprite2d_HostConstant(SPRITE2D_CONST_rwSHADEMODEFLAT));
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE), (void*)(uintptr_t)FALSE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE), (void*)(uintptr_t)FALSE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void*)(uintptr_t)((*col.a) != 255));
	Sprite_HostDraw(Sprite2d_HostConstant(SPRITE2D_CONST_rwPRIMTYPETRIFAN), Sprite2d_HostVertices(), 4);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE), (void*)(uintptr_t)TRUE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE), (void*)(uintptr_t)TRUE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATESHADEMODE), (void*)(uintptr_t)Sprite2d_HostConstant(SPRITE2D_CONST_rwSHADEMODEGOURAUD));

}
void Sprite2d_DrawFourColourRect(const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_SetRectVertices(r, c0, c1, c2, c3);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATETEXTURERASTER), nil);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE), (void*)(uintptr_t)FALSE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE), (void*)(uintptr_t)FALSE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void*)(uintptr_t)FALSE);
	Sprite_HostDraw(Sprite2d_HostConstant(SPRITE2D_CONST_rwPRIMTYPETRIFAN), Sprite2d_HostVertices(), 4);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE), (void*)(uintptr_t)TRUE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE), (void*)(uintptr_t)TRUE);

}
void Sprite2d_DrawRectXLU(const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_SetRectVertices(r, c0, c1, c2, c3);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATETEXTURERASTER), nil);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE), (void*)(uintptr_t)FALSE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE), (void*)(uintptr_t)FALSE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void*)(uintptr_t)TRUE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATESRCBLEND), (void*)(uintptr_t)Sprite2d_HostConstant(SPRITE2D_CONST_rwBLENDSRCALPHA));
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEDESTBLEND), (void*)(uintptr_t)Sprite2d_HostConstant(SPRITE2D_CONST_rwBLENDINVSRCALPHA));
	Sprite_HostDraw(Sprite2d_HostConstant(SPRITE2D_CONST_rwPRIMTYPETRIFAN), Sprite2d_HostVertices(), 4);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE), (void*)(uintptr_t)TRUE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE), (void*)(uintptr_t)TRUE);

}
void Sprite2d_DrawAnyRect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_SetQuadVertices(x1, y1, x2, y2, x3, y3, x4, y4, c0, c1, c2, c3);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATETEXTURERASTER), nil);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATESHADEMODE), (void*)(uintptr_t)Sprite2d_HostConstant(SPRITE2D_CONST_rwSHADEMODEGOURAUD));
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE), (void*)(uintptr_t)FALSE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE), (void*)(uintptr_t)FALSE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void*)(uintptr_t)((*c0.a) != 255 || (*c1.a) != 255 || (*c2.a) != 255 || (*c3.a) != 255));
	Sprite_HostDraw(Sprite2d_HostConstant(SPRITE2D_CONST_rwPRIMTYPETRIFAN), Sprite2d_HostVertices(), 4);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE), (void*)(uintptr_t)TRUE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE), (void*)(uintptr_t)TRUE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATESHADEMODE), (void*)(uintptr_t)Sprite2d_HostConstant(SPRITE2D_CONST_rwSHADEMODEGOURAUD));

}
void Sprite2d_Draw2DPolygon(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, SpriteColourRef color)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_SetQuadVertices(x1, y1, x2, y2, x3, y3, x4, y4, color, color, color, color);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATETEXTURERASTER), 0);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATESHADEMODE), (void*)(uintptr_t)Sprite2d_HostConstant(SPRITE2D_CONST_rwSHADEMODEFLAT));
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE), (void*)(uintptr_t)FALSE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE), (void*)(uintptr_t)FALSE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void*)(uintptr_t)((*color.a) != 255));
	Sprite_HostDraw(Sprite2d_HostConstant(SPRITE2D_CONST_rwPRIMTYPETRIFAN), Sprite2d_HostVertices(), 4);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZTESTENABLE), (void*)(uintptr_t)TRUE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATEZWRITEENABLE), (void*)(uintptr_t)TRUE);
	RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATESHADEMODE), (void*)(uintptr_t)Sprite2d_HostConstant(SPRITE2D_CONST_rwSHADEMODEGOURAUD));

}
void Sprite2d_AddToBuffer(const CRect *r, SpriteColourRef c, float u0, float v0, float u1, float v1, float u3, float v3, float u2, float v2)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	Sprite2d_SetExternalVertices(RwHelper_HostVertexAt(Sprite2d_HostTempVertices(),Sprite2d_nextBufferVertex), r, c, c, c, c, u0, v0, u1, v1, u3, v3, u2, v2);
	uint16_t *pIndexList = &TempBufferRenderIndexList[Sprite2d_nextBufferIndex];
	pIndexList[0] = Sprite2d_nextBufferVertex;
	pIndexList[1] = Sprite2d_nextBufferVertex + 1;
	pIndexList[2] = Sprite2d_nextBufferVertex + 2;
	pIndexList[3] = Sprite2d_nextBufferVertex + 3;
	pIndexList[4] = Sprite2d_nextBufferVertex;
	pIndexList[5] = Sprite2d_nextBufferVertex + 2;
	Sprite2d_nextBufferIndex += 6;
	Sprite2d_nextBufferVertex += 4;
	if (Sprite2d_IsVertexBufferFull())
		Sprite2d_RenderVertexBuffer();

}
bool Sprite2d_IsVertexBufferFull(void)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	return (Sprite2d_nextBufferVertex > TEMPBUFFERVERTSIZE-128-4 || (size_t)Sprite2d_nextBufferIndex > ARRAY_SIZE(TempBufferRenderIndexList)-6);

}
void Sprite2d_RenderVertexBuffer(void)
{
    // Execute the complete original 2D sprite algorithm with canonical C instance and private state

	if (Sprite2d_nextBufferVertex > 0) {
		RwHelper_HostRenderState(Sprite2d_HostConstant(SPRITE2D_CONST_rwRENDERSTATETEXTUREFILTER), (void*)(uintptr_t)Sprite2d_HostConstant(SPRITE2D_CONST_rwFILTERLINEAR));
		Sprite2d_HostIndexedDraw(Sprite2d_HostConstant(SPRITE2D_CONST_rwPRIMTYPETRILIST), Sprite2d_HostTempVertices(), Sprite2d_nextBufferVertex, TempBufferRenderIndexList, Sprite2d_nextBufferIndex);
		Sprite2d_nextBufferVertex = 0;
		Sprite2d_nextBufferIndex = 0;
	}

}
//- rouz edit (ChatGPT)
