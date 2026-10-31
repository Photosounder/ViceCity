//+ rouz edit (ChatGPT)
#include "BrightLights_backend.h"
#include "RenderBuffer_backend.h"
#include "General.h"
#include <stddef.h>
#include <math.h>

static BrightLightVector BrightLights_Add(BrightLightVector a,BrightLightVector b)
{
    // Preserve component-wise original vector addition
    BrightLightVector r={a.x+b.x,a.y+b.y,a.z+b.z};return r;
}
static BrightLightVector BrightLights_LeftScale(float a,BrightLightVector b)
{
    // Preserve original scalar-times-vector operand order
    BrightLightVector r={a*b.x,a*b.y,a*b.z};return r;
}
static BrightLightVector BrightLights_Subtract(BrightLightVector a,BrightLightVector b)
{
    // Preserve component-wise original vector subtraction
    BrightLightVector r={a.x-b.x,a.y-b.y,a.z-b.z};return r;
}
static float BrightLights_Squared(BrightLightVector value)
{
    // Retain the original sum order before any square root
    return value.x*value.x+value.y*value.y+value.z*value.z;
}
static float BrightLights_Distance(BrightLightVector a,BrightLightVector b)
{
    // Retain the original single-precision magnitude operation
    return sqrtf(BrightLights_Squared(BrightLights_Subtract(a,b)));
}
static void BrightLights_Normalise(BrightLightVector *value)
{
    // Preserve the complete original vector normalizer including its zero branch
    float sq=BrightLights_Squared(*value);
    if(sq>0.0f){
        // Multiply original components by the same reciprocal square root
        float invsqrt=1.0f/sqrtf(sq);value->x*=invsqrt;value->y*=invsqrt;value->z*=invsqrt;
    }else{
        // Retain the original x-only fallback
        value->x=1.0f;
    }
}
#define BRIGHTLIGHTS_MAX_DIST (60.0f)	// invisible beyond this
#define BRIGHTLIGHTS_FADE_DIST (45.0f)	// strongest between these two
#define CARLIGHTS_MAX_DIST (30.0f)
#define CARLIGHTS_FADE_DIST (15.0f)	// 31 for close lights

static int32_t NumBrightLights;
static BrightLight aBrightLights[NUMBRIGHTLIGHTS];
static float BrightLightsFlicker[NUMBRIGHTLIGHTS];
static int BrightLightsEnvMapCount;
static BrightLight BrightLightsEnvMap[NUMBRIGHTLIGHTS];
static float BrightLightsEnvMapFlicker[NUMBRIGHTLIGHTS];

void
BrightLights_Init(void)
{
	// Retain the complete original bright-light algorithm and live write order
	NumBrightLights = 0;
	// Reset the saved light list used by environment reflections
		BrightLightsEnvMapCount = 0;
	}

static BrightLightVector
BrightLights_GetCameraPosition(void)
{
	// Retain the complete original bright-light algorithm and live write order
	// Use the camera that owns the active bright-light registration pass
	BrightLightVector cameraPosition = BrightLights_BackendCameraPosition();
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	// Read the auxiliary camera origin while collecting reflection lights
	if(BrightLights_BackendEnvMap() && BrightLights_BackendEnvCamera() != NULL){
		// Preserve original conditions, arithmetic and reflection restoration
		void *effectCamera = BrightLights_BackendEnvCamera();
		void *effectFrame = BrightLights_BackendCameraFrame(effectCamera);
		if(effectFrame && BrightLights_BackendFrameMatrix(effectFrame))
			cameraPosition = BrightLights_BackendMatrixPosition(BrightLights_BackendFrameMatrix(effectFrame));
	}
#endif
	return cameraPosition;
}

void
BrightLights_RegisterOne(BrightLightVector pos, BrightLightVector up, BrightLightVector side, BrightLightVector front,
	uint8_t type, uint8_t red, uint8_t green, uint8_t blue)
{
	// Retain the complete original bright-light algorithm and live write order
	if(NumBrightLights >= NUMBRIGHTLIGHTS)
		return;

	// Measure registration distance from the render pass camera
		// Complete the original C++ assignment right operand before choosing its live destination
	float cameraDistance=BrightLights_Distance(pos,BrightLights_GetCameraPosition());
	aBrightLights[NumBrightLights].m_camDist=cameraDistance;
		if(aBrightLights[NumBrightLights].m_camDist > BRIGHTLIGHTS_MAX_DIST)
		return;

	aBrightLights[NumBrightLights].m_pos = pos;
	aBrightLights[NumBrightLights].m_up = up;
	aBrightLights[NumBrightLights].m_side = side;
	aBrightLights[NumBrightLights].m_front = front;
	aBrightLights[NumBrightLights].m_type = type;
	aBrightLights[NumBrightLights].m_red = red;
	aBrightLights[NumBrightLights].m_green = green;
	aBrightLights[NumBrightLights].m_blue = blue;

	NumBrightLights++;
}

static float TrafficLightsSide[6] = { -0.09f, 0.09f, 0.162f, 0.09f, -0.09f, -0.162f };
static float TrafficLightsUp[6] = { 0.162f, 0.162f, 0.0f, -0.162f, -0.162f, 0.0f };
static float LongCarHeadLightsSide[8] = { -0.2f, 0.2f, -0.2f, 0.2f, -0.2f, 0.2f, -0.2f, 0.2f };
static float LongCarHeadLightsFront[8] = { 0.1f, 0.1f, -0.1f, -0.1f, 0.1f, 0.1f, -0.1f, -0.1f };
static float LongCarHeadLightsUp[8] = { 0.1f, 0.1f, 0.1f, 0.1f, -0.1f, -0.1f, -0.1f, -0.1f };
static float SmallCarHeadLightsSide[8] = { -0.08f, 0.08f, -0.08f, 0.08f, -0.08f, 0.08f, -0.08f, 0.08f };
static float SmallCarHeadLightsFront[8] = { 0.08f, 0.08f, -0.08f, -0.08f, 0.08f, 0.08f, -0.08f, -0.08f };
static float SmallCarHeadLightsUp[8] = { 0.08f, 0.08f, 0.08f, 0.08f, -0.08f, -0.08f, -0.08f, -0.08f };
static float BigCarHeadLightsSide[8] = { -0.15f, 0.15f, -0.15f, 0.15f, -0.15f, 0.15f, -0.15f, 0.15f };
static float BigCarHeadLightsFront[8] = { 0.15f, 0.15f, -0.15f, -0.15f, 0.15f, 0.15f, -0.15f, -0.15f };
static float BigCarHeadLightsUp[8] = { 0.15f, 0.15f, 0.15f, 0.15f, -0.15f, -0.15f, -0.15f, -0.15f };
static float TallCarHeadLightsSide[8] = { -0.08f, 0.08f, -0.08f, 0.08f, -0.08f, 0.08f, -0.08f, 0.08f };
static float TallCarHeadLightsFront[8] = { 0.08f, 0.08f, -0.08f, -0.08f, 0.08f, 0.08f, -0.08f, -0.08f };
static float TallCarHeadLightsUp[8] = { 0.2f, 0.2f, 0.2f, 0.2f, -0.2f, -0.2f, -0.2f, -0.2f };
static float SirenLightsSide[6] = { -0.04f, 0.04f, 0.06f, 0.04f, -0.04f, -0.06f };
static float SirenLightsUp[6] = { 0.06f, 0.06f, 0.0f, -0.06f, -0.06f, 0.0f };
static uint16_t TrafficLightIndices[4*3] = { 0, 1, 5,  1, 2, 3,  1, 3, 4,  1, 4, 5 };
static uint16_t CubeIndices[12*3] = {
	0, 2, 1,  1, 2, 3,  3, 5, 1,  3, 7, 5,
	2, 7, 3,  2, 6, 7,  4, 0, 1,  4, 1, 5,
	6, 0, 4,  6, 2, 0,  6, 5, 7,  6, 4, 5
};

void
BrightLights_Render(bool reuseFlicker)
{
	// Retain the complete original bright-light algorithm and live write order
	int i, j;
	BrightLightVector pos;
	// Resolve the active camera before restoring or combining the saved light list
		BrightLightVector brightLightCameraPosition = BrightLights_GetCameraPosition();
	bool useReflectionCameraPosition = false;
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	useReflectionCameraPosition = reuseFlicker && BrightLights_BackendEnvMap() && BrightLights_BackendEnvCamera() != NULL;
#endif
			// Preserve main-scene lamps while the reflection pass merges its own light list
	int savedMainBrightLightCount = 0;
	BrightLight savedMainBrightLights[NUMBRIGHTLIGHTS];
	float savedMainBrightLightFlicker[NUMBRIGHTLIGHTS];
	if(useReflectionCameraPosition){
		// Preserve original conditions, arithmetic and reflection restoration
		savedMainBrightLightCount = NumBrightLights;
		for(i = 0; i < savedMainBrightLightCount; i++){
			// Preserve original conditions, arithmetic and reflection restoration
			savedMainBrightLights[i] = aBrightLights[i];
			savedMainBrightLightFlicker[i] = BrightLightsFlicker[i];
		}
	}
		// Save main-pass lights and restore them for the later reflection pass
		if(!reuseFlicker){
			// Preserve original conditions, arithmetic and reflection restoration
		BrightLightsEnvMapCount = NumBrightLights;
		// Copy each light before the main pass clears its queue
		for(i = 0; i < NumBrightLights; i++){
			// Preserve original conditions, arithmetic and reflection restoration
			BrightLightsEnvMap[i] = aBrightLights[i];
		}
	}else if(BrightLightsEnvMapCount > 0){
		// Preserve original conditions, arithmetic and reflection restoration
		// Combine reflection-camera registrations with cached vehicle lamps
				if(useReflectionCameraPosition && NumBrightLights > 0){
					// Preserve original conditions, arithmetic and reflection restoration
			int registeredCount = NumBrightLights;
			// Keep cached flicker phases for lights registered in both passes
			for(i = 0; i < registeredCount; i++){
				// Preserve original conditions, arithmetic and reflection restoration
				BrightLightsFlicker[i] = 0.0f;
				for(j = 0; j < BrightLightsEnvMapCount; j++){
					// Preserve original conditions, arithmetic and reflection restoration
					if(aBrightLights[i].m_type == BrightLightsEnvMap[j].m_type &&
					   BrightLights_Squared(BrightLights_Subtract(aBrightLights[i].m_pos,BrightLightsEnvMap[j].m_pos)) < 0.0001f){
						// Preserve original conditions, arithmetic and reflection restoration
						BrightLightsFlicker[i] = BrightLightsEnvMapFlicker[j];
						break;
					}
				}
			}
			// Retain cached vehicle lamps that the software vehicle path did not register again
			for(j = 0; j < BrightLightsEnvMapCount && NumBrightLights < NUMBRIGHTLIGHTS; j++){
				// Preserve original conditions, arithmetic and reflection restoration
				float cameraDistance = BrightLights_Distance(BrightLightsEnvMap[j].m_pos,brightLightCameraPosition);
				// Skip cached lights that are outside this reflection camera range
				if(cameraDistance > BRIGHTLIGHTS_MAX_DIST)
					continue;
				// Search the current reflection list for the same light
				for(i = 0; i < NumBrightLights; i++)
					if(aBrightLights[i].m_type == BrightLightsEnvMap[j].m_type &&
					   BrightLights_Squared(BrightLights_Subtract(aBrightLights[i].m_pos,BrightLightsEnvMap[j].m_pos)) < 0.0001f)
						break;
				// Append cached lamps that the software vehicle path did not register
				if(i == NumBrightLights){
					// Preserve original conditions, arithmetic and reflection restoration
					aBrightLights[i] = BrightLightsEnvMap[j];
					aBrightLights[i].m_camDist = cameraDistance;
					BrightLightsFlicker[i] = BrightLightsEnvMapFlicker[j];
					NumBrightLights++;
				}
			}
		}else{
			// Preserve original conditions, arithmetic and reflection restoration
			// Restore cached main-view lights when this pass registered none
			NumBrightLights = BrightLightsEnvMapCount;
			// Restore the saved lights and their matching flicker values
			for(i = 0; i < NumBrightLights; i++){
				// Preserve original conditions, arithmetic and reflection restoration
				aBrightLights[i] = BrightLightsEnvMap[i];
				BrightLightsFlicker[i] = BrightLightsEnvMapFlicker[i];
			}
		}
			}else if(useReflectionCameraPosition){
				// Preserve original conditions, arithmetic and reflection restoration
		// Initialize reflection-only lights that have no cached main-pass phase
		for(i = 0; i < NumBrightLights; i++)
			BrightLightsFlicker[i] = 0.0f;
	}
		// Use the reflection camera origin for auxiliary light-distance fading
		BrightLightVector brightLightCameraRight = BrightLights_BackendCameraRight(); // rouz edit (ChatGPT)
	BrightLightVector brightLightCameraUp = BrightLights_BackendCameraUp(); // rouz edit (ChatGPT)
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(reuseFlicker && BrightLights_BackendEnvMap() && BrightLights_BackendEnvCamera() != NULL){
		// Preserve original conditions, arithmetic and reflection restoration
		void *effectCamera = BrightLights_BackendEnvCamera();
		void *effectFrame = BrightLights_BackendCameraFrame(effectCamera);
		if(effectFrame && BrightLights_BackendFrameMatrix(effectFrame)){
			// Preserve original conditions, arithmetic and reflection restoration
			const void *cameraMatrix = BrightLights_BackendFrameMatrix(effectFrame);
			// Keep siren billboards facing the active reflection camera
						brightLightCameraRight = BrightLights_BackendMatrixRight(cameraMatrix);
			brightLightCameraUp = BrightLights_BackendMatrixUp(cameraMatrix);
			BrightLights_Normalise(&brightLightCameraRight);
			BrightLights_Normalise(&brightLightCameraUp);
						useReflectionCameraPosition = true;
		}
	}
#endif
	
	if(NumBrightLights == 0)
		return;

	BrightLights_BackendVertexAlpha(true);
	BrightLights_BackendZWrite(true);
	BrightLights_BackendSrcBlend();
	BrightLights_BackendDestBlend();
	BrightLights_BackendClearRaster();

	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;

	for(i = 0; i < NumBrightLights; i++){
		// Preserve original conditions, arithmetic and reflection restoration
		if(TempBufferIndicesStored > TEMPBUFFERINDEXSIZE-40 || TempBufferVerticesStored > TEMPBUFFERVERTSIZE-40)
			BrightLights_RenderOutGeometryBuffer();

		int r, g, b, a;
		// Reuse the main-pass flicker while drawing the same lights into reflections
				float flicker = BrightLightsFlicker[i];
		if(!reuseFlicker){
			// Preserve original conditions, arithmetic and reflection restoration
			// Preserve the original randomized flicker for the main scene pass
			flicker = (General_GetRandomNumber()&0xFF) * 0.2f;
			BrightLightsFlicker[i] = flicker;
			// Keep the reflection copy on the same randomized phase as the main pass
						BrightLightsEnvMapFlicker[i] = flicker;
					}
				switch(aBrightLights[i].m_type){
					// Preserve original conditions, arithmetic and reflection restoration
		case BRIGHTLIGHT_TRAFFIC_GREEN:
			r = flicker; g = 255; b = flicker;
			break;
		case BRIGHTLIGHT_TRAFFIC_YELLOW:
			r = 255; g = 128; b = flicker;
			break;
		case BRIGHTLIGHT_TRAFFIC_RED:
			r = 255; g = flicker; b = flicker;
			break;

		case BRIGHTLIGHT_FRONT_LONG:
		case BRIGHTLIGHT_FRONT_SMALL:
		case BRIGHTLIGHT_FRONT_BIG:
		case BRIGHTLIGHT_FRONT_TALL:
			r = 255; g = 255; b = 255;
			break;

		case BRIGHTLIGHT_REAR_LONG:
		case BRIGHTLIGHT_REAR_SMALL:
		case BRIGHTLIGHT_REAR_BIG:
		case BRIGHTLIGHT_REAR_TALL:
			r = 255; g = flicker; b = flicker;
			break;

		case BRIGHTLIGHT_SIREN:
			r = aBrightLights[i].m_red;
			g = aBrightLights[i].m_green;
			b = aBrightLights[i].m_blue;
			break;
#ifdef FIX_BUGS  //just to make sure that color never will be undefined
		default:
			// Skip an invalid light without bypassing reflection queue restoration
			continue; // rouz edit (ChatGPT)
#endif
		}

		// Recompute reflection light distances from the auxiliary camera while preserving main-pass cache values
				float camDist = useReflectionCameraPosition ?
			BrightLights_Distance(aBrightLights[i].m_pos,brightLightCameraPosition) : aBrightLights[i].m_camDist;
		// Ignore cached lights outside the active reflection camera range
				if(useReflectionCameraPosition && camDist > BRIGHTLIGHTS_MAX_DIST)
			continue;
				if(camDist < BRIGHTLIGHTS_FADE_DIST)
			a = 255;
		else
			a = 255*(1.0f - (camDist-BRIGHTLIGHTS_FADE_DIST)/(BRIGHTLIGHTS_MAX_DIST-BRIGHTLIGHTS_FADE_DIST));
		// fade car lights down to 31 as they come near
		if(aBrightLights[i].m_type >= BRIGHTLIGHT_FRONT_LONG && aBrightLights[i].m_type <= BRIGHTLIGHT_REAR_TALL){
			// Preserve original conditions, arithmetic and reflection restoration
			if(camDist < CARLIGHTS_FADE_DIST)
				a = 31;
			else if(camDist < CARLIGHTS_MAX_DIST)
				a = 31 + (255-31)*((camDist-CARLIGHTS_FADE_DIST)/(CARLIGHTS_MAX_DIST-CARLIGHTS_FADE_DIST));
		}
		
		switch(aBrightLights[i].m_type){
			// Preserve original conditions, arithmetic and reflection restoration
		case BRIGHTLIGHT_TRAFFIC_GREEN:
		case BRIGHTLIGHT_TRAFFIC_YELLOW:
		case BRIGHTLIGHT_TRAFFIC_RED:
			for(j = 0; j < 6; j++){
				// Preserve original conditions, arithmetic and reflection restoration
				pos = BrightLights_Add(BrightLights_Add(BrightLights_LeftScale(TrafficLightsSide[j],aBrightLights[i].m_side),BrightLights_LeftScale(TrafficLightsUp[j],aBrightLights[i].m_up)),aBrightLights[i].m_pos);
				BrightLights_BackendSetRGBA(TempBufferVerticesStored+j, r, g, b, a);
				BrightLights_BackendSetPos(TempBufferVerticesStored+j, pos.x, pos.y, pos.z);
			}
			for(j = 0; j < 4*3; j++)
				TempBufferRenderIndexList[TempBufferIndicesStored+j] = TrafficLightIndices[j] + TempBufferVerticesStored;
			TempBufferVerticesStored += 6;
			TempBufferIndicesStored += 4*3;
			break;

		case BRIGHTLIGHT_FRONT_LONG:
		case BRIGHTLIGHT_REAR_LONG:
			for(j = 0; j < 8; j++){
				// Preserve original conditions, arithmetic and reflection restoration
				pos = BrightLights_Add(BrightLights_Add(BrightLights_Add(BrightLights_LeftScale(LongCarHeadLightsSide[j],aBrightLights[i].m_side),BrightLights_LeftScale(LongCarHeadLightsUp[j],aBrightLights[i].m_up)),BrightLights_LeftScale(LongCarHeadLightsFront[j],aBrightLights[i].m_front)),aBrightLights[i].m_pos);
				BrightLights_BackendSetRGBA(TempBufferVerticesStored+j, r, g, b, a);
				BrightLights_BackendSetPos(TempBufferVerticesStored+j, pos.x, pos.y, pos.z);
			}
			for(j = 0; j < 12*3; j++)
				TempBufferRenderIndexList[TempBufferIndicesStored+j] = CubeIndices[j] + TempBufferVerticesStored;
			TempBufferVerticesStored += 8;
			TempBufferIndicesStored += 12*3;
			break;

		case BRIGHTLIGHT_FRONT_SMALL:
		case BRIGHTLIGHT_REAR_SMALL:
			for(j = 0; j < 8; j++){
				// Preserve original conditions, arithmetic and reflection restoration
				pos = BrightLights_Add(BrightLights_Add(BrightLights_Add(BrightLights_LeftScale(SmallCarHeadLightsSide[j],aBrightLights[i].m_side),BrightLights_LeftScale(SmallCarHeadLightsUp[j],aBrightLights[i].m_up)),BrightLights_LeftScale(SmallCarHeadLightsFront[j],aBrightLights[i].m_front)),aBrightLights[i].m_pos);
				BrightLights_BackendSetRGBA(TempBufferVerticesStored+j, r, g, b, a);
				BrightLights_BackendSetPos(TempBufferVerticesStored+j, pos.x, pos.y, pos.z);
			}
			for(j = 0; j < 12*3; j++)
				TempBufferRenderIndexList[TempBufferIndicesStored+j] = CubeIndices[j] + TempBufferVerticesStored;
			TempBufferVerticesStored += 8;
			TempBufferIndicesStored += 12*3;
			break;

		case BRIGHTLIGHT_FRONT_BIG:
		case BRIGHTLIGHT_REAR_BIG:
			for (j = 0; j < 8; j++) {
				// Preserve original conditions, arithmetic and reflection restoration
				pos = BrightLights_Add(BrightLights_Add(BrightLights_Add(BrightLights_LeftScale(BigCarHeadLightsSide[j],aBrightLights[i].m_side),BrightLights_LeftScale(BigCarHeadLightsUp[j],aBrightLights[i].m_up)),BrightLights_LeftScale(BigCarHeadLightsFront[j],aBrightLights[i].m_front)),aBrightLights[i].m_pos);
				BrightLights_BackendSetRGBA(TempBufferVerticesStored + j, r, g, b, a);
				BrightLights_BackendSetPos(TempBufferVerticesStored + j, pos.x, pos.y, pos.z);
			}
			for (j = 0; j < 12 * 3; j++)
				TempBufferRenderIndexList[TempBufferIndicesStored + j] = CubeIndices[j] + TempBufferVerticesStored;
			TempBufferVerticesStored += 8;
			TempBufferIndicesStored += 12 * 3;
			break;

		case BRIGHTLIGHT_FRONT_TALL:
		case BRIGHTLIGHT_REAR_TALL:
			for(j = 0; j < 8; j++){
				// Preserve original conditions, arithmetic and reflection restoration
				pos = BrightLights_Add(BrightLights_Add(BrightLights_Add(BrightLights_LeftScale(TallCarHeadLightsSide[j],aBrightLights[i].m_side),BrightLights_LeftScale(TallCarHeadLightsUp[j],aBrightLights[i].m_up)),BrightLights_LeftScale(TallCarHeadLightsFront[j],aBrightLights[i].m_front)),aBrightLights[i].m_pos);
				BrightLights_BackendSetRGBA(TempBufferVerticesStored+j, r, g, b, a);
				BrightLights_BackendSetPos(TempBufferVerticesStored+j, pos.x, pos.y, pos.z);
			}
			for(j = 0; j < 12*3; j++)
				TempBufferRenderIndexList[TempBufferIndicesStored+j] = CubeIndices[j] + TempBufferVerticesStored;
			TempBufferVerticesStored += 8;
			TempBufferIndicesStored += 12*3;
			break;

		case BRIGHTLIGHT_SIREN:
			for(j = 0; j < 6; j++){
				// Preserve original conditions, arithmetic and reflection restoration
				pos = BrightLights_Add(BrightLights_Add(BrightLights_LeftScale(SirenLightsSide[j],brightLightCameraRight),BrightLights_LeftScale(SirenLightsUp[j],brightLightCameraUp)),aBrightLights[i].m_pos);
				BrightLights_BackendSetRGBA(TempBufferVerticesStored+j, r, g, b, a);
				BrightLights_BackendSetPos(TempBufferVerticesStored+j, pos.x, pos.y, pos.z);
			}
			for(j = 0; j < 4*3; j++)
				TempBufferRenderIndexList[TempBufferIndicesStored+j] = TrafficLightIndices[j] + TempBufferVerticesStored;
			TempBufferVerticesStored += 6;
			TempBufferIndicesStored += 4*3;
			break;

		}
	}

	BrightLights_RenderOutGeometryBuffer();
	BrightLights_BackendVertexAlpha(false);
		// Restore main-scene lamps so the regular pass can draw them after this reflection
	if(useReflectionCameraPosition){
		// Preserve original conditions, arithmetic and reflection restoration
		NumBrightLights = savedMainBrightLightCount;
		for(i = 0; i < savedMainBrightLightCount; i++){
			// Preserve original conditions, arithmetic and reflection restoration
			aBrightLights[i] = savedMainBrightLights[i];
			BrightLightsFlicker[i] = savedMainBrightLightFlicker[i];
		}
	}else
		NumBrightLights = 0;
	}

void
BrightLights_RenderOutGeometryBuffer(void)
{
	// Retain the complete original bright-light algorithm and live write order
	if(TempBufferIndicesStored != 0){
		// Preserve original conditions, arithmetic and reflection restoration
		BrightLights_BackendLittleTest();
		if(RenderBuffer_BackendTransform(TempBufferVerticesStored)){
			// Preserve original conditions, arithmetic and reflection restoration
			RenderBuffer_BackendRenderIndexed(TempBufferIndicesStored);
			RenderBuffer_BackendEnd();
		}
		TempBufferVerticesStored = 0;
		TempBufferIndicesStored = 0;
	}
}
//- rouz edit (ChatGPT)
