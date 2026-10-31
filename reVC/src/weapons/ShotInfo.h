//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "WeaponType.h"
#ifdef __cplusplus
class CEntity;
extern "C" {
#else
typedef struct CEntity CEntity;
#endif
typedef struct ShotInfoVector {float x,y,z;} ShotInfoVector;
typedef struct ShotInfo {
    enum eWeaponType m_weapon;
    ShotInfoVector m_startPos,m_areaAffected;
    float m_radius;
    CEntity *m_sourceEntity;
    float m_timeout;
    bool m_inUse;
} ShotInfo;
extern ShotInfo gaShotInfo[NUMSHOTINFOS];
extern float ShotInfo_ms_afRandTable[20];
#ifdef SQUEEZE_PERFORMANCE
extern uint32_t shotInfoInUse;
#endif
void ShotInfo_Initialise(void);
bool ShotInfo_AddShot(CEntity *sourceEntity,enum eWeaponType weapon,ShotInfoVector startPos,ShotInfoVector endPos);
void ShotInfo_Shutdown(void);
void ShotInfo_Update(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
