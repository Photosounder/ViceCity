#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include "ModelRegistry.h"
#include "VehicleTypes.h"
#include "ModelInfoType.h"
#include "ModelIds.h"
#include "2dEffect.h"
#include "ModelRegistry_backend.h"
#include "config.h"

static const char *entries[5];
static int visits;
static bool resetting;
static int resetCalls;
static CStore effectsStore;
static int16_t effectIds[5];
static uint8_t effectCounts[5];
static CStore shutdownStores[MODEL_STORE_COUNT];
static int shutdownEntries[MODEL_STORE_COUNT][8];
static int storeOrder[32],storeCalls,shutdownCalls;
static bool allocationTest;
static int allocationKind,allocationId,allocationStage;
static int initLog,clearedSlots,builtinIndex,builtinStep;
static C2dEffect shutdownEffect;
static int textureTrace;
static const int builtinIds[]={MI_CAR_DOOR,MI_CAR_BUMPER,MI_CAR_PANEL,MI_CAR_BONNET,MI_CAR_BOOT,MI_CAR_WHEEL,MI_BODYPARTA,MI_BODYPARTB};
static bool vehicle;
static int subtype,subtypeReads;
void *ModelRegistry_Get(int index)
{
    // Count table reads and preserve null entries
    assert(index>=0 && index<(resetting ? MODELINFOSIZE : 5));++visits;
    return index<5 && entries[index] ? (void *)&entries[index] : 0;
}
void ModelRegistry_BackendLogStores(void)
{
    // Check diagnostics occur before slot clearing
    assert(!clearedSlots);++initLog;
}
void ModelRegistry_Set(int index,void *model)
{
    // Verify the entire table is cleared in order
    assert(model==0 && initLog==1 && index==clearedSlots++);
}
void re3_assert(const char *expression,const char *file,unsigned int line,const char *function)
{
    // Fail the fixture if an allocation exceeds its store capacity
    (void)expression;(void)file;(void)line;(void)function;assert(false);
}
void ModelRegistry_BackendPublish(int id,int kind,void *entry)
{
    // Verify each allocated entry is published before native initialization
    if (allocationTest) {
        assert(kind==allocationKind && id==allocationId && entry==&shutdownEntries[kind][0] && allocationStage==0);
        allocationStage=1;return;
    }
    assert(clearedSlots==MODELINFOSIZE && storeCalls==8+builtinIndex && builtinStep==0);
    assert(kind==MODEL_STORE_SIMPLE && builtinIndex<8 && id==builtinIds[builtinIndex]);
    assert(entry==&shutdownEntries[MODEL_STORE_SIMPLE][builtinIndex]);builtinStep=1;
}
void ModelRegistry_BackendInitEntry(int kind,void *entry)
{
    // Verify native initialization follows publishing for every store type
    if (allocationTest) {
        assert(kind==allocationKind && entry==&shutdownEntries[kind][0] && allocationStage==1);
        allocationStage=2;return;
    }
    assert(kind==MODEL_STORE_SIMPLE && entry==&shutdownEntries[MODEL_STORE_SIMPLE][builtinIndex] && builtinStep==1);
    builtinStep=2;
}
void ModelRegistry_BackendSetBuiltinCollision(void *model,int kind)
{
    // Check collision selection immediately follows native initialization
    assert(model==&shutdownEntries[MODEL_STORE_SIMPLE][builtinIndex] && kind==builtinIndex && builtinStep==2);
    builtinStep=3;
}
void ModelRegistry_BackendSetBuiltinTexture(void *model)
{
    // Check texture assignment follows collision selection
    assert(model==&shutdownEntries[MODEL_STORE_SIMPLE][builtinIndex] && builtinStep==3);
    builtinStep=4;
}
void ModelRegistry_BackendSetBuiltinGeometry(void *model)
{
    // Check geometry assignment completes each built-in model
    assert(model==&shutdownEntries[MODEL_STORE_SIMPLE][builtinIndex] && builtinStep==4);
    builtinStep=0;++builtinIndex;
}
CStore *ModelRegistry_BackendStore(int kind)
{
    // Record store access in shutdown and clear order
    assert(kind>=0 && kind<MODEL_STORE_COUNT && storeCalls<32);
    storeOrder[storeCalls++]=kind;return &shutdownStores[kind];
}
void RwHelper_DestroyTexture(void *texture)
{
    // Verify each destruction precedes its field clear
    if (texture==&entries[0]) {
        assert(shutdownEffect.light.corona==texture && shutdownEffect.light.shadow==&entries[2]);textureTrace=1;
    } else {
        assert(texture==&entries[2] && !shutdownEffect.light.corona && shutdownEffect.light.shadow==texture);textureTrace=textureTrace*10+2;
    }
}
void ModelRegistry_BackendShutdownEntry(int kind,void *entry)
{
    // Check native entries are visited before any stores are cleared
    assert(shutdownStores[kind].allocPtr==1);
    assert(entry==&shutdownEntries[kind][0]);
    assert(kind==shutdownCalls++);
}
CStore *ModelRegistry_EffectsStore(void)
{
    // Return the controlled store for the C reset
    return &effectsStore;
}
int16_t *ModelRegistry_EffectId(void *model)
{
    // Resolve the original identifier for the controlled model
    for (int i=0;i<5;++i) if (model==&entries[i]) return &effectIds[i];
    assert(false);return 0;
}
uint8_t *ModelRegistry_EffectCount(void *model)
{
    // Verify reset ordering and resolve the controlled count
    for (int i=0;i<5;++i) if (model==&entries[i]) { if (resetting) { assert(effectsStore.allocPtr==0);++resetCalls; } return &effectCounts[i]; }
    assert(false);return 0;
}
const char *ModelRegistry_Name(void *model)
{
    // Borrow the selected test model name
    return *(const char **)model;
}
int ModelRegistry_Type(void *model)
{
    // Report the configured category at the first lookup
    assert(model==&entries[2]);return vehicle ? MITYPE_VEHICLE : MITYPE_PED;
}
int ModelRegistry_BackendVehicleType(void *model)
{
    // Record the subtype read after the second lookup
    assert(model==&entries[2]);++subtypeReads;return subtype;
}
bool General_faststricmp(const char *left,const char *right)
{
    // Compare test names without case sensitivity
    while (*left && toupper((unsigned char)*left)==toupper((unsigned char)*right)) { ++left;++right; }
    return toupper((unsigned char)*left)!=toupper((unsigned char)*right);
}
int main(void)
{
    // Find the first matching name and report its identifier
    int id=-1;entries[0]="other";entries[2]="Model";entries[4]="model";
    assert(ModelRegistry_Find("MODEL",0,4,&id)==&entries[2] && id==2 && visits==3);
    // Honor the inclusive range and permit an omitted identifier
    visits=0;
    assert(ModelRegistry_Find("MODEL",3,4,0)==&entries[4] && visits==2);
    // Preserve the caller identifier for empty ranges and missing names
    id=9;visits=0;
    assert(!ModelRegistry_Find("MODEL",4,3,&id) && id==9 && visits==0);
    assert(!ModelRegistry_Find("missing",0,4,&id) && id==9 && visits==5);
    // Initialize the table, stores, and built-in models in their original order
    const int initOrder[7]={6,0,1,2,3,5,4};
    for (int i=0;i<MODEL_STORE_COUNT;++i) {
        shutdownStores[i].allocPtr=1;shutdownStores[i].entries=shutdownEntries[i];
        shutdownStores[i].entrySize=sizeof(shutdownEntries[i][0]);shutdownStores[i].capacity=8;
    }
    ModelRegistry_Initialise();
    assert(initLog==1 && clearedSlots==MODELINFOSIZE && storeCalls==15 && builtinIndex==8);
    for (int i=0;i<7;++i) assert(storeOrder[i]==initOrder[i] && shutdownStores[i].allocPtr==(i==MODEL_STORE_SIMPLE ? 8 : 0));
    // Allocate each model category through its real C store path
    void *(*adders[6])(int)={ModelRegistry_AddSimple,ModelRegistry_AddTime,ModelRegistry_AddWeapon,ModelRegistry_AddClump,ModelRegistry_AddVehicle,ModelRegistry_AddPed};
    allocationTest=true;storeCalls=0;
    for (int i=0;i<6;++i) {
        shutdownStores[i].allocPtr=0;allocationKind=i;allocationId=100+i;allocationStage=0;
        assert(adders[i](allocationId)==&shutdownEntries[i][0]);
        assert(allocationStage==2 && shutdownStores[i].allocPtr==1);
    }
    allocationTest=false;storeCalls=0;
    // Shut down all seven stores before clearing them in their original order
    const int expectedOrder[14]={0,1,2,3,4,5,6,6,0,1,2,5,3,4};
    for (int i=0;i<MODEL_STORE_COUNT;++i) {
        shutdownStores[i].allocPtr=1;shutdownStores[i].entries=shutdownEntries[i];
        shutdownStores[i].entrySize=sizeof(shutdownEntries[i][0]);
    }
    shutdownEffect.type=EFFECT_PARTICLE;shutdownEffect.light.corona=&entries[0];shutdownEffect.light.shadow=&entries[2];
    shutdownStores[MODEL_STORE_EFFECT].entries=&shutdownEffect;shutdownStores[MODEL_STORE_EFFECT].entrySize=sizeof(shutdownEffect);
    ModelRegistry_Shutdown();
    assert(shutdownCalls==MODEL_STORE_COUNT-1 && storeCalls==14);
    assert(shutdownEffect.light.corona==&entries[0] && shutdownEffect.light.shadow==&entries[2]);
    for (int i=0;i<14;++i) assert(storeOrder[i]==expectedOrder[i]);
    for (int i=0;i<MODEL_STORE_COUNT;++i) assert(shutdownStores[i].allocPtr==0);
    // Destroy and clear both light textures in their original order
    shutdownEffect.type=EFFECT_LIGHT;textureTrace=0;storeCalls=0;shutdownCalls=6;
    shutdownStores[MODEL_STORE_EFFECT].allocPtr=1;
    ModelRegistry_Shutdown();
    assert(textureTrace==12 && !shutdownEffect.light.corona && !shutdownEffect.light.shadow);
    // Clear the store and revisit populated entries in table order
    resetting=true;visits=0;resetCalls=0;effectsStore.allocPtr=7;
    ModelRegistry_ReInit2dEffects();
    assert(effectsStore.allocPtr==0 && resetCalls==3 && visits==MODELINFOSIZE+3);
    resetting=false;
    // Add and retrieve effect entries through the C store policy
    int effects[3]={1,2,3};effectsStore.entries=effects;effectsStore.entrySize=sizeof(int);effectsStore.capacity=3;
    ModelRegistry_ResetEffects(&entries[0]);
    assert(effectIds[0]==-1 && effectCounts[0]==0 && !ModelRegistry_GetEffect(&entries[0],0));
    ModelRegistry_AddEffect(&entries[0],&effects[1]);ModelRegistry_AddEffect(&entries[0],&effects[2]);
    assert(effectIds[0]==1 && effectCounts[0]==2 && ModelRegistry_GetEffect(&entries[0],1)==&effects[2]);
    // Preserve the category short-circuit and repeated lookup for each subtype
    vehicle=false;visits=0;subtypeReads=0;
    assert(!ModelRegistry_IsBoatModel(2) && visits==1 && subtypeReads==0);
    vehicle=true;visits=0;subtype=VEHICLE_TYPE_BOAT;
    assert(ModelRegistry_IsBoatModel(2) && visits==2 && subtypeReads==1);
    subtype=VEHICLE_TYPE_BIKE;assert(ModelRegistry_IsBikeModel(2));
    subtype=VEHICLE_TYPE_CAR;assert(ModelRegistry_IsCarModel(2));
    subtype=VEHICLE_TYPE_HELI;assert(ModelRegistry_IsHeliModel(2));
    subtype=VEHICLE_TYPE_PLANE;assert(ModelRegistry_IsPlaneModel(2));
    assert(!ModelRegistry_IsCarModel(2));
    return 0;
}
