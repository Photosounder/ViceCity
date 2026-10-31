//+ rouz edit (ChatGPT)
#include "WaterCannon_backend.h"
#include "Timer.h"
#include "General.h"
#include "AnimationId.h"
#include "ParticleType.h"
#include <math.h>
#include <stddef.h>

#define Min(a,b) ((a) < (b) ? (a) : (b))
#define Max(a,b) ((a) > (b) ? (a) : (b))
void re3_assert(const char *, const char *, unsigned int, const char *);
#ifndef MASTER
#define WC_ASSERT(expr) (void)((!!(expr)) || (re3_assert(#expr, __FILE__, __LINE__, __func__), 0))
#else
#define WC_ASSERT(expr) (expr)
#endif
typedef WaterCannonVector (*WaterCannonRead)(const void *);
static WaterCannonVector WaterCannon_Read(const void *value)
{
	// Read actual C vector storage only at the original ordered copy point
	return *(const WaterCannonVector *)value;
}
static WaterCannonVector WaterCannon_Add(WaterCannonVector a, WaterCannonVector b)
{
	// Retain the original component addition order
	WaterCannonVector result = {a.x+b.x, a.y+b.y, a.z+b.z};
	return result;
}
static WaterCannonVector WaterCannon_Subtract(WaterCannonVector a, WaterCannonVector b)
{
	// Retain all original component subtractions
	WaterCannonVector result = {a.x-b.x, a.y-b.y, a.z-b.z};
	return result;
}
static WaterCannonVector WaterCannon_Scale(WaterCannonVector a, float b)
{
	// Retain original vector-times-scalar operand order
	WaterCannonVector result = {a.x*b, a.y*b, a.z*b};
	return result;
}
static WaterCannonVector WaterCannon_LeftScale(float a, WaterCannonVector b)
{
	// Retain original scalar-times-vector operand order
	WaterCannonVector result = {a*b.x, a*b.y, a*b.z};
	return result;
}
static float WaterCannon_MagnitudeSqr(WaterCannonVector v)
{
	// Retain the original three float products and additions
	return v.x*v.x + v.y*v.y + v.z*v.z;
}
static float WaterCannon_Magnitude(WaterCannonVector v)
{
	// Use the measured original float square root
	return sqrtf(WaterCannon_MagnitudeSqr(v));
}
static float WaterCannon_PedSpeed(CPed *ped)
{
	// Read the original planar speed before computing its float magnitude
	float x = WaterCannon_BackendSpeedX(ped), y = WaterCannon_BackendSpeedY(ped);
	return sqrtf(x*x + y*y);
}
static WaterCannonVector WaterCannon_Cross(WaterCannonVector a, WaterCannonVector b)
{
	// Retain the complete original cross-product expression
	WaterCannonVector result = {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
	return result;
}
static void WaterCannon_Normalise(WaterCannonVector *v)
{
	// Retain the original float reciprocal root and x-only fallback
	float sq = WaterCannon_MagnitudeSqr(*v);
	if (sq > 0.0f) {
		// Scale all three original components in order
		float invsqrt = 1.0f/sqrtf(sq);
		v->x *= invsqrt;v->y *= invsqrt;v->z *= invsqrt;
	} else
		v->x = 1.0f;
}

#define WATERCANNONVERTS 4
#define WATERCANNONINDEXES 12

uint16_t WaterCannonIndexList[WATERCANNONINDEXES];

WaterCannon WaterCannons_aCannons[NUM_WATERCANNONS];

void WaterCannon_Init(WaterCannon *cannon)
{
	// Preserve partial state initialization and every shared UV and index write
	cannon->m_nId = 0;
	cannon->m_nCur = 0;
	cannon->m_nTimeCreated = (*Timer_GetTimeInMilliseconds());
	
	for ( int32_t i = 0; i < WATERCANNON_SEGMENTS; i++ )
		cannon->m_abUsed[i] = false;
	
	WaterCannon_BackendSetU(0, 0.0f);
	WaterCannon_BackendSetV(0, 0.0f);
	
	WaterCannon_BackendSetU(1, 1.0f);
	WaterCannon_BackendSetV(1, 0.0f);
	
	WaterCannon_BackendSetU(2, 0.0f);
	WaterCannon_BackendSetV(2, 0.0f);
	
	WaterCannon_BackendSetU(3, 1.0f);
	WaterCannon_BackendSetV(3, 0.0f);
	
	WaterCannonIndexList[0]  = 0;
	WaterCannonIndexList[1]  = 1;
	WaterCannonIndexList[2]  = 2;
	
	WaterCannonIndexList[3]  = 1;
	WaterCannonIndexList[4]  = 3;
	WaterCannonIndexList[5]  = 2;
	
	WaterCannonIndexList[6]  = 0;
	WaterCannonIndexList[7]  = 2;
	WaterCannonIndexList[8]  = 1;
	
	WaterCannonIndexList[9]  = 1;
	WaterCannonIndexList[10] = 2;
	WaterCannonIndexList[11] = 3;
}

void WaterCannon_Update_OncePerFrame(WaterCannon *cannon, int16_t index)
{
	// Preserve timeout, integration, first fire hit and frame-gated ped updates
	WC_ASSERT(index < NUM_WATERCANNONS);
	
	if ((*Timer_GetTimeInMilliseconds()) > cannon->m_nTimeCreated + WATERCANNON_LIFETIME )
	{
		// Retain the original traversal and write order
		cannon->m_nCur = (cannon->m_nCur + 1) % WATERCANNON_SEGMENTS;
		cannon->m_abUsed[cannon->m_nCur] = false;
	}
	
	for ( int32_t i = 0; i < WATERCANNON_SEGMENTS; i++ )
	{
		// Retain the original traversal and write order
		if ( cannon->m_abUsed[i] )
		{
			// Retain the original traversal and write order
			cannon->m_avecVelocity[i].z += -WATERCANNON_GRAVITY * (*Timer_GetTimeStep());
			cannon->m_avecPos[i] = WaterCannon_Add(cannon->m_avecPos[i], WaterCannon_Scale(cannon->m_avecVelocity[i], (*Timer_GetTimeStep())));
		}
	}
	
	for ( int32_t i = 0; i < WATERCANNON_SEGMENTS; i++ )
	{
		// Retain the original traversal and write order
		if ( cannon->m_abUsed[i] && WaterCannon_BackendExtinguish(cannon->m_avecPos[i], 4.0f) )
		{
			// Retain the original traversal and write order
			break;
		}
	}
	
	if ( ((index + (*Timer_GetFrameCounter())) & 3) == 0 )
		WaterCannon_PushPeds(cannon);
	
	// free if unused
	
	int32_t i = 0;
	while ( 1 )
	{
		// Retain the original traversal and write order
		if ( cannon->m_abUsed[i] )
			break;
		
		if ( ++i >= WATERCANNON_SEGMENTS )
		{
			// Retain the original traversal and write order
			cannon->m_nId = 0;
			return;
		}
	}
}

static void WaterCannon_Input(WaterCannon *cannon, const void *pos, const void *dir, WaterCannonRead read)
{
	// Assert both pointers before copying position then velocity and marking the slot
	WC_ASSERT(pos != NULL);
	WC_ASSERT(dir != NULL);
	
	cannon->m_avecPos[cannon->m_nCur]      = read(pos);
	cannon->m_avecVelocity[cannon->m_nCur] = read(dir);
	cannon->m_abUsed[cannon->m_nCur]       = true;
}

void WaterCannon_Render(WaterCannon *cannon)
{
	// Preserve camera selection, random behavior and circular ribbon traversal
	WaterCannon_BackendZWrite(false);
	WaterCannon_BackendVertexAlpha(true);
	WaterCannon_BackendFog(true);
	WaterCannon_BackendRaster();
	// Orient the spray ribbon to the active camera, including the software reflection camera
	WaterCannonVector cameraForward = WaterCannon_BackendCameraForward();
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(WaterCannon_BackendEnvMap() && WaterCannon_BackendEnvCamera()){
		void *cameraFrame = WaterCannon_BackendCameraFrame(WaterCannon_BackendEnvCamera());
		if(cameraFrame){
			void *cameraMatrix = WaterCannon_BackendFrameMatrix(cameraFrame);
			if(cameraMatrix){
				cameraForward = WaterCannon_BackendMatrixForward(cameraMatrix);
				WaterCannon_Normalise(&cameraForward);
			}
		}
	}
#endif
	
	// Keep the auxiliary reflection draw from consuming gameplay random numbers
	float v;
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(WaterCannon_BackendEnvMap())
		v = (float)(((*Timer_GetFrameCounter()) + cannon->m_nId) & 255) / 256.0f;
	else
#endif
		v = (float)(General_GetRandomNumber() & 255) / 256.0f;
		
	WaterCannon_BackendSetV(0, v);
	WaterCannon_BackendSetV(1, v);
	WaterCannon_BackendSetV(2, v);
	WaterCannon_BackendSetV(3, v);
	
	int16_t pointA = cannon->m_nCur % WATERCANNON_SEGMENTS;
	
	int16_t pointB = pointA - 1;
	if ( pointB < 0 )
		pointB += WATERCANNON_SEGMENTS;

	bool bInit = false;
	WaterCannonVector norm;
	
	for ( int32_t i = 0; i < WATERCANNON_SEGMENTS - 1; i++ )
	{
		// Retain the original traversal and write order
		if ( cannon->m_abUsed[pointA] && cannon->m_abUsed[pointB] )
		{
			// Retain the original traversal and write order
			if ( !bInit )
			{
				// Retain the original traversal and write order
				WaterCannonVector cp = WaterCannon_Cross(WaterCannon_Subtract(cannon->m_avecPos[pointB], cannon->m_avecPos[pointA]), cameraForward); // rouz edit (ChatGPT)
				norm = WaterCannon_Scale(cp, (0.05f / WaterCannon_Magnitude(cp)));
				bInit = true;
			}
			
			float dist       = (float)(i*i*i) / 300.0f + 1.0f;
			float brightness = (float)(i) / WATERCANNON_SEGMENTS;
			
			int32_t color = (int32_t)((1.0f - brightness*brightness) * 255.0f);
			WaterCannonVector offset = WaterCannon_LeftScale(dist, norm);
			
			WaterCannon_BackendSetRGBA(0, color, color, color, color);
			WaterCannon_BackendSetPos(0, cannon->m_avecPos[pointA].x - offset.x, cannon->m_avecPos[pointA].y - offset.y, cannon->m_avecPos[pointA].z - offset.z);
				
			WaterCannon_BackendSetRGBA(1, color, color, color, color);
			WaterCannon_BackendSetPos(1, cannon->m_avecPos[pointA].x + offset.x, cannon->m_avecPos[pointA].y + offset.y, cannon->m_avecPos[pointA].z + offset.z);
			
			WaterCannon_BackendSetRGBA(2, color, color, color, color);
			WaterCannon_BackendSetPos(2, cannon->m_avecPos[pointB].x - offset.x, cannon->m_avecPos[pointB].y - offset.y, cannon->m_avecPos[pointB].z - offset.z);
			
			WaterCannon_BackendSetRGBA(3, color, color, color, color);
			WaterCannon_BackendSetPos(3, cannon->m_avecPos[pointB].x + offset.x, cannon->m_avecPos[pointB].y + offset.y, cannon->m_avecPos[pointB].z + offset.z);
			
			WaterCannon_BackendLittleTest();

			if ( WaterCannon_BackendTransform() )
			{
				// Retain the original traversal and write order
				WaterCannon_BackendRenderIndexed(WaterCannonIndexList);
				WaterCannon_BackendEnd();
			}
		}
		
		pointA = pointB--;
		if ( pointB < 0 )
			pointB += WATERCANNON_SEGMENTS;
	}
	
	WaterCannon_BackendZWrite(true);
	WaterCannon_BackendVertexAlpha(false);
	WaterCannon_BackendFog(false);
}

void WaterCannon_PushPeds(WaterCannon *cannon)
{
	// Preserve bounds, descending live pool traversal and first qualifying segment effects
	float minx = 10000.0f;
	float maxx = -10000.0f;
	float miny = 10000.0f;
	float maxy = -10000.0f;
	float minz = 10000.0f;
	float maxz = -10000.0f;
  
	for ( int32_t i = 0; i < WATERCANNON_SEGMENTS; i++ )
	{
		// Retain the original traversal and write order
		if ( cannon->m_abUsed[i] )
		{
			// Retain the original traversal and write order
			minx = Min(minx, cannon->m_avecPos[i].x);
			maxx = Max(maxx, cannon->m_avecPos[i].x);
			
			miny = Min(miny, cannon->m_avecPos[i].y);
			maxy = Max(maxy, cannon->m_avecPos[i].y);
			
			minz = Min(minz, cannon->m_avecPos[i].z);
			maxz = Max(maxz, cannon->m_avecPos[i].z);
		}
	}
	
	// Access raw storage through the C store or pool API
	for ( int32_t i = CPool_GetSize(WaterCannon_BackendPedPool()) - 1; i >= 0; i--)
	{
		// Retain the original traversal and write order
		// Access raw storage through the C store or pool API
		CPed *ped = WaterCannon_BackendPed(CPool_GetSlot(WaterCannon_BackendPedPool(), i));
		if ( ped )
		{
			// Retain the original traversal and write order
			if (   WaterCannon_BackendPedX(ped) > minx && WaterCannon_BackendPedX(ped) < maxx
				&& WaterCannon_BackendPedY(ped) > miny && WaterCannon_BackendPedY(ped) < maxy
				&& WaterCannon_BackendPedZ(ped) > minz && WaterCannon_BackendPedZ(ped) < maxz )
			{
				// Retain the original traversal and write order
				for ( int32_t j = 0; j < WATERCANNON_SEGMENTS; j++ )
				{
					// Retain the original traversal and write order
					if ( cannon->m_abUsed[j] )
					{
						// Retain the original traversal and write order
						WaterCannonVector dist = WaterCannon_Subtract(cannon->m_avecPos[j], WaterCannon_BackendPedPosition(ped));
						
						if ( WaterCannon_MagnitudeSqr(dist) < 5.0f )
						{
							// Retain the original traversal and write order
							int32_t localDir = WaterCannon_BackendLocalDirection(ped);
							
							WaterCannon_BackendStanding(ped);
							
							WaterCannon_BackendForce(ped, 2.0f * (*Timer_GetTimeStep()));
							
							WaterCannon_BackendSetSpeedX(ped, (0.6f * cannon->m_avecVelocity[j].x + WaterCannon_BackendSpeedX(ped)) * 0.5f);
							WaterCannon_BackendSetSpeedY(ped, (0.6f * cannon->m_avecVelocity[j].y + WaterCannon_BackendSpeedY(ped)) * 0.5f);
							
							float pedSpeed2D = WaterCannon_PedSpeed(ped);

							if ( pedSpeed2D > 0.2f ) {
								WaterCannon_BackendSetSpeedX(ped, WaterCannon_BackendSpeedX(ped) * (0.2f / pedSpeed2D));
								WaterCannon_BackendSetSpeedY(ped, WaterCannon_BackendSpeedY(ped) * (0.2f / pedSpeed2D));
							}
							WaterCannon_BackendFall(ped, localDir + ANIM_STD_HIGHIMPACT_FRONT);
							WaterCannon_BackendParticle(PARTICLE_STEAM_NY_SLOWMOTION, WaterCannon_BackendPedPosition(ped), WaterCannon_Scale(WaterCannon_BackendVelocity(ped), 0.3f), 0, 0, 0);
							#if defined(_MSC_VER) && !defined(__clang__)
                            // Preserve native C++ random draws and narrowing before the position query
                            int32_t rotation = (int32_t)General_GetRandomNumberInRangeFloat(0.f, 90.f);
                            int32_t rotationSpeed = (int32_t)General_GetRandomNumberInRangeFloat(0.f, 10.f);
                            WaterCannon_BackendParticle(PARTICLE_CAR_SPLASH, WaterCannon_BackendPedPosition(ped), WaterCannon_Add(WaterCannon_Scale(WaterCannon_BackendVelocity(ped), -0.3f), (WaterCannonVector){0.f, 0.f, 0.5f}), rotationSpeed, rotation, 1);
#else
                            // Retain the original expression order selected by the other C and C++ front ends
                            WaterCannon_BackendParticle(PARTICLE_CAR_SPLASH, WaterCannon_BackendPedPosition(ped), WaterCannon_Add(WaterCannon_Scale(WaterCannon_BackendVelocity(ped), -0.3f), (WaterCannonVector){0.f, 0.f, 0.5f}),
								General_GetRandomNumberInRangeFloat(0.f, 10.f), General_GetRandomNumberInRangeFloat(0.f, 90.f), 1);
#endif
							
							j = WATERCANNON_SEGMENTS;
						}
					}
				}
			}
		}
	}	
}

void WaterCannons_Init(void)
{
	// Initialize every original cannon in array order
	for ( int32_t i = 0; i < NUM_WATERCANNONS; i++ )
		WaterCannon_Init(&WaterCannons_aCannons[i]);
}

static void WaterCannons_Input(uint32_t id, const void *pos, const void *dir, WaterCannonRead read)
{
	// Preserve assertions, signed and unsigned ID comparisons and first-free selection
	WC_ASSERT(pos != NULL);
	WC_ASSERT(dir != NULL);
	
	// find the one by id
	{
		// Retain the original traversal and write order
		int32_t n = 0;
		while ( n < NUM_WATERCANNONS && id != WaterCannons_aCannons[n].m_nId )
			n++;
		
		if ( n < NUM_WATERCANNONS )
		{
			// Retain the original traversal and write order
			WaterCannon_Input(&WaterCannons_aCannons[n], pos, dir, read);
			return;
		}
	}
	
	// if no luck then find a free one
	{
		// Retain the original traversal and write order
		int32_t n = 0;
		while ( n < NUM_WATERCANNONS && 0 != WaterCannons_aCannons[n].m_nId )
			n++;
		
		if ( n < NUM_WATERCANNONS )
		{
			// Retain the original traversal and write order
			WaterCannon_Init(&WaterCannons_aCannons[n]);
			WaterCannons_aCannons[n].m_nId = id;
			WaterCannon_Input(&WaterCannons_aCannons[n], pos, dir, read);
			return;
		}
	}
}

void WaterCannons_Update(void)
{
	// Update each nonzero-ID cannon at its original array index
	for ( int32_t i = 0; i < NUM_WATERCANNONS; i++ )
	{
		// Retain the original traversal and write order
		if ( WaterCannons_aCannons[i].m_nId != 0 )
			WaterCannon_Update_OncePerFrame(&WaterCannons_aCannons[i], i);
	}
}

void WaterCannons_Render(void)
{
	// Render each nonzero-ID cannon within the original debug group
	WaterCannon_BackendPush("CWaterCannons::Render");
	for ( int32_t i = 0; i < NUM_WATERCANNONS; i++ )
	{
		// Retain the original traversal and write order
		if ( WaterCannons_aCannons[i].m_nId != 0 )
			WaterCannon_Render(&WaterCannons_aCannons[i]);
	}
	WaterCannon_BackendPop();
}

void WaterCannon_Update_NewInput(WaterCannon *cannon, const WaterCannonVector *pos, const WaterCannonVector *dir)
{
	// Preserve assertions and ordered reads even when input pointers alias cannon fields
	WaterCannon_Input(cannon, pos, dir, WaterCannon_Read);
}
void WaterCannon_Update_NewInputLegacy(WaterCannon *cannon, const void *pos, const void *dir)
{
	// Read each legacy typed input only after the preceding original write
	WaterCannon_Input(cannon, pos, dir, WaterCannon_BackendReadVector);
}
void WaterCannons_UpdateOne(uint32_t id, const WaterCannonVector *pos, const WaterCannonVector *dir)
{
	// Enter the complete C manager with actual C vector inputs
	WaterCannons_Input(id, pos, dir, WaterCannon_Read);
}
void WaterCannons_UpdateOneLegacy(uint32_t id, const void *pos, const void *dir)
{
	// Enter the same C manager without eagerly copying legacy objects
	WaterCannons_Input(id, pos, dir, WaterCannon_BackendReadVector);
}
//- rouz edit (ChatGPT)
