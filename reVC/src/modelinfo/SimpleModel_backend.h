//+ rouz edit (ChatGPT)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void *SimpleModel_BackendRelated(void *model);
void SimpleModel_BackendMarkBig(void *model);
float SimpleModel_BackendRelatedNearDistance(void *related);
void SimpleModel_BackendSetNearDistance(void *model,float distance);
int SimpleModel_BackendDrawLast(void *model);
void SimpleModel_BackendClearDrawLast(void *model);
void SimpleModel_BackendRequireSimple(void *model);
void SimpleModel_BackendSetRelated(void *model,void *related);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
