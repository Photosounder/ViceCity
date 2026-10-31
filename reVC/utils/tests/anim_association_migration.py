"""Move the complete animation association owner, playback and callback policy into C11."""
import re
from rect_migration import mask,closing,arguments
from doors_migration import method
import object_names_migration as objects
RENAMES={'src/animation/AnimBlendAssociation.cpp':'src/animation/AnimBlendAssociation.c'}
DECLARATIONS=[('void','Init','CAnimBlendAssociation *self'),('void','CopyConstruct','CAnimBlendAssociation *self,CAnimBlendAssociation *other'),('void','Destroy','CAnimBlendAssociation *self'),('void','AllocateAnimBlendNodeArray','CAnimBlendAssociation *self,int32_t n'),('void','FreeAnimBlendNodeArray','CAnimBlendAssociation *self'),('void','BindClump','CAnimBlendAssociation *self,void *clump,CAnimBlendHierarchy *hier'),('void','InitFrom','CAnimBlendAssociation *self,CAnimBlendAssociation *other'),('void','SetBlend','CAnimBlendAssociation *self,float amount,float delta'),('void','SetFinishCallback','CAnimBlendAssociation *self,AnimAssociationCallback cb,void *arg'),('void','SetDeleteCallback','CAnimBlendAssociation *self,AnimAssociationCallback cb,void *arg'),('void','SetCurrentTime','CAnimBlendAssociation *self,float time'),('void','SyncAnimation','CAnimBlendAssociation *self,CAnimBlendAssociation *other'),('void','Start','CAnimBlendAssociation *self,float time'),('void','UpdateTimeStep','CAnimBlendAssociation *self,float timeDelta,float relSpeed'),('bool','UpdateTime','CAnimBlendAssociation *self,float timeDelta,float relSpeed'),('bool','UpdateBlend','CAnimBlendAssociation *self,float timeDelta')]+[('bool',name,'CAnimBlendAssociation *self') for name in ['IsRunning','IsRepeating','IsPartial','IsMovement','HasTranslation','HasXTranslation']]+[('float','GetBlendAmount','CAnimBlendAssociation *self,float weight'),('CAnimBlendNode *','GetNode','CAnimBlendAssociation *self,int32_t i'),('void','SetRun','CAnimBlendAssociation *self'),('float','GetTimeLeft','CAnimBlendAssociation *self'),('float','GetProgress','CAnimBlendAssociation *self'),('CAnimBlendAssociation *','FromLink','CAnimBlendLink *link')]
OPERATIONS=[name for _,name,_ in DECLARATIONS]
INLINE_FLAGS={'IsRunning':'RUNNING','IsRepeating':'REPEAT','IsPartial':'PARTIAL','IsMovement':'MOVEMENT','HasTranslation':'HAS_TRANSLATION','HasXTranslation':'HAS_X_TRANSLATION'}

def header():
    # Store the complete association state without virtual dispatch or C++ lifetime operations
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AnimBlendList.h"
#include "AnimBlendNode.h"
#include "AnimBlendHierarchy.h"
#include "AnimBlendAssociation_flags.h"
enum {ANIM_ASSOC_CB_NONE=0,ANIM_ASSOC_CB_FINISH=1,ANIM_ASSOC_CB_DELETE=2};
typedef struct CAnimBlendAssociation {
    CAnimBlendLink link;
    int16_t numNodes,groupId;
    CAnimBlendNode *nodes;
    CAnimBlendHierarchy *hierarchy;
    float blendAmount,blendDelta,currentTime,speed,timeStep;
    int16_t animId,flags;
    int32_t callbackType;
    void (*callback)(struct CAnimBlendAssociation*,void*);
    void *callbackArg;
} CAnimBlendAssociation;
typedef void (*AnimAssociationCallback)(CAnimBlendAssociation*,void*);
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' AnimAssociation_'+name+'('+args+');\n' for result,name,args in DECLARATIONS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Expose only actual clump plugin lookup and the separate animation manager operation
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "AnimBlendAssociation.h"
#include "AnimBlendClumpData.h"
#ifdef __cplusplus
extern "C" {
#endif
CAnimBlendClumpData *AnimAssociation_HostClumpData(void *clump);
AnimBlendFrameData *AnimAssociation_HostFindFrame(void *clump,const char *name);
AnimBlendFrameData *AnimAssociation_HostFindBone(void *clump,uint32_t tag);
void AnimAssociation_HostUncompressAnimation(CAnimBlendHierarchy *hierarchy);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def sdk_backend():
    # Retain SDK plugin addressing and existing native lookup operations without association policy
    return '''
CAnimBlendClumpData *AnimAssociation_HostClumpData(void *clump)
{
    // Borrow the original live C clump owner from the SDK plugin slot
    return *RPANIMBLENDCLUMPDATA(static_cast<RpClump*>(clump));
}
AnimBlendFrameData *AnimAssociation_HostFindFrame(void *clump,const char *name)
{
    // Retain the original SDK frame name lookup boundary
    return RpAnimBlendClumpFindFrame(static_cast<RpClump*>(clump),name);
}
AnimBlendFrameData *AnimAssociation_HostFindBone(void *clump,uint32_t tag)
{
    // Preserve the original unsigned bone tag argument at native lookup
    return RpAnimBlendClumpFindBone(static_cast<RpClump*>(clump),tag);
}
'''

def manager_backend():
    # Keep manager cache and decompression ownership outside association playback policy
    return '''
void AnimAssociation_HostUncompressAnimation(CAnimBlendHierarchy *hierarchy)
{
    // Invoke the existing manager operation before reading live playback and hierarchy fields again
    CAnimManager::UncompressAnimation(hierarchy);
}
'''

def original_body(source,signature):
    # Select exact overload definitions rather than preceding calls or other constructor overloads
    begin=source.index(signature+'\n');part=source[begin:];opening=part.index('{');depth=1;end=opening+1
    while depth:
        if part[end]=='{':depth+=1
        elif part[end]=='}':depth-=1
        end+=1
    return part[opening+1:end-1]

def kernel(original):
    # Move complete original algorithms into C while preserving live callbacks and self-destruction
    source='//+ rouz edit (ChatGPT)\n#include "config.h"\n#include "AnimBlendAssociation.h"\n#include "AnimBlendAssociation_backend.h"\n#include "AnimBlendClumpData_backend.h"\n#include <stddef.h>\n#include <stdlib.h>\n#define Association_Max(a,b) ((a)>(b)?(a):(b))\n#define Association_Min(a,b) ((a)<(b)?(a):(b))\n'
    fields=['link','numNodes','groupId','nodes','hierarchy','blendAmount','blendDelta','currentTime','speed','timeStep','animId','flags','callbackType','callback','callbackArg']
    for result,name,args in DECLARATIONS:
        if name in INLINE_FLAGS:body='return !!(self->flags&ASSOC_'+INLINE_FLAGS[name]+');'
        elif name=='GetBlendAmount':body='return AnimAssociation_IsPartial(self)?self->blendAmount:self->blendAmount*weight;'
        elif name=='GetNode':body='return &self->nodes[i];'
        elif name=='SetRun':body='self->flags|=ASSOC_RUNNING;'
        elif name=='GetTimeLeft':body='return self->hierarchy->totalLength-self->currentTime;'
        elif name=='GetProgress':body='return self->currentTime/self->hierarchy->totalLength;'
        elif name=='FromLink':body='return (CAnimBlendAssociation*)((uint8_t*)link - offsetof(CAnimBlendAssociation,link));'
        else:
            signatures={'Init':'CAnimBlendAssociation::CAnimBlendAssociation(void)','CopyConstruct':'CAnimBlendAssociation::CAnimBlendAssociation(CAnimBlendAssociation &other)','Destroy':'CAnimBlendAssociation::~CAnimBlendAssociation(void)','BindClump':'CAnimBlendAssociation::Init(RpClump *clump, CAnimBlendHierarchy *hier)','InitFrom':'CAnimBlendAssociation::Init(CAnimBlendAssociation &assoc)'}
            if name in signatures:signature=signatures[name]
            else:signature=re.search(r'(?m)^CAnimBlendAssociation::'+name+r'\([^\n]*\)',original)[0]
            body=original_body(original,signature).replace('this->~CAnimBlendAssociation();','AnimAssociation_Destroy(self);').replace('Init(other);','AnimAssociation_InitFrom(self,other);')
            if name=='InitFrom':body=body.replace('assoc.','other->')
            body=body.replace('*RPANIMBLENDCLUMPDATA(clump)','AnimAssociation_HostClumpData(clump)').replace('RpAnimBlendClumpFindFrame(','AnimAssociation_HostFindFrame(').replace('RpAnimBlendClumpFindBone(','AnimAssociation_HostFindBone(').replace('CAnimManager::UncompressAnimation(','AnimAssociation_HostUncompressAnimation(')
            for operation in ['AllocateAnimBlendNodeArray','FreeAnimBlendNodeArray','SetCurrentTime','IsRepeating','IsRunning']:
                body=re.sub(r'(?<![\w])'+operation+r'\(',lambda m:'AnimAssociation_'+operation+'(self'+('' if body[m.end():].startswith(')') else ','),body)
            for field in fields:body=re.sub(r'(?<![\w.>])\b'+field+r'\b','self->'+field,body)
            body=re.sub(r'\bthis\b','self',body).replace('nil','NULL').replace('RwMallocAlign(','AnimBlendClump_BackendAllocate(').replace('RwFreeAlign(','AnimBlendClump_BackendFree(').replace('Max(','Association_Max(').replace('Min(','Association_Min(')
            for flag in ['NONE','FINISH','DELETE']:body=re.sub(r'\bCB_'+flag+r'\b','ANIM_ASSOC_CB_'+flag,body)
        source+=result+' AnimAssociation_'+name+'('+args+')\n{\n    // Preserve the complete original association policy and ordered live field accesses\n'+body.strip('\n')+'\n}\n'
    return source+'//- rouz edit (ChatGPT)\n'

def anim_association_calls(source):
    # Replace the complete owner before changing native calls and explicit lifetime operations
    original=source
    if 'class CAnimBlendAssociation\n' in source:return header()
    if 'CAnimBlendAssociation::CAnimBlendAssociation(void)\n{' in source:return kernel(source)
    source=source.replace('class CAnimBlendAssociation;','struct CAnimBlendAssociation;')
    for name in ['NONE','FINISH','DELETE']:source=source.replace('CAnimBlendAssociation::CB_'+name,'ANIM_ASSOC_CB_'+name)
    source=source.replace('CAnimBlendAssociation::FromLink(','AnimAssociation_FromLink(')
    source=re.sub(r'\b(\w+)->~CAnimBlendAssociation\(\)',lambda match:'AnimAssociation_Destroy('+match[1]+')',source)
    source=source.replace('std::allocator<CAnimBlendAssociation>().destroy(&assocList[i]);','AnimAssociation_Destroy(&assocList[i]);').replace('std::allocator<CAnimBlendAssociation>().construct(&assocList[i]);','AnimAssociation_Init(&assocList[i]);').replace('std::allocator<CAnimBlendAssociation>().construct(copy, *anim);','AnimAssociation_CopyConstruct(copy,anim);')
    if 'typedef struct AnimBlendNodeAssociationFields {' in source:return '//+ rouz edit (ChatGPT)\n#pragma once\n#include "AnimBlendNode.h"\n//- rouz edit (ChatGPT)\n'
    if 'static bool Node_IsRunning(' in source and 'AnimBlendNode_BackendAssociation(' in source:
        source=source.replace('#include "AnimBlendNode_backend.h"','#include "AnimBlendAssociation.h"')
        values={'Node_IsRunning':'return AnimAssociation_IsRunning(self->association);','Node_IsRepeating':'return AnimAssociation_IsRepeating(self->association);','Node_TimeStep':'return self->association->timeStep;','Node_BlendAmount':'return AnimAssociation_GetBlendAmount(self->association,weight);'}
        for name,value in values.items():
            match=re.search(r'(?m)^static (?:bool|float) '+name+r'\(',source);assert match,name;old=method(source[match.start():],name);source=source.replace(old,old[:old.index('{')]+'{\n    // Read the complete C association at the original lazy query boundary\n    '+value+'\n}')
    # Adapt both actual direct-return native association expressions before named receivers
    for function in ['CAnimManager::BlendAnimation','CAnimManager::AddAnimation']:
        code=mask(source)
        for match in reversed(list(re.finditer(re.escape(function)+r'\(',code))):
            end=closing(code,match.end()-1);suffix='->SetFinishCallback('
            if source[end:end+len(suffix)]==suffix:
                finish=closing(code,end+len(suffix)-1);source=source[:match.start()]+'AnimAssociation_SetFinishCallback('+source[match.start():end]+','+source[end+len(suffix):finish-1]+')'+source[finish:]
    code=mask(source);rows=[(m.start(),m[1],m[2]) for m in re.finditer(r'\b(C\w+)\s*[*&]\s*(\w+)',code)]
    for declaration in re.finditer(r'\b(C\w+)\s*\*\s*\w+(?:\s*,\s*\*\s*\w+)+\s*;',code):
        for variable in re.finditer(r'\*\s*(\w+)',declaration[0]):rows.append((declaration.start()+variable.start(),declaration[1],variable[1]))
    rows.sort()
    pattern=r'(\(\*node\)->association|(?:\bped->)?m_pVehicleAnim|assocList\[[^\]\n]+\]|\b\w+)(->|\.)([A-Za-z0-9_]+)\('
    for match in reversed(list(re.finditer(pattern,code))):
        receiver,operation=source[match.start(1):match.end(1)],match[3]
        if operation not in OPERATIONS+['Init']:continue
        known=receiver in ['(*node)->association','m_pVehicleAnim','ped->m_pVehicleAnim'] or receiver.startswith('assocList[')
        record=next((kind for pos,kind,var in reversed(rows) if var==receiver and pos<match.start()),None)
        if not known and record!='CAnimBlendAssociation':continue
        end=closing(code,match.end()-1);args=source[match.end():end-1];receiver='&'+receiver if match[2]=='.' else receiver
        if operation=='Init':
            values=arguments(args);operation='BindClump' if len(values)==2 else 'InitFrom'
            if operation=='InitFrom':args='&('+args+')'
        source=source[:match.start()]+'AnimAssociation_'+operation+'('+receiver+(',' if args.strip() else '')+args+')'+source[end:]
    if re.search(r'(?m)^RpAnimBlendAllocateData\(RpClump \*clump\)\n\{',source) and 'AnimAssociation_HostClumpData(' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "AnimBlendAssociation_backend.h"',1)+sdk_backend()
    if re.search(r'(?m)^CAnimManager::UncompressAnimation\(',source) and 'AnimAssociation_HostUncompressAnimation(' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "AnimBlendAssociation_backend.h"',1)+manager_backend()
    if source==original:return source
    oldLines=set(original.splitlines());lines=[]
    for line in source.splitlines():
        if line not in oldLines and not line.lstrip().startswith('#') and ('AnimAssociation_' in line or 'ANIM_ASSOC_CB_' in line) and 'AnimAssociation_Host' not in line:lines.append(re.match(r'\s*',line)[0]+'// Use the complete C association at the original native operation boundary')
        lines.append(line)
    source='\n'.join(lines)+'\n'
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def anim_association_project(source):
    # Compile the complete renamed association producer as C11 and register only its external owner boundaries
    source=source.replace('..\\src\\animation\\AnimBlendAssociation.cpp','..\\src\\animation\\AnimBlendAssociation.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\animation\\AnimBlendAssociation\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve other source metadata while explicitly selecting C11
            body=match[1] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            return '<ClCompile Include="..\\src\\animation\\AnimBlendAssociation.c">'+body+'\n    </ClCompile>'
        source=re.sub(pattern,metadata,source)
    anchor='<ClInclude Include="..\\src\\animation\\AnimBlendAssociation.h" />';entry='<ClInclude Include="..\\src\\animation\\AnimBlendAssociation_backend.h" />'
    if entry not in source:source=source.replace(anchor,anchor+'\n    '+entry)
    return objects.project(source)



def calls(source):
    # Preserve complete association ownership before group ownership and selection
    import anim_group_migration as clumps
    return clumps.calls(anim_association_calls(source))


def project(source):
    # Retain association integration before the renamed C11 group producer
    import anim_group_migration as clumps
    return clumps.project(anim_association_project(source))
