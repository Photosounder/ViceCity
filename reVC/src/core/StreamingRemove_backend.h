//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
int StreamingRemove_NoUnstreaming(void);
void StreamingRemove_RemoveVehicleClass(int32_t id);
void StreamingRemove_AddVehicleClass(int32_t id);
bool StreamingRemove_TryVehicleFallback(void);
int StreamingRemove_ModelRefs(int32_t id);
void StreamingRemove_DeleteModel(int32_t id);
void StreamingRemove_RemoveCol(int32_t id);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
