//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "MemoryHeap.h"
#include "MemoryMgr_native.h"
#include "RwHelper.h"
#include "NodeName.h"
#include "VisibilityPlugins.h"
#include "AnimBlendClumpData_backend.h"
#include "AnimBlendSequence_backend.h"
#include "RpAnimBlend_backend.h"
static_assert(MAKECHUNKID(rwVENDORID_ROCKSTAR,0xFD)==ANIM_PLUGIN_ID,"Original animation plugin ID");
struct AnimPluginNativeVisit {AnimPluginFrameCallback callback;void *argument;};
static RwFrame *AnimPlugin_NativeVisit(RwFrame *frame,void *argument)
{
    // Forward the actual SDK frame and original opaque argument into the complete C traversal callback
    AnimPluginNativeVisit *visit=static_cast<AnimPluginNativeVisit*>(argument);return static_cast<RwFrame*>(visit->callback(frame,visit->argument));
}
int32_t AnimPlugin_HostRegister(size_t bytes,uint32_t id,AnimPluginLifecycle create,AnimPluginLifecycle destroy,AnimPluginCopy copy)
{
    // Register the original plugin size, identifier and complete C lifecycle callbacks with the SDK
    return RpClumpRegisterPlugin(static_cast<int32_t>(bytes),id,create,destroy,copy);
}
void AnimPlugin_HostForChildren(void *frame,AnimPluginFrameCallback callback,void *argument)
{
    // Adapt only SDK callback pointer types while the C owner keeps recursion and traversal policy
    AnimPluginNativeVisit visit={callback,argument};RwFrameForAllChildren(static_cast<RwFrame*>(frame),AnimPlugin_NativeVisit,&visit);
}
void *AnimPlugin_HostClumpFrame(void *clump)
{
    // Borrow the original SDK clump root frame
    return RpClumpGetFrame(static_cast<RpClump*>(clump));
}
void *AnimPlugin_HostFirstAtomic(void *clump)
{
    // Preserve the original external atomic lookup operation
    // Use the C helper and retain the original typed SDK result
    return ((RpAtomic*)GetFirstAtomic(static_cast<RpClump*>(clump)));
}
void *AnimPlugin_HostSkinForAtomic(void *atomic)
{
    // Preserve the exact native geometry and skin lookup expression
    return RpSkinGeometryGetSkin(RpAtomicGetGeometry(static_cast<RpAtomic*>(atomic)));
}
int32_t AnimPlugin_HostNumBones(void *skin)
{
    // Read the SDK skin bone count at the original metadata boundary
    return RpSkinGetNumBones(static_cast<RpSkin*>(skin));
}
void *AnimPlugin_HostHierarchy(void *clump)
{
    // Borrow the existing hierarchy from the separate skin helper owner
    // Use the C helper and retain the original typed SDK result
    return ((RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(static_cast<RpClump*>(clump)));
}
void AnimPlugin_HostBonePositions(void *clump,KeyFrameVector3 *positions)
{
    // Transport all initialized caller coordinates into real SDK vector records without arithmetic
    RwV3d nativePositions[64];for(unsigned i=0;i<64;++i){memcpy(&nativePositions[i].x,&positions[i].x,4);memcpy(&nativePositions[i].y,&positions[i].y,4);memcpy(&nativePositions[i].z,&positions[i].z,4);}
    // Invoke the existing external skin-position owner at its original operation boundary
    SkinGetBonePositionsToTable(static_cast<RpClump*>(clump),nativePositions);
    // Transport exact output bits back into caller-owned C records
    for(unsigned i=0;i<64;++i){memcpy(&positions[i].x,&nativePositions[i].x,4);memcpy(&positions[i].y,&nativePositions[i].y,4);memcpy(&positions[i].z,&nativePositions[i].z,4);}
}
int32_t AnimPlugin_HostNodeId(void *hierarchy,int32_t index)
{
    // Read the actual SDK hierarchy node identifier
    return HIERNODEID(static_cast<RpHAnimHierarchy*>(hierarchy),index);
}
void *AnimPlugin_HostInterpFrame(void *hierarchy,int32_t index)
{
    // Preserve the original SDK-specific interpolation frame lookup
#ifdef LIBRW
    return rpHANIMHIERARCHYGETINTERPFRAME(static_cast<RpHAnimHierarchy*>(hierarchy),index);
#else
    return rtANIMGETINTERPFRAME(static_cast<RpHAnimHierarchy*>(hierarchy)->currentAnim,index);
#endif
}
bool AnimPlugin_HostIsSkinned(void *clump)
{
    // Query the original external SDK classification primitive
    return IsClumpSkinned(static_cast<RpClump*>(clump));
}
int32_t AnimPlugin_HostFindNode(void *hierarchy,int32_t tag)
{
    // Forward the original signed bone-tag lookup to the SDK hierarchy
    return RpHAnimIDGetIndex(static_cast<RpHAnimHierarchy*>(hierarchy),tag);
}
int32_t AnimPlugin_HostFrameHierarchyId(void *frame)
{
    // Read the existing visibility plugin metadata from the actual SDK frame
    return CVisibilityPlugins::GetFrameHierarchyId(static_cast<RwFrame*>(frame));
}
char *AnimPlugin_HostFrameName(void *frame)
{
    // Borrow the original mutable frame name without snapshotting its contents
    return GetFrameNodeName(static_cast<RwFrame*>(frame));
}
KeyFrameVector3 AnimPlugin_HostFramePosition(void *frame)
{
    // Borrow the same current SDK matrix position used by native frame initialization
    const RwV3d *position=RwMatrixGetPos(RwFrameGetMatrix(static_cast<RwFrame*>(frame)));
    // Preserve every coordinate bit when returning the C value
    KeyFrameVector3 value;memcpy(&value.x,&position->x,4);memcpy(&value.y,&position->y,4);memcpy(&value.z,&position->z,4);return value;
}
void AnimPlugin_HostUpdateObjects(void *frame)
{
    // Retain the final original SDK object update after playback policy completes
    RwFrameUpdateObjects(static_cast<RwFrame*>(frame));
}
void *AnimBlendClump_BackendAllocate(uint32_t size,uint32_t alignment)
{
    // Preserve the native allocator's original unsigned argument widths
    return RwMallocAlign(size,alignment);
}
void AnimBlendClump_BackendFree(void *memory)
{
    // Retain the existing aligned free boundary
    RwFreeAlign(memory);
}
//- rouz edit (ChatGPT)

void *AnimSequence_BackendAllocate(size_t bytes)
{
    // Retain the SDK allocator's complete size argument
    return RwMalloc(bytes);
}
void AnimSequence_BackendFree(void *memory)
{
    // Keep native free ordering under the C sequence owner
    RwFree(memory);
}



//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
