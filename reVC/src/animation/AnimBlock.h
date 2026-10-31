//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#define MAX_ANIMBLOCK_NAME 20
typedef struct CAnimBlock {
    char name[MAX_ANIMBLOCK_NAME];
    bool isLoaded;
    int16_t refCount;
    int32_t firstIndex,numAnims;
} CAnimBlock;
//- rouz edit (ChatGPT)
