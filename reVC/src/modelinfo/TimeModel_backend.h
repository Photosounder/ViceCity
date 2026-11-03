//+ rouz edit (ChatGPT)
#pragma once
#include "TimeModelData.h"
#ifdef __cplusplus
extern "C" {
#endif
void *TimeModel_LinkOther(void *model,unsigned int maxNameLength);
TimeModelData *TimeModel_BackendData(void *model);
void *TimeModel_FindOther(const char *modelName,int *index,unsigned int maxNameLength);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
