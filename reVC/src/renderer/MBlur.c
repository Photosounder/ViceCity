//+ rouz edit (ChatGPT)
#include "config.h"
#include "MBlur.h"
#include "MBlur_backend.h"
#include "Camera_effects.h"
#include "Draw.h"
#include "Draw_constants.h"
#include "General.h"
#include "Timecycle.h"
#include "Timer.h"
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#define HALFPX MBlur_HostHalfPixel()
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))
#ifdef GTA_PS2
#ifdef GTA_PAL
#define SCREEN_WIDTH ((float)640)
#define SCREEN_HEIGHT ((float)512)
#else
#define SCREEN_WIDTH ((float)640)
#define SCREEN_HEIGHT ((float)448)
#endif
#else
#define SCREEN_WIDTH ((float)MBlur_HostScreenWidth())
#define SCREEN_HEIGHT ((float)MBlur_HostScreenHeight())
#endif
#define SCREEN_STRETCH_X(a) ((a)*(float)SCREEN_WIDTH/DEFAULT_SCREEN_WIDTH)
#define SCREEN_STRETCH_Y(a) ((a)*(float)SCREEN_HEIGHT/DEFAULT_SCREEN_HEIGHT)
#define SCREEN_SCALE_X(a) SCREEN_SCALE_AR(SCREEN_STRETCH_X(a))
#define SCREEN_SCALE_Y(a) SCREEN_STRETCH_Y(a)
#define SCREEN_SCALE_FROM_BOTTOM(a) (SCREEN_HEIGHT-SCREEN_SCALE_Y(a))
#ifdef ASPECT_RATIO_SCALE
#define SCREEN_SCALE_AR(a) ((a)*DEFAULT_ASPECT_RATIO/Draw_GetAspectRatio())
#if defined(PROPER_SCALING) && !defined(FORCE_PC_SCALING)
#undef SCREEN_SCALE_Y
#define SCREEN_SCALE_Y(a) Draw_ScaleY(SCREEN_STRETCH_Y(a))
#endif
#else
#define SCREEN_SCALE_AR(a) (a)
#endif
static MBlurColour MBlur_MakeColour(uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha)
{
    // Preserve the original constructor's four channel conversions and value result
    const MBlurColour value={red,green,blue,alpha};return value;
}
static MBlurRect MBlur_CopyRect(MBlurRectRef rect)
{
    // Copy only the current scalar values after the original caller-visible rectangle mutations
    const MBlurRect value={*rect.x,*rect.y,*rect.w,*rect.h};return value;
}
//+ rouz edit (ChatGPT)
#ifndef LIBRW
#define WITHD3D
#endif
#ifndef LIBRW
#endif

//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
#endif
//- rouz edit (ChatGPT)

// Originally taken from RW example 'mblur'

void *MBlur_pFrontBuffer;
bool MBlur_ms_bJustInitialised;
bool MBlur_ms_bScaledBlur;
bool MBlur_BlurOn;
float MBlur_Drunkness;

int32_t MBlur_pBufVertCount;

static uint16_t Index[6] = { 0, 1, 2, 0, 2, 3 };

#ifndef LIBRW
#endif
int32_t
MBlur_MotionBlurOpen(void *cam)
{
    // Preserve the complete original blur operation and externally visible mutation order
#ifdef EXTENDED_COLOURFILTER
	MBlur_HostPostOpen(cam);
	return 1;
#else
#ifdef GTA_PS2
	MBlurRect rect = {0, 0, 0, 0};
	
	if (MBlur_pFrontBuffer)
		return 1;
	
	MBlur_BlurOn = true;
	
	rect.w = MBlur_HostWidth(MBlur_HostCameraRaster(cam));
	rect.h = MBlur_HostHeight(MBlur_HostCameraRaster(cam));
	
	MBlur_pFrontBuffer = MBlur_HostCreateRaster(0, 0, 0, MBLUR_DONT_ALLOCATE|MBLUR_CAMERA_TEXTURE);
	if (!MBlur_pFrontBuffer)
	{
		printf("Error creating raster\n");
		return 0;
	}
	
	void *raster = MBlur_HostSubRaster(MBlur_pFrontBuffer, MBlur_HostCameraRaster(cam), &rect);
	if (!raster)
	{
		MBlur_HostDestroyRaster(MBlur_pFrontBuffer);
		MBlur_pFrontBuffer = NULL;
		printf("Error subrastering\n");
		return 0;
	}
	
	MBlur_CreateImmediateModeData(cam, MBlur_RectRef(&rect));
#else
	MBlurRect rect = { 0, 0, 0, 0 };

	if(MBlur_pFrontBuffer)
		MBlur_MotionBlurClose();
	
#ifndef LIBRW
	
	uint32_t total, avaible;
	
	MBlur_HostVideoInfo(&total, &avaible);
	MBlur_HostDebugMemory(avaible);
#endif
		
	if(MBlur_BlurOn)
	{
		uint32_t width  = powf(2.0f, (int32_t)(log2(MBlur_HostWidth (MBlur_HostCameraRaster(cam))))+1);
		uint32_t height = powf(2.0f, (int32_t)(log2(MBlur_HostHeight(MBlur_HostCameraRaster(cam))))+1);
		uint32_t depth  = MBlur_HostDepth(MBlur_HostCameraRaster(cam));
		
#ifndef LIBRW
		
		if ( MBlur_HostMaxTextureWidth() >= width && MBlur_HostMaxTextureHeight() >= height )
		{
			total = MBlur_HostVideoMemory() - 3 *
				( MBlur_HostDepth(MBlur_HostCameraRaster(cam))
				* MBlur_HostHeight(MBlur_HostCameraRaster(cam))
				* MBlur_HostWidth(MBlur_HostCameraRaster(cam)) / 8 );
			MBlur_BlurOn = total >= height*width*(depth/8) + (12*1024*1024) /*12 MB*/;
		}
		else
			MBlur_BlurOn = false;
#endif
		
		if ( MBlur_BlurOn )
		{
			MBlur_ms_bScaledBlur = false;
			rect.w = width;
			rect.h = height;
			
			MBlur_pFrontBuffer = MBlur_HostCreateRaster(rect.w, rect.h, depth, MBLUR_CAMERA_TEXTURE);
			if ( !MBlur_pFrontBuffer )
			{
				MBlur_HostDebugOpenFailure();
				MBlur_BlurOn = false;
				rect.w = MBlur_HostWidth(MBlur_HostCameraRaster(cam));
				rect.h = MBlur_HostHeight(MBlur_HostCameraRaster(cam));
			}
			else
				MBlur_ms_bJustInitialised = true;
		}
		else
		{
			rect.w = MBlur_HostWidth(MBlur_HostCameraRaster(cam));
			rect.h = MBlur_HostHeight(MBlur_HostCameraRaster(cam));
		}
		
#ifndef LIBRW
		MBlur_HostVideoInfo(&total, &avaible);
		MBlur_HostDebugMemory(avaible);
#endif
		MBlur_CreateImmediateModeData(cam, MBlur_RectRef(&rect));
	}
	else
	{
		rect.w = MBlur_HostWidth(MBlur_HostCameraRaster(cam));
		rect.h = MBlur_HostHeight(MBlur_HostCameraRaster(cam));
		MBlur_CreateImmediateModeData(cam, MBlur_RectRef(&rect));
	}
	
	return 1;
#endif
#endif
}

int32_t
MBlur_MotionBlurClose(void)
{
    // Preserve the complete original blur operation and externally visible mutation order
#ifdef EXTENDED_COLOURFILTER
	MBlur_HostPostClose();
#else
	if(MBlur_pFrontBuffer){
		MBlur_HostDestroyRaster(MBlur_pFrontBuffer);
		MBlur_pFrontBuffer = NULL;
		
		return 1;
	}
#endif
	return 0;
}

void
MBlur_CreateImmediateModeData(void *cam, MBlurRectRef rect)
{
    // Preserve the complete original blur operation and externally visible mutation order
	float zero, xmax, ymax;

	if(MBlur_HostDepth(MBlur_HostCameraRaster(cam)) == 16){
		zero = HALFPX;
		xmax = (*rect.w) + HALFPX;
		ymax = (*rect.h) + HALFPX;
	}else{
		zero = -HALFPX;
		xmax = (*rect.w) - HALFPX;
		ymax = (*rect.h) - HALFPX;
	}

	MBlur_HostScreenX(MBlur_HostVertex(MBlur_HostVertices(0),0), zero);
	MBlur_HostScreenY(MBlur_HostVertex(MBlur_HostVertices(0),0), zero);
	MBlur_HostScreenZ(MBlur_HostVertex(MBlur_HostVertices(0),0), MBlur_HostNearScreenZ());
	MBlur_HostCameraZ(MBlur_HostVertex(MBlur_HostVertices(0),0), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(MBlur_HostVertices(0),0), 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostU(MBlur_HostVertex(MBlur_HostVertices(0),0), 0.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostV(MBlur_HostVertex(MBlur_HostVertices(0),0), 0.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),0), 255, 255, 255, 255);

	MBlur_HostScreenX(MBlur_HostVertex(MBlur_HostVertices(0),1), zero);
	MBlur_HostScreenY(MBlur_HostVertex(MBlur_HostVertices(0),1), ymax);
	MBlur_HostScreenZ(MBlur_HostVertex(MBlur_HostVertices(0),1), MBlur_HostNearScreenZ());
	MBlur_HostCameraZ(MBlur_HostVertex(MBlur_HostVertices(0),1), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(MBlur_HostVertices(0),1), 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostU(MBlur_HostVertex(MBlur_HostVertices(0),1), 0.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostV(MBlur_HostVertex(MBlur_HostVertices(0),1), 1.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),1), 255, 255, 255, 255);

	MBlur_HostScreenX(MBlur_HostVertex(MBlur_HostVertices(0),2), xmax);
	MBlur_HostScreenY(MBlur_HostVertex(MBlur_HostVertices(0),2), ymax);
	MBlur_HostScreenZ(MBlur_HostVertex(MBlur_HostVertices(0),2), MBlur_HostNearScreenZ());
	MBlur_HostCameraZ(MBlur_HostVertex(MBlur_HostVertices(0),2), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(MBlur_HostVertices(0),2), 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostU(MBlur_HostVertex(MBlur_HostVertices(0),2), 1.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostV(MBlur_HostVertex(MBlur_HostVertices(0),2), 1.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),2), 255, 255, 255, 255);

	MBlur_HostScreenX(MBlur_HostVertex(MBlur_HostVertices(0),3), xmax);
	MBlur_HostScreenY(MBlur_HostVertex(MBlur_HostVertices(0),3), zero);
	MBlur_HostScreenZ(MBlur_HostVertex(MBlur_HostVertices(0),3), MBlur_HostNearScreenZ());
	MBlur_HostCameraZ(MBlur_HostVertex(MBlur_HostVertices(0),3), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(MBlur_HostVertices(0),3), 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostU(MBlur_HostVertex(MBlur_HostVertices(0),3), 1.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostV(MBlur_HostVertex(MBlur_HostVertices(0),3), 0.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),3), 255, 255, 255, 255);


	MBlur_HostScreenX(MBlur_HostVertex(MBlur_HostVertices(1),0), zero + 2.0f);
	MBlur_HostScreenY(MBlur_HostVertex(MBlur_HostVertices(1),0), zero + 2.0f);
	MBlur_HostScreenZ(MBlur_HostVertex(MBlur_HostVertices(1),0), MBlur_HostNearScreenZ());
	MBlur_HostCameraZ(MBlur_HostVertex(MBlur_HostVertices(1),0), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(MBlur_HostVertices(1),0), 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostU(MBlur_HostVertex(MBlur_HostVertices(1),0), 0.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostV(MBlur_HostVertex(MBlur_HostVertices(1),0), 0.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),0), 255, 255, 255, 255);

	MBlur_HostScreenX(MBlur_HostVertex(MBlur_HostVertices(1),1), 2.0f);
	MBlur_HostScreenY(MBlur_HostVertex(MBlur_HostVertices(1),1), ymax + 2.0f);
	MBlur_HostScreenZ(MBlur_HostVertex(MBlur_HostVertices(1),1), MBlur_HostNearScreenZ());
	MBlur_HostCameraZ(MBlur_HostVertex(MBlur_HostVertices(1),1), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(MBlur_HostVertices(1),1), 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostU(MBlur_HostVertex(MBlur_HostVertices(1),1), 0.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostV(MBlur_HostVertex(MBlur_HostVertices(1),1), 1.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),1), 255, 255, 255, 255);

	MBlur_HostScreenX(MBlur_HostVertex(MBlur_HostVertices(1),2), xmax + 2.0f);
	MBlur_HostScreenY(MBlur_HostVertex(MBlur_HostVertices(1),2), ymax + 2.0f);
	MBlur_HostScreenZ(MBlur_HostVertex(MBlur_HostVertices(1),2), MBlur_HostNearScreenZ());
	MBlur_HostCameraZ(MBlur_HostVertex(MBlur_HostVertices(1),2), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(MBlur_HostVertices(1),2), 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostU(MBlur_HostVertex(MBlur_HostVertices(1),2), 1.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostV(MBlur_HostVertex(MBlur_HostVertices(1),2), 1.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),2), 255, 255, 255, 255);

	MBlur_HostScreenX(MBlur_HostVertex(MBlur_HostVertices(1),3), xmax + 2.0f);
	MBlur_HostScreenY(MBlur_HostVertex(MBlur_HostVertices(1),3), zero + 2.0f);
	MBlur_HostScreenZ(MBlur_HostVertex(MBlur_HostVertices(1),3), MBlur_HostNearScreenZ());
	MBlur_HostCameraZ(MBlur_HostVertex(MBlur_HostVertices(1),3), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(MBlur_HostVertices(1),3), 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostU(MBlur_HostVertex(MBlur_HostVertices(1),3), 1.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostV(MBlur_HostVertex(MBlur_HostVertices(1),3), 0.0f, 1.0f/MBlur_HostNearClip(cam));
	MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),3), 255, 255, 255, 255);
}

void
MBlur_CreateImmediateModeVertices(void *cam, MBlurRectRef rect, void *verts, MBlurColour color, float u1Off, float v1Off, float u2Off, float v2Off, float z, int fullTexture)
{
    // Preserve the complete original blur operation and externally visible mutation order
	float x1 = (*rect.x);
	float y1 = (*rect.y);
	float x2 = (*rect.w);
	float y2 = (*rect.h);

	float u1, v1, u2, v2;
	if(fullTexture){
		u1 = 0.0f;
		v1 = 0.0f;
		u2 = 1.0f;
		v2 = 1.0f;
	}else{
		if(MBlur_HostDepth(MBlur_HostCameraRaster(cam)) == 16){
			x1 += HALFPX;
			y1 += HALFPX;
			x2 += HALFPX;
			y2 += HALFPX;
		}else{
			x1 -= HALFPX;
			y1 -= HALFPX;
			x2 -= HALFPX;
			y2 -= HALFPX;
		}

		int32_t width  = powf(2.0f, (int32_t)(log2(MBlur_HostWidth (MBlur_HostCameraRaster(cam))))+1);
		int32_t height = powf(2.0f, (int32_t)(log2(MBlur_HostHeight(MBlur_HostCameraRaster(cam))))+1);
		u1 = x1/width + u1Off;
		v1 = y1/height + v1Off;
		u2 = x2/width + u2Off;
		v2 = y2/height + v2Off;
		u1 = Clamp(u1, 0.0f, 1.0f);
		v1 = Clamp(v1, 0.0f, 1.0f);
		u2 = Clamp(u2, 0.0f, 1.0f);
		v2 = Clamp(v2, 0.0f, 1.0f);
	}

	float recipz = 1.0f/z;
	// TODO: CameraZ is wrong, what should we do?
	MBlur_HostScreenX(MBlur_HostVertex(verts,0), x1);
	MBlur_HostScreenY(MBlur_HostVertex(verts,0), y1);
	MBlur_HostScreenZ(MBlur_HostVertex(verts,0), z);
	MBlur_HostCameraZ(MBlur_HostVertex(verts,0), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(verts,0), recipz);
	MBlur_HostU(MBlur_HostVertex(verts,0), u1, recipz);
	MBlur_HostV(MBlur_HostVertex(verts,0), v1, recipz);
	MBlur_HostIntRGBA(MBlur_HostVertex(verts,0), color.red, color.green, color.blue, color.alpha);

	MBlur_HostScreenX(MBlur_HostVertex(verts,1), x1);
	MBlur_HostScreenY(MBlur_HostVertex(verts,1), y2);
	MBlur_HostScreenZ(MBlur_HostVertex(verts,1), z);
	MBlur_HostCameraZ(MBlur_HostVertex(verts,1), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(verts,1), recipz);
	MBlur_HostU(MBlur_HostVertex(verts,1), u1, recipz);
	MBlur_HostV(MBlur_HostVertex(verts,1), v2, recipz);
	MBlur_HostIntRGBA(MBlur_HostVertex(verts,1), color.red, color.green, color.blue, color.alpha);

	MBlur_HostScreenX(MBlur_HostVertex(verts,2), x2);
	MBlur_HostScreenY(MBlur_HostVertex(verts,2), y2);
	MBlur_HostScreenZ(MBlur_HostVertex(verts,2), z);
	MBlur_HostCameraZ(MBlur_HostVertex(verts,2), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(verts,2), recipz);
	MBlur_HostU(MBlur_HostVertex(verts,2), u2, recipz);
	MBlur_HostV(MBlur_HostVertex(verts,2), v2, recipz);
	MBlur_HostIntRGBA(MBlur_HostVertex(verts,2), color.red, color.green, color.blue, color.alpha);

	MBlur_HostScreenX(MBlur_HostVertex(verts,3), x2);
	MBlur_HostScreenY(MBlur_HostVertex(verts,3), y1);
	MBlur_HostScreenZ(MBlur_HostVertex(verts,3), z);
	MBlur_HostCameraZ(MBlur_HostVertex(verts,3), MBlur_HostNearClip(cam));
	MBlur_HostRecipCameraZ(MBlur_HostVertex(verts,3), recipz);
	MBlur_HostU(MBlur_HostVertex(verts,3), u2, recipz);
	MBlur_HostV(MBlur_HostVertex(verts,3), v1, recipz);
	MBlur_HostIntRGBA(MBlur_HostVertex(verts,3), color.red, color.green, color.blue, color.alpha);
}

void
MBlur_MotionBlurRender(void *cam, uint32_t red, uint32_t green, uint32_t blue, uint32_t blur, int32_t type, uint32_t bluralpha)
{
    // Preserve the complete original blur operation and externally visible mutation order
#ifdef EXTENDED_COLOURFILTER
	MBlur_HostPostRender(cam, red, green, blue, blur, type, bluralpha);
#else
	MBlur_HostPushGroup("CMBlur::MotionBlurRender");
	MBlurColour color = { (uint8_t)red, (uint8_t)green, (uint8_t)blue, (uint8_t)blur };
#ifdef GTA_PS2
	if( MBlur_pFrontBuffer )
		MBlur_OverlayRender(cam, MBlur_pFrontBuffer, color, type, bluralpha);
#else
	if(MBlur_ms_bJustInitialised)
		MBlur_ms_bJustInitialised = false;
	else
		MBlur_OverlayRender(cam, MBlur_pFrontBuffer, color, type, bluralpha);
	if(MBlur_BlurOn){
		MBlur_HostPushContext(MBlur_pFrontBuffer);
		MBlur_HostRenderFast(MBlur_HostCameraRaster(cam), 0, 0);
		MBlur_HostPopContext();
	}
#endif
	MBlur_HostPopGroup();
#endif
}

static uint8_t DrunkBlurRed = 128;
static uint8_t DrunkBlurGreen = 128;
static uint8_t DrunkBlurBlue = 128;
static int32_t DrunkBlurIncrement = 1;

void
MBlur_OverlayRender(void *cam, void *raster, MBlurColour color, int32_t type, int32_t bluralpha)
{
    // Preserve the complete original blur operation and externally visible mutation order
	int r, g, b, a;

	r = color.red;
	g = color.green;
	b = color.blue;
	a = color.alpha;

	MBlur_HostDefinedState();

	switch(type)
	{
	case MOTION_BLUR_SECURITY_CAM:
		r = 0;
		g = 255;
		b = 0;
		a = 128;
		break;
	case MOTION_BLUR_INTRO:
		r = 100;
		g = 220;
		b = 230;
		a = 158;
		break;
	case MOTION_BLUR_INTRO2:
		r = 80;
		g = 255;
		b = 230;
		a = 138;
		break;
	case MOTION_BLUR_INTRO3:
		r = 255;
		g = 60;
		b = 60;
		a = 200;
		break;
	case MOTION_BLUR_INTRO4:
		r = 255;
		g = 180;
		b = 180;
		a = 128;
		break;
	}

	if(!MBlur_BlurOn){
		// gta clamps these to 255 (probably a macro or inlined function)
		int ovR = r * 0.6f;
		int ovG = g * 0.6f;
		int ovB = b * 0.6f;
		int ovA = type == MOTION_BLUR_SNIPER ? a : a*0.6f;
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),0), ovR, ovG, ovB, ovA);
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),1), ovR, ovG, ovB, ovA);
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),2), ovR, ovG, ovB, ovA);
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),3), ovR, ovG, ovB, ovA);
	}else{
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),0), r, g, b, a);
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),0), r, g, b, a);
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),1), r, g, b, a);
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),1), r, g, b, a);
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),2), r, g, b, a);
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),2), r, g, b, a);
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),3), r, g, b, a);
		MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),3), r, g, b, a);
	}

	MBlur_HostState(MBLUR_STATE_TEXTUREFILTER, (void*)0);
	MBlur_HostState(MBLUR_STATE_FOGENABLE, (void*)0);
	MBlur_HostState(MBLUR_STATE_ZTESTENABLE, (void*)0);
	MBlur_HostState(MBLUR_STATE_ZWRITEENABLE, (void*)0);

	MBlur_HostState(MBLUR_STATE_TEXTURERASTER, raster);
	MBlur_HostState(MBLUR_STATE_VERTEXALPHAENABLE, (void*)1);
	MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_ONE);
	MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_ONE);

	if(MBlur_BlurOn){
		if(type == MOTION_BLUR_SNIPER){
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),0), r, g, b, 80);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),1), r, g, b, 80);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),2), r, g, b, 80);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),3), r, g, b, 80);
			MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_SRCALPHA);
			MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_INVSRCALPHA);
			MBlur_pBufVertCount = 0;
		}else{
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),0), r*2, g*2, b*2, 30);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),1), r*2, g*2, b*2, 30);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),2), r*2, g*2, b*2, 30);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),3), r*2, g*2, b*2, 30);
			MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_SRCALPHA);
			MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_INVSRCALPHA);

			MBlur_HostDraw( MBlur_HostVertices(1), 4, Index, 6);

			MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_ONE);
			MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_ONE);

			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),0), r, g, b, a);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),0), r, g, b, a);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),1), r, g, b, a);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),1), r, g, b, a);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),2), r, g, b, a);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),2), r, g, b, a);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(1),3), r, g, b, a);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),3), r, g, b, a);
			MBlur_HostDraw( MBlur_HostVertices(0), 4, Index, 6);
			MBlur_HostDraw( MBlur_HostVertices(1), 4, Index, 6);
		}
	}

	int DrunkBlurAlpha = 175.0f * MBlur_Drunkness;
	if(DrunkBlurAlpha != 0){
		MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_SRCALPHA);
		MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_INVSRCALPHA);
		if(MBlur_BlurOn){
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),0), 255, 255, 255, DrunkBlurAlpha);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),1), 255, 255, 255, DrunkBlurAlpha);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),2), 255, 255, 255, DrunkBlurAlpha);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),3), 255, 255, 255, DrunkBlurAlpha);
		}else{
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),0), DrunkBlurRed, DrunkBlurGreen, DrunkBlurBlue, DrunkBlurAlpha);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),1), DrunkBlurRed, DrunkBlurGreen, DrunkBlurBlue, DrunkBlurAlpha);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),2), DrunkBlurRed, DrunkBlurGreen, DrunkBlurBlue, DrunkBlurAlpha);
			MBlur_HostIntRGBA(MBlur_HostVertex(MBlur_HostVertices(0),3), DrunkBlurRed, DrunkBlurGreen, DrunkBlurBlue, DrunkBlurAlpha);
			if(DrunkBlurIncrement){
				if(DrunkBlurRed < 255) DrunkBlurRed++;
				if(DrunkBlurGreen < 255) DrunkBlurGreen++;
				if(DrunkBlurBlue < 255) DrunkBlurBlue++;
				if(DrunkBlurRed == 255)
					DrunkBlurIncrement = 0;
			}else{
				if(DrunkBlurRed > 128) DrunkBlurRed--;
				if(DrunkBlurGreen > 128) DrunkBlurGreen--;
				if(DrunkBlurBlue > 128) DrunkBlurBlue--;
				if(DrunkBlurRed == 128)
					DrunkBlurIncrement = 1;
			}
		}
		MBlur_HostDraw( MBlur_HostVertices(0), 4, Index, 6);
	}

	if(type != MOTION_BLUR_SNIPER)
		MBlur_OverlayRenderFx(cam, MBlur_pFrontBuffer);

	MBlur_HostState(MBLUR_STATE_FOGENABLE, (void*)0);
	MBlur_HostState(MBLUR_STATE_ZTESTENABLE, (void*)1);
	MBlur_HostState(MBLUR_STATE_ZWRITEENABLE, (void*)1);
	MBlur_HostState(MBLUR_STATE_TEXTURERASTER, NULL);
	MBlur_HostState(MBLUR_STATE_VERTEXALPHAENABLE, (void*)0);
	MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_SRCALPHA);
	MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_INVSRCALPHA);
}

void
MBlur_SetDrunkBlur(float drunkness)
{
    // Preserve the complete original blur operation and externally visible mutation order
	MBlur_Drunkness = Clamp(drunkness, 0.0f, 1.0f);
}

void
MBlur_ClearDrunkBlur(void)
{
    // Preserve the complete original blur operation and externally visible mutation order
	MBlur_Drunkness = 0.0f;
	Timer_SetTimeScale(1.0f);
}

#define NUM_RENDER_FX 64

static MBlurRect fxRect[NUM_RENDER_FX];
static FxType fxType[NUM_RENDER_FX];
static float fxZ[NUM_RENDER_FX];

bool
MBlur_PosInside(MBlurRectRef rect, float x1, float y1, float x2, float y2)
{
    // Preserve the complete original blur operation and externally visible mutation order
	// Scale the overlap tolerance along with the virtual screen dimensions
	//+ rouz edit (ChatGPT)
	const float paddingX = SCREEN_SCALE_X(10.0f);
	const float paddingY = SCREEN_SCALE_Y(10.0f);
	// Check overlap on both axes so a large effect cannot cover the whole UI region
	if((*rect.w) < x1-paddingX || (*rect.x) > x2+paddingX ||
	   (*rect.h) < y1-paddingY || (*rect.y) > y2+paddingY)
		return false;
	//- rouz edit (ChatGPT)
	return true;
}

bool
MBlur_AddRenderFx(void *cam, MBlurRectRef rect, float z, FxType type)
{
    // Preserve the complete original blur operation and externally visible mutation order
	if(MBlur_pBufVertCount >= NUM_RENDER_FX)
		return false;

	(*rect.x) = Max((*rect.x), 0);
	(*rect.y) = Max((*rect.y), 0);
	(*rect.w) = Min((*rect.w), SCREEN_WIDTH);
	(*rect.h) = Min((*rect.h), SCREEN_HEIGHT);
	if((*rect.x) >= (*rect.w) || (*rect.y) >= (*rect.h))
		return false;

	switch(type){
	// Scope refraction-specific locals to their shared effect cases // rouz edit (ChatGPT)
	case FXTYPE_WATER1:
	case FXTYPE_WATER2:
	case FXTYPE_BLOOD1:
	case FXTYPE_BLOOD2:
	case FXTYPE_HEATHAZE: { // rouz edit (ChatGPT)
		// Match the duplicate-effect margin to the scaled screen coordinates
		//+ rouz edit (ChatGPT)
		const float effectPaddingX = SCREEN_SCALE_X(10.0f);
		const float effectPaddingY = SCREEN_SCALE_Y(10.0f);
		for(int i = 0; i < MBlur_pBufVertCount; i++)
			if(fxType[i] == type && MBlur_PosInside(rect, fxRect[i].x-effectPaddingX, fxRect[i].y-effectPaddingY,
				fxRect[i].w+effectPaddingX, fxRect[i].h+effectPaddingY))
				return false;
		//- rouz edit (ChatGPT)
		// Keep screen-space effect exclusions aligned with the scaled HUD regions
		//+ rouz edit (ChatGPT)
		// Reserve the radar area using the same virtual-screen scaling as the HUD
		if(MBlur_PosInside(rect, SCREEN_SCALE_X(40.0f), SCREEN_SCALE_FROM_BOTTOM(116.0f),
			SCREEN_SCALE_X(40.0f + 94.0f), SCREEN_SCALE_FROM_BOTTOM(116.0f - 76.0f)))
			return false;
		// Reserve the top-right HUD region from refractive overlays
		if(MBlur_PosInside(rect, SCREEN_SCALE_X(400.0f), 0.0f, SCREEN_WIDTH, SCREEN_SCALE_Y(90.0f)))
			return false;
		// Reserve the bottom-right vehicle and zone labels
		if(MBlur_HostVehicleHud() != 0 && MBlur_PosInside(rect, SCREEN_WIDTH/2, SCREEN_SCALE_Y(350.0f), SCREEN_WIDTH, SCREEN_HEIGHT))
			return false;
		if(MBlur_HostZoneHud() != 0 && MBlur_PosInside(rect, SCREEN_WIDTH/2, SCREEN_SCALE_Y(350.0f), SCREEN_WIDTH, SCREEN_HEIGHT))
			return false;
		//- rouz edit (ChatGPT)
		break;
	} // rouz edit (ChatGPT)
	// Protect HUD and radar pixels from the screen-space splash overlays
	//+ rouz edit (ChatGPT)
	case FXTYPE_SPLASH1:
	case FXTYPE_SPLASH2:
	case FXTYPE_SPLASH3:
		if(MBlur_PosInside(rect, SCREEN_SCALE_X(40.0f), SCREEN_SCALE_FROM_BOTTOM(116.0f),
			SCREEN_SCALE_X(40.0f + 94.0f), SCREEN_SCALE_FROM_BOTTOM(116.0f - 76.0f)) ||
		   MBlur_PosInside(rect, SCREEN_SCALE_X(400.0f), 0.0f, SCREEN_WIDTH, SCREEN_SCALE_Y(90.0f)) ||
		   ((MBlur_HostVehicleHud() != 0 || MBlur_HostZoneHud() != 0) &&
			MBlur_PosInside(rect, SCREEN_WIDTH/2, SCREEN_SCALE_Y(350.0f), SCREEN_WIDTH, SCREEN_HEIGHT)))
			return false;
		break;
	//- rouz edit (ChatGPT)
	}

	fxRect[MBlur_pBufVertCount] = MBlur_CopyRect(rect);
	fxZ[MBlur_pBufVertCount] = z;
	fxType[MBlur_pBufVertCount] = type;
	MBlur_pBufVertCount++;

	return true;
}

typedef struct MBlurFxContext {void *camera,*frontBuffer;} MBlurFxContext;
static void
MBlur_OverlayFxInScope(void *opaque,void *vertices)
{
    // Run the complete original effect policy while native vertex storage retains its original stack lifetime
    MBlurFxContext *context=(MBlurFxContext*)opaque;void *cam=context->camera,*frontBuf=context->frontBuffer;

	// Render queued screen refraction and splash effects directly into the CPU frame
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	if(MBlur_HostFramePending() && MBlur_HostCapturingWorldEffects()){
		// Skip the screen snapshot when no particles queued a refractive effect
		//+ rouz edit (ChatGPT)
		if(MBlur_pBufVertCount <= 0)
			return;
		//- rouz edit (ChatGPT)
		// Discard queued effects if the source framebuffer cannot be snapshotted
		if(!MBlur_HostBeginScreenTextureSampling()){
			MBlur_pBufVertCount = 0;
			return;
		}
		const int screenWidth = SCREEN_WIDTH;
		const int screenHeight = SCREEN_HEIGHT;
		int red = Clamp((int)((0.75f*TimeCycle_GetDirectionalRed() + TimeCycle_GetAmbientRed())*0.55f*255.0f), 0, 255);
		int green = Clamp((int)((0.75f*TimeCycle_GetDirectionalGreen() + TimeCycle_GetAmbientGreen())*0.55f*255.0f), 0, 255);
		int blue = Clamp((int)((0.75f*TimeCycle_GetDirectionalBlue() + TimeCycle_GetAmbientBlue())*0.55f*255.0f), 0, 255);
		// Apply each queued effect to the same scene snapshot before post-processing
		for(int i = 0; i < MBlur_pBufVertCount; i++){
			const float left = (float)fxRect[i].x;
			const float top = (float)fxRect[i].y;
			const float right = (float)fxRect[i].w;
			const float bottom = (float)fxRect[i].h;
			// Refract the saved scene through a heat haze rectangle
			if(fxType[i] == FXTYPE_HEATHAZE){
				// Suppress heat haze while a full-screen fade is active
				if(MBlur_HostFade() == FADE_0){
					int alpha = MBlur_HostBrightness() > 255 ?
						MBlur_HostBrightness()-90 : MBlur_HostBrightness()-130;
					alpha = Clamp(alpha, 16, 200)/2;
					const float leftUOffset = General_GetRandomNumberInRangeFloat(-0.002f, 0.002f);
					const float topVOffset = General_GetRandomNumberInRangeFloat(-0.002f, 0.002f);
					const float rightUOffset = General_GetRandomNumberInRangeFloat(-0.002f, 0.002f);
					const float bottomVOffset = General_GetRandomNumberInRangeFloat(-0.002f, 0.002f);
					MBlur_HostRefraction(left, top, right, bottom,
						screenWidth, screenHeight, MBlur_HostRaster(MBLUR_HEATHAZE,0), leftUOffset, topVOffset,
						rightUOffset, bottomVOffset, alpha);
				}
			// Refract and texture the water or blood drop rectangle
			}else if(fxType[i] == FXTYPE_WATER1 || fxType[i] == FXTYPE_WATER2 ||
				 fxType[i] == FXTYPE_BLOOD1 || fxType[i] == FXTYPE_BLOOD2){
				const bool secondFrame = fxType[i] == FXTYPE_WATER2 || fxType[i] == FXTYPE_BLOOD2;
				const bool blood = fxType[i] == FXTYPE_BLOOD1 || fxType[i] == FXTYPE_BLOOD2;
				// Keep blood overlays red while water drops inherit the current scene lighting
				//+ rouz edit (ChatGPT)
				const int tintRed = blood ? 255 : red;
				const int tintGreen = blood ? 0 : green;
				const int tintBlue = blood ? 0 : blue;
				//- rouz edit (ChatGPT)
				const float textureWidth = (float)powf(2.0f, (int32_t)(log2(MBlur_HostWidth(MBlur_HostCameraRaster(cam))))+1);
				const float textureHeight = (float)powf(2.0f, (int32_t)(log2(MBlur_HostHeight(MBlur_HostCameraRaster(cam))))+1);
				const float leftUOffset = (right-left)/textureWidth;
				const float rightUOffset = leftUOffset-(right-left+0.5f)*0.66f/textureWidth;
				const float topVOffset = (bottom-top+0.5f)*0.25f/textureHeight;
				const float bottomVOffset = -topVOffset;
				const int refractionStrength = MBlur_BlurOn ? (blood ? 255 : 160) : (blood ? 128 : 32);
				MBlur_HostRefraction(left, top, right, bottom,
					screenWidth, screenHeight, MBlur_HostRaster(MBLUR_DOT,0), leftUOffset, topVOffset,
					rightUOffset, bottomVOffset, refractionStrength);
				MBlur_HostScreenTexture(left, top, right, bottom,
					screenWidth, screenHeight, MBlur_HostRaster(MBLUR_RAIN,secondFrame ? 1 : 0),
					tintRed, tintGreen, tintBlue, blood ? 255 : 192, true);
				// Apply the dark water-drop texture after its bright reflection
				if(!blood)
					MBlur_HostScreenTexture(left, top, right, bottom,
						screenWidth, screenHeight, MBlur_HostRaster(MBLUR_DARKRAIN,secondFrame ? 1 : 0),
						red, green, blue, 96, false);
			// Draw splash textures with the original additive tint
			}else if(fxType[i] == FXTYPE_SPLASH1 || fxType[i] == FXTYPE_SPLASH2 || fxType[i] == FXTYPE_SPLASH3){
				MBlur_HostScreenTexture(left, top, right, bottom,
					screenWidth, screenHeight, MBlur_HostRaster(MBLUR_SPLASH,0), 200, 200, 200, 255, true);
			}
		}
		MBlur_pBufVertCount = 0;
		return;
	}
	//+ rouz edit (ChatGPT)
	// Discard queued software effects instead of drawing them outside the CPU framebuffer
	MBlur_pBufVertCount = 0;
	return;
	//- rouz edit (ChatGPT)
#endif
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Keep the direct RenderWare backbuffer path exclusive to hardware renderer builds
#ifndef REVC_SOFTWARE_POLYGONS
	bool drawWaterDrops = false;
	void *verts=vertices;
	int red = (0.75f*TimeCycle_GetDirectionalRed() + TimeCycle_GetAmbientRed())*0.55f * 255;
	int green = (0.75f*TimeCycle_GetDirectionalGreen() + TimeCycle_GetAmbientGreen())*0.55f * 255;
	int blue = (0.75f*TimeCycle_GetDirectionalBlue() + TimeCycle_GetAmbientBlue())*0.55f * 255;
	red = Clamp(red, 0, 255);
	green = Clamp(green, 0, 255);
	blue = Clamp(blue, 0, 255);

	MBlur_HostState(MBLUR_STATE_ZTESTENABLE, (void*)1);
	MBlur_HostState(MBLUR_STATE_VERTEXALPHAENABLE, (void*)1);

#ifdef LIBRW
	MBlur_HostStencil(MBLUR_STENCILENABLE, 1);
#else
	MBlur_HostStencil(MBLUR_STENCILENABLE, 1);
#endif

	for(int i = 0; i < MBlur_pBufVertCount; i++)
		switch(fxType[i]){
		case FXTYPE_WATER1:
		case FXTYPE_WATER2:
		case FXTYPE_BLOOD1:
		case FXTYPE_BLOOD2: {
			drawWaterDrops = true;
			int32_t width  = powf(2.0f, (int32_t)(log2(MBlur_HostWidth (MBlur_HostCameraRaster(cam))))+1);
			int32_t height = powf(2.0f, (int32_t)(log2(MBlur_HostHeight(MBlur_HostCameraRaster(cam))))+1);

			float u1Off = (fxRect[i].w - fxRect[i].x)/width;
			float u2Off = u1Off - (fxRect[i].w - fxRect[i].x + 0.5f)*0.66f/width;
			float halfHeight = (fxRect[i].h - fxRect[i].y + 0.5f)*0.25f/height;

			if(MBlur_HostDepth(MBlur_HostCameraRaster(cam)) == 16){
				if(fxType[i] == FXTYPE_BLOOD1 || fxType[i] == FXTYPE_BLOOD2)
					MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(255, 0, 0, 128), 0.0f, 0.0f, 0.0f, 0.0f, fxZ[i], true);
				else
					MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(32, 32, 32, 225), 0.0f, 0.0f, 0.0f, 0.0f, fxZ[i], true);
			}else{
				MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(32, 32, 32, 225), 0.0f, 0.0f, 0.0f, 0.0f, fxZ[i], true);
			}

			MBlur_HostState(MBLUR_STATE_TEXTURERASTER, MBlur_HostRaster(MBLUR_DOT,0));
#ifdef LIBRW
			MBlur_HostStencil(MBLUR_STENCILFUNCTION, MBLUR_STENCILALWAYS);
			MBlur_HostStencil(MBLUR_STENCILFUNCTIONREF, 1);
			MBlur_HostStencil(MBLUR_STENCILFUNCTIONMASK, 0xFFFFFFFF);
			MBlur_HostStencil(MBLUR_STENCILFUNCTIONWRITEMASK, 0xFFFFFFFF);
			MBlur_HostStencil(MBLUR_STENCILZFAIL, MBLUR_STENCILKEEP);
			MBlur_HostStencil(MBLUR_STENCILFAIL, MBLUR_STENCILKEEP);
			MBlur_HostStencil(MBLUR_STENCILPASS, MBLUR_STENCILREPLACE);
#else
			MBlur_HostStencil(MBLUR_STENCILFUNCTION, MBLUR_STENCILALWAYS);
			MBlur_HostStencil(MBLUR_STENCILFUNCTIONREF, 1);
			MBlur_HostStencil(MBLUR_STENCILFUNCTIONMASK, 0xFFFFFFFF);
			MBlur_HostStencil(MBLUR_STENCILFUNCTIONWRITEMASK, 0xFFFFFFFF);
			MBlur_HostStencil(MBLUR_STENCILZFAIL, MBLUR_STENCILKEEP);
			MBlur_HostStencil(MBLUR_STENCILFAIL, MBLUR_STENCILKEEP);
			MBlur_HostStencil(MBLUR_STENCILPASS, MBLUR_STENCILREPLACE);
#endif
			MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_SRCALPHA);
			MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_ONE);
			MBlur_HostDraw( verts, 4, Index, 6);

			if(MBlur_HostDepth(MBlur_HostCameraRaster(cam)) != 16){
				MBlur_HostState(MBLUR_STATE_TEXTURERASTER, frontBuf);
#ifdef LIBRW
				MBlur_HostStencil(MBLUR_STENCILFUNCTION, MBLUR_STENCILEQUAL);
				MBlur_HostStencil(MBLUR_STENCILPASS, MBLUR_STENCILKEEP);
#else
				MBlur_HostStencil(MBLUR_STENCILFUNCTION, MBLUR_STENCILEQUAL);
				MBlur_HostStencil(MBLUR_STENCILPASS, MBLUR_STENCILKEEP);
#endif
				if(MBlur_BlurOn){
					if(fxType[i] == FXTYPE_BLOOD1 || fxType[i] == FXTYPE_BLOOD2)
						MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(255, 0, 0, 255), u1Off, 0.0f+halfHeight, u2Off, 0.0f-halfHeight, fxZ[i], false);
					else
						MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(225, 225, 225, 160), u1Off, 0.0f+halfHeight, u2Off, 0.0f-halfHeight, fxZ[i], false);
					MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_DESTALPHA);
					MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_INVDESTALPHA);
				}else{
					if(fxType[i] == FXTYPE_BLOOD1 || fxType[i] == FXTYPE_BLOOD2)
						MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(255, 0, 0, 128), u1Off, 0.0f+halfHeight, u2Off, 0.0f-halfHeight, fxZ[i], false);
					else
						MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(128, 128, 128, 32), u1Off, 0.0f+halfHeight, u2Off, 0.0f-halfHeight, fxZ[i], false);
					MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_SRCALPHA);
					MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_INVSRCALPHA);
				}
				MBlur_HostDraw( verts, 4, Index, 6);
			}
			break;
		}
		case FXTYPE_SPLASH1:
		case FXTYPE_SPLASH2:
		case FXTYPE_SPLASH3:
			drawWaterDrops = true;
			break;

		case FXTYPE_HEATHAZE:
			if(MBlur_HostFade() == FADE_0 && frontBuf){
				int alpha = MBlur_HostBrightness() > 255 ?
					MBlur_HostBrightness() - 90 :
					MBlur_HostBrightness() - 130;
				alpha = Clamp(alpha, 16, 200)/2;

				MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(0, 0, 0, alpha), 0.0f, 0.0f, 0.0f, 0.0f, fxZ[i], true);
				MBlur_HostState(MBLUR_STATE_TEXTURERASTER, MBlur_HostRaster(MBLUR_HEATHAZE,0));
#ifdef LIBRW
				MBlur_HostStencil(MBLUR_STENCILFUNCTION, MBLUR_STENCILALWAYS);
				MBlur_HostStencil(MBLUR_STENCILFUNCTIONREF, 1);
				MBlur_HostStencil(MBLUR_STENCILFUNCTIONMASK, 0xFFFFFFFF);
				MBlur_HostStencil(MBLUR_STENCILFUNCTIONWRITEMASK, 0xFFFFFFFF);
				MBlur_HostStencil(MBLUR_STENCILZFAIL, MBLUR_STENCILKEEP);
				MBlur_HostStencil(MBLUR_STENCILFAIL, MBLUR_STENCILKEEP);
				MBlur_HostStencil(MBLUR_STENCILPASS, MBLUR_STENCILREPLACE);
#else
				MBlur_HostStencil(MBLUR_STENCILFUNCTION, MBLUR_STENCILALWAYS);
				MBlur_HostStencil(MBLUR_STENCILFUNCTIONREF, 1);
				MBlur_HostStencil(MBLUR_STENCILFUNCTIONMASK, 0xFFFFFFFF);
				MBlur_HostStencil(MBLUR_STENCILFUNCTIONWRITEMASK, 0xFFFFFFFF);
				MBlur_HostStencil(MBLUR_STENCILZFAIL, MBLUR_STENCILKEEP);
				MBlur_HostStencil(MBLUR_STENCILFAIL, MBLUR_STENCILKEEP);
				MBlur_HostStencil(MBLUR_STENCILPASS, MBLUR_STENCILREPLACE);
#endif
				MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_SRCALPHA);
				MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_ONE);
				MBlur_HostDraw( verts, 4, Index, 6);

				MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(255, 255, 255, alpha),
					General_GetRandomNumberInRangeFloat(-0.002f, 0.002f),
					General_GetRandomNumberInRangeFloat(-0.002f, 0.002f),
					General_GetRandomNumberInRangeFloat(-0.002f, 0.002f),
					General_GetRandomNumberInRangeFloat(-0.002f, 0.002f),
					fxZ[i], false);
				MBlur_HostState(MBLUR_STATE_TEXTURERASTER, frontBuf);
#ifdef LIBRW
				MBlur_HostStencil(MBLUR_STENCILFUNCTION, MBLUR_STENCILEQUAL);
				MBlur_HostStencil(MBLUR_STENCILPASS, MBLUR_STENCILKEEP);
#else
				MBlur_HostStencil(MBLUR_STENCILFUNCTION, MBLUR_STENCILEQUAL);
				MBlur_HostStencil(MBLUR_STENCILPASS, MBLUR_STENCILKEEP);
#endif
				MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_SRCALPHA);
				MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_INVSRCALPHA);
				MBlur_HostDraw( verts, 4, Index, 6);
			}
			break;
		}
#ifdef LIBRW
	MBlur_HostStencil(MBLUR_STENCILENABLE, 0);
#else
	MBlur_HostStencil(MBLUR_STENCILENABLE, 0);
#endif

	if(drawWaterDrops){
		MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_SRCALPHA);
		MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_ONE);

		// Draw drops
		MBlur_HostState(MBLUR_STATE_TEXTURERASTER, MBlur_HostRaster(MBLUR_RAIN,0));
		for(int i = 0; i < MBlur_pBufVertCount; i++)
			if(fxType[i] == FXTYPE_WATER1 || fxType[i] == FXTYPE_BLOOD1){
				MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(red, green, blue, fxType[i] == FXTYPE_BLOOD1 ? 255 : 192),
					0.0f, 0.0f, 0.0f, 0.0f, fxZ[i], true);
				MBlur_HostDraw( verts, 4, Index, 6);
			}
		MBlur_HostState(MBLUR_STATE_TEXTURERASTER, MBlur_HostRaster(MBLUR_RAIN,1));
		for(int i = 0; i < MBlur_pBufVertCount; i++)
			if(fxType[i] == FXTYPE_WATER2 || fxType[i] == FXTYPE_BLOOD2){
				MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(red, green, blue, fxType[i] == FXTYPE_BLOOD2 ? 255 : 192),
					0.0f, 0.0f, 0.0f, 0.0f, fxZ[i], true);
				MBlur_HostDraw( verts, 4, Index, 6);
			}

		MBlur_HostState(MBLUR_STATE_TEXTURERASTER, MBlur_HostRaster(MBLUR_SPLASH,0));
		for(int i = 0; i < MBlur_pBufVertCount; i++)
			if(fxType[i] == FXTYPE_SPLASH1 || fxType[i] == FXTYPE_SPLASH2 || fxType[i] == FXTYPE_SPLASH3){
				MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(200, 200, 200, 255),
					0.0f, 0.0f, 0.0f, 0.0f, fxZ[i], true);
				MBlur_HostDraw( verts, 4, Index, 6);
			}

		// Darken the water drops
		int alpha = 192*0.5f;
		MBlur_HostState(MBLUR_STATE_SRCBLEND, (void*)MBLUR_BLEND_SRCALPHA);
		MBlur_HostState(MBLUR_STATE_DESTBLEND, (void*)MBLUR_BLEND_INVSRCALPHA);

		MBlur_HostState(MBLUR_STATE_TEXTURERASTER, MBlur_HostRaster(MBLUR_DARKRAIN,0));
		for(int i = 0; i < MBlur_pBufVertCount; i++)
			if(fxType[i] == FXTYPE_WATER1){
				MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(red, green, blue, alpha),
					0.0f, 0.0f, 0.0f, 0.0f, fxZ[i], true);
				MBlur_HostDraw( verts, 4, Index, 6);
			}
		MBlur_HostState(MBLUR_STATE_TEXTURERASTER, MBlur_HostRaster(MBLUR_DARKRAIN,1));
		for(int i = 0; i < MBlur_pBufVertCount; i++)
			if(fxType[i] == FXTYPE_WATER2){
				MBlur_CreateImmediateModeVertices(cam, MBlur_RectRef(&fxRect[i]), verts, MBlur_MakeColour(red, green, blue, alpha),
					0.0f, 0.0f, 0.0f, 0.0f, fxZ[i], true);
				MBlur_HostDraw( verts, 4, Index, 6);
			}
	}

	MBlur_HostState(MBLUR_STATE_ZTESTENABLE, (void*)0);
	MBlur_HostState(MBLUR_STATE_VERTEXALPHAENABLE, (void*)0);
	MBlur_pBufVertCount = 0;
#endif
	//- rouz edit (ChatGPT)
}

//- rouz edit (ChatGPT)

void MBlur_OverlayRenderFx(void *camera,void *frontBuffer)
{
    // Borrow a distinct native SDK vertex array for each recursive hardware effect invocation
    MBlurFxContext context={camera,frontBuffer};
#ifdef REVC_SOFTWARE_POLYGONS
    MBlur_OverlayFxInScope(&context,NULL);
#else
    MBlur_HostWithVertices(MBlur_OverlayFxInScope,&context);
#endif
}

//- rouz edit (ChatGPT)
