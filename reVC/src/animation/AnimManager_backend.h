//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void AnimManager_HostRemoveAnim(int32_t block);
void * AnimManager_HostOpenStream(const char *filename);
void AnimManager_HostReadStream(void *stream,void *buffer,uint32_t size);
void AnimManager_HostCloseStream(void *stream);
int AnimManager_HostCompareNames(const char *a,const char *b);
void re3_assert(const char *expression,const char *file,unsigned int line,const char *function);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
