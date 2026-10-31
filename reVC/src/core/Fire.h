//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CEntity;class CVector;
typedef CVector FirePosition;
#else
typedef struct CEntity CEntity;
typedef struct CVector FirePosition;
#endif
typedef struct FireVector {float x,y,z;} FireVector;
typedef struct CFire {
    bool m_bIsOngoing,m_bIsScriptFire,m_bPropagationFlag,m_bAudioSet;
    FireVector m_vecPos;
    CEntity *m_pEntity,*m_pSource;
    uint32_t m_nExtinguishTime,m_nStartTime,m_nNextTimeToAddFlames;
    float m_fStrength,m_fWaterExtinguishCountdown;
    bool m_bExtinguishedWithWater;
} CFire;
typedef struct CFireManager {uint32_t m_nTotalFires;CFire m_aFires[NUM_FIRES];} CFireManager;
enum {FIRE_MAX_FIREMEN_ATTENDING=2};
#ifdef __cplusplus
extern "C" {
#endif
extern CFireManager gFireManager;
void Fire_ProcessFire(CFire *fire);
void Fire_ReportThisFire(CFire *fire);
void Fire_Extinguish(CFire *fire);
void FireManager_StartFireAt(CFireManager *manager,FireVector position,float size,uint8_t propagation);
CFire *FireManager_StartFireOn(CFireManager *manager,CEntity *entity,CEntity *source,float strength,uint8_t propagation);
void FireManager_Update(CFireManager *manager);
CFire *FireManager_FindNearestFire(CFireManager *manager,FireVector position,float *distance);
CFire *FireManager_FindFurthestFire_NeverMindFireMen(CFireManager *manager,FireVector position,float minimum,float maximum);
CFire *FireManager_GetNextFreeFire(CFireManager *manager);
uint32_t FireManager_GetTotalActiveFires(const CFireManager *manager);
void FireManager_ExtinguishPoint(CFireManager *manager,FireVector point,float range);
bool FireManager_ExtinguishPointWithWater(CFireManager *manager,FireVector point,float range);
int32_t FireManager_StartScriptFire(CFireManager *manager,const FirePosition *position,CEntity *target,float strength,uint8_t propagation);
bool FireManager_IsScriptFireExtinguish(CFireManager *manager,int16_t index);
void FireManager_RemoveAllScriptFires(CFireManager *manager);
void FireManager_RemoveScriptFire(CFireManager *manager,int16_t index);
void FireManager_SetScriptFireAudio(CFireManager *manager,int16_t index,bool state);
#ifdef __cplusplus
}
FireVector Fire_VectorFromNative(const CVector &position);
CVector Fire_VectorToNative(FireVector position);
#endif
//- rouz edit (ChatGPT)
