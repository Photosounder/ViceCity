#include "common.h"

#include "templates.h"
#include "General.h"
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
CPool *CColStore::ms_pColPool;
//- rouz edit (ChatGPT)
#ifndef MASTER
bool bDispColInMem;
#endif

void
CColStore::Initialise(void)
{
	if(ms_pColPool == nil)
//+ rouz edit (ChatGPT)
		// Construct the collision store pool without invoking C++ new.
	{
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		ms_pColPool = CPool_Create(COLSTORESIZE, sizeof(ColDef), "CollisionFiles");
		//- rouz edit (ChatGPT)
	}
//- rouz edit (ChatGPT)
	AddColSlot("generic");	// slot 0. not streamed
#ifndef MASTER
	VarConsole.Add("Display collision in memory", &bDispColInMem, true);
#endif
}

void
CColStore::Shutdown(void)
{
	int i;
	for(i = 0; i < COLSTORESIZE; i++)
		RemoveColSlot(i);
	if(ms_pColPool)
//+ rouz edit (ChatGPT)
		// Destroy and release the collision store pool without invoking C++ delete.
	{
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPool_Destroy(ms_pColPool);
		//- rouz edit (ChatGPT)
	}
//- rouz edit (ChatGPT)
	ms_pColPool = nil;
}

int
CColStore::AddColSlot(const char *name)
{
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	ColDef *def = ((ColDef*)CPool_New(ms_pColPool));
	//- rouz edit (ChatGPT)
	assert(def);
	def->isLoaded = false;
	def->unused = 0;
	def->bounds.left = 1000000.0f;
	def->bounds.top = 1000000.0f;
	def->bounds.right = -1000000.0f;
	def->bounds.bottom = -1000000.0f;
	def->minIndex = INT16_MAX;
	def->maxIndex = INT16_MIN;
	strcpy(def->name, name);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	return CPool_GetJustIndex(ms_pColPool, def);
	//- rouz edit (ChatGPT)
}

void
CColStore::RemoveColSlot(int slot)
{
	if(GetSlot(slot)){
		if(GetSlot(slot)->isLoaded)
			RemoveCol(slot);
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPool_Delete(ms_pColPool, GetSlot(slot));
		//- rouz edit (ChatGPT)
	}
}

int
CColStore::FindColSlot(const char *name)
{
	ColDef *def;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int size = CPool_GetSize(ms_pColPool);
	//- rouz edit (ChatGPT)
	for(int i = 0; i < size; i++){
		def = GetSlot(i);
		if(def && !CGeneral::faststricmp(def->name, name))
			return i;
	}
	return -1;
}

char*
CColStore::GetColName(int32 slot)
{
	return GetSlot(slot)->name;
}

CRect&
CColStore::GetBoundingBox(int32 slot)
{
	return GetSlot(slot)->bounds;
}

void
CColStore::IncludeModelIndex(int32 slot, int32 modelIndex)
{
	ColDef *def = GetSlot(slot);
	if(modelIndex < def->minIndex)
		def->minIndex = modelIndex;
	if(modelIndex > def->maxIndex)
		def->maxIndex = modelIndex;
}

bool
CColStore::LoadCol(int32 slot, uint8 *buffer, int32 bufsize)
{
	bool success;
	ColDef *def = GetSlot(slot);
	if(def->minIndex > def->maxIndex)
		success = CFileLoader::LoadCollisionFileFirstTime(buffer, bufsize, slot);
	else
		success = CFileLoader::LoadCollisionFile(buffer, bufsize, slot);
	if(success)
		def->isLoaded = true;
	else
		debug("Failed to load Collision\n");
	return success;
}

void
CColStore::RemoveCol(int32 slot)
{
	int id;
	GetSlot(slot)->isLoaded = false;
	for(id = 0; id < MODELINFOSIZE; id++){
		CBaseModelInfo *mi = CModelInfo::GetModelInfo(id);
		if(mi){
			CColModel *col = mi->GetColModel();
			if(col && col->level == slot)
				col->RemoveCollisionVolumes();
		}
	}
}

void
CColStore::LoadAllCollision(void)
{
	int i;
	for(i = 1; i < COLSTORESIZE; i++)
		if(GetSlot(i))
			CStreaming::RequestCol(i, 0);

	CStreaming::LoadAllRequestedModels(false);
}

void
CColStore::RemoveAllCollision(void)
{
	int i;
	for(i = 1; i < COLSTORESIZE; i++)
		if(GetSlot(i))
			if(CStreaming::CanRemoveCol(i))
				CStreaming::RemoveCol(i);
}

static bool bLoadAtSecondPosition;
static CVector2D secondPosition;

void
CColStore::AddCollisionNeededAtPosn(const CVector2D &pos)
{
	bLoadAtSecondPosition = true;
	secondPosition = pos;
}

void
CColStore::LoadCollision(const CVector2D &pos)
{
	int i;

	if(CStreaming::ms_disableStreaming)
		return;

	for(i = 1; i < COLSTORESIZE; i++){
		if(GetSlot(i) == nil)
			continue;

		bool wantThisOne = false;

		if(GetBoundingBox(i).IsPointInside(pos) ||
		   bLoadAtSecondPosition && GetBoundingBox(i).IsPointInside(secondPosition, -119.0f) ||
		   strcmp(GetColName(i), "yacht") == 0){
			wantThisOne = true;
		}else{
			for (int j = 0; j < MAX_CLEANUP; j++) {
				CPhysical* pEntity = nil;
				cleanup_entity_struct* pCleanup = &CTheScripts::MissionCleanUp.m_sEntities[j];
				if (pCleanup->type == CLEANUP_CAR) {
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					pEntity = ((CVehicle*)CPool_GetAt(CPools::GetVehiclePool(), pCleanup->id));
					//- rouz edit (ChatGPT)
					if (!pEntity || pEntity->GetStatus() == STATUS_WRECKED)
						continue;
				}
				else if (pCleanup->type == CLEANUP_CHAR) {
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					pEntity = ((CPed*)CPool_GetAt(CPools::GetPedPool(), pCleanup->id));
					//- rouz edit (ChatGPT)
					if (!pEntity || ((CPed*)pEntity)->DyingOrDead())
						continue;
				}
				if (pEntity && !pEntity->bDontLoadCollision && !pEntity->bIsFrozen) {
					if (GetBoundingBox(i).IsPointInside(pEntity->GetPosition(), -80.0f))
						wantThisOne = true;
				}
			}
		}

		if(wantThisOne)
			CStreaming::RequestCol(i, STREAMFLAGS_PRIORITY);
		else
			CStreaming::RemoveCol(i);
	}
	bLoadAtSecondPosition = false;
}

void
CColStore::RequestCollision(const CVector2D &pos)
{
	int i;

	for(i = 1; i < COLSTORESIZE; i++)
		if(GetSlot(i) && GetBoundingBox(i).IsPointInside(pos, -115.0f))
			CStreaming::RequestCol(i, STREAMFLAGS_PRIORITY);
}

void
CColStore::EnsureCollisionIsInMemory(const CVector2D &pos)
{
	int i;

	if(CStreaming::ms_disableStreaming)
		return;

	for(i = 1; i < COLSTORESIZE; i++)
		if(GetSlot(i) && GetBoundingBox(i).IsPointInside(pos, -110.0f) &&
		   !CStreaming::HasColLoaded(i)){
			CStreaming::RequestCol(i, 0);
			if(TheCamera.GetScreenFadeStatus() == FADE_0)
				FrontEndMenuManager.MessageScreen("LOADCOL", false);
			CTimer::Suspend();
			CStreaming::LoadAllRequestedModels(false);
			CTimer::Resume();
		}
}

bool
CColStore::HasCollisionLoaded(const CVector2D &pos)
{
	int i;

	for(i = 1; i < COLSTORESIZE; i++)
		if(GetSlot(i) && GetBoundingBox(i).IsPointInside(pos, -115.0f) &&
		   !GetSlot(i)->isLoaded)
			return false;
	return true;
}
