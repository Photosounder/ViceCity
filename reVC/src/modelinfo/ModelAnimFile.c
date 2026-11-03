//+ rouz edit (ChatGPT)
#include "ModelAnimFile.h"
#include "AnimManager.h"
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#define ModelAnimFile_Compare _stricmp
#else
#include <strings.h>
#define ModelAnimFile_Compare strcasecmp
#endif

void ModelAnimFile_Set(ModelAnimFile *anim,const char *file)
{
    // Ignore the original null token and copy all other file names
    if (ModelAnimFile_Compare(file,"null")==0) return;
    anim->name=(char*)malloc(strlen(file)+1);
    strcpy(anim->name,file);
}

void ModelAnimFile_Convert(ModelAnimFile *anim)
{
    // Resolve and free the original name only when the sentinel is absent
    if (anim->index!=-1) {
        int32_t index=AnimManager_GetAnimationBlockIndex(anim->name);
        free(anim->name);
        anim->index=index;
    }
}
//- rouz edit (ChatGPT)
