//+ rouz edit (ChatGPT)
#include "SmokeTrails_backend.h"
#include "Timer.h"
#include <math.h>
#define Min(a,b) ((a)<(b)?(a):(b))
static float SmokeTrails_Distance(SmokeTrailVector a,SmokeTrailVector b)
{
    // Keep the original component subtraction and square-root expression order
    float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;return sqrtf(x*x+y*y+z*z);
}
static SmokeTrailVector SmokeTrails_ReadPoint(SmokeTrailPoint point)
{
    // Capture each original implicit CVector value before entering record registration
    SmokeTrailVector value={*point.x,*point.y,*point.z};return value;
}
static void SmokeTrails_UpdateWithPoints(SmokeTrailPoints points);
static void SmokeTrail_Render(SmokeTrail *trail);
static bool
SmokeTrails_IsSphereVisible(SmokeTrailVector center, float radius)
{
    // Preserve the original live reads, decisions and complete state-write order
	// Let the active auxiliary camera clip effect geometry during environment rendering
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(SmokeTrails_BackendEnvMap())
		return true;
#endif
	return SmokeTrails_BackendSphereVisible(center, radius);
}
bool SmokeTrails_CigOn = false;
static SmokeTrail aSmoke[3];

static uint16_t SmokeTrailIndices[32] = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16 };

static float RandomSmoke[16] = { 10.0f, 5.0f, -1.0f, -9.0f, -7.0f, -1.0f, 0.0f, 3.0f, 6.0f, 7.0f, 4.0f, 2.0f,
5.0f, 7.0f };


static void
SmokeTrail_RegisterPoint(SmokeTrail *trail, SmokeTrailVector regPosition, float opacity) {
    // Preserve the original live reads, decisions and complete state-write order
	bool bAddedNewPoint = false;

	if (trail->m_time[0] && (*Timer_GetTimeInMilliseconds()) - trail->m_time[0] > 150) {
		// Preserve the original branch and traversal order
		bAddedNewPoint = true;
		for (int32_t i = 15; i > 0; i--) {
			// Preserve the original branch and traversal order
			trail->m_pos[i] = trail->m_pos[i - 1];
			trail->m_time[i] = trail->m_time[i - 1];
			trail->m_opacity[i] = trail->m_opacity[i - 1];
		}
		++trail->m_seed;
	}
	trail->m_pos[0] = regPosition;

	if (bAddedNewPoint || !trail->m_time[0]) {
		// Preserve the original branch and traversal order
		trail->m_time[0] = (*Timer_GetTimeInMilliseconds());
		float density = 0.1f / SmokeTrails_Distance(trail->m_pos[1],trail->m_pos[2]);
		trail->m_opacity[1] = opacity * Min(density, 1.0f);
	}
	trail->m_opacity[0] = 0.0f;
}

static void
SmokeTrail_Init(SmokeTrail *trail, int num) {
    // Preserve the original live reads, decisions and complete state-write order
	for (int32_t i = 0; i < 16; i++)
		trail->m_time[i] = 0;
	trail->m_seed = num * 2;
}

void
SmokeTrails_Init(void) {
    // Preserve the original live reads, decisions and complete state-write order
	for (int32_t i = 0; i < 3; i++)
		SmokeTrail_Init(&aSmoke[i],i);
}

void
SmokeTrails_Render(void) {
    // Preserve the original live reads, decisions and complete state-write order
	for (int32_t i = 0; i < 3; i++)
		SmokeTrail_Render(&aSmoke[i]);
}

static void
SmokeTrail_Render(SmokeTrail *trail) {
    // Preserve the original live reads, decisions and complete state-write order
	int numVerts = 0;
	//+ rouz edit (ChatGPT)
	// Keep the reflection pass from clearing shared smoke-trail timestamps
	bool renderOnlyExistingPoints = false;
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	renderOnlyExistingPoints = SmokeTrails_BackendEnvMap();
#endif
	//- rouz edit (ChatGPT)

	if (SmokeTrails_IsSphereVisible(trail->m_pos[0], 10.0f)) { // rouz edit (ChatGPT)
		for (int32_t i = 0; i < 16; i++) {
			// Preserve the original branch and traversal order
			int timeSinceSpawned = (*Timer_GetTimeInMilliseconds()) - trail->m_time[i];

			// Skip expired smoke points without mutating reflection-shared state
			//+ rouz edit (ChatGPT)
			if (timeSinceSpawned > 2250) {
				// Preserve the original branch and traversal order
				if (!renderOnlyExistingPoints)
					trail->m_time[i] = 0;
				continue;
			}
			//- rouz edit (ChatGPT)

			if (trail->m_time[i]) {
				// Preserve the original branch and traversal order
				int alpha = (1.0f - timeSinceSpawned / 2250.0f) * 110.0f * trail->m_opacity[i];
				float offset = timeSinceSpawned * SmokeTrails_BackendWind() * 0.0001f;
				float posX = (trail->m_pos[i].x + timeSinceSpawned * RandomSmoke[(i - trail->m_seed) & 0xF] * 0.00001f) - offset;
				float posY = (trail->m_pos[i].y + timeSinceSpawned * RandomSmoke[(i - trail->m_seed + 5) & 0xF] * 0.00001f) - offset;
				float posZ = trail->m_pos[i].z + timeSinceSpawned * 0.0004f;
				SmokeTrails_BackendSetRGBA(i, 200, 200, 200, alpha);
				SmokeTrails_BackendSetPos(i, posX, posY, posZ);
				numVerts++;
			}
		}
	}

	if (numVerts > 1) {
		// Preserve the original branch and traversal order
		SmokeTrails_BackendVertexAlpha(true);
		SmokeTrails_BackendSrcBlend();
		SmokeTrails_BackendDestBlend();
		SmokeTrails_BackendRasterNull();

		if (SmokeTrails_BackendTransform(numVerts)) {
			// Preserve the original branch and traversal order
			SmokeTrails_BackendRenderIndexed( SmokeTrailIndices, 2 * (numVerts - 1));
			SmokeTrails_BackendEnd();
		}
	}
}

void
SmokeTrails_Update(void) {
    // Preserve the original live reads, decisions and complete state-write order

	if (!SmokeTrails_CigOn || SmokeTrails_BackendFirstPerson() || !SmokeTrails_BackendPlayerPed() ||
		SmokeTrails_BackendPlayerVehicle() || SmokeTrails_BackendCutscene() || !SmokeTrails_BackendClump(SmokeTrails_BackendPlayerPed()))
		return;

    // Keep both actual native points alive until the entire original update returns
    SmokeTrails_BackendWithPoints(SmokeTrails_UpdateWithPoints);
}

static void
SmokeTrails_UpdateWithPoints(SmokeTrailPoints points) {
    // Preserve the original live reads, decisions and complete state-write order
    // Initialize actual native point fields before the original hierarchy queries
    *points.start.x=0.026f;*points.start.y=0.15f;*points.start.z=0.02f;
    *points.end.x=0.026f;*points.end.y=0.05f;*points.end.z=0.02f;

	void *hier = SmokeTrails_BackendHierarchy(SmokeTrails_BackendClump(SmokeTrails_BackendPlayerPed()));
	int32_t idx = SmokeTrails_BackendBoneIndex(hier, SmokeTrails_BackendHeadTag());
	void *head=SmokeTrails_BackendMatrixAt(hier,idx);
	SmokeTrails_BackendTransformPoint(points.start.owner,head);
	SmokeTrails_BackendTransformPoint(points.end.owner,head);

	SmokeTrail_RegisterPoint(&aSmoke[0],SmokeTrails_ReadPoint(points.start), 1.0f);
	SmokeTrail_RegisterPoint(&aSmoke[1],SmokeTrails_ReadPoint(points.start), 0.75f);
	SmokeTrail_RegisterPoint(&aSmoke[2],SmokeTrails_ReadPoint(points.start), 0.5f);

	SmokeTrails_BackendVertexAlpha(false);
	SmokeTrails_BackendSrcBlend();
	SmokeTrails_BackendDestBlend();
	SmokeTrails_BackendRasterNull();

	SmokeTrails_BackendSetRGBA(0, 255, 255, 255, 255);
	SmokeTrails_BackendSetPos(0, (*points.start.x), (*points.start.y), (*points.start.z));
	SmokeTrails_BackendSetRGBA(1, 255, 255, 255, 255);
	SmokeTrails_BackendSetPos(1, (*points.end.x), (*points.end.y), (*points.end.z));

	if (SmokeTrails_BackendTransform(2)) {
		// Preserve the original branch and traversal order
		SmokeTrails_BackendRenderIndexed( SmokeTrailIndices, 2);
		SmokeTrails_BackendEnd();
	}
}
//- rouz edit (ChatGPT)
