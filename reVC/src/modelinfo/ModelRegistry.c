//+ rouz edit (ChatGPT)
#include "RwHelper.h"
#include "ModelRegistry.h"
#ifndef MASTER
extern void re3_assert(const char *expression,const char *file,unsigned int line,const char *function);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "ModelRegistry_backend.h"
#include "General.h"
#include "VehicleTypes.h"
#include "ModelInfoType.h"
#include "ModelIds.h"
#include "2dEffect.h"

static void *ModelRegistry_Add(int id,int kind)
{
    // Reserve the existing store slot, publish it, and initialize the native entry
    void *entry=CStore_Alloc(ModelRegistry_BackendStore(kind));
    ModelRegistry_BackendPublish(id,kind,entry);
    ModelRegistry_BackendInitEntry(kind,entry);
    return entry;
}

void *ModelRegistry_AddSimple(int id)
{
    // Allocate from the simple-model store
    return ModelRegistry_Add(id,MODEL_STORE_SIMPLE);
}
void *ModelRegistry_AddTime(int id)
{
    // Allocate from the time-model store
    return ModelRegistry_Add(id,MODEL_STORE_TIME);
}
void *ModelRegistry_AddWeapon(int id)
{
    // Allocate from the weapon-model store
    return ModelRegistry_Add(id,MODEL_STORE_WEAPON);
}
void *ModelRegistry_AddClump(int id)
{
    // Allocate from the clump-model store
    return ModelRegistry_Add(id,MODEL_STORE_CLUMP);
}
void *ModelRegistry_AddVehicle(int id)
{
    // Allocate from the vehicle-model store
    return ModelRegistry_Add(id,MODEL_STORE_VEHICLE);
}
void *ModelRegistry_AddPed(int id)
{
    // Allocate from the pedestrian-model store
    return ModelRegistry_Add(id,MODEL_STORE_PED);
}

void ModelRegistry_Initialise(void)
{
    // Log the native store sizes and clear every registry slot
    ModelRegistry_BackendLogStores();
    for (int i=0;i<MODELINFOSIZE;++i) ModelRegistry_Set(i,0);
    // Reset the stores in the original initialization order
    const int clearOrder[]={MODEL_STORE_EFFECT,MODEL_STORE_SIMPLE,MODEL_STORE_TIME,MODEL_STORE_WEAPON,MODEL_STORE_CLUMP,MODEL_STORE_PED,MODEL_STORE_VEHICLE};
    for (unsigned i=0;i<sizeof(clearOrder)/sizeof(clearOrder[0]);++i)
        CStore_Clear(ModelRegistry_BackendStore(clearOrder[i]));
    // Create each built-in model and configure it before moving to the next
    const int modelIds[]={MI_CAR_DOOR,MI_CAR_BUMPER,MI_CAR_PANEL,MI_CAR_BONNET,MI_CAR_BOOT,MI_CAR_WHEEL,MI_BODYPARTA,MI_BODYPARTB};
    for (unsigned i=0;i<sizeof(modelIds)/sizeof(modelIds[0]);++i) {
        void *model=ModelRegistry_AddSimple(modelIds[i]);
        ModelRegistry_BackendSetBuiltinCollision(model,(int)i);
        ModelRegistry_BackendSetBuiltinTexture(model);
        ModelRegistry_BackendSetBuiltinGeometry(model);
    }
}

static void ModelRegistry_ShutdownEffect(C2dEffect *effect)
{
    // Release the original light textures in order and clear each pointer
    if (effect->type==EFFECT_LIGHT) {
        if (effect->light.corona) RwHelper_DestroyTexture(effect->light.corona);
        effect->light.corona=0;
        if (effect->light.shadow) RwHelper_DestroyTexture(effect->light.shadow);
        effect->light.shadow=0;
    }
}

void ModelRegistry_Shutdown(void)
{
    // Shut down each live native entry in the original store order
    for (int kind=MODEL_STORE_SIMPLE;kind<MODEL_STORE_COUNT;++kind) {
        CStore *store=ModelRegistry_BackendStore(kind);
        for (int i=0;i<store->allocPtr;++i) {
            // Dispatch the effect policy or the original native model shutdown
            void *entry=(uint8_t*)store->entries+store->entrySize*i;
            if (kind==MODEL_STORE_EFFECT) ModelRegistry_ShutdownEffect(entry);
            else ModelRegistry_BackendShutdownEntry(kind,entry);
        }
    }
    // Clear stores in the original post-shutdown order
    const int clearOrder[]={MODEL_STORE_EFFECT,MODEL_STORE_SIMPLE,MODEL_STORE_TIME,MODEL_STORE_WEAPON,MODEL_STORE_PED,MODEL_STORE_CLUMP,MODEL_STORE_VEHICLE};
    for (unsigned i=0;i<sizeof(clearOrder)/sizeof(clearOrder[0]);++i)
        CStore_Clear(ModelRegistry_BackendStore(clearOrder[i]));
}

void ModelRegistry_ResetEffects(void *model)
{
    // Reset the original effect identifier and count in order
    *ModelRegistry_EffectId(model)=-1;
    *ModelRegistry_EffectCount(model)=0;
}

void ModelRegistry_AddEffect(void *model,void *effect)
{
    // Append to the existing range or record the first effect store index
    if (*ModelRegistry_EffectId(model)>=0)
        ++*ModelRegistry_EffectCount(model);
    else {
        *ModelRegistry_EffectId(model)=CStore_GetIndex(ModelRegistry_EffectsStore(),effect);
        *ModelRegistry_EffectCount(model)=1;
    }
}

void *ModelRegistry_GetEffect(void *model,int index)
{
    // Return the original store entry only when a range exists
    int16_t id=*ModelRegistry_EffectId(model);
    return id>=0 ? CStore_GetItem(ModelRegistry_EffectsStore(),id+index) : 0;
}

void ModelRegistry_ReInit2dEffects(void)
{
    // Clear the effect store before walking the original model table
    CStore_Clear(ModelRegistry_EffectsStore());
    for (int i=0;i<MODELINFOSIZE;++i)
        if (ModelRegistry_Get(i))
            ModelRegistry_ResetEffects(ModelRegistry_Get(i));
}

void *ModelRegistry_Find(const char *name,int minIndex,int maxIndex,int *id)
{
    // Search the requested inclusive range in its original order
    for (int i=minIndex;i<=maxIndex;++i) {
        void *model=ModelRegistry_Get(i);
        if (model && !General_faststricmp(ModelRegistry_Name(model),name)) {
            // Publish the optional identifier only for a matching model
            if (id) *id=i;
            return model;
        }
    }
    return 0;
}
static bool ModelRegistry_IsVehicleType(int id,int type)
{
    // Preserve the original short-circuit and repeated model-table lookup
    return ModelRegistry_Type(ModelRegistry_Get(id))==MITYPE_VEHICLE &&
        ModelRegistry_BackendVehicleType(ModelRegistry_Get(id))==type;
}

bool ModelRegistry_IsBoatModel(int id)
{
    // Check the original boat subtype
    return ModelRegistry_IsVehicleType(id,VEHICLE_TYPE_BOAT);
}
bool ModelRegistry_IsBikeModel(int id)
{
    // Check the original bike subtype
    return ModelRegistry_IsVehicleType(id,VEHICLE_TYPE_BIKE);
}
bool ModelRegistry_IsCarModel(int id)
{
    // Check the original car subtype
    return ModelRegistry_IsVehicleType(id,VEHICLE_TYPE_CAR);
}
bool ModelRegistry_IsHeliModel(int id)
{
    // Check the original helicopter subtype
    return ModelRegistry_IsVehicleType(id,VEHICLE_TYPE_HELI);
}
bool ModelRegistry_IsPlaneModel(int id)
{
    // Check the original plane subtype
    return ModelRegistry_IsVehicleType(id,VEHICLE_TYPE_PLANE);
}
//- rouz edit (ChatGPT)
