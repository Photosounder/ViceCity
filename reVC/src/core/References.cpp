//+ rouz edit (ChatGPT)
#include "common.h"
#include "World.h"
#include "Vehicle.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "References_backend.h"

bool References_BackendIsBuilding(CEntity *entity)
{
	// Read the original entity type through its typed owner
	return entity->IsBuilding();
}
CReference **References_BackendFirst(CEntity *entity)
{
	// Expose the live list field without duplicating the entity layout
	return &entity->m_pFirstReference;
}
CEntity *References_BackendPlayerVehicle(void)
{
	// Preserve each original player query and derived-to-base conversion
	return FindPlayerVehicle();
}
CPlayerPed *References_BackendPlayerPed(void)
{
	// Preserve each original player-ped query
	return FindPlayerPed();
}
CEntity *References_BackendPlayerEntity(CPlayerPed *ped)
{
	// Convert the original player type through its actual inheritance
	return ped;
}
void References_BackendRestorePlayer(CPlayerPed *ped)
{
	// Restore the live focused-player field at the original write point
	CWorld::Players[CWorld::PlayerInFocus].m_pPed = ped;
}
CEntity **References_BackendPlayerSlot(void)
{
	// Return the same slot address and cast used by the original registration
	return (CEntity **)&CWorld::Players[CWorld::PlayerInFocus].m_pPed;
}
CPool *References_BackendPedPool(void)
{
	// Query the live pedestrian pool at each original access point
	return CPools::GetPedPool();
}
CPool *References_BackendVehiclePool(void)
{
	// Query the live vehicle pool at each original access point
	return CPools::GetVehiclePool();
}
CPool *References_BackendObjectPool(void)
{
	// Query the live object pool at each original access point
	return CPools::GetObjectPool();
}
CEntity *References_BackendPed(void *slot)
{
	// Convert raw pedestrian storage through its actual owner type
	return (CPed *)slot;
}
CEntity *References_BackendVehicle(void *slot)
{
	// Convert raw vehicle storage through its actual owner type
	return (CVehicle *)slot;
}
CEntity *References_BackendObject(void *slot)
{
	// Convert raw object storage through its actual owner type
	return (CObject *)slot;
}

void CEntity::RegisterReference(CEntity **pent)
{
	// Enter the complete C registration algorithm
	References_RegisterReference(this, pent);
}
void CEntity::CleanUpOldReference(CEntity **pent)
{
	// Enter the complete C cleanup algorithm
	References_CleanUpOldReference(this, pent);
}
void CEntity::ResolveReferences(void)
{
	// Enter the complete C resolution algorithm
	References_ResolveReferences(this);
}
void CEntity::PruneReferences(void)
{
	// Enter the complete C pruning algorithm
	References_PruneReferences(this);
}
//- rouz edit (ChatGPT)
