//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ColSphere_native.h"
#include "CollisionVector_native.h"
#include "ColLine_native.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "CopPed.h"
#include "CutsceneMgr.h"
#include "DMAudio.h"
#include "EventList.h"
#include "Explosion.h"
#include "Fire.h"
#include "Garages.h"
#include "Glass.h"
#include "Messages.h"
#include "ModelIndices.h"
#include "ParticleObject.h"
#include "Pickups.h"
#include "Population.h"
#include "ProjectileInfo.h"
#include "Record.h"
#include "References.h"
#include "Replay.h"
#include "Pools.h" // rouz edit (ChatGPT)
#include "RpAnimBlend.h"
#include "Shadows.h"
#include "TempColModels.h"
#include "WaterLevel.h"
#include "World.h"
#include "WorldLogic.h"
#include "General.h"

#define OBJECT_REPOSITION_OFFSET_Z 2.0f

CColPoint gaTempSphereColPoints[MAX_COLLISION_POINTS];

CPtrList World_ms_bigBuildingsList[NUM_LEVELS];
int World_ms_bigBuildingsListLifetime=PtrList_InitStaticBlock(&World_ms_bigBuildingsList,NUM_LEVELS,sizeof(CPtrList),0,1);
CPtrList World_ms_listMovingEntityPtrs;
int World_ms_movingListLifetime=PtrList_InitStaticBlock(&World_ms_listMovingEntityPtrs,1,sizeof(CPtrList),0,1);
CSector World_ms_aSectors[NUMSECTORS_Y][NUMSECTORS_X];
int World_ms_sectorListsLifetime=PtrList_InitStaticBlock(&World_ms_aSectors,NUMSECTORS_Y*NUMSECTORS_X,sizeof(CSector),offsetof(CSector,m_lists),NUMSECTORENTITYLISTS);
uint16 World_ms_nCurrentScanCode;

uint8 World_PlayerInFocus;
CPlayerInfo World_Players[NUMPLAYERS];
bool World_bNoMoreCollisionTorque;
CEntity *World_pIgnoreEntity;
bool World_bIncludeDeadPeds;
bool World_bSecondShift;
bool World_bForceProcessControl;
bool World_bProcessCutsceneOnly;

bool World_bDoingCarCollisions;
bool World_bIncludeCarTyres;
bool World_bIncludeBikers;

CColPoint World_m_aTempColPts[MAX_COLLISION_POINTS];

void
World_Initialise()
{
	World_pIgnoreEntity = nil;
	World_bDoingCarCollisions = false;
	World_bSecondShift = false;
	World_bNoMoreCollisionTorque = false;
	World_bProcessCutsceneOnly = false;
	World_bIncludeDeadPeds = false;
	World_bForceProcessControl = false;
	World_bIncludeCarTyres = false;
	World_bIncludeBikers = false;
}

void
World_Add(CEntity *ent)
{
	if(ent->IsVehicle() || ent->IsPed()) DMAudio_SetEntityStatus(((CPhysical *)ent)->m_audioEntityId, TRUE);

	if(ent->bIsBIGBuilding)
		PtrList_InsertItem(&World_ms_bigBuildingsList[ent->m_level], ent);
	else
		ent->Add();

	if(ent->IsBuilding() || ent->IsDummy()) return;

	if(!ent->GetIsStatic()) ((CPhysical *)ent)->AddToMovingList();
}

void
World_Remove(CEntity *ent)
{
	if(ent->IsVehicle() || ent->IsPed()) DMAudio_SetEntityStatus(((CPhysical *)ent)->m_audioEntityId, FALSE);

	if(ent->bIsBIGBuilding)
		PtrList_RemoveItem(&World_ms_bigBuildingsList[ent->m_level], ent);
	else
		ent->Remove();

	if(ent->IsBuilding() || ent->IsDummy()) return;

	if(!ent->GetIsStatic()) ((CPhysical *)ent)->RemoveFromMovingList();
}

void
World_ClearScanCodes(void)
{
	// Delegate to the C implementation
	for(int i = 0; i < NUMSECTORS_Y; i++)
		for(int j = 0; j < NUMSECTORS_X; j++) {
			CSector *s = &World_ms_aSectors[i][j];
			World_ClearScanCodesInList(&s->m_lists[ENTITYLIST_BUILDINGS]);
			World_ClearScanCodesInList(&s->m_lists[ENTITYLIST_VEHICLES]);
			World_ClearScanCodesInList(&s->m_lists[ENTITYLIST_PEDS]);
			World_ClearScanCodesInList(&s->m_lists[ENTITYLIST_OBJECTS]);
			World_ClearScanCodesInList(&s->m_lists[ENTITYLIST_DUMMIES]);
		}
}

void
World_ClearExcitingStuffFromArea(const CVector &pos, float radius, bool bRemoveProjectilesAndTidyUpShadows)
{
	// The C implementation picks the peds and vehicles to remove, the removal stays here
	{
		void *selected[NUMPEDS * 2];
		int numSelected = World_SelectPedsInCircleToClear(Pools_GetPedPool(), pos.x, pos.y, radius, selected, ARRAY_SIZE(selected));
		for(int s = 0; s < numSelected; s++)
			Population_RemovePed((CPed*)selected[s]);
	}
	CVehiclePool *VehiclePool = Pools_GetVehiclePool();
	{
		void *selected[NUMVEHICLES * 2];
		int numSelected = World_SelectCarsInCircleToClear(VehiclePool, pos.x, pos.y, radius, selected, ARRAY_SIZE(selected));
		for(int s = 0; s < numSelected; s++) {
			CVehicle *pVehicle = (CVehicle*)selected[s];
			if(pVehicle->pDriver) {
				Population_RemovePed(pVehicle->pDriver);
				pVehicle->pDriver = nil;
			}
			for(int32 j = 0; j < pVehicle->m_nNumMaxPassengers; ++j) {
				if(pVehicle->pPassengers[j]) {
					Population_RemovePed(pVehicle->pPassengers[j]);
					pVehicle->pPassengers[j] = nil;
					--pVehicle->m_nNumPassengers;
				}
			}
			CarCtrl_RemoveFromInterestingVehicleList(pVehicle);
			World_Remove(pVehicle);
			// Destroy and release the vehicle without invoking C++ delete.
			pVehicle->~CVehicle();
			// Access raw storage through the C store or pool API
			CPool_Delete(VehiclePool, pVehicle);
		}
	}
	CObject::DeleteAllTempObjectsInArea(pos, radius);
	FireManager_ExtinguishPoint(&gFireManager,Fire_VectorFromNative(pos), radius);
	World_ExtinguishAllCarFiresInArea(pos, radius);
	// Copy the original by-value position into the C area operation
	ExplosionPosition explosionPosition = {pos.x,pos.y,pos.z};
	Explosion_RemoveAllExplosionsInArea(explosionPosition, radius);
	if(bRemoveProjectilesAndTidyUpShadows) {
		ProjectileInfo_RemoveAllProjectiles();
		CShadows::TidyUpShadows();
	}
	Pickups_RemoveUnnecessaryPickups(pos, radius);
}

bool
World_CameraToIgnoreThisObject(CEntity *ent)
{
	if(Garages_IsModelIndexADoor(ent->GetModelIndex())) return false;
	return ((CObject *)ent)->m_bCameraToAvoidThisObject != 1;
}

bool
World_ProcessLineOfSight(const CVector &point1, const CVector &point2, CColPoint &point, CEntity *&entity,
                           bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects,
                           bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects, bool ignoreShootThrough)
{
	int x, xstart, xend;
	int y, ystart, yend;
	int y1, y2;
	float dist;

	World_AdvanceCurrentScanCode();

	entity = nil;
	dist = 1.0f;

	xstart = World_GetSectorIndexX(point1.x);
	ystart = World_GetSectorIndexY(point1.y);
	xend = World_GetSectorIndexX(point2.x);
	yend = World_GetSectorIndexY(point2.y);

#define LOSARGS                                                                                                        \
	ColLine_Make(COLLISION_POINT_REF(point1),COLLISION_POINT_REF(point2)), point, dist, entity, checkBuildings, checkVehicles, checkPeds, checkObjects,         \
	    checkDummies, ignoreSeeThrough, ignoreSomeObjects, ignoreShootThrough

	if(xstart == xend && ystart == yend) {
		// Only one sector
		return World_ProcessLineOfSightSector(*World_GetSector(xstart, ystart), LOSARGS);
	} else if(xstart == xend) {
		// Only step in y
		if(ystart < yend)
			for(y = ystart; y <= yend; y++) World_ProcessLineOfSightSector(*World_GetSector(xstart, y), LOSARGS);
		else
			for(y = ystart; y >= yend; y--) World_ProcessLineOfSightSector(*World_GetSector(xstart, y), LOSARGS);
		return dist < 1.0f;
	} else if(ystart == yend) {
		// Only step in x
		if(xstart < xend)
			for(x = xstart; x <= xend; x++) World_ProcessLineOfSightSector(*World_GetSector(x, ystart), LOSARGS);
		else
			for(x = xstart; x >= xend; x--) World_ProcessLineOfSightSector(*World_GetSector(x, ystart), LOSARGS);
		return dist < 1.0f;
	} else {
		if(point1.x < point2.x) {
			// Step from left to right
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = World_GetSectorIndexY((World_GetWorldX(xstart + 1) - point1.x) * m + point1.y);
			if(y1 < y2)
				for(y = y1; y <= y2; y++) World_ProcessLineOfSightSector(*World_GetSector(xstart, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--) World_ProcessLineOfSightSector(*World_GetSector(xstart, y), LOSARGS);

			for(x = xstart + 1; x < xend; x++) {
				y1 = y2;
				y2 = World_GetSectorIndexY((World_GetWorldX(x + 1) - point1.x) * m + point1.y);
				if(y1 < y2)
					for(y = y1; y <= y2; y++) World_ProcessLineOfSightSector(*World_GetSector(x, y), LOSARGS);
				else
					for(y = y1; y >= y2; y--) World_ProcessLineOfSightSector(*World_GetSector(x, y), LOSARGS);
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2)
				for(y = y1; y <= y2; y++) World_ProcessLineOfSightSector(*World_GetSector(xend, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--) World_ProcessLineOfSightSector(*World_GetSector(xend, y), LOSARGS);
		} else {
			// Step from right to left
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = World_GetSectorIndexY((World_GetWorldX(xstart) - point1.x) * m + point1.y);
			if(y1 < y2)
				for(y = y1; y <= y2; y++) World_ProcessLineOfSightSector(*World_GetSector(xstart, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--) World_ProcessLineOfSightSector(*World_GetSector(xstart, y), LOSARGS);

			for(x = xstart - 1; x > xend; x--) {
				y1 = y2;
				y2 = World_GetSectorIndexY((World_GetWorldX(x) - point1.x) * m + point1.y);
				if(y1 < y2)
					for(y = y1; y <= y2; y++) World_ProcessLineOfSightSector(*World_GetSector(x, y), LOSARGS);
				else
					for(y = y1; y >= y2; y--) World_ProcessLineOfSightSector(*World_GetSector(x, y), LOSARGS);
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2)
				for(y = y1; y <= y2; y++) World_ProcessLineOfSightSector(*World_GetSector(xend, y), LOSARGS);
			else
				for(y = y1; y >= y2; y--) World_ProcessLineOfSightSector(*World_GetSector(xend, y), LOSARGS);
		}
		return dist < 1.0f;
	}

#undef LOSARGS
}

bool
World_ProcessLineOfSightSector(CSector &sector, const CColLine &line, CColPoint &point, float &dist, CEntity *&entity,
                                 bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects,
                                 bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects, bool ignoreShootThrough)
{
	float mindist = dist;
	bool deadPeds = !!World_bIncludeDeadPeds;
	bool bikers = !!World_bIncludeBikers;
	World_bIncludeDeadPeds = false;
	World_bIncludeBikers = false;

	if(checkBuildings) {
		World_ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_BUILDINGS], line, point, mindist, entity,
		                             ignoreSeeThrough, false, ignoreShootThrough);
		World_ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_BUILDINGS_OVERLAP], line, point, mindist, entity,
		                             ignoreSeeThrough, false, ignoreShootThrough);
	}

	if(checkVehicles) {
		World_ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_VEHICLES], line, point, mindist, entity,
		                             ignoreSeeThrough, false, ignoreShootThrough);
		World_ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_VEHICLES_OVERLAP], line, point, mindist, entity,
		                             ignoreSeeThrough, false, ignoreShootThrough);
	}

	if(checkPeds) {
		if(deadPeds) World_bIncludeDeadPeds = true;
		if(bikers) World_bIncludeBikers = true;
		World_ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_PEDS], line, point, mindist, entity,
		                             ignoreSeeThrough, false, ignoreShootThrough);
		World_ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_PEDS_OVERLAP], line, point, mindist, entity,
		                             ignoreSeeThrough, false, ignoreShootThrough);
		World_bIncludeDeadPeds = false;
		World_bIncludeBikers = false;
	}

	if(checkObjects) {
		World_ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_OBJECTS], line, point, mindist, entity,
		                             ignoreSeeThrough, ignoreSomeObjects, ignoreShootThrough);
		World_ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_OBJECTS_OVERLAP], line, point, mindist, entity,
		                             ignoreSeeThrough, ignoreSomeObjects, ignoreShootThrough);
	}

	if(checkDummies) {
		World_ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_DUMMIES], line, point, mindist, entity,
		                             ignoreSeeThrough, false, ignoreShootThrough);
		World_ProcessLineOfSightSectorList(sector.m_lists[ENTITYLIST_DUMMIES_OVERLAP], line, point, mindist, entity,
		                             ignoreSeeThrough, false, ignoreShootThrough);
	}

	World_bIncludeDeadPeds = deadPeds;
	World_bIncludeBikers = bikers;

	if(mindist < dist) {
		dist = mindist;
		return true;
	} else
		return false;
}

bool
World_ProcessLineOfSightSectorList(CPtrList &list, const CColLine &line, CColPoint &point, float &dist,
                                     CEntity *&entity, bool ignoreSeeThrough, bool ignoreSomeObjects, bool ignoreShootThrough)
{
	bool deadPeds = false;
	bool bikers = false;
	bool carTyres = false;
	float mindist = dist;
	CPtrNode *node;
	CEntity *e;
	CColModel *colmodel;
	CColModel tyreCol;
	// Invoke the complete C collision owner at the original native lifetime or operation boundary
	ColModel_Init(&tyreCol);
	CColSphere tyreSpheres[6];
	CColPoint tyreColPoint;
	float tyreDist;

	if(World_bIncludeCarTyres && list.first && ((CEntity*)list.first->item)->IsVehicle()){
		carTyres = true;
		tyreCol.numTriangles = 0;
		tyreCol.numBoxes = 0;
		tyreCol.numLines = 0;
		tyreCol.spheres = tyreSpheres;
		tyreCol.numSpheres = ARRAY_SIZE(tyreSpheres);
	}
	if(list.first && World_bIncludeDeadPeds && ((CEntity *)list.first->item)->IsPed()) deadPeds = true;
	if(list.first && World_bIncludeBikers && ((CEntity *)list.first->item)->IsPed()) bikers = true;

	for(node = list.first; node; node = node->next) {
		e = (CEntity *)node->item;
		if(e->m_scanCode != World_GetCurrentScanCode() && e != World_pIgnoreEntity && (e->bUsesCollision || deadPeds || bikers) &&
		   !(ignoreSomeObjects && World_CameraToIgnoreThisObject(e))) {
			colmodel = nil;
			tyreDist = mindist;
			e->m_scanCode = World_GetCurrentScanCode();

			if(e->IsPed()) {
				if(e->bUsesCollision || deadPeds && ((CPed *)e)->m_nPedState == PED_DEAD || bikers && ((CPed*)e)->InVehicle() && (((CPed*)e)->m_pMyVehicle->IsBike() || ((CPed*)e)->m_pMyVehicle->IsBoat())) {
					colmodel = ((CPedModelInfo *)((CBaseModelInfo*)ModelRegistry_Get(e->GetModelIndex())))->AnimatePedColModelSkinned(e->GetClump()); // rouz edit (ChatGPT)
				} else
					colmodel = nil;

			} else if(e->bUsesCollision)
				colmodel = ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(e->GetModelIndex()))); // rouz edit (ChatGPT)

			if(colmodel && Collision_ProcessLineOfSight(line, e->GetMatrix(), *colmodel, point, mindist,
			                                              ignoreSeeThrough, ignoreShootThrough))
				entity = e;
			if(carTyres && ((CVehicle*)e)->SetUpWheelColModel(&tyreCol) && Collision_ProcessLineOfSight(line, e->GetMatrix(), tyreCol, tyreColPoint, tyreDist, false, ignoreShootThrough)){
				float dp1 = DotProduct(CollisionVector_ToNative(line.p1) - CollisionVector_ToNative(line.p0), e->GetRight());
				float dp2 = DotProduct(CollisionVector_ToNative(point.point) - e->GetPosition(), e->GetRight());
				if(tyreDist < mindist || dp1 < -0.85f && dp2 > 0.0f || dp1 > 0.85f && dp2 < 0.0f){
					mindist = tyreDist;
					ColPoint_Assign(&point,&tyreColPoint);
					entity = e;
				}
			}
		}
	}
	tyreCol.spheres = nil;

	if(mindist < dist) {
		dist = mindist;
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		ColModel_Destroy(&tyreCol);
		return true;
	} else {
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		ColModel_Destroy(&tyreCol);
		return false;
	}
}

bool
World_ProcessVerticalLine(const CVector &point1, float z2, CColPoint &point, CEntity *&entity, bool checkBuildings,
                            bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies,
                            bool ignoreSeeThrough, CStoredCollPoly *poly)
{
	World_AdvanceCurrentScanCode();
	CVector point2(point1.x, point1.y, z2);
	int secX = World_GetSectorIndexX(point1.x);
	int secY = World_GetSectorIndexY(point1.y);
	secX = Clamp(secX, 0, NUMSECTORS_X-1);
	secY = Clamp(secY, 0, NUMSECTORS_Y-1);
	return World_ProcessVerticalLineSector(*World_GetSector(secX, secY),
	                                 ColLine_Make(COLLISION_POINT_REF(point1),COLLISION_POINT_REF(point2)), point, entity, checkBuildings, checkVehicles,
	                                 checkPeds, checkObjects, checkDummies, ignoreSeeThrough, poly);
}

bool
World_ProcessVerticalLineSector(CSector &sector, const CColLine &line, CColPoint &point, CEntity *&entity,
                                  bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects,
                                  bool checkDummies, bool ignoreSeeThrough, CStoredCollPoly *poly)
{
	float mindist = 1.0f;

	if(checkBuildings) {
		World_ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_BUILDINGS], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
		World_ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_BUILDINGS_OVERLAP], line, point, mindist,
		                              entity, ignoreSeeThrough, poly);
	}

	if(checkVehicles) {
		World_ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_VEHICLES], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
		World_ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_VEHICLES_OVERLAP], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
	}

	if(checkPeds) {
		World_ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_PEDS], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
		World_ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_PEDS_OVERLAP], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
	}

	if(checkObjects) {
		World_ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_OBJECTS], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
		World_ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_OBJECTS_OVERLAP], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
	}

	if(checkDummies) {
		World_ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_DUMMIES], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
		World_ProcessVerticalLineSectorList(sector.m_lists[ENTITYLIST_DUMMIES_OVERLAP], line, point, mindist, entity,
		                              ignoreSeeThrough, poly);
	}

	return mindist < 1.0f;
}

bool
World_ProcessVerticalLineSectorList(CPtrList &list, const CColLine &line, CColPoint &point, float &dist,
                                      CEntity *&entity, bool ignoreSeeThrough, CStoredCollPoly *poly)
{
	float mindist = dist;
	CPtrNode *node;
	CEntity *e;
	CColModel *colmodel;

	for(node = list.first; node; node = node->next) {
		e = (CEntity *)node->item;
		if(e->m_scanCode != World_GetCurrentScanCode() && e->bUsesCollision) {
			e->m_scanCode = World_GetCurrentScanCode();

			colmodel = ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(e->GetModelIndex()))); // rouz edit (ChatGPT)
			if(Collision_ProcessVerticalLine(line, e->GetMatrix(), *colmodel, point, mindist,
			                                   ignoreSeeThrough, false, poly))
				entity = e;
		}
	}

	if(mindist < dist) {
		dist = mindist;
		return true;
	} else
		return false;
}

bool
World_GetIsLineOfSightClear(const CVector &point1, const CVector &point2, bool checkBuildings, bool checkVehicles,
                              bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough,
                              bool ignoreSomeObjects)
{
	int x, xstart, xend;
	int y, ystart, yend;
	int y1, y2;

	World_AdvanceCurrentScanCode();

	xstart = World_GetSectorIndexX(point1.x);
	ystart = World_GetSectorIndexY(point1.y);
	xend = World_GetSectorIndexX(point2.x);
	yend = World_GetSectorIndexY(point2.y);

#define LOSARGS                                                                                                        \
	ColLine_Make(COLLISION_POINT_REF(point1),COLLISION_POINT_REF(point2)), checkBuildings, checkVehicles, checkPeds, checkObjects, checkDummies,                \
	    ignoreSeeThrough, ignoreSomeObjects

	if(xstart == xend && ystart == yend) {
		// Only one sector
		return World_GetIsLineOfSightSectorClear(*World_GetSector(xstart, ystart), LOSARGS);
	} else if(xstart == xend) {
		// Only step in y
		if(ystart < yend) {
			for(y = ystart; y <= yend; y++)
				if(!World_GetIsLineOfSightSectorClear(*World_GetSector(xstart, y), LOSARGS)) return false;
		} else {
			for(y = ystart; y >= yend; y--)
				if(!World_GetIsLineOfSightSectorClear(*World_GetSector(xstart, y), LOSARGS)) return false;
		}
	} else if(ystart == yend) {
		// Only step in x
		if(xstart < xend) {
			for(x = xstart; x <= xend; x++)
				if(!World_GetIsLineOfSightSectorClear(*World_GetSector(x, ystart), LOSARGS)) return false;
		} else {
			for(x = xstart; x >= xend; x--)
				if(!World_GetIsLineOfSightSectorClear(*World_GetSector(x, ystart), LOSARGS)) return false;
		}
	} else {
		if(point1.x < point2.x) {
			// Step from left to right
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = World_GetSectorIndexY((World_GetWorldX(xstart + 1) - point1.x) * m + point1.y);
			if(y1 < y2) {
				for(y = y1; y <= y2; y++)
					if(!World_GetIsLineOfSightSectorClear(*World_GetSector(xstart, y), LOSARGS)) return false;
			} else {
				for(y = y1; y >= y2; y--)
					if(!World_GetIsLineOfSightSectorClear(*World_GetSector(xstart, y), LOSARGS)) return false;
			}

			for(x = xstart + 1; x < xend; x++) {
				y1 = y2;
				y2 = World_GetSectorIndexY((World_GetWorldX(x + 1) - point1.x) * m + point1.y);
				if(y1 < y2) {
					for(y = y1; y <= y2; y++)
						if(!World_GetIsLineOfSightSectorClear(*World_GetSector(x, y), LOSARGS))
							return false;
				} else {
					for(y = y1; y >= y2; y--)
						if(!World_GetIsLineOfSightSectorClear(*World_GetSector(x, y), LOSARGS))
							return false;
				}
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2) {
				for(y = y1; y <= y2; y++)
					if(!World_GetIsLineOfSightSectorClear(*World_GetSector(xend, y), LOSARGS)) return false;
			} else {
				for(y = y1; y >= y2; y--)
					if(!World_GetIsLineOfSightSectorClear(*World_GetSector(xend, y), LOSARGS)) return false;
			}
		} else {
			// Step from right to left
			float m = (point2.y - point1.y) / (point2.x - point1.x);

			y1 = ystart;
			y2 = World_GetSectorIndexY((World_GetWorldX(xstart) - point1.x) * m + point1.y);
			if(y1 < y2) {
				for(y = y1; y <= y2; y++)
					if(!World_GetIsLineOfSightSectorClear(*World_GetSector(xstart, y), LOSARGS)) return false;
			} else {
				for(y = y1; y >= y2; y--)
					if(!World_GetIsLineOfSightSectorClear(*World_GetSector(xstart, y), LOSARGS)) return false;
			}

			for(x = xstart - 1; x > xend; x--) {
				y1 = y2;
				y2 = World_GetSectorIndexY((World_GetWorldX(x) - point1.x) * m + point1.y);
				if(y1 < y2) {
					for(y = y1; y <= y2; y++)
						if(!World_GetIsLineOfSightSectorClear(*World_GetSector(x, y), LOSARGS))
							return false;
				} else {
					for(y = y1; y >= y2; y--)
						if(!World_GetIsLineOfSightSectorClear(*World_GetSector(x, y), LOSARGS))
							return false;
				}
			}

			y1 = y2;
			y2 = yend;
			if(y1 < y2) {
				for(y = y1; y <= y2; y++)
					if(!World_GetIsLineOfSightSectorClear(*World_GetSector(xend, y), LOSARGS)) return false;
			} else {
				for(y = y1; y >= y2; y--)
					if(!World_GetIsLineOfSightSectorClear(*World_GetSector(xend, y), LOSARGS)) return false;
			}
		}
	}

	return true;

#undef LOSARGS
}

bool
World_GetIsLineOfSightSectorClear(CSector &sector, const CColLine &line, bool checkBuildings, bool checkVehicles,
                                    bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough,
                                    bool ignoreSomeObjects)
{
	if(checkBuildings) {
		if(!World_GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_BUILDINGS], line, ignoreSeeThrough))
			return false;
		if(!World_GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_BUILDINGS_OVERLAP], line,
		                                    ignoreSeeThrough))
			return false;
	}

	if(checkVehicles) {
		if(!World_GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_VEHICLES], line, ignoreSeeThrough))
			return false;
		if(!World_GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_VEHICLES_OVERLAP], line,
		                                    ignoreSeeThrough))
			return false;
	}

	if(checkPeds) {
		if(!World_GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_PEDS], line, ignoreSeeThrough))
			return false;
		if(!World_GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_PEDS_OVERLAP], line, ignoreSeeThrough))
			return false;
	}

	if(checkObjects) {
		if(!World_GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_OBJECTS], line, ignoreSeeThrough,
		                                    ignoreSomeObjects))
			return false;
		if(!World_GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_OBJECTS_OVERLAP], line, ignoreSeeThrough,
		                                    ignoreSomeObjects))
			return false;
	}

	if(checkDummies) {
		if(!World_GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_DUMMIES], line, ignoreSeeThrough))
			return false;
		if(!World_GetIsLineOfSightSectorListClear(sector.m_lists[ENTITYLIST_DUMMIES_OVERLAP], line, ignoreSeeThrough))
			return false;
	}

	return true;
}

bool
World_GetIsLineOfSightSectorListClear(CPtrList &list, const CColLine &line, bool ignoreSeeThrough,
                                        bool ignoreSomeObjects)
{
	CPtrNode *node;
	CEntity *e;
	CColModel *colmodel;

	for(node = list.first; node; node = node->next) {
		e = (CEntity *)node->item;
		if(e->m_scanCode != World_GetCurrentScanCode() && e->bUsesCollision) {

			e->m_scanCode = World_GetCurrentScanCode();

			if(e != World_pIgnoreEntity && !(ignoreSomeObjects && World_CameraToIgnoreThisObject(e))) {

				colmodel = ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(e->GetModelIndex()))); // rouz edit (ChatGPT)

				if(Collision_TestLineOfSight(line, e->GetMatrix(), *colmodel, ignoreSeeThrough, false))
					return false;
			}
		}
	}

	return true;
}

void
World_FindObjectsInRangeSectorList(CPtrList &list, Const CVector &centre, float radius, bool ignoreZ, int16 *numObjects,
                                     int16 lastObject, CEntity **objects)
{
	// Delegate to the C implementation
	World_FindObjectsInRangeInList(&list, World_GetCurrentScanCode(), centre.x, centre.y, centre.z, radius, ignoreZ, numObjects, lastObject, (void**)objects);
}

void
World_FindObjectsInRange(Const CVector &centre, float radius, bool ignoreZ, int16 *numObjects, int16 lastObject,
                           CEntity **objects, bool checkBuildings, bool checkVehicles, bool checkPeds,
                           bool checkObjects, bool checkDummies)
{
	int minX = World_GetSectorIndexX(centre.x - radius);
	if(minX <= 0) minX = 0;

	int minY = World_GetSectorIndexY(centre.y - radius);
	if(minY <= 0) minY = 0;

	int maxX = World_GetSectorIndexX(centre.x + radius);
	if(maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X - 1;

	int maxY = World_GetSectorIndexY(centre.y + radius);
	if(maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y - 1;

	World_AdvanceCurrentScanCode();

	*numObjects = 0;
	for(int curY = minY; curY <= maxY; curY++) {
		for(int curX = minX; curX <= maxX; curX++) {
			CSector *sector = World_GetSector(curX, curY);
			if(checkBuildings) {
				World_FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_BUILDINGS], centre, radius,
				                             ignoreZ, numObjects, lastObject, objects);
				World_FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], centre,
				                             radius, ignoreZ, numObjects, lastObject, objects);
			}
			if(checkVehicles) {
				World_FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_VEHICLES], centre, radius,
				                             ignoreZ, numObjects, lastObject, objects);
				World_FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], centre,
				                             radius, ignoreZ, numObjects, lastObject, objects);
			}
			if(checkPeds) {
				World_FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_PEDS], centre, radius, ignoreZ,
				                             numObjects, lastObject, objects);
				World_FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_PEDS_OVERLAP], centre, radius,
				                             ignoreZ, numObjects, lastObject, objects);
			}
			if(checkObjects) {
				World_FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_OBJECTS], centre, radius,
				                             ignoreZ, numObjects, lastObject, objects);
				World_FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], centre,
				                             radius, ignoreZ, numObjects, lastObject, objects);
			}
			if(checkDummies) {
				World_FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_DUMMIES], centre, radius,
				                             ignoreZ, numObjects, lastObject, objects);
				World_FindObjectsInRangeSectorList(sector->m_lists[ENTITYLIST_DUMMIES_OVERLAP], centre,
				                             radius, ignoreZ, numObjects, lastObject, objects);
			}
		}
	}
}

void
World_FindObjectsOfTypeInRangeSectorList(uint32 modelId, CPtrList &list, const CVector &position, float radius,
                                           bool bCheck2DOnly, int16 *nEntitiesFound, int16 maxEntitiesToFind,
                                           CEntity **aEntities)
{
	// Delegate to the C implementation
	World_FindObjectsOfTypeInRangeInList(modelId, &list, World_GetCurrentScanCode(), position.x, position.y, position.z, radius, bCheck2DOnly, nEntitiesFound, maxEntitiesToFind, (void**)aEntities);
}

void
World_FindObjectsOfTypeInRange(uint32 modelId, const CVector &position, float radius, bool bCheck2DOnly,
                                 int16 *nEntitiesFound, int16 maxEntitiesToFind, CEntity **aEntities, bool bBuildings,
                                 bool bVehicles, bool bPeds, bool bObjects, bool bDummies)
{
	World_AdvanceCurrentScanCode();
	*nEntitiesFound = 0;
	const CVector2D vecSectorStartPos(position.x - radius, position.y - radius);
	const CVector2D vecSectorEndPos(position.x + radius, position.y + radius);
	// Delegate to the C implementation
	int32 nStartX, nStartY, nEndX, nEndY;
	World_SectorRange(vecSectorStartPos.x, vecSectorStartPos.y, vecSectorEndPos.x, vecSectorEndPos.y, &nStartX, &nStartY, &nEndX, &nEndY);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = World_GetSector(x, y);
			if(bBuildings) {
				World_FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_BUILDINGS], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				World_FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], position, radius,
				    bCheck2DOnly, nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bVehicles) {
				World_FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_VEHICLES], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				World_FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], position, radius,
				    bCheck2DOnly, nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bPeds) {
				World_FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_PEDS], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				World_FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_PEDS_OVERLAP], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bObjects) {
				World_FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_OBJECTS], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				World_FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], position, radius,
				    bCheck2DOnly, nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bDummies) {
				World_FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_DUMMIES], position, radius, bCheck2DOnly,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				World_FindObjectsOfTypeInRangeSectorList(
				    modelId, pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP], position, radius,
				    bCheck2DOnly, nEntitiesFound, maxEntitiesToFind, aEntities);
			}
		}
	}
}

CEntity *
World_TestSphereAgainstWorld(CVector centre, float radius, CEntity *entityToIgnore, bool checkBuildings,
                               bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies,
                               bool ignoreSomeObjects)
{
	CEntity *foundE = nil;

	int minX = World_GetSectorIndexX(centre.x - radius);
	if(minX <= 0) minX = 0;

	int minY = World_GetSectorIndexY(centre.y - radius);
	if(minY <= 0) minY = 0;

	int maxX = World_GetSectorIndexX(centre.x + radius);
#ifdef FIX_BUGS
	if(maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X - 1;
#else
	if(maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X;
#endif

	int maxY = World_GetSectorIndexY(centre.y + radius);
#ifdef FIX_BUGS
	if(maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y - 1;
#else
	if(maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y;
#endif

	World_AdvanceCurrentScanCode();

	for(int curY = minY; curY <= maxY; curY++) {
		for(int curX = minX; curX <= maxX; curX++) {
			CSector *sector = World_GetSector(curX, curY);
			if(checkBuildings) {
				foundE = World_TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_BUILDINGS], centre,
				                                     radius, entityToIgnore, false);
				if(foundE) return foundE;

				foundE = World_TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP],
				                                     centre, radius, entityToIgnore, false);
				if(foundE) return foundE;
			}
			if(checkVehicles) {
				foundE = World_TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_VEHICLES], centre,
				                                     radius, entityToIgnore, false);
				if(foundE) return foundE;

				foundE = World_TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP],
				                                     centre, radius, entityToIgnore, false);
				if(foundE) return foundE;
			}
			if(checkPeds) {
				foundE = World_TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_PEDS], centre, radius,
				                                     entityToIgnore, false);
				if(foundE) return foundE;

				foundE = World_TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_PEDS_OVERLAP], centre,
				                                     radius, entityToIgnore, false);
				if(foundE) return foundE;
			}
			if(checkObjects) {
				foundE = World_TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_OBJECTS], centre,
				                                     radius, entityToIgnore, ignoreSomeObjects);
				if(foundE) return foundE;

				foundE = World_TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_OBJECTS_OVERLAP],
				                                     centre, radius, entityToIgnore, ignoreSomeObjects);
				if(foundE) return foundE;
			}
			if(checkDummies) {
				foundE = World_TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_DUMMIES], centre,
				                                     radius, entityToIgnore, false);
				if(foundE) return foundE;

				foundE = World_TestSphereAgainstSectorList(sector->m_lists[ENTITYLIST_DUMMIES_OVERLAP],
				                                     centre, radius, entityToIgnore, false);
				if(foundE) return foundE;
			}
		}
	}
	return foundE;
}

static CColModel worldSphereColModel;
static void DestroyStaticWorldSphereModel(void)
{
    // Release the lazily initialized world sphere model at its original process-exit position
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&worldSphereColModel);
}

CEntity *
World_TestSphereAgainstSectorList(CPtrList &list, CVector spherePos, float radius, CEntity *entityToIgnore,
                                    bool ignoreSomeObjects)
{
	// Invoke the complete C collision owner at the original native lifetime or operation boundary
	static const bool modelInitialised=(ColModel_InitStatic(&worldSphereColModel,DestroyStaticWorldSphereModel),true);
	(void)modelInitialised;
	CColSphere sphere;

	worldSphereColModel.boundingSphere.center.x = 0.0f;
	worldSphereColModel.boundingSphere.center.y = 0.0f;
	worldSphereColModel.boundingSphere.center.z = 0.0f;
	worldSphereColModel.boundingSphere.radius = radius;
	worldSphereColModel.boundingBox.min.x = -radius;
	worldSphereColModel.boundingBox.min.y = -radius;
	worldSphereColModel.boundingBox.min.z = -radius;
	worldSphereColModel.boundingBox.max.x = radius;
	worldSphereColModel.boundingBox.max.y = radius;
	worldSphereColModel.boundingBox.max.z = radius;
	worldSphereColModel.numSpheres = 1;
	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter0=CVector(0.0f, 0.0f, 0.0f);
	    ColSphere_SetBounds(&sphere,radius,COLLISION_POINT_REF(sphereCenter0));
	}
	worldSphereColModel.spheres = &sphere;
	worldSphereColModel.numLines = 0;
	worldSphereColModel.numBoxes = 0;
	worldSphereColModel.numTriangles = 0;
	worldSphereColModel.ownsCollisionVolumes = false;

	CMatrix sphereMat;
	sphereMat.SetTranslate(spherePos);

	for(CPtrNode *node = list.first; node; node = node->next) {
		CEntity *e = (CEntity *)node->item;

		if(e->m_scanCode != World_GetCurrentScanCode()) {
			e->m_scanCode = World_GetCurrentScanCode();

			if(e != entityToIgnore && e->bUsesCollision &&
			   !(ignoreSomeObjects && World_CameraToIgnoreThisObject(e))) {
				CVector diff = spherePos - e->GetBoundCentre();
				float distance = diff.Magnitude();

				if(e->GetBoundRadius() + radius > distance) {
					CColModel *eCol = ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(e->GetModelIndex()))); // rouz edit (ChatGPT)
					int collidedSpheres =
					    Collision_ProcessColModels(sphereMat, worldSphereColModel, e->GetMatrix(), *eCol,
					                                 gaTempSphereColPoints, nil, nil);

					if(collidedSpheres != 0 ||
					   (e->IsVehicle() && ((CVehicle *)e)->m_vehType == VEHICLE_TYPE_CAR && e->GetModelIndex() != MI_DODO &&
					    radius + eCol->boundingBox.max.x > distance)) {
						return e;
					}
				}
			}
		}
	}

	return nil;
}

float
World_FindGroundZForCoord(float x, float y)
{
	CColPoint point;
	CEntity *ent;
	if(World_ProcessVerticalLine(CVector(x, y, 1000.0f), -1000.0f, point, ent, true, false, false, false, true, false,
	                       nil))
		return point.point.z;
	else
		return 20.0f;
}

float
World_FindGroundZFor3DCoord(float x, float y, float z, bool *found)
{
	CColPoint point;
	CEntity *ent;
	if(World_ProcessVerticalLine(CVector(x, y, z), -1000.0f, point, ent, true, false, false, false, false, false, nil)) {
		if(found) *found = true;
		return point.point.z;
	} else {
		if(found) *found = false;
		return 0.0f;
	}
}

float
World_FindRoofZFor3DCoord(float x, float y, float z, bool *found)
{
	CColPoint point;
	CEntity *ent;
	if(World_ProcessVerticalLine(CVector(x, y, z), 1000.0f, point, ent, true, false, false, false, true, false, nil)) {
		if(found) *found = true;
		return point.point.z;
	} else {
		if(found == nil)
			printf("THERE IS NO MAP BELOW THE FOLLOWING COORS:%f %f %f. (FindGroundZFor3DCoord)\n", x, y,
			       z);
		if(found) *found = false;
		return 20.0f;
	}
}

void
World_RemoveReferencesToDeletedObject(CEntity *pDeletedObject)
{
	// The ped and object pools are handled by the C implementation
	World_RemoveRefsToDeletedFromPeds(Pools_GetPedPool(), pDeletedObject);
	// Access raw storage through the C store or pool API
	int32 i = CPool_GetSize(Pools_GetVehiclePool());
	while(--i >= 0) {
		CVehicle *pVehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
		if(pVehicle && pVehicle != pDeletedObject) {
			pVehicle->RemoveRefsToEntity(pDeletedObject);
			pVehicle->RemoveRefsToVehicle(pDeletedObject);
		}
	}
	World_RemoveRefsToDeletedFromObjects(Pools_GetObjectPool(), pDeletedObject);
}

void
World_FindObjectsKindaColliding(const CVector &position, float radius, bool bCheck2DOnly, int16 *nCollidingEntities,
                                  int16 maxEntitiesToFind, CEntity **aEntities, bool bBuildings, bool bVehicles,
                                  bool bPeds, bool bObjects, bool bDummies)
{
	World_AdvanceCurrentScanCode();
	*nCollidingEntities = 0;
	const CVector2D vecSectorStartPos(position.x - radius, position.y - radius);
	const CVector2D vecSectorEndPos(position.x + radius, position.y + radius);
	// Delegate to the C implementation
	int32 nStartX, nStartY, nEndX, nEndY;
	World_SectorRange(vecSectorStartPos.x, vecSectorStartPos.y, vecSectorEndPos.x, vecSectorEndPos.y, &nStartX, &nStartY, &nEndX, &nEndY);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = World_GetSector(x, y);
			if(bBuildings) {
				World_FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_BUILDINGS], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
				World_FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
			}
			if(bVehicles) {
				World_FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
				World_FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
			}
			if(bPeds) {
				World_FindObjectsKindaCollidingSectorList(pSector->m_lists[ENTITYLIST_PEDS], position,
				                                            radius, bCheck2DOnly, nCollidingEntities,
				                                            maxEntitiesToFind, aEntities);
				World_FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_PEDS_OVERLAP], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
			}
			if(bObjects) {
				World_FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
				World_FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
			}
			if(bDummies) {
				World_FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_DUMMIES], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
				World_FindObjectsKindaCollidingSectorList(
				    pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP], position, radius, bCheck2DOnly,
				    nCollidingEntities, maxEntitiesToFind, aEntities);
			}
		}
	}
}

void
World_FindObjectsKindaCollidingSectorList(CPtrList &list, const CVector &position, float radius, bool bCheck2DOnly,
                                            int16 *nCollidingEntities, int16 maxEntitiesToFind, CEntity **aEntities)
{
	// Delegate to the C implementation
	World_FindObjectsKindaCollidingInList(&list, World_GetCurrentScanCode(), position.x, position.y, position.z, radius, bCheck2DOnly, nCollidingEntities, maxEntitiesToFind, (void**)aEntities);
}

void
World_FindObjectsIntersectingCube(const CVector &vecStartPos, const CVector &vecEndPos, int16 *nIntersecting,
                                    int16 maxEntitiesToFind, CEntity **aEntities, bool bBuildings, bool bVehicles,
                                    bool bPeds, bool bObjects, bool bDummies)
{
	World_AdvanceCurrentScanCode();
	*nIntersecting = 0;
	// Delegate to the C implementation
	int32 nStartX, nStartY, nEndX, nEndY;
	World_SectorRange(vecStartPos.x, vecStartPos.y, vecStartPos.x, vecStartPos.y, &nStartX, &nStartY, &nEndX, &nEndY);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = World_GetSector(x, y);
			if(bBuildings) {
				World_FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_BUILDINGS],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
				World_FindObjectsIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], vecStartPos, vecEndPos,
				    nIntersecting, maxEntitiesToFind, aEntities);
			}
			if(bVehicles) {
				World_FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_VEHICLES],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
				World_FindObjectsIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], vecStartPos, vecEndPos,
				    nIntersecting, maxEntitiesToFind, aEntities);
			}
			if(bPeds) {
				World_FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_PEDS],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
				World_FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_PEDS_OVERLAP],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
			}
			if(bObjects) {
				World_FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_OBJECTS],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
				World_FindObjectsIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities);
			}
			if(bDummies) {
				World_FindObjectsIntersectingCubeSectorList(pSector->m_lists[ENTITYLIST_DUMMIES],
				                                              vecStartPos, vecEndPos, nIntersecting,
				                                              maxEntitiesToFind, aEntities);
				World_FindObjectsIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities);
			}
		}
	}
}

void
World_FindObjectsIntersectingCubeSectorList(CPtrList &list, const CVector &vecStartPos, const CVector &vecEndPos,
                                              int16 *nIntersecting, int16 maxEntitiesToFind, CEntity **aEntities)
{
	// Delegate to the C implementation
	World_FindObjectsIntersectingCubeInList(&list, World_GetCurrentScanCode(), &vecStartPos, &vecEndPos, nIntersecting, maxEntitiesToFind, (void**)aEntities);
}

void
World_FindObjectsIntersectingAngledCollisionBox(const CBox &boundingBox, const CMatrix &matrix,
                                                  const CVector &position, float fStartX, float fStartY, float fEndX,
                                                  float fEndY, int16 *nEntitiesFound, int16 maxEntitiesToFind,
                                                  CEntity **aEntities, bool bBuildings, bool bVehicles, bool bPeds,
                                                  bool bObjects, bool bDummies)
{
	World_AdvanceCurrentScanCode();
	*nEntitiesFound = 0;
	// Delegate to the C implementation
	int32 nStartX, nStartY, nEndX, nEndY;
	World_SectorRange(fStartX, fStartY, fEndX, fEndY, &nStartX, &nStartY, &nEndX, &nEndY);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = World_GetSector(x, y);
			if(bBuildings) {
				World_FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_BUILDINGS], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				World_FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bVehicles) {
				World_FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
				World_FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bPeds) {
				World_FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_PEDS], boundingBox, matrix, position, nEntitiesFound,
				    maxEntitiesToFind, aEntities);
				World_FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_PEDS_OVERLAP], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bObjects) {
				World_FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS], boundingBox, matrix, position, nEntitiesFound,
				    maxEntitiesToFind, aEntities);
				World_FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
			if(bDummies) {
				World_FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_DUMMIES], boundingBox, matrix, position, nEntitiesFound,
				    maxEntitiesToFind, aEntities);
				World_FindObjectsIntersectingAngledCollisionBoxSectorList(
				    pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP], boundingBox, matrix, position,
				    nEntitiesFound, maxEntitiesToFind, aEntities);
			}
		}
	}
}

void
World_FindObjectsIntersectingAngledCollisionBoxSectorList(CPtrList &list, const CBox &boundingBox,
                                                            const CMatrix &matrix, const CVector &position,
                                                            int16 *nEntitiesFound, int16 maxEntitiesToFind,
                                                            CEntity **aEntities)
{
	for(CPtrNode *pNode = list.first; pNode; pNode = pNode->next) {
		CEntity *pEntity = (CEntity *)pNode->item;
		if(pEntity->m_scanCode != World_GetCurrentScanCode()) {
			pEntity->m_scanCode = World_GetCurrentScanCode();
			CColSphere sphere;
			CVector vecDistance = pEntity->GetPosition() - position;
			sphere.bounds.radius = pEntity->GetBoundRadius();
			sphere.bounds.center = CollisionVector_FromNative(Multiply3x3(vecDistance, matrix));
			if(Collision_TestSphereBox(sphere.bounds, boundingBox) && *nEntitiesFound < maxEntitiesToFind) {
				if(aEntities) aEntities[*nEntitiesFound] = pEntity;
				++*nEntitiesFound;
			}
		}
	}
}

void
World_FindMissionEntitiesIntersectingCube(const CVector &vecStartPos, const CVector &vecEndPos, int16 *nIntersecting,
                                            int16 maxEntitiesToFind, CEntity **aEntities, bool bVehicles, bool bPeds,
                                            bool bObjects)
{
	World_AdvanceCurrentScanCode();
	*nIntersecting = 0;
	// Delegate to the C implementation
	int32 nStartX, nStartY, nEndX, nEndY;
	World_SectorRange(vecStartPos.x, vecStartPos.y, vecEndPos.x, vecEndPos.y, &nStartX, &nStartY, &nEndX, &nEndY);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = World_GetSector(x, y);
			if(bVehicles) {
				World_FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities, true, false);
				World_FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], vecStartPos, vecEndPos,
				    nIntersecting, maxEntitiesToFind, aEntities, true, false);
			}
			if(bPeds) {
				World_FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_PEDS], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities, false, true);
				World_FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_PEDS_OVERLAP], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities, false, true);
			}
			if(bObjects) {
				World_FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities, false, false);
				World_FindMissionEntitiesIntersectingCubeSectorList(
				    pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], vecStartPos, vecEndPos, nIntersecting,
				    maxEntitiesToFind, aEntities, false, false);
			}
		}
	}
}

void
World_FindMissionEntitiesIntersectingCubeSectorList(CPtrList &list, const CVector &vecStartPos,
                                                      const CVector &vecEndPos, int16 *nIntersecting,
                                                      int16 maxEntitiesToFind, CEntity **aEntities, bool bIsVehicleList,
                                                      bool bIsPedList)
{
	// Delegate to the C implementation
	World_FindMissionEntitiesIntersectingCubeInList(&list, World_GetCurrentScanCode(), &vecStartPos, &vecEndPos, nIntersecting, maxEntitiesToFind, (void**)aEntities, bIsVehicleList, bIsPedList);
}

void
World_ClearCarsFromArea(float x1, float y1, float z1, float x2, float y2, float z2)
{
	CVehiclePool *pVehiclePool = Pools_GetVehiclePool();
	// The C implementation picks the vehicles, their removal stays here
	void *selected[NUMVEHICLES * 2];
	int numSelected = World_SelectCarsToClearFromArea(pVehiclePool, x1, y1, z1, x2, y2, z2, selected, ARRAY_SIZE(selected));
	for(int32 s = 0; s < numSelected; s++) {
		CVehicle *pVehicle = (CVehicle*)selected[s];
		if(pVehicle->pDriver) {
			Population_RemovePed(pVehicle->pDriver);
			pVehicle->pDriver = nil;
		}
		for(int32 j = 0; j < pVehicle->m_nNumMaxPassengers; ++j) {
			if(pVehicle->pPassengers[j]) {
				Population_RemovePed(pVehicle->pPassengers[j]);
				pVehicle->pPassengers[j] = nil;
				--pVehicle->m_nNumPassengers;
			}
		}
		CarCtrl_RemoveFromInterestingVehicleList(pVehicle);
		World_Remove(pVehicle);
		// Destroy and release the vehicle without invoking C++ delete.
		pVehicle->~CVehicle();
		// Access raw storage through the C store or pool API
		CPool_Delete(pVehiclePool, pVehicle);
	}
}

void
World_ClearPedsFromArea(float x1, float y1, float z1, float x2, float y2, float z2)
{
	CPedPool *pPedPool = Pools_GetPedPool();
	// The C implementation picks the peds, their removal stays here
	void *selected[NUMPEDS * 2];
	int numSelected = World_SelectPedsToClearFromArea(pPedPool, x1, y1, z1, x2, y2, z2, selected, ARRAY_SIZE(selected));
	for(int32 s = 0; s < numSelected; s++)
		Population_RemovePed((CPed*)selected[s]);
}

void
World_CallOffChaseForArea(float x1, float y1, float x2, float y2)
{
	World_AdvanceCurrentScanCode();
	float fStartX = x1 - 10.0f;
	float fStartY = y1 - 10.0f;
	float fEndX = x2 + 10.0f;
	float fEndY = y2 + 10.0f;
	// Delegate to the C implementation
	int32 nStartX, nStartY, nEndX, nEndY;
	World_SectorRange(fStartX, fStartY, fEndX, fEndY, &nStartX, &nStartY, &nEndX, &nEndY);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = World_GetSector(x, y);
			World_CallOffChaseForAreaSectorListVehicles(pSector->m_lists[ENTITYLIST_VEHICLES], x1, y1, x2,
			                                              y2, fStartX, fStartY, fEndX, fEndY);
			World_CallOffChaseForAreaSectorListVehicles(pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], x1,
			                                              y1, x2, y2, fStartX, fStartY, fEndX, fEndY);
			World_CallOffChaseForAreaSectorListPeds(pSector->m_lists[ENTITYLIST_PEDS], x1, y1, x2, y2);
			World_CallOffChaseForAreaSectorListPeds(pSector->m_lists[ENTITYLIST_PEDS_OVERLAP], x1, y1, x2,
			                                          y2);
		}
	}
}

void
World_CallOffChaseForAreaSectorListVehicles(CPtrList &list, float x1, float y1, float x2, float y2, float fStartX,
                                              float fStartY, float fEndX, float fEndY)
{
	// Delegate to the C implementation
	World_CallOffChaseForVehiclesInList(&list, World_GetCurrentScanCode(), x1, y1, x2, y2, fStartX, fStartY, fEndX, fEndY, FindPlayerVehicle(), (*Timer_GetTimeInMilliseconds()));
}

void
World_CallOffChaseForAreaSectorListPeds(CPtrList &list, float x1, float y1, float x2, float y2)
{
	// The C implementation picks the peds, calling off their pursuit stays here
	void *selected[NUMPEDS * 2];
	int numSelected = World_SelectPedsToCallOffChase(&list, World_GetCurrentScanCode(), x1, y1, x2, y2, FindPlayerPed(), FindPlayerVehicle(), selected, ARRAY_SIZE(selected));
	for(int s = 0; s < numSelected; s++) {
		CPed *pPed = (CPed*)selected[s];
		if(pPed->IsPedInControl()) {
			if(pPed->m_nPedType == PEDTYPE_COP)
				((CCopPed *)pPed)->ClearPursuit();
			else
				pPed->SetIdle();
			pPed->SetObjective(OBJECTIVE_NONE);
		} else {
			pPed->m_prevObjective = OBJECTIVE_NONE;
			pPed->m_nLastPedState = PED_IDLE;
		}
	}
}

void
World_RemoveEntityInsteadOfProcessingIt(CEntity *ent)
{
	if(ent->IsPed()) {
		if(FindPlayerPed() == ent)
			World_Remove(ent);
		else
			Population_RemovePed((CPed *)ent);
	} else {
		World_Remove(ent);
		// Destroy and release the entity without invoking C++ delete.
		if(ent->IsVehicle()) {
			ent->~CEntity();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetVehiclePool(), (CVehicle *)ent);
					} else if(ent->IsObject()) {
			ent->~CEntity();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetObjectPool(), (CObject *)ent);
					} else if(ent->IsBuilding()) {
			bool isTreadable = ((CBuilding *)ent)->GetIsATreadable();
			ent->~CEntity();
			if(isTreadable)
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetTreadablePool(), (CTreadable *)ent);
							else
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetBuildingPool(), (CBuilding *)ent);
						} else if(ent->IsDummy()) {
			ent->~CEntity();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetDummyPool(), (CDummy *)ent);
					}
	}
}

void
World_RemoveFallenPeds(void)
{
	// The C implementation picks the peds and what to do with them, the effects stay here
	void *selected[NUMPEDS * 2];
	int action[NUMPEDS * 2];
	int numSelected = World_SelectFallenPeds(Pools_GetPedPool(), MAP_Z_LOW_LIMIT, selected, action, ARRAY_SIZE(selected));
	for(int s = 0; s < numSelected; s++) {
		CPed *ped = (CPed*)selected[s];
		if(action[s] == 1) {
			int closestNode = ThePaths.FindNodeClosestToCoors(ped->GetPosition(), PATH_PED,
			                                                  999999.9f, false, false);
			CVector newPos = ThePaths.m_pathNodes[closestNode].GetPosition();
			newPos.z += 2.0f;
			ped->Teleport(newPos);
			ped->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
		} else {
			Population_RemovePed(ped);
		}
	}
}

void
World_RemoveFallenCars(void)
{
	// The C implementation picks the vehicles and what to do with them, the effects stay here
	void *selected[NUMVEHICLES * 2];
	int action[NUMVEHICLES * 2];
	int numSelected = World_SelectFallenCars(Pools_GetVehiclePool(), MAP_Z_LOW_LIMIT, FindPlayerVehicle(), selected, action, ARRAY_SIZE(selected));
	for(int s = 0; s < numSelected; s++) {
		CVehicle *veh = (CVehicle*)selected[s];
		if(action[s] == 1) {
			int closestNode = ThePaths.FindNodeClosestToCoors(veh->GetPosition(), PATH_CAR,
			                                                  999999.9f, false, false);
			CVector newPos = ThePaths.m_pathNodes[closestNode].GetPosition();
			newPos.z += 3.0f;
			veh->Teleport(newPos);
			veh->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
		} else {
			World_Remove(veh);
			// Destroy and release the fallen vehicle without invoking C++ delete.
			veh->~CVehicle();
			// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetVehiclePool(), veh);
		}
	}
}

void
World_StopAllLawEnforcersInTheirTracks(void)
{
	// Delegate to the C implementation
	WorldLogic_StopAllLawEnforcersInTheirTracks(Pools_GetVehiclePool());
}

void
World_SetAllCarsCanBeDamaged(bool toggle)
{
	// Delegate to the C implementation
	WorldLogic_SetAllCarsCanBeDamaged(Pools_GetVehiclePool(), toggle);
}

void
World_ExtinguishAllCarFiresInArea(CVector point, float range)
{
	// The C implementation picks the vehicles, extinguishing stays here
	void *selected[NUMVEHICLES * 2];
	int numSelected = World_SelectCarsInRangeOfPoint(Pools_GetVehiclePool(), point.x, point.y, point.z, range, selected, ARRAY_SIZE(selected));
	for(int s = 0; s < numSelected; s++)
		((CVehicle*)selected[s])->ExtinguishCarFire();
}

inline void
AddSteamsFromGround(CPtrList& list)
{
	CPtrNode* pNode = list.first;
	while (pNode) {
		((CEntity*)pNode->item)->AddSteamsFromGround(nil);
		pNode = pNode->next;
	}
}

void
World_AddParticles(void)
{
	for(int32 y = 0; y < NUMSECTORS_Y; y++) {
		for(int32 x = 0; x < NUMSECTORS_X; x++) {
			CSector *pSector = World_GetSector(x, y);
			AddSteamsFromGround(pSector->m_lists[ENTITYLIST_BUILDINGS]);
			AddSteamsFromGround(pSector->m_lists[ENTITYLIST_DUMMIES]);
		}
	}
}

void
World_ShutDown(void)
{
	for(int i = 0; i < NUMSECTORS_X * NUMSECTORS_Y; i++) {
		CSector *pSector = World_GetSector(i % NUMSECTORS_X, i / NUMSECTORS_Y);
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_BUILDINGS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			World_Remove(pEntity);
			// Destroy and release the building without invoking C++ delete.
			bool isTreadable = ((CBuilding *)pEntity)->GetIsATreadable();
			pEntity->~CEntity();
			if(isTreadable)
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetTreadablePool(), (CTreadable *)pEntity);
							else
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetBuildingPool(), (CBuilding *)pEntity);
						}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_VEHICLES].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			World_Remove(pEntity);
			// Destroy and release the vehicle without invoking C++ delete.
			pEntity->~CEntity();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetVehiclePool(), (CVehicle *)pEntity);
					}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_PEDS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			World_Remove(pEntity);
			// Destroy and release the ped without invoking C++ delete.
			pEntity->~CEntity();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetPedPool(), (CPed *)pEntity);
					}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_OBJECTS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			World_Remove(pEntity);
			// Destroy and release the object without invoking C++ delete.
			pEntity->~CEntity();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetObjectPool(), (CObject *)pEntity);
					}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_DUMMIES].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			World_Remove(pEntity);
			// Destroy and release the dummy without invoking C++ delete.
			pEntity->~CEntity();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetDummyPool(), (CDummy *)pEntity);
					}
#ifndef FIX_BUGS
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_BUILDINGS]);
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_DUMMIES]);
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP]);
#endif
	}
	for(int32 i = 0; i < NUM_LEVELS; i++) {
		for(CPtrNode *pNode = World_ms_bigBuildingsList[i].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			// Maybe remove from world here?
			// Destroy and release the big building without invoking C++ delete.
			bool isTreadable = ((CBuilding *)pEntity)->GetIsATreadable();
			pEntity->~CEntity();
			if(isTreadable)
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetTreadablePool(), (CTreadable *)pEntity);
							else
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetBuildingPool(), (CBuilding *)pEntity);
						}
		PtrList_Flush(&World_ms_bigBuildingsList[i]);
	}
	for(int i = 0; i < NUMSECTORS_X * NUMSECTORS_Y; i++) {
		CSector *pSector = World_GetSector(i % NUMSECTORS_X, i / NUMSECTORS_Y);
#ifdef FIX_BUGS
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_BUILDINGS]);
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_DUMMIES]);
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP]);
#endif
		if(pSector->m_lists[ENTITYLIST_BUILDINGS].first) {
			sprintf(gString, "Building list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			PtrList_Flush(&pSector->m_lists[ENTITYLIST_BUILDINGS]);
		}
		if(pSector->m_lists[ENTITYLIST_DUMMIES].first) {
			sprintf(gString, "Dummy list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			PtrList_Flush(&pSector->m_lists[ENTITYLIST_DUMMIES]);
		}
		if(pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP].first) {
			sprintf(gString, "Building overlap list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			PtrList_Flush(&pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
		}
		if(pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP].first) {
			sprintf(gString, "Vehicle overlap list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			PtrList_Flush(&pSector->m_lists[ENTITYLIST_VEHICLES_OVERLAP]);
		}
		if(pSector->m_lists[ENTITYLIST_PEDS_OVERLAP].first) {
			sprintf(gString, "Ped overlap list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			PtrList_Flush(&pSector->m_lists[ENTITYLIST_PEDS_OVERLAP]);
		}
		if(pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP].first) {
			sprintf(gString, "Object overlap list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			PtrList_Flush(&pSector->m_lists[ENTITYLIST_OBJECTS_OVERLAP]);
		}
		if(pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP].first) {
			sprintf(gString, "Dummy overlap list %d,%d not empty\n", i % NUMSECTORS_X, i / NUMSECTORS_Y);
			PtrList_Flush(&pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP]);
		}
	}
	PtrList_Flush(&World_ms_listMovingEntityPtrs);
}

void
World_ClearForRestart(void)
{
	if(CutsceneMgr_HasLoaded()) CutsceneMgr_DeleteCutsceneData();
	ProjectileInfo_RemoveAllProjectiles();
	CObject::DeleteAllTempObjects();
	CObject::DeleteAllMissionObjects();
	Population_ConvertAllObjectsToDummyObjects();
	for(int i = 0; i < NUMSECTORS_X * NUMSECTORS_Y; i++) {
		CSector *pSector = World_GetSector(i % NUMSECTORS_X, i / NUMSECTORS_Y);
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_PEDS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			World_Remove(pEntity);
			// Destroy and release the ped without invoking C++ delete.
			pEntity->~CEntity();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetPedPool(), (CPed *)pEntity);
					}
		for(CPtrNode *pNode = World_GetBigBuildingList(LEVEL_GENERIC).first; pNode; pNode = pNode->next) {
			CVehicle *pVehicle = (CVehicle *)pNode->item;
			if(pVehicle && pVehicle->IsVehicle() && pVehicle->IsPlane()) {
				World_Remove(pVehicle);
				// Destroy and release the plane without invoking C++ delete.
				pVehicle->~CVehicle();
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetVehiclePool(), pVehicle);
							}
		}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_VEHICLES].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			World_Remove(pEntity);
			// Destroy and release the vehicle without invoking C++ delete.
			pEntity->~CEntity();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetVehiclePool(), (CVehicle *)pEntity);
					}
	}
	Pools_CheckPoolsEmpty();
}

void
World_RepositionCertainDynamicObjects()
{
		// Access raw storage through the C store or pool API
	int32 i = CPool_GetSize(Pools_GetDummyPool());
		while(--i >= 0) {
				// Access raw storage through the C store or pool API
		CDummy *dummy = ((CDummy*)CPool_GetSlot(Pools_GetDummyPool(), i));
				if(dummy) { World_RepositionOneObject(dummy); }
	}
}

void
World_RepositionOneObject(CEntity *pEntity)
{
	int16 modelId = pEntity->GetModelIndex();
	if (modelId == MI_PARKINGMETER || modelId == MI_PHONEBOOTH1 || modelId == MI_WASTEBIN ||
		modelId == MI_BIN || modelId == MI_POSTBOX1 || modelId == MI_NEWSSTAND || modelId == MI_TRAFFICCONE ||
		modelId == MI_DUMP1 || modelId == MI_ROADWORKBARRIER1 || modelId == MI_BUSSIGN1 || modelId == MI_NOPARKINGSIGN1 ||
		modelId == MI_PHONESIGN || modelId == MI_FIRE_HYDRANT || modelId == MI_BOLLARDLIGHT ||
		modelId == MI_PARKTABLE || modelId == MI_PARKINGMETER2 || modelId == MI_TELPOLE02 ||
		modelId == MI_PARKBENCH || modelId == MI_BARRIER1 || IsTreeModel(modelId)
		) {
		CVector& position = pEntity->GetMatrix().GetPosition();
		CColModel* pColModel = pEntity->GetColModel();
		float fBoundingBoxMinZ = pColModel->boundingBox.min.z;
		float fHeight = pColModel->boundingBox.max.z - pColModel->boundingBox.min.z;
		if (fHeight < OBJECT_REPOSITION_OFFSET_Z) fHeight = OBJECT_REPOSITION_OFFSET_Z;
		position.z = World_FindGroundZFor3DCoord(position.x, position.y,
			position.z + fHeight, nil) -
			fBoundingBoxMinZ;
		pEntity->GetMatrix().UpdateRW();
		pEntity->UpdateRwFrame();
	} else if(IsLightThatNeedsRepositioning(modelId)) {
		CVector position = pEntity->GetMatrix().GetPosition();
		CColModel* pColModel = pEntity->GetColModel();
		float fBoundingBoxMinZ = pColModel->boundingBox.min.z;
		float fHeight = pColModel->boundingBox.max.z - pColModel->boundingBox.min.z;
		if (fHeight < OBJECT_REPOSITION_OFFSET_Z) fHeight = OBJECT_REPOSITION_OFFSET_Z;
		if (pColModel->numBoxes == 1)
			position = pEntity->GetMatrix() * CVector(
				(pColModel->boxes[0].bounds.min.x + pColModel->boxes[0].bounds.max.x) / 2,
				(pColModel->boxes[0].bounds.min.y + pColModel->boxes[0].bounds.max.y) / 2,
				pColModel->boxes[0].bounds.min.z);
		else if (pColModel->numSpheres > 0) {
			position.z = 1000.0f;
			for (int i = 0; i < pColModel->numSpheres; i++) {
				if (pColModel->spheres[i].bounds.center.z < position.z)
					position = CollisionVector_ToNative(pColModel->spheres[i].bounds.center);
			}
			if (position.z < 1000.0f)
				position = pEntity->GetMatrix() * position;
		}
		pEntity->GetMatrix().GetPosition().z = World_FindGroundZFor3DCoord(position.x, position.y, pEntity->GetMatrix().GetPosition().z + fHeight, nil) - fBoundingBoxMinZ;
		pEntity->GetMatrix().UpdateRW();
		pEntity->UpdateRwFrame();
		
	}
	if(modelId == MI_BUOY) {
		bool bFound = true;
		const CVector &position = pEntity->GetPosition();
		float fGroundZ = World_FindGroundZFor3DCoord(position.x, position.y,
		                                               position.z + OBJECT_REPOSITION_OFFSET_Z, &bFound);
		CColModel *pColModel = pEntity->GetColModel();
		float fHeight = pColModel->boundingBox.max.z - pColModel->boundingBox.min.z;
		pEntity->GetMatrix().GetPosition().z = 0.2f * fHeight + 6.0f - 0.5f * fHeight;
	}
}

void
World_SetCarsOnFire(float x, float y, float z, float radius, CEntity *reason)
{
	// The C implementation picks the vehicles, starting the fire stays here
	void *selected[NUMVEHICLES * 2];
	int numSelected = World_SelectCarsToSetOnFire(Pools_GetVehiclePool(), x, y, z, radius, selected, ARRAY_SIZE(selected));
	for(int s = 0; s < numSelected; s++)
		FireManager_StartFireOn(&gFireManager, (CVehicle*)selected[s], reason, 0.8f, true);
}

void
World_SetPedsChoking(float x, float y, float z, float radius, CEntity* reason)
{
	// The C implementation picks the peds, their reaction stays here
	void *selected[NUMPEDS * 2];
	int numSelected = World_SelectPedsToChoke(Pools_GetPedPool(), x, y, z, radius, selected, ARRAY_SIZE(selected));
	for(int s = 0; s < numSelected; s++) {
		CPed *pPed = (CPed*)selected[s];
		if (!pPed->IsPlayer())
			pPed->SetFlee(CVector2D(x, y), 10000);
#ifdef FIX_BUGS
		pPed->InflictDamage(reason, WEAPONTYPE_TEARGAS, 1.0f, PEDPIECE_TORSO, 0);
#else
		pPed->InflictDamage(nil, WEAPONTYPE_TEARGAS, 1.0f, PEDPIECE_TORSO, 0);
#endif
	}
}

void
World_SetPedsOnFire(float x, float y, float z, float radius, CEntity *reason)
{
	// The C implementation picks the peds, starting the fire stays here
	void *selected[NUMPEDS * 2];
	int numSelected = World_SelectPedsToSetOnFire(Pools_GetPedPool(), x, y, z, radius, selected, ARRAY_SIZE(selected));
	for(int s = 0; s < numSelected; s++)
		FireManager_StartFireOn(&gFireManager, (CPed*)selected[s], reason, 0.8f, true);
}

void
World_RemoveStaticObjects()
{
	for(int i = 0; i < NUMSECTORS_X * NUMSECTORS_Y; i++) {
		CSector *pSector = World_GetSector(i % NUMSECTORS_X, i / NUMSECTORS_Y);
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_BUILDINGS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			World_Remove(pEntity);
			// Destroy and release the static building without invoking C++ delete.
			bool isTreadable = ((CBuilding *)pEntity)->GetIsATreadable();
			pEntity->~CEntity();
			if(isTreadable)
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetTreadablePool(), (CTreadable *)pEntity);
							else
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetBuildingPool(), (CBuilding *)pEntity);
						}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_OBJECTS].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			World_Remove(pEntity);
			// Destroy and release the static object without invoking C++ delete.
			pEntity->~CEntity();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetObjectPool(), (CObject *)pEntity);
					}
		for(CPtrNode *pNode = pSector->m_lists[ENTITYLIST_DUMMIES].first; pNode; pNode = pNode->next) {
			CEntity *pEntity = (CEntity *)pNode->item;
			World_Remove(pEntity);
			// Destroy and release the static dummy without invoking C++ delete.
			pEntity->~CEntity();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetDummyPool(), (CDummy *)pEntity);
					}
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_BUILDINGS]);
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_DUMMIES]);
		PtrList_Flush(&pSector->m_lists[ENTITYLIST_DUMMIES_OVERLAP]);
	}
}

void
World_Process(void)
{
	if(!((*Timer_GetFrameCounter()) & 63)) References_PruneAllReferencesInWorld();

	if(World_bProcessCutsceneOnly) {
		for(int i = 0; i < NUMCUTSCENEOBJECTS; i++) {
			CCutsceneObject *csObj = CutsceneMgr_GetCutsceneObject(i);
			if(csObj && csObj->m_entryInfoList.first) {
				if(csObj->m_rwObject && RwObjectGetType(csObj->m_rwObject) == rpCLUMP &&
				   RpAnimBlendClumpGetFirstAssociation(csObj->GetClump())) {
					if (csObj->IsObject())
						// Use the complete C animation plugin API with explicit overload and render arguments
						RpAnimBlendClumpUpdateAnimations(csObj->GetClump(), Timer_GetTimeStepNonClippedInSeconds(), true);
					else {
						if (!csObj->bOffscreen)
							csObj->bOffscreen = !csObj->GetIsOnScreen();
						RpAnimBlendClumpUpdateAnimations(csObj->GetClump(), Timer_GetTimeStepInSeconds(), !csObj->bOffscreen);
					}
				}
				csObj->ProcessControl();
				csObj->ProcessCollision();
				csObj->GetMatrix().UpdateRW();
				csObj->UpdateRwFrame();
			}
		}
		RecordChase_ProcessControlCars(); // rouz edit (ChatGPT)
		RecordChase_SaveOrRetrieveCarPositions(); // rouz edit (ChatGPT)
	} else {
		for(CPtrNode *node = World_ms_listMovingEntityPtrs.first; node; node = node->next) {
			CEntity *movingEnt = (CEntity *)node->item;
			if(!movingEnt->bRemoveFromWorld && movingEnt->m_rwObject && RwObjectGetType(movingEnt->m_rwObject) == rpCLUMP &&
			   RpAnimBlendClumpGetFirstAssociation(movingEnt->GetClump())) {
				if (movingEnt->IsObject())
					// Use the complete C animation plugin API with explicit overload and render arguments
					RpAnimBlendClumpUpdateAnimations(movingEnt->GetClump(), Timer_GetTimeStepNonClippedInSeconds(), true);
				else {
					if (!movingEnt->bOffscreen)
						movingEnt->bOffscreen = !movingEnt->GetIsOnScreen();
					RpAnimBlendClumpUpdateAnimations(movingEnt->GetClump(), Timer_GetTimeStepInSeconds(), !movingEnt->bOffscreen);
				}
			}
		}
		for(CPtrNode *node = World_ms_listMovingEntityPtrs.first; node; node = node->next) {
			CPhysical *movingEnt = (CPhysical *)node->item;
			if(movingEnt->bRemoveFromWorld) {
				World_RemoveEntityInsteadOfProcessingIt(movingEnt);
			} else {
				movingEnt->ProcessControl();
				if(movingEnt->GetIsStatic()) { movingEnt->RemoveFromMovingList(); }
			}
		}
		World_bForceProcessControl = true;
		for(CPtrNode *node = World_ms_listMovingEntityPtrs.first; node; node = node->next) {
			CPhysical *movingEnt = (CPhysical *)node->item;
			if(movingEnt->bWasPostponed) {
				if(movingEnt->bRemoveFromWorld) {
					World_RemoveEntityInsteadOfProcessingIt(movingEnt);
				} else {
					movingEnt->ProcessControl();
					if(movingEnt->GetIsStatic()) { movingEnt->RemoveFromMovingList(); }
				}
			}
		}
		World_bForceProcessControl = false;
		if(Replay_IsPlayingBack()) { // rouz edit (ChatGPT)
			for(CPtrNode *node = World_ms_listMovingEntityPtrs.first; node; node = node->next) {
				CEntity *movingEnt = (CEntity *)node->item;
				movingEnt->bIsInSafePosition = true;
				movingEnt->GetMatrix().UpdateRW();
				movingEnt->UpdateRwFrame();
			}
		} else {
			World_bNoMoreCollisionTorque = false;
			for(CPtrNode *node = World_ms_listMovingEntityPtrs.first; node; node = node->next) {
				CEntity *movingEnt = (CEntity *)node->item;
				if(!movingEnt->bIsInSafePosition) {
					movingEnt->ProcessCollision();
					movingEnt->GetMatrix().UpdateRW();
					movingEnt->UpdateRwFrame();
				}
			}
			World_bNoMoreCollisionTorque = true;
			for(int i = 0; i < 4; i++) {
				for(CPtrNode *node = World_ms_listMovingEntityPtrs.first; node; node = node->next) {
					CEntity *movingEnt = (CEntity *)node->item;
					if(!movingEnt->bIsInSafePosition) {
						movingEnt->ProcessCollision();
						movingEnt->GetMatrix().UpdateRW();
						movingEnt->UpdateRwFrame();
					}
				}
			}
			for(CPtrNode *node = World_ms_listMovingEntityPtrs.first; node; node = node->next) {
				CEntity *movingEnt = (CEntity *)node->item;
				if(!movingEnt->bIsInSafePosition) {
					movingEnt->bIsStuck = true;
					movingEnt->ProcessCollision();
					movingEnt->GetMatrix().UpdateRW();
					movingEnt->UpdateRwFrame();
					if(!movingEnt->bIsInSafePosition) { movingEnt->bIsStuck = true; }
				}
			}
			World_bSecondShift = false;
			for(CPtrNode *node = World_ms_listMovingEntityPtrs.first; node; node = node->next) {
				CEntity *movingEnt = (CEntity *)node->item;
				if(!movingEnt->bIsInSafePosition) {
					movingEnt->ProcessShift();
					movingEnt->GetMatrix().UpdateRW();
					movingEnt->UpdateRwFrame();
					if(!movingEnt->bIsInSafePosition) { movingEnt->bIsStuck = true; }
				}
			}
			World_bSecondShift = true;
			for(CPtrNode *node = World_ms_listMovingEntityPtrs.first; node; node = node->next) {
				CPhysical *movingEnt = (CPhysical *)node->item;
				if(!movingEnt->bIsInSafePosition) {
					movingEnt->ProcessShift();
					movingEnt->GetMatrix().UpdateRW();
					movingEnt->UpdateRwFrame();
					if(!movingEnt->bIsInSafePosition) {
						movingEnt->bIsStuck = true;
						if(movingEnt->GetStatus() == STATUS_PLAYER) {
							printf("STUCK: Final Step: Player Entity %d Is Stuck\n", movingEnt->GetModelIndex());
							movingEnt->m_vecMoveSpeed *= Pow(0.707f, (*Timer_GetTimeStep()));
							movingEnt->ApplyMoveSpeed();
							movingEnt->ApplyTurnSpeed();
						}
					}
				}
			}
		}
		for(CPtrNode *node = World_ms_listMovingEntityPtrs.first; node; node = node->next) {
			CPed *movingPed = (CPed *)node->item;
			if(movingPed->IsPed()) {
								// Re-sync vehicle-carried peds after all vehicle collision movement has finished.
				if(rouz.glue_on_vehs && !movingPed->bInVehicle && movingPed->bIsStanding &&
				   movingPed->m_pCurrentPhysSurface && movingPed->m_pCurrentPhysSurface->IsVehicle() &&
				   !((CVehicle*)movingPed->m_pCurrentPhysSurface)->IsBoat()) {
					CPhysical *surface = movingPed->m_pCurrentPhysSurface;
					CVector surfacePoint = surface->GetMatrix() * movingPed->surf_rel_origin;
					movingPed->m_vecOffsetFromPhysSurface = surfacePoint - surface->GetPosition();

					CVector carriedPos = surfacePoint;
					carriedPos.z += FEET_OFFSET;
					movingPed->SetPosition(carriedPos);

					CVector surfaceVelocity = surface->GetSpeed(movingPed->m_vecOffsetFromPhysSurface);
					movingPed->m_vecMoveSpeed.x = surfaceVelocity.x + movingPed->m_moved.x;
					movingPed->m_vecMoveSpeed.y = surfaceVelocity.y + movingPed->m_moved.y;
					movingPed->m_vecMoveSpeed.z = surfaceVelocity.z;

					movingPed->GetMatrix().UpdateRW();
					movingPed->UpdateRwFrame();
					movingPed->RemoveAndAdd();
				}
				
				if(movingPed->bInVehicle && movingPed->m_nPedState != PED_EXIT_TRAIN ||
				   movingPed->EnteringCar()) {
					CVehicle *movingCar = movingPed->m_pMyVehicle;
					if(movingCar) {
#ifdef GTA_TRAIN
						if(movingCar->IsTrain()) {
							movingPed->SetPedPositionInTrain();
						} else
#endif
						{
							switch(movingPed->m_nPedState) {
							case PED_ENTER_CAR:
							case PED_CARJACK: movingPed->EnterCar(); break;
							case PED_DRAG_FROM_CAR: movingPed->BeingDraggedFromCar(); break;
							case PED_EXIT_CAR: movingPed->ExitCar(); break;
							case PED_ARRESTED:
								if(movingPed->m_nLastPedState == PED_DRAG_FROM_CAR) {
									movingPed->BeingDraggedFromCar();
									break;
								}
								// fall through
							default: movingPed->SetPedPositionInCar(); break;
							}
						}
						movingPed->GetMatrix().UpdateRW();
						movingPed->UpdateRwFrame();
					} else {
						movingPed->bInVehicle = false;
						movingPed->QuitEnteringCar();
					}
				} else if (movingPed->m_attachedTo) {
					movingPed->PositionAttachedPed();
					movingPed->GetMatrix().UpdateRW();
					movingPed->UpdateRwFrame();
				}
			}
		}
		Messages_Process();
		World_Players[World_PlayerInFocus].Process();
		RecordChase_SaveOrRetrieveCarPositions(); // rouz edit (ChatGPT)
		if(((*Timer_GetFrameCounter()) & 7) == 1) {
			World_RemoveFallenPeds();
		} else if(((*Timer_GetFrameCounter()) & 7) == 5) {
			World_RemoveFallenCars();
		}
	}
}

void
World_TriggerExplosion(const CVector &position, float fRadius, float fPower, CEntity *pCreator,
                         bool bProcessVehicleBombTimer)
{
	CVector2D vecStartPos(position.x - fRadius, position.y - fRadius);
	CVector2D vecEndPos(position.x + fRadius, position.y + fRadius);
	// Delegate to the C implementation
	int32 nStartX, nStartY, nEndX, nEndY;
	World_SectorRangeClamped(vecStartPos.x, vecStartPos.y, vecEndPos.x, vecEndPos.y, &nStartX, &nStartY, &nEndX, &nEndY);
	for(int32 y = nStartY; y <= nEndY; y++) {
		for(int32 x = nStartX; x <= nEndX; x++) {
			CSector *pSector = World_GetSector(x, y);
			World_TriggerExplosionSectorList(pSector->m_lists[ENTITYLIST_VEHICLES], position, fRadius,
			                                   fPower, pCreator, bProcessVehicleBombTimer);
			World_TriggerExplosionSectorList(pSector->m_lists[ENTITYLIST_PEDS], position, fRadius, fPower,
			                                   pCreator, bProcessVehicleBombTimer);
			World_TriggerExplosionSectorList(pSector->m_lists[ENTITYLIST_OBJECTS], position, fRadius,
			                                   fPower, pCreator, bProcessVehicleBombTimer);
		}
	}
}

void
World_TriggerExplosionSectorList(CPtrList &list, const CVector &position, float fRadius, float fPower,
                                   CEntity *pCreator, bool bProcessVehicleBombTimer)
{
	for(CPtrNode *pNode = list.first; pNode; pNode = pNode->next) {
		CPhysical *pEntity = (CPhysical *)pNode->item;
		CVector vecDistance = pEntity->GetPosition() - position;
		float fMagnitude = vecDistance.Magnitude();
		if(fRadius > fMagnitude) {
			CWeapon::BlowUpExplosiveThings(pEntity);
			CPed *pPed = (CPed *)pEntity;
			CObject *pObject = (CObject *)pEntity;
			CVehicle *pVehicle = (CVehicle *)pEntity;
			if(!pEntity->bExplosionProof && (!pEntity->IsPed() || !pPed->bInVehicle)) {
				if(pEntity->GetIsStatic()) {
					if(pEntity->IsObject()) {
						if (fPower > pObject->m_fUprootLimit || IsFence(pObject->GetModelIndex())) {
							if (IsGlass(pObject->GetModelIndex())) {
								Glass_WindowRespondsToExplosion(pObject, position);
							} else {
								pObject->SetIsStatic(false);
								pObject->AddToMovingList();
								int16 modelId = pEntity->GetModelIndex();
								if(modelId != MI_FIRE_HYDRANT ||
								   pObject->bHasBeenDamaged) {
									if(pEntity->IsObject() &&
									   modelId != MI_EXPLODINGBARREL &&
									   modelId != MI_PETROLPUMP && modelId != MI_PETROLPUMP2)
										pObject->bHasBeenDamaged = true;
								} else {
									CVector pos = pEntity->GetPosition();
									pos.z -= 0.5f;
									CParticleObject::AddObject(POBJECT_FIRE_HYDRANT,
									                           pos, true);
									pObject->bHasBeenDamaged = true;
								}
							}
						}
						if(pEntity->GetIsStatic()) {
							float fDamageMultiplier =
							    (fRadius - fMagnitude) * 2.0f / fRadius;
							// Delegate the damage to the C implementation
							float fDamage = World_FireDamage(fDamageMultiplier);
							pObject->ObjectDamage(fDamage);
						}
					} else {
						pEntity->SetIsStatic(false);
						pEntity->AddToMovingList();
					}
				}
				if(!pEntity->GetIsStatic()) {
					// Delegate the damage multiplier and the force of the explosion to the C implementation
					float fDamageMultiplier = World_ExplosionDamageMultiplier(fRadius, fMagnitude);
					CVector forceC;
					World_ExplosionForce(&vecDistance, fPower, pEntity->m_fMass, fDamageMultiplier, fMagnitude, pEntity == FindPlayerPed(), &forceC);
					CVector vecForceDir(forceC.x, forceC.y, forceC.z);
					pEntity->ApplyMoveForce(vecForceDir);
					if(!pEntity->bPedPhysics) {
						float fBoundRadius = pEntity->GetBoundRadius();
						float fDistanceZ = position.z - pEntity->GetPosition().z;
						// Delegate the point of the turn force to the C implementation
						float fPointZ = World_ExplosionTurnPointZ(fDistanceZ, fBoundRadius);
						pEntity->ApplyTurnForce(vecForceDir.x, vecForceDir.y, vecForceDir.z,
						                        0.0f, 0.0f, fPointZ);
					}
					switch(pEntity->GetType()) {
					case ENTITY_TYPE_VEHICLE:
						if(pEntity->GetStatus() == STATUS_SIMPLE) {
							pEntity->SetStatus(STATUS_PHYSICS);
							CarCtrl_SwitchVehicleToRealPhysics(pVehicle);
						}
						pVehicle->InflictDamage(pCreator, WEAPONTYPE_EXPLOSION,
						                        1100.0f * fDamageMultiplier);
						if(bProcessVehicleBombTimer) {
							if(pVehicle->m_nBombTimer) pVehicle->m_nBombTimer /= 10;
						}
						break;
					case ENTITY_TYPE_PED: {
						int8 direction = pPed->GetLocalDirection(-vecForceDir);
						pPed->bIsStanding = false;
						pPed->ApplyMoveForce(0.0, 0.0, 2.0f);
						float fDamage = 250.0f * fDamageMultiplier;
						pPed->InflictDamage(pCreator, WEAPONTYPE_EXPLOSION, fDamage,
						                    PEDPIECE_TORSO, direction);
						if(pPed->m_nPedState != PED_DIE)
							pPed->SetFall(2000,
							              (AnimationId)(direction + ANIM_STD_HIGHIMPACT_FRONT), 0);
						if(pCreator && pCreator->IsPed()) {
							eEventType eventType = EVENT_SHOOT_PED;
							if(pPed->m_nPedType == PEDTYPE_COP) eventType = EVENT_SHOOT_COP;
							Events_RegisterEntity(eventType,EVENT_ENTITY_PED,pEntity,(CPed *)pCreator,10000);
							pPed->RegisterThreatWithGangPeds(pCreator);
						}
						break;
					}
					case ENTITY_TYPE_OBJECT:
						pObject->ObjectDamage(300.0f * fDamageMultiplier);
						break;
					default: break;
					}
				}
			}
		}
	}
}

void
World_UseDetonator(CEntity *pEntity)
{
	// Delegate to the C implementation
	WorldLogic_UseDetonator(Pools_GetVehiclePool(), pEntity);
	ProjectileInfo_RemoveDetonatorProjectiles();
}

bool
World_IsWanderPathClear(CVector const& point1, CVector const& point2, float distance, int maxSteps)
{
	if (General_AbsDiffAbove(point1.z, point2.z, distance))
		return false;
	if (!World_GetIsLineOfSightClear(point1, point2, true, false, false, false, false, false, false))
		return false;
	CVector vecBetween = point2 - point1;
	// Delegate the number of steps to the C implementation
	uint32 nSteps = World_WanderPathSteps(vecBetween.Magnitude(), maxSteps);
	if (nSteps == 0)
		return true;
	vecBetween.Normalise();
	uint32 step = 1;
	for (step = 1; step < nSteps; step++) {
		CVector posThisStep = point1 + vecBetween * step;
		float level;
		if (!WaterLevel_GetWaterLevel(posThisStep, &level, false))
			continue;
		posThisStep.z = level;
		World_AdvanceCurrentScanCode();

		CVector vecCheckedPos(posThisStep.x, posThisStep.y, World_HigherZ(point1.z, point2.z));
		CColPoint colpoint;
		CEntity* entity;
		if (!World_ProcessVerticalLineSector(*World_GetSector(World_GetSectorIndexX(posThisStep.x), World_GetSectorIndexY(posThisStep.y)),
			ColLine_Make(COLLISION_POINT_REF(posThisStep),COLLISION_POINT_REF(vecCheckedPos)), colpoint, entity, true, false, false, false, false, false, nil))
			return false;
	}

	CVector posThisStep = point1;
	World_AdvanceCurrentScanCode();
	CVector vecCheckedPos(posThisStep.x, posThisStep.y, point1.z - 5.0f);

	CColPoint colpoint;
	CEntity* entity;
	if (!World_ProcessVerticalLineSector(*World_GetSector(World_GetSectorIndexX(posThisStep.x), World_GetSectorIndexY(posThisStep.y)),
		ColLine_Make(COLLISION_POINT_REF(posThisStep),COLLISION_POINT_REF(vecCheckedPos)), colpoint, entity, true, false, false, false, false, false, nil))
		return false;

	float heightNextStep = colpoint.point.z + 0.5f;
	for (step = 1; step < nSteps; step++) {
		CVector posThisStep = point1 + vecBetween * step;
		posThisStep.z = heightNextStep;
		World_AdvanceCurrentScanCode();
		CVector vecCheckedPos(posThisStep.x, posThisStep.y, heightNextStep - 2.0f);
		if (!World_ProcessVerticalLineSector(*World_GetSector(World_GetSectorIndexX(posThisStep.x), World_GetSectorIndexY(posThisStep.y)),
			ColLine_Make(COLLISION_POINT_REF(posThisStep),COLLISION_POINT_REF(vecCheckedPos)), colpoint, entity, true, false, false, false, false, false, nil))
			return false;
		if (General_AbsDiffAbove(colpoint.point.z, heightNextStep, 1.0f))
			return false;
		heightNextStep = colpoint.point.z + 0.5f;
	}
	return true;
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
