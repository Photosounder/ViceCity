"""Project the complete animation manager implementation into C without changing policy."""
import re
from anim_manager_schema import DECLARATIONS, header, data_header
from anim_manager_data import data
from anim_association_migration import original_body
from rect_migration import mask, closing, arguments

RENAMES={'src/animation/AnimManager.cpp':'src/animation/AnimManager.c'}

SIGNATURES={
    'GetAnimationBlockByName':'GetAnimationBlock(const char *name)',
    'GetAnimationByName':'GetAnimation(const char *name, CAnimBlock *animBlock)',
    'GetAnimGroupName':'GetAnimGroupName(AssocGroupId groupId)',
    'CreateAnimAssociation':'CreateAnimAssociation(AssocGroupId groupId, AnimationId animId)',
    'GetAnimAssociationById':'GetAnimAssociation(AssocGroupId groupId, AnimationId animId)',
    'GetAnimAssociationByName':'GetAnimAssociation(AssocGroupId groupId, const char *name)',
    'AddAnimation':'AddAnimation(RpClump *clump, AssocGroupId groupId, AnimationId animId)',
    'AddAnimationAndSync':'AddAnimationAndSync(RpClump *clump, CAnimBlendAssociation *syncanim, AssocGroupId groupId, AnimationId animId)',
    'BlendAnimation':'BlendAnimation(RpClump *clump, AssocGroupId groupId, AnimationId animId, float delta)',
    'LoadAnimFileByName':'LoadAnimFile(const char *filename)',
    'LoadAnimStream':'LoadAnimFile(RwStream *stream, bool compress, char (*uncompressedAnims)[32])',
}
INLINE={
    'GetAnimationBlockById':'return &ms_aAnimBlocks[block];',
    'GetAnimationById':'return &ms_aAnimations[n];',
    'GetAnimAssocGroups':'return ms_aAnimAssocGroups;',
}
HOSTS=[('void','RemoveAnim','int32_t block','CStreaming::RemoveAnim(block)'),
       ('void *','OpenStream','const char *filename','RwStreamOpen(rwSTREAMFILENAME,rwSTREAMREAD,filename)'),
       ('void','ReadStream','void *stream,void *buffer,uint32_t size','RwStreamRead(static_cast<RwStream*>(stream),buffer,size)'),
       ('void','CloseStream','void *stream','RwStreamClose(static_cast<RwStream*>(stream),nil)'),
       ('int','CompareNames','const char *a,const char *b','strcasecmp(a,b)')]

def backend_header():
    # Declare only native stream, streaming and string primitives outside the C manager owner
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' AnimManager_Host'+name+'('+args+');\n' for result,name,args,_ in HOSTS)+'''void re3_assert(const char *expression,const char *file,unsigned int line,const char *function);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def replace_code(source,replacements):
    # Replace identifiers only in code so comments and animation names remain unchanged
    matches=list(re.finditer(r'\b(?:'+ '|'.join(re.escape(s) for s in replacements)+r')\b',mask(source)))
    for match in reversed(matches):
        # Keep offsets stable while changing complete code identifiers
        source=source[:match.start()]+replacements[match.group()]+source[match.end():]
    return source

def body(original,name,args):
    # Select exact overload definitions rather than earlier calls with matching names
    if name in INLINE:
        # Retain the original inline getter expression in the C implementation
        return '\n    // Return the original manager storage directly\n    '+INLINE[name]+'\n'
    signature=SIGNATURES.get(name,name+'('+args.replace('int32_t','int32')+')')
    value=original_body(original,'CAnimManager::'+signature)
    value=value.replace('CStreaming::RemoveAnim','AnimManager_HostRemoveAnim')
    value=value.replace('*RPANIMBLENDCLUMPDATA(clump)','AnimAssociation_HostClumpData(clump)')
    value=value.replace('CBaseModelInfo *mi = CModelInfo::GetModelInfo(ms_aAnimAssocDefinitions[i].modelIndex);',
                        'void *mi = AnimGroup_HostModelAt(ms_aAnimAssocDefinitions[i].modelIndex);')
    value=value.replace('RpClump *clump = (RpClump*)mi->CreateInstance();','void *clump = AnimGroup_HostCreateInstance(mi);')
    value=value.replace('RpAnimBlendClumpInit(clump)','AnimGroup_HostInitClump(clump)')
    value=value.replace('IsClumpSkinned(clump)','AnimGroup_HostIsSkinned(clump)')
    value=value.replace('RpClumpForAllAtomics(clump, AtomicRemoveAnimFromSkinCB, nil)','AnimGroup_HostRemoveSkinAnimation(clump)')
    value=value.replace('RpClumpDestroy(clump)','AnimGroup_HostDestroyClump(clump)')
    value=value.replace('RwStream *stream;','void *stream;')
    value=value.replace('RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename)','AnimManager_HostOpenStream(filename)')
    value=value.replace('RwStreamClose(stream, nil)','AnimManager_HostCloseStream(stream)')
    value=value.replace('LoadAnimFile(stream, true)','AnimManager_LoadAnimStream(stream, true, NULL)')
    value=value.replace('struct IfpHeader {','typedef struct IfpHeader {').replace('\n\t};\n\tIfpHeader','\n\t} IfpHeader;\n\tIfpHeader')
    value=value.replace('CVector trans(fbuf[4], fbuf[5], fbuf[6]);','KeyFrameVector3 trans = {fbuf[4], fbuf[5], fbuf[6]};')
    value=value.replace('KeyFrame_VectorRef(trans)','AnimManager_TranslationRef(&trans)').replace('KeyFrame_FromNative(trans)','trans')
    replacements={op:'AnimManager_'+op for _,op,_ in DECLARATIONS if op not in INLINE and op not in SIGNATURES}
    replacements.update({'CreateAnimAssociation':'AnimManager_CreateAnimAssociation','AddAnimationAndSync':'AnimManager_AddAnimationAndSync',
                         'GetAnimationBlock':'AnimManager_GetAnimationBlockByName','GetAnimAssociation':'AnimManager_GetAnimAssociationById',
                         'LoadAnimFile':'AnimManager_LoadAnimFileByName','RwStreamRead':'AnimManager_HostReadStream',
                         'strcasecmp':'AnimManager_HostCompareNames','assert':'AnimManager_Assert','debug':'AnimManager_Debug',
                         'nil':'NULL','int32':'int32_t','uint32':'uint32_t'})
    value=replace_code(value,replacements)
    return '\n    // Preserve the original manager operation and its live field access order\n'+value.lstrip('\n')

def kernel(original):
    # Keep tables, cache policy, block references, association blending and complete IFP parsing in C
    source=data(original)+'''//+ rouz edit (ChatGPT)
#include <stdlib.h>
#include <string.h>
#include "General.h"
#include "AnimManager_backend.h"
#include "AnimBlendAssocGroup_backend.h"
#include "AnimBlendAssociation_backend.h"
#include "AnimBlendHierarchy_backend.h"
#include "KeyFrames.h"
#ifndef MASTER
#define AnimManager_Assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define AnimManager_Assert(expression) (expression)
#endif
#define AnimManager_Debug(...) re3_debug("[DBG]: " __VA_ARGS__)
static KeyFrameVectorRef AnimManager_TranslationRef(const KeyFrameVector3 *value)
{
    // Borrow the actual C translation fields for the existing compression operation
    KeyFrameVectorRef reference = {&value->x,&value->y,&value->z};
    return reference;
}
'''
    for result,name,args in DECLARATIONS:
        # Emit every original operation as a distinct public C function
        source+=result+' AnimManager_'+name+'('+args+')\n{'+body(original,name,args)+'}\n'
    aliases=[('void','AnimHierarchy_HostRemoveCache','CAnimBlendHierarchy *self','AnimManager_RemoveFromUncompressedCache(self)'),
             ('void','AnimAssociation_HostUncompressAnimation','CAnimBlendHierarchy *hierarchy','AnimManager_UncompressAnimation(hierarchy)'),
             ('CAnimBlock *','AnimGroup_HostBlock','const char *name','AnimManager_GetAnimationBlockByName(name)'),
             ('CAnimBlendHierarchy *','AnimGroup_HostAnimationId','int32_t id','AnimManager_GetAnimationById(id)'),
             ('CAnimBlendHierarchy *','AnimGroup_HostAnimationName','const char *name,CAnimBlock *block','AnimManager_GetAnimationByName(name,block)')]
    for result,name,args,expression in aliases:
        # Retain existing C owner entry points as direct forwards into the complete C manager
        source+=result+' '+name+'('+args+')\n{\n    // Forward the existing C owner boundary to manager storage and policy\n    '+('' if result=='void' else 'return ')+expression+';\n}\n'
    return source+'//- rouz edit (ChatGPT)\n'

def native(original):
    # Move only preexisting SDK leaves and the five native primitives to a separate translation unit
    start=original.index('void * AnimGroup_HostModelAt(')
    leaves=original[start:].replace('//- rouz edit (ChatGPT)','')
    source='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "AnimManager_backend.h"
#include "AnimBlendAssocGroup_backend.h"
#include "ModelInfo.h"
#include "RpAnimBlend.h"
#include "RwHelper.h"
#include "Streaming.h"
'''+leaves
    for result,name,args,expression in HOSTS:
        # Forward each individual SDK or native subsystem primitive without moving manager policy
        source+=result+' AnimManager_Host'+name+'('+args+')\n{\n    // Forward only the original native primitive\n    '+('' if result=='void' else 'return ')+expression+';\n}\n'
    return source+'//- rouz edit (ChatGPT)\n'

def anim_manager_calls(source):
    # Replace complete owner definitions before touching native consumer call sites
    if 'class CAnimManager\n' in source:
        # Expose complete C records and explicit overload names without a compatibility class
        return header()
    if 'CAnimBlock CAnimManager::ms_aAnimBlocks[' in source:
        # Convert every manager operation and remove native SDK bodies from the C producer
        return kernel(source)
    matches=list(re.finditer(r'\bCAnimManager::(\w+)\s*\(',mask(source)))
    if not matches:
        # Leave unrelated files byte-for-byte unchanged
        return source
    for match in reversed(matches):
        # Resolve actual overload argument types from the diagnosed finite consumer inventory
        opening=match.end()-1;end=closing(mask(source),opening)
        values=arguments(source[opening+1:end-1]);name=match[1]
        if name=='GetAnimationBlock':
            # Distinguish string labels from integer streaming and player block indices
            arg=values[0]
            names={'key','groupName'};indices={'idleAnimBlockIndex','streamId-STREAM_OFFSET_ANIM','animId'}
            assert arg.startswith('"') or arg in names|indices,(name,arg)
            name+='ByName' if arg.startswith('"') or arg in names else 'ById'
        elif name=='GetAnimation':
            # Every external hierarchy lookup uses a diagnosed integer index
            assert len(values)==1 and values[0] in {'i','animIndex','0'},values
            name+='ById'
        elif name=='GetAnimAssociation':
            # The fight data parser alone requests associations by a string name
            arg=values[1]
            assert arg=='animName' or arg.startswith('ANIM_') or arg in {'GetCrouchFireAnim(curWeapon)','fireAnim','moveAnims[i]'},values
            name+='ByName' if arg=='animName' else 'ById'
        elif name=='LoadAnimFile':
            # Both native stream consumers provide the original explicit third argument
            assert len(values)==3 and values[0]=='stream',values
            name='LoadAnimStream'
        assert name in {op for _,op,_ in DECLARATIONS},name
        source=source[:match.start()]+'AnimManager_'+name+source[opening:]
    lines=source.splitlines(keepends=True);result=[]
    for line in lines:
        # Describe each added native call block while keeping its surrounding control flow intact
        if 'AnimManager_' in mask(line):
            indent=line[:len(line)-len(line.lstrip())]
            result.append(indent+'// Use the complete C animation manager at the original call boundary\n')
        result.append(line)
    return '//+ rouz edit (ChatGPT)\n'+''.join(result)+'//- rouz edit (ChatGPT)\n'

def anim_manager_project(source):
    # Select the C manager producer and its separate native primitive unit in existing project formats
    import object_names_migration as objects
    source=source.replace('src\\animation\\AnimManager.cpp','src\\animation\\AnimManager.c')
    pattern=r'<ClCompile Include="([^"]*src\\animation\\AnimManager\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
    def metadata(match):
        # Preserve existing metadata and explicitly select C11 for the complete algorithm producer
        path=match[1];body=match[2] or ''
        for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
            if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
            else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
        result='<ClCompile Include="'+path+'">'+body.rstrip()+'\n    </ClCompile>'
        native=path[:-len('AnimManager.c')]+'AnimManager_native.cpp'
        if '<ClCompile Include="'+native+'"' not in source:result+='\n    <ClCompile Include="'+native+'" />'
        return result
    if 'Microsoft.Cpp.targets' in source:source=re.sub(pattern,metadata,source)
    anchor=re.search(r'<ClInclude Include="([^"]*src\\animation\\AnimManager\.h)"\s*/>',source)
    if anchor:
        # Include complete descriptor records and primitive declarations beside the public manager API
        prefix=anchor[1][:-len('AnimManager.h')];entries=''
        for name in ['AnimManagerData.h','AnimManager_backend.h']:
            entry='<ClInclude Include="'+prefix+name+'" />'
            if entry not in source:entries+='\n    '+entry
        source=source[:anchor.end()]+entries+source[anchor.end():]
    return objects.project(source)

if __name__=='__main__':
    from pathlib import Path
    # Generate an isolated candidate without changing any production manager source or caller
    root=Path(__file__).resolve().parents[2]
    destination=root/'.diagnostics/anim-manager-c-tests/schema'
    original=(root/'src/animation/AnimManager.cpp').read_text()
    destination.mkdir(parents=True,exist_ok=True)
    for name,value in [('AnimManager.h',header()),('AnimManagerData.h',data_header()),('AnimManager_backend.h',backend_header()),('AnimManager.c',kernel(original)),('AnimManager_native.cpp',native(original))]:
        # Write generated candidates with consistent LF line endings
        (destination/name).write_text(value,newline='\n')
    print('Generated complete C manager candidate and native primitive leaves')



def calls(source):
    # Preserve complete C manager ownership before the bone helper and shared constant conversion
    import bones_migration as bones
    return bones.calls(anim_manager_calls(source))


def project(source):
    # Retain all manager integration metadata before selecting the C11 bone mapping producer
    import bones_migration as bones
    return bones.project(anim_manager_project(source))
