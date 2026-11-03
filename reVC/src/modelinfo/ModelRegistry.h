//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#include <stdint.h>
struct CStore;
#ifdef __cplusplus
extern "C" {
#endif
void *ModelRegistry_AddSimple(int id);
void *ModelRegistry_AddTime(int id);
void *ModelRegistry_AddWeapon(int id);
void *ModelRegistry_AddClump(int id);
void *ModelRegistry_AddVehicle(int id);
void *ModelRegistry_AddPed(int id);
struct CStore *ModelRegistry_EffectsStore(void);
int16_t *ModelRegistry_EffectId(void *model);
uint8_t *ModelRegistry_EffectCount(void *model);
void ModelRegistry_ResetEffects(void *model);
void ModelRegistry_AddEffect(void *model,void *effect);
void *ModelRegistry_GetEffect(void *model,int index);
void ModelRegistry_Initialise(void);
void ModelRegistry_Shutdown(void);
void ModelRegistry_ReInit2dEffects(void);
void *ModelRegistry_Get(int id);
const char *ModelRegistry_Name(void *model);
int ModelRegistry_Type(void *model);
void *ModelRegistry_Collision(void *model);
bool ModelRegistry_IsBoatModel(int id);
bool ModelRegistry_IsBikeModel(int id);
bool ModelRegistry_IsCarModel(int id);
bool ModelRegistry_IsHeliModel(int id);
bool ModelRegistry_IsPlaneModel(int id);
void *ModelRegistry_Find(const char *name,int minIndex,int maxIndex,int *id);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
