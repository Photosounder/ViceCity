#include "common.h"

#include "main.h"
#include "CutsceneMgr.h"
#include "Lights.h"
#include "Camera.h"
#include "Weather.h"
#include "World.h"
#include "Collision.h"
#include "Sprite.h"
#include "RenderBuffer.h" // rouz edit (ChatGPT)
#include "Timer.h"
#include "PointLights.h"
//+ rouz edit (ChatGPT)
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
#include "custompipes.h"
#endif
//- rouz edit (ChatGPT)

int16 CPointLights::NumLights;
CRegisteredPointLight CPointLights::aLights[NUMPOINTLIGHTS];
CVector CPointLights::aCachedMapReads[32];
float CPointLights::aCachedMapReadResults[32];
int32 CPointLights::NextCachedValue;
//+ rouz edit (ChatGPT)
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
static CRegisteredPointLight aEnvMapLights[NUMPOINTLIGHTS];
static int16 NumEnvMapLights;
#endif
//- rouz edit (ChatGPT)

void
CPointLights::Init(void)
{
	for(int i = 0; i < ARRAY_SIZE(aCachedMapReads); i++){
		aCachedMapReads[i] = CVector(0.0f, 0.0f, 0.0f);
		aCachedMapReadResults[i] = 0.0f;
	}
	NextCachedValue = 0;
}

void
CPointLights::InitPerFrame(void)
{
	NumLights = 0;
	//+ rouz edit (ChatGPT)
	// Reset reflection-only point lights for the new frame
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	NumEnvMapLights = 0;
#endif
	//- rouz edit (ChatGPT)
}

//+ rouz edit (ChatGPT)
void
CPointLights::ResetEnvMapLights(void)
{
	// Clear reflection-only registrations before each auxiliary camera pass
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	NumEnvMapLights = 0;
#endif
}
//- rouz edit (ChatGPT)

#define MAX_DIST 22.0f

void
CPointLights::AddLight(uint8 type, CVector coors, CVector dir, float radius, float red, float green, float blue, uint8 fogType, bool castExtraShadows)
{
	CVector dist;
	CVector cameraPosition;
	float distance;
	CRegisteredPointLight *lightArray;
	int16 *lightCount;

	// The check is done in some weird way in the game
	// we're doing it a bit better here
	//+ rouz edit (ChatGPT)
	// Select the active camera's independent registration list
	cameraPosition = TheCamera.GetPosition();
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	if(CustomPipes::bRenderingEnvMap && CustomPipes::EnvMapCam && CustomPipes::EnvMapCam->getFrame() &&
	   CustomPipes::EnvMapCam->getFrame()->getLTM()){
		cameraPosition = CVector(CustomPipes::EnvMapCam->getFrame()->getLTM()->pos);
		lightArray = aEnvMapLights;
		lightCount = &NumEnvMapLights;
	}else
#endif
	{
		lightArray = aLights;
		lightCount = &NumLights;
	}
	if(*lightCount >= NUMPOINTLIGHTS)
		return;
	//- rouz edit (ChatGPT)

	dist = coors - cameraPosition;
	if(Abs(dist.x) < MAX_DIST && Abs(dist.y) < MAX_DIST){
		distance = dist.Magnitude();
		if(distance < MAX_DIST){
			//+ rouz edit (ChatGPT)
			// Store this source in the list selected for its active camera
			lightArray[*lightCount].type = type;
			lightArray[*lightCount].fogType = fogType;
			lightArray[*lightCount].coors = coors;
			lightArray[*lightCount].dir = dir;
			lightArray[*lightCount].radius = radius;
			lightArray[*lightCount].castExtraShadows = castExtraShadows;
			if(distance < MAX_DIST*0.75f){
				lightArray[*lightCount].red = red;
				lightArray[*lightCount].green = green;
				lightArray[*lightCount].blue = blue;
			}else{
				float fade = 1.0f - (distance/MAX_DIST - 0.75f)*4.0f;
				lightArray[*lightCount].red = red * fade;
				lightArray[*lightCount].green = green * fade;
				lightArray[*lightCount].blue = blue * fade;
			}
			(*lightCount)++;
			//- rouz edit (ChatGPT)
		}
	}
}

float
CPointLights::GenerateLightsAffectingObject(Const CVector *objCoors)
{
	int i;
	float ret;
	CVector dist;
	float radius, distance;
	//+ rouz edit (ChatGPT)
	// Keep the primary and reflection point-light sources separate
	CRegisteredPointLight *lightLists[2];
	int32 lightCounts[2];
	bool useEnvMapLights;
	//- rouz edit (ChatGPT)

	ret = 1.0f;
	//+ rouz edit (ChatGPT)
	// Select the light lists for the active world camera
	useEnvMapLights = false;
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	useEnvMapLights = CustomPipes::bRenderingEnvMap;
#endif
	lightLists[0] = aLights;
	lightCounts[0] = NumLights;
	int lightSetCount = 1;
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	if(useEnvMapLights){
		lightLists[1] = aEnvMapLights;
		lightCounts[1] = NumEnvMapLights;
		lightSetCount = 2;
	}
#endif
	//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Apply every camera-visible source while preferring reflection-specific registrations
	for(int lightSet = 0; lightSet < lightSetCount; lightSet++){
		// Process the selected camera's current point-light registrations
		for(i = 0; i < lightCounts[lightSet]; i++){
			CRegisteredPointLight *light = &lightLists[lightSet][i];
			// Prefer reflection-camera copies when the main camera registered the same source
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
			if(lightSet == 0 && useEnvMapLights){
				bool hasReflectionCopy = false;
				for(int j = 0; j < NumEnvMapLights; j++){
					if(aEnvMapLights[j].type == light->type &&
					   aEnvMapLights[j].fogType == light->fogType &&
					   Abs(aEnvMapLights[j].radius - light->radius) < 0.01f &&
					   Abs(aEnvMapLights[j].coors.x - light->coors.x) < 0.01f &&
					   Abs(aEnvMapLights[j].coors.y - light->coors.y) < 0.01f &&
					   Abs(aEnvMapLights[j].coors.z - light->coors.z) < 0.01f &&
					   Abs(aEnvMapLights[j].dir.x - light->dir.x) < 0.01f &&
					   Abs(aEnvMapLights[j].dir.y - light->dir.y) < 0.01f &&
					   Abs(aEnvMapLights[j].dir.z - light->dir.z) < 0.01f){
						hasReflectionCopy = true;
						break;
					}
				}
				if(hasReflectionCopy)
					continue;
			}
#endif
			if(light->type == LIGHT_FOGONLY || light->type == LIGHT_FOGONLY_ALWAYS)
				continue;

			// Apply this camera's registered point light to the object
			// Keep the existing distance calculation
			dist = light->coors - *objCoors;
			radius = light->radius;
			if(Abs(dist.x) < radius &&
			   Abs(dist.y) < radius &&
			   Abs(dist.z) < radius){

				distance = dist.Magnitude();
				if(distance < radius){

					float distNorm = distance/radius;
					if(light->type == LIGHT_DARKEN){
						// Darken the object more when the light is closer
						ret *= distNorm;
					}else{
						float intensity;
						// Fade the light by distance
						if(distNorm < 0.5f)
							intensity = 1.0f;
						else
							intensity = 1.0f - (distNorm - 0.5f)/(1.0f - 0.5f);

						if(distance != 0.0f){
							CVector dir = dist / distance;

							if(light->type == LIGHT_DIRECTIONAL){
								float dot = -DotProduct(dir, light->dir);
								intensity *= Max((dot-0.5f)*2.0f, 0.0f);
							}

							if(intensity > 0.0f)
								AddAnExtraDirectionalLight(Scene.world,
									dir.x, dir.y, dir.z,
									light->red*intensity, light->green*intensity, light->blue*intensity);
						}
					}
				}
			}
		}
	}
	//- rouz edit (ChatGPT)

	return ret;
}
extern RwRaster *gpPointlightRaster;

void
CPointLights::RemoveLightsAffectingObject(void)
{
	RemoveExtraDirectionalLights(Scene.world);
}

// for directional fog
#define FOG_AREA_LENGTH 12.0f
#define FOG_AREA_WIDTH 5.0f
// for pointlight fog
#define FOG_AREA_RADIUS 9.0f

float FogSizes[8] = { 1.3f, 2.0f, 1.7f, 2.0f, 1.4f, 2.1f, 1.5f, 2.3f };

//+ rouz edit (ChatGPT)
static void
StoreFogBillboard(const CVector &position, const CVector &cameraRight,
	const CVector &cameraUp, float size, float lightRed, float lightGreen,
	float lightBlue, float intensity, float rotation)
{
	// Ignore samples with no visible contribution
	if(intensity <= 0.0f)
		return;

	// Convert the existing sprite intensity into vertex color for the textured quad
	uint8 red = (uint8)(lightRed*intensity);
	uint8 green = (uint8)(lightGreen*intensity);
	uint8 blue = (uint8)(lightBlue*intensity);
	red = red*(int)intensity >> 8;
	green = green*(int)intensity >> 8;
	blue = blue*(int)intensity >> 8;

	// Rotate a world-facing quad in the reflection camera plane
	float cosine = Cos(rotation);
	float sine = Sin(rotation);
	CVector horizontal = cameraRight*(size*cosine) + cameraUp*(size*sine);
	CVector vertical = cameraUp*(size*cosine) - cameraRight*(size*sine);
	CVector corners[4] = {
		position-horizontal+vertical,
		position+horizontal+vertical,
		position-horizontal-vertical,
		position+horizontal-vertical
	};

	// Store the additive billboard in the shared immediate geometry buffer
	RwImVertexIndex *indices;
	RwIm3DVertex *vertices;
	RenderBuffer::StartStoring(6, 4, &indices, &vertices);
	for(int vertex = 0; vertex < 4; vertex++){
		RwIm3DVertexSetPos(&vertices[vertex], corners[vertex].x, corners[vertex].y, corners[vertex].z);
		RwIm3DVertexSetRGBA(&vertices[vertex], red, green, blue, 255);
	}
	RwIm3DVertexSetU(&vertices[0], 0.0f); RwIm3DVertexSetV(&vertices[0], 0.0f);
	RwIm3DVertexSetU(&vertices[1], 1.0f); RwIm3DVertexSetV(&vertices[1], 0.0f);
	RwIm3DVertexSetU(&vertices[2], 0.0f); RwIm3DVertexSetV(&vertices[2], 1.0f);
	RwIm3DVertexSetU(&vertices[3], 1.0f); RwIm3DVertexSetV(&vertices[3], 1.0f);
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 2; indices[4] = 1; indices[5] = 3;
	RenderBuffer::StopStoring();
}
//- rouz edit (ChatGPT)

void
CPointLights::RenderFogEffect(void)
{
	int i;
	float fogginess;
	CColPoint point;
	CEntity *entity;
	float xmin, ymin;
	float xmax, ymax;
	int16 xi, yi;
	CVector spriteCoors;
	float spritew, spriteh;

	if(CCutsceneMgr::IsRunning())
		return;

	PUSH_RENDERGROUP("CPointLights::RenderFogEffect");

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, gpPointlightRaster);

	CSprite::InitSpriteBuffer();

	for(i = 0; i < NumLights; i++){
		if(aLights[i].fogType != FOG_NORMAL && aLights[i].fogType != FOG_ALWAYS)
			continue;

		fogginess = aLights[i].fogType == FOG_NORMAL ? CWeather::Foggyness : 1.0f;
		if(fogginess == 0.0f)
			continue;

		if(aLights[i].type == LIGHT_DIRECTIONAL){

			// TODO: test this. haven't found directional fog so far

			float coors2X = aLights[i].coors.x + FOG_AREA_LENGTH*aLights[i].dir.x;
			float coors2Y = aLights[i].coors.y + FOG_AREA_LENGTH*aLights[i].dir.y;

			if(coors2X < aLights[i].coors.x){
				xmin = coors2X;
				xmax = aLights[i].coors.x;
			}else{
				xmax = coors2X;
				xmin = aLights[i].coors.x;
			}
			if(coors2Y < aLights[i].coors.y){
				ymin = coors2Y;
				ymax = aLights[i].coors.y;
			}else{
				ymax = coors2Y;
				ymin = aLights[i].coors.y;
			}

			xmin -= 5.0f;
			ymin -= 5.0f;
			xmax += 5.0f;
			ymax += 5.0f;

			for(xi = (int16)xmin - (int16)xmin % 4; xi <= (int16)xmax + 4; xi += 4){
				for(yi = (int16)ymin - (int16)ymin % 4; yi <= (int16)ymax + 4; yi += 4){
					// Some kind of pseudo random number?
					int r = (xi ^ yi)>>2 & 0xF;
					if((r & 1) == 0)
						continue;

					// Check if fog effect is close enough to directional line in x and y
					float dx = xi - aLights[i].coors.x;
					float dy = yi - aLights[i].coors.y;
					float dot = dx*aLights[i].dir.x + dy*aLights[i].dir.y;
					float distsq = sq(dx) + sq(dy);
					float linedistsq = distsq - sq(dot);
					if(dot > 0.0f && dot < FOG_AREA_LENGTH && linedistsq < sq(FOG_AREA_WIDTH)){
						CVector fogcoors(xi, yi, aLights[i].coors.z + 10.0f);
						if(CWorld::ProcessVerticalLine(fogcoors, fogcoors.z - 20.0f,
								point, entity, true, false, false, false, true, false, nil)){
							// Now same check again in xyz
							fogcoors.z = point.point.z + 1.3f;
							// actually we don't have to recalculate x and y, but the game does it that way
							dx = xi - aLights[i].coors.x;
							dy = yi - aLights[i].coors.y;
							float dz = fogcoors.z - aLights[i].coors.z;
							dot = dx*aLights[i].dir.x + dy*aLights[i].dir.y + dz*aLights[i].dir.z;
							distsq = sq(dx) + sq(dy) + sq(dz);
							linedistsq = distsq - sq(dot);
							if(dot > 0.0f && dot < FOG_AREA_LENGTH && linedistsq < sq(FOG_AREA_WIDTH)){
								float intensity = 158.0f * fogginess;
								// more intensity the smaller the angle
								intensity *= dot/Sqrt(distsq);
								// more intensity the closer to light source
								intensity *= 1.0f - sq(dot/FOG_AREA_LENGTH);
								// more intensity the closer to line
								intensity *= 1.0f - sq(Sqrt(linedistsq) / FOG_AREA_WIDTH);

								if(CSprite::CalcScreenCoors(fogcoors, &spriteCoors, &spritew, &spriteh, true)) {
									float rotation = (CTimer::GetTimeInMilliseconds()&0x1FFF) * 2*3.14f / 0x2000;
									float size = FogSizes[r>>1];
									CSprite::RenderBufferedOneXLUSprite_Rotate_Aspect(spriteCoors.x, spriteCoors.y, spriteCoors.z,
										spritew * size, spriteh * size,
										aLights[i].red * intensity, aLights[i].green * intensity, aLights[i].blue * intensity,
										intensity, 1/spriteCoors.z, rotation, 255);
								}
							}
						}
					}
				}
			}

		}else if(aLights[i].type == LIGHT_POINT || aLights[i].type == LIGHT_FOGONLY || aLights[i].type == LIGHT_FOGONLY_ALWAYS){
			float groundZ;
			if(ProcessVerticalLineUsingCache(aLights[i].coors, &groundZ)){
				xmin = aLights[i].coors.x - FOG_AREA_RADIUS;
				ymin = aLights[i].coors.y - FOG_AREA_RADIUS;
				xmax = aLights[i].coors.x + FOG_AREA_RADIUS;
				ymax = aLights[i].coors.y + FOG_AREA_RADIUS;

				for(xi = (int16)xmin - (int16)xmin % 2; xi <= (int16)xmax + 2; xi += 2){
					for(yi = (int16)ymin - (int16)ymin % 2; yi <= (int16)ymax + 2; yi += 2){
						// Some kind of pseudo random number?
						int r = (xi ^ yi)>>1 & 0xF;
						if((r & 1) == 0)
							continue;

						float dx = xi - aLights[i].coors.x;
						float dy = yi - aLights[i].coors.y;
						float lightdist = Sqrt(sq(dx) + sq(dy));
						if(lightdist < FOG_AREA_RADIUS){
							dx = xi - TheCamera.GetPosition().x;
							dy = yi - TheCamera.GetPosition().y;
							float camdist = Sqrt(sq(dx) + sq(dy));
							if(camdist < MAX_DIST){
								float intensity;
								// distance fade
								if(camdist < MAX_DIST/2)
									intensity = 1.0f;
								else
									intensity = 1.0f - (camdist - MAX_DIST/2) / (MAX_DIST/2);
								intensity *= 132.0f * fogginess;
								// more intensity the closer to light source
								intensity *= 1.0f - sq(lightdist / FOG_AREA_RADIUS);

								CVector fogcoors(xi, yi, groundZ + 1.6f);
								if(CSprite::CalcScreenCoors(fogcoors, &spriteCoors, &spritew, &spriteh, true)) {
									float rotation = (CTimer::GetTimeInMilliseconds()&0x3FFF) * 2*3.14f / 0x4000;
									float size = FogSizes[r>>1];
									CSprite::RenderBufferedOneXLUSprite_Rotate_Aspect(spriteCoors.x, spriteCoors.y, spriteCoors.z,
										spritew * size, spriteh * size,
										aLights[i].red * intensity, aLights[i].green * intensity, aLights[i].blue * intensity,
										intensity, 1/spriteCoors.z, rotation, 255);
								}
							}
						}
					}
				}
			}
		}
	}

	CSprite::FlushSpriteBuffer();

	POP_RENDERGROUP();
}

//+ rouz edit (ChatGPT)
void
CPointLights::RenderFogEffectForEnvMapLights(RwCamera *camera, CRegisteredPointLight *lights, int32 lightCount)
{
	// Skip fog billboards when the reflection camera or fog texture is unavailable
	if(camera == nil || gpPointlightRaster == nil || CCutsceneMgr::IsRunning())
		return;
	RwFrame *cameraFrame = RwCameraGetFrame(camera);
	// Skip rendering if the reflection camera has no frame
	if(cameraFrame == nil)
		return;
	RwMatrix *cameraMatrix = RwFrameGetMatrix(cameraFrame);
	// Skip rendering if the reflection frame has no matrix
	if(cameraMatrix == nil)
		return;

	// Extract the reflection camera basis used to orient each fog billboard
	CVector cameraPos(cameraMatrix->pos);
	CVector cameraForward(cameraMatrix->at);
	CVector cameraRight(cameraMatrix->right);
	CVector cameraUp(cameraMatrix->up);
	cameraForward.Normalise();
	cameraRight.Normalise();
	cameraUp.Normalise();

	// Preserve pending geometry before switching the shared buffer to fog rendering
	RenderBuffer::RenderStuffInBuffer();
	PUSH_RENDERGROUP("CPointLights::RenderFogEffectForEnvMap");
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, gpPointlightRaster);
	RenderBuffer::ClearRenderBuffer();

	// Project active fog lights into deterministic world-space samples
	for(int i = 0; i < lightCount; i++){
		// Skip lights that do not contribute fog
		if(lights[i].fogType != FOG_NORMAL && lights[i].fogType != FOG_ALWAYS)
			continue;
		float fogginess = lights[i].fogType == FOG_NORMAL ? CWeather::Foggyness : 1.0f;
		// Skip fog lights whose current weather intensity is zero
		if(fogginess == 0.0f)
			continue;

		// Render the directional fog corridor as world-space billboards
		if(lights[i].type == LIGHT_DIRECTIONAL){
			// Lay samples along a directional light's fog corridor
			float coors2X = lights[i].coors.x + FOG_AREA_LENGTH*lights[i].dir.x;
			float coors2Y = lights[i].coors.y + FOG_AREA_LENGTH*lights[i].dir.y;
			float xmin = Min(coors2X, lights[i].coors.x) - 5.0f;
			float xmax = Max(coors2X, lights[i].coors.x) + 5.0f;
			float ymin = Min(coors2Y, lights[i].coors.y) - 5.0f;
			float ymax = Max(coors2Y, lights[i].coors.y) + 5.0f;
			for(int16 xi = (int16)xmin - (int16)xmin % 4; xi <= (int16)xmax + 4; xi += 4){
				for(int16 yi = (int16)ymin - (int16)ymin % 4; yi <= (int16)ymax + 4; yi += 4){
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
					CVector fogPosition(xi, yi, lights[i].coors.z + 10.0f);
					CColPoint point;
					CEntity *entity;
					if(!CWorld::ProcessVerticalLine(fogPosition, fogPosition.z - 20.0f,
						point, entity, true, false, false, false, true, false, nil))
						continue;
					fogPosition.z = point.point.z + 1.3f;
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
					float rotation = (CTimer::GetTimeInMilliseconds()&0x1FFF)*TWOPI/0x2000;
					StoreFogBillboard(fogPosition, cameraRight, cameraUp, FogSizes[r>>1],
						lights[i].red, lights[i].green, lights[i].blue, intensity, rotation);
				}
			}
		}else if(lights[i].type == LIGHT_POINT || lights[i].type == LIGHT_FOGONLY || lights[i].type == LIGHT_FOGONLY_ALWAYS){
			// Scatter samples around point lights that project fog onto the ground
			float groundZ;
			// Skip point lights without a ground sample
			if(!ProcessVerticalLineUsingCache(lights[i].coors, &groundZ))
				continue;
			float xmin = lights[i].coors.x - FOG_AREA_RADIUS;
			float ymin = lights[i].coors.y - FOG_AREA_RADIUS;
			float xmax = lights[i].coors.x + FOG_AREA_RADIUS;
			float ymax = lights[i].coors.y + FOG_AREA_RADIUS;
			for(int16 xi = (int16)xmin - (int16)xmin % 2; xi <= (int16)xmax + 2; xi += 2){
				for(int16 yi = (int16)ymin - (int16)ymin % 2; yi <= (int16)ymax + 2; yi += 2){
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
					CVector fogPosition(xi, yi, groundZ + 1.6f);
					float rotation = (CTimer::GetTimeInMilliseconds()&0x3FFF)*TWOPI/0x4000;
					StoreFogBillboard(fogPosition, cameraRight, cameraUp, FogSizes[r>>1],
						lights[i].red, lights[i].green, lights[i].blue, intensity, rotation);
				}
			}
		}
	}

	// Flush the fog billboards and restore the normal effect render states
	RenderBuffer::RenderStuffInBuffer();
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLBACK);
	POP_RENDERGROUP();
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
void
CPointLights::RenderFogEffectForEnvMap(RwCamera *camera)
{
	// Merge the two camera lists while preferring reflection-correct color fades
	CRegisteredPointLight mergedLights[NUMPOINTLIGHTS * 2];
	int32 mergedCount = 0;
	// Preserve main-view fog sources that were not rediscovered in the reflection pass
	for(int i = 0; i < NumLights; i++)
		mergedLights[mergedCount++] = aLights[i];
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	// Replace duplicate main-view sources with their reflection-camera-faded registrations
	for(int i = 0; i < NumEnvMapLights; i++){
		int duplicateIndex = -1;
		for(int j = 0; j < mergedCount; j++){
			if(mergedLights[j].type == aEnvMapLights[i].type &&
			   mergedLights[j].fogType == aEnvMapLights[i].fogType &&
			   Abs(mergedLights[j].radius - aEnvMapLights[i].radius) < 0.01f &&
			   Abs(mergedLights[j].coors.x - aEnvMapLights[i].coors.x) < 0.01f &&
			   Abs(mergedLights[j].coors.y - aEnvMapLights[i].coors.y) < 0.01f &&
			   Abs(mergedLights[j].coors.z - aEnvMapLights[i].coors.z) < 0.01f &&
			   Abs(mergedLights[j].dir.x - aEnvMapLights[i].dir.x) < 0.01f &&
			   Abs(mergedLights[j].dir.y - aEnvMapLights[i].dir.y) < 0.01f &&
			   Abs(mergedLights[j].dir.z - aEnvMapLights[i].dir.z) < 0.01f){
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
	RenderFogEffectForEnvMapLights(camera, mergedLights, mergedCount);
}
//- rouz edit (ChatGPT)

bool
CPointLights::ProcessVerticalLineUsingCache(CVector coors, float *groundZ)
{
	for(int i = 0; i < ARRAY_SIZE(aCachedMapReads); i++)
		if(aCachedMapReads[i] == coors){
			*groundZ = aCachedMapReadResults[i];
			return true;
		}

	CColPoint point;
	CEntity *entity;
	if(CWorld::ProcessVerticalLine(coors, coors.z - 20.0f, point, entity, true, false, false, false, true, false, nil)){
		aCachedMapReads[NextCachedValue] = coors;
		aCachedMapReadResults[NextCachedValue] = point.point.z;
		NextCachedValue = (NextCachedValue+1) % ARRAY_SIZE(aCachedMapReads);
		*groundZ = point.point.z;
		return true;
	}
	return false;
}
