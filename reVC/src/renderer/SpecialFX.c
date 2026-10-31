//+ rouz edit (ChatGPT)
#include "SpecialFX_backend.h"
#include "Markers3D.h"
#include "BrightLights.h"
#include "ShinyTexts.h"
#include "MoneyMessages.h"
#include "BulletTraces.h"
#include "Timer.h"
#include "General.h"
#include "Draw.h"
#include "Draw_constants.h"
#include "FontStyles.h"
#include "WeaponType.h"
#include <stdio.h>
#include <stddef.h>

#define SCREEN_WIDTH (SpecialFX_BackendScreenWidth())
#define SCREEN_HEIGHT (SpecialFX_BackendScreenHeight())
#define SCREEN_ASPECT_RATIO (Draw_GetAspectRatio())
#define SCREEN_STRETCH_X(a)   ((a) * (float) SCREEN_WIDTH / DEFAULT_SCREEN_WIDTH)
#define SCREEN_STRETCH_Y(a)   ((a) * (float) SCREEN_HEIGHT / DEFAULT_SCREEN_HEIGHT)
#define SCREEN_STRETCH_FROM_RIGHT(a)  (SCREEN_WIDTH - SCREEN_STRETCH_X(a))
#define SCREEN_STRETCH_FROM_BOTTOM(a) (SCREEN_HEIGHT - SCREEN_STRETCH_Y(a))

// This scales from PS2 pixel coordinates while optionally maintaining the aspect ratio
#define SCREEN_SCALE_X(a) SCREEN_SCALE_AR(SCREEN_STRETCH_X(a))
#define SCREEN_SCALE_Y(a) SCREEN_STRETCH_Y(a)
#define SCREEN_SCALE_FROM_RIGHT(a) (SCREEN_WIDTH - SCREEN_SCALE_X(a))
#define SCREEN_SCALE_FROM_BOTTOM(a) (SCREEN_HEIGHT - SCREEN_SCALE_Y(a))

#ifdef ASPECT_RATIO_SCALE
#define SCREEN_SCALE_AR(a) ((a) * DEFAULT_ASPECT_RATIO / SCREEN_ASPECT_RATIO)
#define SCALE_AND_CENTER_X(x) ((SCREEN_WIDTH == DEFAULT_SCREEN_WIDTH) ? (x) : (SCREEN_WIDTH - SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH)) / 2 + SCREEN_SCALE_X((x)))
#ifdef PROPER_SCALING
	#ifndef FORCE_PC_SCALING			
		#undef SCREEN_SCALE_Y	
		#define SCREEN_SCALE_Y(a) Draw_ScaleY(SCREEN_STRETCH_Y(a))	
	#endif
#endif
#else
#define SCREEN_SCALE_AR(a) (a)
#define SCALE_AND_CENTER_X(x) SCREEN_STRETCH_X(x)
#endif

#ifdef MORE_LANGUAGES
#define SPECIAL_FX_FONT_LOCALE(style) (SpecialFX_BackendJapanese()?FONT_JAPANESE:(style))
#else
#define SPECIAL_FX_FONT_LOCALE(style) (style)
#endif
static MotionBlurStreakVector SpecialFX_Transform(void *matrix,MotionBlurStreakVector v)
{
    // Retain the complete original matrix-vector products and additions in C
    SpecialFXMatrix m=SpecialFX_BackendReadMatrix(matrix);
    MotionBlurStreakVector result={m.rx*v.x+m.fx*v.y+m.ux*v.z+m.px,m.ry*v.x+m.fy*v.y+m.uy*v.z+m.py,m.rz*v.x+m.fz*v.y+m.uz*v.z+m.pz};return result;
}
bool TimerHost_SnapshotActive(void)
{
    // Read the same live snapshot flag now owned by the C controller
    return SpecialFX_bSnapShotActive;
}
bool SpecialFX_bVideoCam;
bool SpecialFX_bLiftCam;
bool SpecialFX_bSnapShotActive;
int32_t SpecialFX_SnapShotFrames;


void
SpecialFX_Init(void)
{
	// Preserve the complete original effect, overlay and state-update order
	// Enter the C bullet-trace algorithm with the original owner expressions
	BulletTraces_Init();

	// Initialize trace UV fields at their original shared-effect setup point
	BulletTraces_InitGeometry();

	// Initialize streak geometry at the original point in the shared effect setup
	MotionBlurStreaks_InitGeometry();

	// Enter the C motion-blur streak algorithm with the original owner expressions
	MotionBlurStreaks_Init();
	// Enter the C bright-light algorithm with the original arguments
	BrightLights_Init();
	// Enter the C shiny-text algorithm at the original effect pass
	ShinyTexts_Init();
	// Enter the C money-message algorithm with the original arguments
	MoneyMessages_Init();
	// Enter the C 3D-marker controller with the original arguments
	Markers3D_Init();
	SpecialFX_bSnapShotActive = false;
	SpecialFX_bVideoCam = false;
	SpecialFX_SnapShotFrames = 0;
	SpecialFX_bLiftCam = false;
	// Acquire the trace texture at its original point in shared effect initialization
	BulletTraces_InitTexture();
}

void
SpecialFX_AddWeaponStreak(int type)
{
	// Preserve the complete original effect, overlay and state-update order
	void *matrix=SpecialFX_BackendWeaponMatrix();
	MotionBlurStreakVector start;
	MotionBlurStreakVector end;

	if (SpecialFX_BackendPlayerPed() != NULL && SpecialFX_BackendWeapon(SpecialFX_BackendPlayerPed()) != NULL) {
		// Retain original live reads, short circuits and writes
		switch (type) {
			// Retain original live reads, short circuits and writes
		case WEAPONTYPE_BASEBALLBAT:
			SpecialFX_BackendSetMatrix(matrix,SpecialFX_BackendFrameLTM(SpecialFX_BackendAtomicFrame(SpecialFX_BackendWeapon(SpecialFX_BackendPlayerPed()))));
			start = SpecialFX_Transform(matrix,(MotionBlurStreakVector){0.02f, 0.05f, 0.07f});
			end = SpecialFX_Transform(matrix,(MotionBlurStreakVector){0.246f, 0.0325f, 0.796f});
			break;
		case WEAPONTYPE_GOLFCLUB:
			SpecialFX_BackendSetMatrix(matrix,SpecialFX_BackendFrameLTM(SpecialFX_BackendAtomicFrame(SpecialFX_BackendWeapon(SpecialFX_BackendPlayerPed()))));
			start = SpecialFX_Transform(matrix,(MotionBlurStreakVector){0.02f, 0.05f, 0.07f});
			end = SpecialFX_Transform(matrix,(MotionBlurStreakVector){-0.054f, 0.0325f, 0.796f});
			break;
		case WEAPONTYPE_KATANA:
			SpecialFX_BackendSetMatrix(matrix,SpecialFX_BackendFrameLTM(SpecialFX_BackendAtomicFrame(SpecialFX_BackendWeapon(SpecialFX_BackendPlayerPed()))));
			start = SpecialFX_Transform(matrix,(MotionBlurStreakVector){0.02f, 0.05f, 0.07f});
			end = SpecialFX_Transform(matrix,(MotionBlurStreakVector){0.096f, -0.0175f, 1.096f});
			break;
		default:
			return;
		}
		// Enter the C motion-blur streak algorithm with the original owner expressions
		MotionBlurStreaks_RegisterStreak((uintptr_t)SpecialFX_BackendWeapon(SpecialFX_BackendPlayerPed()), 100, 100, 100, start, end);
	}
}

void *
SpecialFX_LookForBat(void *object,void *data)
{
	// Preserve the complete original effect, overlay and state-update order
	void *MatLTM=SpecialFX_BackendCallbackMatrix();

	if(SpecialFX_BackendModelInfo(object) == data){
		// Retain original live reads, short circuits and writes
		SpecialFX_BackendSetMatrix(MatLTM,SpecialFX_BackendFrameLTM(SpecialFX_BackendAtomicFrame(object)));
		MotionBlurStreakVector p1 = SpecialFX_Transform(MatLTM,(MotionBlurStreakVector){0.02f, 0.05f, 0.07f});
		MotionBlurStreakVector p2 = SpecialFX_Transform(MatLTM,(MotionBlurStreakVector){0.246f, 0.0325f, 0.796f});
		// Enter the C motion-blur streak algorithm with the original owner expressions
		MotionBlurStreaks_RegisterStreak((uintptr_t)object, 100, 100, 100, p1, p2);
	}
	return NULL;
}

void
SpecialFX_Update(void)
{
	// Preserve the complete original effect, overlay and state-update order
	// Enter the C motion-blur streak algorithm with the original owner expressions
	MotionBlurStreaks_Update();
	// Enter the C bullet-trace algorithm with the original owner expressions
	BulletTraces_Update();
}

void
SpecialFX_Shutdown(void)
{
	// Preserve the complete original effect, overlay and state-update order
	// Enter the C 3D-marker controller with the original arguments
	Markers3D_Shutdown();
	// Release the trace texture at its original shutdown point
	BulletTraces_ShutdownTexture();
}

void
SpecialFX_Render(void)
{
	// Preserve the complete original effect, overlay and state-update order
	SpecialFX_BackendPush("CSpecialFX::Render");
	// Enter the C motion-blur streak algorithm with the original owner expressions
	MotionBlurStreaks_Render();
	// Enter the C bullet-trace algorithm with the original owner expressions
	BulletTraces_Render();
	// Enter the C bright-light algorithm with the original arguments
	BrightLights_Render(false);
	// Enter the C shiny-text algorithm at the original effect pass
	ShinyTexts_Render();
	// Enter the C money-message algorithm with the original arguments
	MoneyMessages_Render();
#ifdef NEW_RENDERER
	if(!(SpecialFX_BackendNewRenderer() && SpecialFX_BackendFirstPerson()))
#endif
	// Enter the C 3D-marker controller with the original arguments
	Markers3D_Render();
	SpecialFX_BackendPop();
}

void
SpecialFX_Render2DFXs(void)
{
	// Preserve the complete original effect, overlay and state-update order
	if (SpecialFX_bVideoCam) {
		// Retain original live reads, short circuits and writes
		SpecialFX_BackendFontScale(SCREEN_SCALE_X(1.5f), SCREEN_SCALE_Y(1.5f));
		SpecialFX_BackendFontJustifyOff();
		SpecialFX_BackendFontBackgroundOff();
		SpecialFX_BackendFontCentreSize(SCREEN_SCALE_X(620.0f)); // unused
		SpecialFX_BackendFontCentreOff();
		SpecialFX_BackendFontPropOn();
		SpecialFX_BackendFontColor(0, 255, 0, 200);
		SpecialFX_BackendFontStyle(SPECIAL_FX_FONT_LOCALE(FONT_STANDARD));
		sprintf(SpecialFX_BackendString(), "%d", (*Timer_GetFrameCounter()) & 0x3F); // mb % 63
		SpecialFX_BackendAsciiToUnicode(SpecialFX_BackendString(), SpecialFX_BackendUnicode());
		SpecialFX_BackendPrintString(SCREEN_WIDTH * 8 / 10, SCREEN_HEIGHT * 8 / 10, SpecialFX_BackendUnicode());
		for (int32_t i = 0; i < SCREEN_HEIGHT; i += 4) {
			// Retain original live reads, short circuits and writes
			SpecialFX_BackendSrcBlend(true);
			SpecialFX_BackendDestBlend(true);
			SpecialFX_BackendPolygon(0.0f, i, SCREEN_WIDTH, i, 0.0f, i+1, SCREEN_WIDTH, i+1,0, 100, 0, 100);
			SpecialFX_BackendSrcBlend(false);
			SpecialFX_BackendDestBlend(false);
			SpecialFX_BackendPolygon(0.0f, i+2, SCREEN_WIDTH, i+2, 0.0f, i+3, SCREEN_WIDTH, i+3,0, 0, 0, 150);
		}
		int32_t tmp = ((*Timer_GetTimeInMilliseconds()) & 0x7ff) * (SCREEN_HEIGHT + 70.0f) / 2048 - 70.0f; //mb % 2048
		SpecialFX_BackendSrcBlend(false);
		SpecialFX_BackendDestBlend(false);
		SpecialFX_BackendPolygon(0.0, tmp, SCREEN_WIDTH, tmp, 0.0, tmp + 70.0f, SCREEN_WIDTH, tmp + 70.0f ,0, 100, 0, 60);
	}
	if (SpecialFX_bLiftCam) {
		// Retain original live reads, short circuits and writes
		SpecialFX_BackendFontScale(SCREEN_SCALE_X(1.5f), SCREEN_SCALE_Y(1.5f));
		SpecialFX_BackendFontJustifyOff();
		SpecialFX_BackendFontBackgroundOff();
		SpecialFX_BackendFontCentreSize(SCREEN_SCALE_X(620.0f)); // unused
		SpecialFX_BackendFontCentreOff();
		SpecialFX_BackendFontPropOn();
		SpecialFX_BackendFontColor(100, 100, 100, 200);
		SpecialFX_BackendFontStyle(SPECIAL_FX_FONT_LOCALE(FONT_STANDARD));
		for (int32_t i = 0; i < SCREEN_HEIGHT; i += 4) {
			// Retain original live reads, short circuits and writes
			SpecialFX_BackendSrcBlend(true);
			SpecialFX_BackendDestBlend(true);
			SpecialFX_BackendPolygon(0.0f, i, SCREEN_WIDTH, i, 0.0f, i + 1, SCREEN_WIDTH, i + 1,100, 100, 100, 100);
			SpecialFX_BackendSrcBlend(false);
			SpecialFX_BackendDestBlend(false);
			SpecialFX_BackendPolygon(0.0f, i + 2, SCREEN_WIDTH, i + 2, 0.0f, i + 3, SCREEN_WIDTH, i + 3,0, 0, 0, 150);
		}
		int32_t tmp = ((*Timer_GetTimeInMilliseconds()) & 0x7ff) * (SCREEN_HEIGHT + 70.0f) / 2048 - 70.0f; //mb % 2048
		SpecialFX_BackendSrcBlend(false);
		SpecialFX_BackendDestBlend(false);
		SpecialFX_BackendPolygon(0.0, tmp, SCREEN_WIDTH, tmp, 0.0, tmp + 70.0f, SCREEN_WIDTH, tmp + 70.0f,100, 100, 100, 60);
		for (int32_t i = 0; i < 200; i++) {
			// Retain original live reads, short circuits and writes
			int32_t posX = General_GetRandomNumber() % (int32_t)SCREEN_WIDTH;
			int32_t posY = General_GetRandomNumber() % (int32_t)SCREEN_HEIGHT;
			SpecialFX_BackendRect(posX, posY + 2, posX+20, posY,255, 255, 255, 64);
		}
	}
	if (SpecialFX_bSnapShotActive) {
		// Retain original live reads, short circuits and writes
		if (++SpecialFX_SnapShotFrames > 20) {
			// Retain original live reads, short circuits and writes
			SpecialFX_bSnapShotActive = false;
			Timer_SetTimeScale(1.0f);
		} else {
			// Retain original live reads, short circuits and writes
			Timer_SetTimeScale(0.0f); //in andro it's 0.00001
			if (SpecialFX_SnapShotFrames < 10) {
				// Retain original live reads, short circuits and writes
				int32_t tmp = (255 - 255 * SpecialFX_SnapShotFrames / 10) * 0.65f;
				SpecialFX_BackendSrcBlend(true);
				SpecialFX_BackendDestBlend(true);
				SpecialFX_BackendPolygon(0.0f, 0.0f, SCREEN_WIDTH, 0.0f, 0.0f, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT,tmp, tmp, tmp, tmp);
				SpecialFX_BackendSrcBlend(false);
				SpecialFX_BackendDestBlend(false);
			}
		}
	}
}
//- rouz edit (ChatGPT)
