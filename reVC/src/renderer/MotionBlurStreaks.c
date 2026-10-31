//+ rouz edit (ChatGPT)
#include "MotionBlurStreaks_backend.h"

static uint16_t StreakIndexList[12];

void MotionBlurStreaks_InitGeometry(void)
{
	// Preserve the original ordered sample, pool and shared-geometry operations
	MotionBlurStreaks_BackendSetU(0, 0.0f);
	MotionBlurStreaks_BackendSetV(0, 0.0f);
	MotionBlurStreaks_BackendSetU(1, 1.0f);
	MotionBlurStreaks_BackendSetV(1, 0.0f);
	MotionBlurStreaks_BackendSetU(2, 0.0f);
	MotionBlurStreaks_BackendSetV(2, 0.0f);
	MotionBlurStreaks_BackendSetU(3, 1.0f);
	MotionBlurStreaks_BackendSetV(3, 0.0f);
	StreakIndexList[0] = 0;
	StreakIndexList[1] = 1;
	StreakIndexList[2] = 2;
	StreakIndexList[3] = 1;
	StreakIndexList[4] = 3;
	StreakIndexList[5] = 2;
	StreakIndexList[6] = 0;
	StreakIndexList[7] = 2;
	StreakIndexList[8] = 1;
	StreakIndexList[9] = 1;
	StreakIndexList[10] = 2;
	StreakIndexList[11] = 3;
}

static MotionBlurStreak aStreaks[NUMMBLURSTREAKS];

void
MotionBlurStreak_Update(MotionBlurStreak *streak)
{
	// Preserve the original ordered sample, pool and shared-geometry operations
	int i;
	bool wasUpdated;
	bool lastWasUpdated = false;
	for(i = 2; i > 0; i--){
		// Retain the original short circuits, initialization and write order
		streak->m_pos1[i] = streak->m_pos1[i-1];
		streak->m_pos2[i] = streak->m_pos2[i-1];
		streak->m_isValid[i] = streak->m_isValid[i-1];
		wasUpdated = true;
		if(!lastWasUpdated && !streak->m_isValid[i])
			wasUpdated = false;
		lastWasUpdated = wasUpdated;
	}
	streak->m_isValid[0] = false;
	if(!wasUpdated)
		streak->m_id = 0;
}

void
MotionBlurStreak_Render(MotionBlurStreak *streak)
{
	// Preserve the original ordered sample, pool and shared-geometry operations
	int i;
	int a1, a2;
	for(i = 0; i < 2; i++)
		if(streak->m_isValid[i] && streak->m_isValid[i+1]){
			// Retain the original short circuits, initialization and write order
			a1 = (255/3)*(3-i)/3;
			MotionBlurStreaks_BackendSetRGBA(0, streak->m_red, streak->m_green, streak->m_blue, a1);
			MotionBlurStreaks_BackendSetRGBA(1, streak->m_red, streak->m_green, streak->m_blue, a1);
			a2 = (255/3)*(3-(i+1))/3;
			MotionBlurStreaks_BackendSetRGBA(2, streak->m_red, streak->m_green, streak->m_blue, a2);
			MotionBlurStreaks_BackendSetRGBA(3, streak->m_red, streak->m_green, streak->m_blue, a2);
			MotionBlurStreaks_BackendSetPos(0, streak->m_pos1[i].x, streak->m_pos1[i].y, streak->m_pos1[i].z);
			MotionBlurStreaks_BackendSetPos(1, streak->m_pos2[i].x, streak->m_pos2[i].y, streak->m_pos2[i].z);
			MotionBlurStreaks_BackendSetPos(2, streak->m_pos1[i+1].x, streak->m_pos1[i+1].y, streak->m_pos1[i+1].z);
			MotionBlurStreaks_BackendSetPos(3, streak->m_pos2[i+1].x, streak->m_pos2[i+1].y, streak->m_pos2[i+1].z);
			MotionBlurStreaks_BackendLittleTest();
			if(MotionBlurStreaks_BackendTransform()){
				// Retain the original short circuits, initialization and write order
				MotionBlurStreaks_BackendRenderIndexed(StreakIndexList);
				MotionBlurStreaks_BackendEnd();
			}
		}
}

void
MotionBlurStreaks_Init(void)
{
	// Preserve the original ordered sample, pool and shared-geometry operations
	int i;
	for(i = 0; i < NUMMBLURSTREAKS; i++)
		aStreaks[i].m_id = 0;
}

void
MotionBlurStreaks_Update(void)
{
	// Preserve the original ordered sample, pool and shared-geometry operations
	int i;
	for(i = 0; i < NUMMBLURSTREAKS; i++)
		if(aStreaks[i].m_id != 0)
			MotionBlurStreak_Update(&aStreaks[i]);
}

void
MotionBlurStreaks_RegisterStreak(uintptr_t id, uint8_t r, uint8_t g, uint8_t b, MotionBlurStreakVector p1, MotionBlurStreakVector p2)
{
	// Preserve the original ordered sample, pool and shared-geometry operations
	int i;

	for(i = 0; i < NUMMBLURSTREAKS; i++){
		// Retain the original short circuits, initialization and write order
		if(aStreaks[i].m_id == id){
			// Retain the original short circuits, initialization and write order
			// Found a streak from last frame, update
			aStreaks[i].m_red = r;
			aStreaks[i].m_green = g;
			aStreaks[i].m_blue = b;
			aStreaks[i].m_pos1[0] = p1;
			aStreaks[i].m_pos2[0] = p2;
			aStreaks[i].m_isValid[0] = true;
			return;
		}
	}

	// Find free slot
	for(i = 0; aStreaks[i].m_id != 0 ; i++)
		if(i == NUMMBLURSTREAKS-1)
			return;

	// Create a new streak
	aStreaks[i].m_id = id;
	aStreaks[i].m_red = r;
	aStreaks[i].m_green = g;
	aStreaks[i].m_blue = b;
	aStreaks[i].m_pos1[0] = p1;
	aStreaks[i].m_pos2[0] = p2;
	aStreaks[i].m_isValid[0] = true;
	aStreaks[i].m_isValid[1] = false;
	aStreaks[i].m_isValid[2] = false;
}

bool
MotionBlurStreaks_IsRegisteredForCurrentFrame(uintptr_t id)
{
	// Preserve the original ordered sample, pool and shared-geometry operations
	// Find a matching streak whose leading sample was refreshed this frame
	for(int i = 0; i < NUMMBLURSTREAKS; i++)
		if(aStreaks[i].m_id == id)
			return aStreaks[i].m_isValid[0];
	return false;
}

void
MotionBlurStreaks_Render(void)
{
	// Preserve the original ordered sample, pool and shared-geometry operations
	bool setRenderStates = false;
	int i;
	for(i = 0; i < NUMMBLURSTREAKS; i++)
		if(aStreaks[i].m_id != 0){
			// Retain the original short circuits, initialization and write order
			if(!setRenderStates){
				// Retain the original short circuits, initialization and write order
				MotionBlurStreaks_BackendZWrite(false);
				MotionBlurStreaks_BackendVertexAlpha(true);
				MotionBlurStreaks_BackendFog(true);
				MotionBlurStreaks_BackendFogColor(((uint32_t)255 << 24) | ((uint32_t)MotionBlurStreaks_BackendFogRed() << 16) | ((uint32_t)MotionBlurStreaks_BackendFogGreen() << 8) | (uint32_t)MotionBlurStreaks_BackendFogBlue());
				MotionBlurStreaks_BackendSrcBlend();
				MotionBlurStreaks_BackendDestBlend();
				MotionBlurStreaks_BackendRaster();
 				setRenderStates = true;
			}
			MotionBlurStreak_Render(&aStreaks[i]);
		}
	if(setRenderStates){
		// Retain the original short circuits, initialization and write order
		MotionBlurStreaks_BackendZWrite(true);
		MotionBlurStreaks_BackendVertexAlpha(false);
		MotionBlurStreaks_BackendFog(false);
	}
}
//- rouz edit (ChatGPT)
