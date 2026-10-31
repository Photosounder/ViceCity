//+ rouz edit (ChatGPT)
#include "Sprite.h"
#include "Sprite_backend.h"
#include "Draw.h"
#include "Draw_constants.h"
#include <math.h>
#include <string.h>
#define SPRITEBUFFERSIZE 64
#define TRUE 1
#define FALSE 0
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))
#define Sin(x) sinf(x)
#define Cos(x) cosf(x)
#ifdef GTA_PS2
#ifdef GTA_PAL
#define SCREEN_WIDTH ((float)640)
#define SCREEN_HEIGHT ((float)512)
#else
#define SCREEN_WIDTH ((float)640)
#define SCREEN_HEIGHT ((float)448)
#endif
#else
#define SCREEN_WIDTH ((float)Sprite_HostScreenWidth())
#define SCREEN_HEIGHT ((float)Sprite_HostScreenHeight())
#endif
#ifdef ASPECT_RATIO_SCALE
#define SCREEN_SCALE_AR(a) ((a)*DEFAULT_ASPECT_RATIO/Draw_GetAspectRatio())
#else
#define SCREEN_SCALE_AR(a) (a)
#endif
static float Sprite_m_f2DNearScreenZ;
static float Sprite_m_f2DFarScreenZ;
static float Sprite_m_fRecipNearClipPlane;
static int32_t Sprite_m_bFlushSpriteBufferSwitchZTest;
static int32_t Sprite_nSpriteBufferIndex;
float Sprite_GetNearScreenZ(void)
{
    // Retain the original near-depth getter on the canonical C state
    return Sprite_m_f2DNearScreenZ;
}
float Sprite_GetFarScreenZ(void)
{
    // Retain the original far-depth getter on the canonical C state
    return Sprite_m_f2DFarScreenZ;
}
float Sprite_CalcHorizonCoors(void)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	SpriteVectorRef position=Sprite_HostCameraPosition();
	SpriteVector p={*position.x+Sprite_HostFrontX()*3000.0f,*position.y+Sprite_HostFrontY()*3000.0f,*position.z+0.0f*3000.0f};
	p.z = 0.0f;
	p = Sprite_HostViewTransform(&p);
	return p.y * SCREEN_HEIGHT / p.z;

}
bool Sprite_CalcScreenCoors(SpriteVectorRef in, SpriteVectorOut out, float *outw, float *outh, bool farclip)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	SpriteVector input={*in.x,*in.y,*in.z};
	SpriteVector viewvec=Sprite_HostViewTransform(&input);
	memcpy(out.x,&viewvec.x,sizeof(float));memcpy(out.y,&viewvec.y,sizeof(float));memcpy(out.z,&viewvec.z,sizeof(float));
	if((*out.z) <= Draw_GetNearClipZ() + 1.0f) return false;
	if((*out.z) >= Draw_GetFarClipZ() && farclip) return false;
	float recip = 1.0f/(*out.z);
	(*out.x) *= SCREEN_WIDTH * recip;
	(*out.y) *= SCREEN_HEIGHT * recip;
	// Match projected world sprites to the software framebuffer's screen Y direction
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	if(Sprite_HostCapturingWorldEffects())
		(*out.y) = SCREEN_HEIGHT - (*out.y);
#endif
	//- rouz edit (ChatGPT)
	const float fov = Sprite_HostDefaultFOV();
	// this is used to scale correctly if you zoom in with sniper rifle
	float fovScale = fov / Draw_GetFOV();

#ifdef FIX_SPRITES
	*outw = Draw_ms_bFixSprites ? (fovScale * recip * SCREEN_HEIGHT) : (fovScale * SCREEN_SCALE_AR(recip) * SCREEN_WIDTH);
#else
	*outw = fovScale * SCREEN_SCALE_AR(recip) * SCREEN_WIDTH;
#endif
	*outh = fovScale * recip * SCREEN_HEIGHT;

	return true;

}
void Sprite_InitSpriteBuffer(void)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	Sprite_m_f2DNearScreenZ = Sprite_HostNearScreenZ();
	Sprite_m_f2DFarScreenZ = Sprite_HostFarScreenZ();

}
void Sprite_InitSpriteBuffer2D(void)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	Sprite_m_fRecipNearClipPlane = 1.0f / Sprite_HostNearClip(Sprite_HostSceneCamera());
	Sprite_InitSpriteBuffer();

}
void Sprite_FlushSpriteBuffer(void)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	if(Sprite_nSpriteBufferIndex > 0){
		if(Sprite_m_bFlushSpriteBufferSwitchZTest){
			RwHelper_HostRenderState(Sprite_HostConstant(SPRITE_CONST_rwRENDERSTATEZTESTENABLE), (void*)FALSE);
			Sprite_HostDraw(Sprite_HostConstant(SPRITE_CONST_rwPRIMTYPETRILIST), Sprite_HostBufferedVertices(), Sprite_nSpriteBufferIndex*6);
			RwHelper_HostRenderState(Sprite_HostConstant(SPRITE_CONST_rwRENDERSTATEZTESTENABLE), (void*)TRUE);
		}else
			Sprite_HostDraw(Sprite_HostConstant(SPRITE_CONST_rwPRIMTYPETRILIST), Sprite_HostBufferedVertices(), Sprite_nSpriteBufferIndex*6);
		Sprite_nSpriteBufferIndex = 0;
	}

}
void Sprite_RenderOneXLUSprite(float x, float y, float z, float w, float h, uint8_t r, uint8_t g, uint8_t b, int16_t intens, float recipz, uint8_t a)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	static short indices[] = { 0, 1, 2, 3 };
	// Retain the original unused static table without introducing an observable operation
	(void)indices;
	// 0---3
	// |   |
	// 1---2
	float xs[4];
	float ys[4];
	float us[4];
	float vs[4];
	int i;

	xs[0] = x-w;	us[0] = 0.0f;
	xs[1] = x-w;	us[1] = 0.0f;
	xs[2] = x+w;	us[2] = 1.0f;
	xs[3] = x+w;	us[3] = 1.0f;

	ys[0] = y-h;	vs[0] = 0.0f;
	ys[1] = y+h;	vs[1] = 1.0f;
	ys[2] = y+h;	vs[2] = 1.0f;
	ys[3] = y-h;	vs[3] = 0.0f;

	// clip
	for(i = 0; i < 4; i++){
		if(xs[i] < 0.0f){
			us[i] = -xs[i] / (2.0f*w);
			xs[i] = 0.0f;
		}
		if(xs[i] > SCREEN_WIDTH){
			us[i] = 1.0f - (xs[i]-SCREEN_WIDTH) / (2.0f*w);
			xs[i] = SCREEN_WIDTH;
		}
		if(ys[i] < 0.0f){
			vs[i] = -ys[i] / (2.0f*h);
			ys[i] = 0.0f;
		}
		if(ys[i] > SCREEN_HEIGHT){
			vs[i] = 1.0f - (ys[i]-SCREEN_HEIGHT) / (2.0f*h);
			ys[i] = SCREEN_HEIGHT;
		}
	}

	// (DrawZ - DrawNear)/(DrawFar - DrawNear) = (SpriteZ-SpriteNear)/(SpriteFar-SpriteNear)
	// So to calculate SpriteZ:
	float screenz = Sprite_m_f2DNearScreenZ +
		(z-Draw_GetNearClipZ())*(Sprite_m_f2DFarScreenZ-Sprite_m_f2DNearScreenZ)*Draw_GetFarClipZ() /
		((Draw_GetFarClipZ()-Draw_GetNearClipZ())*z);

	for(i = 0; i < 4; i++){
		RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), xs[i]);
		RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), ys[i]);
		RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), screenz);
		RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), z);
		RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), recipz);
		RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), r*intens>>8, g*intens>>8, b*intens>>8, a);
		RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), us[i], recipz);
		RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), vs[i], recipz);
	}
	Sprite_HostDraw(Sprite_HostConstant(SPRITE_CONST_rwPRIMTYPETRIFAN), Sprite_HostImmediateVertices(), 4);

}
void Sprite_RenderOneXLUSprite_Rotate_Aspect(float x, float y, float z, float w, float h, uint8_t r, uint8_t g, uint8_t b, int16_t intens, float recipz, float rotation, uint8_t a)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	float c = Cos(rotation);
	float s = Sin(rotation);

	float xs[4];
	float ys[4];
	float us[4];
	float vs[4];
	int i;

	// Fade out when too near
	// why not in buffered version?
	if(z < 2.3f){
		if(z < 1.3f)
			return;
		int f = (z - 1.3f)/(2.3f-1.3f) * 255;
		r = f*r >> 8;
		g = f*g >> 8;
		b = f*b >> 8;
		intens = f*intens >> 8;
	}

	xs[0] = x + w*(-c-s);	us[0] = 0.0f;
	xs[1] = x + w*(-c+s);	us[1] = 0.0f;
	xs[2] = x + w*(+c+s);	us[2] = 1.0f;
	xs[3] = x + w*(+c-s);	us[3] = 1.0f;

	ys[0] = y + h*(-c+s);	vs[0] = 0.0f;
	ys[1] = y + h*(+c+s);	vs[1] = 1.0f;
	ys[2] = y + h*(+c-s);	vs[2] = 1.0f;
	ys[3] = y + h*(-c-s);	vs[3] = 0.0f;

	// No clipping, just culling
	if(xs[0] < 0.0f && xs[1] < 0.0f && xs[2] < 0.0f && xs[3] < 0.0f) return;
	if(ys[0] < 0.0f && ys[1] < 0.0f && ys[2] < 0.0f && ys[3] < 0.0f) return;
	if(xs[0] > SCREEN_WIDTH && xs[1] > SCREEN_WIDTH &&
	   xs[2] > SCREEN_WIDTH && xs[3] > SCREEN_WIDTH) return;
	if(ys[0] > SCREEN_HEIGHT && ys[1] > SCREEN_HEIGHT &&
	   ys[2] > SCREEN_HEIGHT && ys[3] > SCREEN_HEIGHT) return;

	float screenz = Sprite_m_f2DNearScreenZ +
		(z-Draw_GetNearClipZ())*(Sprite_m_f2DFarScreenZ-Sprite_m_f2DNearScreenZ)*Draw_GetFarClipZ() /
		((Draw_GetFarClipZ()-Draw_GetNearClipZ())*z);

	for(i = 0; i < 4; i++){
		RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), xs[i]);
		RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), ys[i]);
		RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), screenz);
		RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), z);
		RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), recipz);
		RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), r*intens>>8, g*intens>>8, b*intens>>8, a);
		RwHelper_HostVertexU(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), us[i], recipz);
		RwHelper_HostVertexV(RwHelper_HostVertexAt(Sprite_HostImmediateVertices(),i), vs[i], recipz);
	}
	Sprite_HostDraw(Sprite_HostConstant(SPRITE_CONST_rwPRIMTYPETRIFAN), Sprite_HostImmediateVertices(), 4);

}
void Sprite_RenderBufferedOneXLUSprite(float x, float y, float z, float w, float h, uint8_t r, uint8_t g, uint8_t b, int16_t intens, float recipz, uint8_t a)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	Sprite_m_bFlushSpriteBufferSwitchZTest = 0;

	// 0---3
	// |   |
	// 1---2
	float xs[4];
	float ys[4];
	float us[4];
	float vs[4];
	int i;

	xs[0] = x-w;	us[0] = 0.0f;
	xs[1] = x-w;	us[1] = 0.0f;
	xs[2] = x+w;	us[2] = 1.0f;
	xs[3] = x+w;	us[3] = 1.0f;

	ys[0] = y-h;	vs[0] = 0.0f;
	ys[1] = y+h;	vs[1] = 1.0f;
	ys[2] = y+h;	vs[2] = 1.0f;
	ys[3] = y-h;	vs[3] = 0.0f;

	// clip
	for(i = 0; i < 4; i++){
		if(xs[i] < 0.0f){
			us[i] = -xs[i] / (2.0f*w);
			xs[i] = 0.0f;
		}
		if(xs[i] > SCREEN_WIDTH){
			us[i] = 1.0f - (xs[i]-SCREEN_WIDTH) / (2.0f*w);
			xs[i] = SCREEN_WIDTH;
		}
		if(ys[i] < 0.0f){
			vs[i] = -ys[i] / (2.0f*h);
			ys[i] = 0.0f;
		}
		if(ys[i] > SCREEN_HEIGHT){
			vs[i] = 1.0f - (ys[i]-SCREEN_HEIGHT) / (2.0f*h);
			ys[i] = SCREEN_HEIGHT;
		}
	}

	float screenz = Sprite_m_f2DNearScreenZ +
		(z-Draw_GetNearClipZ())*(Sprite_m_f2DFarScreenZ-Sprite_m_f2DNearScreenZ)*Draw_GetFarClipZ() /
		((Draw_GetFarClipZ()-Draw_GetNearClipZ())*z);

	void *vert = RwHelper_HostVertexAt(Sprite_HostBufferedVertices(),Sprite_nSpriteBufferIndex*6);
	static int indices[6] = { 0, 1, 2, 3, 0, 2 };
	for(i = 0; i < 6; i++){
		RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(vert,i), xs[indices[i]]);
		RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(vert,i), ys[indices[i]]);
		RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(vert,i), screenz);
		RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(vert,i), z);
		RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(vert,i), recipz);
		RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(vert,i), r*intens>>8, g*intens>>8, b*intens>>8, a);
		RwHelper_HostVertexU(RwHelper_HostVertexAt(vert,i), us[indices[i]], recipz);
		RwHelper_HostVertexV(RwHelper_HostVertexAt(vert,i), vs[indices[i]], recipz);
	}
	Sprite_nSpriteBufferIndex++;
	if(Sprite_nSpriteBufferIndex >= SPRITEBUFFERSIZE)
		Sprite_FlushSpriteBuffer();

}
void Sprite_RenderBufferedOneXLUSprite_Rotate_Dimension(float x, float y, float z, float w, float h, uint8_t r, uint8_t g, uint8_t b, int16_t intens, float recipz, float rotation, uint8_t a)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	Sprite_m_bFlushSpriteBufferSwitchZTest = 0;
	// TODO: replace with lookup
	float c = Cos(rotation);
	float s = Sin(rotation);

	float xs[4];
	float ys[4];
	float us[4];
	float vs[4];
	int i;

	xs[0] = x - c*w - s*h;	us[0] = 0.0f;
	xs[1] = x - c*w + s*h;	us[1] = 0.0f;
	xs[2] = x + c*w + s*h;	us[2] = 1.0f;
	xs[3] = x + c*w - s*h;	us[3] = 1.0f;

	ys[0] = y - c*h + s*w;	vs[0] = 0.0f;
	ys[1] = y + c*h + s*w;	vs[1] = 1.0f;
	ys[2] = y + c*h - s*w;	vs[2] = 1.0f;
	ys[3] = y - c*h - s*w;	vs[3] = 0.0f;

	// No clipping, just culling
	if(xs[0] < 0.0f && xs[1] < 0.0f && xs[2] < 0.0f && xs[3] < 0.0f) return;
	if(ys[0] < 0.0f && ys[1] < 0.0f && ys[2] < 0.0f && ys[3] < 0.0f) return;
	if(xs[0] > SCREEN_WIDTH && xs[1] > SCREEN_WIDTH &&
	   xs[2] > SCREEN_WIDTH && xs[3] > SCREEN_WIDTH) return;
	if(ys[0] > SCREEN_HEIGHT && ys[1] > SCREEN_HEIGHT &&
	   ys[2] > SCREEN_HEIGHT && ys[3] > SCREEN_HEIGHT) return;

	float screenz = Sprite_m_f2DNearScreenZ +
		(z-Draw_GetNearClipZ())*(Sprite_m_f2DFarScreenZ-Sprite_m_f2DNearScreenZ)*Draw_GetFarClipZ() /
		((Draw_GetFarClipZ()-Draw_GetNearClipZ())*z);

	void *vert = RwHelper_HostVertexAt(Sprite_HostBufferedVertices(),Sprite_nSpriteBufferIndex*6);
	static int indices[6] = { 0, 1, 2, 3, 0, 2 };
	for(i = 0; i < 6; i++){
		RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(vert,i), xs[indices[i]]);
		RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(vert,i), ys[indices[i]]);
		RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(vert,i), screenz);
		RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(vert,i), z);
		RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(vert,i), recipz);
		RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(vert,i), r*intens>>8, g*intens>>8, b*intens>>8, a);
		RwHelper_HostVertexU(RwHelper_HostVertexAt(vert,i), us[indices[i]], recipz);
		RwHelper_HostVertexV(RwHelper_HostVertexAt(vert,i), vs[indices[i]], recipz);
	}
	Sprite_nSpriteBufferIndex++;
	if(Sprite_nSpriteBufferIndex >= SPRITEBUFFERSIZE)
		Sprite_FlushSpriteBuffer();

}
void Sprite_RenderBufferedOneXLUSprite_Rotate_Aspect(float x, float y, float z, float w, float h, uint8_t r, uint8_t g, uint8_t b, int16_t intens, float recipz, float rotation, uint8_t a)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	Sprite_m_bFlushSpriteBufferSwitchZTest = 0;
	float c = Cos(rotation);
	float s = Sin(rotation);

	float xs[4];
	float ys[4];
	float us[4];
	float vs[4];
	int i;

	xs[0] = x + w*(-c-s);	us[0] = 0.0f;
	xs[1] = x + w*(-c+s);	us[1] = 0.0f;
	xs[2] = x + w*(+c+s);	us[2] = 1.0f;
	xs[3] = x + w*(+c-s);	us[3] = 1.0f;

	ys[0] = y + h*(-c+s);	vs[0] = 0.0f;
	ys[1] = y + h*(+c+s);	vs[1] = 1.0f;
	ys[2] = y + h*(+c-s);	vs[2] = 1.0f;
	ys[3] = y + h*(-c-s);	vs[3] = 0.0f;

	// No clipping, just culling
	if(xs[0] < 0.0f && xs[1] < 0.0f && xs[2] < 0.0f && xs[3] < 0.0f) return;
	if(ys[0] < 0.0f && ys[1] < 0.0f && ys[2] < 0.0f && ys[3] < 0.0f) return;
	if(xs[0] > SCREEN_WIDTH && xs[1] > SCREEN_WIDTH &&
	   xs[2] > SCREEN_WIDTH && xs[3] > SCREEN_WIDTH) return;
	if(ys[0] > SCREEN_HEIGHT && ys[1] > SCREEN_HEIGHT &&
	   ys[2] > SCREEN_HEIGHT && ys[3] > SCREEN_HEIGHT) return;

	float screenz = Sprite_m_f2DNearScreenZ +
		(z-Draw_GetNearClipZ())*(Sprite_m_f2DFarScreenZ-Sprite_m_f2DNearScreenZ)*Draw_GetFarClipZ() /
		((Draw_GetFarClipZ()-Draw_GetNearClipZ())*z);

	void *vert = RwHelper_HostVertexAt(Sprite_HostBufferedVertices(),Sprite_nSpriteBufferIndex*6);
	static int indices[6] = { 0, 1, 2, 3, 0, 2 };
	for(i = 0; i < 6; i++){
		RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(vert,i), xs[indices[i]]);
		RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(vert,i), ys[indices[i]]);
		RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(vert,i), screenz);
		RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(vert,i), z);
		RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(vert,i), recipz);
		RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(vert,i), r*intens>>8, g*intens>>8, b*intens>>8, a);
		RwHelper_HostVertexU(RwHelper_HostVertexAt(vert,i), us[indices[i]], recipz);
		RwHelper_HostVertexV(RwHelper_HostVertexAt(vert,i), vs[indices[i]], recipz);
	}
	Sprite_nSpriteBufferIndex++;
	if(Sprite_nSpriteBufferIndex >= SPRITEBUFFERSIZE)
		Sprite_FlushSpriteBuffer();

}
void Sprite_RenderBufferedOneXLUSprite_Rotate_2Colours(float x, float y, float z, float w, float h, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, float cx, float cy, float recipz, float rotation, uint8_t a)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	Sprite_m_bFlushSpriteBufferSwitchZTest = 0;
	float c = Cos(rotation);
	float s = Sin(rotation);

	float xs[4];
	float ys[4];
	float us[4];
	float vs[4];
	float cf[4];
	int i;

	xs[0] = x + w*(-c-s);	us[0] = 0.0f;
	xs[1] = x + w*(-c+s);	us[1] = 0.0f;
	xs[2] = x + w*(+c+s);	us[2] = 1.0f;
	xs[3] = x + w*(+c-s);	us[3] = 1.0f;

	ys[0] = y + h*(-c+s);	vs[0] = 0.0f;
	ys[1] = y + h*(+c+s);	vs[1] = 1.0f;
	ys[2] = y + h*(+c-s);	vs[2] = 1.0f;
	ys[3] = y + h*(-c-s);	vs[3] = 0.0f;

	// No clipping, just culling
	if(xs[0] < 0.0f && xs[1] < 0.0f && xs[2] < 0.0f && xs[3] < 0.0f) return;
	if(ys[0] < 0.0f && ys[1] < 0.0f && ys[2] < 0.0f && ys[3] < 0.0f) return;
	if(xs[0] > SCREEN_WIDTH && xs[1] > SCREEN_WIDTH &&
	   xs[2] > SCREEN_WIDTH && xs[3] > SCREEN_WIDTH) return;
	if(ys[0] > SCREEN_HEIGHT && ys[1] > SCREEN_HEIGHT &&
	   ys[2] > SCREEN_HEIGHT && ys[3] > SCREEN_HEIGHT) return;

	// Colour factors, cx/y is the direction in which colours change from rgb1 to rgb2
	cf[0] = (cx*(-c-s) + cy*(-c+s))*0.5f + 0.5f;
	cf[0] = Clamp(cf[0], 0.0f, 1.0f);
	cf[1] = (cx*(-c+s) + cy*( c+s))*0.5f + 0.5f;
	cf[1] = Clamp(cf[1], 0.0f, 1.0f);
	cf[2] = (cx*( c+s) + cy*( c-s))*0.5f + 0.5f;
	cf[2] = Clamp(cf[2], 0.0f, 1.0f);
	cf[3] = (cx*( c-s) + cy*(-c-s))*0.5f + 0.5f;
	cf[3] = Clamp(cf[3], 0.0f, 1.0f);

	float screenz = Sprite_m_f2DNearScreenZ +
		(z-Draw_GetNearClipZ())*(Sprite_m_f2DFarScreenZ-Sprite_m_f2DNearScreenZ)*Draw_GetFarClipZ() /
		((Draw_GetFarClipZ()-Draw_GetNearClipZ())*z);

	void *vert = RwHelper_HostVertexAt(Sprite_HostBufferedVertices(),Sprite_nSpriteBufferIndex*6);
	static int indices[6] = { 0, 1, 2, 3, 0, 2 };
	for(i = 0; i < 6; i++){
		RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(vert,i), xs[indices[i]]);
		RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(vert,i), ys[indices[i]]);
		RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(vert,i), screenz);
		RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(vert,i), z);
		RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(vert,i), recipz);
		RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(vert,i),
			r1*cf[indices[i]] + r2*(1.0f - cf[indices[i]]),
			g1*cf[indices[i]] + g2*(1.0f - cf[indices[i]]),
			b1*cf[indices[i]] + b2*(1.0f - cf[indices[i]]),
			a);
		RwHelper_HostVertexU(RwHelper_HostVertexAt(vert,i), us[indices[i]], recipz);
		RwHelper_HostVertexV(RwHelper_HostVertexAt(vert,i), vs[indices[i]], recipz);
	}
	Sprite_nSpriteBufferIndex++;
	if(Sprite_nSpriteBufferIndex >= SPRITEBUFFERSIZE)
		Sprite_FlushSpriteBuffer();

}
void Sprite_Set6Vertices2DRect(void *verts, const CRect *r, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	float screenz, recipz;
	float z = Sprite_HostNearClip(Sprite_HostSceneCamera());	// not done by game

	screenz = Sprite_m_f2DNearScreenZ;
	recipz = Sprite_m_fRecipNearClipPlane;

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,0), r->left);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,0), r->top);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,0), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,0), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,0), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,0), (*c2.r), (*c2.g), (*c2.b), (*c2.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,0), 0.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,0), 0.0f, recipz);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,1), r->right);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,1), r->top);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,1), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,1), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,1), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,1), (*c3.r), (*c3.g), (*c3.b), (*c3.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,1), 1.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,1), 0.0f, recipz);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,2), r->right);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,2), r->bottom);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,2), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,2), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,2), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,2), (*c1.r), (*c1.g), (*c1.b), (*c1.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,2), 1.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,2), 1.0f, recipz);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,3), r->left);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,3), r->bottom);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,3), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,3), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,3), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,3), (*c0.r), (*c0.g), (*c0.b), (*c0.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,3), 0.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,3), 1.0f, recipz);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,4), r->left);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,4), r->top);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,4), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,4), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,4), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,4), (*c2.r), (*c2.g), (*c2.b), (*c2.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,4), 0.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,4), 0.0f, recipz);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,5), r->right);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,5), r->bottom);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,5), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,5), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,5), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,5), (*c1.r), (*c1.g), (*c1.b), (*c1.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,5), 1.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,5), 1.0f, recipz);

}
void Sprite_Set6Vertices2DCoords(void *verts, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, SpriteColourRef c0, SpriteColourRef c1, SpriteColourRef c2, SpriteColourRef c3)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	float screenz, recipz;
	float z = Sprite_HostNearClip(Sprite_HostSceneCamera());	// not done by game

	screenz = Sprite_m_f2DNearScreenZ;
	recipz = Sprite_m_fRecipNearClipPlane;

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,0), x3);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,0), y3);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,0), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,0), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,0), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,0), (*c2.r), (*c2.g), (*c2.b), (*c2.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,0), 0.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,0), 0.0f, recipz);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,1), x4);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,1), y4);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,1), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,1), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,1), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,1), (*c3.r), (*c3.g), (*c3.b), (*c3.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,1), 1.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,1), 0.0f, recipz);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,2), x2);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,2), y2);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,2), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,2), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,2), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,2), (*c1.r), (*c1.g), (*c1.b), (*c1.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,2), 1.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,2), 1.0f, recipz);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,3), x1);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,3), y1);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,3), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,3), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,3), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,3), (*c0.r), (*c0.g), (*c0.b), (*c0.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,3), 0.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,3), 1.0f, recipz);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,4), x3);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,4), y3);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,4), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,4), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,4), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,4), (*c2.r), (*c2.g), (*c2.b), (*c2.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,4), 0.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,4), 0.0f, recipz);

	RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(verts,5), x2);
	RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(verts,5), y2);
	RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(verts,5), screenz);
	RwHelper_HostVertexCameraZ(RwHelper_HostVertexAt(verts,5), z);
	RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(verts,5), recipz);
	RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(verts,5), (*c1.r), (*c1.g), (*c1.b), (*c1.a));
	RwHelper_HostVertexU(RwHelper_HostVertexAt(verts,5), 1.0f, recipz);
	RwHelper_HostVertexV(RwHelper_HostVertexAt(verts,5), 1.0f, recipz);

}
void Sprite_RenderBufferedOneXLUSprite2D(float x, float y, float w, float h, SpriteColourRef colour, int16_t intens, uint8_t alpha)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	Sprite_m_bFlushSpriteBufferSwitchZTest = 1;
	SpriteColour col={intens * (*colour.r) >> 8, intens * (*colour.g) >> 8, intens * (*colour.b) >> 8, alpha};
	CRect rect = Rect_Make(x - w, y - h, x + h, y + h);
	Sprite_Set6Vertices2DRect(RwHelper_HostVertexAt(Sprite_HostBufferedVertices(),6 * Sprite_nSpriteBufferIndex), &rect, Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a), Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a), Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a), Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
	Sprite_nSpriteBufferIndex++;
	if(Sprite_nSpriteBufferIndex >= SPRITEBUFFERSIZE)
		Sprite_FlushSpriteBuffer();

}
void Sprite_RenderBufferedOneXLUSprite2D_Rotate_Dimension(float x, float y, float w, float h, SpriteColourRef colour, int16_t intens, float rotation, uint8_t alpha)
{
    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives

	Sprite_m_bFlushSpriteBufferSwitchZTest = 1;
	SpriteColour col={intens * (*colour.r) >> 8, intens * (*colour.g) >> 8, intens * (*colour.b) >> 8, alpha};
	float c = Cos(rotation);
	float s = Sin(rotation);

	Sprite_Set6Vertices2DCoords(RwHelper_HostVertexAt(Sprite_HostBufferedVertices(),6 * Sprite_nSpriteBufferIndex),
		x + c*w - s*h,
		y - c*h - s*w,
		x + c*w + s*h,
		y + c*h - s*w,
		x - c*w - s*h,
		y - c*h + s*w,
		x - c*w + s*h,
		y + c*h + s*w,
		Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a), Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a), Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a), Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
	Sprite_nSpriteBufferIndex++;
	if(Sprite_nSpriteBufferIndex >= SPRITEBUFFERSIZE)
		Sprite_FlushSpriteBuffer();

}
//- rouz edit (ChatGPT)
