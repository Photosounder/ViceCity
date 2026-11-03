//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#include "Store.h"
#include <stdint.h>
enum ModelRegistryStore { MODEL_STORE_SIMPLE, MODEL_STORE_TIME, MODEL_STORE_WEAPON, MODEL_STORE_CLUMP, MODEL_STORE_VEHICLE, MODEL_STORE_PED, MODEL_STORE_EFFECT, MODEL_STORE_COUNT };
#ifdef __cplusplus
extern "C" {
#endif
void ModelRegistry_BackendLogStores(void);
void ModelRegistry_Set(int id,void *model);
void ModelRegistry_BackendPublish(int id,int kind,void *entry);
void ModelRegistry_BackendInitEntry(int kind,void *entry);
void ModelRegistry_BackendSetBuiltinCollision(void *model,int kind);
void ModelRegistry_BackendSetBuiltinTexture(void *model);
void ModelRegistry_BackendSetBuiltinGeometry(void *model);
int ModelRegistry_BackendVehicleType(void *model);
CStore *ModelRegistry_BackendStore(int kind);
void ModelRegistry_BackendShutdownEntry(int kind,void *entry);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
