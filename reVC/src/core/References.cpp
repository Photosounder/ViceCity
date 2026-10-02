#include "common.h"

#include "World.h"
#include "Vehicle.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "References.h"

CReference CReferences::aRefs[NUMREFERENCES];
CReference *CReferences::pEmptyList;

void
CReferences::Init(void)
{
	int i;
	pEmptyList = &aRefs[0];
	for(i = 0; i < NUMREFERENCES; i++){
		aRefs[i].pentity = nil;
		aRefs[i].next = &aRefs[i+1];
	}
	aRefs[NUMREFERENCES-1].next = nil;
}

void
CEntity::RegisterReference(CEntity **pent)
{
	if(IsBuilding())
		return;
	CReference *ref;
	// check if already registered
	for(ref = m_pFirstReference; ref; ref = ref->next)
		if(ref->pentity == pent)
			return;
	// have to allocate new reference
	ref = CReferences::pEmptyList;
	if(ref){
		CReferences::pEmptyList = ref->next;

		ref->pentity = pent;
		ref->next = m_pFirstReference;
		m_pFirstReference = ref;
	}
}

// Clean up the reference from *pent -> 'this'
void
CEntity::CleanUpOldReference(CEntity **pent)
{
	CReference* ref, ** lastnextp;
	lastnextp = &m_pFirstReference;
	for (ref = m_pFirstReference; ref; ref = ref->next) {
		if (ref->pentity == pent) {
			*lastnextp = ref->next;
			ref->next = CReferences::pEmptyList;
			CReferences::pEmptyList = ref;
			break;
		}
		lastnextp = &ref->next;
	}
}

// Clear all references to this entity
void
CEntity::ResolveReferences(void)
{
	CReference *ref;
	// clear pointers to this entity
	for(ref = m_pFirstReference; ref; ref = ref->next)
		if(*ref->pentity == this)
			*ref->pentity = nil;
	// free list
	if(m_pFirstReference){
		for(ref = m_pFirstReference; ref->next; ref = ref->next)
			;
		ref->next = CReferences::pEmptyList;
		CReferences::pEmptyList = m_pFirstReference;
		m_pFirstReference = nil;
	}
}

// Free all references that no longer point to this entity
void
CEntity::PruneReferences(void)
{
	CReference *ref, *next, **lastnextp;
	lastnextp = &m_pFirstReference;
	for(ref = m_pFirstReference; ref; ref = next){
		next = ref->next;
		if(*ref->pentity == this)
			lastnextp = &ref->next;
		else{
			*lastnextp = ref->next;
			ref->next = CReferences::pEmptyList;
			CReferences::pEmptyList = ref;
		}
	}
}

void
CReferences::RemoveReferencesToPlayer(void)
{
	if(FindPlayerVehicle())
		FindPlayerVehicle()->ResolveReferences();
#ifdef FIX_BUGS
	if (FindPlayerPed()) {
		CPlayerPed* pPlayerPed = FindPlayerPed();
		FindPlayerPed()->ResolveReferences();
		CWorld::Players[CWorld::PlayerInFocus].m_pPed = pPlayerPed;
		pPlayerPed->RegisterReference((CEntity**)&CWorld::Players[CWorld::PlayerInFocus].m_pPed);
	}
#else
	if(FindPlayerPed())
		FindPlayerPed()->ResolveReferences();
#endif
}

void
CReferences::PruneAllReferencesInWorld(void)
{
	int i;
	CEntity *e;

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	i = CPool_GetSize(CPools::GetPedPool());
	//- rouz edit (ChatGPT)
	while(--i >= 0){
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		e = ((CPed*)CPool_GetSlot(CPools::GetPedPool(), i));
		//- rouz edit (ChatGPT)
		if(e)
			e->PruneReferences();
	}

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	i = CPool_GetSize(CPools::GetVehiclePool());
	//- rouz edit (ChatGPT)
	while(--i >= 0){
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		e = ((CVehicle*)CPool_GetSlot(CPools::GetVehiclePool(), i));
		//- rouz edit (ChatGPT)
		if(e)
			e->PruneReferences();
	}

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	i = CPool_GetSize(CPools::GetObjectPool());
	//- rouz edit (ChatGPT)
	while(--i >= 0){
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		e = ((CObject*)CPool_GetSlot(CPools::GetObjectPool(), i));
		//- rouz edit (ChatGPT)
		if(e)
			e->PruneReferences();
	}
}
