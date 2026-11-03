//+ rouz edit (ChatGPT)
#pragma once
#ifdef TIMEBARS
#include "timebars_state.h"
#ifdef __cplusplus
extern "C" {
#endif
void tbDisplay(void);
#ifdef __cplusplus
}
#endif
#else
#define tbInit()
#define tbStartTimer(a, b)
#define tbEndTimer(a)
#define tbDisplay()
#endif
//- rouz edit (ChatGPT)
