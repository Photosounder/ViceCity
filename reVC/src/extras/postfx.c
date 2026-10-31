//+ rouz edit (ChatGPT)
#include "config.h"
#include "postfx.h"
#include "PostFX_backend.h"
#include "MBlur.h"
#include "Camera_effects.h"
#include <math.h>
#include <stddef.h>
#if defined(RW_GL3) && !defined(RW_OPENGL)
#define RW_OPENGL
#endif
#define HALFPX PostFX_HostHalfPixel()
#define Min(a,b) ((a)<(b)?(a):(b))
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
//+ rouz edit (ChatGPT)

#ifdef EXTENDED_COLOURFILTER

#ifndef LIBRW
#error "Need librw for EXTENDED_COLOURFILTER"
#endif


void *PostFX_pFrontBuffer;
void *PostFX_pBackBuffer;
bool PostFX_bJustInitialised;
int PostFX_EffectSwitch = POSTFX_NORMAL;
bool PostFX_BlurOn = false;
bool PostFX_MotionBlurOn = false;

static uint16_t Index[6] = { 0, 1, 2, 0, 2, 3 };

#ifdef RW_D3D9
void *PostFX_colourfilterVC_PS;
void *PostFX_contrast_PS;
#endif
#ifdef RW_OPENGL
int32_t PostFX_u_blurcolor;
int32_t PostFX_u_contrastAdd;
int32_t PostFX_u_contrastMult;
void *PostFX_colourFilterVC;
void *PostFX_contrast;
#endif

void
PostFX_InitOnce(void)
{
    // Preserve the complete original post-processing operation and SDK call order
#ifdef RW_OPENGL
	PostFX_u_blurcolor = PostFX_HostGLRegister("u_blurcolor");
	PostFX_u_contrastAdd = PostFX_HostGLRegister("u_contrastAdd");
	PostFX_u_contrastMult = PostFX_HostGLRegister("u_contrastMult");
#endif
}

void
PostFX_Open(void *cam)
{
    // Preserve the complete original post-processing operation and SDK call order
	if(PostFX_pFrontBuffer)
		PostFX_Close();

	uint32_t width  = powf(2.0f, (int32_t)(log2(PostFX_HostWidth (PostFX_HostCameraRaster(cam))))+1);
	uint32_t height = powf(2.0f, (int32_t)(log2(PostFX_HostHeight(PostFX_HostCameraRaster(cam))))+1);
	uint32_t depth  = PostFX_HostDepth(PostFX_HostCameraRaster(cam));
	PostFX_pFrontBuffer = PostFX_HostCreateRaster(width,height,depth);
	PostFX_pBackBuffer = PostFX_HostCreateRaster(width,height,depth);
	PostFX_bJustInitialised = true;

	float zero, xmax, ymax;

	if(PostFX_HostDepth(PostFX_HostCameraRaster(cam)) == 16){
		zero = HALFPX;
		xmax = width + HALFPX;
		ymax = height + HALFPX;
	}else{
		zero = -HALFPX;
		xmax = width - HALFPX;
		ymax = height - HALFPX;
	}

	PostFX_HostScreenX(PostFX_HostVertex(PostFX_HostVertices(0),0), zero);
	PostFX_HostScreenY(PostFX_HostVertex(PostFX_HostVertices(0),0), zero);
	PostFX_HostScreenZ(PostFX_HostVertex(PostFX_HostVertices(0),0), PostFX_HostNearScreenZ());
	PostFX_HostCameraZ(PostFX_HostVertex(PostFX_HostVertices(0),0), PostFX_HostNearClip(cam));
	PostFX_HostRecipCameraZ(PostFX_HostVertex(PostFX_HostVertices(0),0), 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostU(PostFX_HostVertex(PostFX_HostVertices(0),0), 0.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostV(PostFX_HostVertex(PostFX_HostVertices(0),0), 0.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),0), 255, 255, 255, 255);

	PostFX_HostScreenX(PostFX_HostVertex(PostFX_HostVertices(0),1), zero);
	PostFX_HostScreenY(PostFX_HostVertex(PostFX_HostVertices(0),1), ymax);
	PostFX_HostScreenZ(PostFX_HostVertex(PostFX_HostVertices(0),1), PostFX_HostNearScreenZ());
	PostFX_HostCameraZ(PostFX_HostVertex(PostFX_HostVertices(0),1), PostFX_HostNearClip(cam));
	PostFX_HostRecipCameraZ(PostFX_HostVertex(PostFX_HostVertices(0),1), 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostU(PostFX_HostVertex(PostFX_HostVertices(0),1), 0.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostV(PostFX_HostVertex(PostFX_HostVertices(0),1), 1.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),1), 255, 255, 255, 255);

	PostFX_HostScreenX(PostFX_HostVertex(PostFX_HostVertices(0),2), xmax);
	PostFX_HostScreenY(PostFX_HostVertex(PostFX_HostVertices(0),2), ymax);
	PostFX_HostScreenZ(PostFX_HostVertex(PostFX_HostVertices(0),2), PostFX_HostNearScreenZ());
	PostFX_HostCameraZ(PostFX_HostVertex(PostFX_HostVertices(0),2), PostFX_HostNearClip(cam));
	PostFX_HostRecipCameraZ(PostFX_HostVertex(PostFX_HostVertices(0),2), 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostU(PostFX_HostVertex(PostFX_HostVertices(0),2), 1.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostV(PostFX_HostVertex(PostFX_HostVertices(0),2), 1.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),2), 255, 255, 255, 255);

	PostFX_HostScreenX(PostFX_HostVertex(PostFX_HostVertices(0),3), xmax);
	PostFX_HostScreenY(PostFX_HostVertex(PostFX_HostVertices(0),3), zero);
	PostFX_HostScreenZ(PostFX_HostVertex(PostFX_HostVertices(0),3), PostFX_HostNearScreenZ());
	PostFX_HostCameraZ(PostFX_HostVertex(PostFX_HostVertices(0),3), PostFX_HostNearClip(cam));
	PostFX_HostRecipCameraZ(PostFX_HostVertex(PostFX_HostVertices(0),3), 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostU(PostFX_HostVertex(PostFX_HostVertices(0),3), 1.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostV(PostFX_HostVertex(PostFX_HostVertices(0),3), 0.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),3), 255, 255, 255, 255);


	PostFX_HostScreenX(PostFX_HostVertex(PostFX_HostVertices(1),0), zero + 2.0f);
	PostFX_HostScreenY(PostFX_HostVertex(PostFX_HostVertices(1),0), zero + 2.0f);
	PostFX_HostScreenZ(PostFX_HostVertex(PostFX_HostVertices(1),0), PostFX_HostNearScreenZ());
	PostFX_HostCameraZ(PostFX_HostVertex(PostFX_HostVertices(1),0), PostFX_HostNearClip(cam));
	PostFX_HostRecipCameraZ(PostFX_HostVertex(PostFX_HostVertices(1),0), 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostU(PostFX_HostVertex(PostFX_HostVertices(1),0), 0.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostV(PostFX_HostVertex(PostFX_HostVertices(1),0), 0.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),0), 255, 255, 255, 255);

	PostFX_HostScreenX(PostFX_HostVertex(PostFX_HostVertices(1),1), 2.0f);
	PostFX_HostScreenY(PostFX_HostVertex(PostFX_HostVertices(1),1), ymax + 2.0f);
	PostFX_HostScreenZ(PostFX_HostVertex(PostFX_HostVertices(1),1), PostFX_HostNearScreenZ());
	PostFX_HostCameraZ(PostFX_HostVertex(PostFX_HostVertices(1),1), PostFX_HostNearClip(cam));
	PostFX_HostRecipCameraZ(PostFX_HostVertex(PostFX_HostVertices(1),1), 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostU(PostFX_HostVertex(PostFX_HostVertices(1),1), 0.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostV(PostFX_HostVertex(PostFX_HostVertices(1),1), 1.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),1), 255, 255, 255, 255);

	PostFX_HostScreenX(PostFX_HostVertex(PostFX_HostVertices(1),2), xmax + 2.0f);
	PostFX_HostScreenY(PostFX_HostVertex(PostFX_HostVertices(1),2), ymax + 2.0f);
	PostFX_HostScreenZ(PostFX_HostVertex(PostFX_HostVertices(1),2), PostFX_HostNearScreenZ());
	PostFX_HostCameraZ(PostFX_HostVertex(PostFX_HostVertices(1),2), PostFX_HostNearClip(cam));
	PostFX_HostRecipCameraZ(PostFX_HostVertex(PostFX_HostVertices(1),2), 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostU(PostFX_HostVertex(PostFX_HostVertices(1),2), 1.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostV(PostFX_HostVertex(PostFX_HostVertices(1),2), 1.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),2), 255, 255, 255, 255);

	PostFX_HostScreenX(PostFX_HostVertex(PostFX_HostVertices(1),3), xmax + 2.0f);
	PostFX_HostScreenY(PostFX_HostVertex(PostFX_HostVertices(1),3), zero + 2.0f);
	PostFX_HostScreenZ(PostFX_HostVertex(PostFX_HostVertices(1),3), PostFX_HostNearScreenZ());
	PostFX_HostCameraZ(PostFX_HostVertex(PostFX_HostVertices(1),3), PostFX_HostNearClip(cam));
	PostFX_HostRecipCameraZ(PostFX_HostVertex(PostFX_HostVertices(1),3), 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostU(PostFX_HostVertex(PostFX_HostVertices(1),3), 1.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostV(PostFX_HostVertex(PostFX_HostVertices(1),3), 0.0f, 1.0f/PostFX_HostNearClip(cam));
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),3), 255, 255, 255, 255);


#ifdef RW_D3D9
#include "shaders/obj/colourfilterVC_PS.inc"
	PostFX_colourfilterVC_PS = PostFX_HostD3DCreate(colourfilterVC_PS_cso);
#include "shaders/obj/contrastPS.inc"
	PostFX_contrast_PS = PostFX_HostD3DCreate(contrastPS_cso);
#endif
#ifdef RW_OPENGL
	

	{
#include "shaders/obj/im2d_vert.inc"
#include "shaders/obj/colourfilterVC_frag.inc"
	const char *vs[] = { PostFX_HostGLText(POSTFX_SHADER_DECL), PostFX_HostGLText(POSTFX_VERTEX_HEADER), im2d_vert_src, NULL };
	const char *fs[] = { PostFX_HostGLText(POSTFX_SHADER_DECL), PostFX_HostGLText(POSTFX_FRAGMENT_HEADER), colourfilterVC_frag_src, NULL };
	PostFX_colourFilterVC = PostFX_HostGLCreate(vs, fs);
	assert(PostFX_colourFilterVC);
	}

	{
#include "shaders/obj/im2d_vert.inc"
#include "shaders/obj/contrast_frag.inc"
	const char *vs[] = { PostFX_HostGLText(POSTFX_SHADER_DECL), PostFX_HostGLText(POSTFX_VERTEX_HEADER), im2d_vert_src, NULL };
	const char *fs[] = { PostFX_HostGLText(POSTFX_SHADER_DECL), PostFX_HostGLText(POSTFX_FRAGMENT_HEADER), contrast_frag_src, NULL };
	PostFX_contrast = PostFX_HostGLCreate(vs, fs);
	assert(PostFX_contrast);
	}

#endif
}

void
PostFX_Close(void)
{
    // Preserve the complete original post-processing operation and SDK call order
	if(PostFX_pFrontBuffer){
		PostFX_HostDestroyRaster(PostFX_pFrontBuffer);
		PostFX_pFrontBuffer = NULL;
	}
	if(PostFX_pBackBuffer){
		PostFX_HostDestroyRaster(PostFX_pBackBuffer);
		PostFX_pBackBuffer = NULL;
	}
#ifdef RW_D3D9
	if(PostFX_colourfilterVC_PS){
		PostFX_HostD3DDestroy(PostFX_colourfilterVC_PS);
		PostFX_colourfilterVC_PS = NULL;
	}
	if(PostFX_contrast_PS){
		PostFX_HostD3DDestroy(PostFX_contrast_PS);
		PostFX_contrast_PS = NULL;
	}
#endif
#ifdef RW_OPENGL
	if(PostFX_colourFilterVC){
		PostFX_HostGLDestroy(PostFX_colourFilterVC);
		PostFX_colourFilterVC = NULL;
	}
	if(PostFX_contrast){
		PostFX_HostGLDestroy(PostFX_contrast);
		PostFX_contrast = NULL;
	}
#endif
}

void
PostFX_RenderOverlayBlur(void *cam, int32_t r, int32_t g, int32_t b, int32_t a)
{
    // Preserve the complete original post-processing operation and SDK call order
	PostFX_HostState(POSTFX_STATE_TEXTURERASTER, PostFX_pFrontBuffer);
	PostFX_HostState(POSTFX_STATE_VERTEXALPHAENABLE, (void*)1);

	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),0), r*2, g*2, b*2, 30);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),1), r*2, g*2, b*2, 30);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),2), r*2, g*2, b*2, 30);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),3), r*2, g*2, b*2, 30);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),0), r*2, g*2, b*2, 30);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),1), r*2, g*2, b*2, 30);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),2), r*2, g*2, b*2, 30);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),3), r*2, g*2, b*2, 30);

	PostFX_HostState(POSTFX_STATE_SRCBLEND, (void*)POSTFX_BLEND_SRCALPHA);
	PostFX_HostState(POSTFX_STATE_DESTBLEND, (void*)POSTFX_BLEND_INVSRCALPHA);

	PostFX_HostDraw( PostFX_BlurOn ? PostFX_HostVertices(1) : PostFX_HostVertices(0), 4, Index, 6);


	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),0), r, g, b, a);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),0), r, g, b, a);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),1), r, g, b, a);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),1), r, g, b, a);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),2), r, g, b, a);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),2), r, g, b, a);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(1),3), r, g, b, a);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),3), r, g, b, a);

	PostFX_HostState(POSTFX_STATE_SRCBLEND, (void*)POSTFX_BLEND_ONE);
	PostFX_HostState(POSTFX_STATE_DESTBLEND, (void*)POSTFX_BLEND_ONE);

	PostFX_HostDraw( PostFX_HostVertices(0), 4, Index, 6);
	PostFX_HostDraw( PostFX_BlurOn ? PostFX_HostVertices(1) : PostFX_HostVertices(0), 4, Index, 6);
}

void
PostFX_RenderOverlaySniper(void *cam, int32_t r, int32_t g, int32_t b, int32_t a)
{
    // Preserve the complete original post-processing operation and SDK call order
	PostFX_HostState(POSTFX_STATE_TEXTURERASTER, PostFX_pFrontBuffer);
	PostFX_HostState(POSTFX_STATE_VERTEXALPHAENABLE, (void*)1);

	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),0), r, g, b, 80);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),1), r, g, b, 80);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),2), r, g, b, 80);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),3), r, g, b, 80);
	PostFX_HostState(POSTFX_STATE_SRCBLEND, (void*)POSTFX_BLEND_SRCALPHA);
	PostFX_HostState(POSTFX_STATE_DESTBLEND, (void*)POSTFX_BLEND_INVSRCALPHA);

	PostFX_HostDraw( PostFX_HostVertices(0), 4, Index, 6);
}

float PostFX_Intensity = 1.0f;

void
PostFX_RenderOverlayShader(void *cam, int32_t r, int32_t g, int32_t b, int32_t a)
{
    // Preserve the complete original post-processing operation and SDK call order
	PostFX_HostState(POSTFX_STATE_TEXTURERASTER, PostFX_pBackBuffer);

	if(PostFX_EffectSwitch == POSTFX_MOBILE){
		float mult[3], add[3];
		mult[0] = (r-64)/256.0f + 1.4f;
		mult[1] = (g-64)/256.0f + 1.4f;
		mult[2] = (b-64)/256.0f + 1.4f;
		add[0] = r/1536.f - 0.05f;
		add[1] = g/1536.f - 0.05f;
		add[2] = b/1536.f - 0.05f;
#ifdef RW_D3D9
		PostFX_HostD3DConstants(10, mult, 1);
		PostFX_HostD3DConstants(11, add, 1);

		PostFX_HostD3DOverride(PostFX_contrast_PS);
#endif
#ifdef RW_OPENGL
		PostFX_HostGLOverride(PostFX_contrast);
		PostFX_HostGLUse(PostFX_contrast);
		PostFX_HostGLUniform3(PostFX_HostGLLocation(PostFX_contrast,PostFX_u_contrastMult), 1, mult);
		PostFX_HostGLUniform3(PostFX_HostGLLocation(PostFX_contrast,PostFX_u_contrastAdd), 1, add);
#endif
	}else{
		float f = PostFX_Intensity;
		float blurcolors[4];
		blurcolors[0] = r*f/255.0f;
		blurcolors[1] = g*f/255.0f;
		blurcolors[2] = b*f/255.0f;
		blurcolors[3] = 30/255.0f;
#ifdef RW_D3D9
		PostFX_HostD3DConstants(10, blurcolors, 1);
		PostFX_HostD3DOverride(PostFX_colourfilterVC_PS);
#endif
#ifdef RW_OPENGL
		PostFX_HostGLOverride(PostFX_colourFilterVC);
		PostFX_HostGLUse(PostFX_colourFilterVC);
		PostFX_HostGLUniform4(PostFX_HostGLLocation(PostFX_colourFilterVC,PostFX_u_blurcolor), 1, blurcolors);
#endif
	}
	PostFX_HostDraw( PostFX_HostVertices(0), 4, Index, 6);
#ifdef RW_D3D9
	PostFX_HostD3DOverride(NULL);
#endif
#ifdef RW_OPENGL
	PostFX_HostGLOverride(NULL);
#endif
}

void
PostFX_RenderMotionBlur(void *cam, uint32_t blur)
{
    // Preserve the complete original post-processing operation and SDK call order
	if(blur == 0)
		return;

	PostFX_HostState(POSTFX_STATE_TEXTURERASTER, PostFX_pFrontBuffer);
	PostFX_HostState(POSTFX_STATE_VERTEXALPHAENABLE, (void*)1);
	PostFX_HostState(POSTFX_STATE_SRCBLEND, (void*)POSTFX_BLEND_SRCALPHA);
	PostFX_HostState(POSTFX_STATE_DESTBLEND, (void*)POSTFX_BLEND_INVSRCALPHA);

	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),0), 255, 255, 255, blur);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),1), 255, 255, 255, blur);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),2), 255, 255, 255, blur);
	PostFX_HostIntRGBA(PostFX_HostVertex(PostFX_HostVertices(0),3), 255, 255, 255, blur);

	PostFX_HostDraw( PostFX_HostVertices(0), 4, Index, 6);
}

bool
PostFX_NeedBackBuffer(void)
{
    // Preserve the complete original post-processing operation and SDK call order
	// Current frame -- needed for non-blur effect
	switch(PostFX_EffectSwitch){
	case POSTFX_OFF:
	case POSTFX_SIMPLE:
		// no actual rendering here
		return false;
	case POSTFX_NORMAL:
		if(PostFX_MotionBlurOn)
			return false;
		else
			return true;
	case POSTFX_MOBILE:
		return true;
	}
	return false;
}

bool
PostFX_NeedFrontBuffer(int32_t type)
{
    // Preserve the complete original post-processing operation and SDK call order
	// Last frame -- needed for motion blur
	// Use the complete C motion-blur owner and its live shared state
	if(MBlur_Drunkness > 0.0f)
		return true;
	if(type == MOTION_BLUR_SNIPER)
		return true;

	switch(PostFX_EffectSwitch){
	case POSTFX_OFF:
	case POSTFX_SIMPLE:
		// no actual rendering here
		return false;
	case POSTFX_NORMAL:
		if(PostFX_MotionBlurOn)
			return true;
		else
			return false;
	case POSTFX_MOBILE:
		return false;
	}
	return false;
}

void
PostFX_GetBackBuffer(void *cam)
{
    // Preserve the complete original post-processing operation and SDK call order
	PostFX_HostPushContext(PostFX_pBackBuffer);
	PostFX_HostRenderFast(PostFX_HostCameraRaster(cam), 0, 0);
	PostFX_HostPopContext();
}

void
PostFX_Render(void *cam, uint32_t red, uint32_t green, uint32_t blue, uint32_t blur, int32_t type, uint32_t bluralpha)
{
    // Preserve the complete original post-processing operation and SDK call order
	PostFX_HostPushGroup("CPostFX::Render");

	if(PostFX_pFrontBuffer == NULL)
		PostFX_Open(cam);
	assert(PostFX_pFrontBuffer);
	assert(PostFX_pBackBuffer);

	if(type == MOTION_BLUR_LIGHT_SCENE){
		PostFX_SmoothColor(red, green, blue, blur);
		red = PostFX_AvgRed;
		green = PostFX_AvgGreen;
		blue = PostFX_AvgBlue;
		blur = PostFX_AvgAlpha;
	}

	if(PostFX_NeedBackBuffer())
		PostFX_GetBackBuffer(cam);

	PostFX_HostDefinedState();

	PostFX_HostState(POSTFX_STATE_FOGENABLE, (void*)0);
	PostFX_HostState(POSTFX_STATE_TEXTUREFILTER, (void*)0);
	PostFX_HostState(POSTFX_STATE_ZTESTENABLE, (void*)0);
	PostFX_HostState(POSTFX_STATE_ZWRITEENABLE, (void*)0);

	if(type == MOTION_BLUR_SNIPER){
		if(!PostFX_bJustInitialised)
			PostFX_RenderOverlaySniper(cam, red, green, blue, blur);
	}else switch(PostFX_EffectSwitch){
	case POSTFX_OFF:
	case POSTFX_SIMPLE:
		// no actual rendering here
		break;
	case POSTFX_NORMAL:
		if(PostFX_MotionBlurOn){
			if(!PostFX_bJustInitialised)
				PostFX_RenderOverlayBlur(cam, red, green, blue, blur);
		}else{
			PostFX_RenderOverlayShader(cam, red, green, blue, blur);
		}
		break;
	case POSTFX_MOBILE:
		PostFX_RenderOverlayShader(cam, red, green, blue, blur);
		break;
	}

	if(!PostFX_bJustInitialised)
		// Use the complete C motion-blur owner and its live shared state
		PostFX_RenderMotionBlur(cam, 175.0f * MBlur_Drunkness);

	PostFX_HostState(POSTFX_STATE_ZTESTENABLE, (void*)1);
	PostFX_HostState(POSTFX_STATE_ZWRITEENABLE, (void*)1);
	PostFX_HostState(POSTFX_STATE_TEXTURERASTER, NULL);
	PostFX_HostState(POSTFX_STATE_VERTEXALPHAENABLE, (void*)0);
	PostFX_HostState(POSTFX_STATE_SRCBLEND, (void*)POSTFX_BLEND_SRCALPHA);
	PostFX_HostState(POSTFX_STATE_DESTBLEND, (void*)POSTFX_BLEND_INVSRCALPHA);

	if(PostFX_NeedFrontBuffer(type)){
		PostFX_HostPushContext(PostFX_pFrontBuffer);
		PostFX_HostRenderFast(PostFX_HostCameraRaster(cam), 0, 0);
		PostFX_HostPopContext();
		PostFX_bJustInitialised = false;
	}else
		PostFX_bJustInitialised = true;

	PostFX_HostPopGroup();
}

int PostFX_PrevRed[POSTFX_NUMAVERAGE], PostFX_AvgRed;
int PostFX_PrevGreen[POSTFX_NUMAVERAGE], PostFX_AvgGreen;
int PostFX_PrevBlue[POSTFX_NUMAVERAGE], PostFX_AvgBlue;
int PostFX_PrevAlpha[POSTFX_NUMAVERAGE], PostFX_AvgAlpha;
int PostFX_Next;
int PostFX_NumValues;

// This is rather annoying...the blur color can flicker slightly
// which becomes very visible when amplified by the shader
void
PostFX_SmoothColor(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha)
{
    // Preserve the complete original post-processing operation and SDK call order
	PostFX_PrevRed[PostFX_Next] = red;
	PostFX_PrevGreen[PostFX_Next] = green;
	PostFX_PrevBlue[PostFX_Next] = blue;
	PostFX_PrevAlpha[PostFX_Next] = alpha;
	PostFX_Next = (PostFX_Next+1) % POSTFX_NUMAVERAGE;
	PostFX_NumValues = Min(PostFX_NumValues+1, POSTFX_NUMAVERAGE);

	PostFX_AvgRed = 0;
	PostFX_AvgGreen = 0;
	PostFX_AvgBlue = 0;
	PostFX_AvgAlpha = 0;
	for(int i = 0; i < PostFX_NumValues; i++){
		PostFX_AvgRed += PostFX_PrevRed[i];
		PostFX_AvgGreen += PostFX_PrevGreen[i];
		PostFX_AvgBlue += PostFX_PrevBlue[i];
		PostFX_AvgAlpha += PostFX_PrevAlpha[i];
	}
	PostFX_AvgRed /= PostFX_NumValues;
	PostFX_AvgGreen /= PostFX_NumValues;
	PostFX_AvgBlue /= PostFX_NumValues;
	PostFX_AvgAlpha /= PostFX_NumValues;
}

#endif
//- rouz edit (ChatGPT)

#ifdef EXTENDED_COLOURFILTER
bool PostFX_UseBlurColours(void)
{
    // Preserve the complete original inline predicate using its live C effect preference
    return PostFX_EffectSwitch!=POSTFX_SIMPLE;
}
#endif

//- rouz edit (ChatGPT)
