"""Move the full stored-car record and its store/restore policy into C11."""
import re
from doors_migration import method
import object_names_migration as objects
OPERATIONS=['Init','Clear','HasCar','Assign','StoreCar','RestoreCar']
RENAMES={}
FIELDS=['m_nModelIndex','m_vecPos','m_vecAngle','m_nFlags','m_nPrimaryColor','m_nSecondaryColor','m_nRadioStation','m_nVariationA','m_nVariationB','m_nCarBombType']
BRIDGES=[]

def bridge(name,result,args,body):
    # Describe one typed native property, allocation or engine operation
    BRIDGES.append((name,result,args,body))

bridge('ModelIndex','int32_t','CVehicle *vehicle','return vehicle->GetModelIndex();')
for name,expression in [('Position','vehicle->GetPosition()'),('Forward','vehicle->GetForward()')]:
    bridge(name,'StoredCarVector','CVehicle *vehicle','const CVector &value = '+expression+';\n    const StoredCarVector result = { value.x, value.y, value.z };\n    return result;')
for name,result,field in [('PrimaryColor','uint8_t','m_currentColour1'),('SecondaryColor','uint8_t','m_currentColour2'),('RadioStation','uint8_t','m_nRadioStation'),('VariationA','int8_t','m_aExtras[0]'),('VariationB','int8_t','m_aExtras[1]')]:bridge(name,result,'CVehicle *vehicle','return vehicle->'+field+';')
for name in ['BulletProof','FireProof','ExplosionProof','CollisionProof','MeleeProof']:
    bridge(name,'bool','CVehicle *vehicle','return vehicle->b'+name+';')
    bridge('Set'+name,'void','CVehicle *vehicle, bool value','vehicle->b'+name+' = value;')
for name in ['Car','Bike']:bridge('Is'+name,'bool','CVehicle *vehicle','return vehicle->Is'+name+'();')
bridge('BombType','int8_t','CVehicle *vehicle','return ((CAutomobile*)vehicle)->m_bombType;')
bridge('RequestModel','void','int32_t model, int32_t flags','CStreaming::RequestModel(model, flags);')
bridge('HasModelLoaded','bool','int32_t model','return CStreaming::HasModelLoaded(model);')
bridge('ModelInfo','CVehicleModelInfo *','int32_t model','return (CVehicleModelInfo*)CModelInfo::GetModelInfo(model);')
bridge('NumComponents','int8_t','CVehicleModelInfo *info','return info->m_numComps;')
bridge('SetComponents','void','int8_t first, int8_t second','CVehicleModelInfo::SetComponentsToUse(first, second);')
for name in ['Boat','Bike']:bridge('Is'+name+'Model','bool','int32_t model','return CModelInfo::Is'+name+'Model(model);')
bridge('AllocateVehicle','CVehicle *','void','return (CVehicle*)CPool_New(CPools::GetVehiclePool());')
for kind in ['Boat','Bike','Automobile']:bridge('Construct'+kind,'void','CVehicle *vehicle, int32_t model','std::allocator<C'+kind+'>().construct((C'+kind+'*)vehicle, model, RANDOM_VEHICLE);')
bridge('SetStanding','void','CVehicle *vehicle, bool value','((CBike*)vehicle)->bIsStanding = value;')
for name,expression in [('Position','vehicle->SetPosition(CVector(value.x, value.y, value.z));'),('Forward','vehicle->GetForward() = CVector(value.x, value.y, value.z);'),('Right','vehicle->GetRight() = CVector(value.x, value.y, value.z);'),('Up','vehicle->GetUp() = CVector(value.x, value.y, value.z);')]:bridge('Set'+name,'void','CVehicle *vehicle, StoredCarVector value',expression)
bridge('SetStatus','void','CVehicle *vehicle, int32_t status','vehicle->SetStatus((eEntityStatus)status);')
bridge('ClearDriver','void','CVehicle *vehicle','vehicle->pDriver = nil;')
for name,kind,field in [('PrimaryColor','int8_t','m_currentColour1'),('SecondaryColor','int8_t','m_currentColour2'),('RadioStation','int8_t','m_nRadioStation'),('Freebies','bool','bFreebies'),('BombType','int8_t','m_bombType'),('OwnedByPlayer','bool','bHasBeenOwnedByPlayer')]:bridge('Set'+name,'void','CVehicle *vehicle, '+kind+' value',('((CAutomobile*)vehicle)' if name=='BombType' else 'vehicle')+'->'+field+' = value;')
bridge('PlayerPed','CPed *','void','return FindPlayerPed();')
bridge('SetBombRigger','void','CVehicle *vehicle, CPed *ped','((CAutomobile*)vehicle)->m_pBombRigger = ped;')
bridge('Unlock','void','CVehicle *vehicle','vehicle->m_nDoorLock = CARLOCK_UNLOCKED;')
bridge('AssertFailed','void','const char *expression, const char *file, unsigned line, const char *function','re3_assert(expression, file, line, function);')

def record(original):
    # Extract all original private data and proof flag values without changing their order
    start=original.index('class CStoredCar\n');end=original.index('\nVALIDATE_SIZE(CStoredCar, 0x28);',start)
    return original[start:end]

def header(original):
    # Keep the exact forty-byte record with two plain three-float values
    source=record(original);enums=source[source.index('\tenum {'):source.index('\tint32 m_nModelIndex;')]
    enums=enums.replace('\tenum {','enum StoredCarFlags {').replace('FLAG_','STORED_CAR_FLAG_')
    fields=source[source.index('\tint32 m_nModelIndex;'):source.index('\npublic:')].replace('int32 ','int32_t ').replace('int8 ','int8_t ').replace('CVector ','StoredCarVector ')
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct StoredCarVector { float x, y, z; } StoredCarVector;
'''+enums+'''typedef struct CStoredCar {
'''+fields+'''
} CStoredCar;
#ifdef __cplusplus
class CVehicle;
extern "C" {
#else
typedef struct CVehicle CVehicle;
#endif
void StoredCar_Init(CStoredCar *self);
void StoredCar_Clear(CStoredCar *self);
bool StoredCar_HasCar(CStoredCar *self);
const CStoredCar *StoredCar_Assign(CStoredCar *self, const CStoredCar *other);
void StoredCar_StoreCar(CStoredCar *self, CVehicle *vehicle);
CVehicle *StoredCar_RestoreCar(CStoredCar *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Expose only typed operations on the remaining native vehicle and model owners
    source='''//+ rouz edit (ChatGPT)
#pragma once
#include "StoredCar.h"
#ifdef __cplusplus
class CVehicleModelInfo;class CPed;
extern "C" {
#else
typedef struct CVehicleModelInfo CVehicleModelInfo;
typedef struct CPed CPed;
#endif
'''
    for name,result,args,body in BRIDGES:source+=result+' StoredCar_Backend'+name+'('+args+');\n'
    return source+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def native():
    # Keep native leaves free of stored-car decisions and serialization policy
    source=''
    for name,result,args,body in BRIDGES:source+='\n'+result+' StoredCar_Backend'+name+'('+args+')\n{\n    // Access the remaining native owner through its original operation\n    '+body+'\n}\n'
    return source

def kernel(original):
    # Retain explicit partial initialization, member assignment and the full store/restore policies
    source='''//+ rouz edit (ChatGPT)
#include "StoredCar_backend.h"
#include "config.h"
#include "EntityStatus.h"
#include "VehicleBombTypes.h"
#include "StreamingInfo.h"
#include <stddef.h>
#ifdef MASTER
#define assert(expression) (expression)
#else
#define assert(expression) (void)((!!(expression))||(StoredCar_BackendAssertFailed(#expression,__FILE__,__LINE__,__func__),0))
#endif

void StoredCar_Init(CStoredCar *self)
{
    // Initialize only the original model sentinel
    self->m_nModelIndex = 0;
}

void StoredCar_Clear(CStoredCar *self)
{
    // Clear only the original model sentinel
    self->m_nModelIndex = 0;
}

bool StoredCar_HasCar(CStoredCar *self)
{
    // Test the stored model without modifying the record
    return self->m_nModelIndex != 0;
}

const CStoredCar *StoredCar_Assign(CStoredCar *self, const CStoredCar *other)
{
    // Copy original members in order while retaining destination tail padding
'''
    assignment=method(original,'CStoredCar::operator=');assignment=assignment[assignment.index('{')+1:assignment.rindex('}')]
    assignment=assignment.replace('other.','other->').replace('return *this;','return self;')
    assignment=re.sub(r'(?<![\w>.])('+ '|'.join(FIELDS)+r')\b',r'self->\1',assignment)
    source+=assignment.strip('\n')+'\n}\n\n'
    store=method(original,'CStoredCar::StoreCar');store=store[store.index('{')+1:store.rindex('}')]
    expressions={'pVehicle->GetModelIndex()':'ModelIndex','pVehicle->GetPosition()':'Position','pVehicle->GetForward()':'Forward','pVehicle->m_currentColour1':'PrimaryColor','pVehicle->m_currentColour2':'SecondaryColor','pVehicle->m_nRadioStation':'RadioStation','pVehicle->m_aExtras[0]':'VariationA','pVehicle->m_aExtras[1]':'VariationB','((CAutomobile*)pVehicle)->m_bombType':'BombType','pVehicle->IsCar()':'IsCar','pVehicle->IsBike()':'IsBike'}
    for name in ['BulletProof','FireProof','ExplosionProof','CollisionProof','MeleeProof']:expressions['pVehicle->b'+name]=name
    for expression,name in expressions.items():store=store.replace(expression,'StoredCar_Backend'+name+'(pVehicle)')
    store=re.sub(r'\b('+ '|'.join(FIELDS)+r')\b',r'self->\1',store).replace('FLAG_','STORED_CAR_FLAG_')
    source+='void StoredCar_StoreCar(CStoredCar *self, CVehicle *pVehicle)\n{\n    // Capture the original fields and only store a bomb type for cars or bikes\n'+store.strip('\n')+'\n}\n\n'
    source+='''CVehicle *StoredCar_RestoreCar(CStoredCar *self)
{
    // Request the stored model before checking whether restoration can proceed
    StoredCar_BackendRequestModel(self->m_nModelIndex, STREAMFLAGS_DEPENDENCY);
    if (!StoredCar_BackendHasModelLoaded(self->m_nModelIndex))
        return NULL;
#ifdef FIX_BUGS
    // Preserve the fixed model assertion and component-count gate
    CVehicleModelInfo *pModelInfo = StoredCar_BackendModelInfo(self->m_nModelIndex);
    assert(pModelInfo);
    if (StoredCar_BackendNumComponents(pModelInfo) != 0)
#endif
    {
        // Set the original component selections before vehicle construction
        StoredCar_BackendSetComponents(self->m_nVariationA, self->m_nVariationB);
    }

    // Preserve boat-first selection and each allocation, assertion and construction boundary
    CVehicle *pVehicle;
    if (StoredCar_BackendIsBoatModel(self->m_nModelIndex)) {
        pVehicle = StoredCar_BackendAllocateVehicle();
        assert(pVehicle);
        StoredCar_BackendConstructBoat(pVehicle, self->m_nModelIndex);
    } else if (StoredCar_BackendIsBikeModel(self->m_nModelIndex)) {
        CVehicle *pBike = StoredCar_BackendAllocateVehicle();
        assert(pBike);
        StoredCar_BackendConstructBike(pBike, self->m_nModelIndex);
        StoredCar_BackendSetStanding(pBike, true);
        pVehicle = pBike;
    } else {
        pVehicle = StoredCar_BackendAllocateVehicle();
        assert(pVehicle);
        StoredCar_BackendConstructAutomobile(pVehicle, self->m_nModelIndex);
    }

    // Restore the transform and identity fields in their original order
    StoredCar_BackendSetPosition(pVehicle, self->m_vecPos);
    StoredCar_BackendSetStatus(pVehicle, STATUS_ABANDONED);
    StoredCar_BackendSetForward(pVehicle, self->m_vecAngle);
    const StoredCarVector right = { self->m_vecAngle.y, -self->m_vecAngle.x, 0.0f };
    StoredCar_BackendSetRight(pVehicle, right);
    const StoredCarVector up = { 0.0f, 0.0f, 1.0f };
    StoredCar_BackendSetUp(pVehicle, up);
    StoredCar_BackendClearDriver(pVehicle);
    StoredCar_BackendSetPrimaryColor(pVehicle, self->m_nPrimaryColor);
    StoredCar_BackendSetSecondaryColor(pVehicle, self->m_nSecondaryColor);
    StoredCar_BackendSetRadioStation(pVehicle, self->m_nRadioStation);
    StoredCar_BackendSetFreebies(pVehicle, false);
    if (StoredCar_BackendIsCar(pVehicle)) {
        // Restore only car bombs and retain the fixed nonempty-bomb ownership rule
        StoredCar_BackendSetBombType(pVehicle, self->m_nCarBombType);
#ifdef FIX_BUGS
        if (self->m_nCarBombType != CARBOMB_NONE)
            StoredCar_BackendSetBombRigger(pVehicle, StoredCar_BackendPlayerPed());
#endif
    }

    // Preserve ownership, unlock and one-way proof flag restoration
    StoredCar_BackendSetOwnedByPlayer(pVehicle, true);
    StoredCar_BackendUnlock(pVehicle);
'''
    for name in ['BulletProof','FireProof','ExplosionProof','CollisionProof','MeleeProof']:source+='    if (self->m_nFlags & STORED_CAR_FLAG_'+name.upper()+') StoredCar_BackendSet'+name+'(pVehicle, true);\n'
    return source+'    return pVehicle;\n}\n//- rouz edit (ChatGPT)\n'

def garage_fields(source):
    # Retain the exact enclosing member sequence whose assignment previously called the custom operator
    data=source[source.index('class CGarage\n'):source.index('\n\tvoid OpenThisGarage();')]
    return re.findall(r'\b(m_\w+|field_2)\s*;',data)

def garage_copy(fields):
    # Keep the remaining native garage member assignment explicit at both noncompatible save boundaries
    source='static void Garage_AssignForSave(CGarage *self, const CGarage *other)\n{\n    // Preserve enclosing member assignment and the stored record padding contract\n'
    for field in fields:source+=('    StoredCar_Assign(&self->m_sStoredCar, &other->m_sStoredCar);' if field=='m_sStoredCar' else '    self->'+field+' = other->'+field+';')+'\n'
    return source+'}\n\n'

def stored_car_calls(source):
    # Remove the complete class and all native methods, then adapt every garage and save caller
    original=source
    if 'class CStoredCar\n' in source:return source.replace(record(source),'//+ rouz edit (ChatGPT)\n#include "StoredCar.h"\n//- rouz edit (ChatGPT)')
    if 'CStoredCar::StoreCar(' in source:
        for name in ['StoreCar','RestoreCar','operator=']:source=source.replace(method(source,'CStoredCar::'+name)+'\n','')
        source=source.replace('#include "Garages.h"','#include "Garages.h"\n#include "StoredCar_backend.h"')
        source=source.rstrip()+'\n'+native()+'\n'
        # Recover the complete enclosing field sequence from the original declaration snapshot
        from pathlib import Path
        root=Path(__file__).resolve().parents[2];saved=root/'.diagnostics/stored-car-c-tests/before/src/control/Garages.h'
        owner=saved.read_text() if saved.exists() else (root/'src/control/Garages.h').read_text()
        source=source.replace('void CGarages::Save(',garage_copy(garage_fields(owner))+'void CGarages::Save(',1)
    receiver=r'(aCarsInSafeHouses\[[^\n;]+?\]\[[^\n;]+?\]|aCars\[[^\n;]+?\])'
    source=re.sub(receiver+r'\.(Init|Clear|HasCar|RestoreCar)\(\)',r'StoredCar_\2(&\1)',source)
    source=re.sub(receiver+r'\.StoreCar\(',r'StoredCar_StoreCar(&\1, ',source)
    source=source.replace('*(CStoredCar*)buf = aCarsInSafeHouses[j][i];','StoredCar_Assign((CStoredCar*)buf, &aCarsInSafeHouses[j][i]);').replace('aCarsInSafeHouses[j][i] = *(CStoredCar*)buf;','StoredCar_Assign(&aCarsInSafeHouses[j][i], (const CStoredCar*)buf);')
    source=source.replace('*(CGarage*)buf = aGarages[i];','Garage_AssignForSave((CGarage*)buf, &aGarages[i]);').replace('aGarages[i] = *(CGarage*)buf;','Garage_AssignForSave(&aGarages[i], (const CGarage*)buf);')
    if source==original:return source
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def stored_car_project(source):
    # Register the independent C producer and its public/native boundary headers
    for tag,old,new in [('ClCompile','Garages.cpp','StoredCar.c'),('ClInclude','Garages.h','StoredCar.h'),('ClInclude','Garages.h','StoredCar_backend.h')]:
        entry='<'+tag+' Include="..\\src\\control\\'+old+'" />';addition='<'+tag+' Include="..\\src\\control\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\control\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\control\\'+re.escape(old)+r'">\n\s*<Filter>control</Filter>\n\s*</'+tag+'>)';source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)


def calls(source):
    # Preserve stored-car ownership before moving the complete rectangle owner to C
    import rect_migration as rect
    return rect.calls(stored_car_calls(source))

def project(source):
    # Retain stored-car source registration before the complete rectangle producer rename
    import rect_migration as rect
    return rect.project(stored_car_project(source))
