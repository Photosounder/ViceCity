//+ rouz edit (ChatGPT)
#pragma once

class CEntity;

#ifdef FIX_BUGS
#define LOD_DISTANCE (300.0f*TheCamera.LODDistMultiplier)
#else
#define LOD_DISTANCE 300.0f
#endif
#define FADE_DISTANCE 20.0f
#define STREAM_DISTANCE 30.0f

extern bool gbShowPedRoadGroups;
extern bool gbShowCarRoadGroups;
extern bool gbShowCollisionPolys;
extern bool gbShowCollisionLines;
extern bool gbBigWhiteDebugLightSwitchedOn;

extern bool gbDontRenderBuildings;
extern bool gbDontRenderBigBuildings;
extern bool gbDontRenderPeds;
extern bool gbDontRenderObjects;
extern bool gbDontRenderVehicles;

class CVehicle;
struct CPtrList;

// unused
struct BlockedRange
{
	float a, b;	// unknown
	BlockedRange *prev, *next;
};

//+ rouz edit (Claude)
// Formerly the static class CRenderer, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern int32 Renderer_ms_nNoOfVisibleEntities;
extern CEntity * Renderer_ms_aVisibleEntityPtrs[NUMVISIBLEENTITIES];
extern int32 Renderer_ms_nNoOfInVisibleEntities;
extern CEntity * Renderer_ms_aInVisibleEntityPtrs[NUMINVISIBLEENTITIES];
#ifdef NEW_RENDERER
extern int32 Renderer_ms_nNoOfVisibleVehicles;
extern CEntity * Renderer_ms_aVisibleVehiclePtrs[NUMVISIBLEENTITIES];
extern int32 Renderer_ms_nNoOfVisibleBuildings;
extern CEntity * Renderer_ms_aVisibleBuildingPtrs[NUMVISIBLEENTITIES];
#endif
extern CVector Renderer_ms_vecCameraPosition;
extern CVehicle * Renderer_m_pFirstPersonVehicle;
extern BlockedRange Renderer_aBlockedRanges[16];
extern BlockedRange * Renderer_pFullBlockedRanges;
extern BlockedRange * Renderer_pEmptyBlockedRanges;
extern float Renderer_ms_lodDistScale;
extern bool Renderer_m_loadingPriority;
void Renderer_Init(void);
void Renderer_Shutdown(void);
void Renderer_PreRender(void);
void Renderer_RenderRoads(void);
void Renderer_RenderFadingInEntities(void);
void Renderer_RenderFadingInUnderwaterEntities(void);
void Renderer_RenderEverythingBarRoads(void);
void Renderer_RenderBoats(void);
void Renderer_RenderOneRoad(CEntity *);
void Renderer_RenderOneNonRoad(CEntity *);
void Renderer_RenderFirstPersonVehicle(void);
void Renderer_RenderCollisionLines(void);
int32 Renderer_SetupEntityVisibility(CEntity *ent);
int32 Renderer_SetupBigBuildingVisibility(CEntity *ent);
void Renderer_ConstructRenderList(void);
void Renderer_ScanWorld(void);
void Renderer_RequestObjectsInFrustum(void);
void Renderer_ScanSectorPoly(RwV2d *poly, int32 numVertices, void (*scanfunc)(CPtrList *));
void Renderer_ScanFarAwayVehicles(void); // rouz edit (ChatGPT)
void Renderer_ScanSectorList(CPtrList *lists);
void Renderer_ScanSectorList_Priority(CPtrList *lists);
void Renderer_ScanSectorList_Subway(CPtrList *lists);
void Renderer_ScanSectorList_FarStaticMapEntities(CPtrList *lists); // rouz edit (ChatGPT)
void Renderer_ScanSectorList_RequestModels(CPtrList *lists);
void Renderer_ScanFarAwayVehicleModels(void); // rouz edit (ChatGPT)
void Renderer_ScanSectorList_RequestFarStaticMapEntityModels(CPtrList *lists); // rouz edit (ChatGPT)
void Renderer_SortBIGBuildings(void);
void Renderer_SortBIGBuildingsForSectorList(CPtrList *list);
void Renderer_RemoveVehiclePedLights(CEntity *ent, bool reset);
#ifdef NEW_RENDERER
void Renderer_ClearForFrame(void);
void Renderer_RenderPeds(void);
void Renderer_RenderVehicles(void);	// also renders peds in LCS
void Renderer_RenderSoftwareVehicles(void); // rouz edit (ChatGPT)
void Renderer_RegisterSoftwareReflectionLights(void); // rouz edit (ChatGPT)
void Renderer_RenderSoftwareReflectionEntities(void); // rouz edit (ChatGPT)
void Renderer_RenderWorld(int pass);	// like cWorldStream::Render(int)
void Renderer_RenderTransparentWater(void);	// keep-out polys and transparent water
#endif
void Renderer_InsertEntityIntoList(CEntity *ent);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
#ifdef NEW_RENDERER
#endif
void Renderer_ScanBigBuildingList(CPtrList &list);
bool Renderer_ShouldModelBeStreamed(CEntity *ent, const CVector &campos);
#ifdef NEW_RENDERER
void Renderer_RenderOneBuilding(CEntity *ent, float camdist = 0.0f);
#endif
#endif
//- rouz edit (Claude)
//- rouz edit (ChatGPT)
