//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
int16_t StreamingRequest_TxdSlot(void *model);
int16_t StreamingOwnership_TxdSlot(void *model);
int32_t StreamingRequest_AnimIndex(void *model);
int32_t StreamingDependencies_AnimIndex(void *model);
void StreamingRequest_SetOpaqueIfSimple(void *model);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
