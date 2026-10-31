//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
void *AnimSequence_BackendAllocate(size_t bytes);
void AnimSequence_BackendFree(void *memory);
void AnimSequence_BackendPushAnimation(void);
void AnimSequence_BackendPopMemory(void);
void AnimSequence_BackendRegister(void **memory);
void *AnimSequence_BackendMoveMemory(void *memory);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
