#include "common.h"

#include "General.h"
#include "TempColModels.h"
#include "ModelIndices.h"
#include "ModelInfo.h"
#include "ModelRegistry.h" // rouz edit (ChatGPT)
#include "ModelRegistry_backend.h" // rouz edit (ChatGPT)


//+ rouz edit (ChatGPT)
static CSimpleModelInfo simpleModelEntries[SIMPLEMODELSIZE];
static CStore simpleModelStore = { 0, simpleModelEntries, sizeof(CSimpleModelInfo), SIMPLEMODELSIZE };
static CTimeModelInfo timeModelEntries[TIMEMODELSIZE];
static CStore timeModelStore = { 0, timeModelEntries, sizeof(CTimeModelInfo), TIMEMODELSIZE };
static CWeaponModelInfo weaponModelEntries[WEAPONMODELSIZE];
static CStore weaponModelStore = { 0, weaponModelEntries, sizeof(CWeaponModelInfo), WEAPONMODELSIZE };
static CClumpModelInfo clumpModelEntries[CLUMPMODELSIZE];
static CStore clumpModelStore = { 0, clumpModelEntries, sizeof(CClumpModelInfo), CLUMPMODELSIZE };
static CPedModelInfo pedModelEntries[PEDMODELSIZE];
static CStore pedModelStore = { 0, pedModelEntries, sizeof(CPedModelInfo), PEDMODELSIZE };
static CVehicleModelInfo vehicleModelEntries[VEHICLEMODELSIZE];
static CStore vehicleModelStore = { 0, vehicleModelEntries, sizeof(CVehicleModelInfo), VEHICLEMODELSIZE };
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
void ModelRegistry_BackendLogStores(void)
{
    // Preserve the original store-size diagnostics
    debug("sizeof SimpleModelStore %d\n",sizeof(simpleModelEntries));
    debug("sizeof TimeModelStore %d\n",sizeof(timeModelEntries));
    debug("sizeof WeaponModelStore %d\n",sizeof(weaponModelEntries));
    debug("sizeof ClumpModelStore %d\n",sizeof(clumpModelEntries));
    debug("sizeof VehicleModelStore %d\n",sizeof(vehicleModelEntries));
    debug("sizeof PedModelStore %d\n",sizeof(pedModelEntries));
    debug("sizeof 2deffectsModelStore %d\n",sizeof(C2dEffect)*TWODFXSIZE);
}

void ModelRegistry_BackendSetBuiltinCollision(void *model,int kind)
{
    // Assign the matching original temporary collision model
    CColModel *models[]={&CTempColModels::ms_colModelDoor1,&CTempColModels::ms_colModelBumper1,
        &CTempColModels::ms_colModelPanel1,&CTempColModels::ms_colModelBonnet1,
        &CTempColModels::ms_colModelBoot1,&CTempColModels::ms_colModelWheel1,
        &CTempColModels::ms_colModelBodyPart1,&CTempColModels::ms_colModelBodyPart2};
    ((CSimpleModelInfo*)model)->SetColModel(models[kind]);
}

void ModelRegistry_BackendSetBuiltinTexture(void *model)
{
    // Assign the original generic texture dictionary
    ((CSimpleModelInfo*)model)->SetTexDictionary("generic");
}

void ModelRegistry_BackendSetBuiltinGeometry(void *model)
{
    // Preserve the built-in atomic count and draw distance
    CSimpleModelInfo *simple=(CSimpleModelInfo*)model;
    simple->m_numAtomics=1;
    simple->m_lodDistances[0]=80.0f;
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
CStore *ModelRegistry_BackendStore(int kind)
{
    // Resolve each original native store without changing its ownership
    switch (kind) {
    case MODEL_STORE_SIMPLE: return &simpleModelStore;
    case MODEL_STORE_TIME: return &timeModelStore;
    case MODEL_STORE_WEAPON: return &weaponModelStore;
    case MODEL_STORE_CLUMP: return &clumpModelStore;
    case MODEL_STORE_VEHICLE: return &vehicleModelStore;
    case MODEL_STORE_PED: return &pedModelStore;
    default: return ModelRegistry_EffectsStore();
    }
}

void ModelRegistry_BackendShutdownEntry(int kind,void *entry)
{
    // Dispatch shutdown through each entry's original native type
    switch (kind) {
    case MODEL_STORE_SIMPLE: ((CSimpleModelInfo*)entry)->Shutdown(); break;
    case MODEL_STORE_TIME: ((CTimeModelInfo*)entry)->Shutdown(); break;
    case MODEL_STORE_WEAPON: ((CWeaponModelInfo*)entry)->Shutdown(); break;
    case MODEL_STORE_CLUMP: ((CClumpModelInfo*)entry)->Shutdown(); break;
    case MODEL_STORE_VEHICLE: ((CVehicleModelInfo*)entry)->Shutdown(); break;
    case MODEL_STORE_PED: ((CPedModelInfo*)entry)->Shutdown(); break;
    }
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
void ModelRegistry_BackendPublish(int id,int kind,void *entry)
{
    // Convert each native entry to its base pointer before publishing it
    CBaseModelInfo *base;
    switch (kind) {
    case MODEL_STORE_SIMPLE: base=(CSimpleModelInfo*)entry; break;
    case MODEL_STORE_TIME: base=(CTimeModelInfo*)entry; break;
    case MODEL_STORE_WEAPON: base=(CWeaponModelInfo*)entry; break;
    case MODEL_STORE_CLUMP: base=(CClumpModelInfo*)entry; break;
    case MODEL_STORE_VEHICLE: base=(CVehicleModelInfo*)entry; break;
    default: base=(CPedModelInfo*)entry; break;
    }
    ModelRegistry_Set(id,base);
}

void ModelRegistry_BackendInitEntry(int kind,void *entry)
{
    // Apply the original per-type initialization after publishing the entry
    switch (kind) {
    case MODEL_STORE_SIMPLE: ((CSimpleModelInfo*)entry)->Init(); break;
    case MODEL_STORE_TIME: ((CTimeModelInfo*)entry)->Init(); break;
    case MODEL_STORE_WEAPON: ((CWeaponModelInfo*)entry)->Init(); break;
    case MODEL_STORE_CLUMP: ((CClumpModelInfo*)entry)->m_clump=nil; break;
    case MODEL_STORE_PED: ((CPedModelInfo*)entry)->m_clump=nil; break;
    case MODEL_STORE_VEHICLE: {
        // Reset only the vehicle fields initialized by the original allocator
        CVehicleModelInfo *model=(CVehicleModelInfo*)entry;
        model->m_clump=nil;
        model->m_vehicleType=-1;
        model->m_wheelId=-1;
        model->m_materials1[0]=nil;
        model->m_materials2[0]=nil;
        model->m_bikeSteerAngle=999.99f;
        break;
    }
    }
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
const char *ModelRegistry_Name(void *model)
{
    // Borrow the native name without changing its lifetime
    return ((CBaseModelInfo*)model)->GetModelName();
}

//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
void *ModelRegistry_Collision(void *model)
{
    // Borrow the original collision model pointer
    return ((CBaseModelInfo*)model)->GetColModel();
}

int ModelRegistry_Type(void *model)
{
    // Read the original model category without changing null handling
    return ((CBaseModelInfo*)model)->GetModelType();
}

int ModelRegistry_BackendVehicleType(void *model)
{
    // Read the original vehicle subtype from the native model
    return ((CVehicleModelInfo*)((CBaseModelInfo*)model))->m_vehicleType;
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
