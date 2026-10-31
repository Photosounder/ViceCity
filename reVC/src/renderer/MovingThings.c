//+ rouz edit (ChatGPT)
#include "MovingThings_backend.h"
#include "Escalators.h"
#include "ScrollBar.h"
#include "Timer.h"
#include "Clock.h"
#include "GameLevels.h"
#include "PlaneTrails.h"
#include "PlaneBanners.h"
#include "SmokeTrails.h"
#include "PointLights.h"
#include "Draw_constants.h"
#include <math.h>
#include <stddef.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define SQR(a) ((a)*(a))
static MovingThingVector MovingThings_Subtract(MovingThingVector a,MovingThingVector b)
{
    // Keep original per-component subtraction and vector value boundaries
    MovingThingVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static float MovingThings_Magnitude(MovingThingVector a)
{
    // Preserve the original square-root argument and left-associated sum order
    return sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
}
static MovingThingVector MovingThings_Divide(MovingThingVector a,float divisor)
{
    // Retain unguarded component division including the original coincident-position behavior
    MovingThingVector result={a.x/divisor,a.y/divisor,a.z/divisor};return result;
}
static MovingThingVector MovingThings_Scale(MovingThingVector a,float scale)
{
    // Preserve the original separate component multiplication before subtraction
    MovingThingVector result={a.x*scale,a.y*scale,a.z*scale};return result;
}
static float MovingThings_Dot(MovingThingVector a,MovingThingVector b)
{
    // Retain the original dot-product expression and float arithmetic order
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
MovingThing MovingThings_StartCloseList;
MovingThing MovingThings_EndCloseList;
int16_t MovingThings_Num;
MovingThing MovingThings_aMovingThings[NUMMOVINGTHINGS];
static float maxUpdateDists[5] = { 100.0f, 1500.0f, 400.0f, 100.0f, 2000.0f };
void MovingThings_Init(void)
{
    // Preserve all original live reads, traversal decisions and state-write ordering
	MovingThings_StartCloseList.m_pNext = &MovingThings_EndCloseList;
	MovingThings_StartCloseList.m_pPrev = NULL;
	MovingThings_EndCloseList.m_pNext = NULL;
	MovingThings_EndCloseList.m_pPrev = &MovingThings_StartCloseList;

	// Enter the C plane-trail algorithm with the original input capture
	PlaneTrails_Init();
	// Use the C smoke-trail state or algorithm at the original call point
	SmokeTrails_Init();
	// Enter the C plane-banner algorithm with the original input capture
	PlaneBanners_Init();
	// Enter the C point-light algorithm with the original owner expressions
	PointLights_Init();

	MovingThings_Num = 0;

	for (int32_t i = 0; i < NUMMOVINGTHINGS; i++) {
		// Preserve the original branch and traversal order
		MovingThings_aMovingThings[i].m_nType = 0;
		MovingThings_aMovingThings[i].m_farAway = 0;
	}

	for (int i = 0; i < MovingThings_BackendSectorsX(); i++) {
		// Preserve the original branch and traversal order
		for (int j = 0; j < MovingThings_BackendSectorsY(); j++) {
			// Preserve the original branch and traversal order
			for (void *pNode=MovingThings_BackendSectorFirst(i,j); pNode; pNode=MovingThings_BackendNodeNext(pNode)) {
				// Preserve the original branch and traversal order
				CEntity *pEntity = MovingThings_BackendNodeEntity(pNode);
				MovingThings_PossiblyAddThisEntity(pEntity);
			}
		}
	}

	for (int32_t i = 0; i < NUM_LEVELS; i++) {
		// Preserve the original branch and traversal order
		for (void *pNode=MovingThings_BackendBigFirst(i); pNode; pNode=MovingThings_BackendNodeNext(pNode)) {
			// Preserve the original branch and traversal order
			CEntity *pEntity = MovingThings_BackendNodeEntity(pNode);
			MovingThings_PossiblyAddThisEntity(pEntity);
		}
	}

	Escalators_Init();
	ScrollBar_Init(&aScrollBars[0],(ScrollBarVector){-1069.209f,1320.126f,18.848f},(ScrollBarVector){-1069.209f,1342.299f,22.612f},0,128,255,0,0.3f);
}

void MovingThings_Shutdown(void)
{
    // Preserve all original live reads, traversal decisions and state-write ordering
	
	ScrollBar_SetVisibility(&aScrollBars[0],false);
	Escalators_Shutdown();
}

void MovingThings_Update(void)
{
    // Preserve all original live reads, traversal decisions and state-write ordering
	// Enter the C plane-banner algorithm with the original input capture
	PlaneBanners_Update();
	// Enter the C plane-trail algorithm with the original input capture
	PlaneTrails_Update();
	Escalators_Update();

	const int TIME_SPAN = 8; // frames to process all MovingThings_aMovingThings

	int16_t i;

	int block = (*Timer_GetFrameCounter()) % TIME_SPAN;

	for (i = (block * NUMMOVINGTHINGS) / TIME_SPAN; i < ((block + 1) * NUMMOVINGTHINGS) / TIME_SPAN; i++) {
		// Preserve the original branch and traversal order
		if (MovingThings_aMovingThings[i].m_farAway == 1)
			MovingThing_Update(&MovingThings_aMovingThings[i]);
	}

	for (i = 0; i < MovingThings_Num; i++) {
		// Preserve the original branch and traversal order
		if (MovingThings_aMovingThings[i].m_farAway == 0)
			MovingThing_Update(&MovingThings_aMovingThings[i]);
	}

	for (i = 0; i < NUMSCROLLBARS; ++i)
	{
		// Preserve the original branch and traversal order
		if (ScrollBar_IsVisible(&aScrollBars[i]) || ((*Timer_GetFrameCounter()) + i) % 8 == 0)
			ScrollBar_Update(&aScrollBars[i]);
	}
}

void MovingThings_Render(void)
{
    // Preserve all original live reads, traversal decisions and state-write ordering
	MovingThings_BackendPush("CMovingThings::Render");
	// Use the C smoke-trail state or algorithm at the original call point
	SmokeTrails_Update();

	int i;
	for (i = 0; i < NUMSCROLLBARS; ++i)
	{
		// Preserve the original branch and traversal order
		if (ScrollBar_IsVisible(&aScrollBars[i]))
			ScrollBar_Render(&aScrollBars[i]);
	}

	// Enter the C plane-trail algorithm with the original input capture
	PlaneTrails_Render();
	// Use the C smoke-trail state or algorithm at the original call point
	SmokeTrails_Render();
	// Enter the C plane-banner algorithm with the original input capture
	PlaneBanners_Render();
	MovingThings_BackendPop();
}

#ifdef REVC_SOFTWARE_POLYGONS
void
MovingThings_RenderForEnvMap(void *camera)
{
    // Preserve all original live reads, traversal decisions and state-write ordering
	// Render stadium message boards using the reflection camera and its own distance range
	for(int i = 0; i < NUMSCROLLBARS; i++)
		ScrollBar_RenderForEnvMap(&aScrollBars[i],camera);
}
#endif

void MovingThings_RegisterOne(CEntity *pEnt, uint16_t nType) {
    // Preserve all original live reads, traversal decisions and state-write ordering
	if (MovingThings_Num >= NUMMOVINGTHINGS)
		return;

	MovingThings_aMovingThings[MovingThings_Num].m_pEntity = pEnt;
	MovingThings_aMovingThings[MovingThings_Num].m_nType = nType;
	MovingThings_aMovingThings[MovingThings_Num].m_farAway = 0;
	MovingThings_aMovingThings[MovingThings_Num].m_vecPosn = MovingThings_BackendPosition(pEnt);
	MovingThing_AddToList(&MovingThings_aMovingThings[MovingThings_Num],&MovingThings_StartCloseList);
	MovingThings_Num++;
}

void MovingThings_PossiblyAddThisEntity(CEntity *pEnt) {
    // Preserve all original live reads, traversal decisions and state-write ordering
	if (MovingThings_BackendModelIndex(pEnt) == MovingThings_BackendModelLIGHTBEAM()) {
		// Preserve the original branch and traversal order
		MovingThings_RegisterOne(pEnt, 1);
	}
	else if (MovingThings_BackendModelIndex(pEnt) == MovingThings_BackendModelAIRPORTRADAR()) {
		// Preserve the original branch and traversal order
		MovingThings_RegisterOne(pEnt, 2);
	}
	else if (MovingThings_BackendModelIndex(pEnt) == MovingThings_BackendModelMALLFAN() || MovingThings_BackendModelIndex(pEnt) == MovingThings_BackendModelHOTELFAN_NIGHT()
		|| MovingThings_BackendModelIndex(pEnt) == MovingThings_BackendModelHOTELFAN_DAY() || MovingThings_BackendModelIndex(pEnt) == MovingThings_BackendModelHOTROOMFAN()) {
			// Preserve the original branch and traversal order
		MovingThings_RegisterOne(pEnt, 3);
	}
	else if (MovingThings_BackendModelIndex(pEnt) == MovingThings_BackendModelBLIMP_NIGHT() || MovingThings_BackendModelIndex(pEnt) == MovingThings_BackendModelBLIMP_DAY()) {
		// Preserve the original branch and traversal order
		MovingThings_RegisterOne(pEnt, 4);
	}
}

void MovingThing_Update(MovingThing *thing)
{
    // Preserve all original live reads, traversal decisions and state-write ordering
	switch (thing->m_nType) {
		// Preserve the original branch and traversal order
	case 1: {
		// Preserve the original branch and traversal order
		float angle = ((*Timer_GetTimeInMilliseconds()) % 0x3FFF) * TWOPI / 0x3FFF;
		float s = sinf(angle);
		float c = cosf(angle);
		MovingThings_BackendSetRight(thing->m_pEntity,-s, c, 0.0f);
		MovingThings_BackendSetForward(thing->m_pEntity,0.0f, 0.0f, 1.0f);
		MovingThings_BackendSetUp(thing->m_pEntity,c, s, 0.0f);

		if (Clock_GetHours() >= 20 || Clock_GetHours() < 5) {
			// Preserve the original branch and traversal order
			if (fabsf(MovingThings_BackendCameraAxis(0) - MovingThings_BackendPositionAxis(thing->m_pEntity,0)) < 600.0f &&
				fabsf(MovingThings_BackendCameraAxis(1) - MovingThings_BackendPositionAxis(thing->m_pEntity,1)) < 600.0f) {
					// Preserve the original branch and traversal order
				MovingThingVector delta=MovingThings_Subtract(MovingThings_BackendPosition(thing->m_pEntity),MovingThings_BackendCameraPosition());
				delta=MovingThings_Divide(delta,MovingThings_Magnitude(delta));

				if (MovingThings_Dot(delta,(MovingThingVector){c,s,0.0f}) < -0.92f) {
					// Preserve the original branch and traversal order
					MovingThingVector coors=MovingThings_Subtract(MovingThings_BackendPosition(thing->m_pEntity),MovingThings_Scale(delta,10.0f));
					MovingThings_BackendCorona(coors);
				}
			}
		}
	}
		break;
	case 2: {
		// Preserve the original branch and traversal order
		float angle = ((*Timer_GetTimeInMilliseconds()) % 0x7FF) * TWOPI / 0x7FF;
		float s = sinf(angle);
		float c = cosf(angle);
		MovingThings_BackendSetRight(thing->m_pEntity,c, s, 0.0f);
		MovingThings_BackendSetForward(thing->m_pEntity,-s, c, 0.0f);
		MovingThings_BackendSetUp(thing->m_pEntity,0.0f, 0.0f, 1.0f);
	}
		break;
	case 3: {
		// Preserve the original branch and traversal order
		float angle = ((*Timer_GetTimeInMilliseconds()) % 0x3FF) * TWOPI / 0x3FF;
		float s = sinf(angle);
		float c = cosf(angle);
		MovingThings_BackendSetRight(thing->m_pEntity,c, s, 0.0f);
		MovingThings_BackendSetForward(thing->m_pEntity,-s, c, 0.0f);
		MovingThings_BackendSetUp(thing->m_pEntity,0.0f, 0.0f, 1.0f);
	}
		break;
	case 4: {
		// Preserve the original branch and traversal order
		float angle = ((*Timer_GetTimeInMilliseconds()) % 0x3FFFF) * TWOPI / 0x3FFFF;
		float s = sinf(angle);
		float c = cosf(angle);
		MovingThings_BackendSetRight(thing->m_pEntity,-c, -s, 0.0f);
		MovingThings_BackendSetForward(thing->m_pEntity,s, -c, 0.0f);
		MovingThings_BackendSetUp(thing->m_pEntity,0.0f, 0.0f, 1.0f);
		MovingThings_BackendSetPosition(thing->m_pEntity,350.0f*c-465.0f,350.0f*s+1163.0f,260.0f);
	}
		break;
	default:
		break;
	}

	MovingThings_BackendUpdateRW(thing->m_pEntity);
	MovingThings_BackendUpdateFrame(thing->m_pEntity);
	
	if (SQR(MovingThings_BackendPositionAxis(thing->m_pEntity,0) - MovingThings_BackendCameraAxis(0)) + SQR(MovingThings_BackendPositionAxis(thing->m_pEntity,1) - MovingThings_BackendCameraAxis(1)) < SQR(maxUpdateDists[thing->m_nType])) {
		// Preserve the original branch and traversal order
		if (thing->m_farAway == 1) {
			// Preserve the original branch and traversal order
			MovingThing_AddToList(thing,&MovingThings_StartCloseList);
			thing->m_farAway = 0;
		}
	}
	else {
		// Preserve the original branch and traversal order
		if (thing->m_farAway == 0) {
			// Preserve the original branch and traversal order
			MovingThing_RemoveFromList(thing);
			thing->m_farAway = 1;
		}
	}
}

void MovingThing_AddToList(MovingThing *thing, MovingThing *pThing)
{
    // Preserve all original live reads, traversal decisions and state-write ordering
	thing->m_pNext = pThing->m_pNext;
	thing->m_pPrev = pThing;
	pThing->m_pNext = thing;
	thing->m_pNext->m_pPrev = thing;
}

void MovingThing_RemoveFromList(MovingThing *thing)
{
    // Preserve all original live reads, traversal decisions and state-write ordering
	thing->m_pNext->m_pPrev = thing->m_pPrev;
	thing->m_pPrev->m_pNext = thing->m_pNext;
}

int16_t MovingThing_SizeList(MovingThing *thing)
{
    // Preserve all original live reads, traversal decisions and state-write ordering
	MovingThing *next = thing->m_pNext;
	int16_t count = 0;

	while (next != NULL) {
		// Preserve the original branch and traversal order
		next = next->m_pNext;
		count++;
	}

	return count;
}
//- rouz edit (ChatGPT)
