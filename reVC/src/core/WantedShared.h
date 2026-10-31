//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "AudioCrimes.h"
#ifdef __cplusplus
class CVector;class CPed;class CVehicle;class CCopPed;
#else
typedef struct CVector CVector;typedef struct CPed CPed;typedef struct CVehicle CVehicle;typedef struct CCopPed CCopPed;
#endif
typedef AudioCrimePosition WantedVector;
typedef struct WantedPositionRef {const WantedVector *plain;const CVector *native;} WantedPositionRef;
#ifdef __cplusplus
WantedVector Wanted_VectorFromNative(const CVector &value);
WantedPositionRef Wanted_ReferenceNative(const CVector &value);
#endif
//- rouz edit (ChatGPT)
