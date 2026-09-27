#define WITHD3D
#include "common.h"

#include "main.h"
#include "Lights.h"
#include "ModelInfo.h"
#include "Treadable.h"
#include "Ped.h"
#include "Pools.h" // rouz edit (ChatGPT)
#include "RwHelper.h" // rouz edit (ChatGPT)
#include "Vehicle.h"
#include "Automobile.h" // rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
#include "PlayerPed.h"
#include "Plane.h" // rouz edit (ChatGPT)
#include "Antennas.h" // rouz edit (ChatGPT)
#include "SpecialFX.h" // rouz edit (ChatGPT)
#endif
//- rouz edit (ChatGPT)
#include "Boat.h"
#include "Heli.h"
#include "Bike.h"
#include "Object.h"
#include "PathFind.h"
#include "Collision.h"
#include "VisibilityPlugins.h"
#include "Clock.h"
#include "World.h"
#include "Camera.h"
#include "ModelIndices.h"
#include "Streaming.h"
#include "Shadows.h"
#include "PointLights.h"
#include "Coronas.h" // rouz edit (ChatGPT)
#include "Occlusion.h"
#include "Renderer.h"
#include "custompipes.h"
#include "Frontend.h"
#include "CutsceneMgr.h" // rouz edit (ChatGPT)
#include "TimeStep.h" // rouz edit (ChatGPT)
#include "Timecycle.h" // rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
#include "SoftwarePolygons.h"
#endif
//- rouz edit (ChatGPT)

bool gbShowPedRoadGroups;
bool gbShowCarRoadGroups;
bool gbShowCollisionPolys;
bool gbShowCollisionPolysReflections;
bool gbShowCollisionPolysNoShadows;
bool gbShowCollisionLines;
bool gbBigWhiteDebugLightSwitchedOn;

bool gbDontRenderBuildings;
bool gbDontRenderBigBuildings;
bool gbDontRenderPeds;
bool gbDontRenderObjects;
bool gbDontRenderVehicles;

// unused
int16 TestCloseThings;
int16 TestBigThings;

struct EntityInfo
{
	CEntity *ent;
	float sort;
};

CLinkList<EntityInfo> gSortedVehiclesAndPeds;

int32 CRenderer::ms_nNoOfVisibleEntities;
CEntity *CRenderer::ms_aVisibleEntityPtrs[NUMVISIBLEENTITIES];
CEntity *CRenderer::ms_aInVisibleEntityPtrs[NUMINVISIBLEENTITIES];
int32 CRenderer::ms_nNoOfInVisibleEntities;
#ifdef NEW_RENDERER
int32 CRenderer::ms_nNoOfVisibleVehicles;
CEntity *CRenderer::ms_aVisibleVehiclePtrs[NUMVISIBLEENTITIES];
int32 CRenderer::ms_nNoOfVisibleBuildings;
CEntity *CRenderer::ms_aVisibleBuildingPtrs[NUMVISIBLEENTITIES];
#endif

CVector CRenderer::ms_vecCameraPosition;
CVehicle *CRenderer::m_pFirstPersonVehicle;
bool CRenderer::m_loadingPriority;
float CRenderer::ms_lodDistScale = 1.2f;

// unused
BlockedRange CRenderer::aBlockedRanges[16];
BlockedRange* CRenderer::pFullBlockedRanges;
BlockedRange* CRenderer::pEmptyBlockedRanges;

void
CRenderer::Init(void)
{
	gSortedVehiclesAndPeds.Init(40);
	SortBIGBuildings();
}

void
CRenderer::Shutdown(void)
{
	gSortedVehiclesAndPeds.Shutdown();
}

void
CRenderer::PreRender(void)
{
	int i;
	CLink<CVisibilityPlugins::AlphaObjectInfo> *node;

	for(i = 0; i < ms_nNoOfVisibleEntities; i++)
		ms_aVisibleEntityPtrs[i]->PreRender();

#ifdef NEW_RENDERER
	if(gbNewRenderer){
		for(i = 0; i < ms_nNoOfVisibleVehicles; i++)
			ms_aVisibleVehiclePtrs[i]->PreRender();
		// How is this done with cWorldStream?
		for(i = 0; i < ms_nNoOfVisibleBuildings; i++)
			ms_aVisibleBuildingPtrs[i]->PreRender();
		for(node = CVisibilityPlugins::m_alphaBuildingList.head.next;
		    node != &CVisibilityPlugins::m_alphaBuildingList.tail;
		    node = node->next)
			((CEntity*)node->item.entity)->PreRender();
	}
#endif

	for (i = 0; i < ms_nNoOfInVisibleEntities; i++) {
#ifdef SQUEEZE_PERFORMANCE
		if (ms_aInVisibleEntityPtrs[i]->IsVehicle() && ((CVehicle*)ms_aInVisibleEntityPtrs[i])->IsHeli())
#endif
		ms_aInVisibleEntityPtrs[i]->PreRender();
	}

	for(node = CVisibilityPlugins::m_alphaEntityList.head.next;
	    node != &CVisibilityPlugins::m_alphaEntityList.tail;
	    node = node->next)
		((CEntity*)node->item.entity)->PreRender();

	CHeli::SpecialHeliPreRender();
	CShadows::RenderExtraPlayerShadows();
}

void
CRenderer::RenderOneRoad(CEntity *e)
{
#ifndef FINAL
	if(gbDontRenderBuildings)
		return;
#endif
#ifndef MASTER
	if(gbShowCollisionPolys || gbShowCollisionPolysReflections || gbShowCollisionPolysNoShadows)
		CCollision::DrawColModel_Coloured(e->GetMatrix(), *CModelInfo::GetColModel(e->GetModelIndex()), e->GetModelIndex());
	else
#endif
	{
		PUSH_RENDERGROUP(CModelInfo::GetModelInfo(e->GetModelIndex())->GetModelName());

		e->Render();

		POP_RENDERGROUP();
	}
}

//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
static void RenderSoftwarePed(CPed *ped);
//+ rouz edit (ChatGPT)
static void RenderSoftwareVehicle(CVehicle *vehicle); // rouz edit (ChatGPT)
static bool IsSoftwareVehicleOccupant(CPed *ped);
static void RenderSoftwareVehicleOccupant(CPed *ped);
//- rouz edit (ChatGPT)
#endif
//- rouz edit (ChatGPT)

void
CRenderer::RenderOneNonRoad(CEntity *e)
{
	CPed *ped;
	CVehicle *veh;
	int i;
	bool resetLights;

#ifndef MASTER
	if(gbShowCollisionPolys || gbShowCollisionPolysReflections || gbShowCollisionPolysNoShadows){
		if(!e->IsVehicle()){
			CCollision::DrawColModel_Coloured(e->GetMatrix(), *CModelInfo::GetColModel(e->GetModelIndex()), e->GetModelIndex());
			return;
		}
	}else
#endif
#ifndef FINAL
	if(e->IsBuilding()){
		if(e->bIsBIGBuilding){
			if(gbDontRenderBigBuildings)
				return;
		}else{
			if(gbDontRenderBuildings)
				return;
		}
	}else
#endif
	if(e->IsPed()){
		// Keep pedestrians separate from transparent world geometry
		//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
		if(!SoftwarePolygons::renderPedsStage.load(std::memory_order_relaxed))
			return;
#endif
		//- rouz edit (ChatGPT)
#ifndef FINAL
		if(gbDontRenderPeds)
			return;
#endif
		ped = (CPed*)e;
//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
		// Keep seated passengers in the vehicle pass instead of drawing them twice
		if(ped->m_nPedState == PED_DRIVING || IsSoftwareVehicleOccupant(ped)) // rouz edit (ChatGPT)
#else
		if(ped->m_nPedState == PED_DRIVING)
#endif
//- rouz edit (ChatGPT)
			return;
	}
#ifndef FINAL
	else if(e->IsObject() || e->IsDummy()){
		if(gbDontRenderObjects)
			return;
	}else if(e->IsVehicle()){
		// re3 addition
		if(gbDontRenderVehicles)
			return;
	}
#endif

#ifdef REVC_SOFTWARE_POLYGONS
	//+ rouz edit (ChatGPT)
	// Route alpha-sorted and first-person vehicle draws through the CPU renderer
	if(e->IsVehicle()){
		// Keep vehicles separate from the transparent sorting option
		//+ rouz edit (ChatGPT)
		if(!SoftwarePolygons::renderVehiclesStage.load(std::memory_order_relaxed))
			return;
		//- rouz edit (ChatGPT)
		RenderSoftwareVehicle((CVehicle*)e);
		return;
	}
	//- rouz edit (ChatGPT)
#endif

	PUSH_RENDERGROUP(CModelInfo::GetModelInfo(e->GetModelIndex())->GetModelName());

	resetLights = e->SetupLighting();

	if(e->IsVehicle()){
		// unfortunately can't use GetClump here
		CVisibilityPlugins::SetupVehicleVariables((RpClump*)e->m_rwObject);
		CVisibilityPlugins::InitAlphaAtomicList();
	}

	// Render Peds in vehicle before vehicle itself
	if(e->IsVehicle()){
		veh = (CVehicle*)e;
//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
		// Use the software occupant path for every ped attached to a rendered seat
		RenderSoftwareVehicleOccupant(veh->pDriver);
		for(i = 0; i < 8; i++)
			RenderSoftwareVehicleOccupant(veh->pPassengers[i]);
#else
		if(veh->pDriver && veh->pDriver->m_nPedState == PED_DRIVING)
			veh->pDriver->Render();
		for(i = 0; i < 8; i++)
			if(veh->pPassengers[i] && veh->pPassengers[i]->m_nPedState == PED_DRIVING)
				veh->pPassengers[i]->Render();
#endif
//- rouz edit (ChatGPT)
		SetCullMode(rwCULLMODECULLNONE);
	}
	// Route peds through the wrapper so reflection rendering does not advance render-only animation
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	if(e->IsPed())
		RenderSoftwarePed((CPed*)e);
	else
#endif
		e->Render();
	//- rouz edit (ChatGPT)

	if(e->IsVehicle()){
		e->bImBeingRendered = true;
		CVisibilityPlugins::RenderAlphaAtomics();
		e->bImBeingRendered = false;
		SetCullMode(rwCULLMODECULLBACK);
	}

	e->RemoveLighting(resetLights);

	POP_RENDERGROUP();
}

void
CRenderer::RenderFirstPersonVehicle(void)
{
	if(m_pFirstPersonVehicle == nil)
		return;
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RenderOneNonRoad(m_pFirstPersonVehicle);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
}

inline bool IsRoad(CEntity *e) { return e->IsBuilding() && ((CSimpleModelInfo*)CModelInfo::GetModelInfo(e->GetModelIndex()))->m_wetRoadReflection; }

//+ rouz edit (ChatGPT)
#define HIGH_ALTITUDE_STATIC_MAP_RENDER_Z 40.0f

struct HighAltitudeStaticMapInstance
{
	CSimpleModelInfo *modelInfo;
	CVector position;
};

static bool gbUseHighAltitudeStaticMapDistance;
static int32 gNumHighAltitudeStaticMapInstances;
static HighAltitudeStaticMapInstance gaHighAltitudeStaticMapInstances[NUMVISIBLEENTITIES];

static bool
ShouldScanFarStaticMapEntities(void)
{
	return TheCamera.GetPosition().z > HIGH_ALTITUDE_STATIC_MAP_RENDER_Z;
}

static bool
IsLowStaticMapEntity(CEntity *e)
{
	if(!(e->IsBuilding() || e->IsObject() || e->IsDummy()) || e->bIsBIGBuilding)
		return false;

	CColModel *col = CModelInfo::GetColModel(e->GetModelIndex());
	return col && e->GetPosition().z + col->boundingBox.max.z <= HIGH_ALTITUDE_STATIC_MAP_RENDER_Z;
}

static bool
IsHighAltitudeStaticMapEntity(CEntity *e)
{
	if(!(e->IsBuilding() || e->IsObject() || e->IsDummy()) || e->bIsBIGBuilding)
		return false;

	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(e->GetModelIndex());
	if(mi->m_drawLast || mi->m_additive || mi->m_noZwrite || e->bDrawLast)
		return false;

	return e->IsObject() ? IsLowStaticMapEntity(e) : true;
}

static bool
ShouldForceHighAltitudeStaticMapEntity(CEntity *e)
{
	return IsHighAltitudeStaticMapEntity(e);
}

static float
GetHighAltitudeLodDistance(CEntity *e, CSimpleModelInfo *mi, float dist)
{
	if(gbUseHighAltitudeStaticMapDistance && ShouldForceHighAltitudeStaticMapEntity(e)){
		float largestLodDist = mi->GetLargestLodDistance();
		if(dist > largestLodDist)
			return Max(0.0f, largestLodDist - 0.1f);
	}
	return dist;
}

static void
MarkHighAltitudeStaticMapInstance(CEntity *e)
{
	if(!gbUseHighAltitudeStaticMapDistance || !ShouldForceHighAltitudeStaticMapEntity(e) ||
	   gNumHighAltitudeStaticMapInstances >= NUMVISIBLEENTITIES)
		return;

	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(e->GetModelIndex());
	for(int32 i = 0; i < gNumHighAltitudeStaticMapInstances; i++)
		if(gaHighAltitudeStaticMapInstances[i].modelInfo == mi &&
		   (gaHighAltitudeStaticMapInstances[i].position - e->GetPosition()).MagnitudeSqr() < 1.0f)
			return;

	gaHighAltitudeStaticMapInstances[gNumHighAltitudeStaticMapInstances].modelInfo = mi;
	gaHighAltitudeStaticMapInstances[gNumHighAltitudeStaticMapInstances].position = e->GetPosition();
	gNumHighAltitudeStaticMapInstances++;
}

static bool
IsHighAltitudeStaticMapInstanceQueued(CSimpleModelInfo *mi, const CVector &position)
{
	for(int32 i = 0; i < gNumHighAltitudeStaticMapInstances; i++)
		if(gaHighAltitudeStaticMapInstances[i].modelInfo == mi &&
		   (gaHighAltitudeStaticMapInstances[i].position - position).MagnitudeSqr() < 1.0f)
			return true;
	return false;
}
//- rouz edit (ChatGPT)

void
CRenderer::RenderRoads(void)
{
	int i;
	CEntity *e;

	PUSH_RENDERGROUP("CRenderer::RenderRoads");
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	SetCullMode(rwCULLMODECULLBACK);
	DeActivateDirectional();
	SetAmbientColours();

	for(i = 0; i < ms_nNoOfVisibleEntities; i++){
		e = ms_aVisibleEntityPtrs[i];
		if(IsRoad(e))
			RenderOneRoad(e);
	}
	POP_RENDERGROUP();
}

inline bool PutIntoSortedVehicleList(CVehicle *veh)
{
	if(veh->IsBoat()){
		int mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
		if(mode == CCam::MODE_WHEELCAM ||
		   mode == CCam::MODE_1STPERSON && TheCamera.GetLookDirection() != LOOKING_FORWARD && TheCamera.GetLookDirection() != LOOKING_BEHIND ||
		   CVisibilityPlugins::GetClumpAlpha(veh->GetClump()) != 255)
			return false;
		return true;
	}else
		return veh->bTouchingWater;		
}

//+ rouz edit (ChatGPT)
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
static bool IsVisibleToSoftwareEnvMap(CEntity *entity)
{
	// Test the entity bound against the reflection camera frustum
	if(!entity || !entity->m_rwObject || !entity->bIsVisible ||
	   entity->bRemoveFromWorld || !CustomPipes::EnvMapCam)
		return false;
	rw::Sphere sphere;
	CVector centre = entity->GetBoundCentre();
	sphere.center.x = centre.x;
	sphere.center.y = centre.y;
	sphere.center.z = centre.z;
	sphere.radius = entity->GetBoundRadius();
	return CustomPipes::EnvMapCam->frustumTestSphere(&sphere) != rw::Camera::SPHEREOUTSIDE;
}

//+ rouz edit (ChatGPT)
static void StoreSoftwareReflectionPoleShadow(CEntity *entity)
{
	// Register cached shadows for reflection-visible streetlight and signal poles
	if(!entity)
		return;
	// Compare model indices at runtime because model IDs are assigned while loading
	//+ rouz edit (ChatGPT)
	int modelIndex = entity->GetModelIndex();
	if(modelIndex == MI_TRAFFICLIGHTS)
		CShadows::StoreShadowForPole(entity, 2.957f, 0.147f, 0.0f, 16.0f, 0.4f, 0);
	else if(modelIndex == MI_TRAFFICLIGHTS_MIAMI)
		CShadows::StoreShadowForPole(entity, 4.819f, 1.315f, 0.0f, 16.0f, 0.4f, 0);
	else if(modelIndex == MI_TRAFFICLIGHTS_TWOVERTICAL)
		CShadows::StoreShadowForPole(entity, 7.503f, 0.0f, 0.0f, 16.0f, 0.4f, 0);
	else if(modelIndex == MI_SINGLESTREETLIGHTS1)
		CShadows::StoreShadowForPole(entity, 0.744f, 0.0f, 0.0f, 16.0f, 0.4f, 0);
	else if(modelIndex == MI_SINGLESTREETLIGHTS2)
		CShadows::StoreShadowForPole(entity, 0.043f, 0.0f, 0.0f, 16.0f, 0.4f, 0);
	else if(modelIndex == MI_SINGLESTREETLIGHTS3)
		CShadows::StoreShadowForPole(entity, 1.143f, 0.145f, 0.0f, 16.0f, 0.4f, 0);
	else if(modelIndex == MI_DOUBLESTREETLIGHTS)
		CShadows::StoreShadowForPole(entity, 0.0f, -0.048f, 0.0f, 16.0f, 0.4f, 0);
	//- rouz edit (ChatGPT)
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
static bool IsQueuedForSoftwareUnderwaterFade(CEntity *entity)
{
	// Find underwater entities already queued for the pass rendered beneath transparent water
	// Reject a missing entity before searching the visibility list
	if(!entity)
		return false;
	// Search the queued underwater entries for this entity
	CLink<CVisibilityPlugins::AlphaObjectInfo> *node;
	for(node = CVisibilityPlugins::m_alphaUnderwaterEntityList.head.next;
	    node != &CVisibilityPlugins::m_alphaUnderwaterEntityList.tail; node = node->next)
		if(node->item.entity == entity)
			return true;
	// Report when no underwater fade entry matches
	return false;
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
static int CompareEntityInfoByDistance(const void *leftValue, const void *rightValue)
{
	// Order reflection entities from nearest to farthest before reverse drawing
	const EntityInfo *left = (const EntityInfo*)leftValue;
	const EntityInfo *right = (const EntityInfo*)rightValue;
	if(left->sort < right->sort)
		return -1;
	if(left->sort > right->sort)
		return 1;
	return 0;
}
//- rouz edit (ChatGPT)
#endif
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
//+ rouz edit (ChatGPT)
static RwObject *GetSoftwareVehicleAtomicCB(RwObject *object, void *data)
{
	// Select render-enabled vehicle component atomics for matching alpha setup
	RpAtomic *atomic = (RpAtomic*)object;
	if(RwObjectGetType(object) == rpATOMIC && (RpAtomicGetFlags(atomic) & rpATOMICRENDER))
		*(RpAtomic**)data = atomic;
	return object;
}

static void RenderSoftwareAutomobileEffects(CVehicle *vehicle)
{
	// Reproduce the rotor transparency normally prepared by CAutomobile::Render
	if(!vehicle || !vehicle->IsCar() || !vehicle->IsRealHeli())
		return;
	CAutomobile *automobile = (CAutomobile*)vehicle;
	const int rotorAlpha = (int)((1.5f - Min(1.7f*Max(automobile->m_aWheelSpeed[1], 0.0f)/0.22f, 1.5f))*255.0f);
	const int blurAlpha = (int)(Max(1.5f*automobile->m_aWheelSpeed[1]/0.22f - 0.4f, 0.0f)*150.0f);
	RpAtomic *atomic = nil;
	if(automobile->m_aCarNodes[CAR_BONNET]){
		RwFrameForAllObjects(automobile->m_aCarNodes[CAR_BONNET], GetSoftwareVehicleAtomicCB, &atomic);
		if(atomic)
			vehicle->SetComponentAtomicAlpha(atomic, Min(rotorAlpha, 255));
	}
	atomic = nil;
	if(automobile->m_aCarNodes[CAR_BOOT]){
		RwFrameForAllObjects(automobile->m_aCarNodes[CAR_BOOT], GetSoftwareVehicleAtomicCB, &atomic);
		if(atomic)
			vehicle->SetComponentAtomicAlpha(atomic, Min(rotorAlpha, 255));
	}
	atomic = nil;
	if(automobile->m_aCarNodes[CAR_WINDSCREEN]){
		RwFrameForAllObjects(automobile->m_aCarNodes[CAR_WINDSCREEN], GetSoftwareVehicleAtomicCB, &atomic);
		if(atomic)
			vehicle->SetComponentAtomicAlpha(atomic, Min(blurAlpha, 150));
	}
	atomic = nil;
	if(automobile->m_aCarNodes[CAR_BUMP_REAR]){
		RwFrameForAllObjects(automobile->m_aCarNodes[CAR_BUMP_REAR], GetSoftwareVehicleAtomicCB, &atomic);
		if(atomic)
			vehicle->SetComponentAtomicAlpha(atomic, Min(blurAlpha, 150));
	}
}

static void RenderSoftwarePed(CPed *ped)
{
	// Reuse the current minigun pose during the additional reflection camera pass
	if(!ped)
		return;
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
	if(CustomPipes::bRenderingEnvMap && ped->IsPlayer()){
		CPlayerPed *player = (CPlayerPed*)ped;
		const float gunSpinSpeed = player->m_fGunSpinSpeed;
		player->m_fGunSpinSpeed = 0.0f;
		ped->Render();
		player->m_fGunSpinSpeed = gunSpinSpeed;
		return;
	}
#endif
	// Keep ordinary software-camera rendering on the standard animation path
	ped->Render();
}
//+ rouz edit (ChatGPT)
static bool IsSoftwareVehicleOccupant(CPed *ped)
{
	// Match CPed::Render's in-vehicle condition while leaving exit animations in the ped pass
	return ped && ped->bInVehicle && ped->m_pMyVehicle &&
		ped->m_nPedState != PED_EXIT_CAR && ped->m_nPedState != PED_DRAG_FROM_CAR;
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
#endif
//- rouz edit (ChatGPT)

void
CRenderer::RenderEverythingBarRoads(void)
{
	int i;
	CEntity *e;
	EntityInfo ei;

	PUSH_RENDERGROUP("CRenderer::RenderEverythingBarRoads");
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	SetCullMode(rwCULLMODECULLBACK);
	gSortedVehiclesAndPeds.Clear();

	for(i = 0; i < ms_nNoOfVisibleEntities; i++){
		e = ms_aVisibleEntityPtrs[i];

		if(IsRoad(e))
			continue;

#ifdef EXTENDED_PIPELINES
		if(CustomPipes::bRenderingEnvMap && (e->IsPed() || e->IsVehicle()))
			continue;
#endif
		//+ rouz edit (ChatGPT)
		// Let the software reflection pass gather props from its own camera frustum
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
		if(CustomPipes::bRenderingEnvMap && e->IsObject())
			continue;
#endif
		//- rouz edit (ChatGPT)

		if(e->IsVehicle() ||
		   e->IsPed() && CVisibilityPlugins::GetClumpAlpha((RpClump*)e->m_rwObject) != 255){
			if(e->IsVehicle() && PutIntoSortedVehicleList((CVehicle*)e)){
				ei.ent = e;
				ei.sort = (ms_vecCameraPosition - e->GetPosition()).MagnitudeSqr();
				gSortedVehiclesAndPeds.InsertSorted(ei);
			}else{
				if(!CVisibilityPlugins::InsertEntityIntoSortedList(e, (ms_vecCameraPosition - e->GetPosition()).Magnitude())){
					printf("Ran out of space in alpha entity list");
					RenderOneNonRoad(e);
				}
			}
		}else
			RenderOneNonRoad(e);
	}
	POP_RENDERGROUP();
}

void
CRenderer::RenderBoats(void)
{
	CLink<EntityInfo> *node;

	PUSH_RENDERGROUP("CRenderer::RenderBoats");
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	SetCullMode(rwCULLMODECULLBACK);
	//+ rouz edit (ChatGPT)
	// Gather reflection-camera boats independently from the main camera visibility list
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	if(CustomPipes::bRenderingEnvMap && CustomPipes::EnvMapCam){
		CVehiclePool *vehiclePool = CPools::GetVehiclePool();
		if(vehiclePool)
			for(int32 i = 0; i < vehiclePool->GetSize(); i++){
				CVehicle *vehicle = vehiclePool->GetSlot(i);
				if(vehicle && vehicle->IsBoat() && IsVisibleToSoftwareEnvMap(vehicle)){
					// Add the Skimmer's seaplane shadow to the reflection-only effect queue
					//+ rouz edit (ChatGPT)
					if(vehicle->GetModelIndex() == MI_SKIMMER)
						CustomPipes::StoreVehicleShadowForEnvMap(vehicle, VEH_SHD_TYPE_SEAPLANE);
					//- rouz edit (ChatGPT)
					RenderSoftwareVehicle(vehicle);
				}
			}
		POP_RENDERGROUP();
		return;
	}
#endif
	//- rouz edit (ChatGPT)

#ifdef NEW_RENDERER
	int i;
	CEntity *e;
	EntityInfo ei;
	if(gbNewRenderer){
		gSortedVehiclesAndPeds.Clear();
		// not the real thing
		for(i = 0; i < ms_nNoOfVisibleVehicles; i++){
			e = ms_aVisibleVehiclePtrs[i];
			if(e->IsVehicle() && PutIntoSortedVehicleList((CVehicle*)e)){
				ei.ent = e;
				ei.sort = (ms_vecCameraPosition - e->GetPosition()).MagnitudeSqr();
				gSortedVehiclesAndPeds.InsertSorted(ei);
			}
		}
	}
#endif

	for(node = gSortedVehiclesAndPeds.tail.prev;
	    node != &gSortedVehiclesAndPeds.head;
	    node = node->prev){
		CVehicle *v = (CVehicle*)node->item.ent;
		//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
		// Submit boat clumps through the CPU renderer
		RenderSoftwareVehicle(v);
#else
		RenderOneNonRoad(v);
#endif
		//- rouz edit (ChatGPT)
	}
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	// Submit translucent boats omitted from the sorted boat list
	for(int32 i = 0; i < ms_nNoOfVisibleVehicles; i++){
		CEntity *entity = ms_aVisibleVehiclePtrs[i];
		if(entity && entity->IsVehicle() && entity->m_rwObject && ((CVehicle*)entity)->IsBoat() &&
		   !PutIntoSortedVehicleList((CVehicle*)entity))
			RenderSoftwareVehicle((CVehicle*)entity);
	}
#endif
	//- rouz edit (ChatGPT)
	POP_RENDERGROUP();
}

#ifdef NEW_RENDERER
#ifndef LIBRW
#error "Need librw for EXTENDED_PIPELINES"
#endif
#include "WaterLevel.h"

enum {
	// blend passes
	PASS_NOZ,	// no z-write
	PASS_ADD,	// additive
	PASS_BLEND	// normal blend
};

static void
SetStencilState(int state)
{
	switch(state){
	// disable stencil
	case 0:
		rw::SetRenderState(rw::STENCILENABLE, FALSE);
		break;
	// test against stencil
	case 1:
		rw::SetRenderState(rw::STENCILENABLE, TRUE);
		rw::SetRenderState(rw::STENCILFUNCTION, rw::STENCILNOTEQUAL);
		rw::SetRenderState(rw::STENCILPASS, rw::STENCILKEEP);
		rw::SetRenderState(rw::STENCILFAIL, rw::STENCILKEEP);
		rw::SetRenderState(rw::STENCILZFAIL, rw::STENCILKEEP);
		rw::SetRenderState(rw::STENCILFUNCTIONMASK, 0xFF);
		rw::SetRenderState(rw::STENCILFUNCTIONREF, 0xFF);
		break;
	// write to stencil
	case 2:
		rw::SetRenderState(rw::STENCILENABLE, TRUE);
		rw::SetRenderState(rw::STENCILFUNCTION, rw::STENCILALWAYS);
		rw::SetRenderState(rw::STENCILPASS, rw::STENCILREPLACE);
		rw::SetRenderState(rw::STENCILFUNCTIONREF, 0xFF);
		break;
	}
}

void
CRenderer::RenderOneBuilding(CEntity *ent, float camdist)
{
	if(ent->m_rwObject == nil)
		return;

	ent->bImBeingRendered = true;	// TODO: this seems wrong, but do we even need it?

	assert(RwObjectGetType(ent->m_rwObject) == rpATOMIC);
	RpAtomic *atomic = (RpAtomic*)ent->m_rwObject;
	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(ent->GetModelIndex());
	// Select the world blend pass from the building model flags
	//+ rouz edit (ChatGPT)
	int pass = PASS_BLEND;
	if(mi->m_additive)	// very questionable
		pass = PASS_ADD;
	if(mi->m_noZwrite)
		pass = PASS_NOZ;
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	// Preserve the building blend mode and distance-fade LOD on the CPU path
	//+ rouz edit (ChatGPT)
	uint32 alpha = 255;
	RpGeometry *originalGeometry = nil;
	if(ent->bDistanceFade){
		RpAtomic *lodAtomic = mi->GetAtomicFromDistance(camdist-FADE_DISTANCE);
		float fadeFactor = (mi->GetLargestLodDistance()-(camdist-FADE_DISTANCE))/FADE_DISTANCE;
		fadeFactor = Clamp(fadeFactor, 0.0f, 1.0f);
		alpha = (uint32)(mi->m_alpha*fadeFactor);
		if(alpha < 255 && lodAtomic){
			RpGeometry *fadeGeometry = RpAtomicGetGeometry(lodAtomic);
			if(fadeGeometry){
				originalGeometry = RpAtomicGetGeometry(atomic);
				if(fadeGeometry != originalGeometry)
					RpAtomicSetGeometry(atomic, fadeGeometry, rpATOMICSAMEBOUNDINGSPHERE);
			}else
				alpha = 255;
		}else
			alpha = 255;
	}
	const rw::uint32 oldVertexAlpha = rw::GetRenderState(rw::VERTEXALPHA);
	const rw::uint32 oldDepthWrite = rw::GetRenderState(rw::ZWRITEENABLE);
	const rw::uint32 oldSourceBlend = rw::GetRenderState(rw::SRCBLEND);
	const rw::uint32 oldDestinationBlend = rw::GetRenderState(rw::DESTBLEND);
	// Match the alpha and depth-write settings used by the corresponding blend pass
	if(alpha < 255)
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND,
		(void*)(pass == PASS_ADD ? rwBLENDONE : rwBLENDINVSRCALPHA));
	if(pass == PASS_NOZ || alpha < 255)
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	SoftwarePolygons::RenderAtomic(atomic, (int)alpha);
	// Restore the selected LOD geometry and render states after this building
	if(originalGeometry)
		RpAtomicSetGeometry(atomic, originalGeometry, rpATOMICSAMEBOUNDINGSPHERE);
	rw::SetRenderState(rw::VERTEXALPHA, oldVertexAlpha);
	rw::SetRenderState(rw::ZWRITEENABLE, oldDepthWrite);
	rw::SetRenderState(rw::SRCBLEND, oldSourceBlend);
	rw::SetRenderState(rw::DESTBLEND, oldDestinationBlend);
	//- rouz edit (ChatGPT)
	ent->bImBeingRendered = false;
	return;
#endif
	//- rouz edit (ChatGPT)

	if(ent->bDistanceFade){
		RpAtomic *lodatm;
		float fadefactor;
		uint32 alpha;

		lodatm = mi->GetAtomicFromDistance(camdist - FADE_DISTANCE);
		fadefactor = (mi->GetLargestLodDistance() - (camdist - FADE_DISTANCE))/FADE_DISTANCE;
		if(fadefactor > 1.0f)
			fadefactor = 1.0f;
		alpha = mi->m_alpha * fadefactor;

		if(alpha == 255 || lodatm == nil) // rouz edit (ChatGPT)
			WorldRender::AtomicFirstPass(atomic, pass);
		else{
			// not quite sure what this is about, do we have to do that?
			RpGeometry *geo = RpAtomicGetGeometry(lodatm);
			if(geo != RpAtomicGetGeometry(atomic))
				RpAtomicSetGeometry(atomic, geo, rpATOMICSAMEBOUNDINGSPHERE);
			WorldRender::AtomicFullyTransparent(atomic, pass, alpha);
		}
	}else
		WorldRender::AtomicFirstPass(atomic, pass);

	ent->bImBeingRendered = false;	// TODO: this seems wrong, but do we even need it?
}

//+ rouz edit (ChatGPT)
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
template <class PoolType>
static void RenderSoftwareReflectionBuildingPool(PoolType *pool, int pass, const CVector &cameraPosition)
{
	// Draw loaded building atomics visible only from the reflection camera
	if(!pool)
		return;
	for(int i = 0; i < pool->GetSize(); i++){
		CEntity *entity = pool->GetSlot(i);
		if(!IsVisibleToSoftwareEnvMap(entity) || RwObjectGetType(entity->m_rwObject) != rpATOMIC)
			continue;
		if(!entity->bOffscreen &&
		   (entity->bIsBIGBuilding || entity->m_scanCode == CWorld::GetCurrentScanCode()))
			continue;
		if(!IsAreaVisible(entity->m_area))
			continue;

		CSimpleModelInfo *modelInfo = (CSimpleModelInfo*)CModelInfo::GetModelInfo(entity->GetModelIndex());
		if(!modelInfo)
			continue;
		if(modelInfo->GetModelType() == MITYPE_TIME){
			CTimeModelInfo *timeInfo = (CTimeModelInfo*)modelInfo;
			if(!CClock::GetIsTimeInRange(timeInfo->GetTimeOn(), timeInfo->GetTimeOff())){
				int32 otherModel = timeInfo->GetOtherTimeModel();
				if(otherModel == -1 || CModelInfo::GetModelInfo(otherModel)->GetRwObject())
					continue;
			}
		}

		const float distance = (cameraPosition - entity->GetPosition()).Magnitude();
		const bool roadsPass = entity->bIsBIGBuilding || IsRoad(entity);
		if((pass == 0) != roadsPass)
			continue;
		if(entity->bIsBIGBuilding){
			CSimpleModelInfo *nonLOD = modelInfo->GetRelatedModel();
			if(nonLOD && IsHighAltitudeStaticMapInstanceQueued(nonLOD, entity->GetPosition()))
				continue;
			if(distance < modelInfo->GetNearDistance() && distance < LOD_DISTANCE){
				if(nonLOD == nil || (nonLOD->GetRwObject() && nonLOD->m_alpha == 255))
					continue;
				if(nonLOD->GetModelType() == MITYPE_TIME){
					CTimeModelInfo *timeNonLOD = (CTimeModelInfo*)nonLOD;
					int32 otherModel = timeNonLOD->GetOtherTimeModel();
					if(otherModel != -1 && CModelInfo::GetModelInfo(otherModel)->GetRwObject())
						continue;
				}
			}
		}

		RpAtomic *drawAtomic = modelInfo->GetFirstAtomicFromDistance(distance);
		if(!drawAtomic && !modelInfo->m_noFade)
			drawAtomic = modelInfo->GetFirstAtomicFromDistance(distance - FADE_DISTANCE);
		if(!drawAtomic)
			continue;
		RpAtomic *atomic = (RpAtomic*)entity->m_rwObject;
		RpGeometry *savedGeometry = RpAtomicGetGeometry(atomic);
		if(RpAtomicGetGeometry(drawAtomic) != savedGeometry)
			RpAtomicSetGeometry(atomic, RpAtomicGetGeometry(drawAtomic), rpATOMICSAMEBOUNDINGSPHERE);
		const bool savedDistanceFade = entity->bDistanceFade;
		entity->bDistanceFade = false;
		CRenderer::RenderOneBuilding(entity, distance);
		entity->bDistanceFade = savedDistanceFade;
		if(RpAtomicGetGeometry(atomic) != savedGeometry)
			RpAtomicSetGeometry(atomic, savedGeometry, rpATOMICSAMEBOUNDINGSPHERE);
	}
}

static void RenderSoftwareReflectionBuildings(int pass)
{
	// Use the auxiliary camera position to select visible static building LODs
	if(!CustomPipes::bRenderingEnvMap || !CustomPipes::EnvMapCam ||
	   !CustomPipes::EnvMapCam->getFrame())
		return;
	const CVector cameraPosition(CustomPipes::EnvMapCam->getFrame()->getLTM()->pos);
	RenderSoftwareReflectionBuildingPool(CPools::GetBuildingPool(), pass, cameraPosition);
	RenderSoftwareReflectionBuildingPool(CPools::GetTreadablePool(), pass, cameraPosition);
}
#endif
//- rouz edit (ChatGPT)

void
CRenderer::RenderWorld(int pass)
{
	int i;
	CEntity *e;
	CLink<CVisibilityPlugins::AlphaObjectInfo> *node;

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	SetCullMode(rwCULLMODECULLBACK);
	DeActivateDirectional();
	SetAmbientColours();

	// Temporary...have to figure out sorting better
	switch(pass){
	case 0:
		// Roads
		PUSH_RENDERGROUP("CRenderer::RenderWorld - Roads");
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
		for(i = 0; i < ms_nNoOfVisibleBuildings; i++){
			e = ms_aVisibleBuildingPtrs[i];
			if(e->bIsBIGBuilding || IsRoad(e))
				RenderOneBuilding(e);
		}
		for(node = CVisibilityPlugins::m_alphaBuildingList.tail.prev;
		    node != &CVisibilityPlugins::m_alphaBuildingList.head;
		    node = node->prev){
			e = node->item.entity;
			if(e->bIsBIGBuilding || IsRoad(e))
				RenderOneBuilding(e, node->item.sort);
		}
		// Include reflection-only roads and distant building LODs
		//+ rouz edit (ChatGPT)
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
		RenderSoftwareReflectionBuildings(0);
#endif
		//- rouz edit (ChatGPT)
		POP_RENDERGROUP();
		break;
	case 1:
		// Opaque
		PUSH_RENDERGROUP("CRenderer::RenderWorld - Opaque");
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
		for(i = 0; i < ms_nNoOfVisibleBuildings; i++){
			e = ms_aVisibleBuildingPtrs[i];
			if(!(e->bIsBIGBuilding || IsRoad(e)))
				RenderOneBuilding(e);
		}
		for(node = CVisibilityPlugins::m_alphaBuildingList.tail.prev;
		    node != &CVisibilityPlugins::m_alphaBuildingList.head;
		    node = node->prev){
			e = node->item.entity;
			if(!(e->bIsBIGBuilding || IsRoad(e)))
				RenderOneBuilding(e, node->item.sort);
		}
		// Include reflection-only opaque building atomics
		//+ rouz edit (ChatGPT)
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
		RenderSoftwareReflectionBuildings(1);
#endif
		//- rouz edit (ChatGPT)
		// Now we have iterated through all visible buildings (unsorted and sorted)
		// and the transparency list is done.

		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, FALSE);
		WorldRender::RenderBlendPass(PASS_NOZ);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
		POP_RENDERGROUP();
		break;
	case 2:
		// Transparent
		PUSH_RENDERGROUP("CRenderer::RenderWorld - Transparent");
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
		WorldRender::RenderBlendPass(PASS_ADD);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		WorldRender::RenderBlendPass(PASS_BLEND);
		POP_RENDERGROUP();
		break;
	}
}

void
CRenderer::RenderPeds(void)
{
	int i;
	CEntity *e;

	PUSH_RENDERGROUP("CRenderer::RenderPeds");
	//+ rouz edit (ChatGPT)
	// Gather reflection-camera peds independently from the main camera visibility list
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	if(CustomPipes::bRenderingEnvMap && CustomPipes::EnvMapCam){
		CPedPool *pedPool = CPools::GetPedPool();
		if(pedPool)
			for(i = 0; i < pedPool->GetSize(); i++){
				CPed *ped = pedPool->GetSlot(i);
				if(IsVisibleToSoftwareEnvMap(ped))
					RenderOneNonRoad(ped);
			}
		POP_RENDERGROUP();
		return;
	}
#endif
	//- rouz edit (ChatGPT)
	for(i = 0; i < ms_nNoOfVisibleVehicles; i++){
		e = ms_aVisibleVehiclePtrs[i];
		if(e->IsPed())
			RenderOneNonRoad(e);
	}
	POP_RENDERGROUP();
}

void
CRenderer::RenderVehicles(void)
{
	int i;
	CEntity *e;
	EntityInfo ei;
	CLink<EntityInfo> *node;

	PUSH_RENDERGROUP("CRenderer::RenderVehicles");
	// not the real thing
	for(i = 0; i < ms_nNoOfVisibleVehicles; i++){
		e = ms_aVisibleVehiclePtrs[i];
		if(!e->IsVehicle())
			continue;
		if(PutIntoSortedVehicleList((CVehicle*)e))
			continue;	// boats handled elsewhere
		ei.ent = e;
		ei.sort = (ms_vecCameraPosition - e->GetPosition()).MagnitudeSqr();
		gSortedVehiclesAndPeds.InsertSorted(ei);
	}

	for(node = gSortedVehiclesAndPeds.tail.prev;
	    node != &gSortedVehiclesAndPeds.head;
	    node = node->prev)
		RenderOneNonRoad(node->item.ent);
	POP_RENDERGROUP();
}

//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
static void RenderSoftwareVehicleOccupant(CPed *ped)
{
	// Skip missing peds and peds that are not seated in this vehicle
	if(!ped || (ped->m_nPedState != PED_DRIVING && !IsSoftwareVehicleOccupant(ped))) // rouz edit (ChatGPT)
		return;
	// Respect the independent pedestrian stage for seated occupants
	if(!SoftwarePolygons::renderPedsStage.load(std::memory_order_relaxed)) // rouz edit (ChatGPT)
		return; // rouz edit (ChatGPT)
	// Respect the existing debug switch for pedestrians drawn through vehicle seats
	//+ rouz edit (ChatGPT)
#ifndef FINAL
	if(gbDontRenderPeds)
		return;
#endif
	//- rouz edit (ChatGPT)
	// Draw seated peds here because the ordinary pedestrian pass skips them
	//+ rouz edit (ChatGPT)
	const uint32 oldRenderPedInCar = ped->bRenderPedInCar;
	ped->bRenderPedInCar = true;
	RenderSoftwarePed(ped); // rouz edit (ChatGPT)
	ped->bRenderPedInCar = oldRenderPedInCar;
	//- rouz edit (ChatGPT)
}

static void RenderSoftwareVehicle(CVehicle *vehicle)
{
	// Skip vehicles without renderable geometry
	if(!vehicle || !vehicle->m_rwObject)
		return;
	// Respect the vehicle stage in every software vehicle pass
	if(!SoftwarePolygons::renderVehiclesStage.load(std::memory_order_relaxed)) // rouz edit (ChatGPT)
		return; // rouz edit (ChatGPT)
	// Respect the renderer's debug visibility switch in every software vehicle pass
	//+ rouz edit (ChatGPT)
#ifndef FINAL
	if(gbDontRenderVehicles)
		return;
#endif
	//- rouz edit (ChatGPT)
	// Preserve CPlane::Render hiding planes during cutscenes
	//+ rouz edit (ChatGPT)
	if(vehicle->IsPlane() && CCutsceneMgr::IsRunning())
		return;
	//- rouz edit (ChatGPT)
	// Preserve the caller's culling state while drawing both sides of vehicle meshes
	//+ rouz edit (ChatGPT)
	const rw::uint32 oldCullMode = rw::GetRenderState(rw::CULLMODE);
	rw::SetRenderState(rw::CULLMODE, rw::CULLNONE);
	//- rouz edit (ChatGPT)
	// Render distant atomic vehicle LODs with the regular entity lighting
	//+ rouz edit (ChatGPT)
	if(RwObjectGetType(vehicle->m_rwObject) == rpATOMIC){
		const bool resetLights = vehicle->SetupLighting();
		// Draw seated occupants before the distant vehicle body
		//+ rouz edit (ChatGPT)
		RenderSoftwareVehicleOccupant(vehicle->pDriver);
		for(int passenger = 0; passenger < 8; passenger++)
			RenderSoftwareVehicleOccupant(vehicle->pPassengers[passenger]);
		//- rouz edit (ChatGPT)
		// Apply the current paint colors before rasterizing the distant body
		//+ rouz edit (ChatGPT)
		CBaseModelInfo *modelInfo = CModelInfo::GetModelInfo(vehicle->GetModelIndex());
		if(modelInfo && modelInfo->GetModelType() == MITYPE_VEHICLE){
			((CVehicleModelInfo*)modelInfo)->SetVehicleColour(vehicle->m_currentColour1, vehicle->m_currentColour2);
		}
		//- rouz edit (ChatGPT)
		// Preserve vehicle visibility callbacks while the installed atomic pipeline targets the CPU framebuffer
		vehicle->Render(); // rouz edit (ChatGPT)
		vehicle->RemoveLighting(resetLights);
		rw::SetRenderState(rw::CULLMODE, oldCullMode);
		return;
	}
	//- rouz edit (ChatGPT)
	// Skip other RenderWare object types
	if(RwObjectGetType(vehicle->m_rwObject) != rpCLUMP){
		rw::SetRenderState(rw::CULLMODE, oldCullMode);
		return;
	}
	// Rotate helicopter rotor frames when the software clump path bypasses CHeli::Render
	//+ rouz edit (ChatGPT)
	bool renderingReflection = false;
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
	renderingReflection = CustomPipes::bRenderingEnvMap;
#endif
	// Keep reflection-only rotor poses temporary so the extra camera pass cannot advance simulation state
	//+ rouz edit (ChatGPT)
	RwFrame *reflectionTopRotor = nil;
	RwFrame *reflectionBackRotor = nil;
	RwFrame *reflectionRhinoTurret = nil;
	RwFrame *reflectionDodoPropeller = nil;
	RwFrame *reflectionDodoRudder = nil;
	RwFrame *reflectionPlaneTopRotor = nil;
	RwFrame *reflectionPlaneBackRotor = nil;
	RwFrame *reflectionRealHeliRotors[4] = { nil, nil, nil, nil };
	RwMatrix savedTopRotorMatrix;
	RwMatrix savedBackRotorMatrix;
	RwMatrix savedRhinoTurretMatrix;
	RwMatrix savedDodoPropellerMatrix;
	RwMatrix savedDodoRudderMatrix;
	RwMatrix savedPlaneTopRotorMatrix;
	RwMatrix savedPlaneBackRotorMatrix;
	RwMatrix savedRealHeliRotorMatrices[4];
	if(vehicle->IsHeli()){
		CHeli *heli = (CHeli*)vehicle;
		CMatrix rotorMatrix;
		CVector rotorPosition;
		if(renderingReflection){
			reflectionTopRotor = heli->m_aHeliNodes[HELI_TOPROTOR];
			reflectionBackRotor = heli->m_aHeliNodes[HELI_BACKROTOR];
			if(reflectionTopRotor){
				RwMatrixCopy(&savedTopRotorMatrix, RwFrameGetMatrix(reflectionTopRotor));
				rotorMatrix.Attach(RwFrameGetMatrix(reflectionTopRotor));
				rotorPosition = rotorMatrix.GetPosition();
				rotorMatrix.SetRotateZ(heli->m_fRotorRotation);
				rotorMatrix.Translate(rotorPosition);
				rotorMatrix.UpdateRW();
				RwFrameUpdateObjects(reflectionTopRotor);
			}
			if(reflectionBackRotor){
				RwMatrixCopy(&savedBackRotorMatrix, RwFrameGetMatrix(reflectionBackRotor));
				rotorMatrix.Attach(RwFrameGetMatrix(reflectionBackRotor));
				rotorPosition = rotorMatrix.GetPosition();
				rotorMatrix.SetRotateX(heli->m_fRotorRotation);
				rotorMatrix.Translate(rotorPosition);
				rotorMatrix.UpdateRW();
				RwFrameUpdateObjects(reflectionBackRotor);
			}
		}else{
			rotorMatrix.Attach(RwFrameGetMatrix(heli->m_aHeliNodes[HELI_TOPROTOR]));
			rotorPosition = rotorMatrix.GetPosition();
			rotorMatrix.SetRotateZ(heli->m_fRotorRotation);
			rotorMatrix.Translate(rotorPosition);
			rotorMatrix.UpdateRW();
			rotorMatrix.Attach(RwFrameGetMatrix(heli->m_aHeliNodes[HELI_BACKROTOR]));
			rotorPosition = rotorMatrix.GetPosition();
			rotorMatrix.SetRotateX(heli->m_fRotorRotation);
			rotorMatrix.Translate(rotorPosition);
			rotorMatrix.UpdateRW();
			heli->m_fRotorRotation += 3.14f/6.5f;
			if(heli->m_fRotorRotation > 6.28f)
				heli->m_fRotorRotation -= 6.28f;
		}
	}
	//- rouz edit (ChatGPT)
	// Draw seated occupants with the vehicle lighting before the body
	//+ rouz edit (ChatGPT)
	const bool resetLights = vehicle->SetupLighting();
	RenderSoftwareVehicleOccupant(vehicle->pDriver);
	for(int passenger = 0; passenger < 8; passenger++)
		RenderSoftwareVehicleOccupant(vehicle->pPassengers[passenger]);
	//- rouz edit (ChatGPT)
	// Apply the current paint colors before rasterizing shared vehicle materials
	//+ rouz edit (ChatGPT)
	CBaseModelInfo *modelInfo = CModelInfo::GetModelInfo(vehicle->GetModelIndex());
	if(modelInfo && modelInfo->GetModelType() == MITYPE_VEHICLE){
		((CVehicleModelInfo*)modelInfo)->SetVehicleColour(vehicle->m_currentColour1, vehicle->m_currentColour2);
	}
	//- rouz edit (ChatGPT)
	// Keep the wheels-only cheat from drawing boat hulls while retaining the patrol boat propeller
	//+ rouz edit (ChatGPT)
	if(CVehicle::bWheelsOnlyCheat && vehicle->IsBoat()){
		CBoat *boat = (CBoat*)vehicle;
		boat->m_nSetPieceExtendedRangeTime = CTimer::GetTimeInMilliseconds() + 3000;
		if((boat->GetModelIndex() == MI_PREDATOR || boat->GetModelIndex() == MI_REEFER) && boat->m_aBoatNodes[BOAT_MOVING]){
			RpAtomic *propeller = nil;
			RwFrameForAllObjects(boat->m_aBoatNodes[BOAT_MOVING], GetSoftwareVehicleAtomicCB, &propeller);
			if(propeller)
				SoftwarePolygons::RenderAtomic((rw::Atomic*)propeller);
		}
		vehicle->RemoveLighting(resetLights);
		rw::SetRenderState(rw::CULLMODE, oldCullMode);
		return;
	}
	//- rouz edit (ChatGPT)
	// Match the automobile wheels-only cheat in the software clump path
	//+ rouz edit (ChatGPT)
	if(CVehicle::bWheelsOnlyCheat && vehicle->IsCar()){
		CAutomobile *automobile = (CAutomobile*)vehicle;
		const int wheelNodes[] = { CAR_WHEEL_RB, CAR_WHEEL_LB, CAR_WHEEL_RF, CAR_WHEEL_LF, CAR_WHEEL_RM, CAR_WHEEL_LM };
		for(int wheel = 0; wheel < 6; wheel++){
			if(!automobile->m_aCarNodes[wheelNodes[wheel]])
				continue;
			RwObject *object = GetFirstObject(automobile->m_aCarNodes[wheelNodes[wheel]]);
			if(object)
				SoftwarePolygons::RenderAtomic((rw::Atomic*)object);
		}
		vehicle->RemoveLighting(resetLights);
		rw::SetRenderState(rw::CULLMODE, oldCullMode);
		return;
	}
	//- rouz edit (ChatGPT)
	// Apply reflection-only vehicle part poses after wheel-cheat exits so every saved matrix is restored
	//+ rouz edit (ChatGPT)
	if(renderingReflection && vehicle->GetModelIndex() == MI_RHINO){
		CAutomobile *tank = (CAutomobile*)vehicle;
		reflectionRhinoTurret = tank->m_aCarNodes[CAR_WINDSCREEN];
		if(reflectionRhinoTurret){
			RwMatrixCopy(&savedRhinoTurretMatrix, RwFrameGetMatrix(reflectionRhinoTurret));
			CMatrix turretMatrix;
			turretMatrix.Attach(RwFrameGetMatrix(reflectionRhinoTurret));
			CVector turretPosition = turretMatrix.GetPosition();
			turretMatrix.SetRotateZ(tank->m_fCarGunLR);
			turretMatrix.Translate(turretPosition);
			turretMatrix.UpdateRW();
			RwFrameUpdateObjects(reflectionRhinoTurret);
		}
	}
	if(renderingReflection && vehicle->GetModelIndex() == MI_DODO){
		CAutomobile *dodo = (CAutomobile*)vehicle;
		CMatrix partMatrix;
		CVector partPosition;
		reflectionDodoPropeller = dodo->m_aCarNodes[CAR_WINDSCREEN];
		if(reflectionDodoPropeller){
			RwMatrixCopy(&savedDodoPropellerMatrix, RwFrameGetMatrix(reflectionDodoPropeller));
			partMatrix.Attach(RwFrameGetMatrix(reflectionDodoPropeller));
			partPosition = partMatrix.GetPosition();
			partMatrix.SetRotateY(dodo->m_fPropellerRotation);
			partMatrix.Translate(partPosition);
			partMatrix.UpdateRW();
			RwFrameUpdateObjects(reflectionDodoPropeller);
		}
		reflectionDodoRudder = dodo->m_aCarNodes[CAR_BOOT];
		if(reflectionDodoRudder && dodo->Damage.GetDoorStatus(DOOR_BOOT) != DOOR_STATUS_MISSING){
			RwMatrixCopy(&savedDodoRudderMatrix, RwFrameGetMatrix(reflectionDodoRudder));
			partMatrix.Attach(RwFrameGetMatrix(reflectionDodoRudder));
			partPosition = partMatrix.GetPosition();
			partMatrix.SetRotate(0.0f, 0.0f, -dodo->m_fSteerAngle);
			partMatrix.Rotate(0.0f, Sin(dodo->m_fSteerAngle)*DEGTORAD(22.0f), 0.0f);
			partMatrix.Translate(partPosition);
			partMatrix.UpdateRW();
			RwFrameUpdateObjects(reflectionDodoRudder);
		}else
			reflectionDodoRudder = nil;
	}
	// Spin the airport chopper rotors for reflection-only visibility without advancing their angle
	//+ rouz edit (ChatGPT)
#ifdef CPLANE_ROTORS
	if(renderingReflection && vehicle->GetModelIndex() == MI_CHOPPER){
		CPlane *plane = (CPlane*)vehicle;
		CMatrix rotorMatrix;
		CVector rotorPosition;
		reflectionPlaneTopRotor = plane->m_aPlaneNodes[PLANE_TOPROTOR];
		if(reflectionPlaneTopRotor){
			RwMatrixCopy(&savedPlaneTopRotorMatrix, RwFrameGetMatrix(reflectionPlaneTopRotor));
			rotorMatrix.Attach(RwFrameGetMatrix(reflectionPlaneTopRotor));
			rotorPosition = rotorMatrix.GetPosition();
			rotorMatrix.SetRotateZ(plane->m_fRotorRotation);
			rotorMatrix.Translate(rotorPosition);
			rotorMatrix.UpdateRW();
			RwFrameUpdateObjects(reflectionPlaneTopRotor);
		}
		reflectionPlaneBackRotor = plane->m_aPlaneNodes[PLANE_BACKROTOR];
		if(reflectionPlaneBackRotor){
			RwMatrixCopy(&savedPlaneBackRotorMatrix, RwFrameGetMatrix(reflectionPlaneBackRotor));
			rotorMatrix.Attach(RwFrameGetMatrix(reflectionPlaneBackRotor));
			rotorPosition = rotorMatrix.GetPosition();
			rotorMatrix.SetRotateX(plane->m_fRotorRotation);
			rotorMatrix.Translate(rotorPosition);
			rotorMatrix.UpdateRW();
			RwFrameUpdateObjects(reflectionPlaneBackRotor);
		}
	}
#endif
	//- rouz edit (ChatGPT)
	// Rotate the four rotor components on automobile-based helicopters for this reflection only
	//+ rouz edit (ChatGPT)
	if(renderingReflection && vehicle->IsCar() && ((CAutomobile*)vehicle)->IsRealHeli()){
		CAutomobile *heli = (CAutomobile*)vehicle;
		const int rotorNodes[4] = { CAR_BONNET, CAR_WINDSCREEN, CAR_BOOT, CAR_BUMP_REAR };
		const float rotorAngles[4] = {
			heli->m_aWheelRotation[1], -heli->m_aWheelRotation[1],
			heli->m_aWheelRotation[3], -heli->m_aWheelRotation[3]
		};
		for(int rotor = 0; rotor < 4; rotor++){
			reflectionRealHeliRotors[rotor] = heli->m_aCarNodes[rotorNodes[rotor]];
			if(!reflectionRealHeliRotors[rotor])
				continue;
			RwMatrixCopy(&savedRealHeliRotorMatrices[rotor], RwFrameGetMatrix(reflectionRealHeliRotors[rotor]));
			CMatrix rotorMatrix;
			rotorMatrix.Attach(RwFrameGetMatrix(reflectionRealHeliRotors[rotor]));
			CVector rotorPosition = rotorMatrix.GetPosition();
			if(rotor < 2)
				rotorMatrix.SetRotateZ(rotorAngles[rotor]);
			else
				rotorMatrix.SetRotateX(rotorAngles[rotor]);
			rotorMatrix.Translate(rotorPosition);
			rotorMatrix.UpdateRW();
			RwFrameUpdateObjects(reflectionRealHeliRotors[rotor]);
		}
	}
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
	// Preserve the extended range of boat and bike set pieces when bypassing their Render overrides
	//+ rouz edit (ChatGPT)
	if(vehicle->IsBike() || vehicle->IsBoat())
		vehicle->m_nSetPieceExtendedRangeTime = CTimer::GetTimeInMilliseconds() + 3000;
	//- rouz edit (ChatGPT)
	// Restore rotor alpha before submitting the automobile clump
	//+ rouz edit (ChatGPT)
	RenderSoftwareAutomobileEffects(vehicle);
	//- rouz edit (ChatGPT)
	// Submit the clump and its deferred translucent parts to the CPU rasterizer
	//+ rouz edit (ChatGPT)
	RpClump *clump = (RpClump*)vehicle->m_rwObject;
	CVisibilityPlugins::SetupVehicleVariables(clump);
	CVisibilityPlugins::InitAlphaAtomicList();
	SoftwarePolygons::RenderVehicleClump(clump);
	CVisibilityPlugins::RenderAlphaAtomics();
	//+ rouz edit (ChatGPT)
#ifdef NEW_RENDERER
	// Preserve the legacy boat hull mask when the software renderer bypasses CBoat::Render
	if(!gbNewRenderer && vehicle->IsBoat())
#else
	// Preserve boat hull masking when the software renderer bypasses CBoat::Render
	if(vehicle->IsBoat())
#endif
		((CBoat*)vehicle)->RenderWaterOutPolys();
	//- rouz edit (ChatGPT)
	// Restore reflection-only helicopter rotor matrices before the main camera continues rendering
	//+ rouz edit (ChatGPT)
	if(reflectionTopRotor){
		RwMatrixCopy(RwFrameGetMatrix(reflectionTopRotor), &savedTopRotorMatrix);
		RwFrameUpdateObjects(reflectionTopRotor);
	}
	if(reflectionBackRotor){
		RwMatrixCopy(RwFrameGetMatrix(reflectionBackRotor), &savedBackRotorMatrix);
		RwFrameUpdateObjects(reflectionBackRotor);
	}
	// Restore the main-view turret matrix after drawing the reflection-only Rhino pose
	//+ rouz edit (ChatGPT)
	if(reflectionRhinoTurret){
		RwMatrixCopy(RwFrameGetMatrix(reflectionRhinoTurret), &savedRhinoTurretMatrix);
		RwFrameUpdateObjects(reflectionRhinoTurret);
	}
	if(reflectionDodoPropeller){
		RwMatrixCopy(RwFrameGetMatrix(reflectionDodoPropeller), &savedDodoPropellerMatrix);
		RwFrameUpdateObjects(reflectionDodoPropeller);
	}
	if(reflectionDodoRudder){
		RwMatrixCopy(RwFrameGetMatrix(reflectionDodoRudder), &savedDodoRudderMatrix);
		RwFrameUpdateObjects(reflectionDodoRudder);
	}
	// Restore the airport chopper's temporary reflection rotor pose
	//+ rouz edit (ChatGPT)
	if(reflectionPlaneTopRotor){
		RwMatrixCopy(RwFrameGetMatrix(reflectionPlaneTopRotor), &savedPlaneTopRotorMatrix);
		RwFrameUpdateObjects(reflectionPlaneTopRotor);
	}
	if(reflectionPlaneBackRotor){
		RwMatrixCopy(RwFrameGetMatrix(reflectionPlaneBackRotor), &savedPlaneBackRotorMatrix);
		RwFrameUpdateObjects(reflectionPlaneBackRotor);
	}
	//- rouz edit (ChatGPT)
	// Restore the automobile helicopter rotor frames after the reflection draw
	//+ rouz edit (ChatGPT)
	for(int rotor = 0; rotor < 4; rotor++)
		if(reflectionRealHeliRotors[rotor]){
			RwMatrixCopy(RwFrameGetMatrix(reflectionRealHeliRotors[rotor]), &savedRealHeliRotorMatrices[rotor]);
			RwFrameUpdateObjects(reflectionRealHeliRotors[rotor]);
		}
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
	vehicle->RemoveLighting(resetLights);
	rw::SetRenderState(rw::CULLMODE, oldCullMode);
	//- rouz edit (ChatGPT)
}
#endif

//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
static void RenderSoftwareReflectionObject(CObject *object)
{
	// Skip missing, hidden, or non-renderable reflection props
	if(!object || !object->m_rwObject || object->bDoNotRender)
		return;
	// Preserve temporary vehicle paint without running object gameplay effects
	if(object->m_nRefModelIndex != -1 && object->ObjectCreatedBy == TEMP_OBJECT && object->bUseVehicleColours){
		CBaseModelInfo *modelInfo = CModelInfo::GetModelInfo(object->m_nRefModelIndex);
		if(modelInfo && modelInfo->GetModelType() == MITYPE_VEHICLE)
			((CVehicleModelInfo*)modelInfo)->SetVehicleColour(object->m_colour1, object->m_colour2);
	}
	// Draw the base entity only so this extra camera pass cannot spawn gameplay particles
	const bool resetLights = object->SetupLighting();
	object->CEntity::Render();
	object->RemoveLighting(resetLights);
}
#endif
//- rouz edit (ChatGPT)

void
CRenderer::RenderSoftwareVehicles(void)
{
#ifdef REVC_SOFTWARE_POLYGONS
	//+ rouz edit (ChatGPT)
	// Gather reflection-camera vehicles independently from the main camera visibility list
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
	if(CustomPipes::bRenderingEnvMap && CustomPipes::EnvMapCam){
		CVehiclePool *vehiclePool = CPools::GetVehiclePool();
		if(vehiclePool)
			for(int i = 0; i < vehiclePool->GetSize(); i++){
				CVehicle *vehicle = vehiclePool->GetSlot(i);
				if(vehicle && !vehicle->IsBoat() && IsVisibleToSoftwareEnvMap(vehicle))
					RenderSoftwareVehicle(vehicle);
			}
		return;
	}
#endif
	//- rouz edit (ChatGPT)
	// Submit visible vehicle clumps and distant atomic LODs through the CPU path
	for(int i = 0; i < ms_nNoOfVisibleVehicles; i++){
		CEntity *entity = ms_aVisibleVehiclePtrs[i];
		if(!entity || !entity->IsVehicle() || !entity->m_rwObject)
			continue;
		// Keep sorted water-touching cars in software reflections while preserving the main-pass split
		//+ rouz edit (ChatGPT)
		CVehicle *vehicle = (CVehicle*)entity;
		bool renderInReflection = false;
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
		renderInReflection = CustomPipes::bRenderingEnvMap;
#endif
		if(vehicle->IsBoat() || (PutIntoSortedVehicleList(vehicle) && !renderInReflection))
			continue;
		RenderSoftwareVehicle(vehicle);
		//- rouz edit (ChatGPT)
	}
#endif
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
template <class PoolType>
static void RegisterSoftwareReflectionLightPool(PoolType *pool)
{
	// Gather light sources from loaded entities visible to the reflection camera
	if(!pool)
		return;
	for(int i = 0; i < pool->GetSize(); i++){
		CEntity *entity = pool->GetSlot(i);
		if(IsVisibleToSoftwareEnvMap(entity))
			CCoronas::ProcessLightsForEnvMap(entity);
	}
}
#endif

void
CRenderer::RegisterSoftwareReflectionLights(void)
{
	// Collect reflection-camera point lights before any geometry is shaded
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	if(!CustomPipes::bRenderingEnvMap || !CustomPipes::EnvMapCam)
		return;
	RegisterSoftwareReflectionLightPool(CPools::GetBuildingPool());
	RegisterSoftwareReflectionLightPool(CPools::GetTreadablePool());
	RegisterSoftwareReflectionLightPool(CPools::GetPedPool());
	RegisterSoftwareReflectionLightPool(CPools::GetVehiclePool());
	RegisterSoftwareReflectionLightPool(CPools::GetObjectPool());
#endif
}

void
CRenderer::RenderSoftwareReflectionEntities(void)
{
	// Render reflection-camera actors and props in one back-to-front order
	PUSH_RENDERGROUP("CRenderer::RenderSoftwareReflectionEntities");
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	if(CustomPipes::bRenderingEnvMap && CustomPipes::EnvMapCam && CustomPipes::EnvMapCam->getFrame()){
		static EntityInfo entities[NUMVISIBLEENTITIES];
		int entityCount = 0;
		const CVector cameraPosition(CustomPipes::EnvMapCam->getFrame()->getLTM()->pos);
		const CVector cameraRight(CustomPipes::EnvMapCam->getFrame()->getLTM()->right);
		CPedPool *pedPool = CPools::GetPedPool();
		CVehiclePool *vehiclePool = CPools::GetVehiclePool();
		CObjectPool *objectPool = CPools::GetObjectPool();
		EntityInfo entityInfo;
		CRegisteredMotionBlurStreak reflectionStreaks[NUMMBLURSTREAKS];
		int reflectionStreakCount = 0;

		// Gather reflection-visible pedestrians that are not rendered as vehicle occupants
		if(pedPool)
			for(int i = 0; i < pedPool->GetSize() && entityCount < NUMVISIBLEENTITIES; i++){
				CPed *ped = pedPool->GetSlot(i);
				if(!ped || ped->m_nPedState == PED_DRIVING || IsSoftwareVehicleOccupant(ped) || !IsVisibleToSoftwareEnvMap(ped)) // rouz edit (ChatGPT)
					continue;
				// Add the pedestrian's ground shadow using reflection-camera visibility
				//+ rouz edit (ChatGPT)
				CustomPipes::StorePedShadowForEnvMap(ped);
				//- rouz edit (ChatGPT)
				entityInfo.ent = ped;
				entityInfo.sort = (cameraPosition - ped->GetPosition()).MagnitudeSqr();
				entities[entityCount++] = entityInfo;
			}

		// Gather reflection-visible cars and aircraft while leaving boats to their dedicated pass
		if(vehiclePool)
			for(int i = 0; i < vehiclePool->GetSize() && entityCount < NUMVISIBLEENTITIES; i++){
				CVehicle *vehicle = vehiclePool->GetSlot(i);
				if(!vehicle || vehicle->IsBoat() || !IsVisibleToSoftwareEnvMap(vehicle))
					continue;
				// Add reflection-only vehicle shadows without changing the main scene queue
				//+ rouz edit (ChatGPT)
				VEH_SHD_TYPE shadowType = VEH_SHD_TYPE_CAR;
				if(vehicle->IsBike())
					shadowType = VEH_SHD_TYPE_BIKE;
				else if(vehicle->IsHeli())
					shadowType = VEH_SHD_TYPE_HELI;
				else if(vehicle->GetModelIndex() == MI_RCBARON)
					shadowType = VEH_SHD_TYPE_RCPLANE;
				// Preserve the helicopter searchlight pool when only the reflection camera sees it
				//+ rouz edit (ChatGPT)
				if(vehicle->IsHeli())
					CustomPipes::StoreHeliSearchLightShadowForEnvMap((CHeli*)vehicle);
				//- rouz edit (ChatGPT)
				// Register shadows for parked vehicles visible only to the reflection camera
				CustomPipes::StoreVehicleShadowForEnvMap(vehicle, shadowType); // rouz edit (ChatGPT)
				// Register the RC Bandit's antenna when its normal pre-render pass missed it
				if(vehicle->GetModelIndex() == MI_RCBANDIT){
					CVector antennaPosition = vehicle->GetMatrix() * CVector(0.218f, -0.444f, 0.391f);
					CAntennas::RegisterOneForEnvMap((uintptr)vehicle, vehicle->GetUp(), antennaPosition, 1.0f);
				}
				//- rouz edit (ChatGPT)
				entityInfo.ent = vehicle;
				entityInfo.sort = (cameraPosition - vehicle->GetPosition()).MagnitudeSqr();
				entities[entityCount++] = entityInfo;
			}

		// Gather reflection-visible props into the same transparency ordering
		if(objectPool)
			for(int i = 0; i < objectPool->GetSize() && entityCount < NUMVISIBLEENTITIES; i++){
				CObject *object = objectPool->GetSlot(i);
				// Let queued underwater objects render before transparent reflection water
				if(IsQueuedForSoftwareUnderwaterFade(object) || !IsVisibleToSoftwareEnvMap(object))
					continue;
				// Rebuild throwable streaks omitted by the main-camera pre-render cull
				if((object->GetModelIndex() == MI_GRENADE || object->GetModelIndex() == MI_MOLOTOV) &&
				   !CMotionBlurStreaks::IsRegisteredForCurrentFrame((uintptr)object) &&
				   reflectionStreakCount < NUMMBLURSTREAKS){
					CVector movement = object->m_vecMoveSpeed * CTimeStep::ms_fTimeStep;
					// Skip stationary throwables that would create a zero-area trail
					if(movement.MagnitudeSqr() > 0.000001f){
						CRegisteredMotionBlurStreak *streak = &reflectionStreaks[reflectionStreakCount++];
						streak->m_id = (uintptr)object;
						streak->m_red = object->GetModelIndex() == MI_MOLOTOV ? 0 : 100;
						streak->m_green = 100;
						streak->m_blue = object->GetModelIndex() == MI_MOLOTOV ? 0 : 100;
						CVector currentPosition = object->GetPosition();
						streak->m_pos1[0] = currentPosition - 0.07f*cameraRight;
						streak->m_pos2[0] = currentPosition + 0.07f*cameraRight;
						CVector previousPosition = currentPosition - movement;
						streak->m_pos1[1] = previousPosition - 0.07f*cameraRight;
						streak->m_pos2[1] = previousPosition + 0.07f*cameraRight;
						streak->m_isValid[0] = true;
						streak->m_isValid[1] = true;
						streak->m_isValid[2] = false;
					}
				}
				// Register pole shadows for objects that only the reflection camera can see
				//+ rouz edit (ChatGPT)
				StoreSoftwareReflectionPoleShadow(object);
				// Preserve the small ground shadow normally registered for beachballs
				if(object->GetModelIndex() == MI_BEACHBALL)
					CustomPipes::StoreBeachBallShadowForEnvMap(object);
				//- rouz edit (ChatGPT)
				entityInfo.ent = object;
				entityInfo.sort = (cameraPosition - object->GetPosition()).MagnitudeSqr();
				entities[entityCount++] = entityInfo;
			}

		// Sort entities in N log N time so dense reflection scenes stay affordable
		qsort(entities, entityCount, sizeof(entities[0]), CompareEntityInfoByDistance);

		// Draw farthest entities first while preserving vehicle occupant handling
		for(int i = entityCount - 1; i >= 0; i--){
			CEntity *entity = entities[i].ent;
			if(entity->IsVehicle())
				RenderSoftwareVehicle((CVehicle*)entity);
			else if(entity->IsObject())
				RenderSoftwareReflectionObject((CObject*)entity);
			else
				RenderOneNonRoad(entity);
		}
		// Draw the local reflection-only projectile trails with depth testing and scene fog
		if(reflectionStreakCount > 0){
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
			RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
			RwRenderStateSet(rwRENDERSTATEFOGCOLOR,
				(void*)RWRGBALONG(CTimeCycle::GetFogRed(), CTimeCycle::GetFogGreen(), CTimeCycle::GetFogBlue(), 255));
			RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
			RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
			RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
			for(int i = 0; i < reflectionStreakCount; i++)
				reflectionStreaks[i].Render();
			// Restore the baseline depth and fog state for following reflection effects
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
			RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
		}
	}
#endif
	POP_RENDERGROUP();
}
//- rouz edit (ChatGPT)

void
CRenderer::RenderTransparentWater(void)
{
	int i;
	CEntity *e;

	PUSH_RENDERGROUP("CRenderer::RenderTransparentWater");
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDZERO);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	SetStencilState(2);

	//+ rouz edit (ChatGPT)
	// Cut reflection water around boats visible only to the environment camera
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
	if(CustomPipes::bRenderingEnvMap && CustomPipes::EnvMapCam){
		CVehiclePool *vehiclePool = CPools::GetVehiclePool();
		if(vehiclePool)
			for(int32 poolIndex = 0; poolIndex < vehiclePool->GetSize(); poolIndex++){
				CVehicle *vehicle = vehiclePool->GetSlot(poolIndex);
				if(vehicle && vehicle->IsBoat() && IsVisibleToSoftwareEnvMap(vehicle))
					((CBoat*)vehicle)->RenderWaterOutPolys();
			}
	}else
#endif
	//- rouz edit (ChatGPT)
	for(i = 0; i < ms_nNoOfVisibleVehicles; i++){
		e = ms_aVisibleVehiclePtrs[i];
		if(e->IsVehicle() && ((CVehicle*)e)->IsBoat())
			((CBoat*)e)->RenderWaterOutPolys();
	}

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	SetStencilState(1);

	CWaterLevel::RenderTransparentWater();

	SetStencilState(0);
	POP_RENDERGROUP();
}

void
CRenderer::ClearForFrame(void)
{
	ms_nNoOfVisibleEntities = 0;
	ms_nNoOfVisibleVehicles = 0;
	ms_nNoOfVisibleBuildings = 0;
	ms_nNoOfInVisibleEntities = 0;
	gSortedVehiclesAndPeds.Clear();

	WorldRender::numBlendInsts[PASS_NOZ] = 0;
	WorldRender::numBlendInsts[PASS_ADD] = 0;
	WorldRender::numBlendInsts[PASS_BLEND] = 0;
}
#endif

void
CRenderer::RenderFadingInEntities(void)
{
	PUSH_RENDERGROUP("CRenderer::RenderFadingInEntities");
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	SetCullMode(rwCULLMODECULLBACK);
	DeActivateDirectional();
	SetAmbientColours();
	CVisibilityPlugins::RenderFadingEntities();
	POP_RENDERGROUP();
}

void
CRenderer::RenderFadingInUnderwaterEntities(void)
{
	PUSH_RENDERGROUP("CRenderer::RenderFadingInUnderwaterEntities");
	DeActivateDirectional();
	SetAmbientColours();
	CVisibilityPlugins::RenderFadingUnderwaterEntities();
	POP_RENDERGROUP();
}

void
CRenderer::RenderCollisionLines(void)
{
	int i;

	// game doesn't draw fading in entities
	// this should probably be fixed
	for(i = 0; i < ms_nNoOfVisibleEntities; i++){
		CEntity *e = ms_aVisibleEntityPtrs[i];
		if(Abs(e->GetPosition().x - ms_vecCameraPosition.x) < 100.0f &&
		   Abs(e->GetPosition().y - ms_vecCameraPosition.y) < 100.0f)
			CCollision::DrawColModel(e->GetMatrix(), *e->GetColModel());
	}
}

enum Visbility
{
	VIS_INVISIBLE,
	VIS_VISIBLE,
	VIS_OFFSCREEN,
	VIS_STREAMME
};

// Time Objects can be time culled if
//   other == -1 || CModelInfo::GetModelInfo(other)->GetRwObject()
// i.e. we have to draw even at the wrong time if
//   other != -1 && CModelInfo::GetModelInfo(other)->GetRwObject() == nil

#define OTHERUNAVAILABLE (other != -1 && CModelInfo::GetModelInfo(other)->GetRwObject() == nil)
#define CANTIMECULL (!OTHERUNAVAILABLE)

int32
CRenderer::SetupEntityVisibility(CEntity *ent)
{
	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(ent->m_modelIndex);
	CTimeModelInfo *ti;
	int32 other;
	float dist;

	bool request = true;
	if(mi->GetModelType() == MITYPE_TIME){
 		ti = (CTimeModelInfo*)mi;
		other = ti->GetOtherTimeModel();
		if(CClock::GetIsTimeInRange(ti->GetTimeOn(), ti->GetTimeOff())){
			// don't fade in, or between time objects
			if(CANTIMECULL)
				ti->m_alpha = 255;
		}else{
			// Hide if possible
			if(CANTIMECULL){
				ent->DeleteRwObject();
				return VIS_INVISIBLE;
			}
			// can't cull, so we'll try to draw this one, but don't request
			// it since what we really want is the other one.
			request = false;
		}
	}else{
		if(mi->GetModelType() != MITYPE_SIMPLE && mi->GetModelType() != MITYPE_WEAPON){
			if(FindPlayerVehicle() == ent &&
			   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON &&
			   !(FindPlayerVehicle()->IsBike() && ((CBike*)FindPlayerVehicle())->bWheelieCam)){
				// Player's vehicle in first person mode
				CVehicle *veh = (CVehicle*)ent;
				int model = veh->GetModelIndex();
				int direction = TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking;
				if(direction == LOOKING_FORWARD ||
				   ent->GetModelIndex() == MI_RHINO ||
				   ent->GetModelIndex() == MI_COACH ||
				   TheCamera.m_bInATunnelAndABigVehicle ||
				   direction == LOOKING_BEHIND && veh->pHandling->Flags & HANDLING_UNKNOWN){
					// rouz edit
					if (rouz.car_first_person)
						return VIS_VISIBLE;
					ent->bNoBrightHeadLights = true;
					return VIS_OFFSCREEN;
				}

				if(direction != LOOKING_BEHIND ||
				   !veh->IsBoat() || model == MI_REEFER || model == MI_TROPIC || model == MI_PREDATOR || model == MI_SKIMMER){
					m_pFirstPersonVehicle = (CVehicle*)ent;
					ent->bNoBrightHeadLights = false;
					return VIS_OFFSCREEN;
				}
			}

			// All sorts of Clumps
			if(ent->m_rwObject == nil || !ent->bIsVisible)
				return VIS_INVISIBLE;
			if(!ent->GetIsOnScreen() || ent->IsEntityOccluded())
				return VIS_OFFSCREEN;
			if(ent->bDrawLast){
				dist = (ent->GetPosition() - ms_vecCameraPosition).Magnitude();
				CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
				ent->bDistanceFade = false;
				return VIS_INVISIBLE;
			}
			return VIS_VISIBLE;
		}
		if(ent->bDontStream){
			if(ent->m_rwObject == nil || !ent->bIsVisible)
				return VIS_INVISIBLE;
			if(!ent->GetIsOnScreen() || ent->IsEntityOccluded())
				return VIS_OFFSCREEN;
			if(ent->bDrawLast){
				dist = (ent->GetPosition() - ms_vecCameraPosition).Magnitude();
				CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
				ent->bDistanceFade = false;
				return VIS_INVISIBLE;
			}
			return VIS_VISIBLE;
		}
	}

	// Simple ModelInfo

	if(!IsAreaVisible(ent->m_area))
		return VIS_INVISIBLE;

	dist = (ent->GetPosition() - ms_vecCameraPosition).Magnitude();

#ifndef FIX_BUGS
	// Whatever this is supposed to do, it breaks fading for objects
	// whose draw dist is > LOD_DISTANCE-FADE_DISTANCE, i.e. 280
	// because decreasing dist here makes the object visible above LOD_DISTANCE
	// before fading normally once below LOD_DISTANCE.
	// aha! this must be a workaround for the fact that we're not taking
	// the LOD multiplier into account here anywhere
	if(LOD_DISTANCE < dist && dist < mi->GetLargestLodDistance() + FADE_DISTANCE)
		dist += mi->GetLargestLodDistance() - LOD_DISTANCE;
#endif

	if(ent->IsObject() && ent->bRenderDamaged)
		mi->m_isDamaged = true;

	//+ rouz edit (ChatGPT)
	float lodDist = GetHighAltitudeLodDistance(ent, mi, dist);
	RpAtomic *a = mi->GetAtomicFromDistance(lodDist);
	//- rouz edit (ChatGPT)

	if(a){
		mi->m_isDamaged = false;
		if(ent->m_rwObject == nil)
			ent->CreateRwObject();
		assert(ent->m_rwObject);
		RpAtomic *rwobj = (RpAtomic*)ent->m_rwObject;
		// Make sure our atomic uses the right geometry and not
		// that of an atomic for another draw distance.
		if(RpAtomicGetGeometry(a) != RpAtomicGetGeometry(rwobj))
			RpAtomicSetGeometry(rwobj, RpAtomicGetGeometry(a), rpATOMICSAMEBOUNDINGSPHERE); // originally 5 (mistake?)
		mi->IncreaseAlpha();
		if(ent->m_rwObject == nil || !ent->bIsVisible)
			return VIS_INVISIBLE;

		if(!ent->GetIsOnScreen() || ent->IsEntityOccluded()){
			mi->m_alpha = 255;
			return VIS_OFFSCREEN;
		}

		if(mi->m_alpha != 255){
			CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
			ent->bDistanceFade = true;
			MarkHighAltitudeStaticMapInstance(ent); // rouz edit (ChatGPT)
			return VIS_INVISIBLE;
		}

		if(mi->m_drawLast || ent->bDrawLast){
			if(CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist)){
				ent->bDistanceFade = false;
				MarkHighAltitudeStaticMapInstance(ent); // rouz edit (ChatGPT)
				return VIS_INVISIBLE;
			}
		}
		return VIS_VISIBLE;
	}

	// Object is not loaded, figure out what to do

	if(mi->m_noFade){
		mi->m_isDamaged = false;
		// request model
		if(lodDist - STREAM_DISTANCE < mi->GetLargestLodDistance() && request) // rouz edit (ChatGPT)
			return VIS_STREAMME;
		return VIS_INVISIBLE;
	}

	// We might be fading

	a = mi->GetAtomicFromDistance(lodDist - FADE_DISTANCE); // rouz edit (ChatGPT)
	mi->m_isDamaged = false;
	if(a == nil){
		// request model
		if(lodDist - FADE_DISTANCE - STREAM_DISTANCE < mi->GetLargestLodDistance() && request) // rouz edit (ChatGPT)
			return VIS_STREAMME;
		return VIS_INVISIBLE;
	}

	if(ent->m_rwObject == nil)
		ent->CreateRwObject();
	assert(ent->m_rwObject);
	RpAtomic *rwobj = (RpAtomic*)ent->m_rwObject;
	if(RpAtomicGetGeometry(a) != RpAtomicGetGeometry(rwobj))
		RpAtomicSetGeometry(rwobj, RpAtomicGetGeometry(a), rpATOMICSAMEBOUNDINGSPHERE); // originally 5 (mistake?)
	mi->IncreaseAlpha();
	if(ent->m_rwObject == nil || !ent->bIsVisible)
		return VIS_INVISIBLE;

	if(!ent->GetIsOnScreen() || ent->IsEntityOccluded()){
		mi->m_alpha = 255;
		return VIS_OFFSCREEN;
	}else{
		CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
		ent->bDistanceFade = true;
		// rouz edit (ChatGPT)
		MarkHighAltitudeStaticMapInstance(ent);
		return VIS_OFFSCREEN;	// Why this?
	}
}

int32
CRenderer::SetupBigBuildingVisibility(CEntity *ent)
{
	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(ent->m_modelIndex);
	CTimeModelInfo *ti;
	int32 other;

	if(!IsAreaVisible(ent->m_area))
		return VIS_INVISIBLE;

	bool request = true;
	if(mi->GetModelType() == MITYPE_TIME){
		ti = (CTimeModelInfo*)mi;
		other = ti->GetOtherTimeModel();
		if(CClock::GetIsTimeInRange(ti->GetTimeOn(), ti->GetTimeOff())){
			// don't fade in, or between time objects
			if(CANTIMECULL)
				ti->m_alpha = 255;
		}else{
			// Hide if possible
			if(CANTIMECULL){
				ent->DeleteRwObject();
				return VIS_INVISIBLE;
			}
			// can't cull, so we'll try to draw this one, but don't request
			// it since what we really want is the other one.
			request = false;
		}
	}else if(mi->GetModelType() == MITYPE_VEHICLE)
		return ent->IsVisible() ? VIS_VISIBLE : VIS_INVISIBLE;

	float dist = (ms_vecCameraPosition-ent->GetPosition()).Magnitude();
	CSimpleModelInfo *nonLOD = mi->GetRelatedModel();

	// rouz edit (ChatGPT)
	if(nonLOD && IsHighAltitudeStaticMapInstanceQueued(nonLOD, ent->GetPosition()))
		return VIS_INVISIBLE;

	// Find out whether to draw below near distance.
	// This is only the case if there is a non-LOD which is either not
	// loaded or not completely faded in yet.
	if(dist < mi->GetNearDistance() && dist < LOD_DISTANCE){
		// No non-LOD or non-LOD is completely visible.
		if(nonLOD == nil ||
		   nonLOD->GetRwObject() && nonLOD->m_alpha == 255)
			return VIS_INVISIBLE;

		// But if it is a time object, we'd rather draw the wrong
		// non-LOD than the right LOD.
		if(nonLOD->GetModelType() == MITYPE_TIME){
			ti = (CTimeModelInfo*)nonLOD;
			other = ti->GetOtherTimeModel();
			if(other != -1 && CModelInfo::GetModelInfo(other)->GetRwObject())
				return VIS_INVISIBLE;
		}
	}

	RpAtomic *a = mi->GetFirstAtomicFromDistance(dist);
	if(a){
		if(ent->m_rwObject == nil)
			ent->CreateRwObject();
		assert(ent->m_rwObject);
		RpAtomic *rwobj = (RpAtomic*)ent->m_rwObject;

		// Make sure our atomic uses the right geometry and not
		// that of an atomic for another draw distance.
		if(RpAtomicGetGeometry(a) != RpAtomicGetGeometry(rwobj))
			RpAtomicSetGeometry(rwobj, RpAtomicGetGeometry(a), rpATOMICSAMEBOUNDINGSPHERE); // originally 5 (mistake?)
		mi->IncreaseAlpha();
		if(!ent->IsVisible() || !ent->GetIsOnScreenComplex() || ent->IsEntityOccluded()){
			mi->m_alpha = 255;
			return VIS_INVISIBLE;
		}

		if(mi->m_alpha != 255){
			CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
			ent->bDistanceFade = true;
			return VIS_INVISIBLE;
		}

		if(mi->m_drawLast){
			CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
			ent->bDistanceFade = false;
			return VIS_INVISIBLE;
		}
		return VIS_VISIBLE;
	}

	if(mi->m_noFade){
		ent->DeleteRwObject();
		return VIS_INVISIBLE;
	}


	// get faded atomic
	a = mi->GetFirstAtomicFromDistance(dist - FADE_DISTANCE);
	if(a == nil){
		if(ent->bStreamBIGBuilding && dist-STREAM_DISTANCE < mi->GetLodDistance(0) && request){
			return ent->GetIsOnScreen() ? VIS_STREAMME : VIS_INVISIBLE;
		}else{
			ent->DeleteRwObject();
			return VIS_INVISIBLE;
		}
	}

	// Fade...
	if(ent->m_rwObject == nil)
		ent->CreateRwObject();
	assert(ent->m_rwObject);
	RpAtomic *rwobj = (RpAtomic*)ent->m_rwObject;
	if(RpAtomicGetGeometry(a) != RpAtomicGetGeometry(rwobj))
		RpAtomicSetGeometry(rwobj, RpAtomicGetGeometry(a), rpATOMICSAMEBOUNDINGSPHERE); // originally 5 (mistake?)
	mi->IncreaseAlpha();
	if(!ent->IsVisible() || !ent->GetIsOnScreenComplex() || ent->IsEntityOccluded()){
		mi->m_alpha = 255;
		return VIS_INVISIBLE;
	}
	CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
	ent->bDistanceFade = true;
	return VIS_INVISIBLE;
}

void
CRenderer::ConstructRenderList(void)
{
	COcclusion::ProcessBeforeRendering();
#ifdef NEW_RENDERER
	if(!gbNewRenderer)
#endif
{
	ms_nNoOfVisibleEntities = 0;
	ms_nNoOfInVisibleEntities = 0;
}
	ms_vecCameraPosition = TheCamera.GetPosition();
	//+ rouz edit (ChatGPT)
	gNumHighAltitudeStaticMapInstances = 0;
	gbUseHighAltitudeStaticMapDistance = false;
	//- rouz edit (ChatGPT)

	// unused
	pFullBlockedRanges = nil;
	pEmptyBlockedRanges = aBlockedRanges;
	for(int i = 0; i < 16; i++){
		aBlockedRanges[i].prev = &aBlockedRanges[i-1];
		aBlockedRanges[i].next = &aBlockedRanges[i+1];
	}
	aBlockedRanges[0].prev = nil;
	aBlockedRanges[15].next = nil;

	// unused
	TestCloseThings = 0;
	TestBigThings = 0;

	ScanWorld();
}

void
LimitFrustumVector(CVector &vec1, const CVector &vec2, float l)
{
	float f;
	f = (l - vec2.z) / (vec1.z - vec2.z);
	vec1.x = f*(vec1.x - vec2.x) + vec2.x;
	vec1.y = f*(vec1.y - vec2.y) + vec2.y;
	vec1.z = f*(vec1.z - vec2.z) + vec2.z;
}

enum Corners
{
	CORNER_CAM = 0,
	CORNER_FAR_TOPLEFT,
	CORNER_FAR_TOPRIGHT,
	CORNER_FAR_BOTRIGHT,
	CORNER_FAR_BOTLEFT,
//+ rouz edit (ChatGPT)
	CORNER_LOD_TOPLEFT,
	CORNER_LOD_TOPRIGHT,
	CORNER_LOD_BOTRIGHT,
	CORNER_LOD_BOTLEFT,
	CORNER_PRIO_TOPLEFT,
	CORNER_PRIO_TOPRIGHT,
	CORNER_PRIO_BOTRIGHT,
	CORNER_PRIO_BOTLEFT,
	CORNER_COUNT,
};

static void
SetSectorPolyPoint(RwV2d &point, const CVector &coors)
{
	point.x = CWorld::GetSectorX(coors.x);
	point.y = CWorld::GetSectorY(coors.y);
}

static void
ScanSectorFrustum(CVector *vectors, int32 firstCorner, void (*scanfunc)(CPtrList *))
{
	RwV2d poly[3];
	int32 i;

	SetSectorPolyPoint(poly[0], vectors[CORNER_CAM]);
	for(i = 0; i < 4; i++){
		SetSectorPolyPoint(poly[1], vectors[firstCorner + i]);
		SetSectorPolyPoint(poly[2], vectors[firstCorner + ((i + 1) & 3)]);
		CRenderer::ScanSectorPoly(poly, 3, scanfunc);
	}
}
//- rouz edit (ChatGPT)

void
CRenderer::ScanWorld(void)
{
	float f = RwCameraGetFarClipPlane(TheCamera.m_pRwCamera);
	RwV2d vw = *RwCameraGetViewWindow(TheCamera.m_pRwCamera);
	CVector vectors[CORNER_COUNT]; // rouz edit (ChatGPT)
	RwMatrix *cammatrix;

	memset(vectors, 0, sizeof(vectors));
	vectors[CORNER_FAR_TOPLEFT].x = -vw.x * f;
	vectors[CORNER_FAR_TOPLEFT].y = vw.y * f;
	vectors[CORNER_FAR_TOPLEFT].z = f;
	vectors[CORNER_FAR_TOPRIGHT].x = vw.x * f;
	vectors[CORNER_FAR_TOPRIGHT].y = vw.y * f;
	vectors[CORNER_FAR_TOPRIGHT].z = f;
	vectors[CORNER_FAR_BOTRIGHT].x = vw.x * f;
	vectors[CORNER_FAR_BOTRIGHT].y = -vw.y * f;
	vectors[CORNER_FAR_BOTRIGHT].z = f;
	vectors[CORNER_FAR_BOTLEFT].x = -vw.x * f;
	vectors[CORNER_FAR_BOTLEFT].y = -vw.y * f;
	vectors[CORNER_FAR_BOTLEFT].z = f;

	cammatrix = RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera));

	m_pFirstPersonVehicle = nil;
	CVisibilityPlugins::InitAlphaEntityList();
	CWorld::AdvanceCurrentScanCode();

	// unused
	static CVector prevPos;
	static CVector prevFwd;
	static bool smallMovement;
	smallMovement = (TheCamera.GetPosition() - prevPos).MagnitudeSqr() < SQR(4.0f) &&
		DotProduct(TheCamera.GetForward(), prevFwd) > 0.98f;
	prevPos = TheCamera.GetPosition();
	prevFwd = TheCamera.GetForward();

	//+ rouz edit (ChatGPT)
	vectors[CORNER_LOD_TOPLEFT] = vectors[CORNER_FAR_TOPLEFT] * LOD_DISTANCE/f;
	vectors[CORNER_LOD_TOPRIGHT] = vectors[CORNER_FAR_TOPRIGHT] * LOD_DISTANCE/f;
	vectors[CORNER_LOD_BOTRIGHT] = vectors[CORNER_FAR_BOTRIGHT] * LOD_DISTANCE/f;
	vectors[CORNER_LOD_BOTLEFT] = vectors[CORNER_FAR_BOTLEFT] * LOD_DISTANCE/f;

	for(int32 i = 0; i < 4; i++){
		vectors[CORNER_PRIO_TOPLEFT + i].x = vectors[CORNER_LOD_TOPLEFT + i].x * 0.2f;
		vectors[CORNER_PRIO_TOPLEFT + i].y = vectors[CORNER_LOD_TOPLEFT + i].y * 0.2f;
		vectors[CORNER_PRIO_TOPLEFT + i].z = vectors[CORNER_LOD_TOPLEFT + i].z;
	}
	RwV3dTransformPoints(vectors, vectors, ARRAY_SIZE(vectors), cammatrix);
	//- rouz edit (ChatGPT)

	m_loadingPriority = false;
	if(TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN ||
#ifdef FIX_BUGS
	   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_GTACLASSIC ||
#endif
	   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED){
		CRect rect;
		int x1, x2, y1, y2;
		LimitFrustumVector(vectors[CORNER_FAR_TOPLEFT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_TOPLEFT]);
		LimitFrustumVector(vectors[CORNER_FAR_TOPRIGHT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_TOPRIGHT]);
		LimitFrustumVector(vectors[CORNER_FAR_BOTRIGHT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_BOTRIGHT]);
		LimitFrustumVector(vectors[CORNER_FAR_BOTLEFT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_BOTLEFT]);
		x1 = CWorld::GetSectorIndexX(rect.left);
		if(x1 < 0) x1 = 0;
		x2 = CWorld::GetSectorIndexX(rect.right);
		if(x2 >= NUMSECTORS_X-1) x2 = NUMSECTORS_X-1;
		y1 = CWorld::GetSectorIndexY(rect.top);
		if(y1 < 0) y1 = 0;
		y2 = CWorld::GetSectorIndexY(rect.bottom);
		if(y2 >= NUMSECTORS_Y-1) y2 = NUMSECTORS_Y-1;
		for(; x1 <= x2; x1++)
			for(int y = y1; y <= y2; y++)
				ScanSectorList(CWorld::GetSector(x1, y)->m_lists);
	}else{
#ifdef GTA_TRAIN
		CVehicle *train = FindPlayerTrain();
		if(train && train->GetPosition().z < 0.0f){
			// rouz edit (ChatGPT)
			ScanSectorFrustum(vectors, CORNER_LOD_TOPLEFT, ScanSectorList_Subway);
		}else
#endif
		{
			//+ rouz edit (ChatGPT)
			if(f > LOD_DISTANCE){
				// priority
				ScanSectorFrustum(vectors, CORNER_PRIO_TOPLEFT, ScanSectorList_Priority);

				// below LOD
				ScanSectorFrustum(vectors, CORNER_LOD_TOPLEFT, ScanSectorList);

				// rouz edit (ChatGPT)
				if(ShouldScanFarStaticMapEntities())
					ScanSectorFrustum(vectors, CORNER_FAR_TOPLEFT, ScanSectorList_FarStaticMapEntities);
			}else{
				ScanSectorFrustum(vectors, CORNER_FAR_TOPLEFT, ScanSectorList);
			}
			//- rouz edit (ChatGPT)
			
#ifdef NO_ISLAND_LOADING
			if (FrontEndMenuManager.m_PrefsIslandLoading == CMenuManager::ISLAND_LOADING_HIGH) {
				ScanBigBuildingList(CWorld::GetBigBuildingList(LEVEL_BEACH));
				ScanBigBuildingList(CWorld::GetBigBuildingList(LEVEL_MAINLAND));
			} else 
#endif
			{
#ifdef FIX_BUGS
			if(CCollision::ms_collisionInMemory != LEVEL_GENERIC)
#endif
				ScanBigBuildingList(CWorld::GetBigBuildingList(CGame::currLevel));
			}
			ScanBigBuildingList(CWorld::GetBigBuildingList(LEVEL_GENERIC));
		}
	}
	ScanFarAwayVehicles(); // rouz edit (ChatGPT)
}

void
CRenderer::RequestObjectsInFrustum(void)
{
	float f = RwCameraGetFarClipPlane(TheCamera.m_pRwCamera);
	RwV2d vw = *RwCameraGetViewWindow(TheCamera.m_pRwCamera);
	CVector vectors[CORNER_COUNT]; // rouz edit (ChatGPT)
	RwMatrix *cammatrix;

	memset(vectors, 0, sizeof(vectors));
	vectors[CORNER_FAR_TOPLEFT].x = -vw.x * f;
	vectors[CORNER_FAR_TOPLEFT].y = vw.y * f;
	vectors[CORNER_FAR_TOPLEFT].z = f;
	vectors[CORNER_FAR_TOPRIGHT].x = vw.x * f;
	vectors[CORNER_FAR_TOPRIGHT].y = vw.y * f;
	vectors[CORNER_FAR_TOPRIGHT].z = f;
	vectors[CORNER_FAR_BOTRIGHT].x = vw.x * f;
	vectors[CORNER_FAR_BOTRIGHT].y = -vw.y * f;
	vectors[CORNER_FAR_BOTRIGHT].z = f;
	vectors[CORNER_FAR_BOTLEFT].x = -vw.x * f;
	vectors[CORNER_FAR_BOTLEFT].y = -vw.y * f;
	vectors[CORNER_FAR_BOTLEFT].z = f;

	cammatrix = RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera));

	CWorld::AdvanceCurrentScanCode();
	ms_vecCameraPosition = TheCamera.GetPosition();

	//+ rouz edit (ChatGPT)
	vectors[CORNER_LOD_TOPLEFT] = vectors[CORNER_FAR_TOPLEFT] * LOD_DISTANCE/f;
	vectors[CORNER_LOD_TOPRIGHT] = vectors[CORNER_FAR_TOPRIGHT] * LOD_DISTANCE/f;
	vectors[CORNER_LOD_BOTRIGHT] = vectors[CORNER_FAR_BOTRIGHT] * LOD_DISTANCE/f;
	vectors[CORNER_LOD_BOTLEFT] = vectors[CORNER_FAR_BOTLEFT] * LOD_DISTANCE/f;

	for(int32 i = 0; i < 4; i++){
		vectors[CORNER_PRIO_TOPLEFT + i].x = vectors[CORNER_LOD_TOPLEFT + i].x * 0.2f;
		vectors[CORNER_PRIO_TOPLEFT + i].y = vectors[CORNER_LOD_TOPLEFT + i].y * 0.2f;
		vectors[CORNER_PRIO_TOPLEFT + i].z = vectors[CORNER_LOD_TOPLEFT + i].z;
	}
	RwV3dTransformPoints(vectors, vectors, ARRAY_SIZE(vectors), cammatrix);
	//- rouz edit (ChatGPT)

	if(TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN ||
#ifdef FIX_BUGS
	   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_GTACLASSIC ||
#endif
	   TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED){
		CRect rect;
		int x1, x2, y1, y2;
		LimitFrustumVector(vectors[CORNER_FAR_TOPLEFT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_TOPLEFT]);
		LimitFrustumVector(vectors[CORNER_FAR_TOPRIGHT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_TOPRIGHT]);
		LimitFrustumVector(vectors[CORNER_FAR_BOTRIGHT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_BOTRIGHT]);
		LimitFrustumVector(vectors[CORNER_FAR_BOTLEFT], vectors[CORNER_CAM], -100.0f);
		rect.ContainPoint(vectors[CORNER_FAR_BOTLEFT]);
		x1 = CWorld::GetSectorIndexX(rect.left);
		if(x1 < 0) x1 = 0;
		x2 = CWorld::GetSectorIndexX(rect.right);
		if(x2 >= NUMSECTORS_X-1) x2 = NUMSECTORS_X-1;
		y1 = CWorld::GetSectorIndexY(rect.top);
		if(y1 < 0) y1 = 0;
		y2 = CWorld::GetSectorIndexY(rect.bottom);
		if(y2 >= NUMSECTORS_Y-1) y2 = NUMSECTORS_Y-1;
		for(; x1 <= x2; x1++)
			for(int y = y1; y <= y2; y++)
				ScanSectorList_RequestModels(CWorld::GetSector(x1, y)->m_lists);
	}else{
		//+ rouz edit (ChatGPT)
		if(f > LOD_DISTANCE){
			ScanSectorFrustum(vectors, CORNER_LOD_TOPLEFT, ScanSectorList_RequestModels);
			if(ShouldScanFarStaticMapEntities())
				ScanSectorFrustum(vectors, CORNER_FAR_TOPLEFT, ScanSectorList_RequestFarStaticMapEntityModels);
		}else{
			ScanSectorFrustum(vectors, CORNER_FAR_TOPLEFT, ScanSectorList_RequestModels);
		}
		//- rouz edit (ChatGPT)
	}
	ScanFarAwayVehicleModels(); // rouz edit (ChatGPT)
}

bool
CEntity::SetupLighting(void)
{
	return false;
}

void
CEntity::RemoveLighting(bool)
{
}

bool
CPed::SetupLighting(void)
{
	ActivateDirectional();
	SetAmbientColoursForPedsCarsAndObjects();

#ifndef MASTER
	// Originally this was being called through iteration of Sectors, but putting it here is better.
	if (GetDebugDisplay() != 0 && !IsPlayer())
		DebugRenderOnePedText();
#endif

	if (bRenderScorched) {
		WorldReplaceNormalLightsWithScorched(Scene.world, 0.1f);
	} else {
		// Note that this lightMult is only affected by LIGHT_DARKEN. If there's no LIGHT_DARKEN, it will be 1.0.
		float lightMult = CPointLights::GenerateLightsAffectingObject(&GetPosition());
		if (lightMult != 1.0f) {
			SetAmbientAndDirectionalColours(lightMult);
			return true;
		}
	}
	return false;
}

void
CPed::RemoveLighting(bool reset)
{
	if (!bRenderScorched) {
		CRenderer::RemoveVehiclePedLights(this, reset);
		if (reset)
			ReSetAmbientAndDirectionalColours();
	}
	SetAmbientColours();
	DeActivateDirectional();
}

float
CalcNewDelta(RwV2d *a, RwV2d *b)
{
	return (b->x - a->x) / (b->y - a->y);
}

#ifdef FIX_BUGS
#define TOINT(x) ((int)Floor(x))
#else
#define TOINT(x) ((int)(x))
#endif

void
CRenderer::ScanSectorPoly(RwV2d *poly, int32 numVertices, void (*scanfunc)(CPtrList *))
{
	float miny, maxy;
	int y, yend;
	int x, xstart, xend;
	int i;
	int a1, a2, b1, b2;
	float deltaA, deltaB;
	float xA, xB;

	miny = poly[0].y;
	maxy = poly[0].y;
	a2 = 0;
	xstart = 9999;
	xend = -9999;

	for(i = 1; i < numVertices; i++){
		if(poly[i].y > maxy)
			maxy = poly[i].y;
		if(poly[i].y < miny){
			miny = poly[i].y;
			a2 = i;
		}
	}
	y = TOINT(miny);
	yend = TOINT(maxy);

	// Go left in poly to find first edge b
	b2 = a2;
	for(i = 0; i < numVertices; i++){
		b1 = b2--;
		if(b2 < 0) b2 = numVertices-1;
		if(poly[b1].x < xstart)
			xstart = TOINT(poly[b1].x);
		if(TOINT(poly[b1].y) != TOINT(poly[b2].y))
			break;
	}
	// Go right to find first edge a
	for(i = 0; i < numVertices; i++){
		a1 = a2++;
		if(a2 == numVertices) a2 = 0;
		if(poly[a1].x > xend)
			xend = TOINT(poly[a1].x);
		if(TOINT(poly[a1].y) != TOINT(poly[a2].y))
			break;
	}

	// prestep x1 and x2 to next integer y
	deltaA = CalcNewDelta(&poly[a1], &poly[a2]);
	xA = deltaA * (Ceil(poly[a1].y) - poly[a1].y) + poly[a1].x;
	deltaB = CalcNewDelta(&poly[b1], &poly[b2]);
	xB = deltaB * (Ceil(poly[b1].y) - poly[b1].y) + poly[b1].x;

	if(y != yend){
		if(deltaB < 0.0f && TOINT(xB) < xstart)
			xstart = TOINT(xB);
		if(deltaA >= 0.0f && TOINT(xA) > xend)
			xend = TOINT(xA);
	}

	while(y <= yend && y < NUMSECTORS_Y){
		// scan one x-line
		if(y >= 0 && xstart < NUMSECTORS_X)
			for(x = xstart; x <= xend && x != NUMSECTORS_X; x++)
				if(x >= 0)
					scanfunc(CWorld::GetSector(x, y)->m_lists);

		// advance one scan line
		y++;
		xA += deltaA;
		xB += deltaB;

		// update left side
		if(y == TOINT(poly[b2].y)){
			// reached end of edge
			if(y == yend){
				if(deltaB < 0.0f){
					do{
						xstart = TOINT(poly[b2--].x);
						if(b2 < 0) b2 = numVertices-1;
					}while(xstart > TOINT(poly[b2].x));
				}else
					xstart = TOINT(xB - deltaB);
			}else{
				// switch edges
				if(deltaB < 0.0f)
					xstart = TOINT(poly[b2].x);
				else
					xstart = TOINT(xB - deltaB);
				do{
					b1 = b2--;
					if(b2 < 0) b2 = numVertices-1;
					if(TOINT(poly[b1].x) < xstart)
						xstart = TOINT(poly[b1].x);
				}while(y == TOINT(poly[b2].y));
				deltaB = CalcNewDelta(&poly[b1], &poly[b2]);
				xB = deltaB * (Ceil(poly[b1].y) - poly[b1].y) + poly[b1].x;
				if(deltaB < 0.0f && TOINT(xB) < xstart)
					xstart = TOINT(xB);
			}
		}else{
			if(deltaB < 0.0f)
				xstart = TOINT(xB);
			else
				xstart = TOINT(xB - deltaB);
		}

		// update right side
		if(y == TOINT(poly[a2].y)){
			// reached end of edge
			if(y == yend){
				if(deltaA < 0.0f)
					xend = TOINT(xA - deltaA);
				else{
					do{
						xend = TOINT(poly[a2++].x);
						if(a2 == numVertices) a2 = 0;
					}while(xend < TOINT(poly[a2].x));
				}
			}else{
				// switch edges
				if(deltaA < 0.0f)
					xend = TOINT(xA - deltaA);
				else
					xend = TOINT(poly[a2].x);
				do{
					a1 = a2++;
					if(a2 == numVertices) a2 = 0;
					if(TOINT(poly[a1].x) > xend)
						xend = TOINT(poly[a1].x);
				}while(y == TOINT(poly[a2].y));
				deltaA = CalcNewDelta(&poly[a1], &poly[a2]);
				xA = deltaA * (Ceil(poly[a1].y) - poly[a1].y) + poly[a1].x;
				if(deltaA >= 0.0f && TOINT(xA) > xend)
					xend = TOINT(xA);
			}
		}else{
			if(deltaA < 0.0f)
				xend = TOINT(xA - deltaA);
			else
				xend = TOINT(xA);
		}
	}
}

void
CRenderer::InsertEntityIntoList(CEntity *ent)
{
#ifdef FIX_BUGS
	if (!ent->m_rwObject) return;
#endif

#ifdef NEW_RENDERER
	// TODO: there are more flags being checked here
	//+ rouz edit (ChatGPT)
	if(gbNewRenderer && (ent->IsVehicle() || ent->IsPed())){
		if(ms_nNoOfVisibleVehicles >= NUMVISIBLEENTITIES)
			return;
		ms_aVisibleVehiclePtrs[ms_nNoOfVisibleVehicles++] = ent;
	}else if(gbNewRenderer && ent->IsBuilding()){
		if(ms_nNoOfVisibleBuildings >= NUMVISIBLEENTITIES)
			return;
		ms_aVisibleBuildingPtrs[ms_nNoOfVisibleBuildings++] = ent;
	}else
#endif
	{
		if(ms_nNoOfVisibleEntities >= NUMVISIBLEENTITIES)
			return;
		ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities++] = ent;
	}
	//- rouz edit (ChatGPT)
}

void
CRenderer::ScanBigBuildingList(CPtrList &list)
{
	CPtrNode *node;
	CEntity *ent;
	int vis;

	int f = CTimer::GetFrameCounter() & 3;
	for(node = list.first; node; node = node->next){
		ent = (CEntity*)node->item;
		if(ent->bOffscreen || (ent->m_randomSeed&3) != f){
			ent->bOffscreen = true;
			vis = SetupBigBuildingVisibility(ent);
		}else
			vis = VIS_VISIBLE;
		switch(vis){
		case VIS_VISIBLE:
			InsertEntityIntoList(ent);
			ent->bOffscreen = false;
			break;
		case VIS_STREAMME:
			if(!CStreaming::ms_disableStreaming)
				CStreaming::RequestModel(ent->GetModelIndex(), 0);
			break;
		}
	}
}

//+ rouz edit (ChatGPT)
void
CRenderer::ScanFarAwayVehicles(void)
{
	CVehiclePool *vehiclePool = CPools::GetVehiclePool();
	CEntity *ent;
	int vis;

	if(vehiclePool == nil)
		return;

	for(int i = 0; i < vehiclePool->GetSize(); i++){
		ent = vehiclePool->GetSlot(i);
		if(ent == nil ||
		   !ent->bDrawFarAway ||
		   ent->m_scanCode == CWorld::GetCurrentScanCode())
			continue;

		ent->m_scanCode = CWorld::GetCurrentScanCode();
		vis = SetupBigBuildingVisibility(ent);
		switch(vis){
		case VIS_VISIBLE:
			InsertEntityIntoList(ent);
			ent->bOffscreen = false;
			break;
		case VIS_STREAMME:
			if(!CStreaming::ms_disableStreaming)
				CStreaming::RequestModel(ent->GetModelIndex(), 0);
			break;
		}
	}
}
//- rouz edit (ChatGPT)

void
CRenderer::ScanSectorList(CPtrList *lists)
{
	CPtrNode *node;
	CPtrList *list;
	CEntity *ent;
	int i;
	float dx, dy;

	for(i = 0; i < NUMSECTORENTITYLISTS; i++){
		list = &lists[i];
		for(node = list->first; node; node = node->next){
			ent = (CEntity*)node->item;
			if(ent->m_scanCode == CWorld::GetCurrentScanCode())
				continue;	// already seen
			ent->m_scanCode = CWorld::GetCurrentScanCode();
			ent->bOffscreen = false;

			switch(SetupEntityVisibility(ent)){
			case VIS_VISIBLE:
				InsertEntityIntoList(ent);
				break;
			case VIS_INVISIBLE:
				if(!IsGlass(ent->GetModelIndex()))
					break;
				// fall through
			case VIS_OFFSCREEN:
				ent->bOffscreen = true;
				dx = ms_vecCameraPosition.x - ent->GetPosition().x;
				dy = ms_vecCameraPosition.y - ent->GetPosition().y;
				if(dx > -30.0f && dx < 30.0f &&
				   dy > -30.0f && dy < 30.0f &&
				   ms_nNoOfInVisibleEntities < NUMINVISIBLEENTITIES - 1)
					ms_aInVisibleEntityPtrs[ms_nNoOfInVisibleEntities++] = ent;
				break;
			case VIS_STREAMME:
				if(!CStreaming::ms_disableStreaming)
					if(!m_loadingPriority || CStreaming::ms_numModelsRequested < 10)
						CStreaming::RequestModel(ent->GetModelIndex(), 0);
				break;
			}
		}
	}
}

void
CRenderer::ScanSectorList_Priority(CPtrList *lists)
{
	CPtrNode *node;
	CPtrList *list;
	CEntity *ent;
	int i;
	float dx, dy;

	for(i = 0; i < NUMSECTORENTITYLISTS; i++){
		list = &lists[i];
		for(node = list->first; node; node = node->next){
			ent = (CEntity*)node->item;
			if(ent->m_scanCode == CWorld::GetCurrentScanCode())
				continue;	// already seen
			ent->m_scanCode = CWorld::GetCurrentScanCode();
			ent->bOffscreen = false;

			switch(SetupEntityVisibility(ent)){
			case VIS_VISIBLE:
				InsertEntityIntoList(ent);
				break;
			case VIS_INVISIBLE:
				if(!IsGlass(ent->GetModelIndex()))
					break;
				// fall through
			case VIS_OFFSCREEN:
				ent->bOffscreen = true;
				dx = ms_vecCameraPosition.x - ent->GetPosition().x;
				dy = ms_vecCameraPosition.y - ent->GetPosition().y;
				if(dx > -30.0f && dx < 30.0f &&
				   dy > -30.0f && dy < 30.0f &&
				   ms_nNoOfInVisibleEntities < NUMINVISIBLEENTITIES - 1)
					ms_aInVisibleEntityPtrs[ms_nNoOfInVisibleEntities++] = ent;
				break;
			case VIS_STREAMME:
				if(!CStreaming::ms_disableStreaming){
					CStreaming::RequestModel(ent->GetModelIndex(), 0);
					if(CStreaming::ms_aInfoForModel[ent->GetModelIndex()].m_loadState != STREAMSTATE_LOADED)
						m_loadingPriority = true;
				}
				break;
			}
		}
	}
}

//+ rouz edit (ChatGPT)
void
CRenderer::ScanSectorList_FarStaticMapEntities(CPtrList *lists)
{
	CPtrNode *node;
	CPtrList *list;
	CEntity *ent;
	int i;
	static int32 listIDs[] = {
		ENTITYLIST_BUILDINGS,
		ENTITYLIST_BUILDINGS_OVERLAP,
		ENTITYLIST_OBJECTS,
		ENTITYLIST_OBJECTS_OVERLAP,
		ENTITYLIST_DUMMIES,
		ENTITYLIST_DUMMIES_OVERLAP,
	};
	float dx, dy;

	for(i = 0; i < ARRAY_SIZE(listIDs); i++){
		list = &lists[listIDs[i]];
		for(node = list->first; node; node = node->next){
			ent = (CEntity*)node->item;
			if(ent->m_scanCode == CWorld::GetCurrentScanCode())
				continue;	// already seen
			if(!ShouldForceHighAltitudeStaticMapEntity(ent))
				continue;
			ent->m_scanCode = CWorld::GetCurrentScanCode();
			ent->bOffscreen = false;

			gbUseHighAltitudeStaticMapDistance = true;
			switch(SetupEntityVisibility(ent)){
			case VIS_VISIBLE:
				MarkHighAltitudeStaticMapInstance(ent);
				InsertEntityIntoList(ent);
				break;
			case VIS_INVISIBLE:
				if(!IsGlass(ent->GetModelIndex()))
					break;
				// fall through
			case VIS_OFFSCREEN:
				ent->bOffscreen = true;
				dx = ms_vecCameraPosition.x - ent->GetPosition().x;
				dy = ms_vecCameraPosition.y - ent->GetPosition().y;
				if(dx > -30.0f && dx < 30.0f &&
				   dy > -30.0f && dy < 30.0f &&
				   ms_nNoOfInVisibleEntities < NUMINVISIBLEENTITIES - 1)
					ms_aInVisibleEntityPtrs[ms_nNoOfInVisibleEntities++] = ent;
				break;
			case VIS_STREAMME:
				if(!CStreaming::ms_disableStreaming)
					if(!m_loadingPriority || CStreaming::ms_numModelsRequested < 10)
						CStreaming::RequestModel(ent->GetModelIndex(), 0);
				break;
			}
			gbUseHighAltitudeStaticMapDistance = false;
		}
	}
}
//- rouz edit (ChatGPT)

#ifdef GTA_TRAIN
void
CRenderer::ScanSectorList_Subway(CPtrList *lists)
{
	CPtrNode *node;
	CPtrList *list;
	CEntity *ent;
	int i;
	float dx, dy;

	for(i = 0; i < NUMSECTORENTITYLISTS; i++){
		list = &lists[i];
		for(node = list->first; node; node = node->next){
			ent = (CEntity*)node->item;
			if(ent->m_scanCode == CWorld::GetCurrentScanCode())
				continue;	// already seen
			ent->m_scanCode = CWorld::GetCurrentScanCode();
			ent->bOffscreen = false;
			switch(SetupEntityVisibility(ent)){
			case VIS_VISIBLE:
				InsertEntityIntoList(ent);
				break;
			case VIS_OFFSCREEN:
				ent->bOffscreen = true;
				dx = ms_vecCameraPosition.x - ent->GetPosition().x;
				dy = ms_vecCameraPosition.y - ent->GetPosition().y;
				if(dx > -30.0f && dx < 30.0f &&
				   dy > -30.0f && dy < 30.0f &&
				   ms_nNoOfInVisibleEntities < NUMINVISIBLEENTITIES - 1)
					ms_aInVisibleEntityPtrs[ms_nNoOfInVisibleEntities++] = ent;
				break;
			}
		}
	}
}
#endif

void
CRenderer::ScanSectorList_RequestModels(CPtrList *lists)
{
	CPtrNode *node;
	CPtrList *list;
	CEntity *ent;
	int i;

	for(i = 0; i < NUMSECTORENTITYLISTS; i++){
		list = &lists[i];
		for(node = list->first; node; node = node->next){
			ent = (CEntity*)node->item;
			if(ent->m_scanCode == CWorld::GetCurrentScanCode())
				continue;	// already seen
			ent->m_scanCode = CWorld::GetCurrentScanCode();
			if(ShouldModelBeStreamed(ent, ms_vecCameraPosition))
				CStreaming::RequestModel(ent->GetModelIndex(), 0);
		}
	}
}

//+ rouz edit (ChatGPT)
void
CRenderer::ScanFarAwayVehicleModels(void)
{
	CVehiclePool *vehiclePool = CPools::GetVehiclePool();
	CEntity *ent;

	if(vehiclePool == nil)
		return;

	for(int i = 0; i < vehiclePool->GetSize(); i++){
		ent = vehiclePool->GetSlot(i);
		if(ent == nil ||
		   !ent->bDrawFarAway ||
		   ent->m_scanCode == CWorld::GetCurrentScanCode())
			continue;

		ent->m_scanCode = CWorld::GetCurrentScanCode();
		if(ShouldModelBeStreamed(ent, ms_vecCameraPosition))
			CStreaming::RequestModel(ent->GetModelIndex(), 0);
	}
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
void
CRenderer::ScanSectorList_RequestFarStaticMapEntityModels(CPtrList *lists)
{
	CPtrNode *node;
	CPtrList *list;
	CEntity *ent;
	int i;
	static int32 listIDs[] = {
		ENTITYLIST_BUILDINGS,
		ENTITYLIST_BUILDINGS_OVERLAP,
		ENTITYLIST_OBJECTS,
		ENTITYLIST_OBJECTS_OVERLAP,
		ENTITYLIST_DUMMIES,
		ENTITYLIST_DUMMIES_OVERLAP,
	};

	for(i = 0; i < ARRAY_SIZE(listIDs); i++){
		list = &lists[listIDs[i]];
		for(node = list->first; node; node = node->next){
			ent = (CEntity*)node->item;
			if(ent->m_scanCode == CWorld::GetCurrentScanCode())
				continue;	// already seen
			if(!ShouldForceHighAltitudeStaticMapEntity(ent))
				continue;
			ent->m_scanCode = CWorld::GetCurrentScanCode();

			gbUseHighAltitudeStaticMapDistance = true;
			if(ShouldModelBeStreamed(ent, ms_vecCameraPosition))
				CStreaming::RequestModel(ent->GetModelIndex(), 0);
			gbUseHighAltitudeStaticMapDistance = false;
		}
	}
}
//- rouz edit (ChatGPT)

// Put big buildings in front
// This seems pointless because the sector lists shouldn't have big buildings in the first place
void
CRenderer::SortBIGBuildings(void)
{
	int x, y;
	for(y = 0; y < NUMSECTORS_Y; y++)
		for(x = 0; x < NUMSECTORS_X; x++){
			SortBIGBuildingsForSectorList(&CWorld::GetSector(x, y)->m_lists[ENTITYLIST_BUILDINGS]);
			SortBIGBuildingsForSectorList(&CWorld::GetSector(x, y)->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
		}
}

void
CRenderer::SortBIGBuildingsForSectorList(CPtrList *list)
{
	CPtrNode *node;
	CEntity *ent;

	for(node = list->first; node; node = node->next){
		ent = (CEntity*)node->item;
		if(ent->bIsBIGBuilding){
			list->RemoveNode(node);
			list->InsertNode(node);
		}
	}
}

bool
CRenderer::ShouldModelBeStreamed(CEntity *ent, const CVector &campos)
{
	if(!IsAreaVisible(ent->m_area))
		return false;
	CTimeModelInfo *mi = (CTimeModelInfo *)CModelInfo::GetModelInfo(ent->GetModelIndex());
	if(mi->GetModelType() == MITYPE_TIME)
		if(!CClock::GetIsTimeInRange(mi->GetTimeOn(), mi->GetTimeOff()))
			return false;
	float dist = (ent->GetPosition() - campos).Magnitude();

	// rouz edit (ChatGPT)
	if(gbUseHighAltitudeStaticMapDistance && ShouldForceHighAltitudeStaticMapEntity(ent))
		return true;

	if(mi->m_noFade)
		return dist - STREAM_DISTANCE < mi->GetLargestLodDistance();
	else
		return dist - FADE_DISTANCE - STREAM_DISTANCE < mi->GetLargestLodDistance();
}

void
CRenderer::RemoveVehiclePedLights(CEntity *ent, bool reset)
{
	if(!ent->bRenderScorched){
		CPointLights::RemoveLightsAffectingObject();
		if(reset)
			ReSetAmbientAndDirectionalColours();
	}
	SetAmbientColours();
	DeActivateDirectional();
}
