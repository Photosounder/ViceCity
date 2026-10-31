//+ rouz edit (ChatGPT)
#include "PointLights_backend.h"
#include "Timer.h"
#include "Draw_constants.h"
#include <stddef.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define sq(a) ((a)*(a))
#if defined(_MSC_VER)
float fast_absf(float value);
#define Abs fast_absf
#else
#define Abs fabsf
#endif
#define Sqrt sqrtf
#define Sin sinf
#define Cos cosf
typedef PointLightsVector (*PointLightsRead)(const void *);
static PointLightsVector PointLights_Read(const void *value)
{
    // Read actual C vector storage at the original object-coordinate use
    return *(const PointLightsVector *)value;
}
static PointLightsVector PointLights_Add(PointLightsVector a,PointLightsVector b)
{
    // Retain component-wise original addition
    PointLightsVector r={a.x+b.x,a.y+b.y,a.z+b.z};return r;
}
static PointLightsVector PointLights_Subtract(PointLightsVector a,PointLightsVector b)
{
    // Retain component-wise original subtraction
    PointLightsVector r={a.x-b.x,a.y-b.y,a.z-b.z};return r;
}
static PointLightsVector PointLights_Scale(PointLightsVector a,float b)
{
    // Retain original vector-times-scalar order
    PointLightsVector r={a.x*b,a.y*b,a.z*b};return r;
}
static PointLightsVector PointLights_Divide(PointLightsVector a,float b)
{
    // Retain three original divisions rather than multiplying by a reciprocal
    PointLightsVector r={a.x/b,a.y/b,a.z/b};return r;
}
static float PointLights_Magnitude(PointLightsVector a)
{
    // Retain original float square root and product/sum order
    return Sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}
static float PointLights_Dot(PointLightsVector a,PointLightsVector b)
{
    // Retain the three original dot products and additions
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static bool PointLights_Equal(PointLightsVector a,PointLightsVector b)
{
    // Retain original exact component comparisons and short-circuit order
    return a.x==b.x && a.y==b.y && a.z==b.z;
}
static void PointLights_Normalise(PointLightsVector *a)
{
    // Preserve the original reciprocal float root and x-only fallback
    float square=a->x*a->x+a->y*a->y+a->z*a->z;
    if(square>0.0f){
        // Scale each original component in order
        float scale=1.0f/Sqrt(square);a->x*=scale;a->y*=scale;a->z*=scale;
    }else a->x=1.0f;
}
int16_t PointLights_NumLights;
PointLight PointLights_aLights[NUMPOINTLIGHTS];
PointLightsVector PointLights_aCachedMapReads[32];
float PointLights_aCachedMapReadResults[32];
int32_t PointLights_NextCachedValue;
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
static PointLight aEnvMapLights[NUMPOINTLIGHTS];
static int16_t NumEnvMapLights;
#endif

void
PointLights_Init(void)
{
	// Preserve the complete original point-light traversal and operation order
	for(int i = 0; i < ARRAY_SIZE(PointLights_aCachedMapReads); i++){
		// Retain the original eligibility, arithmetic and write order
		PointLights_aCachedMapReads[i] = (PointLightsVector){0.0f,0.0f,0.0f};
		PointLights_aCachedMapReadResults[i] = 0.0f;
	}
	PointLights_NextCachedValue = 0;
}

void
PointLights_InitPerFrame(void)
{
	// Preserve the complete original point-light traversal and operation order
	PointLights_NumLights = 0;
	// Reset reflection-only point lights for the new frame
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	NumEnvMapLights = 0;
#endif
}

void
PointLights_ResetEnvMapLights(void)
{
	// Preserve the complete original point-light traversal and operation order
	// Clear reflection-only registrations before each auxiliary camera pass
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	NumEnvMapLights = 0;
#endif
}

#define MAX_DIST 22.0f

void
PointLights_AddLight(uint8_t type, PointLightsVector coors, PointLightsVector dir, float radius, float red, float green, float blue, uint8_t fogType, bool castExtraShadows)
{
	// Preserve the complete original point-light traversal and operation order
	PointLightsVector dist;
	PointLightsVector cameraPosition;
	float distance;
	PointLight *lightArray;
	int16_t *lightCount;

	// The check is done in some weird way in the game
	// we're doing it a bit better here
	// Select the active camera's independent registration list
	cameraPosition = PointLights_BackendCamera();
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	if(PointLights_BackendEnvMap() && PointLights_BackendEnvCamera() && PointLights_BackendEnvFrame(PointLights_BackendEnvCamera()) &&
	   PointLights_BackendFrameLTM(PointLights_BackendEnvFrame(PointLights_BackendEnvCamera()))){
		// Retain the original eligibility, arithmetic and write order
		cameraPosition = PointLights_BackendMatrixPosition(PointLights_BackendFrameLTM(PointLights_BackendEnvFrame(PointLights_BackendEnvCamera())));
		lightArray = aEnvMapLights;
		lightCount = &NumEnvMapLights;
	}else
#endif
	{
		// Retain the original eligibility, arithmetic and write order
		lightArray = PointLights_aLights;
		lightCount = &PointLights_NumLights;
	}
	if(*lightCount >= NUMPOINTLIGHTS)
		return;

	dist = PointLights_Subtract(coors,cameraPosition);
	if(Abs(dist.x) < MAX_DIST && Abs(dist.y) < MAX_DIST){
		// Retain the original eligibility, arithmetic and write order
		distance = PointLights_Magnitude(dist);
		if(distance < MAX_DIST){
			// Retain the original eligibility, arithmetic and write order
			// Store this source in the list selected for its active camera
			lightArray[*lightCount].type = type;
			lightArray[*lightCount].fogType = fogType;
			lightArray[*lightCount].coors = coors;
			lightArray[*lightCount].dir = dir;
			lightArray[*lightCount].radius = radius;
			lightArray[*lightCount].castExtraShadows = castExtraShadows;
			if(distance < MAX_DIST*0.75f){
				// Retain the original eligibility, arithmetic and write order
				lightArray[*lightCount].red = red;
				lightArray[*lightCount].green = green;
				lightArray[*lightCount].blue = blue;
			}else{
				// Retain the original eligibility, arithmetic and write order
				float fade = 1.0f - (distance/MAX_DIST - 0.75f)*4.0f;
				lightArray[*lightCount].red = red * fade;
				lightArray[*lightCount].green = green * fade;
				lightArray[*lightCount].blue = blue * fade;
			}
			(*lightCount)++;
		}
	}
}

static float
PointLights_Generate(const void *objCoors, PointLightsRead read)
{
	// Preserve the complete original point-light traversal and operation order
	int i;
	float ret;
	PointLightsVector dist;
	float radius, distance;
	// Keep the primary and reflection point-light sources separate
	PointLight *lightLists[2];
	int32_t lightCounts[2];
	bool useEnvMapLights;

	ret = 1.0f;
	// Select the light lists for the active world camera
	useEnvMapLights = false;
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	useEnvMapLights = PointLights_BackendEnvMap();
#endif
	lightLists[0] = PointLights_aLights;
	lightCounts[0] = PointLights_NumLights;
	int lightSetCount = 1;
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	if(useEnvMapLights){
		// Retain the original eligibility, arithmetic and write order
		lightLists[1] = aEnvMapLights;
		lightCounts[1] = NumEnvMapLights;
		lightSetCount = 2;
	}
#endif

	// Apply every camera-visible source while preferring reflection-specific registrations
	for(int lightSet = 0; lightSet < lightSetCount; lightSet++){
		// Retain the original eligibility, arithmetic and write order
		// Process the selected camera's current point-light registrations
		for(i = 0; i < lightCounts[lightSet]; i++){
			// Retain the original eligibility, arithmetic and write order
			PointLight *light = &lightLists[lightSet][i];
			// Prefer reflection-camera copies when the main camera registered the same source
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
			if(lightSet == 0 && useEnvMapLights){
				// Retain the original eligibility, arithmetic and write order
				bool hasReflectionCopy = false;
				for(int j = 0; j < NumEnvMapLights; j++){
					// Retain the original eligibility, arithmetic and write order
					if(aEnvMapLights[j].type == light->type &&
					   aEnvMapLights[j].fogType == light->fogType &&
					   Abs(aEnvMapLights[j].radius - light->radius) < 0.01f &&
					   Abs(aEnvMapLights[j].coors.x - light->coors.x) < 0.01f &&
					   Abs(aEnvMapLights[j].coors.y - light->coors.y) < 0.01f &&
					   Abs(aEnvMapLights[j].coors.z - light->coors.z) < 0.01f &&
					   Abs(aEnvMapLights[j].dir.x - light->dir.x) < 0.01f &&
					   Abs(aEnvMapLights[j].dir.y - light->dir.y) < 0.01f &&
					   Abs(aEnvMapLights[j].dir.z - light->dir.z) < 0.01f){
						// Retain the original eligibility, arithmetic and write order
						hasReflectionCopy = true;
						break;
					}
				}
				if(hasReflectionCopy)
					continue;
			}
#endif
			if(light->type == POINTLIGHTS_LIGHT_FOGONLY || light->type == POINTLIGHTS_LIGHT_FOGONLY_ALWAYS)
				continue;

			// Apply this camera's registered point light to the object
			// Keep the existing distance calculation
			dist = PointLights_Subtract(light->coors,read(objCoors));
			radius = light->radius;
			if(Abs(dist.x) < radius &&
			   Abs(dist.y) < radius &&
			   Abs(dist.z) < radius){
				// Retain the original eligibility, arithmetic and write order

				distance = PointLights_Magnitude(dist);
				if(distance < radius){
					// Retain the original eligibility, arithmetic and write order

					float distNorm = distance/radius;
					if(light->type == POINTLIGHTS_LIGHT_DARKEN){
						// Retain the original eligibility, arithmetic and write order
						// Darken the object more when the light is closer
						ret *= distNorm;
					}else{
						// Retain the original eligibility, arithmetic and write order
						float intensity;
						// Fade the light by distance
						if(distNorm < 0.5f)
							intensity = 1.0f;
						else
							intensity = 1.0f - (distNorm - 0.5f)/(1.0f - 0.5f);

						if(distance != 0.0f){
							// Retain the original eligibility, arithmetic and write order
							PointLightsVector dir = PointLights_Divide(dist,distance);

							if(light->type == POINTLIGHTS_LIGHT_DIRECTIONAL){
								// Retain the original eligibility, arithmetic and write order
								float dot = -PointLights_Dot(dir,light->dir);
								intensity *= Max((dot-0.5f)*2.0f, 0.0f);
							}

							if(intensity > 0.0f)
								PointLights_BackendExtraLight(
									dir.x, dir.y, dir.z,
									light->red*intensity, light->green*intensity, light->blue*intensity);
						}
					}
				}
			}
		}
	}

	return ret;
}

void
PointLights_RemoveLightsAffectingObject(void)
{
	// Preserve the complete original point-light traversal and operation order
	PointLights_BackendRemoveExtra();
}

// for directional fog
#define FOG_AREA_LENGTH 12.0f
#define FOG_AREA_WIDTH 5.0f
// for pointlight fog
#define FOG_AREA_RADIUS 9.0f

float FogSizes[8] = { 1.3f, 2.0f, 1.7f, 2.0f, 1.4f, 2.1f, 1.5f, 2.3f };

static void
StoreFogBillboard(PointLightsVector position, PointLightsVector cameraRight,
	PointLightsVector cameraUp, float size, float lightRed, float lightGreen,
	float lightBlue, float intensity, float rotation)
{
	// Preserve the complete original point-light traversal and operation order
	// Ignore samples with no visible contribution
	if(intensity <= 0.0f)
		return;

	// Convert the existing sprite intensity into vertex color for the textured quad
	uint8_t red = (uint8_t)(lightRed*intensity);
	uint8_t green = (uint8_t)(lightGreen*intensity);
	uint8_t blue = (uint8_t)(lightBlue*intensity);
	red = red*(int)intensity >> 8;
	green = green*(int)intensity >> 8;
	blue = blue*(int)intensity >> 8;

	// Rotate a world-facing quad in the reflection camera plane
	float cosine = Cos(rotation);
	float sine = Sin(rotation);
	PointLightsVector horizontal = PointLights_Add(PointLights_Scale(cameraRight,(size*cosine)),PointLights_Scale(cameraUp,(size*sine)));
	PointLightsVector vertical = PointLights_Subtract(PointLights_Scale(cameraUp,(size*cosine)),PointLights_Scale(cameraRight,(size*sine)));
	PointLightsVector corners[4] = {
		// Retain the original eligibility, arithmetic and write order
		PointLights_Add(PointLights_Subtract(position,horizontal),vertical),
		PointLights_Add(PointLights_Add(position,horizontal),vertical),
		PointLights_Subtract(PointLights_Subtract(position,horizontal),vertical),
		PointLights_Subtract(PointLights_Add(position,horizontal),vertical)
	};

	// Store the additive billboard in the shared immediate geometry buffer
	uint16_t *indices;
	void *vertices;
	PointLights_BackendStart(6, 4, &indices, &vertices);
	for(int vertex = 0; vertex < 4; vertex++){
		// Retain the original eligibility, arithmetic and write order
		PointLights_BackendSetPos(vertices,vertex, corners[vertex].x, corners[vertex].y, corners[vertex].z);
		PointLights_BackendSetRGBA(vertices,vertex, red, green, blue, 255);
	}
	PointLights_BackendSetU(vertices,0, 0.0f); PointLights_BackendSetV(vertices,0, 0.0f);
	PointLights_BackendSetU(vertices,1, 1.0f); PointLights_BackendSetV(vertices,1, 0.0f);
	PointLights_BackendSetU(vertices,2, 0.0f); PointLights_BackendSetV(vertices,2, 1.0f);
	PointLights_BackendSetU(vertices,3, 1.0f); PointLights_BackendSetV(vertices,3, 1.0f);
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 2; indices[4] = 1; indices[5] = 3;
	PointLights_BackendStop();
}

void
PointLights_RenderFogEffect(void)
{
	// Preserve the complete original point-light traversal and operation order
	int i;
	float fogginess;
	float pointZ;
	float xmin, ymin;
	float xmax, ymax;
	int16_t xi, yi;
	PointLightsVector spriteCoors;
	float spritew, spriteh;

	if(PointLights_BackendCutscene())
		return;

	PointLights_BackendPush("CPointLights::RenderFogEffect");

	PointLights_BackendZWrite(false);
	PointLights_BackendVertexAlpha(true);
	PointLights_BackendSrcBlend(true);
	PointLights_BackendDestBlend(true);
	PointLights_BackendSetRaster(PointLights_BackendRaster());

	PointLights_BackendSpriteInit();

	for(i = 0; i < PointLights_NumLights; i++){
		// Retain the original eligibility, arithmetic and write order
		if(PointLights_aLights[i].fogType != POINTLIGHTS_FOG_NORMAL && PointLights_aLights[i].fogType != POINTLIGHTS_FOG_ALWAYS)
			continue;

		fogginess = PointLights_aLights[i].fogType == POINTLIGHTS_FOG_NORMAL ? PointLights_BackendFogginess() : 1.0f;
		if(fogginess == 0.0f)
			continue;

		if(PointLights_aLights[i].type == POINTLIGHTS_LIGHT_DIRECTIONAL){
			// Retain the original eligibility, arithmetic and write order

			// TODO: test this. haven't found directional fog so far

			float coors2X = PointLights_aLights[i].coors.x + FOG_AREA_LENGTH*PointLights_aLights[i].dir.x;
			float coors2Y = PointLights_aLights[i].coors.y + FOG_AREA_LENGTH*PointLights_aLights[i].dir.y;

			if(coors2X < PointLights_aLights[i].coors.x){
				// Retain the original eligibility, arithmetic and write order
				xmin = coors2X;
				xmax = PointLights_aLights[i].coors.x;
			}else{
				// Retain the original eligibility, arithmetic and write order
				xmax = coors2X;
				xmin = PointLights_aLights[i].coors.x;
			}
			if(coors2Y < PointLights_aLights[i].coors.y){
				// Retain the original eligibility, arithmetic and write order
				ymin = coors2Y;
				ymax = PointLights_aLights[i].coors.y;
			}else{
				// Retain the original eligibility, arithmetic and write order
				ymax = coors2Y;
				ymin = PointLights_aLights[i].coors.y;
			}

			xmin -= 5.0f;
			ymin -= 5.0f;
			xmax += 5.0f;
			ymax += 5.0f;

			for(xi = (int16_t)xmin - (int16_t)xmin % 4; xi <= (int16_t)xmax + 4; xi += 4){
				// Retain the original eligibility, arithmetic and write order
				for(yi = (int16_t)ymin - (int16_t)ymin % 4; yi <= (int16_t)ymax + 4; yi += 4){
					// Retain the original eligibility, arithmetic and write order
					// Some kind of pseudo random number?
					int r = (xi ^ yi)>>2 & 0xF;
					if((r & 1) == 0)
						continue;

					// Check if fog effect is close enough to directional line in x and y
					float dx = xi - PointLights_aLights[i].coors.x;
					float dy = yi - PointLights_aLights[i].coors.y;
					float dot = dx*PointLights_aLights[i].dir.x + dy*PointLights_aLights[i].dir.y;
					float distsq = sq(dx) + sq(dy);
					float linedistsq = distsq - sq(dot);
					if(dot > 0.0f && dot < FOG_AREA_LENGTH && linedistsq < sq(FOG_AREA_WIDTH)){
						// Retain the original eligibility, arithmetic and write order
						PointLightsVector fogcoors={(float)xi,(float)yi,PointLights_aLights[i].coors.z + 10.0f};
						if(PointLights_BackendVerticalLine(fogcoors,fogcoors.z - 20.0f,&pointZ)){
							// Retain the original eligibility, arithmetic and write order
							// Now same check again in xyz
							fogcoors.z = pointZ + 1.3f;
							// actually we don't have to recalculate x and y, but the game does it that way
							dx = xi - PointLights_aLights[i].coors.x;
							dy = yi - PointLights_aLights[i].coors.y;
							float dz = fogcoors.z - PointLights_aLights[i].coors.z;
							dot = dx*PointLights_aLights[i].dir.x + dy*PointLights_aLights[i].dir.y + dz*PointLights_aLights[i].dir.z;
							distsq = sq(dx) + sq(dy) + sq(dz);
							linedistsq = distsq - sq(dot);
							if(dot > 0.0f && dot < FOG_AREA_LENGTH && linedistsq < sq(FOG_AREA_WIDTH)){
								// Retain the original eligibility, arithmetic and write order
								float intensity = 158.0f * fogginess;
								// more intensity the smaller the angle
								intensity *= dot/Sqrt(distsq);
								// more intensity the closer to light source
								intensity *= 1.0f - sq(dot/FOG_AREA_LENGTH);
								// more intensity the closer to line
								intensity *= 1.0f - sq(Sqrt(linedistsq) / FOG_AREA_WIDTH);

								if(PointLights_BackendScreen(fogcoors, &spriteCoors, &spritew, &spriteh, true)) {
									// Retain the original eligibility, arithmetic and write order
									float rotation = ((*Timer_GetTimeInMilliseconds())&0x1FFF) * 2*3.14f / 0x2000;
									float size = FogSizes[r>>1];
									PointLights_BackendSprite(spriteCoors.x, spriteCoors.y, spriteCoors.z,
										spritew * size, spriteh * size,
										PointLights_aLights[i].red * intensity, PointLights_aLights[i].green * intensity, PointLights_aLights[i].blue * intensity,
										intensity, 1/spriteCoors.z, rotation, 255);
								}
							}
						}
					}
				}
			}

		}else if(PointLights_aLights[i].type == POINTLIGHTS_LIGHT_POINT || PointLights_aLights[i].type == POINTLIGHTS_LIGHT_FOGONLY || PointLights_aLights[i].type == POINTLIGHTS_LIGHT_FOGONLY_ALWAYS){
			// Retain the original eligibility, arithmetic and write order
			float groundZ;
			if(PointLights_ProcessVerticalLineUsingCache(PointLights_aLights[i].coors, &groundZ)){
				// Retain the original eligibility, arithmetic and write order
				xmin = PointLights_aLights[i].coors.x - FOG_AREA_RADIUS;
				ymin = PointLights_aLights[i].coors.y - FOG_AREA_RADIUS;
				xmax = PointLights_aLights[i].coors.x + FOG_AREA_RADIUS;
				ymax = PointLights_aLights[i].coors.y + FOG_AREA_RADIUS;

				for(xi = (int16_t)xmin - (int16_t)xmin % 2; xi <= (int16_t)xmax + 2; xi += 2){
					// Retain the original eligibility, arithmetic and write order
					for(yi = (int16_t)ymin - (int16_t)ymin % 2; yi <= (int16_t)ymax + 2; yi += 2){
						// Retain the original eligibility, arithmetic and write order
						// Some kind of pseudo random number?
						int r = (xi ^ yi)>>1 & 0xF;
						if((r & 1) == 0)
							continue;

						float dx = xi - PointLights_aLights[i].coors.x;
						float dy = yi - PointLights_aLights[i].coors.y;
						float lightdist = Sqrt(sq(dx) + sq(dy));
						if(lightdist < FOG_AREA_RADIUS){
							// Retain the original eligibility, arithmetic and write order
							dx = xi - PointLights_BackendCamera().x;
							dy = yi - PointLights_BackendCamera().y;
							float camdist = Sqrt(sq(dx) + sq(dy));
							if(camdist < MAX_DIST){
								// Retain the original eligibility, arithmetic and write order
								float intensity;
								// distance fade
								if(camdist < MAX_DIST/2)
									intensity = 1.0f;
								else
									intensity = 1.0f - (camdist - MAX_DIST/2) / (MAX_DIST/2);
								intensity *= 132.0f * fogginess;
								// more intensity the closer to light source
								intensity *= 1.0f - sq(lightdist / FOG_AREA_RADIUS);

								PointLightsVector fogcoors={(float)xi,(float)yi,groundZ + 1.6f};
								if(PointLights_BackendScreen(fogcoors, &spriteCoors, &spritew, &spriteh, true)) {
									// Retain the original eligibility, arithmetic and write order
									float rotation = ((*Timer_GetTimeInMilliseconds())&0x3FFF) * 2*3.14f / 0x4000;
									float size = FogSizes[r>>1];
									PointLights_BackendSprite(spriteCoors.x, spriteCoors.y, spriteCoors.z,
										spritew * size, spriteh * size,
										PointLights_aLights[i].red * intensity, PointLights_aLights[i].green * intensity, PointLights_aLights[i].blue * intensity,
										intensity, 1/spriteCoors.z, rotation, 255);
								}
							}
						}
					}
				}
			}
		}
	}

	PointLights_BackendSpriteFlush();

	PointLights_BackendPop();
}

void
PointLights_RenderFogEffectForEnvMapLights(void *camera, PointLight *lights, int32_t lightCount)
{
	// Preserve the complete original point-light traversal and operation order
	// Skip fog billboards when the reflection camera or fog texture is unavailable
	if(camera == NULL || PointLights_BackendRaster() == NULL || PointLights_BackendCutscene())
		return;
	void *cameraFrame = PointLights_BackendCameraFrame(camera);
	// Skip rendering if the reflection camera has no frame
	if(cameraFrame == NULL)
		return;
	void *cameraMatrix = PointLights_BackendFrameMatrix(cameraFrame);
	// Skip rendering if the reflection frame has no matrix
	if(cameraMatrix == NULL)
		return;

	// Extract the reflection camera basis used to orient each fog billboard
	PointLightsVector cameraPos = PointLights_BackendMatrixPosition(cameraMatrix);
	PointLightsVector cameraForward = PointLights_BackendMatrixForward(cameraMatrix);
	PointLightsVector cameraRight = PointLights_BackendMatrixRight(cameraMatrix);
	PointLightsVector cameraUp = PointLights_BackendMatrixUp(cameraMatrix);
	PointLights_Normalise(&cameraForward);
	PointLights_Normalise(&cameraRight);
	PointLights_Normalise(&cameraUp);

	// Preserve pending geometry before switching the shared buffer to fog rendering
	PointLights_BackendBufferRender();
	PointLights_BackendPush("CPointLights::RenderFogEffectForEnvMap");
	PointLights_BackendFog(false);
	PointLights_BackendZWrite(false);
	PointLights_BackendZTest(true);
	PointLights_BackendVertexAlpha(true);
	PointLights_BackendSrcBlend(true);
	PointLights_BackendDestBlend(true);
	PointLights_BackendCull(true);
	PointLights_BackendSetRaster(PointLights_BackendRaster());
	PointLights_BackendBufferClear();

	// Project active fog lights into deterministic world-space samples
	for(int i = 0; i < lightCount; i++){
		// Retain the original eligibility, arithmetic and write order
		// Skip lights that do not contribute fog
		if(lights[i].fogType != POINTLIGHTS_FOG_NORMAL && lights[i].fogType != POINTLIGHTS_FOG_ALWAYS)
			continue;
		float fogginess = lights[i].fogType == POINTLIGHTS_FOG_NORMAL ? PointLights_BackendFogginess() : 1.0f;
		// Skip fog lights whose current weather intensity is zero
		if(fogginess == 0.0f)
			continue;

		// Render the directional fog corridor as world-space billboards
		if(lights[i].type == POINTLIGHTS_LIGHT_DIRECTIONAL){
			// Retain the original eligibility, arithmetic and write order
			// Lay samples along a directional light's fog corridor
			float coors2X = lights[i].coors.x + FOG_AREA_LENGTH*lights[i].dir.x;
			float coors2Y = lights[i].coors.y + FOG_AREA_LENGTH*lights[i].dir.y;
			float xmin = Min(coors2X, lights[i].coors.x) - 5.0f;
			float xmax = Max(coors2X, lights[i].coors.x) + 5.0f;
			float ymin = Min(coors2Y, lights[i].coors.y) - 5.0f;
			float ymax = Max(coors2Y, lights[i].coors.y) + 5.0f;
			for(int16_t xi = (int16_t)xmin - (int16_t)xmin % 4; xi <= (int16_t)xmax + 4; xi += 4){
				// Retain the original eligibility, arithmetic and write order
				for(int16_t yi = (int16_t)ymin - (int16_t)ymin % 4; yi <= (int16_t)ymax + 4; yi += 4){
					// Retain the original eligibility, arithmetic and write order
					// Keep the same stable sparse pattern as the main fog pass
					int r = (xi ^ yi)>>2 & 0xF;
					if((r & 1) == 0)
						continue;

					// Reject samples outside the directional fog corridor
					float dx = xi - lights[i].coors.x;
					float dy = yi - lights[i].coors.y;
					float dot = dx*lights[i].dir.x + dy*lights[i].dir.y;
					float distanceSqr = sq(dx) + sq(dy);
					float lineDistanceSqr = distanceSqr - sq(dot);
					lineDistanceSqr = Max(lineDistanceSqr, 0.0f);
					if(dot <= 0.0f || dot >= FOG_AREA_LENGTH || lineDistanceSqr >= sq(FOG_AREA_WIDTH))
						continue;

					// Place the sample just above the surface under the corridor
					PointLightsVector fogPosition={(float)xi,(float)yi,lights[i].coors.z + 10.0f};
					float pointZ;
					if(!PointLights_BackendVerticalLine(fogPosition,fogPosition.z - 20.0f,&pointZ))
						continue;
					fogPosition.z = pointZ + 1.3f;
					dx = xi - lights[i].coors.x;
					dy = yi - lights[i].coors.y;
					float dz = fogPosition.z - lights[i].coors.z;
					dot = dx*lights[i].dir.x + dy*lights[i].dir.y + dz*lights[i].dir.z;
					distanceSqr = sq(dx) + sq(dy) + sq(dz);
					lineDistanceSqr = distanceSqr - sq(dot);
					lineDistanceSqr = Max(lineDistanceSqr, 0.0f);
					// Reject samples that leave the corridor after accounting for ground height
					if(dot <= 0.0f || dot >= FOG_AREA_LENGTH || lineDistanceSqr >= sq(FOG_AREA_WIDTH))
						continue;

					// Fade directional fog by angle, distance, and corridor width
					float intensity = 158.0f*fogginess;
					intensity *= dot/Sqrt(distanceSqr);
					intensity *= 1.0f - sq(dot/FOG_AREA_LENGTH);
					intensity *= 1.0f - sq(Sqrt(lineDistanceSqr)/FOG_AREA_WIDTH);
					float rotation = ((*Timer_GetTimeInMilliseconds())&0x1FFF)*TWOPI/0x2000;
					StoreFogBillboard(fogPosition, cameraRight, cameraUp, FogSizes[r>>1],
						lights[i].red, lights[i].green, lights[i].blue, intensity, rotation);
				}
			}
		}else if(lights[i].type == POINTLIGHTS_LIGHT_POINT || lights[i].type == POINTLIGHTS_LIGHT_FOGONLY || lights[i].type == POINTLIGHTS_LIGHT_FOGONLY_ALWAYS){
			// Retain the original eligibility, arithmetic and write order
			// Scatter samples around point lights that project fog onto the ground
			float groundZ;
			// Skip point lights without a ground sample
			if(!PointLights_ProcessVerticalLineUsingCache(lights[i].coors, &groundZ))
				continue;
			float xmin = lights[i].coors.x - FOG_AREA_RADIUS;
			float ymin = lights[i].coors.y - FOG_AREA_RADIUS;
			float xmax = lights[i].coors.x + FOG_AREA_RADIUS;
			float ymax = lights[i].coors.y + FOG_AREA_RADIUS;
			for(int16_t xi = (int16_t)xmin - (int16_t)xmin % 2; xi <= (int16_t)xmax + 2; xi += 2){
				// Retain the original eligibility, arithmetic and write order
				for(int16_t yi = (int16_t)ymin - (int16_t)ymin % 2; yi <= (int16_t)ymax + 2; yi += 2){
					// Retain the original eligibility, arithmetic and write order
					// Keep the same stable sparse pattern as the main fog pass
					int r = (xi ^ yi)>>1 & 0xF;
					if((r & 1) == 0)
						continue;

					// Fade samples radially around the point light
					float dx = xi - lights[i].coors.x;
					float dy = yi - lights[i].coors.y;
					float lightDistance = Sqrt(sq(dx) + sq(dy));
					if(lightDistance >= FOG_AREA_RADIUS)
						continue;
					dx = xi - cameraPos.x;
					dy = yi - cameraPos.y;
					float cameraDistance = Sqrt(sq(dx) + sq(dy));
					// Omit point fog outside the existing camera distance fade
					if(cameraDistance >= MAX_DIST)
						continue;
					float intensity = cameraDistance < MAX_DIST/2 ? 1.0f :
						1.0f - (cameraDistance - MAX_DIST/2)/(MAX_DIST/2);
					intensity *= 132.0f*fogginess;
					intensity *= 1.0f - sq(lightDistance/FOG_AREA_RADIUS);

					// Draw the fog sample just above the sampled ground height
					PointLightsVector fogPosition={(float)xi,(float)yi,groundZ + 1.6f};
					float rotation = ((*Timer_GetTimeInMilliseconds())&0x3FFF)*TWOPI/0x4000;
					StoreFogBillboard(fogPosition, cameraRight, cameraUp, FogSizes[r>>1],
						lights[i].red, lights[i].green, lights[i].blue, intensity, rotation);
				}
			}
		}
	}

	// Flush the fog billboards and restore the normal effect render states
	PointLights_BackendBufferRender();
	PointLights_BackendZWrite(true);
	PointLights_BackendZTest(true);
	PointLights_BackendVertexAlpha(false);
	PointLights_BackendFog(false);
	PointLights_BackendSrcBlend(false);
	PointLights_BackendDestBlend(false);
	PointLights_BackendSetRaster(NULL);
	PointLights_BackendCull(false);
	PointLights_BackendPop();
}

void
PointLights_RenderFogEffectForEnvMap(void *camera)
{
	// Preserve the complete original point-light traversal and operation order
	// Merge the two camera lists while preferring reflection-correct color fades
	PointLight mergedLights[NUMPOINTLIGHTS * 2];
	int32_t mergedCount = 0;
	// Preserve main-view fog sources that were not rediscovered in the reflection pass
	for(int i = 0; i < PointLights_NumLights; i++)
		mergedLights[mergedCount++] = PointLights_aLights[i];
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	// Replace duplicate main-view sources with their reflection-camera-faded registrations
	for(int i = 0; i < NumEnvMapLights; i++){
		// Retain the original eligibility, arithmetic and write order
		int duplicateIndex = -1;
		for(int j = 0; j < mergedCount; j++){
			// Retain the original eligibility, arithmetic and write order
			if(mergedLights[j].type == aEnvMapLights[i].type &&
			   mergedLights[j].fogType == aEnvMapLights[i].fogType &&
			   Abs(mergedLights[j].radius - aEnvMapLights[i].radius) < 0.01f &&
			   Abs(mergedLights[j].coors.x - aEnvMapLights[i].coors.x) < 0.01f &&
			   Abs(mergedLights[j].coors.y - aEnvMapLights[i].coors.y) < 0.01f &&
			   Abs(mergedLights[j].coors.z - aEnvMapLights[i].coors.z) < 0.01f &&
			   Abs(mergedLights[j].dir.x - aEnvMapLights[i].dir.x) < 0.01f &&
			   Abs(mergedLights[j].dir.y - aEnvMapLights[i].dir.y) < 0.01f &&
			   Abs(mergedLights[j].dir.z - aEnvMapLights[i].dir.z) < 0.01f){
				// Retain the original eligibility, arithmetic and write order
				duplicateIndex = j;
				break;
			}
		}
		if(duplicateIndex >= 0)
			mergedLights[duplicateIndex] = aEnvMapLights[i];
		else if(mergedCount < ARRAY_SIZE(mergedLights))
			mergedLights[mergedCount++] = aEnvMapLights[i];
	}
#endif
	// Render the merged sources once to avoid doubled fog from lights seen by both cameras
	PointLights_RenderFogEffectForEnvMapLights(camera, mergedLights, mergedCount);
}

bool
PointLights_ProcessVerticalLineUsingCache(PointLightsVector coors, float *groundZ)
{
	// Preserve the complete original point-light traversal and operation order
	for(int i = 0; i < ARRAY_SIZE(PointLights_aCachedMapReads); i++)
		if(PointLights_Equal(PointLights_aCachedMapReads[i],coors)){
			// Retain the original eligibility, arithmetic and write order
			*groundZ = PointLights_aCachedMapReadResults[i];
			return true;
		}

	float pointZ;
	if(PointLights_BackendVerticalLine(coors,coors.z - 20.0f,&pointZ)){
		// Retain the original eligibility, arithmetic and write order
		PointLights_aCachedMapReads[PointLights_NextCachedValue] = coors;
		PointLights_aCachedMapReadResults[PointLights_NextCachedValue] = pointZ;
		PointLights_NextCachedValue = (PointLights_NextCachedValue+1) % ARRAY_SIZE(PointLights_aCachedMapReads);
		*groundZ = pointZ;
		return true;
	}
	return false;
}

float PointLights_GenerateLightsAffectingObject(const PointLightsVector *objCoors)
{
    // Preserve live C coordinate reads at every original light iteration
    return PointLights_Generate(objCoors,PointLights_Read);
}
float PointLights_GenerateLightsAffectingObjectLegacy(const void *objCoors)
{
    // Read actual legacy vector storage lazily at every original use
    return PointLights_Generate(objCoors,PointLights_BackendReadVector);
}
//- rouz edit (ChatGPT)
