//+ rouz edit (ChatGPT)
#include "common.h"
#include "WaterCreatures_backend.h"
#include "World.h"
#include "WaterLevel.h"
#include "Camera.h"
#include "PlayerPed.h"
#include "Object.h"
#include "Pools.h"
static_assert(TEMP_OBJECT==WATER_CREATURE_TEMP_OBJECT && CONTROLLED_SUB_OBJECT==WATER_CREATURE_CONTROLLED_SUB_OBJECT,"Original native creation tags");
int16_t * WaterCreatures_BackendTempCount(void)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    return &CObject::nNoTempObjects;
}
CPool * WaterCreatures_BackendObjectPool(void)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    return CPools::GetObjectPool();
}
int8_t WaterCreatures_BackendCreatedBy(CObject *object)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    return object->ObjectCreatedBy;
}
void WaterCreatures_BackendConstruct(CObject *object,int16_t modelID)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    std::allocator<CObject>().construct(object,modelID,true);
}
void WaterCreatures_BackendDestroy(CObject *object)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    object->~CObject();
}
void WaterCreatures_BackendWorldRemove(CObject *object)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    CWorld::Remove(object);
}
void WaterCreatures_BackendWorldAdd(CObject *object)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    CWorld::Add(object);
}
void WaterCreatures_BackendSetPosition(CObject *object,const WaterCreaturePosition *position)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    object->SetPosition(*position);
}
void WaterCreatures_BackendUpdateRW(CObject *object)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    object->GetMatrix().UpdateRW();
}
void WaterCreatures_BackendRotateZ(CObject *object,float angle)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    object->GetMatrix().SetRotateZOnly(angle);
}
void WaterCreatures_BackendSetCreatedBy(CObject *object,int8_t value)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    object->ObjectCreatedBy=value;
}
void WaterCreatures_BackendSetStatic(CObject *object,bool value)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    object->bIsStatic=value;
}
void WaterCreatures_BackendSetTouchingWater(CObject *object,bool value)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    object->bTouchingWater=value;
}
void WaterCreatures_BackendSetUnderwater(CObject *object,bool value)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    object->bUnderwater=value;
}
uint32_t * WaterCreatures_BackendEndTime(CObject *object)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    return &object->m_nEndOfLifeTime;
}
void WaterCreatures_BackendSetAlpha(CObject *object,int32_t value)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    object->SetRwObjectAlpha(value);
}
bool WaterCreatures_BackendOnScreen(CObject *object)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    return object->GetIsOnScreen();
}
WaterCreatureVector WaterCreatures_BackendPlayerPosition(void)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    const CVector &value=FindPlayerPed()->GetPosition();WaterCreatureVector result={value.x,value.y,value.z};return result;
}
WaterCreatureVector WaterCreatures_BackendVector(const WaterCreaturePosition *position)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    WaterCreatureVector result={position->x,position->y,position->z};return result;
}
WaterCreatureVector WaterCreatures_BackendPosition(CObject *object)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    const CVector &value=object->GetPosition();WaterCreatureVector result={value.x,value.y,value.z};return result;
}
WaterCreatureVector WaterCreatures_BackendRight(CObject *object)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    const CVector &value=object->GetRight();WaterCreatureVector result={value.x,value.y,value.z};return result;
}
const WaterCreaturePosition * WaterCreatures_BackendObjectPosition(CObject *object)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    return &object->GetPosition();
}
bool WaterCreatures_BackendVisible(WaterCreaturePosition *position,float radius)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    return TheCamera.IsSphereVisible(*position,radius);
}
bool WaterCreatures_BackendDepth(const WaterCreaturePosition *position,float *depth,float *level,float *ground)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    return CWaterLevel::GetWaterDepth(*position,depth,level,ground);
}
void WaterCreatures_BackendSetZ(WaterCreaturePosition *position,float value)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    position->z=value;
}
void WaterCreatures_BackendSetMoveSpeed(CObject *object,WaterCreatureVector value)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    object->m_vecMoveSpeed=CVector(value.x,value.y,value.z);
}
void WaterCreatures_BackendSetTurnSpeed(CObject *object,WaterCreatureVector value)
{
    // Preserve the actual typed owner operation and original borrowed-field access
    object->m_vecTurnSpeed=CVector(value.x,value.y,value.z);
}
void WaterCreatures_BackendWithFishPosition(const WaterCreaturePosition *position,WaterCreatureVector player,int32_t modelID,WaterCreatureSpawnCallback callback)
{
    // Keep the original native fish-position copy alive through visibility, depth, allocation and publication
    CVector fishPos=*position;callback(player,&fishPos,modelID);
}
//- rouz edit (ChatGPT)
