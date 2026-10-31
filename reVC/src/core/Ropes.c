//+ rouz edit (ChatGPT)
#include "Ropes_backend.h"
#include "Timer.h"
#include <math.h>

#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
#define ROPES_CLAMP(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))
static RopesVector Ropes_Add(RopesVector left, RopesVector right)
{
	// Preserve the original componentwise vector addition
	RopesVector result = {left.x+right.x, left.y+right.y, left.z+right.z};
	return result;
}
static RopesVector Ropes_Subtract(RopesVector left, RopesVector right)
{
	// Preserve the original componentwise vector subtraction
	RopesVector result = {left.x-right.x, left.y-right.y, left.z-right.z};
	return result;
}
static RopesVector Ropes_Scale(RopesVector vector, float scalar)
{
	// Multiply components in the same operand order as vector times scalar
	RopesVector result = {vector.x*scalar, vector.y*scalar, vector.z*scalar};
	return result;
}
static RopesVector Ropes_LeftScale(float scalar, RopesVector vector)
{
	// Multiply components in the same operand order as scalar times vector
	RopesVector result = {scalar*vector.x, scalar*vector.y, scalar*vector.z};
	return result;
}
static RopesVector Ropes_Divide(RopesVector vector, float scalar)
{
	// Divide every original component without folding the timestep expression
	RopesVector result = {vector.x/scalar, vector.y/scalar, vector.z/scalar};
	return result;
}
static float Ropes_Magnitude(RopesVector vector)
{
	// Use the probed original float sum and sqrtf wrapper
	return sqrtf(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);
}

static Rope aRopes[8];

uint16_t RopeIndices[64] = {
	0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7,
	7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15,
	15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23,
	23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31,
	31, 32	// unused
};

static bool
IsRopeVisibleToActiveCamera(RopesVector center, float radius)
{
	// Let the active auxiliary camera clip ropes during environment rendering
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(Ropes_BackendEnvMap())
		return true;
#endif
	return Ropes_BackendSphereVisible(center, radius);
}

void
Rope_Update(Rope *rope)
{
	// Preserve the original live timer reads and sequential node physics
	int i;
	float step = powf(0.85f, (*Timer_GetTimeStep()));
	if(!rope->m_bWasRegistered && (*Timer_GetTimeInMilliseconds()) > rope->m_updateTimer){
		// Preserve the original rope traversal and write order
		rope->m_speed[0].z -= 0.0015f*(*Timer_GetTimeStep());
		rope->m_pos[0] = Ropes_Add(rope->m_pos[0], Ropes_Scale(rope->m_speed[0], (*Timer_GetTimeStep())));
	}
	for(i = 1; i < ARRAY_SIZE(rope->m_pos); i++){
		// Preserve the original rope traversal and write order
		RopesVector prevPos = rope->m_pos[i];
		rope->m_pos[i] = Ropes_Add(rope->m_pos[i], Ropes_Scale(Ropes_Scale(rope->m_speed[i], step), (*Timer_GetTimeStep())));
		rope->m_pos[i].z -= 0.05f*(*Timer_GetTimeStep());
		RopesVector dist = Ropes_Subtract(rope->m_pos[i], rope->m_pos[i-1]);
		rope->m_pos[i] = Ropes_Add(rope->m_pos[i-1], Ropes_LeftScale(0.625f/Ropes_Magnitude(dist), dist));
		rope->m_speed[i] = Ropes_Divide(Ropes_Subtract(rope->m_pos[i], prevPos), (*Timer_GetTimeStep()));
	}
	if(!rope->m_bWasRegistered && rope->m_pos[0].z < 0.0f)
		rope->m_bActive = false;
	rope->m_bWasRegistered = false;
}

void
Rope_Render(Rope *rope)
{
	// Retain sphere visibility and the shared vertex and index draw protocol
	int i;
	int numVerts = 0;
	if(!IsRopeVisibleToActiveCamera(rope->m_pos[16], 20.0f)) // rouz edit (ChatGPT)
		return;

	for(i = 0; i < ARRAY_SIZE(rope->m_pos); i++){
		// Preserve the original rope traversal and write order
		Ropes_BackendSetRGBA(i, 128, 128, 128, 100);
		Ropes_BackendSetPos(i, rope->m_pos[i].x, rope->m_pos[i].y, rope->m_pos[i].z);
	}

	Ropes_BackendVertexAlpha();
	Ropes_BackendSrcBlend();
	Ropes_BackendDestBlend();
	Ropes_BackendTextureRaster();

	if(Ropes_BackendTransform((int)ARRAY_SIZE(rope->m_pos))){
		// Preserve the original rope traversal and write order
#ifdef FIX_BUGS
		Ropes_BackendRenderIndexed(true, RopeIndices, (int)(2*(ARRAY_SIZE(rope->m_pos)-1)));
#else
		Ropes_BackendRenderIndexed(false, RopeIndices, (int)(2*(ARRAY_SIZE(rope->m_pos)-1)));
#endif
		Ropes_BackendEnd();
	}
}


void
Ropes_Init(void)
{
	// Clear only the original active flags
	int i;
	for(i = 0; i < ARRAY_SIZE(aRopes); i++)
		aRopes[i].m_bActive = false;
}

void
Ropes_Update(void)
{
	// Update active records in the original array order
	int i;
	for(i = 0; i < ARRAY_SIZE(aRopes); i++)
		if(aRopes[i].m_bActive)
			Rope_Update(&aRopes[i]);
}

void
Ropes_Render(void)
{
	// Render active records inside the original debug group
	int i;
	Ropes_BackendPush("CRopes::Render");
	for(i = 0; i < ARRAY_SIZE(aRopes); i++)
		if(aRopes[i].m_bActive)
			Rope_Render(&aRopes[i]);
	Ropes_BackendPop();
}

bool
Ropes_RegisterRope(uintptr_t id, RopesVector pos, bool setUpdateTimer)
{
	// Retain first-match updates and first-free record initialization
	int i, j;
	for(i = 0; i < ARRAY_SIZE(aRopes); i++){
		// Preserve the original rope traversal and write order
		if(aRopes[i].m_bActive && aRopes[i].m_id == id){
			// Preserve the original rope traversal and write order
			aRopes[i].m_pos[0] = pos;
			aRopes[i].m_speed[0] = (RopesVector){0.0f, 0.0f, 0.0f};
			aRopes[i].m_bWasRegistered = true;
			return true;
		}
	}
	for(i = 0; i < ARRAY_SIZE(aRopes); i++)
		if(!aRopes[i].m_bActive){
			// Preserve the original rope traversal and write order
			aRopes[i].m_id = id;
			aRopes[i].m_pos[0] = pos;
			aRopes[i].m_speed[0] = (RopesVector){0.0f, 0.0f, 0.0f};
			aRopes[i].m_unk = false;
			aRopes[i].m_bWasRegistered = true;
			aRopes[i].m_updateTimer = setUpdateTimer ? (*Timer_GetTimeInMilliseconds()) + 20000 : 0;
			for(j = 1; j < ARRAY_SIZE(aRopes[0].m_pos); j++){
				// Preserve the original rope traversal and write order
				if(j & 1)
					aRopes[i].m_pos[j] = Ropes_Add(aRopes[i].m_pos[j-1], (RopesVector){0.0f, 0.0f, 0.625f});
				else
					aRopes[i].m_pos[j] = Ropes_Subtract(aRopes[i].m_pos[j-1], (RopesVector){0.0f, 0.0f, 0.625f});
				aRopes[i].m_speed[j] = (RopesVector){0.0f, 0.0f, 0.0f};
			}
			aRopes[i].m_bActive = true;
			return true;
		}
	return false;
}

void
Ropes_SetSpeedOfTopNode(uintptr_t id, RopesVector speed)
{
	// Write only the first matching active rope top speed
	int i;
	for(i = 0; i < ARRAY_SIZE(aRopes); i++)
		if(aRopes[i].m_bActive && aRopes[i].m_id == id){
			// Preserve the original rope traversal and write order
			aRopes[i].m_speed[0] = speed;
			return;
		}
}

bool
Ropes_FindCoorsAlongRope(uintptr_t id, float t, RopesVector *coors)
{
	// Preserve clamp, interpolation order and failure output behavior
	int i, j;
	float f;
	for(i = 0; i < ARRAY_SIZE(aRopes); i++)
		if(aRopes[i].m_bActive && aRopes[i].m_id == id){
			// Preserve the original rope traversal and write order
			t = (ARRAY_SIZE(aRopes[0].m_pos)-1)*ROPES_CLAMP(t, 0.0f, 0.999f);
			j = t;
			f = t - j; 
			*coors = Ropes_Add(Ropes_LeftScale(1.0f-f, aRopes[i].m_pos[j]), Ropes_LeftScale(f, aRopes[i].m_pos[j+1]));
			return true;
		}
	return false;
}

bool
Ropes_CreateRopeWithSwatComingDown(RopesVector pos)
{
	// Preserve lazy loading, rope allocation and ordered SWAT setup
	static uint32_t ropeId = 0;

	if(!Ropes_BackendSwatLoaded() || !Ropes_RegisterRope(ropeId+100, pos, true))
		return false;
	CCopPed *swat = Ropes_BackendAddSwat(pos);
	Ropes_BackendDisableCollision(swat);
	Ropes_BackendRopeSentinel(swat);
	Ropes_BackendRopeID(swat, 100 + ropeId);
	Ropes_BackendBlendSwat(swat);
	ropeId++;
	return true;
}

bool Ropes_FindCoorsAlongRopeToVector(uintptr_t id, float t, void *coors)
{
	// Preserve the legacy output object's contents on every failed lookup
	RopesVector result;
	if (!Ropes_FindCoorsAlongRope(id, t, &result))
		return false;
	Ropes_BackendStoreVector(coors, result);
	return true;
}
//- rouz edit (ChatGPT)
