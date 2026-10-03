//+ rouz edit (ChatGPT)
#pragma once

#include "AudioCrimes.h"
#include "AudioPoliceState.h"
#include "AudioPoliceQueue.h"

#ifdef __cplusplus
static_assert(sizeof(cPoliceRadioQueue) == 244, "Police radio queue layout");
#else
_Static_assert(sizeof(cPoliceRadioQueue) == 244, "Police radio queue layout");
#endif
//- rouz edit (ChatGPT)
