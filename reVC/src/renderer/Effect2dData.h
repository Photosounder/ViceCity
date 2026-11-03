//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "CollisionVector.h"
typedef struct Effect2dLight {
    float dist,range,size,shadowSize;
    uint8_t lightType,roadReflection,flareType,shadowIntensity,flags;
    void *corona,*shadow;
} Effect2dLight;
typedef struct Effect2dColor { uint8_t r,g,b,a; } Effect2dColor;
typedef struct Effect2dParticle {
    int particleType;
    CollisionVector3 dir;
    float scale;
} Effect2dParticle;
typedef struct Effect2dAttractor {
    CollisionVector3 dir;
    int8_t type;
    uint8_t probability;
} Effect2dAttractor;
typedef struct Effect2dPedAttractor {
    CollisionVector3 queueDir;
    CollisionVector3 useDir;
    int8_t type;
} Effect2dPedAttractor;
//- rouz edit (ChatGPT)
