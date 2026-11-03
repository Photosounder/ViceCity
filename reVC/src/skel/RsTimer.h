//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef _WIN32
typedef uint32_t RsTimerValue;
#else
typedef double RsTimerValue;
#endif
#ifdef __cplusplus
extern "C" {
#endif
RsTimerValue RsTimer(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
