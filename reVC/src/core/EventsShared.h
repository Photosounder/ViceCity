//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CEntity;class CPed;
#else
typedef struct CEntity CEntity;typedef struct CPed CPed;
#endif
typedef struct EventsVector {float x,y,z;} EventsVector;
#ifdef __cplusplus
class CVector;
EventsVector Events_VectorFromNative(const CVector &value);
#endif
//- rouz edit (ChatGPT)
