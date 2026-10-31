//+ rouz edit (ChatGPT)
#include <stddef.h>
#include "References_backend.h"

CReference References_aRefs[NUMREFERENCES];
CReference *References_pEmptyList;

void
References_Init(void)
{
	// Link every original pool record before terminating the free list
	int i;
	References_pEmptyList = &References_aRefs[0];
	for(i = 0; i < NUMREFERENCES; i++){
		// Preserve the original list update and traversal order
		References_aRefs[i].pentity = NULL;
		References_aRefs[i].next = &References_aRefs[i+1];
	}
	References_aRefs[NUMREFERENCES-1].next = NULL;
}

void
References_RegisterReference(CEntity *entity, CEntity **pent)
{
	// Retain building rejection and duplicate detection before consuming the free head
	if(References_BackendIsBuilding(entity))
		return;
	CReference **first = References_BackendFirst(entity);
	CReference *ref;
	// check if already registered
	for(ref = (*first); ref; ref = ref->next)
		if(ref->pentity == pent)
			return;
	// have to allocate new reference
	ref = References_pEmptyList;
	if(ref){
		// Preserve the original list update and traversal order
		References_pEmptyList = ref->next;

		ref->pentity = pent;
		ref->next = (*first);
		(*first) = ref;
	}
}

// Clean up the reference from *pent -> 'this'
void
References_CleanUpOldReference(CEntity *entity, CEntity **pent)
{
	// Remove only the first matching pointer address and return its record
	CReference **first = References_BackendFirst(entity);
	CReference* ref, ** lastnextp;
	lastnextp = first;
	for (ref = (*first); ref; ref = ref->next) {
		// Preserve the original list update and traversal order
		if (ref->pentity == pent) {
			// Preserve the original list update and traversal order
			*lastnextp = ref->next;
			ref->next = References_pEmptyList;
			References_pEmptyList = ref;
			break;
		}
		lastnextp = &ref->next;
	}
}

// Clear all references to this entity
void
References_ResolveReferences(CEntity *entity)
{
	// Clear only pointers still equal to this owner and return the whole chain
	CReference **first = References_BackendFirst(entity);
	CReference *ref;
	// clear pointers to this entity
	for(ref = (*first); ref; ref = ref->next)
		if(*ref->pentity == entity)
			*ref->pentity = NULL;
	// free list
	if((*first)){
		// Preserve the original list update and traversal order
		for(ref = (*first); ref->next; ref = ref->next)
			;
		ref->next = References_pEmptyList;
		References_pEmptyList = (*first);
		(*first) = NULL;
	}
}

// Free all references that no longer point to this entity
void
References_PruneReferences(CEntity *entity)
{
	// Return stale records in their original traversal order
	CReference **first = References_BackendFirst(entity);
	CReference *ref, *next, **lastnextp;
	lastnextp = first;
	for(ref = (*first); ref; ref = next){
		// Preserve the original list update and traversal order
		next = ref->next;
		if(*ref->pentity == entity)
			lastnextp = &ref->next;
		else{
			// Preserve the original list update and traversal order
			*lastnextp = ref->next;
			ref->next = References_pEmptyList;
			References_pEmptyList = ref;
		}
	}
}

void
References_RemoveReferencesToPlayer(void)
{
	// Preserve repeated queries and the focused-player slot restoration
	if(References_BackendPlayerVehicle())
		References_ResolveReferences(References_BackendPlayerVehicle());
#ifdef FIX_BUGS
	if (References_BackendPlayerPed()) {
		// Preserve the original list update and traversal order
		CPlayerPed* pPlayerPed = References_BackendPlayerPed();
		References_ResolveReferences(References_BackendPlayerEntity(References_BackendPlayerPed()));
		References_BackendRestorePlayer(pPlayerPed);
		References_RegisterReference(References_BackendPlayerEntity(pPlayerPed), References_BackendPlayerSlot());
	}
#else
	if(References_BackendPlayerPed())
		References_ResolveReferences(References_BackendPlayerEntity(References_BackendPlayerPed()));
#endif
}

void
References_PruneAllReferencesInWorld(void)
{
	// Visit live pedestrian, vehicle and object pools in descending slot order
	int i;
	CEntity *e;

	// Access raw storage through the C store or pool API
	i = CPool_GetSize(References_BackendPedPool());
	while(--i >= 0){
		// Preserve the original list update and traversal order
		// Access raw storage through the C store or pool API
		e = References_BackendPed(CPool_GetSlot(References_BackendPedPool(), i));
		if(e)
			References_PruneReferences(e);
	}

	// Access raw storage through the C store or pool API
	i = CPool_GetSize(References_BackendVehiclePool());
	while(--i >= 0){
		// Preserve the original list update and traversal order
		// Access raw storage through the C store or pool API
		e = References_BackendVehicle(CPool_GetSlot(References_BackendVehiclePool(), i));
		if(e)
			References_PruneReferences(e);
	}

	// Access raw storage through the C store or pool API
	i = CPool_GetSize(References_BackendObjectPool());
	while(--i >= 0){
		// Preserve the original list update and traversal order
		// Access raw storage through the C store or pool API
		e = References_BackendObject(CPool_GetSlot(References_BackendObjectPool(), i));
		if(e)
			References_PruneReferences(e);
	}
}
//- rouz edit (ChatGPT)
