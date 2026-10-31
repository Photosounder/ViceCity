//+ rouz edit (ChatGPT)
#include "PlaneTrails_backend.h"
#include "Clock.h"
#include "Timer.h"
#include "Draw_constants.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
static PlaneTrail aArray[6];
static PlaneTrailVector PlaneTrails_ReadPosition(PlaneTrailsPosition position)
{
    // Capture the original by-value vector before entering trail registration
    PlaneTrailVector value={*position.x,*position.y,*position.z};return value;
}
static uint16_t TrailIndices[32] = {
	0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,
	10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16
};

static void
PlaneTrail_Init(PlaneTrail *trail)
{
    // Preserve all original live reads, timestamp decisions and state-write ordering
	int i;
	for(i = 0; i < ARRAY_SIZE(trail->m_time); i++)
		trail->m_time[i] = 0;
}

static void
PlaneTrail_Render(PlaneTrail *trail, float visibility)
{
    // Preserve all original live reads, timestamp decisions and state-write ordering
	int i;
	int numVerts = 0;
	//+ rouz edit (ChatGPT)
	// Keep the reflection pass from clearing shared plane-trail timestamps
	bool renderOnlyExistingPoints = false;
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	renderOnlyExistingPoints = PlaneTrails_BackendRenderingEnvMap();
#endif
	//- rouz edit (ChatGPT)
	if(!PlaneTrails_BackendSphereVisible(trail->m_pos[0], 1000.0f)) // rouz edit (ChatGPT)
		return;

	int alpha = visibility*110.0f;
	if(alpha == 0)
		return;

	for(i = 0; i < ARRAY_SIZE(trail->m_pos); i++){
		// Preserve the original branch and traversal order
		int32_t time = (*Timer_GetTimeInMilliseconds()) - trail->m_time[i];
		// Skip expired trail points without mutating reflection-shared state
		//+ rouz edit (ChatGPT)
		if(time > 30000){
			// Preserve the original branch and traversal order
			if(!renderOnlyExistingPoints)
				trail->m_time[i] = 0;
			continue;
		}
		//- rouz edit (ChatGPT)
		if(trail->m_time[i] != 0){
			// Preserve the original branch and traversal order
			float fade = (30000.0f - time) / 10000.0f;
			fade = Min(fade, 1.0f);
			PlaneTrails_BackendSetRGBA(numVerts,(int)(alpha*fade));
			PlaneTrails_BackendSetPos(numVerts, trail->m_pos[i].x, trail->m_pos[i].y, trail->m_pos[i].z);
			numVerts++;
		}
	}
	if(numVerts > 1){
		// Preserve the original branch and traversal order
		PlaneTrails_BackendVertexAlpha();
		PlaneTrails_BackendSrcBlend();
		PlaneTrails_BackendDestBlend();
		PlaneTrails_BackendRasterNull();

		if(PlaneTrails_BackendTransform(numVerts)){
			// Preserve the original branch and traversal order
			PlaneTrails_BackendRenderIndexed(TrailIndices, (numVerts-1)*2);
			PlaneTrails_BackendEnd();
		}
	}
}

static void
PlaneTrail_RegisterPoint(PlaneTrail *trail, PlaneTrailVector pos)
{
    // Preserve all original live reads, timestamp decisions and state-write ordering
	int i;
	bool bNewPoint = false;
	if(trail->m_time[0] != 0 && (*Timer_GetTimeInMilliseconds()) - trail->m_time[0] > 2000){
		// Preserve the original branch and traversal order
		bNewPoint = true;
		for(i = ARRAY_SIZE(trail->m_pos)-1; i > 0; i--){
			// Preserve the original branch and traversal order
			trail->m_pos[i] = trail->m_pos[i-1];
			trail->m_time[i] = trail->m_time[i-1];
		}
	}
	trail->m_pos[0] = pos;
	if(bNewPoint || trail->m_time[0] == 0)
		trail->m_time[0] = (*Timer_GetTimeInMilliseconds());
}

void
PlaneTrails_Init(void)
{
    // Preserve all original live reads, timestamp decisions and state-write ordering
	int i;
	for(i = 0; i < ARRAY_SIZE(aArray); i++)
		PlaneTrail_Init(&aArray[i]);
}

static void
PlaneTrails_UpdateWithPosition(PlaneTrailsPosition planePos)
{
    // Preserve all original live reads, timestamp decisions and state-write ordering
	

	(*planePos.x) = 1590.0f * sinf((float)((*Timer_GetTimeInMilliseconds()) & 0x1FFFF)/0x20000 * TWOPI);
	(*planePos.y) = 1200.0f * cosf((float)((*Timer_GetTimeInMilliseconds()) & 0x1FFFF)/0x20000 * TWOPI);
	(*planePos.z) = 550.0f;
	PlaneTrails_RegisterPoint(PlaneTrails_ReadPosition(planePos), 3);
	if(Clock_GetHours() > 22 || Clock_GetHours() < 7){
		// Preserve the original branch and traversal order
		if((*Timer_GetTimeInMilliseconds()) & 0x200)
			PlaneTrails_BackendRegisterCorona(101,planePos.owner);
		else
			PlaneTrails_BackendUpdateCorona(101,planePos.owner);
	}

	(*planePos.x) = 1000.0f * sinf((float)((*Timer_GetTimeInMilliseconds()) & 0x1FFFF)/0x20000 * TWOPI);
	(*planePos.y) = -1600.0f * cosf((float)((*Timer_GetTimeInMilliseconds()) & 0x1FFFF)/0x20000 * TWOPI);
	(*planePos.z) = 500.0f;
	PlaneTrails_RegisterPoint(PlaneTrails_ReadPosition(planePos), 4);
	if(Clock_GetHours() > 22 || Clock_GetHours() < 7){
		// Preserve the original branch and traversal order
		if((*Timer_GetTimeInMilliseconds()) & 0x200)
			PlaneTrails_BackendRegisterCorona(102,planePos.owner);
		else
			PlaneTrails_BackendUpdateCorona(102,planePos.owner);
	}

	(*planePos.x) = 1100.0f * cosf((float)((*Timer_GetTimeInMilliseconds()) & 0x1FFFF)/0x20000 * TWOPI);
	(*planePos.y) = 700.0f * sinf((float)((*Timer_GetTimeInMilliseconds()) & 0x1FFFF)/0x20000 * TWOPI);
	(*planePos.z) = 600.0f;
	PlaneTrails_RegisterPoint(PlaneTrails_ReadPosition(planePos), 5);
	if(Clock_GetHours() > 22 || Clock_GetHours() < 7){
		// Preserve the original branch and traversal order
		if((*Timer_GetTimeInMilliseconds()) & 0x200)
			PlaneTrails_BackendRegisterCorona(103,planePos.owner);
		else
			PlaneTrails_BackendUpdateCorona(103,planePos.owner);
	}
}

void
PlaneTrails_Render(void)
{
    // Preserve all original live reads, timestamp decisions and state-write ordering
	int i;
	float visibility = Min(1.0f-PlaneTrails_BackendFoggyness(), 1.0f-PlaneTrails_BackendCloudCoverage());
	visibility = Min(visibility, 1.0f-PlaneTrails_BackendRain());
	visibility = Min(Max(Max(PlaneTrails_BackendSkyTopRed(), PlaneTrails_BackendSkyTopGreen()), PlaneTrails_BackendSkyTopBlue())/256.0f, visibility);
	if(visibility > 0.0001f)
		for(i = 0; i < ARRAY_SIZE(aArray); i++)
			PlaneTrail_Render(&aArray[i],visibility);
}

void
PlaneTrails_RegisterPoint(PlaneTrailVector pos, uint32_t id)
{
    // Preserve all original live reads, timestamp decisions and state-write ordering
	PlaneTrail_RegisterPoint(&aArray[id],pos);
}
void PlaneTrails_Update(void)
{
    // Keep one actual temporary vector alive across all three original corona calls
    PlaneTrails_BackendWithPosition(PlaneTrails_UpdateWithPosition);
}
void PlaneTrails_RegisterPointVector(const void *position,uint32_t id)
{
    // Capture the original typed input once before any timestamp reads or trail writes
    PlaneTrails_RegisterPoint(PlaneTrails_BackendVector(position),id);
}
//- rouz edit (ChatGPT)
