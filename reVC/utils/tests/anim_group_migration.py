"""Move complete association-group algorithms and model selection into C11."""
import re
from anim_group_schema import DECLARATIONS,block_header,group_header
from anim_association_migration import original_body
from rect_migration import mask,closing,arguments
import object_names_migration as objects
RENAMES={'src/animation/AnimBlendAssocGroup.cpp':'src/animation/AnimBlendAssocGroup.c'}

def header():
    # Expose explicit C lifetime registration at the native cutscene storage's original startup point
    return group_header().replace('#ifdef __cplusplus\n}\n#endif','CAnimBlendAssocGroup AnimGroup_InitStaticCutscene(CAnimBlendAssocGroup *self);\nbool AnimGroup_CompareIgnoringDigits(const char *s1,const char *s2);\nvoid *AnimGroup_FindModel(const char *name);\n#ifdef __cplusplus\n}\n#endif')

def lifetime():
    # Own exit cleanup in C while the existing native client keeps its pre-main initialization timing
    return '''
static CAnimBlendAssocGroup *cutsceneGroup;
static void AnimGroup_DestroyStaticCutscene(void)
{
    // Release the same static cutscene group at process exit through the complete C owner
    AnimGroup_Destroy(cutsceneGroup);
}
CAnimBlendAssocGroup AnimGroup_InitStaticCutscene(CAnimBlendAssocGroup *self)
{
    // Retain pre-main initialization and matching exit registration at the original native storage definition
    AnimGroup_Init(self);cutsceneGroup=self;atexit(AnimGroup_DestroyStaticCutscene);return *self;
}
'''

def native_backend():
    # Keep manager and SDK primitives as individual leaves without group selection algorithms
    operations=[('CAnimBlock *','Block','const char *name','CAnimManager::GetAnimationBlock(name)'),('CAnimBlendHierarchy *','AnimationId','int32_t id','CAnimManager::GetAnimation(id)'),('CAnimBlendHierarchy *','AnimationName','const char *name,CAnimBlock *block','CAnimManager::GetAnimation(name,block)'),('void *','ModelAt','int32_t id','CModelInfo::GetModelInfo(id)'),('const char *','ModelName','void *model','static_cast<CBaseModelInfo*>(model)->GetModelName()'),('void *','ModelObject','void *model','static_cast<CBaseModelInfo*>(model)->GetRwObject()'),('bool','ObjectIsClump','void *object','RwObjectGetType(static_cast<RwObject*>(object))==rpCLUMP'),('void *','CreateInstance','void *model','static_cast<CBaseModelInfo*>(model)->CreateInstance()'),('void','InitClump','void *clump','RpAnimBlendClumpInit(static_cast<RpClump*>(clump))'),('bool','IsSkinned','void *clump','IsClumpSkinned(static_cast<RpClump*>(clump))'),('void','RemoveSkinAnimation','void *clump','RpClumpForAllAtomics(static_cast<RpClump*>(clump),AtomicRemoveAnimFromSkinCB,nil)'),('void','DestroyClump','void *clump','RpClumpDestroy(static_cast<RpClump*>(clump))'),('int','PrefixCompare','const char *a,const char *b,size_t count','strncasecmp(a,b,count)')]
    return '\n'+''.join(result+' AnimGroup_Host'+name+'('+args+')\n{\n    // Forward only the original manager or native metadata primitive\n    '+('' if result=='void' else 'return ')+expression+';\n}\n' for result,name,args,expression in operations)

def backend_header():
    # Keep external manager and SDK metadata access separate from group selection and lifetime policy
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include "AnimBlendAssocGroup.h"
#ifdef __cplusplus
extern "C" {
#endif
CAnimBlock *AnimGroup_HostBlock(const char *name);
CAnimBlendHierarchy *AnimGroup_HostAnimationId(int32_t id);
CAnimBlendHierarchy *AnimGroup_HostAnimationName(const char *name,CAnimBlock *block);
void *AnimGroup_HostModelAt(int32_t id);
const char *AnimGroup_HostModelName(void *model);
void *AnimGroup_HostModelObject(void *model);
bool AnimGroup_HostObjectIsClump(void *object);
void *AnimGroup_HostCreateInstance(void *model);
void AnimGroup_HostInitClump(void *clump);
bool AnimGroup_HostIsSkinned(void *clump);
void AnimGroup_HostRemoveSkinAnimation(void *clump);
void AnimGroup_HostDestroyClump(void *clump);
int AnimGroup_HostPrefixCompare(const char *a,const char *b,size_t count);
void re3_debug(const char *format,...);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Retain full helper loops and all overload algorithms in the C producer
    source='''//+ rouz edit (ChatGPT)
#include "config.h"
#include "AnimBlendAssocGroup.h"
#include "AnimBlendAssocGroup_backend.h"
#include "AnimBlendAssociation_backend.h"
#include "ModelIndices.h"
#include "General.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#define AnimGroup_Debug(format,...) re3_debug("[DBG]: " format,__VA_ARGS__)
'''
    compare=original_body(original,'strcmpIgnoringDigits(const char *s1, const char *s2)')
    source+='bool AnimGroup_CompareIgnoringDigits(const char *s1,const char *s2)\n{\n    // Preserve paired digit skipping, original prefix matching and locale character queries\n'+compare+'}\n'
    model=original_body(original,'GetModelFromName(const char *name)').replace('CBaseModelInfo *mi;','void *mi;')
    model=model.replace('strncasecmp(','AnimGroup_HostPrefixCompare(').replace('CModelInfo::GetModelInfo(MI_PLAYER)->GetModelName()','AnimGroup_HostModelName(AnimGroup_HostModelAt(MI_PLAYER))').replace('CModelInfo::GetModelInfo(','AnimGroup_HostModelAt(')
    model=model.replace('RwObjectGetType(mi->GetRwObject()) == rpCLUMP','AnimGroup_HostObjectIsClump(AnimGroup_HostModelObject(mi))').replace('mi->GetRwObject()','AnimGroup_HostModelObject(mi)').replace('mi->GetModelName()','AnimGroup_HostModelName(mi)').replace('strcmpIgnoringDigits(','AnimGroup_CompareIgnoringDigits(').replace('nil','NULL')
    source+='void *AnimGroup_FindModel(const char *name)\n{\n    // Keep the complete original player-name substitution and ordered model selection loop\n'+model+'}\n'
    signatures={'Init':'CAnimBlendAssocGroup::CAnimBlendAssocGroup(void)','Destroy':'CAnimBlendAssocGroup::~CAnimBlendAssocGroup(void)','DestroyAssociations':'CAnimBlendAssocGroup::DestroyAssociations(void)','GetById':'CAnimBlendAssocGroup::GetAnimation(uint32 id)','GetByName':'CAnimBlendAssocGroup::GetAnimation(const char *name)','CopyById':'CAnimBlendAssocGroup::CopyAnimation(uint32 id)','CopyByName':'CAnimBlendAssocGroup::CopyAnimation(const char *name)','CreateForModels':'CAnimBlendAssocGroup::CreateAssociations(const char *name)','CreateForClump':'CAnimBlendAssocGroup::CreateAssociations(const char *blockName, RpClump *clump, const char **animNames, int numAssocs)'}
    for result,name,args in DECLARATIONS:
        body=original_body(original,signatures[name]);body=body.replace('DestroyAssociations();','AnimGroup_DestroyAssociations(self);')
        if name in ['CopyById','CopyByName']:body=body.replace('GetAnimation('+('id' if name=='CopyById' else 'name')+')','AnimGroup_'+('GetById' if name=='CopyById' else 'GetByName')+'(self,'+('id' if name=='CopyById' else 'name')+')')
        if name=='CreateForModels':body=body.replace('CBaseModelInfo *model','void *model').replace('GetModelFromName(','AnimGroup_FindModel(').replace('model->GetModelName()','AnimGroup_HostModelName(model)').replace('RpClump *clump = (RpClump*)model->CreateInstance();','void *clump = AnimGroup_HostCreateInstance(model);').replace('RpAnimBlendClumpInit(','AnimGroup_HostInitClump(').replace('IsClumpSkinned(','AnimGroup_HostIsSkinned(').replace('RpClumpForAllAtomics(clump, AtomicRemoveAnimFromSkinCB, nil);','AnimGroup_HostRemoveSkinAnimation(clump);').replace('RpClumpDestroy(','AnimGroup_HostDestroyClump(')
        body=body.replace('CAnimManager::UncompressAnimation(','AnimAssociation_HostUncompressAnimation(').replace('CAnimManager::GetAnimationBlock(','AnimGroup_HostBlock(')
        body=body.replace('CAnimManager::GetAnimation(animBlock->firstIndex + i)','AnimGroup_HostAnimationId(animBlock->firstIndex + i)').replace('CAnimManager::GetAnimation(animNames[i], animBlock)','AnimGroup_HostAnimationName(animNames[i], animBlock)')
        for field in ['assocList','numAssociations','firstAnimId','groupId']+([] if name=='CreateForModels' else ['animBlock']):body=re.sub(r'(?<![\w.>])\b'+field+r'\b','self->'+field,body)
        body=body.replace('nil','NULL').replace('debug(','AnimGroup_Debug(')
        source+=result+' AnimGroup_'+name+'('+args+')\n{\n    // Preserve the complete original group policy and ordered ownership operations\n'+body+'}\n'
    return source+lifetime()+'//- rouz edit (ChatGPT)\n'

def anim_group_calls(source):
    # Convert the complete group owner and all diagnosed native overload and lifetime callers
    original=source
    if 'class CAnimBlendAssocGroup\n' in source:return header()
    if 'CAnimBlendAssocGroup::CAnimBlendAssocGroup(void)\n{' in source:return kernel(source)
    source=source.replace('class CAnimBlendAssocGroup;','struct CAnimBlendAssocGroup;')
    if '#define MAX_ANIMBLOCK_NAME 20' in source and 'struct CAnimBlock\n{' in source:
        start=source.index('#define MAX_ANIMBLOCK_NAME 20');end=source.index('\n};',source.index('struct CAnimBlock\n{',start))+3;source=source[:start]+'#include "AnimBlock.h"'+source[end:]
    source=source.replace('std::allocator<CAnimBlendAssocGroup>().destroy(&ms_aAnimAssocGroups[i]);','AnimGroup_Destroy(&ms_aAnimAssocGroups[i]);').replace('std::allocator<CAnimBlendAssocGroup>().construct(&ms_aAnimAssocGroups[i]);','AnimGroup_Init(&ms_aAnimAssocGroups[i]);')
    source=source.replace('CAnimBlendAssocGroup CCutsceneMgr::ms_cutsceneAssociations;','CAnimBlendAssocGroup CCutsceneMgr::ms_cutsceneAssociations=AnimGroup_InitStaticCutscene(&CCutsceneMgr::ms_cutsceneAssociations);')
    code=mask(source)
    for match in reversed(list(re.finditer(r'(\bms_aAnimAssocGroups\[[^\]\n]+\]|\bms_cutsceneAssociations|\bgroup)(\.|->)(DestroyAssociations|GetAnimation|CopyAnimation|CreateAssociations)\(',code))):
        end=closing(code,match.end()-1);args=source[match.end():end-1];values=arguments(args);operation=match[3]
        if operation in ['GetAnimation','CopyAnimation']:operation=('Get' if operation=='GetAnimation' else 'Copy')+('ByName' if values[0] in ['name','animName'] else 'ById')
        elif operation=='CreateAssociations':operation='CreateForModels' if len(values)==1 else 'CreateForClump'
        receiver=source[match.start(1):match.end(1)];receiver='&'+receiver if match[2]=='.' else receiver
        source=source[:match.start()]+'AnimGroup_'+operation+'('+receiver+(',' if args.strip() else '')+args+')'+source[end:]
    if re.search(r'(?m)^CAnimManager::CreateAnimAssocGroups\(',source) and 'AnimGroup_HostModelAt(' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "AnimBlendAssocGroup_backend.h"',1)+native_backend()
    if source==original:return source
    oldLines=set(original.splitlines());lines=[]
    for line in source.splitlines():
        if line not in oldLines and 'AnimGroup_' in line and not line.lstrip().startswith('#') and 'AnimGroup_Host' not in line and 'CCutsceneMgr::ms_cutsceneAssociations=' not in line:lines.append(re.match(r'\s*',line)[0]+'// Use complete C group ownership at the original native call boundary')
        lines.append(line)
    source='\n'.join(lines)+'\n'
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def anim_group_project(source):
    # Preserve native source metadata while selecting C11 for the complete group producer
    source=source.replace('..\\src\\animation\\AnimBlendAssocGroup.cpp','..\\src\\animation\\AnimBlendAssocGroup.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\animation\\AnimBlendAssocGroup\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Retain other metadata and require the complete producer to compile as C11
            body=match[1] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            return '<ClCompile Include="..\\src\\animation\\AnimBlendAssocGroup.c">'+body+'\n    </ClCompile>'
        source=re.sub(pattern,metadata,source)
    anchor='<ClInclude Include="..\\src\\animation\\AnimBlendAssocGroup.h" />'
    for name in ['AnimBlock.h','AnimBlendAssocGroup_backend.h']:
        entry='<ClInclude Include="..\\src\\animation\\'+name+'" />'
        if entry not in source:source=source.replace(anchor,anchor+'\n    '+entry)
    return objects.project(source)



def calls(source):
    # Preserve complete group ownership before the full manager algorithms and native caller conversion
    import anim_manager_migration as manager
    return manager.calls(anim_group_calls(source))


def project(source):
    # Retain group project metadata before the C manager and separate native primitive producer
    import anim_manager_migration as manager
    return manager.project(anim_group_project(source))
