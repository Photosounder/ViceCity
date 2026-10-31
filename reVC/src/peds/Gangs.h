//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "PedType.h"
#include "GangTypes.h"
typedef struct GangInfo {
    int32_t m_nVehicleMI;
    int32_t m_nPedModel1MI;
    int32_t m_nPedModel2MI;
    int8_t m_nPedModelOverride;
    int32_t m_Weapon1;
    int32_t m_Weapon2;
} GangInfo;
#ifdef __cplusplus
extern "C" {
#endif
void Gangs_Initialise(void);
bool Gangs_HaveGangModelsLoaded(int16_t gang);
void Gangs_SetGangPedModels(int16_t gang,int32_t first,int32_t second);
void Gangs_SetWillAttackPlayerWithCops(ePedType type,bool will);
bool Gangs_GetWillAttackPlayerWithCops(ePedType type);
int32_t Gangs_ChooseGangPedModel(int16_t gang);
void Gangs_SetGangVehicleModel(int16_t gang,int32_t model);
void Gangs_SetGangWeapons(int16_t gang,int32_t first,int32_t second);
void Gangs_SetGangPedModelOverride(int16_t gang,int8_t overrideModel);
int8_t Gangs_GetGangPedModelOverride(int16_t gang);
void Gangs_SaveAllGangData(uint8_t *buffer,uint32_t *size);
void Gangs_LoadAllGangData(uint8_t *buffer,uint32_t size);
int32_t Gangs_GetGangPedModel1(int16_t gang);
int32_t Gangs_GetGangPedModel2(int16_t gang);
int32_t Gangs_GetGangVehicleModel(int16_t gang);
GangInfo * Gangs_GetGangInfo(int16_t gang);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
