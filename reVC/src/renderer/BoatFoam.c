//+ rouz edit (ChatGPT)
#include "BoatFoam_backend.h"
#include "Timer.h"
#include "General.h"
#include "ParticleType.h"
#include <math.h>

#ifdef _MSC_VER
float fast_absf(float value);
#define BOAT_FOAM_ABS fast_absf
#else
#define BOAT_FOAM_ABS fabsf
#endif
static BoatFoamVector BoatFoam_Add(BoatFoamVector a,BoatFoamVector b)
{
    // Retain the original component-wise position addition
    BoatFoamVector result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static BoatFoamVector BoatFoam_Subtract(BoatFoamVector a,BoatFoamVector b)
{
    // Retain the original component-wise displacement calculation
    BoatFoamVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static float BoatFoam_Dot(BoatFoamVector a,BoatFoamVector b)
{
    // Preserve the original three-product addition order
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static float BoatFoam_Dot2D(BoatFoamVector a,BoatFoamVector b)
{
    // Preserve the original two-dimensional camera rejection expression
    return a.x*b.x+a.y*b.y;
}
static BoatFoamVector BoatFoam_RandomPosition(int rnd,float innerFw)
{
    // Preserve both original signed-byte conversions and multiply-then-divide order
    BoatFoamVector pos;pos.x=(int8_t)(rnd-128)*innerFw/110.0f;pos.y=(int8_t)((rnd>>8)-128)*innerFw/110.0f;pos.z=0.0f;return pos;
}
static BoatFoamVector BoatFoam_Transform(void *matrix,BoatFoamVector v)
{
    // Retain every original matrix-vector product and addition in C
    BoatFoamMatrix m=BoatFoam_BackendReadMatrix(matrix);
    BoatFoamVector result={m.rx*v.x+m.fx*v.y+m.ux*v.z+m.px,m.ry*v.x+m.fy*v.y+m.uy*v.z+m.py,m.rz*v.x+m.fz*v.y+m.uz*v.z+m.pz};return result;
}
static uint32_t BoatFromStart;

void
BoatFoam_CreateFoamAroundObject(void *pMatrix, float innerFw, float innerRg, float innerUp, int32_t particles)
{
	// Retain the complete original foam and animation operation order
	float outerFw = innerFw + 5.0f;
	float outerRg = innerRg + 5.0f;
	float outerUp = innerUp + 5.0f;
	for (int attempts = 0; particles > 0 && attempts < 1000; attempts++) {
		// Preserve original timer, random, rejection and callback behavior
		BoatFoamVector pos;
		int rnd = General_GetRandomNumber();
		pos = BoatFoam_RandomPosition(rnd,innerFw);
		if (BoatFoam_Dot2D(pos,BoatFoam_BackendCameraForward()) >= 0)
			continue;
		// was there any point in adding it here?
		pos = BoatFoam_Add(pos,BoatFoam_BackendMatrixPosition(pMatrix));
		pos.z = 2.0f;
		float fw = BOAT_FOAM_ABS(BoatFoam_Dot(BoatFoam_BackendMatrixForward(pMatrix),BoatFoam_Subtract(pos,BoatFoam_BackendMatrixPosition(pMatrix))));
		if (fw >= outerFw)
			continue;
		float rg = BOAT_FOAM_ABS(BoatFoam_Dot(BoatFoam_BackendMatrixRight(pMatrix),BoatFoam_Subtract(pos,BoatFoam_BackendMatrixPosition(pMatrix))));
		if (rg >= outerRg)
			continue;
		float up = BOAT_FOAM_ABS(BoatFoam_Dot(BoatFoam_BackendMatrixUp(pMatrix),BoatFoam_Subtract(pos,BoatFoam_BackendMatrixPosition(pMatrix))));
		if (up >= outerUp)
			continue;
		if (fw > innerFw || rg > innerRg || up > innerUp) {
			// Preserve original timer, random, rejection and callback behavior
			BoatFoam_BackendParticle(PARTICLE_STEAM2,pos,4.0f);
			particles--;
		}
	}
}

void
BoatFoam_StartBoatFoamAnimation(void)
{
	// Retain the complete original foam and animation operation order
	BoatFromStart = (*Timer_GetTimeInMilliseconds());
}

void
BoatFoam_UpdateBoatFoamAnimation(void *pMatrix)
{
	// Retain the complete original foam and animation operation order
		static uint32_t AnimationStartTime = 0;
	static uint32_t LastFrameInAnimation = 0;
		static float X, Y, Z, dX, dY, dZ;
	BoatFoam_CreateFoamAroundObject(pMatrix, 107.0f, 24.1f, 30.5f, 2);
	uint32_t prev = Timer_GetPreviousTimeInMilliseconds();
	uint32_t cur = (*Timer_GetTimeInMilliseconds());

		if (AnimationStartTime != 0) {
			// Preserve original timer, random, rejection and callback behavior
		uint32_t frameInAnimation = (cur - AnimationStartTime) * 30 / 1000 + 1;
		if (frameInAnimation > 15) {
			// Preserve original timer, random, rejection and callback behavior
			AnimationStartTime = 0;
		} else {
			// Preserve original timer, random, rejection and callback behavior
			if (frameInAnimation != LastFrameInAnimation) {
				// Preserve original timer, random, rejection and callback behavior
				BoatFoamVector pos = BoatFoam_Transform(pMatrix,(BoatFoamVector){X + dX * frameInAnimation, Y + dY * frameInAnimation, Z + dZ * frameInAnimation});
				BoatFoam_BackendParticle(PARTICLE_STEAM_NY,pos,frameInAnimation * 0.5f + 2.0f);
				LastFrameInAnimation = frameInAnimation;
			}
		}
			}
	if ((cur & 0x3FF) < (prev & 0x3FF)) {
		// Preserve original timer, random, rejection and callback behavior
				AnimationStartTime = cur;
		LastFrameInAnimation = 0;
				int rnd = General_GetRandomNumber();
		X = (int8_t)(rnd - 128) * 0.2f;
		Y = (int8_t)((rnd >> 8) - 128) * 0.2f;
		Z = 10.0f;
		rnd = General_GetRandomNumber();
		dX = (int8_t)(rnd - 128) * 0.02f;
		dY = (int8_t)((rnd >> 8) - 128) * 0.02f;
		dZ = 2.0f;
	}
}
//- rouz edit (ChatGPT)
