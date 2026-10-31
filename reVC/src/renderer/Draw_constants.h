//+ rouz edit (ChatGPT)
#pragma once
#include "config.h"
#if defined(PROPER_SCALING)
	#ifdef FORCE_PC_SCALING
		#define DEFAULT_SCREEN_WIDTH  (640)
		#define DEFAULT_SCREEN_HEIGHT (448)
	#else
		#define DEFAULT_SCREEN_WIDTH  (640)
		#define DEFAULT_SCREEN_HEIGHT (480)
	#endif
#elif defined(GTA_PS2)
		#define DEFAULT_SCREEN_WIDTH  (640)
		#define DEFAULT_SCREEN_HEIGHT (480)
#else //elif defined(GTA_PC)
		#define DEFAULT_SCREEN_WIDTH  (640)
		#define DEFAULT_SCREEN_HEIGHT (448)
#endif

#define DEFAULT_ASPECT_RATIO (4.0f/3.0f)
#define DEFAULT_VIEWWINDOW (0.7f)

#define SCREEN_HEIGHT_PAL ((float)512)
#define SCREEN_HEIGHT_NTSC ((float)448)

#define PI (float)M_PI
#define TWOPI (PI*2)
#define HALFPI (PI/2)
#define DEGTORAD(x) ((x) * PI / 180.0f)
#define RADTODEG(x) ((x) * 180.0f / PI)
//- rouz edit (ChatGPT)
