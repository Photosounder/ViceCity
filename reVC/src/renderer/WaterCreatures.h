//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CObject;
class CVector;
typedef CVector WaterCreaturePosition;
#else
typedef struct CObject CObject;
typedef struct CVector WaterCreaturePosition;
#endif
enum eFishSlotState { WATER_CREATURE_INIT=0,WATER_CREATURE_ACTIVE,WATER_CREATURE_FADE_OUT,WATER_CREATURE_REMOVE,WATER_CREATURE_DISABLED };
typedef struct WaterCreatureVector { float x,y,z; } WaterCreatureVector;
typedef struct WaterCreature {
    CObject *m_pObj;
    float m_fFwdSpeed,m_fZTurnSpeed;
    int32_t m_alpha;
    float m_fWaterDepth;
    int32_t m_state;
} WaterCreature;
typedef struct WaterCreatureProperties {
    int16_t *modelID;
    float fFwdSpeed,fLevel,fUnknown,fWaterDepth;
} WaterCreatureProperties;
#ifdef __cplusplus
extern "C" {
#endif
extern WaterCreature WaterCreatures_aWaterCreatures[NUM_WATER_CREATURES];
extern int32_t WaterCreatures_nNumActiveSeaLifeForms;
extern WaterCreatureProperties WaterCreatures_Properties[65];
void WaterCreature_Construct(WaterCreature *record);
void WaterCreature_Initialise(WaterCreature *record,CObject *object,float forward,float turn,float depth,uint32_t alpha,enum eFishSlotState state);
void WaterCreature_Allocate(WaterCreature *record,CObject *object,float forward,float turn,float depth,uint32_t alpha,enum eFishSlotState state);
void WaterCreature_Free(WaterCreature *record);
WaterCreature *WaterCreatures_GetFishStructSlot(void);
CObject *WaterCreatures_CreateSeaLifeForm(const WaterCreaturePosition *position,int16_t modelID,int32_t rotation);
bool WaterCreatures_IsSpaceForMoreWaterCreatures(void);
float WaterCreatures_CalculateFishHeading(const WaterCreaturePosition *position1,const WaterCreaturePosition *position2);
void WaterCreatures_CreateOne(const WaterCreaturePosition *position,int32_t modelID);
void WaterCreatures_FreeFishStructSlot(WaterCreature *record);
void WaterCreatures_UpdateAll(void);
void WaterCreatures_RemoveAll(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
