//+ rouz edit (ChatGPT)
#include "common.h"
#include "ShotInfo_backend.h"
#include "Ped.h"
#include "World.h"
bool ShotInfo_HostIsPed(CEntity *entity)
{
    // Use the actual separately owned native entity or vector operation
    return entity->IsPed();
}
bool ShotInfo_HostValid(CEntity *entity)
{
    // Use the actual separately owned native entity or vector operation
    return ((CPed*)entity)->IsPointerValid();
}
int32_t ShotInfo_HostNearCount(CEntity *entity)
{
    // Use the actual separately owned native entity or vector operation
    return ((CPed*)entity)->m_numNearPeds;
}
CEntity * ShotInfo_HostNearPed(CEntity *entity,int32_t index)
{
    // Use the actual separately owned native entity or vector operation
    return ((CPed*)entity)->m_nearPeds[index];
}
bool ShotInfo_HostInControl(CEntity *entity)
{
    // Use the actual separately owned native entity or vector operation
    return ((CPed*)entity)->IsPedInControl();
}
ShotInfoVector ShotInfo_HostPosition(CEntity *entity)
{
    // Use the actual separately owned native entity or vector operation
    const CVector &position=entity->GetPosition();ShotInfoVector result={position.x,position.y,position.z};return result;
}
bool ShotInfo_HostFireProof(CEntity *entity)
{
    // Use the actual separately owned native entity or vector operation
    return entity->bFireProof;
}
bool ShotInfo_HostIsPlayer(CEntity *entity)
{
    // Use the actual separately owned native entity or vector operation
    return ((CPed*)entity)->IsPlayer();
}
void ShotInfo_HostFlee(CEntity *entity,CEntity *source,uint32_t duration)
{
    // Use the actual separately owned native entity or vector operation
    ((CPed*)entity)->SetFindPathAndFlee(source,duration);
}
void ShotInfo_HostSprint(CEntity *entity)
{
    // Use the actual separately owned native entity or vector operation
    ((CPed*)entity)->SetMoveState(PEDMOVE_SPRINT);
}
void ShotInfo_HostCarsOnFire(float x,float y,float z,float radius,CEntity *source)
{
    // Use the actual separately owned native entity or vector operation
    CWorld::SetCarsOnFire(x,y,z,radius,source);
}
void ShotInfo_HostNormalise(ShotInfoVector *value)
{
    // Use the actual separately owned native entity or vector operation
    CVector vector(value->x,value->y,value->z);vector.Normalise();value->x=vector.x;value->y=vector.y;value->z=vector.z;
}
void ShotInfo_HostDebug(const char *message)
{
    // Use the actual separately owned native entity or vector operation
    debug("%s",message);
}
void ShotInfo_HostAssert(const char *expression,const char *file,uint32_t line,const char *function)
{
    // Use the actual separately owned native entity or vector operation
    re3_assert(expression,file,line,function);
}
//- rouz edit (ChatGPT)
