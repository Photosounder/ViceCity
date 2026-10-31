"""Move complete animation sequence ownership and keyframe policy from C++ into C11."""
import re
from rect_migration import mask,closing,arguments
from doors_migration import method
import object_names_migration as objects
RENAMES={'src/animation/AnimBlendSequence.cpp':'src/animation/AnimBlendSequence.c'}
DECLARATIONS=[('void','Init','CAnimBlendSequence *self'),('void','Destroy','CAnimBlendSequence *self'),('void','SetName','CAnimBlendSequence *self,char *name'),('void','SetNumFrames','CAnimBlendSequence *self,int32_t numFrames,bool translation,bool compressed'),('void','RemoveQuaternionFlips','CAnimBlendSequence *self'),('KeyFrame *','GetKeyFrame','CAnimBlendSequence *self,int32_t n'),('KeyFrameCompressed *','GetKeyFrameCompressed','CAnimBlendSequence *self,int32_t n'),('bool','HasTranslation','CAnimBlendSequence *self'),('void','Uncompress','CAnimBlendSequence *self'),('void','CompressKeyframes','CAnimBlendSequence *self'),('void','RemoveUncompressedData','CAnimBlendSequence *self'),('bool','MoveMemory','CAnimBlendSequence *self'),('void','SetBoneTag','CAnimBlendSequence *self,int32_t tag')]
OPERATIONS=[name for _,name,_ in DECLARATIONS]

def header():
    # Store only the actual sequence fields after removing unused virtual dispatch
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "KeyFrames.h"
typedef struct CAnimBlendSequence {
    int32_t type;
    char name[24];
    int32_t numFrames;
    int16_t boneTag;
    void *keyFrames,*keyFramesCompressed;
} CAnimBlendSequence;
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' AnimSequence_'+name+'('+args+');\n' for result,name,args in DECLARATIONS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Expose only native allocation and optional heap primitives
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
void *AnimSequence_BackendAllocate(size_t bytes);
void AnimSequence_BackendFree(void *memory);
void AnimSequence_BackendPushAnimation(void);
void AnimSequence_BackendPopMemory(void);
void AnimSequence_BackendRegister(void **memory);
void *AnimSequence_BackendMoveMemory(void *memory);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend():
    # Keep native allocation, heap identification and relocation at their actual SDK boundaries
    return '''
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
void AnimSequence_BackendPushAnimation(void)
{
    // Preserve the original animation memory category boundary
    PUSH_MEMID(MEMID_ANIMATION);
}
void AnimSequence_BackendPopMemory(void)
{
    // Restore the original heap memory category after keyframe work
    POP_MEMID();
}
void AnimSequence_BackendRegister(void **memory)
{
    // Register the address of the owner's actual keyframe pointer
    REGISTER_MEMPTR(memory);
}
#ifdef USE_CUSTOM_ALLOCATOR
void *AnimSequence_BackendMoveMemory(void *memory)
{
    // Forward only the native heap relocation primitive
    return gMainHeap.MoveMemory(memory);
}
#endif
'''

def kernel(original):
    # Preserve each complete sequence method and its exact floating-point and storage policy
    source='//+ rouz edit (ChatGPT)\n#include "config.h"\n#include "AnimBlendSequence.h"\n#include "AnimBlendSequence_backend.h"\n#include <stddef.h>\n#include <string.h>\n'
    for result,name,args in DECLARATIONS:
        if name=='GetKeyFrame':body='return self->type&ANIM_KEYFRAME_TRANS?&((KeyFrameTrans*)self->keyFrames)[n].base:&((KeyFrame*)self->keyFrames)[n];'
        elif name=='GetKeyFrameCompressed':body='return self->type&ANIM_KEYFRAME_TRANS?&((KeyFrameTransCompressed*)self->keyFramesCompressed)[n].base:&((KeyFrameCompressed*)self->keyFramesCompressed)[n];'
        elif name=='HasTranslation':body='return !!(self->type&ANIM_KEYFRAME_TRANS);'
        elif name=='SetBoneTag':body='self->boneTag=tag;'
        else:
            old='CAnimBlendSequence::'+('CAnimBlendSequence' if name=='Init' else '~CAnimBlendSequence' if name=='Destroy' else name)
            text=method(original,old);body=text[text.index('{')+1:text.rfind('}')]
            body=body.replace('this->numFrames','self->numFrames').replace('this->name','self->name')
            # Preserve the SetNumFrames parameter separately from the live owner count
            fields=['type','name','boneTag','keyFrames','keyFramesCompressed']+([] if name in ['SetNumFrames','SetName'] else ['numFrames'])
            if name=='SetName':fields.remove('name')
            for field in fields:body=re.sub(r'(?<![\w.>])\b'+field+r'\b','self->'+field,body)
            for operation in ['GetKeyFrame','CompressKeyframes']:
                body=re.sub(r'(?<!\w)'+operation+r'\(', 'AnimSequence_'+operation+'(self'+('' if operation=='CompressKeyframes' else ','),body)
            body=body.replace('KF_ROT','ANIM_KEYFRAME_ROT').replace('KF_TRANS','ANIM_KEYFRAME_TRANS').replace('nil','NULL')
            body=body.replace('RwMalloc(','AnimSequence_BackendAllocate(').replace('RwFree(','AnimSequence_BackendFree(').replace('PUSH_MEMID(MEMID_ANIMATION);','AnimSequence_BackendPushAnimation();').replace('POP_MEMID();','AnimSequence_BackendPopMemory();').replace('REGISTER_MEMPTR(','AnimSequence_BackendRegister(').replace('gMainHeap.MoveMemory(','AnimSequence_BackendMoveMemory(')
        if name=='MoveMemory':source+='#ifdef USE_CUSTOM_ALLOCATOR\n'
        source+=result+' AnimSequence_'+name+'('+args+')\n{\n    // Retain the complete original sequence policy and its observable operation order\n'+body.strip('\n')+'\n}\n'
        if name=='MoveMemory':source+='#endif\n'
    return source+'//- rouz edit (ChatGPT)\n'

def anim_sequence_calls(source):
    # Remove the complete sequence class before translating its diagnosed native consumers
    original=source
    if 'class CAnimBlendSequence\n' in source:return header()
    if 'CAnimBlendSequence::CAnimBlendSequence(void)\n{' in source:return kernel(source)
    source=source.replace('class CAnimBlendSequence;','struct CAnimBlendSequence;').replace('CAnimBlendSequence::KF_ROT','ANIM_KEYFRAME_ROT').replace('CAnimBlendSequence::KF_TRANS','ANIM_KEYFRAME_TRANS')
    source=source.replace('std::allocator<CAnimBlendSequence>().construct(&hier->sequences[k]);','AnimSequence_Init(&hier->sequences[k]);').replace('std::allocator<CAnimBlendSequence>().destroy(&sequences[i]);','AnimSequence_Destroy(&sequences[i]);')
    # Expose the C record to C nodes and eliminate the obsolete native sequence field interface
    if 'typedef struct CAnimBlendNode {' in source and '#include "AnimBlendSequence.h"\n#ifdef __cplusplus' not in source:
        source=source.replace('#ifdef __cplusplus\n#include "AnimBlendSequence.h"','#include "AnimBlendSequence.h"\n#ifdef __cplusplus').replace('typedef struct CAnimBlendSequence CAnimBlendSequence;\n','')
    if 'typedef struct AnimBlendNodeSequenceFields {' in source:
        start=source.index('typedef struct AnimBlendNodeSequenceFields {');end=source.index('} AnimBlendNodeSequenceFields;',start)+len('} AnimBlendNodeSequenceFields;\n');source=source[:start]+source[end:]
        source=source.replace('AnimBlendNodeSequenceFields AnimBlendNode_BackendSequence(CAnimBlendSequence *value);\n','')
    if 'static int32_t Node_Type(' in source and 'AnimBlendNode_BackendSequence(' in source:
        values={'Node_Type':'return self->sequence->type;','Node_Count':'return self->sequence->numFrames;','Node_Frame':'return AnimSequence_GetKeyFrame(self->sequence,index);','Node_PackedFrame':'return AnimSequence_GetKeyFrameCompressed(self->sequence,index);'}
        for name,value in values.items():
            old=method(source,name);opening=old.index('{');source=source.replace(old,old[:opening]+'{\n    // Read the complete C sequence at the original lazy query boundary\n    '+value+'\n}')
    code=mask(source);pattern=r'(\(\*node\)->sequence|(?:pAnimBlendAssoc->hierarchy->)?sequences\[[^\]\n]+\]|\bseq)(->|\.)([A-Za-z0-9_]+)\('
    for match in reversed(list(re.finditer(pattern,code))):
        if match[3] not in OPERATIONS:continue
        end=closing(code,match.end()-1);receiver=source[match.start(1):match.end(1)];args=source[match.end():end-1];receiver='&'+receiver if match[2]=='.' else receiver
        source=source[:match.start()]+'AnimSequence_'+match[3]+'('+receiver+(',' if args.strip() else '')+args+')'+source[end:]
    if re.search(r'(?m)^RpAnimBlendAllocateData\(RpClump \*clump\)\n\{',source) and 'AnimSequence_BackendAllocate(' not in source:
        source=source.replace('#include "common.h"','#include "common.h"\n#include "AnimBlendSequence_backend.h"\n#include "MemoryHeap.h"',1)+backend()
    if source==original:return source
    oldLines=set(original.splitlines());lines=[]
    for line in source.splitlines():
        if line not in oldLines and not line.lstrip().startswith('#') and ('AnimSequence_' in line or 'ANIM_KEYFRAME_' in line) and not any(text in line for text in ['BackendAllocate(','BackendFree(','BackendPushAnimation(','BackendPopMemory(','BackendRegister(','BackendMoveMemory(']):lines.append(re.match(r'\s*',line)[0]+'// Use the complete C sequence at the original native operation boundary')
        lines.append(line)
    source='\n'.join(lines)+'\n'
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def anim_sequence_project(source):
    # Keep the complete sequence producer in C11 and register its native memory interface
    source=source.replace('..\\src\\animation\\AnimBlendSequence.cpp','..\\src\\animation\\AnimBlendSequence.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\animation\\AnimBlendSequence\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve existing source metadata while requiring the complete sequence producer to compile as C11
            body=match[1] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            return '<ClCompile Include="..\\src\\animation\\AnimBlendSequence.c">'+body+'\n    </ClCompile>'
        source=re.sub(pattern,metadata,source)
    anchor='<ClInclude Include="..\\src\\animation\\AnimBlendSequence.h" />';entry='<ClInclude Include="..\\src\\animation\\AnimBlendSequence_backend.h" />'
    if entry not in source:source=source.replace(anchor,anchor+'\n    '+entry)
    return objects.project(source)



def calls(source):
    # Preserve complete sequence ownership before complete hierarchy ownership
    import anim_hierarchy_migration as clumps
    return clumps.calls(anim_sequence_calls(source))


def project(source):
    # Retain sequence integration before the renamed C11 hierarchy producer
    import anim_hierarchy_migration as clumps
    return clumps.project(anim_sequence_project(source))
