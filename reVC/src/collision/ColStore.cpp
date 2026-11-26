//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "templates.h"
#include "General.h"
#include "ColStoreLogic.h"
#include "ModelInfo.h"
#include "Streaming.h"
#include "FileLoader.h"
#include "Script.h"
#include "Timer.h"
#include "Camera.h"
#include "Frontend.h"
#include "Physical.h"
#include "ColStore.h"
#include "VarConsole.h"
#include "Pools.h"

//+ rouz edit (ChatGPT)
CPool *ColStore_ms_pColPool;
//- rouz edit (ChatGPT)
#ifndef MASTER
bool bDispColInMem;
#endif

void
ColStore_Initialise(void)
{
	if(ColStore_ms_pColPool == nil)
//+ rouz edit (ChatGPT)
		// Construct the collision store pool without invoking C++ new.
	{
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		ColStore_ms_pColPool = CPool_Create(COLSTORESIZE, sizeof(ColDef), "CollisionFiles");
		//- rouz edit (ChatGPT)
	}
//- rouz edit (ChatGPT)
	ColStore_AddColSlot("generic");	// slot 0. not streamed
#ifndef MASTER
	VarConsole_AddBool(&VarConsole,"Display collision in memory", &bDispColInMem, true); // rouz edit (ChatGPT)
#endif
}

void
ColStore_Shutdown(void)
{
	int i;
	for(i = 0; i < COLSTORESIZE; i++)
		ColStore_RemoveColSlot(i);
	if(ColStore_ms_pColPool)
//+ rouz edit (ChatGPT)
		// Destroy and release the collision store pool without invoking C++ delete.
	{
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPool_Destroy(ColStore_ms_pColPool);
		//- rouz edit (ChatGPT)
	}
//- rouz edit (ChatGPT)
	ColStore_ms_pColPool = nil;
}

int
ColStore_AddColSlot(const char *name)
{
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	ColDef *def = ((ColDef*)CPool_New(ColStore_ms_pColPool));
	//- rouz edit (ChatGPT)
	assert(def);
	// Set the slot up in C
	ColStore_InitSlot(def, name);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	return CPool_GetJustIndex(ColStore_ms_pColPool, def);
	//- rouz edit (ChatGPT)
}

void
ColStore_RemoveColSlot(int slot)
{
	if(ColStore_GetSlot(slot)){
		if(ColStore_GetSlot(slot)->isLoaded)
			ColStore_RemoveCol(slot);
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPool_Delete(ColStore_ms_pColPool, ColStore_GetSlot(slot));
		//- rouz edit (ChatGPT)
	}
}

int
ColStore_FindColSlot(const char *name)
{
	// Delegate to the C implementation
	return ColStoreLogic_FindColSlot(ColStore_ms_pColPool, name);
}

char*
ColStore_GetColName(int32 slot)
{
	return ColStore_GetSlot(slot)->name;
}

CRect&
ColStore_GetBoundingBox(int32 slot)
{
	return ColStore_GetSlot(slot)->bounds;
}

void
ColStore_IncludeModelIndex(int32 slot, int32 modelIndex)
{
	// Delegate to the C implementation
	ColStoreLogic_IncludeModelIndex(ColStore_GetSlot(slot), modelIndex);
}

bool
ColStore_LoadCol(int32 slot, uint8 *buffer, int32 bufsize)
{
	bool success;
	ColDef *def = ColStore_GetSlot(slot);
	if(def->minIndex > def->maxIndex)
		success = FileLoader_LoadCollisionFileFirstTime(buffer, bufsize, slot);
	else
		success = FileLoader_LoadCollisionFile(buffer, bufsize, slot);
	if(success)
		def->isLoaded = true;
	else
		debug("Failed to load Collision\n");
	return success;
}

void
ColStore_RemoveCol(int32 slot)
{
	int id;
	ColStore_GetSlot(slot)->isLoaded = false;
	for(id = 0; id < MODELINFOSIZE; id++){
		CBaseModelInfo *mi = ((CBaseModelInfo*)ModelRegistry_Get(id)); // rouz edit (ChatGPT)
		if(mi){
			CColModel *col = mi->GetColModel();
			if(col && col->level == slot)
				// Invoke the complete C collision owner at the original native lifetime or operation boundary
				ColModel_RemoveCollisionVolumes(col);
		}
	}
}

void
ColStore_LoadAllCollision(void)
{
	int i;
	for(i = 1; i < COLSTORESIZE; i++)
		if(ColStore_GetSlot(i))
			Streaming_RequestCol(i, 0); // rouz edit (ChatGPT)

	Streaming_LoadAllRequestedModels(false);
}

void
ColStore_RemoveAllCollision(void)
{
	int i;
	for(i = 1; i < COLSTORESIZE; i++)
		if(ColStore_GetSlot(i))
			if(Streaming_CanRemoveCol(i)) // rouz edit (ChatGPT)
				Streaming_RemoveCol(i); // rouz edit (ChatGPT)
}

static bool bLoadAtSecondPosition;
static CVector2D secondPosition;

void
ColStore_AddCollisionNeededAtPosn(const CVector2D &pos)
{
	bLoadAtSecondPosition = true;
	secondPosition = pos;
}

void
ColStore_LoadCollision(const CVector2D &pos)
{
	int i;

	if(Streaming_ms_disableStreaming)
		return;

	for(i = 1; i < COLSTORESIZE; i++){
		if(ColStore_GetSlot(i) == nil)
			continue;

		bool wantThisOne = false;

		if(Rect_IsPointInside(&ColStore_GetBoundingBox(i), &pos.x, &pos.y) ||
		   bLoadAtSecondPosition && Rect_IsPointInsideRadius(&ColStore_GetBoundingBox(i), &secondPosition.x, &secondPosition.y, -119.0f) ||
		   strcmp(ColStore_GetColName(i), "yacht") == 0){
			wantThisOne = true;
		}else{
			for (int j = 0; j < MAX_CLEANUP; j++) {
				CPhysical* pEntity = nil;
				cleanup_entity_struct* pCleanup = &TheScripts_MissionCleanUp.m_sEntities[j];
				if (pCleanup->type == CLEANUP_CAR) {
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					pEntity = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), pCleanup->id));
					//- rouz edit (ChatGPT)
					if (!pEntity || pEntity->GetStatus() == STATUS_WRECKED)
						continue;
				}
				else if (pCleanup->type == CLEANUP_CHAR) {
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					pEntity = ((CPed*)CPool_GetAt(Pools_GetPedPool(), pCleanup->id));
					//- rouz edit (ChatGPT)
					if (!pEntity || ((CPed*)pEntity)->DyingOrDead())
						continue;
				}
				if (pEntity && !pEntity->bDontLoadCollision && !pEntity->bIsFrozen) {
					// Retain the original conversion before the collision point test
                    const CVector2D collisionPoint(pEntity->GetPosition());
                    if (Rect_IsPointInsideRadius(&ColStore_GetBoundingBox(i), &collisionPoint.x, &collisionPoint.y, -80.0f))
						wantThisOne = true;
				}
			}
		}

		if(wantThisOne)
			Streaming_RequestCol(i, STREAMFLAGS_PRIORITY); // rouz edit (ChatGPT)
		else
			Streaming_RemoveCol(i); // rouz edit (ChatGPT)
	}
	bLoadAtSecondPosition = false;
}

void
ColStore_RequestCollision(const CVector2D &pos)
{
	int i;

	for(i = 1; i < COLSTORESIZE; i++)
		if(ColStore_GetSlot(i) && Rect_IsPointInsideRadius(&ColStore_GetBoundingBox(i), &pos.x, &pos.y, -115.0f))
			Streaming_RequestCol(i, STREAMFLAGS_PRIORITY); // rouz edit (ChatGPT)
}

void
ColStore_EnsureCollisionIsInMemory(const CVector2D &pos)
{
	int i;

	if(Streaming_ms_disableStreaming)
		return;

	for(i = 1; i < COLSTORESIZE; i++)
		if(ColStore_GetSlot(i) && Rect_IsPointInsideRadius(&ColStore_GetBoundingBox(i), &pos.x, &pos.y, -110.0f) &&
		   !Streaming_HasColLoaded(i)){ // rouz edit (ChatGPT)
			Streaming_RequestCol(i, 0); // rouz edit (ChatGPT)
			if(TheCamera.GetScreenFadeStatus() == FADE_0)
				FrontEndMenuManager.MessageScreen("LOADCOL", false);
			Timer_Suspend();
			Streaming_LoadAllRequestedModels(false);
			Timer_Resume();
		}
}

bool
ColStore_HasCollisionLoaded(const CVector2D &pos)
{
	// Delegate to the C implementation
	return ColStoreLogic_HasCollisionLoaded(ColStore_ms_pColPool, COLSTORESIZE, pos.x, pos.y);
}

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
