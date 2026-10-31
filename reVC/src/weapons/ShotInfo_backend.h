//+ rouz edit (ChatGPT)
#pragma once
#include "ShotInfo.h"
#ifdef __cplusplus
extern "C" {
#endif
bool ShotInfo_HostIsPed(CEntity *entity);
bool ShotInfo_HostValid(CEntity *entity);
int32_t ShotInfo_HostNearCount(CEntity *entity);
CEntity * ShotInfo_HostNearPed(CEntity *entity,int32_t index);
bool ShotInfo_HostInControl(CEntity *entity);
ShotInfoVector ShotInfo_HostPosition(CEntity *entity);
bool ShotInfo_HostFireProof(CEntity *entity);
bool ShotInfo_HostIsPlayer(CEntity *entity);
void ShotInfo_HostFlee(CEntity *entity,CEntity *source,uint32_t duration);
void ShotInfo_HostSprint(CEntity *entity);
void ShotInfo_HostCarsOnFire(float x,float y,float z,float radius,CEntity *source);
void ShotInfo_HostNormalise(ShotInfoVector *value);
void ShotInfo_HostDebug(const char *message);
void ShotInfo_HostAssert(const char *expression,const char *file,uint32_t line,const char *function);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
