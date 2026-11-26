//+ rouz edit (ChatGPT)
#pragma once

#include "Game.h"
#include "Lists.h"
#include "PlayerInfo.h"
#include "Collision.h"

#include "WorldSectors.h"

VALIDATE_SIZE(CSector, 0x28);

//+ rouz edit (Claude)
// Formerly the static class CWorld, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern int World_ms_bigBuildingsListLifetime, World_ms_movingListLifetime, World_ms_sectorListsLifetime;
extern uint16 World_ms_nCurrentScanCode;
extern uint8 World_PlayerInFocus;
extern CEntity * World_pIgnoreEntity;
extern bool World_bIncludeDeadPeds;
extern bool World_bNoMoreCollisionTorque;
extern bool World_bSecondShift;
extern bool World_bForceProcessControl;
extern bool World_bProcessCutsceneOnly;
extern bool World_bDoingCarCollisions;
extern bool World_bIncludeCarTyres;
extern bool World_bIncludeBikers;
void World_Remove(CEntity *entity);
void World_Add(CEntity *entity);
void World_ClearScanCodes(void);
bool World_CameraToIgnoreThisObject(CEntity *ent);
CEntity *World_TestSphereAgainstWorld(CVector centre, float radius, CEntity *entityToIgnore, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSomeObjects);
float World_FindGroundZForCoord(float x, float y);
float World_FindGroundZFor3DCoord(float x, float y, float z, bool *found);
float World_FindRoofZFor3DCoord(float x, float y, float z, bool *found);
void World_RemoveReferencesToDeletedObject(CEntity*);
void World_ClearCarsFromArea(float x1, float y1, float z1, float x2, float y2, float z2);
void World_ClearPedsFromArea(float x1, float y1, float z1, float x2, float y2, float z2);
void World_CallOffChaseForArea(float x1, float y1, float x2, float y2);
void World_RemoveEntityInsteadOfProcessingIt(CEntity* ent);
void World_RemoveFallenPeds();
void World_RemoveFallenCars();
void World_StopAllLawEnforcersInTheirTracks();
void World_SetAllCarsCanBeDamaged(bool);
void World_ExtinguishAllCarFiresInArea(CVector, float);
void World_SetCarsOnFire(float x, float y, float z, float radius, CEntity* reason);
void World_SetPedsChoking(float x, float y, float z, float radius, CEntity* reason);
void World_SetPedsOnFire(float x, float y, float z, float radius, CEntity* reason);
void World_Initialise();
void World_AddParticles();
void World_ShutDown();
void World_ClearForRestart(void);
void World_RepositionCertainDynamicObjects();
void World_RepositionOneObject(CEntity* pEntity);
void World_RemoveStaticObjects();
void World_Process();
void World_UseDetonator(CEntity *pEntity);
void World_CastShadow(float, float, float, float);
void World_FindLowestZForCoord(float, float);
void World_CheckBlockListIntegrity(void);
void World_PrintCarChanges(void);
void World_HandleCollisionZoneChange(eLevelName, eLevelName);
void World_DoZoneTestForChaser(class CPhysical*);
void World_FindPlayerSlotWithPedPointer(void*);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
extern CPtrList World_ms_bigBuildingsList[NUM_LEVELS];
extern CPtrList World_ms_listMovingEntityPtrs;
extern CSector World_ms_aSectors[NUMSECTORS_Y][NUMSECTORS_X];
extern CPlayerInfo World_Players[NUMPLAYERS];
extern CColPoint World_m_aTempColPts[MAX_COLLISION_POINTS];
static inline CSector *World_GetSector(int x, int y);
static inline CPtrList &World_GetBigBuildingList(eLevelName i);
static inline CPtrList &World_GetMovingEntityList(void);
static inline uint16 World_GetCurrentScanCode(void);
static inline void World_AdvanceCurrentScanCode(void);
void World_ClearExcitingStuffFromArea(const CVector &pos, float radius, bool bRemoveProjectilesAndTidyUpShadows);
bool World_ProcessLineOfSight(const CVector &point1, const CVector &point2, CColPoint &point, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects = false, bool ignoreShootThrough = false);
bool World_ProcessLineOfSightSector(CSector &sector, const CColLine &line, CColPoint &point, float &dist, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects, bool ignoreShootThrough);
bool World_ProcessLineOfSightSectorList(CPtrList &list, const CColLine &line, CColPoint &point, float &dist, CEntity *&entity, bool ignoreSeeThrough, bool ignoreSomeObjects, bool ignoreShootThrough);
bool World_ProcessVerticalLine(const CVector &point1, float z2, CColPoint &point, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, CStoredCollPoly *poly);
bool World_ProcessVerticalLineSector(CSector &sector, const CColLine &line, CColPoint &point, CEntity *&entity, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, CStoredCollPoly *poly);
bool World_ProcessVerticalLineSectorList(CPtrList &list, const CColLine &line, CColPoint &point, float &dist, CEntity *&entity, bool ignoreSeeThrough, CStoredCollPoly *poly);
bool World_GetIsLineOfSightClear(const CVector &point1, const CVector &point2, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects = false);
bool World_GetIsLineOfSightSectorClear(CSector &sector, const CColLine &line, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects = false);
bool World_GetIsLineOfSightSectorListClear(CPtrList &list, const CColLine &line, bool ignoreSeeThrough, bool ignoreSomeObjects = false);
CEntity *World_TestSphereAgainstSectorList(CPtrList&, CVector, float, CEntity*, bool);
void World_FindObjectsInRangeSectorList(CPtrList &list, Const CVector &centre, float radius, bool ignoreZ, int16 *numObjects, int16 lastObject, CEntity **objects);
void World_FindObjectsInRange(Const CVector &centre, float radius, bool ignoreZ, int16 *numObjects, int16 lastObject, CEntity **objects, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies);
void World_FindObjectsOfTypeInRangeSectorList(uint32 modelId, CPtrList& list, const CVector& position, float radius, bool bCheck2DOnly, int16* nEntitiesFound, int16 maxEntitiesToFind, CEntity** aEntities);
void World_FindObjectsOfTypeInRange(uint32 modelId, const CVector& position, float radius, bool bCheck2DOnly, int16* nEntitiesFound, int16 maxEntitiesToFind, CEntity** aEntities, bool bBuildings, bool bVehicles, bool bPeds, bool bObjects, bool bDummies);
void World_FindObjectsKindaColliding(const CVector& position, float radius, bool bCheck2DOnly, int16* nCollidingEntities, int16 maxEntitiesToFind, CEntity** aEntities, bool bBuildings, bool bVehicles, bool bPeds, bool bObjects, bool bDummies);
void World_FindObjectsKindaCollidingSectorList(CPtrList& list, const CVector& position, float radius, bool bCheck2DOnly, int16* nCollidingEntities, int16 maxEntitiesToFind, CEntity** aEntities);
void World_FindObjectsIntersectingCube(const CVector& vecStartPos, const CVector& vecEndPos, int16* nIntersecting, int16 maxEntitiesToFind, CEntity** aEntities, bool bBuildings, bool bVehicles, bool bPeds, bool bObjects, bool bDummies);
void World_FindObjectsIntersectingCubeSectorList(CPtrList& list, const CVector& vecStartPos, const CVector& vecEndPos, int16* nIntersecting, int16 maxEntitiesToFind, CEntity** aEntities);
void World_FindObjectsIntersectingAngledCollisionBox(const CBox &, const CMatrix &, const CVector &, float, float, float, float, int16*, int16, CEntity **, bool, bool, bool, bool, bool);
void World_FindObjectsIntersectingAngledCollisionBoxSectorList(CPtrList& list, const CBox& boundingBox, const CMatrix& matrix, const CVector& position, int16* nEntitiesFound, int16 maxEntitiesToFind, CEntity** aEntities);
void World_FindMissionEntitiesIntersectingCube(const CVector& vecStartPos, const CVector& vecEndPos, int16* nIntersecting, int16 maxEntitiesToFind, CEntity** aEntities, bool bVehicles, bool bPeds, bool bObjects);
void World_FindMissionEntitiesIntersectingCubeSectorList(CPtrList& list, const CVector& vecStartPos, const CVector& vecEndPos, int16* nIntersecting, int16 maxEntitiesToFind, CEntity** aEntities, bool bIsVehicleList, bool bIsPedList);
void World_CallOffChaseForAreaSectorListVehicles(CPtrList& list, float x1, float y1, float x2, float y2, float fStartX, float fStartY, float fEndX, float fEndY);
void World_CallOffChaseForAreaSectorListPeds(CPtrList& list, float x1, float y1, float x2, float y2);
bool World_IsWanderPathClear(CVector const&, CVector const&, float, int);
static inline float World_GetSectorX(float f);
static inline float World_GetSectorY(float f);
static inline int World_GetSectorIndexX(float f);
static inline int World_GetSectorIndexY(float f);
static inline float World_GetWorldX(int x);
static inline float World_GetWorldY(int y);
void World_TriggerExplosion(const CVector& position, float fRadius, float fPower, CEntity* pCreator, bool bProcessVehicleBombTimer);
void World_TriggerExplosionSectorList(CPtrList& list, const CVector& position, float fRadius, float fPower, CEntity* pCreator, bool bProcessVehicleBombTimer);
void World_CastShadowSectorList(CPtrList&, float, float, float, float);
void World_ProcessVerticalLineSectorList_FillGlobeColPoints(CPtrList&, const CColLine&, CEntity*&, bool, CStoredCollPoly*);
void World_ProcessVerticalLineSector_FillGlobeColPoints(CSector&, const CColLine&, CEntity*&, bool, bool, bool, bool, bool, bool, CStoredCollPoly*);
void World_ProcessVerticalLine_FillGlobeColPoints(const CVector&, float, CEntity*&, bool, bool, bool, bool, bool, bool, CStoredCollPoly*);
void World_TestForBuildingsOnTopOfEachOther(CPtrList&);
void World_TestForBuildingsOnTopOfEachOther(void);
void World_TestForUnusedModels(CPtrList&, int*);
void World_TestForUnusedModels(void);
static inline CSector *World_GetSector(int x, int y) { if (x > NUMSECTORS_X - 1 || y > NUMSECTORS_Y - 1) return &World_ms_aSectors[0][0]; return &World_ms_aSectors[y][x]; }
static inline CPtrList &World_GetBigBuildingList(eLevelName i) { return World_ms_bigBuildingsList[i]; }
static inline CPtrList &World_GetMovingEntityList(void) { return World_ms_listMovingEntityPtrs; }
static inline uint16 World_GetCurrentScanCode(void) { return World_ms_nCurrentScanCode; }
static inline void World_AdvanceCurrentScanCode(void){
		if(++World_ms_nCurrentScanCode == 0){
			World_ClearScanCodes();
			World_ms_nCurrentScanCode = 1;
		}
	}
static inline float World_GetSectorX(float f) { return ((f - WORLD_MIN_X)/SECTOR_SIZE_X); }
static inline float World_GetSectorY(float f) { return ((f - WORLD_MIN_Y)/SECTOR_SIZE_Y); }
static inline int World_GetSectorIndexX(float f) { return (int)World_GetSectorX(f); }
static inline int World_GetSectorIndexY(float f) { return (int)World_GetSectorY(f); }
static inline float World_GetWorldX(int x) { return x*SECTOR_SIZE_X + WORLD_MIN_X; }
static inline float World_GetWorldY(int y) { return y*SECTOR_SIZE_Y + WORLD_MIN_Y; }
#endif
//- rouz edit (Claude)

extern CColPoint gaTempSphereColPoints[MAX_COLLISION_POINTS];
//- rouz edit (ChatGPT)
