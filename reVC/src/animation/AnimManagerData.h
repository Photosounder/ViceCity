//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
typedef struct AnimAssocDesc {int32_t animId,flags;} AnimAssocDesc;
typedef struct AnimAssocDefinition {
    const char *name,*blockName;
    int32_t modelIndex,numAnims;
    const char **animNames;
    AnimAssocDesc *animDescs;
} AnimAssocDefinition;
//- rouz edit (ChatGPT)
