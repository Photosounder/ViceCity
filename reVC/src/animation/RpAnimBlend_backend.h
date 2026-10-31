//+ rouz edit (ChatGPT)
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
