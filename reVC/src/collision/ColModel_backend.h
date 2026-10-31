//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "ColModel.h"
#ifdef __cplusplus
extern "C" {
#endif
void *ColModel_BackendAllocate(size_t size);
void ColModel_BackendFree(void *memory);
void ColModel_BackendPush(int32_t id);
void ColModel_BackendPop(void);
void ColModel_BackendRegister(void *location);
void ColModel_BackendRemoveCachedPlanes(CColModel *model);
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
