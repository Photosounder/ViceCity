"""Move complete plugin lifetime, hierarchy setup, association queries and playback policy into C11."""
import re
from rect_migration import mask,closing,arguments
from anim_plugin_schema import DECLARATIONS,header,backend_header
import object_names_migration as objects
RENAMES={'src/animation/RpAnimBlend.cpp':'src/animation/RpAnimBlend.c'}

def identifiers(source,replacements):
    # Change complete code identifiers while preserving comments and string contents
    pattern=r'\b(?:'+'|'.join(re.escape(name) for name in replacements)+r')\b'
    for match in reversed(list(re.finditer(pattern,mask(source)))):
        # Preserve source offsets while replacing complete identifiers from the end
        source=source[:match.start()]+replacements[match.group()]+source[match.end():]
    return source

def kernel(original):
    # Keep every complete original algorithm and global before moving only native allocator and metadata leaves
    source=original[:original.index('void *AnimBlendClump_BackendAllocate(')]
    source=re.sub(r'^#include[^\n]*\n','',source,flags=re.M)
    source=source[source.index('RwInt32 ClumpOffset;'):]
    source=source.replace('MAKECHUNKID(rwVENDORID_ROCKSTAR, 0xFD)','ANIM_PLUGIN_ID')
    source=source.replace('*RWPLUGINOFFSET(CAnimBlendClumpData*, object, offsetInObject)','*(CAnimBlendClumpData**)((unsigned char*)object+offsetInObject)')
    source=source.replace('RpAnimBlendGetNextAssociation(CAnimBlendAssociation *assoc, uint32 mask)','RpAnimBlendGetNextAssociationWithMask(CAnimBlendAssociation *assoc, uint32 mask)')
    source=source.replace('RpAnimBlendClumpGetFirstAssociation(RpClump *clump, uint32 mask)','RpAnimBlendClumpGetFirstAssociationWithMask(RpClump *clump, uint32 mask)')
    source=source.replace('FrameInitCBnonskin(AnimBlendFrameData *frameData, void*)','FrameInitCBnonskin(AnimBlendFrameData *frameData, void *argument)').replace('FrameInitCBskin(AnimBlendFrameData *frameData, void*)','FrameInitCBskin(AnimBlendFrameData *frameData, void *argument)')
    source=source.replace('AnimBlendFrame_FromNativePosition(*RwMatrixGetPos(RwFrameGetMatrix(AnimBlendFrame_GetFrame(frameData))))','AnimPlugin_HostFramePosition(frameData->frame)')
    source=source.replace('RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic))','AnimPlugin_HostSkinForAtomic(atomic)')
    source=source.replace('AnimBlendFrame_FromNativePosition(boneTab[i])','boneTab[i]')
    pattern=r'#ifdef LIBRW\n\s*frames\[i\]\.hanimFrame = [^\n]+;\n#else\n\s*frames\[i\]\.hanimFrame = [^\n]+;\n#endif'
    source,count=re.subn(pattern,'\t\tframes[i].hanimFrame = AnimPlugin_HostInterpFrame(hier, i);',source);assert count==1
    source=source.replace('CVisibilityPlugins::GetFrameHierarchyId(AnimBlendFrame_GetFrame(frame))','AnimPlugin_HostFrameHierarchyId(frame->frame)')
    source=source.replace('GetFrameNodeName(AnimBlendFrame_GetFrame(frame))','AnimPlugin_HostFrameName(frame->frame)')
    replacements={'RwInt32':'int32_t','uint32':'uint32_t','int32':'int32_t','uintptr':'uintptr_t','RpClump':'void','RwFrame':'void','RpAtomic':'void','RpSkin':'void','RpHAnimHierarchy':'void','RwV3d':'KeyFrameVector3','nil':'NULL','assert':'AnimPlugin_Assert','debug':'AnimPlugin_Debug','RpClumpRegisterPlugin':'AnimPlugin_HostRegister','RwFrameForAllChildren':'AnimPlugin_HostForChildren','GetFirstAtomic':'AnimPlugin_HostFirstAtomic','RpSkinGetNumBones':'AnimPlugin_HostNumBones','GetAnimHierarchyFromSkinClump':'AnimPlugin_HostHierarchy','SkinGetBonePositionsToTable':'AnimPlugin_HostBonePositions','HIERNODEID':'AnimPlugin_HostNodeId','IsClumpSkinned':'AnimPlugin_HostIsSkinned','RpClumpGetFrame':'AnimPlugin_HostClumpFrame','RpHAnimIDGetIndex':'AnimPlugin_HostFindNode','RwFrameUpdateObjects':'AnimPlugin_HostUpdateObjects'}
    source=identifiers(source,replacements)
    source=source.replace('(void*)boneTag','(void*)(uintptr_t)boneTag')
    source=re.sub(r'(?m)^(\w[^\n]*\([^\n]*\)\n\{)\n',r'\1\n    // Preserve the entire original plugin operation and live field access order\n',source)
    source=source.replace('sizeInObject) { return NULL; }','sizeInObject) {\n    // Preserve the original unsupported plugin copy result\n    return NULL;\n}')
    prefix='''//+ rouz edit (ChatGPT)
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "RpAnimBlend.h"
#include "RpAnimBlend_backend.h"
#include "AnimBlendClumpData.h"
#include "AnimBlendAssociation_backend.h"
#include "AnimManager.h"
#include "General.h"
#include "Bones.h"
#include "PedNode.h"
#ifndef MASTER
#define AnimPlugin_Assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define AnimPlugin_Assert(expression) (expression)
#endif
#define AnimPlugin_Debug(...) re3_debug("[DBG]: " __VA_ARGS__)
'''
    aliases='''
CAnimBlendClumpData *AnimAssociation_HostClumpData(void *clump)
{
    // Borrow the actual plugin owner directly from the complete C plugin implementation
    return *RPANIMBLENDCLUMPDATA(clump);
}
AnimBlendFrameData *AnimAssociation_HostFindFrame(void *clump,const char *name)
{
    // Invoke the complete C frame-name search through its existing owner boundary
    return RpAnimBlendClumpFindFrame(clump,name);
}
AnimBlendFrameData *AnimAssociation_HostFindBone(void *clump,uint32_t tag)
{
    // Invoke the complete C bone-tag search without a native algorithm bridge
    return RpAnimBlendClumpFindBone(clump,tag);
}
'''
    return prefix+source+aliases+'//- rouz edit (ChatGPT)\n'

def native(original):
    # Retain actual SDK field access and existing allocator primitives without moving plugin algorithms into C++
    start=original.index('void *AnimBlendClump_BackendAllocate(');end=original.index('CAnimBlendClumpData *AnimAssociation_HostClumpData(',start)
    memory=original[start:end]
    return '''//+ rouz edit (ChatGPT)
#include "common.h"
#include "MemoryHeap.h"
#include "MemoryMgr.h"
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
    return GetFirstAtomic(static_cast<RpClump*>(clump));
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
    return GetAnimHierarchyFromSkinClump(static_cast<RpClump*>(clump));
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
'''+memory+'//- rouz edit (ChatGPT)\n'

def anim_plugin_calls(source):
    # Move the complete owner and public API before updating diagnosed overload and default-argument consumers
    if 'RwInt32 ClumpOffset;' in source and 'RpAnimBlendClumpInitSkinned(RpClump *clump)' in source:return kernel(source)
    if 'extern RwInt32 ClumpOffset;' in source and '#define RPANIMBLENDCLUMPDATA' in source:return header()
    original=source;code=mask(source)
    for match in reversed(list(re.finditer(r'\b(RpAnimBlendGetNextAssociation|RpAnimBlendClumpGetFirstAssociation|RpAnimBlendClumpUpdateAnimations)\s*\(',code))):
        # Resolve overloads only by their diagnosed argument count and retain explicit existing render decisions
        end=closing(code,match.end()-1);values=arguments(source[match.end():end-1]);name=match[1]
        if name=='RpAnimBlendClumpUpdateAnimations':
            if len(values)==2:source=source[:end-1]+', true'+source[end-1:]
            else:assert len(values)==3,values
        elif len(values)==2:source=source[:match.start()]+name+'WithMask'+source[match.end(1):]
        else:assert len(values)==1,values
    if source==original:return source
    lines=[];oldLines=set(original.splitlines())
    for line in source.splitlines():
        # Describe each changed executable call at its original native control-flow boundary
        if line not in oldLines and ('WithMask(' in line or 'RpAnimBlendClumpUpdateAnimations(' in line):lines.append(line[:len(line)-len(line.lstrip())]+'// Use the complete C animation plugin API with explicit overload and render arguments')
        lines.append(line)
    return '//+ rouz edit (ChatGPT)\n'+'\n'.join(lines)+'\n//- rouz edit (ChatGPT)\n'

def anim_plugin_project(source):
    # Select the complete C plugin owner and its distinct native SDK primitive unit in generated projects
    source=source.replace('src\\animation\\RpAnimBlend.cpp','src\\animation\\RpAnimBlend.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\animation\\RpAnimBlend\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Retain all other metadata while explicitly compiling the plugin algorithms as C11
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';native=match[1][:-len('RpAnimBlend.c')]+'RpAnimBlend_native.cpp'
            if '<ClCompile Include="'+native+'"' not in source:result+='\n    <ClCompile Include="'+native+'" />'
            return result
        source=re.sub(pattern,metadata,source)
    anchor=re.search(r'<ClInclude Include="([^"]*src\\animation\\RpAnimBlend\.h)"\s*/>',source)
    if anchor:
        # Keep the narrow native primitive contract adjacent to the new public C API
        entry='<ClInclude Include="'+anchor[1][:-len('RpAnimBlend.h')]+'RpAnimBlend_backend.h" />'
        if entry not in source:source=source[:anchor.end()]+'\n    '+entry+source[anchor.end():]
    return objects.project(source)



def calls(source):
    # Preserve complete C animation plugin ownership before the frame-name plugin conversion
    import node_name_migration as names
    return names.calls(anim_plugin_calls(source))


def project(source):
    # Retain C animation plugin metadata before adding the complete C name owner and SDK primitives
    import node_name_migration as names
    return names.project(anim_plugin_project(source))
