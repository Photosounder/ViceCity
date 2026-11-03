//+ rouz edit (ChatGPT)
#pragma once
#include "config.h"
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int32_t CdStreamSync(int32_t channel);
#ifdef FLUSHABLE_STREAMING
extern bool flushStream[MAX_CDCHANNELS];
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
