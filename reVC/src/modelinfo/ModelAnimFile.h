//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
typedef union ModelAnimFile {
    int32_t index;
    char *name;
} ModelAnimFile;
#ifdef __cplusplus
extern "C" {
#endif
void ModelAnimFile_Set(ModelAnimFile *anim,const char *file);
void ModelAnimFile_Convert(ModelAnimFile *anim);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
