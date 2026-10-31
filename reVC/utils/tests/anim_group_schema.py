"""Define complete C association-group and animation-block records before moving group algorithms."""
DECLARATIONS=[('void','Init','CAnimBlendAssocGroup *self'),('void','Destroy','CAnimBlendAssocGroup *self'),('void','DestroyAssociations','CAnimBlendAssocGroup *self'),('CAnimBlendAssociation *','GetById','CAnimBlendAssocGroup *self,uint32_t id'),('CAnimBlendAssociation *','GetByName','CAnimBlendAssocGroup *self,const char *name'),('CAnimBlendAssociation *','CopyById','CAnimBlendAssocGroup *self,uint32_t id'),('CAnimBlendAssociation *','CopyByName','CAnimBlendAssocGroup *self,const char *name'),('void','CreateForModels','CAnimBlendAssocGroup *self,const char *name'),('void','CreateForClump','CAnimBlendAssocGroup *self,const char *blockName,void *clump,const char **animNames,int32_t numAssocs')]

def block_header():
    # Retain every original animation-block field with C11 scalar types
    return '''//+ rouz edit (ChatGPT)
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
'''

def group_header():
    # Keep group ownership state in a complete C record with explicit overload names
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "AnimBlock.h"
#include "AnimBlendAssociation.h"
typedef struct CAnimBlendAssocGroup {
    CAnimBlock *animBlock;
    CAnimBlendAssociation *assocList;
    int32_t numAssociations,firstAnimId,groupId;
} CAnimBlendAssocGroup;
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' AnimGroup_'+name+'('+args+');\n' for result,name,args in DECLARATIONS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
