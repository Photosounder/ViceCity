//+ rouz edit (ChatGPT)
#include "ReplayState.h"
#include "RwHelper.h"
#include "Skidmarks_backend.h"
#include "Timer.h"
#include <math.h>
#include <stddef.h>

typedef struct SkidmarksVector2 { float x, y; } SkidmarksVector2;
typedef struct SkidmarksColor { uint8_t red, green, blue, alpha; } SkidmarksColor;
#ifdef _MSC_VER
float fast_absf(float x);
#define Skidmarks_Abs fast_absf
#else
#define Skidmarks_Abs fabsf
#endif
static void Skidmarks_Normalise(SkidmarksVector2 *vector)
{
	// Retain the original float reciprocal root and x-only fallback
	float sq = vector->x*vector->x + vector->y*vector->y;
	if (sq > 0.0f) {
		// Scale the original two components in order
		float invsqrt = 1.0f/sqrtf(sq);
		vector->x *= invsqrt;
		vector->y *= invsqrt;
	} else
		vector->x = 1.0f;
}
static float Skidmarks_Dot2(SkidmarksVector2 left, SkidmarksVector2 right)
{
	// Retain the original multiply and add order
	return left.x*right.x + left.y*right.y;
}

static Skidmark aSkidmarks[NUMSKIDMARKS];

uint16_t SkidmarkIndexList[SKIDMARK_LENGTH * 6];
static void *gpSkidTex;

void
Skidmarks_Init(void)
{
	// Preserve texture acquisition, partial record initialization and the entire index table
	int i, ix, slot;
	Skidmarks_BackendTxdPush();
	slot = Skidmarks_BackendTxdFind("particle");
	Skidmarks_BackendTxdSet(slot);
	gpSkidTex = RwHelper_ReadTexture("particleskid", NULL);
	Skidmarks_BackendTxdPop();

	for(i = 0; i < NUMSKIDMARKS; i++){
		// Retain the original skidmark traversal and write order
		aSkidmarks[i].m_state = 0;
		aSkidmarks[i].m_wasUpdated = false;
	}

	ix = 0;
	for(i = 0; i < SKIDMARK_LENGTH; i++){
		// Retain the original skidmark traversal and write order
		SkidmarkIndexList[i*6+0] = ix+0;
		SkidmarkIndexList[i*6+1] = ix+2;
		SkidmarkIndexList[i*6+2] = ix+1;
		SkidmarkIndexList[i*6+3] = ix+1;
		SkidmarkIndexList[i*6+4] = ix+2;
		SkidmarkIndexList[i*6+5] = ix+3;
		ix += 2;
	}
}

void
Skidmarks_Shutdown(void)
{
	// Destroy then clear the original texture handle
	RwHelper_DestroyTexture(gpSkidTex);
	gpSkidTex = NULL;
}

void
Skidmarks_Clear(void)
{
	// Clear only the original state and updated flags
	int i;
	for(i = 0; i < NUMSKIDMARKS; i++){
		// Retain the original skidmark traversal and write order
		aSkidmarks[i].m_state = 0;
		aSkidmarks[i].m_wasUpdated = false;
	}
}

void
Skidmarks_Update(void)
{
	// Preserve unsigned fade deadlines, strict expiry and updated-flag clearing
	int i;
	uint32_t t1 = (*Timer_GetTimeInMilliseconds()) + 2500;
	uint32_t t2 = (*Timer_GetTimeInMilliseconds()) + 5000;
	uint32_t t3 = (*Timer_GetTimeInMilliseconds()) + 10000;
	uint32_t t4 = (*Timer_GetTimeInMilliseconds()) + 20000;
	for(i = 0; i < NUMSKIDMARKS; i++){
		// Retain the original skidmark traversal and write order
		switch(aSkidmarks[i].m_state){
			// Retain the original skidmark traversal and write order
		case 1:
			if(!aSkidmarks[i].m_wasUpdated){
				// Retain the original skidmark traversal and write order
				// Didn't continue this one last time, so finish it and set fade times
				aSkidmarks[i].m_state = 2;
				if(aSkidmarks[i].m_last < 4){
					// Retain the original skidmark traversal and write order
					aSkidmarks[i].m_fadeStart = t1;
					aSkidmarks[i].m_fadeEnd = t2;
				}else if(aSkidmarks[i].m_last < 9){
					// Retain the original skidmark traversal and write order
					aSkidmarks[i].m_fadeStart = t2;
					aSkidmarks[i].m_fadeEnd = t3;
				}else{
					// Retain the original skidmark traversal and write order
					aSkidmarks[i].m_fadeStart = t3;
					aSkidmarks[i].m_fadeEnd = t4;
				}
			}
			break;
		case 2:
			if((*Timer_GetTimeInMilliseconds()) > aSkidmarks[i].m_fadeEnd)
				aSkidmarks[i].m_state = 0;
			break;
		}
		aSkidmarks[i].m_wasUpdated = false;
	}
}

void
Skidmarks_Render(void)
{
	// Retain unsigned fade math and every original vertex and render operation
	int i, j;

	Skidmarks_BackendPush("CSkidmarks::Render");

	Skidmarks_BackendZWrite(false);
	Skidmarks_BackendVertexAlpha(true);
	Skidmarks_BackendSrcBlend();
	Skidmarks_BackendDestBlend();
	RwHelper_SetTextureRaster(RwHelper_TextureRaster(gpSkidTex));

	for(i = 0; i < NUMSKIDMARKS; i++){
		// Retain the original skidmark traversal and write order
		if(aSkidmarks[i].m_state == 0 || aSkidmarks[i].m_last < 1)
			continue;

		SkidmarksColor color = {0, 0, 0, 255};
		switch(aSkidmarks[i].m_type){
			// Retain the original skidmark traversal and write order
		case SKIDMARK_NORMAL: color = (SkidmarksColor){0, 0, 0, 255}; break;
		case SKIDMARK_MUDDY: color = (SkidmarksColor){90, 62, 9, 255}; break;
		case SKIDMARK_SANDY: color = (SkidmarksColor){108, 108, 96, 255}; break;
		case SKIDMARK_BLOODY: color = (SkidmarksColor){132, 34, 11, 255}; break;
		}

		uint32_t fade, alpha;
		if(aSkidmarks[i].m_state == 1 || (*Timer_GetTimeInMilliseconds()) < aSkidmarks[i].m_fadeStart)
			fade = 255;
		else
			fade = 255*(aSkidmarks[i].m_fadeEnd - (*Timer_GetTimeInMilliseconds())) / (aSkidmarks[i].m_fadeEnd - aSkidmarks[i].m_fadeStart);

		for(j = 0; j <= aSkidmarks[i].m_last; j++){
			// Retain the original skidmark traversal and write order
			alpha = 128;
			if(j == 0 || j == aSkidmarks[i].m_last && aSkidmarks[i].m_state == 2)
				alpha = 0;
			alpha = alpha*fade/256;

			SkidmarksVector p1 = aSkidmarks[i].m_pos[j];
			p1.x += aSkidmarks[i].m_sideX[j];
			p1.y += aSkidmarks[i].m_sideY[j];
			SkidmarksVector p2 = aSkidmarks[i].m_pos[j];
			p2.x -= aSkidmarks[i].m_sideX[j];
			p2.y -= aSkidmarks[i].m_sideY[j];
			Skidmarks_BackendSetRGBA(j*2+0, color.red, color.green, color.blue, alpha);
			Skidmarks_BackendSetPos(j*2+0, p1.x, p1.y, p1.z+0.1f);
			Skidmarks_BackendSetU(j*2+0, 0.0f);
			Skidmarks_BackendSetV(j*2+0, j*5.01f);
			Skidmarks_BackendSetRGBA(j*2+1, color.red, color.green, color.blue, alpha);
			Skidmarks_BackendSetPos(j*2+1, p2.x, p2.y, p2.z+0.1f);
			Skidmarks_BackendSetU(j*2+1, 1.0f);
			Skidmarks_BackendSetV(j*2+1, j*5.01f);
		}

		Skidmarks_BackendLittleTest();
		if(Skidmarks_BackendTransform(2*(aSkidmarks[i].m_last+1))){
			// Retain the original skidmark traversal and write order
			Skidmarks_BackendRenderIndexed(SkidmarkIndexList, 6*aSkidmarks[i].m_last);
			Skidmarks_BackendEnd();
		}
	}

	Skidmarks_BackendVertexAlpha(false);
	Skidmarks_BackendZWrite(true);
	Skidmarks_BackendZTest();

	Skidmarks_BackendPop();
}

void
Skidmarks_RegisterOneFlags(uintptr_t id, const void *pos, float fwdX, float fwdY, bool *isMuddy, bool *isBloody)
{
	// Preserve blood priority and lazy muddy-pointer reads before forwarding
	eSkidmarkType type;
	if(*isBloody)
		type = SKIDMARK_BLOODY;
	else if(*isMuddy)
		type = SKIDMARK_MUDDY;
	else
		type = SKIDMARK_NORMAL;
	Skidmarks_RegisterOneType(id, pos, fwdX, fwdY, type, isBloody);
}

void
Skidmarks_RegisterOneType(uintptr_t id, const void *pos, float fwdX, float fwdY, eSkidmarkType type, bool *isBloody)
{
	// Preserve replay rejection, borrowed position timing and original continuation rules
	int i;
	SkidmarksVector2 fwd = {fwdX, fwdY};

	if(Replay_IsPlayingBack())
		return;

	// Find a skidmark to continue
	for(i = 0; i < NUMSKIDMARKS; i++)
		if(aSkidmarks[i].m_state == 1 && aSkidmarks[i].m_id == id)
			break;

	if(i < NUMSKIDMARKS){
		// Retain the original skidmark traversal and write order
		// Continue this one

		if((aSkidmarks[i].m_type==SKIDMARK_BLOODY) != *isBloody){
			// Retain the original skidmark traversal and write order
			// Blood-status changed, end this one
			aSkidmarks[i].m_state = 2;
			aSkidmarks[i].m_fadeStart = (*Timer_GetTimeInMilliseconds()) + 10000;
			aSkidmarks[i].m_fadeEnd = (*Timer_GetTimeInMilliseconds()) + 20000;
			return;
		}

		aSkidmarks[i].m_wasUpdated = true;

		if((*Timer_GetTimeInMilliseconds()) - aSkidmarks[i].m_lastUpdate <= 100){
			// Retain the original skidmark traversal and write order
			// Last update was recently, just change last coords
			aSkidmarks[i].m_pos[aSkidmarks[i].m_last] = Skidmarks_BackendReadPosition(pos);
			return;
		}
		aSkidmarks[i].m_lastUpdate = (*Timer_GetTimeInMilliseconds());

		if(aSkidmarks[i].m_last >= SKIDMARK_LENGTH-1){
			// Retain the original skidmark traversal and write order
			// No space to continue, end it
			aSkidmarks[i].m_state = 2;
			aSkidmarks[i].m_fadeStart = (*Timer_GetTimeInMilliseconds()) + 10000;
			aSkidmarks[i].m_fadeEnd = (*Timer_GetTimeInMilliseconds()) + 20000;
			*isBloody = false;	// stpo blood marks at end
			return;
		}
		aSkidmarks[i].m_last++;

		aSkidmarks[i].m_pos[aSkidmarks[i].m_last] = Skidmarks_BackendReadPosition(pos);

		SkidmarksVector2 right = {aSkidmarks[i].m_pos[aSkidmarks[i].m_last].y - aSkidmarks[i].m_pos[aSkidmarks[i].m_last - 1].y,
		                aSkidmarks[i].m_pos[aSkidmarks[i].m_last - 1].x - aSkidmarks[i].m_pos[aSkidmarks[i].m_last].x};

		Skidmarks_Normalise(&right);
		Skidmarks_Normalise(&fwd);
		float turn = Skidmarks_Dot2(fwd, right);
		turn = Skidmarks_Abs(turn) + 1.0f;
		aSkidmarks[i].m_sideX[aSkidmarks[i].m_last] = right.x * turn * 0.125f;
		aSkidmarks[i].m_sideY[aSkidmarks[i].m_last] = right.y * turn * 0.125f;
		if(aSkidmarks[i].m_last == 1){
			// Retain the original skidmark traversal and write order
			aSkidmarks[i].m_sideX[0] = aSkidmarks[i].m_sideX[1];
			aSkidmarks[i].m_sideY[0] = aSkidmarks[i].m_sideY[1];
		}

		if(aSkidmarks[i].m_last > 8)
			*isBloody = false;	// stop blood marks after 8
		return;
	}

	// Start a new one
	for(i = 0; i < NUMSKIDMARKS; i++)
		if(aSkidmarks[i].m_state == 0)
			break;
	if(i < NUMSKIDMARKS){
		// Retain the original skidmark traversal and write order
		// Found a free slot
		aSkidmarks[i].m_state = 1;
		aSkidmarks[i].m_id = id;
		aSkidmarks[i].m_pos[0] = Skidmarks_BackendReadPosition(pos);
		aSkidmarks[i].m_sideX[0] = 0.0f;
		aSkidmarks[i].m_sideY[0] = 0.0f;
		aSkidmarks[i].m_wasUpdated = true;
		aSkidmarks[i].m_last = 0;
		aSkidmarks[i].m_lastUpdate = (*Timer_GetTimeInMilliseconds()) - 1000;
		if(*isBloody)
			aSkidmarks[i].m_type = SKIDMARK_BLOODY;
		else
			aSkidmarks[i].m_type = type;
	}else
		*isBloody = false;	// stop blood marks if no space
}
//- rouz edit (ChatGPT)
