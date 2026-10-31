//+ rouz edit (ChatGPT)
#include "PlaneBanners_backend.h"
#include "Timer.h"
#include "RenderBuffer.h"
#include "Rubbish_backend.h"
#include <math.h>
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define Max(a,b) ((a)>(b)?(a):(b))
static PlaneBannerVector PlaneBanners_Subtract(PlaneBannerVector a,PlaneBannerVector b)
{
    // Preserve each original float subtraction and vector value boundary
    PlaneBannerVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static PlaneBannerVector PlaneBanners_Add(PlaneBannerVector a,PlaneBannerVector b)
{
    // Preserve each original component addition after scaled-distance evaluation
    PlaneBannerVector result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static PlaneBannerVector PlaneBanners_Divide(PlaneBannerVector a,float divisor)
{
    // Divide each original component before the following multiplication by eight
    PlaneBannerVector result={a.x/divisor,a.y/divisor,a.z/divisor};return result;
}
static PlaneBannerVector PlaneBanners_Scale(PlaneBannerVector a,float factor)
{
    // Keep the original separate multiplication and vector value boundary
    PlaneBannerVector result={a.x*factor,a.y*factor,a.z*factor};return result;
}
static float PlaneBanners_Magnitude(PlaneBannerVector a)
{
    // Retain the original square-root argument and left-associated sum order
    return sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
}
static bool
PlaneBanners_IsSphereVisible(PlaneBannerVector center, float radius)
{
    // Preserve original arithmetic, live owner reads and state-write ordering
	// Let the active auxiliary camera clip effect geometry during environment rendering
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(PlaneBanners_BackendEnvMap())
		return true;
#endif
	return PlaneBanners_BackendSphereVisible(center, radius);
}
static PlaneBannerVector
PlaneBanners_GetCameraPosition(void)
{
    // Preserve original arithmetic, live owner reads and state-write ordering
	// Use the active reflection camera for distance-based world-effect fades
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(PlaneBanners_BackendEnvMap() && PlaneBanners_BackendEnvCamera()){
		// Preserve the original branch and traversal order
		void *cameraFrame = PlaneBanners_BackendCameraFrame(PlaneBanners_BackendEnvCamera());
		if(cameraFrame && PlaneBanners_BackendFrameMatrix(cameraFrame))
			return PlaneBanners_BackendMatrixPosition(PlaneBanners_BackendFrameMatrix(cameraFrame));
	}
#endif
	return PlaneBanners_BackendCameraPosition();
}
static PlaneBanner aArray[5];

static void
PlaneBanner_Init(PlaneBanner *banner)
{
    // Preserve original arithmetic, live owner reads and state-write ordering
	int i;
	for(i = 0; i < ARRAY_SIZE(banner->m_pos); i++){
		// Preserve the original branch and traversal order
		banner->m_pos[i].x = i;
		banner->m_pos[i].y = 0.0f;
		banner->m_pos[i].z = -60.0f;
	}
}

static void
PlaneBanner_Update(PlaneBanner *banner)
{
    // Preserve original arithmetic, live owner reads and state-write ordering
	int i;
	if(banner->m_pos[0].z > -50.0f){
		// Preserve the original branch and traversal order
		banner->m_pos[0].z -= 0.05f*(*Timer_GetTimeStep());
		banner->m_pos[0].z = Max(banner->m_pos[0].z, -100.0f);
		for(i = 1; i < ARRAY_SIZE(banner->m_pos); i++){
			// Preserve the original branch and traversal order
			PlaneBannerVector dist = PlaneBanners_Subtract(banner->m_pos[i],banner->m_pos[i-1]);
			float len = PlaneBanners_Magnitude(dist);
			if(len > 8.0f)
				banner->m_pos[i] = PlaneBanners_Add(banner->m_pos[i-1],PlaneBanners_Scale(PlaneBanners_Divide(dist,len),8.0f));
		}
	}
}

static void
PlaneBanner_Render(PlaneBanner *banner)
{
    // Preserve original arithmetic, live owner reads and state-write ordering
	int i;
	if(banner->m_pos[0].z > -50.0f){
		// Preserve the original branch and traversal order
		// Calculate the banner fade from the camera currently rendering the scene
		PlaneBannerVector cameraPosition=PlaneBanners_GetCameraPosition();
        // Read the live anchor only after the original camera queries have completed
        float camDist=PlaneBanners_Magnitude(PlaneBanners_Subtract(cameraPosition,banner->m_pos[0])); // rouz edit (ChatGPT)
		if(PlaneBanners_IsSphereVisible(banner->m_pos[4], 32.0f) && camDist < 300.0f){ // rouz edit (ChatGPT)
			TempBufferVerticesStored = 0;
			TempBufferIndicesStored = 0;
			int alpha = camDist < 250.0f ? 160 : (300.0f-camDist)/(300.0f-250.0f)*160;

			TempBufferVerticesStored += 2;
			PlaneBanners_BackendSetRGBA(0,alpha);
			PlaneBanners_BackendSetRGBA(1,alpha);
			PlaneBanners_BackendSetPos(0, banner->m_pos[2].x, banner->m_pos[2].y, banner->m_pos[2].z);
			PlaneBanners_BackendSetPos(1, banner->m_pos[2].x, banner->m_pos[2].y, banner->m_pos[2].z - 4.0f);
			PlaneBanners_BackendSetU(0, 0.0f);
			PlaneBanners_BackendSetV(0, 0.0f);
			PlaneBanners_BackendSetU(1, 0.0f);
			PlaneBanners_BackendSetV(1, 1.0f);
			for(i = 2; i < 8; i++){
				// Preserve the original branch and traversal order
				PlaneBanners_BackendSetRGBA(TempBufferVerticesStored+0,alpha);
				PlaneBanners_BackendSetRGBA(TempBufferVerticesStored+1,alpha);
				PlaneBanners_BackendSetPos(TempBufferVerticesStored+0, banner->m_pos[i].x, banner->m_pos[i].y, banner->m_pos[i].z);
				PlaneBanners_BackendSetPos(TempBufferVerticesStored+1, banner->m_pos[i].x, banner->m_pos[i].y, banner->m_pos[i].z - 4.0f);
				PlaneBanners_BackendSetU(TempBufferVerticesStored+0, (i-2)/5.0f);
				PlaneBanners_BackendSetV(TempBufferVerticesStored+0, 0.0f);
				PlaneBanners_BackendSetU(TempBufferVerticesStored+1, (i-2)/5.0f);
				PlaneBanners_BackendSetV(TempBufferVerticesStored+1, 1.0f);
				TempBufferRenderIndexList[TempBufferIndicesStored+0] = TempBufferVerticesStored-2;
				TempBufferRenderIndexList[TempBufferIndicesStored+1] = TempBufferVerticesStored-1;
				TempBufferRenderIndexList[TempBufferIndicesStored+2] = TempBufferVerticesStored+1;
				TempBufferRenderIndexList[TempBufferIndicesStored+3] = TempBufferVerticesStored-2;
				TempBufferRenderIndexList[TempBufferIndicesStored+4] = TempBufferVerticesStored+1;
				TempBufferRenderIndexList[TempBufferIndicesStored+5] = TempBufferVerticesStored;
				TempBufferVerticesStored += 2;
				TempBufferIndicesStored += 6;
			}
			PlaneBanners_BackendZWrite(false);
			PlaneBanners_BackendZTest(true);
			PlaneBanners_BackendFog(true);
			PlaneBanners_BackendSrcBlend();
			PlaneBanners_BackendDestBlend();
			PlaneBanners_BackendVertexAlpha(true);
			// Read the C-owned rubbish texture through its typed raster boundary
			PlaneBanners_BackendRaster( Rubbish_BackendRaster(gpRubbishTexture[2]));

			if(PlaneBanners_BackendTransform(TempBufferVerticesStored)){
				// Preserve the original branch and traversal order
				PlaneBanners_BackendRenderIndexed( TempBufferIndicesStored);
				PlaneBanners_BackendEnd();
			}

			PlaneBanners_BackendZWrite(true);
			PlaneBanners_BackendVertexAlpha(false);
			PlaneBanners_BackendFog(false);
			PlaneBanners_BackendVertexAlpha(false);

			TempBufferVerticesStored = 0;
			TempBufferIndicesStored = 0;
		}
	}
}

static void
PlaneBanner_RegisterPoint(PlaneBanner *banner, PlaneBannerVector pos)
{
    // Preserve original arithmetic, live owner reads and state-write ordering
	banner->m_pos[0] = pos;
}

void
PlaneBanners_Init(void)
{
    // Preserve original arithmetic, live owner reads and state-write ordering
	int i;
	for(i = 0; i < ARRAY_SIZE(aArray); i++)
		PlaneBanner_Init(&aArray[i]);
}

void
PlaneBanners_Update(void)
{
    // Preserve original arithmetic, live owner reads and state-write ordering
	int i;
	for(i = 0; i < ARRAY_SIZE(aArray); i++)
		PlaneBanner_Update(&aArray[i]);
}

void
PlaneBanners_Render(void)
{
    // Preserve original arithmetic, live owner reads and state-write ordering
	int i;
	for(i = 0; i < ARRAY_SIZE(aArray); i++)
		PlaneBanner_Render(&aArray[i]);
}

void
PlaneBanners_RegisterPoint(PlaneBannerVector pos, uint32_t id)
{
    // Preserve original arithmetic, live owner reads and state-write ordering
	PlaneBanner_RegisterPoint(&aArray[id],pos);
}
void PlaneBanners_RegisterPointVector(const void *position,uint32_t id)
{
    // Capture the original typed position once before writing any destination fields
    PlaneBanners_RegisterPoint(PlaneBanners_BackendVector(position),id);
}
//- rouz edit (ChatGPT)
