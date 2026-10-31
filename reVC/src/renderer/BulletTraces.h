//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct BulletTraceVector { float x,y,z; } BulletTraceVector;
typedef struct BulletTrace {
    BulletTraceVector m_vecStartPos,m_vecEndPos;
    bool m_bInUse;
    uint32_t m_nCreationTime,m_nLifeTime;
    float m_fThickness;
    uint8_t m_fVisibility;
} BulletTrace;
#ifdef __cplusplus
extern "C" {
#endif
extern BulletTrace BulletTraces_aTraces[NUMBULLETTRACES];
void BulletTrace_Update(BulletTrace *trace);
void BulletTraces_Init(void);
void BulletTraces_InitGeometry(void);
void BulletTraces_InitTexture(void);
void BulletTraces_ShutdownTexture(void);
void BulletTraces_Update(void);
void BulletTraces_Render(void);
void BulletTraces_Add(const BulletTraceVector *start,const BulletTraceVector *end,float thickness,uint32_t lifeTime,uint8_t visibility);
void BulletTraces_AddLegacy(const void *start,const void *end,float thickness,uint32_t lifeTime,uint8_t visibility);
void BulletTraces_AddWeapon(const BulletTraceVector *start,const BulletTraceVector *end,int32_t weaponType,void *shooter);
void BulletTraces_AddWeaponLegacy(const void *start,const void *end,int32_t weaponType,void *shooter);
#ifdef __cplusplus
}
class CVector;
BulletTraceVector BulletTraces_FromVector(CVector value);
CVector BulletTraces_ToVector(BulletTraceVector value);
#endif
//- rouz edit (ChatGPT)
