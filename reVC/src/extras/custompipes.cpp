#define WITHD3D
#include "common.h"

#ifdef EXTENDED_PIPELINES

#include "main.h"
#include "RwHelper.h"
#include "Lights.h"
#include "Timecycle.h"
#include "FileMgr.h"
#include "Clock.h"
#include "Weather.h"
#include "ZoneCull.h" // rouz edit (ChatGPT)
#include "TxdStore.h"
#include "Renderer.h"
#include "World.h"
#include "custompipes.h"
//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
#include "SoftwarePolygons.h"
#include "Entity.h" // rouz edit (ChatGPT)
#include "Heli.h" // rouz edit (ChatGPT)
#include "Clouds.h" // rouz edit (ChatGPT)
#include "Coronas.h" // rouz edit (ChatGPT)
#include "Glass.h" // rouz edit (ChatGPT)
#include "Shadows.h" // rouz edit (ChatGPT)
#include "Skidmarks.h" // rouz edit (ChatGPT)
#include "Rubbish.h" // rouz edit (ChatGPT)
#include "Antennas.h" // rouz edit (ChatGPT)
#include "Ropes.h" // rouz edit (ChatGPT)
#include "SpecialFX.h" // rouz edit (ChatGPT)
#include "Fluff.h" // rouz edit (ChatGPT)
#include "WaterCannon.h" // rouz edit (ChatGPT)
#include "WaterLevel.h" // rouz edit (ChatGPT)
#include "PointLights.h" // rouz edit (ChatGPT)
#include "Particle.h" // rouz edit (ChatGPT)
#endif
//- rouz edit (ChatGPT)

#ifndef LIBRW
#error "Need librw for EXTENDED_PIPELINES"
#endif

namespace CustomPipes {

rw::int32 CustomMatOffset;

void*
CustomMatCtor(void *object, int32, int32)
{
	CustomMatExt *ext = GetCustomMatExt((rw::Material*)object);
	ext->glossTex = nil;
	ext->haveGloss = false;
	return object;
}

void*
CustomMatCopy(void *dst, void *src, int32, int32)
{
	CustomMatExt *srcext = GetCustomMatExt((rw::Material*)src);
	CustomMatExt *dstext = GetCustomMatExt((rw::Material*)dst);
	dstext->glossTex = srcext->glossTex;
	dstext->haveGloss = srcext->haveGloss;
	return dst;
}



rw::TexDictionary *neoTxd;

bool bRenderingEnvMap;
int32 EnvMapSize = 128;
rw::Camera *EnvMapCam;
rw::Texture *EnvMapTex;
rw::Texture *EnvMaskTex;
static rw::RWDEVICE::Im2DVertex EnvScreenQuad[4];
static int16 QuadIndices[6] = { 0, 1, 2, 0, 2, 3 };

#ifdef REVC_SOFTWARE_POLYGONS
//+ rouz edit (ChatGPT)
static CStoredShadow EnvMapStoredShadows[MAX_STOREDSHADOWS];
static int16 EnvMapStoredShadowCount;
static CStoredShadow EnvMapAdditionalShadows[MAX_STOREDSHADOWS];
static int16 EnvMapAdditionalShadowCount;

static bool
IsDuplicateEnvMapShadow(const CStoredShadow &shadow)
{
	// Compare the shadow geometry so main-view shadows are not added twice
	for(int16 i = 0; i < EnvMapStoredShadowCount; i++){
		CStoredShadow *stored = &EnvMapStoredShadows[i];
		if(stored->m_pTexture == shadow.m_pTexture &&
		   stored->m_ShadowType == shadow.m_ShadowType &&
		   Abs(stored->m_vecPos.x - shadow.m_vecPos.x) < 0.01f &&
		   Abs(stored->m_vecPos.y - shadow.m_vecPos.y) < 0.01f &&
		   Abs(stored->m_vecPos.z - shadow.m_vecPos.z) < 0.01f &&
		   Abs(stored->m_vecFront.x - shadow.m_vecFront.x) < 0.01f &&
		   Abs(stored->m_vecFront.y - shadow.m_vecFront.y) < 0.01f &&
		   Abs(stored->m_vecSide.x - shadow.m_vecSide.x) < 0.01f &&
		   Abs(stored->m_vecSide.y - shadow.m_vecSide.y) < 0.01f)
			return true;
	}

	// Compare earlier reflection-only shadows before adding another entry
	for(int16 i = 0; i < EnvMapAdditionalShadowCount; i++){
		CStoredShadow *stored = &EnvMapAdditionalShadows[i];
		if(stored->m_pTexture == shadow.m_pTexture &&
		   stored->m_ShadowType == shadow.m_ShadowType &&
		   Abs(stored->m_vecPos.x - shadow.m_vecPos.x) < 0.01f &&
		   Abs(stored->m_vecPos.y - shadow.m_vecPos.y) < 0.01f &&
		   Abs(stored->m_vecPos.z - shadow.m_vecPos.z) < 0.01f &&
		   Abs(stored->m_vecFront.x - shadow.m_vecFront.x) < 0.01f &&
		   Abs(stored->m_vecFront.y - shadow.m_vecFront.y) < 0.01f &&
		   Abs(stored->m_vecSide.x - shadow.m_vecSide.x) < 0.01f &&
		   Abs(stored->m_vecSide.y - shadow.m_vecSide.y) < 0.01f)
			return true;
	}

	return false;
}

void
CaptureStoredShadowsForEnvMap(void)
{
	// Copy the frame's temporary shadows before the main pass clears them
	EnvMapStoredShadowCount = CShadows::ShadowsStoredToBeRendered;
	for(int16 i = 0; i < EnvMapStoredShadowCount; i++)
		EnvMapStoredShadows[i] = CShadows::asShadowsStored[i];
	EnvMapAdditionalShadowCount = 0;
}

void
StoreVehicleShadowForEnvMap(CVehicle *vehicle, int32 shadowType)
{
	//+ rouz edit (ChatGPT)
	// Add a camera-relative temporary shadow when the reflection pass lacks a cached static one
	if(!bRenderingEnvMap || vehicle == nil || EnvMapAdditionalShadowCount >= MAX_STOREDSHADOWS || EnvMapCam == nil)
		return;
	//- rouz edit (ChatGPT)
	// Require a usable reflection camera matrix before evaluating shadow distance
	//+ rouz edit (ChatGPT)
	RwFrame *envMapFrame = EnvMapCam->getFrame();
	RwMatrix *envMapMatrix = envMapFrame ? envMapFrame->getLTM() : nil;
	if(envMapMatrix == nil)
		return;
	//- rouz edit (ChatGPT)
	// Reuse a cached static vehicle shadow when the main view already registered it
	//+ rouz edit (ChatGPT)
	const uint32 shadowId = (uint32)((uintptr)vehicle + 1);
	for(int i = 0; i < MAX_STATICSHADOWS; i++)
		if(CShadows::aStaticShadows[i].m_nId == shadowId && CShadows::aStaticShadows[i].m_pPolyBunch != nil){
			CVector staticShadowOffset = CShadows::aStaticShadows[i].m_vecPosn - vehicle->GetPosition();
			if(staticShadowOffset.MagnitudeSqr2D() < 25.0f)
				return;
		}
	//- rouz edit (ChatGPT)

	// Temporarily isolate the reflection shadow registration from the main pass queue
	int16 savedCount = CShadows::ShadowsStoredToBeRendered;
	CStoredShadow savedFirstShadow;
	if(savedCount > 0)
		savedFirstShadow = CShadows::asShadowsStored[0];
	CShadows::ShadowsStoredToBeRendered = 0;
	// Register the shadow using the reflection camera and keep it out of static main-view state
	//+ rouz edit (ChatGPT)
	CVector shadowCameraPosition(envMapMatrix->pos);
	CShadows::StoreShadowForVehicle(vehicle, (VEH_SHD_TYPE)shadowType, &shadowCameraPosition, true);
	//- rouz edit (ChatGPT)

	// Keep a new temporary shadow only when the main view did not already add it
	if(CShadows::ShadowsStoredToBeRendered > 0){
		CStoredShadow shadow = CShadows::asShadowsStored[0];
		if(!IsDuplicateEnvMapShadow(shadow))
			EnvMapAdditionalShadows[EnvMapAdditionalShadowCount++] = shadow;
	}

	// Restore the queue contents so the reflection cannot alter the main scene
	if(savedCount > 0)
		CShadows::asShadowsStored[0] = savedFirstShadow;
	CShadows::ShadowsStoredToBeRendered = savedCount;
}

//+ rouz edit (ChatGPT)
void
StoreHeliSearchLightShadowForEnvMap(CHeli *heli)
{
	// Add an active helicopter searchlight pool to the isolated reflection shadow list
	if(!bRenderingEnvMap || heli == nil || heli->m_fSearchLightIntensity <= 0.0f ||
	   EnvMapAdditionalShadowCount >= MAX_STOREDSHADOWS || EnvMapCam == nil)
		return;

	CVector shadowPosition(heli->m_fSearchLightX, heli->m_fSearchLightY, heli->GetPosition().z);
	int16 savedCount = CShadows::ShadowsStoredToBeRendered;
	CStoredShadow savedFirstShadow;
	if(savedCount > 0)
		savedFirstShadow = CShadows::asShadowsStored[0];
	CShadows::ShadowsStoredToBeRendered = 0;
	CShadows::StoreShadowToBeRendered(SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &shadowPosition,
		6.0f, 0.0f, 0.0f, -6.0f,
		80*heli->m_fSearchLightIntensity, 80*heli->m_fSearchLightIntensity,
		80*heli->m_fSearchLightIntensity, 80*heli->m_fSearchLightIntensity,
		50.0f, true, 1.0f, nil, false);

	// Keep the generated shadow only when it is not already present in the reflection set
	if(CShadows::ShadowsStoredToBeRendered > 0){
		CStoredShadow shadow = CShadows::asShadowsStored[0];
		if(!IsDuplicateEnvMapShadow(shadow))
			EnvMapAdditionalShadows[EnvMapAdditionalShadowCount++] = shadow;
	}
	if(savedCount > 0)
		CShadows::asShadowsStored[0] = savedFirstShadow;
	CShadows::ShadowsStoredToBeRendered = savedCount;
}
//- rouz edit (ChatGPT)

void
StorePedShadowForEnvMap(CEntity *ped)
{
	// Use reflection-camera distance without relying on the main camera's visibility test
	if(!bRenderingEnvMap || ped == nil || EnvMapAdditionalShadowCount >= MAX_STOREDSHADOWS ||
	   CTimeCycle::GetShadowStrength() == 0 || EnvMapCam == nil || EnvMapCam->getFrame() == nil)
		return;

	// Match the game's pedestrian shadow range and sunlight fade
	RwMatrix *cameraMatrix = EnvMapCam->getFrame()->getLTM();
	if(cameraMatrix == nil)
		return;
	CVector cameraPosition(cameraMatrix->pos);
	CVector shadowPosition = ped->GetPosition();
	float distanceSquared = (shadowPosition - cameraPosition).MagnitudeSqr2D();
	const float drawDistance = 26.0f;
	if(distanceSquared >= SQR(drawDistance * 0.5f))
		return;
	float distance = Sqrt(distanceSquared);
	float fade = 1.0f - (4.0f / drawDistance) * (distance - drawDistance * 0.25f);
	int intensity = distance >= drawDistance * 0.25f ?
		(int)(CTimeCycle::GetShadowStrength() * fade) : CTimeCycle::GetShadowStrength();
	shadowPosition.x += CTimeCycle::GetShadowDisplacementX();
	shadowPosition.y += CTimeCycle::GetShadowDisplacementY();

	// Isolate the generated shadow so it cannot change the main scene queue
	int16 savedCount = CShadows::ShadowsStoredToBeRendered;
	CStoredShadow savedFirstShadow;
	if(savedCount > 0)
		savedFirstShadow = CShadows::asShadowsStored[0];
	CShadows::ShadowsStoredToBeRendered = 0;
	CShadows::StoreShadowToBeRendered(SHADOWTYPE_DARK, gpShadowPedTex, &shadowPosition,
		CTimeCycle::GetShadowFrontX(), CTimeCycle::GetShadowFrontY(),
		CTimeCycle::GetShadowSideX(), CTimeCycle::GetShadowSideY(),
		intensity, intensity, intensity, intensity, 4.0f, false, 1.0f, nil, false);

	// Keep the new shadow only when it does not duplicate an existing reflection entry
	if(CShadows::ShadowsStoredToBeRendered > 0){
		CStoredShadow shadow = CShadows::asShadowsStored[0];
		if(!IsDuplicateEnvMapShadow(shadow))
			EnvMapAdditionalShadows[EnvMapAdditionalShadowCount++] = shadow;
	}

	// Restore the main queue after capturing the reflection shadow
	if(savedCount > 0)
		CShadows::asShadowsStored[0] = savedFirstShadow;
	CShadows::ShadowsStoredToBeRendered = savedCount;
}

//+ rouz edit (ChatGPT)
void
StoreBeachBallShadowForEnvMap(CEntity *beachBall)
{
	// Add the beachball's normal compact ground shadow to the reflection queue
	if(!bRenderingEnvMap || beachBall == nil || EnvMapAdditionalShadowCount >= MAX_STOREDSHADOWS ||
	   CTimeCycle::GetShadowStrength() == 0 || EnvMapCam == nil)
		return;
	CVector shadowPosition = beachBall->GetPosition();
	// Isolate registration so this reflection effect does not consume the main queue
	int16 savedCount = CShadows::ShadowsStoredToBeRendered;
	CStoredShadow savedFirstShadow;
	if(savedCount > 0)
		savedFirstShadow = CShadows::asShadowsStored[0];
	CShadows::ShadowsStoredToBeRendered = 0;
	CShadows::StoreShadowToBeRendered(SHADOWTYPE_DARK, gpShadowPedTex, &shadowPosition,
		0.4f, 0.0f, 0.0f, 0.4f,
		CTimeCycle::GetShadowStrength(), CTimeCycle::GetShadowStrength(),
		CTimeCycle::GetShadowStrength(), CTimeCycle::GetShadowStrength(),
		20.0f, false, 1.0f, nil, false);
	// Keep the shadow only when the reflection queue does not already contain it
	if(CShadows::ShadowsStoredToBeRendered > 0){
		CStoredShadow shadow = CShadows::asShadowsStored[0];
		if(!IsDuplicateEnvMapShadow(shadow))
			EnvMapAdditionalShadows[EnvMapAdditionalShadowCount++] = shadow;
	}
	// Restore the pending main-view shadow after capturing the reflection entry
	if(savedCount > 0)
		CShadows::asShadowsStored[0] = savedFirstShadow;
	CShadows::ShadowsStoredToBeRendered = savedCount;
}
//- rouz edit (ChatGPT)

static void
RenderStoredShadowsForEnvMap(void)
{
	// Skip the reflection pass when neither scene produced temporary shadows
	if(EnvMapStoredShadowCount <= 0 && EnvMapAdditionalShadowCount <= 0)
		return;

	// Preserve any live queue while swapping in the cached reflection shadows
	int16 savedCount = CShadows::ShadowsStoredToBeRendered;
	CStoredShadow savedShadows[MAX_STOREDSHADOWS];
	for(int16 i = 0; i < savedCount; i++)
		savedShadows[i] = CShadows::asShadowsStored[i];
	for(int16 i = 0; i < EnvMapStoredShadowCount; i++)
		CShadows::asShadowsStored[i] = EnvMapStoredShadows[i];
	int16 reflectionShadowCount = EnvMapStoredShadowCount;
	for(int16 i = 0; i < EnvMapAdditionalShadowCount && reflectionShadowCount < MAX_STOREDSHADOWS; i++)
		CShadows::asShadowsStored[reflectionShadowCount++] = EnvMapAdditionalShadows[i];
	CShadows::ShadowsStoredToBeRendered = reflectionShadowCount;

	// Render the cached shadows through the existing software-compatible path
	CShadows::RenderStoredShadows();

	// Restore the main pass queue and discard this frame's reflection snapshot
	for(int16 i = 0; i < savedCount; i++)
		CShadows::asShadowsStored[i] = savedShadows[i];
	CShadows::ShadowsStoredToBeRendered = savedCount;
	EnvMapStoredShadowCount = 0;
	EnvMapAdditionalShadowCount = 0;
}
//- rouz edit (ChatGPT)
#endif

static rw::Camera*
CreateEnvMapCam(rw::World *world)
{
	rw::Raster *fbuf = rw::Raster::create(EnvMapSize, EnvMapSize, 0, rw::Raster::CAMERATEXTURE);
	if(fbuf){
		rw::Raster *zbuf = rw::Raster::create(EnvMapSize, EnvMapSize, 0, rw::Raster::ZBUFFER);
		if(zbuf){
			rw::Frame *frame = rw::Frame::create();
			if(frame){
				rw::Camera *cam = rw::Camera::create();
				if(cam){
					cam->frameBuffer = fbuf;
					cam->zBuffer = zbuf;
					cam->setFrame(frame);
					cam->setNearPlane(0.1f);
					cam->setFarPlane(250.0f);
					rw::V2d vw = { 2.0f, 2.0f };
					cam->setViewWindow(&vw);
					world->addCamera(cam);
					EnvMapTex = rw::Texture::create(fbuf);
					EnvMapTex->setFilter(rw::Texture::LINEAR);

					frame->matrix.right.x = -1.0f;
					frame->matrix.up.y = -1.0f;
					frame->matrix.update();
					return cam;
				}
				frame->destroy();
			}
			zbuf->destroy();
		}
		fbuf->destroy();
	}
	return nil;
}

static void
DestroyCam(rw::Camera *cam)
{
	if(cam == nil)
		return;
	if(cam->frameBuffer){
		cam->frameBuffer->destroy();
		cam->frameBuffer = nil;
	}
	if(cam->zBuffer){
		cam->zBuffer->destroy();
		cam->zBuffer = nil;
	}
	rw::Frame *f = cam->getFrame();
	if(f){
		cam->setFrame(nil);
		f->destroy();
	}
	cam->world->removeCamera(cam);
	cam->destroy();
}

void
RenderEnvMapScene(void)
{
	//+ rouz edit (ChatGPT)
	// Register reflection-only entity lights before drawing the reflected scene
#ifdef REVC_SOFTWARE_POLYGONS
	CCoronas::ResetEnvMapCoronas();
	CRenderer::RegisterSoftwareReflectionLights();
#endif
	//- rouz edit (ChatGPT)
	// Fill the reflection target with distant water before world geometry
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	CWaterLevel::RenderWater();
	// Render the reflection camera's sky layers before opaque world geometry
	//+ rouz edit (ChatGPT)
	CClouds::RenderForEnvMap(EnvMapCam);
	DefinedState();
	//- rouz edit (ChatGPT)
#endif
	//- rouz edit (ChatGPT)
	// Include the opaque and transparent building passes in vehicle reflections
	//+ rouz edit (ChatGPT)
	CRenderer::RenderWorld(0);
	CRenderer::RenderWorld(1);
	//- rouz edit (ChatGPT)
	CRenderer::RenderRoads();
	// Render boats before cutting their hull silhouettes into the transparent water pass
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	CRenderer::RenderBoats();
	// Keep fading submerged entities beneath the transparent reflection-water pass
	CRenderer::RenderFadingInUnderwaterEntities();
	CRenderer::RenderTransparentWater();
#endif
	//- rouz edit (ChatGPT)
	CRenderer::RenderEverythingBarRoads();
	//+ rouz edit (ChatGPT)
	// Sort reflection-camera actors and props together for alpha blending
#ifdef REVC_SOFTWARE_POLYGONS
	CRenderer::RenderSoftwareReflectionEntities();
	DefinedState();
#endif
	//- rouz edit (ChatGPT)
	// Render transparent buildings after the opaque environment and dynamic props
	//+ rouz edit (ChatGPT)
	CRenderer::RenderWorld(2);
	//- rouz edit (ChatGPT)
	CRenderer::RenderFadingInEntities();
	// Render reflection-safe world effects without advancing their shared simulations
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	CShadows::RenderStaticShadows();
	// Add this frame's temporary shadows to the vehicle reflection
	//+ rouz edit (ChatGPT)
	RenderStoredShadowsForEnvMap();
	//- rouz edit (ChatGPT)
	CSkidmarks::Render();
	CRubbish::Render();
	// Draw persistent shattered glass panes into vehicle reflections
	//+ rouz edit (ChatGPT)
	CGlass::Render();
	DefinedState();
	//- rouz edit (ChatGPT)
	// Draw sun glints, rain streaks, and fire-fighting spray in reflections
	CCoronas::RenderSunReflection();
	// Draw nearby corona lights using the auxiliary camera's world orientation
	//+ rouz edit (ChatGPT)
	CCoronas::RenderForEnvMap(EnvMapCam);
	//+ rouz edit (ChatGPT)
	// Reflect wet-road light streaks into the software environment image
	CCoronas::RenderReflectionsForEnvMap(EnvMapCam);
	//- rouz edit (ChatGPT)
	DefinedState();
	//- rouz edit (ChatGPT)
	CWeather::RenderRainStreaks();
	CWaterCannons::Render();
	DefinedState();
	// Reflect traffic signals and vehicle lamps using the main pass's flicker values
	//+ rouz edit (ChatGPT)
	CBrightLights::Render(true);
	DefinedState();
	//- rouz edit (ChatGPT)
	// Replay registered shiny text geometry in the vehicle reflection
	//+ rouz edit (ChatGPT)
	CShinyTexts::RenderForEnvMap();
	DefinedState();
	//- rouz edit (ChatGPT)
	// Replay active world markers without mutating their registration state
	//+ rouz edit (ChatGPT)
	C3dMarkers::RenderForEnvMap(EnvMapCam);
	DefinedState();
	//- rouz edit (ChatGPT)
	CAntennas::Render();
	// Add antennas from RC Bandits that were visible only to the reflection camera
	//+ rouz edit (ChatGPT)
	CAntennas::RenderForEnvMap();
	//- rouz edit (ChatGPT)
	CRopes::Render();
	CMotionBlurStreaks::Render();
	CBulletTraces::Render();
	CPlaneTrails::Render();
	CSmokeTrails::Render();
	CPlaneBanners::Render();
	// Reflect the animated stadium message board from its current pixel state
	//+ rouz edit (ChatGPT)
	CMovingThings::RenderForEnvMap(EnvMapCam);
	DefinedState();
	//- rouz edit (ChatGPT)
	// Add camera-corrected point-light fog to the software reflection
	//+ rouz edit (ChatGPT)
	CPointLights::RenderFogEffectForEnvMap(EnvMapCam);
	DefinedState();
	//- rouz edit (ChatGPT)
	// Draw world-space smoke, sparks, and debris particles in the software reflection
	//+ rouz edit (ChatGPT)
	CParticle::RenderForEnvMap(EnvMapCam);
	DefinedState();
	//- rouz edit (ChatGPT)
#endif
	//- rouz edit (ChatGPT)
}

void
EnvMapRender(void)
{
	if(VehiclePipeSwitch != VEHICLEPIPE_NEO)
		return;
	// Cache the previous environment image before the camera starts writing this frame
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	SoftwarePolygons::PrimeTexture(EnvMapTex);
#endif
	//- rouz edit (ChatGPT)

	RwCameraEndUpdate(Scene.camera);

	// Neo does this differently, but i'm not quite convinced it's much better
	rw::V3d camPos = FindPlayerCoors();
	EnvMapCam->getFrame()->matrix.pos = camPos;
	EnvMapCam->getFrame()->transform(&EnvMapCam->getFrame()->matrix, rw::COMBINEREPLACE);

	rw::RGBA skycol;
	skycol.red = CTimeCycle::GetSkyBottomRed();
	skycol.green = CTimeCycle::GetSkyBottomGreen();
	skycol.blue = CTimeCycle::GetSkyBottomBlue();
	skycol.alpha = 255;
	// Keep a timecycle gradient behind geometry in the software environment map
	//+ rouz edit (ChatGPT)
	rw::RGBA skyTop = {
		(rw::uint8)CTimeCycle::GetSkyTopRed(),
		(rw::uint8)CTimeCycle::GetSkyTopGreen(),
		(rw::uint8)CTimeCycle::GetSkyTopBlue(), 255
	};
	// Match the main software framebuffer during visible lightning flashes
	//+ rouz edit (ChatGPT)
	if(CWeather::LightningFlash && !CCullZones::CamNoRain()){
		skyTop.red = skyTop.green = skyTop.blue = 255;
		skycol.red = skycol.green = skycol.blue = 255;
	}
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
	EnvMapCam->clear(&skycol, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);
	RwCameraBeginUpdate(EnvMapCam);
	// Switch software rasterization to a private buffer for the auxiliary camera
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	if(!SoftwarePolygons::BeginOffscreenFrame(EnvMapCam, skyTop, skycol)){ // rouz edit (ChatGPT)
		RwCameraEndUpdate(EnvMapCam);
		RwCameraBeginUpdate(Scene.camera);
		return;
	}
#endif
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Clear auxiliary point-light registrations before gathering this reflection scene
#ifdef REVC_SOFTWARE_POLYGONS
	CPointLights::ResetEnvMapLights();
#endif
	//- rouz edit (ChatGPT)
	bRenderingEnvMap = true;
	RenderEnvMapScene();
	bRenderingEnvMap = false;

	if(EnvMaskTex){
		// Apply the same reflection mask directly to the offscreen CPU pixels
		//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
		SoftwarePolygons::ApplyTextureMask(EnvMaskTex);
#else
		rw::SetRenderState(rw::VERTEXALPHA, TRUE);
		rw::SetRenderState(rw::SRCBLEND, rw::BLENDZERO);
		rw::SetRenderState(rw::DESTBLEND, rw::BLENDSRCCOLOR);
		rw::SetRenderStatePtr(rw::TEXTURERASTER, EnvMaskTex->raster);
		rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST, EnvScreenQuad, 4, QuadIndices, 6);
		rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
		rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
#endif
		//- rouz edit (ChatGPT)
	}
	RwCameraEndUpdate(EnvMapCam);

	// Upload the software environment image and resume the main camera
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	SoftwarePolygons::EndOffscreenFrame(EnvMapCam->frameBuffer);
	RwCameraBeginUpdate(Scene.camera);
	return;
#endif
	//- rouz edit (ChatGPT)
	RwCameraBeginUpdate(Scene.camera);

	// debug env map
//	rw::SetRenderStatePtr(rw::TEXTURERASTER, EnvMapTex->raster);
//	rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST, EnvScreenQuad, 4, QuadIndices, 6);
}

static void
EnvMapInit(void)
{
	if(neoTxd)
		EnvMaskTex = neoTxd->find("CarReflectionMask");

	EnvMapCam = CreateEnvMapCam(Scene.world);

	int width = EnvMapCam->frameBuffer->width;
	int height = EnvMapCam->frameBuffer->height;
	float screenZ = RwIm2DGetNearScreenZ();
	float recipZ = 1.0f/EnvMapCam->nearPlane;

	EnvScreenQuad[0].setScreenX(0.0f);
	EnvScreenQuad[0].setScreenY(0.0f);
	EnvScreenQuad[0].setScreenZ(screenZ);
	EnvScreenQuad[0].setCameraZ(EnvMapCam->nearPlane);
	EnvScreenQuad[0].setRecipCameraZ(recipZ);
	EnvScreenQuad[0].setColor(255, 255, 255, 255);
	EnvScreenQuad[0].setU(0.0f, recipZ);
	EnvScreenQuad[0].setV(0.0f, recipZ);

	EnvScreenQuad[1].setScreenX(0.0f);
	EnvScreenQuad[1].setScreenY(height);
	EnvScreenQuad[1].setScreenZ(screenZ);
	EnvScreenQuad[1].setCameraZ(EnvMapCam->nearPlane);
	EnvScreenQuad[1].setRecipCameraZ(recipZ);
	EnvScreenQuad[1].setColor(255, 255, 255, 255);
	EnvScreenQuad[1].setU(0.0f, recipZ);
	EnvScreenQuad[1].setV(1.0f, recipZ);

	EnvScreenQuad[2].setScreenX(width);
	EnvScreenQuad[2].setScreenY(height);
	EnvScreenQuad[2].setScreenZ(screenZ);
	EnvScreenQuad[2].setCameraZ(EnvMapCam->nearPlane);
	EnvScreenQuad[2].setRecipCameraZ(recipZ);
	EnvScreenQuad[2].setColor(255, 255, 255, 255);
	EnvScreenQuad[2].setU(1.0f, recipZ);
	EnvScreenQuad[2].setV(1.0f, recipZ);

	EnvScreenQuad[3].setScreenX(width);
	EnvScreenQuad[3].setScreenY(0.0f);
	EnvScreenQuad[3].setScreenZ(screenZ);
	EnvScreenQuad[3].setCameraZ(EnvMapCam->nearPlane);
	EnvScreenQuad[3].setRecipCameraZ(recipZ);
	EnvScreenQuad[3].setColor(255, 255, 255, 255);
	EnvScreenQuad[3].setU(1.0f, recipZ);
	EnvScreenQuad[3].setV(0.0f, recipZ);
}

static void
EnvMapShutdown(void)
{
	EnvMapTex->raster = nil;
	EnvMapTex->destroy();
	EnvMapTex = nil;
	DestroyCam(EnvMapCam);
	EnvMapCam = nil;
}

/*
 * Tweak values
 */

#define INTERP_SETUP \
		int h1 = CClock::GetHours();								  \
		int h2 = (h1+1)%24;										  \
		int w1 = CWeather::OldWeatherType;								  \
		int w2 = CWeather::NewWeatherType;								  \
		float timeInterp = (CClock::GetSeconds()/60.0f + CClock::GetMinutes())/60.0f;	  \
		float c0 = (1.0f-timeInterp)*(1.0f-CWeather::InterpolationValue);				  \
		float c1 = timeInterp*(1.0f-CWeather::InterpolationValue);					  \
		float c2 = (1.0f-timeInterp)*CWeather::InterpolationValue;					  \
		float c3 = timeInterp*CWeather::InterpolationValue;
#define INTERP(v) v[h1][w1]*c0 + v[h2][w1]*c1 + v[h1][w2]*c2 + v[h2][w2]*c3;
#define INTERPF(v,f) v[h1][w1].f*c0 + v[h2][w1].f*c1 + v[h1][w2].f*c2 + v[h2][w2].f*c3;

InterpolatedFloat::InterpolatedFloat(float init)
{
	curInterpolator = 61;	// compared against second
	for(int h = 0; h < 24; h++)
		for(int w = 0; w < NUMWEATHERS; w++)
			data[h][w] = init;
}

void
InterpolatedFloat::Read(char *s, int line, int field)
{
	sscanf(s, "%f", &data[line][field]);
}

float
InterpolatedFloat::Get(void)
{
	if(curInterpolator != CClock::GetSeconds()){
		INTERP_SETUP
		curVal = INTERP(data);
		curInterpolator = CClock::GetSeconds();
	}
	return curVal;
}

InterpolatedColor::InterpolatedColor(const Color &init)
{
	curInterpolator = 61;	// compared against second
	for(int h = 0; h < 24; h++)
		for(int w = 0; w < NUMWEATHERS; w++)
			data[h][w] = init;
}

void
InterpolatedColor::Read(char *s, int line, int field)
{
	int r, g, b, a;
	sscanf(s, "%i, %i, %i, %i", &r, &g, &b, &a);
	data[line][field] = Color(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
}

Color
InterpolatedColor::Get(void)
{
	if(curInterpolator != CClock::GetSeconds()){
		INTERP_SETUP
		curVal.r = INTERPF(data, r);
		curVal.g = INTERPF(data, g);
		curVal.b = INTERPF(data, b);
		curVal.a = INTERPF(data, a);
		curInterpolator = CClock::GetSeconds();
	}
	return curVal;
}

void
InterpolatedLight::Read(char *s, int line, int field)
{
	int r, g, b, a;
	sscanf(s, "%i, %i, %i, %i", &r, &g, &b, &a);
	data[line][field] = Color(r/255.0f, g/255.0f, b/255.0f, a/100.0f);
}

char*
ReadTweakValueTable(char *fp, InterpolatedValue &interp)
{
	char buf[24], *p;
	int c;
	int line, field;

	line = 0;
	c = *fp++;
	while(c != '\0' && line < 24){
		field = 0;
		if(c != '\0' && c != '#'){
			while(c != '\0' && c != '\n' && field < NUMWEATHERS){
				p = buf;
				while(c != '\0' && c == '\t')
					c = *fp++;
				*p++ = c;
				while(c = *fp++, c != '\0' && c != '\t' && c != '\n')
					*p++ = c;
				*p++ = '\0';
				interp.Read(buf, line, field);
				field++;
			}
			line++;
		}
		while(c != '\0' && c != '\n')
			c = *fp++;
		c = *fp++;
	}
	return fp-1;
}



/*
 * Neo Vehicle pipe
 */

int32 VehiclePipeSwitch = VEHICLEPIPE_MATFX;
float VehicleShininess = 1.0f;
float VehicleSpecularity = 1.0f;
InterpolatedFloat Fresnel(0.4f);
InterpolatedFloat Power(18.0f);
InterpolatedLight DiffColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
InterpolatedLight SpecColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
rw::ObjPipeline *vehiclePipe;

void
AttachVehiclePipe(rw::Atomic *atomic)
{
	atomic->pipeline = vehiclePipe;
}

void
AttachVehiclePipe(rw::Clump *clump)
{
	FORLIST(lnk, clump->atomics)
		AttachVehiclePipe(rw::Atomic::fromClump(lnk));
}



/*
 * Neo World pipe
 */

bool LightmapEnable;
float LightmapMult = 1.0f;
InterpolatedFloat WorldLightmapBlend(1.0f);
rw::ObjPipeline *worldPipe;

void
AttachWorldPipe(rw::Atomic *atomic)
{
	atomic->pipeline = worldPipe;
}

void
AttachWorldPipe(rw::Clump *clump)
{
	FORLIST(lnk, clump->atomics)
		AttachWorldPipe(rw::Atomic::fromClump(lnk));
}




/*
 * Neo Gloss pipe
 */

bool GlossEnable;
float GlossMult = 1.0f;
rw::ObjPipeline *glossPipe;

rw::Texture*
GetGlossTex(rw::Material *mat)
{
	if(neoTxd == nil)
		return nil;
	CustomMatExt *ext = GetCustomMatExt(mat);
	if(!ext->haveGloss){
		char glossname[128];
		strcpy(glossname, mat->texture->name);
		strcat(glossname, "_gloss");
		ext->glossTex = neoTxd->find(glossname);
		ext->haveGloss = true;
	}
	return ext->glossTex;
}

void
AttachGlossPipe(rw::Atomic *atomic)
{
	atomic->pipeline = glossPipe;
}

void
AttachGlossPipe(rw::Clump *clump)
{
	FORLIST(lnk, clump->atomics)
		AttachWorldPipe(rw::Atomic::fromClump(lnk));
}



/*
 * Neo Rim pipes
 */

bool RimlightEnable;
float RimlightMult = 1.0f;
InterpolatedColor RampStart(Color(0.0f, 0.0f, 0.0f, 1.0f));
InterpolatedColor RampEnd(Color(1.0f, 1.0f, 1.0f, 1.0f));
InterpolatedFloat Offset(0.5f);
InterpolatedFloat Scale(1.5f);
InterpolatedFloat Scaling(2.0f);
rw::ObjPipeline *rimPipe;
rw::ObjPipeline *rimSkinPipe;

void
AttachRimPipe(rw::Atomic *atomic)
{
	if(rw::Skin::get(atomic->geometry))
		atomic->pipeline = rimSkinPipe;
	else
		atomic->pipeline = rimPipe;
}

void
AttachRimPipe(rw::Clump *clump)
{
	FORLIST(lnk, clump->atomics)
		AttachRimPipe(rw::Atomic::fromClump(lnk));
}

/*
 * High level stuff
 */

void
CustomPipeInit(void)
{
	RwStream *stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, "neo/neo.txd");
	if(stream == nil)
		printf("Error: couldn't open 'neo/neo.txd'\n");
	else{
		if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nil, nil))
			neoTxd = RwTexDictionaryGtaStreamRead(stream);
		RwStreamClose(stream, nil);
	}

	EnvMapInit();

	CreateVehiclePipe();
	CreateWorldPipe();
	CreateGlossPipe();
	CreateRimLightPipes();
}

void
CustomPipeShutdown(void)
{
	DestroyVehiclePipe();
	DestroyWorldPipe();
	DestroyGlossPipe();
	DestroyRimLightPipes();

	EnvMapShutdown();

	if(neoTxd){
		neoTxd->destroy();
		neoTxd = nil;
	}
}

void
CustomPipeRegister(void)
{
#ifdef RW_OPENGL
	CustomPipeRegisterGL();
#endif

	CustomMatOffset = rw::Material::registerPlugin(sizeof(CustomMatExt), MAKECHUNKID(rwVENDORID_ROCKSTAR, 0x80),
		CustomMatCtor, nil, CustomMatCopy);
}


// Load textures from generic as fallback

rw::TexDictionary *genericTxd;
rw::Texture *(*defaultFindCB)(const char *name);

static rw::Texture*
customFindCB(const char *name)
{
	rw::Texture *res = defaultFindCB(name);
	if(res == nil)
		res = genericTxd->find(name);
	return res;
}

void
SetTxdFindCallback(void)
{
	int slot = CTxdStore::FindTxdSlot("generic");
	CTxdStore::AddRef(slot);
	// TODO: function for this
	genericTxd = CTxdStore::GetSlot(slot)->texDict;
	assert(genericTxd);
	if(defaultFindCB == nil)
		defaultFindCB = rw::Texture::findCB;
	rw::Texture::findCB = customFindCB;
}

}

#endif
