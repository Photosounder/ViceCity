//+ rouz edit (ChatGPT)
#include "common.h"
#include "AnimManager_backend.h"
#include "AnimBlendAssocGroup_backend.h"
#include "ModelInfo.h"
#include "RpAnimBlend.h"
#include "RwHelper.h"
#include "Streaming.h"
void * AnimGroup_HostModelObject(void *model)
{
    // Forward only the original manager or native metadata primitive
    return static_cast<CBaseModelInfo*>(model)->GetRwObject();
}
bool AnimGroup_HostObjectIsClump(void *object)
{
    // Forward only the original manager or native metadata primitive
    return RwObjectGetType(static_cast<RwObject*>(object))==rpCLUMP;
}
void * AnimGroup_HostCreateInstance(void *model)
{
    // Forward only the original manager or native metadata primitive
    return static_cast<CBaseModelInfo*>(model)->CreateInstance();
}
void AnimGroup_HostInitClump(void *clump)
{
    // Forward only the original manager or native metadata primitive
    RpAnimBlendClumpInit(static_cast<RpClump*>(clump));
}
bool AnimGroup_HostIsSkinned(void *clump)
{
    // Forward only the original manager or native metadata primitive
    return IsClumpSkinned(static_cast<RpClump*>(clump));
}
void AnimGroup_HostRemoveSkinAnimation(void *clump)
{
    // Forward only the original manager or native metadata primitive
    RpClumpForAllAtomics(static_cast<RpClump*>(clump),AtomicRemoveAnimFromSkinCB,nil);
}
void AnimGroup_HostDestroyClump(void *clump)
{
    // Forward only the original manager or native metadata primitive
    RpClumpDestroy(static_cast<RpClump*>(clump));
}
int AnimGroup_HostPrefixCompare(const char *a,const char *b,size_t count)
{
    // Forward only the original manager or native metadata primitive
    return strncasecmp(a,b,count);
}

void AnimManager_HostRemoveAnim(int32_t block)
{
    // Forward only the original native primitive
    Streaming_RemoveAnim(block); // rouz edit (ChatGPT)
}
void * AnimManager_HostOpenStream(const char *filename)
{
    // Forward only the original native primitive
    return RwStreamOpen(rwSTREAMFILENAME,rwSTREAMREAD,filename);
}
void AnimManager_HostReadStream(void *stream,void *buffer,uint32_t size)
{
    // Forward only the original native primitive
    RwStreamRead(static_cast<RwStream*>(stream),buffer,size);
}
void AnimManager_HostCloseStream(void *stream)
{
    // Forward only the original native primitive
    RwStreamClose(static_cast<RwStream*>(stream),nil);
}
int AnimManager_HostCompareNames(const char *a,const char *b)
{
    // Forward only the original native primitive
    return strcasecmp(a,b);
}
//- rouz edit (ChatGPT)
