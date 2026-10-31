"""Expose the animation plugin and playback API without SDK types, overloads or default arguments."""
DECLARATIONS=[
('bool','RpAnimBlendPluginAttach','void'),
('CAnimBlendAssociation *','RpAnimBlendGetNextAssociation','CAnimBlendAssociation *assoc'),
('CAnimBlendAssociation *','RpAnimBlendGetNextAssociationWithMask','CAnimBlendAssociation *assoc,uint32_t mask'),
('void','RpAnimBlendAllocateData','void *clump'),
('void','RpAnimBlendClumpSetBlendDeltas','void *clump,uint32_t mask,float delta'),
('void','RpAnimBlendClumpRemoveAllAssociations','void *clump'),
('void','RpAnimBlendClumpRemoveAssociations','void *clump,uint32_t mask'),
('void','RpAnimBlendClumpInit','void *clump'),
('bool','RpAnimBlendClumpIsInitialized','void *clump'),
('void','RpAnimBlendClumpFillFrameArray','void *clump,AnimBlendFrameData **frames'),
('AnimBlendFrameData *','RpAnimBlendClumpFindFrame','void *clump,const char *name'),
('AnimBlendFrameData *','RpAnimBlendClumpFindBone','void *clump,uint32_t boneTag'),
('CAnimBlendAssociation *','RpAnimBlendClumpGetAssociation','void *clump,uint32_t id'),
('CAnimBlendAssociation *','RpAnimBlendClumpGetMainAssociation','void *clump,CAnimBlendAssociation **assocRet,float *blendRet'),
('CAnimBlendAssociation *','RpAnimBlendClumpGetMainPartialAssociation','void *clump'),
('CAnimBlendAssociation *','RpAnimBlendClumpGetMainAssociation_N','void *clump,int n'),
('CAnimBlendAssociation *','RpAnimBlendClumpGetMainPartialAssociation_N','void *clump,int n'),
('CAnimBlendAssociation *','RpAnimBlendClumpGetFirstAssociationWithMask','void *clump,uint32_t mask'),
('CAnimBlendAssociation *','RpAnimBlendClumpGetFirstAssociation','void *clump'),
('void','RpAnimBlendNodeUpdateKeyframes','AnimBlendFrameData *frames,AnimBlendFrameUpdateData *updateData,int32_t numNodes'),
('void','RpAnimBlendClumpUpdateAnimations','void *clump,float timeDelta,bool doRender')]

def header():
    # Keep existing free function names where unambiguous and expose opaque clump handles to C11
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AnimBlendAssociation.h"
#include "AnimBlendFrameUpdate.h"
typedef struct CAnimBlendClumpData CAnimBlendClumpData;
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t ClumpOffset;
static inline CAnimBlendClumpData **RpAnimBlendClumpDataSlot(void *clump)
{
    // Address only the SDK-registered plugin slot using its original byte offset
    return (CAnimBlendClumpData**)((unsigned char*)clump+ClumpOffset);
}
#define RPANIMBLENDCLUMPDATA(object) (RpAnimBlendClumpDataSlot((void*)(object)))
'''+''.join(result+' '+name+'('+args+');\n' for result,name,args in DECLARATIONS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Limit native interfaces to SDK registration, enumeration, metadata and existing external skin position ownership
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include "RpAnimBlend.h"
enum {ANIM_PLUGIN_ID=0x0253F2FD};
typedef void *(*AnimPluginLifecycle)(void *object,int32_t offset,int32_t size);
typedef void *(*AnimPluginCopy)(void *destination,const void *source,int32_t offset,int32_t size);
typedef void *(*AnimPluginFrameCallback)(void *frame,void *argument);
#ifdef __cplusplus
extern "C" {
#endif
int32_t AnimPlugin_HostRegister(size_t bytes,uint32_t id,AnimPluginLifecycle create,AnimPluginLifecycle destroy,AnimPluginCopy copy);
void AnimPlugin_HostForChildren(void *frame,AnimPluginFrameCallback callback,void *argument);
void *AnimPlugin_HostClumpFrame(void *clump);
void *AnimPlugin_HostFirstAtomic(void *clump);
void *AnimPlugin_HostSkinForAtomic(void *atomic);
int32_t AnimPlugin_HostNumBones(void *skin);
void *AnimPlugin_HostHierarchy(void *clump);
void AnimPlugin_HostBonePositions(void *clump,KeyFrameVector3 *positions);
int32_t AnimPlugin_HostNodeId(void *hierarchy,int32_t index);
void *AnimPlugin_HostInterpFrame(void *hierarchy,int32_t index);
bool AnimPlugin_HostIsSkinned(void *clump);
int32_t AnimPlugin_HostFindNode(void *hierarchy,int32_t tag);
int32_t AnimPlugin_HostFrameHierarchyId(void *frame);
char *AnimPlugin_HostFrameName(void *frame);
KeyFrameVector3 AnimPlugin_HostFramePosition(void *frame);
void AnimPlugin_HostUpdateObjects(void *frame);
void re3_assert(const char *expression,const char *file,unsigned int line,const char *function);
void re3_debug(const char *format,...);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
