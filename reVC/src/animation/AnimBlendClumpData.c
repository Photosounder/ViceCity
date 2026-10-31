//+ rouz edit (ChatGPT)
#include "AnimBlendClumpData.h"
#include "AnimBlendClumpData_backend.h"
#include <stddef.h>
void AnimBlendClump_Init(CAnimBlendClumpData *self)
{
    // Retain constructor field order and initialize the embedded link last
    self->numFrames=0;self->velocity2d=NULL;self->frames=NULL;AnimBlendLink_Init(&self->link);
}
void AnimBlendClump_Destroy(CAnimBlendClumpData *self)
{
    // Detach the clump link before releasing any existing frame allocation
    AnimBlendLink_Remove(&self->link);
    if(self->frames)AnimBlendClump_BackendFree(self->frames);
}
void AnimBlendClump_SetNumberOfFrames(CAnimBlendClumpData *self,int32_t count)
{
    // Free old frames before storing the new signed count and forwarding the original unsigned allocation size
    if(self->frames)AnimBlendClump_BackendFree(self->frames);
    self->numFrames=count;
    self->frames=(AnimBlendFrameData*)AnimBlendClump_BackendAllocate((uint32_t)((size_t)self->numFrames*sizeof(AnimBlendFrameData)),64);
}
void AnimBlendClump_SetNumberOfBones(CAnimBlendClumpData *self,int32_t count)
{
    // Preserve the original bone-count alias at the same allocation boundary
    AnimBlendClump_SetNumberOfFrames(self,count);
}
void AnimBlendClump_ForAllFrames(CAnimBlendClumpData *self,AnimBlendFrameCallback callback,void *arg)
{
    // Re-read count and frame storage around every callback so callbacks can replace or resize the live container
    // Use the C animation frame while retaining the original live SDK objects
    for(int32_t i=0;i<self->numFrames;++i)callback(&self->frames[i],arg);
}
//- rouz edit (ChatGPT)
