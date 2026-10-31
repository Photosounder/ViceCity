"""Expose the complete animation manager API and descriptor records without a C++ static class."""
DECLARATIONS=[('void','Initialise','void'),('void','Shutdown','void'),('void','UncompressAnimation','CAnimBlendHierarchy *hier'),('void','RemoveFromUncompressedCache','CAnimBlendHierarchy *hier'),('CAnimBlock *','GetAnimationBlockById','int32_t block'),('CAnimBlock *','GetAnimationBlockByName','const char *name'),('int32_t','GetAnimationBlockIndex','const char *name'),('int32_t','RegisterAnimBlock','const char *name'),('int32_t','GetNumRefsToAnimBlock','int32_t block'),('void','AddAnimBlockRef','int32_t block'),('void','RemoveAnimBlockRefWithoutDelete','int32_t block'),('void','RemoveAnimBlockRef','int32_t block'),('void','RemoveAnimBlock','int32_t block'),('CAnimBlendHierarchy *','GetAnimationByName','const char *name,CAnimBlock *animBlock'),('CAnimBlendHierarchy *','GetAnimationById','int32_t n'),('const char *','GetAnimGroupName','int32_t groupId'),('CAnimBlendAssociation *','CreateAnimAssociation','int32_t groupId,int32_t animId'),('CAnimBlendAssociation *','GetAnimAssociationById','int32_t groupId,int32_t animId'),('CAnimBlendAssociation *','GetAnimAssociationByName','int32_t groupId,const char *name'),('CAnimBlendAssociation *','AddAnimation','void *clump,int32_t groupId,int32_t animId'),('CAnimBlendAssociation *','AddAnimationAndSync','void *clump,CAnimBlendAssociation *syncanim,int32_t groupId,int32_t animId'),('CAnimBlendAssociation *','BlendAnimation','void *clump,int32_t groupId,int32_t animId,float delta'),('void','LoadAnimFiles','void'),('void','LoadAnimFileByName','const char *filename'),('void','LoadAnimStream','void *stream,bool compress,char (*uncompressedAnims)[32]'),('void','CreateAnimAssocGroups','void'),('void','RemoveLastAnimFile','void'),('CAnimBlendAssocGroup *','GetAnimAssocGroups','void')]

def data_header():
    # Keep complete original descriptor fields in C with unchanged pointer and integer layouts
    return '''//+ rouz edit (ChatGPT)
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
'''

def header():
    # Expose every manager overload explicitly while keeping SDK stream and clump objects opaque
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AnimBlendHierarchy.h"
#include "AnimBlendAssociation.h"
#include "AnimBlendAssocGroup.h"
#include "AnimBlock.h"
#include "AnimationId.h"
#include "AnimGroups.h"
#include "AnimManagerData.h"
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' AnimManager_'+name+'('+args+');\n' for result,name,args in DECLARATIONS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
