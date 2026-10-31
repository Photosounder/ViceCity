//+ rouz edit (ChatGPT)
#include "common.h"
#include "Bridge_backend.h"
#ifdef GTA_BRIDGE
#include "Pools.h"
#include "ModelIndices.h"
#include "PathFind.h"
#include "Stats.h"
CPool * Bridge_BackendBuildingPool(void)
{
    // Enter the original typed native field or action at the C decision point
    return CPools::GetBuildingPool();
}
CEntity * Bridge_BackendBuilding(void *slot)
{
    // Enter the original typed native field or action at the C decision point
    return (CBuilding *)slot;
}
int32_t Bridge_BackendModel(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return entity->GetModelIndex();
}
int32_t Bridge_BackendLiftModel(void)
{
    // Enter the original typed native field or action at the C decision point
    return MI_BRIDGELIFT;
}
int32_t Bridge_BackendRoadModel(void)
{
    // Enter the original typed native field or action at the C decision point
    return MI_BRIDGEROADSEGMENT;
}
int32_t Bridge_BackendWeightModel(void)
{
    // Enter the original typed native field or action at the C decision point
    return MI_BRIDGEWEIGHT;
}
float Bridge_BackendPositionZ(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return entity->GetPosition().z;
}
void Bridge_BackendSetPositionZ(CEntity *entity,float value)
{
    // Enter the original typed native field or action at the C decision point
    entity->GetMatrix().GetPosition().z=value;
}
void Bridge_BackendUpdateMatrix(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    entity->GetMatrix().UpdateRW();
}
void Bridge_BackendUpdateFrame(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    entity->UpdateRwFrame();
}
int32_t Bridge_BackendCommercialPassed(void)
{
    // Enter the original typed native field or action at the C decision point
    return Stats_CommercialPassed;
}
void Bridge_BackendSetLights(float x1,float x2,float y1,float y2,bool enabled)
{
    // Enter the original typed native field or action at the C decision point
    ThePaths.SetLinksBridgeLights(x1,x2,y1,y2,enabled);
}
#endif
//- rouz edit (ChatGPT)
