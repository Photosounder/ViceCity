"""Relocate every reference algorithm while retaining typed owner access."""
import re
import object_names_migration as objects
import antennas_migration as antennas

HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "config.h"
#ifdef __cplusplus
class CEntity;
#else
typedef struct CEntity CEntity;
#endif

typedef struct CReference {
	struct CReference *next;
	CEntity **pentity;
} CReference;
#ifdef __cplusplus
extern "C" {
#endif
extern CReference References_aRefs[NUMREFERENCES];
extern CReference *References_pEmptyList;
void References_Init(void);
void References_RegisterReference(CEntity *entity, CEntity **pent);
void References_CleanUpOldReference(CEntity *entity, CEntity **pent);
void References_ResolveReferences(CEntity *entity);
void References_PruneReferences(CEntity *entity);
void References_RemoveReferencesToPlayer(void);
void References_PruneAllReferencesInWorld(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#include "References.h"
#include "Pool.h"
#ifdef __cplusplus
class CPlayerPed;
extern "C" {
#else
typedef struct CPlayerPed CPlayerPed;
#endif
bool References_BackendIsBuilding(CEntity *entity);
CReference **References_BackendFirst(CEntity *entity);
CEntity *References_BackendPlayerVehicle(void);
CPlayerPed *References_BackendPlayerPed(void);
CEntity *References_BackendPlayerEntity(CPlayerPed *ped);
void References_BackendRestorePlayer(CPlayerPed *ped);
CEntity **References_BackendPlayerSlot(void);
CPool *References_BackendPedPool(void);
CPool *References_BackendVehiclePool(void);
CPool *References_BackendObjectPool(void);
CEntity *References_BackendPed(void *slot);
CEntity *References_BackendVehicle(void *slot);
CEntity *References_BackendObject(void *slot);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

HOST='''//+ rouz edit (ChatGPT)
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
'''

def calls(source):
    # Replace the removed static class and every direct pool access
    if 'class CReferences\n' in source:return HEADER
    if 'CReference CReferences::aRefs[NUMREFERENCES];' in source:return HOST
    return antennas.calls(re.sub(r'\bCReferences::(\w+)',r'References_\1',source))

def kernel(source):
    # Relocate all seven complete bodies from the immutable C++ source
    source=source[source.index('CReference CReferences::aRefs'):]
    source=re.sub(r'\bCReferences::(\w+)',r'References_\1',source)
    source=re.sub(r'(?<![\w:])\bpEmptyList\b','References_pEmptyList',source)
    source=re.sub(r'(?<![\w:])\baRefs\b','References_aRefs',source)
    for name,args in [('RegisterReference','CEntity **pent'),('CleanUpOldReference','CEntity **pent'),('ResolveReferences','void'),('PruneReferences','void')]:
        signature='CEntity *entity'+(', '+args if args!='void' else '')
        source=source.replace('CEntity::'+name+'('+args+')','References_'+name+'('+signature+')')
    source=source.replace('if(IsBuilding())','if(References_BackendIsBuilding(entity))')
    source=source.replace('CReference *ref;\n\t// check','CReference **first = References_BackendFirst(entity);\n\tCReference *ref;\n\t// check')
    source=source.replace('CReference* ref, ** lastnextp;','CReference **first = References_BackendFirst(entity);\n\tCReference* ref, ** lastnextp;')
    source=source.replace('CReference *ref;\n\t// clear','CReference **first = References_BackendFirst(entity);\n\tCReference *ref;\n\t// clear')
    source=source.replace('CReference *ref, *next, **lastnextp;','CReference **first = References_BackendFirst(entity);\n\tCReference *ref, *next, **lastnextp;')
    source=source.replace('&m_pFirstReference','first').replace('m_pFirstReference','(*first)').replace('== this','== entity')
    source=source.replace('if(FindPlayerVehicle())','if(References_BackendPlayerVehicle())')
    source=source.replace('FindPlayerVehicle()->ResolveReferences();','References_ResolveReferences(References_BackendPlayerVehicle());')
    source=source.replace('FindPlayerPed()->ResolveReferences();','References_ResolveReferences(References_BackendPlayerEntity(References_BackendPlayerPed()));')
    source=source.replace('FindPlayerPed()','References_BackendPlayerPed()')
    source=source.replace('CWorld::Players[CWorld::PlayerInFocus].m_pPed = pPlayerPed;','References_BackendRestorePlayer(pPlayerPed);')
    source=source.replace('pPlayerPed->RegisterReference((CEntity**)&CWorld::Players[CWorld::PlayerInFocus].m_pPed);','References_RegisterReference(References_BackendPlayerEntity(pPlayerPed), References_BackendPlayerSlot());')
    for owner in ['Ped','Vehicle','Object']:
        source=source.replace('CPools::Get'+owner+'Pool()','References_Backend'+owner+'Pool()')
        source=source.replace('((C'+owner+'*)CPool_GetSlot(References_Backend'+owner+'Pool(), i))','References_Backend'+owner+'(CPool_GetSlot(References_Backend'+owner+'Pool(), i))')
    source=source.replace('e->PruneReferences();','References_PruneReferences(e);').replace('nil','NULL')
    source=re.sub(r'\t*//[+-] rouz edit \(ChatGPT\)\n','',source)
    comments={'Init':'Link every original pool record before terminating the free list','RegisterReference':'Retain building rejection and duplicate detection before consuming the free head','CleanUpOldReference':'Remove only the first matching pointer address and return its record','ResolveReferences':'Clear only pointers still equal to this owner and return the whole chain','PruneReferences':'Return stale records in their original traversal order','RemoveReferencesToPlayer':'Preserve repeated queries and the focused-player slot restoration','PruneAllReferencesInWorld':'Visit live pedestrian, vehicle and object pools in descending slot order'}
    for name,comment in comments.items():
        source=re.sub(r'(References_'+name+r'\([^\n]*\)\n\{)',r'\1\n\t// '+comment,source)
    source=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original list update and traversal order\n',source,flags=re.M)
    assert 'CReferences' not in source and 'CEntity::' not in source and '->PruneReferences' not in source
    return '//+ rouz edit (ChatGPT)\n#include <stddef.h>\n#include "References_backend.h"\n\n'+source.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Add the C11 unit before assigning distinct objects to the new pair
    if '..\\src\\core\\References.c"' not in source:
        entry='<ClCompile Include="..\\src\\core\\References.cpp" />'
        addition='<ClCompile Include="..\\src\\core\\References.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
        source=source.replace(entry,entry+'\n    '+addition)
        entry='<ClCompile Include="..\\src\\core\\References.cpp">\n      <Filter>core</Filter>\n    </ClCompile>'
        source=source.replace(entry,entry+'\n    '+entry.replace('References.cpp','References.c'))
    if '..\\src\\core\\References_backend.h"' not in source:
        for entry in ['<ClInclude Include="..\\src\\core\\References.h" />','<ClInclude Include="..\\src\\core\\References.h">\n      <Filter>core</Filter>\n    </ClInclude>']:
            source=source.replace(entry,entry+'\n    '+entry.replace('References.h','References_backend.h'))
    return antennas.project(objects.project(source))
