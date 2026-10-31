"""Move the complete blend-node record, interpolation and keyframe advancement policy into C11."""
import re
from rect_migration import mask,closing,arguments
from doors_migration import method
import object_names_migration as objects
RENAMES={'src/animation/AnimBlendNode.cpp':'src/animation/AnimBlendNode.c'}
DECLARATIONS=[('void','Init','CAnimBlendNode *self'),('bool','Update','CAnimBlendNode *self,KeyFrameVectorOut trans,CQuaternion *rot,float weight'),('bool','UpdateCompressed','CAnimBlendNode *self,KeyFrameVectorOut trans,CQuaternion *rot,float weight'),('bool','NextKeyFrame','CAnimBlendNode *self'),('bool','NextKeyFrameCompressed','CAnimBlendNode *self'),('bool','FindKeyFrame','CAnimBlendNode *self,float t'),('bool','SetupKeyFrameCompressed','CAnimBlendNode *self'),('void','CalcDeltas','CAnimBlendNode *self'),('void','CalcDeltasCompressed','CAnimBlendNode *self'),('void','GetCurrentTranslation','CAnimBlendNode *self,KeyFrameVectorOut trans,float weight'),('void','GetCurrentTranslationCompressed','CAnimBlendNode *self,KeyFrameVectorOut trans,float weight'),('void','GetEndTranslation','CAnimBlendNode *self,KeyFrameVectorOut trans,float weight'),('void','GetEndTranslationCompressed','CAnimBlendNode *self,KeyFrameVectorOut trans,float weight')]
OPERATIONS=[name for result,name,args in DECLARATIONS]

def header():
    # Keep the original node fields and native owner pointer types without native record definitions
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "KeyFrames.h"
#ifdef __cplusplus
#include "AnimBlendSequence.h"
class CAnimBlendAssociation;
#else
typedef struct CAnimBlendSequence CAnimBlendSequence;
typedef struct CAnimBlendAssociation CAnimBlendAssociation;
#endif
typedef struct CAnimBlendNode {
    float theta,invSin;
    int32_t frameA,frameB;
    float remainingTime;
    CAnimBlendSequence *sequence;
    CAnimBlendAssociation *association;
} CAnimBlendNode;
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' AnimBlendNode_'+name+'('+args+');\n' for result,name,args in DECLARATIONS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def flags(original):
    # Share the original native association flag constants with the C policy
    start=original.index('enum {');end=original.index('\n};',start)+3
    return '//+ rouz edit (ChatGPT)\n#pragma once\n'+original[start:end]+'\n//- rouz edit (ChatGPT)\n'

def backend_header():
    # Borrow owner fields lazily through two C interfaces without moving native object layouts
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "AnimBlendNode.h"
typedef struct AnimBlendNodeSequenceFields {
    const int32_t *type,*numFrames;
    void *const *frames;
    void *const *packedFrames;
} AnimBlendNodeSequenceFields;
typedef struct AnimBlendNodeAssociationFields {
    const int16_t *flags;
    const float *timeStep,*blendAmount;
} AnimBlendNodeAssociationFields;
#ifdef __cplusplus
extern "C" {
#endif
AnimBlendNodeSequenceFields AnimBlendNode_BackendSequence(CAnimBlendSequence *value);
AnimBlendNodeAssociationFields AnimBlendNode_BackendAssociation(CAnimBlendAssociation *value);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def sequence_backend():
    # Publish actual native sequence field addresses without caching values or executing animation policy
    return '''
AnimBlendNodeSequenceFields AnimBlendNode_BackendSequence(CAnimBlendSequence *value)
{
    // Borrow actual sequence fields without reading unused storage or frame counts
    const AnimBlendNodeSequenceFields result={&value->type,&value->numFrames,&value->keyFrames,&value->keyFramesCompressed};return result;
}
'''

def association_backend():
    # Publish actual association fields while retaining lazy reads of partial initialization states
    return '''
AnimBlendNodeAssociationFields AnimBlendNode_BackendAssociation(CAnimBlendAssociation *value)
{
    // Borrow actual association fields without reading unused blend or timestep values
    const AnimBlendNodeAssociationFields result={&value->flags,&value->timeStep,&value->blendAmount};return result;
}
'''

def kernel(original):
    # Keep complete native vector temporaries and all animation field access order in C
    source='''//+ rouz edit (ChatGPT)
#include "AnimBlendNode.h"
#include "AnimBlendNode_backend.h"
#include "AnimBlendAssociation_flags.h"
#include <math.h>
#include <stddef.h>
static int32_t Node_Type(CAnimBlendNode *self)
{
    // Read sequence flags only at the original flag query
    const AnimBlendNodeSequenceFields fields=AnimBlendNode_BackendSequence(self->sequence);return *fields.type;
}
static int32_t Node_Count(CAnimBlendNode *self)
{
    // Read frame counts only at their original policy branches
    const AnimBlendNodeSequenceFields fields=AnimBlendNode_BackendSequence(self->sequence);return *fields.numFrames;
}
static KeyFrame *Node_Frame(CAnimBlendNode *self,int32_t index)
{
    // Preserve the original conditional stride and return actual common frame subobjects
    const AnimBlendNodeSequenceFields fields=AnimBlendNode_BackendSequence(self->sequence);
    return *fields.type&ANIM_KEYFRAME_TRANS?&((KeyFrameTrans*)*fields.frames)[index].base:&((KeyFrame*)*fields.frames)[index];
}
static KeyFrameCompressed *Node_PackedFrame(CAnimBlendNode *self,int32_t index)
{
    // Preserve the original packed conditional stride without reading unpacked storage
    const AnimBlendNodeSequenceFields fields=AnimBlendNode_BackendSequence(self->sequence);
    return *fields.type&ANIM_KEYFRAME_TRANS?&((KeyFrameTransCompressed*)*fields.packedFrames)[index].base:&((KeyFrameCompressed*)*fields.packedFrames)[index];
}
static bool Node_IsRunning(CAnimBlendNode *self)
{
    // Read only the original association running flag
    const AnimBlendNodeAssociationFields fields=AnimBlendNode_BackendAssociation(self->association);return (*fields.flags&ASSOC_RUNNING)!=0;
}
static bool Node_IsRepeating(CAnimBlendNode *self)
{
    // Retain the original association repeat test at loop boundaries
    const AnimBlendNodeAssociationFields fields=AnimBlendNode_BackendAssociation(self->association);return (*fields.flags&ASSOC_REPEAT)!=0;
}
static float Node_TimeStep(CAnimBlendNode *self)
{
    // Read the timestep only after the running condition succeeds
    const AnimBlendNodeAssociationFields fields=AnimBlendNode_BackendAssociation(self->association);return *fields.timeStep;
}
static float Node_BlendAmount(CAnimBlendNode *self,float weight)
{
    // Keep partial associations unweighted and defer the amount read until after the flag test
    const AnimBlendNodeAssociationFields fields=AnimBlendNode_BackendAssociation(self->association);return *fields.flags&ASSOC_PARTIAL?*fields.blendAmount:*fields.blendAmount*weight;
}
static KeyFrameVector3 Node_Add(KeyFrameVector3 a,KeyFrameVector3 b)
{
    // Preserve complete native addition temporaries before publishing their coordinates
    const KeyFrameVector3 result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static KeyFrameVector3 Node_Sub(KeyFrameVector3 a,KeyFrameVector3 b)
{
    // Retain the original vector difference and its component arithmetic
    const KeyFrameVector3 result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static KeyFrameVector3 Node_LeftScale(float scale,KeyFrameVector3 value)
{
    // Keep scalar-first multiplication in the interpolation expression
    const KeyFrameVector3 result={scale*value.x,scale*value.y,scale*value.z};return result;
}
static KeyFrameVector3 Node_RightScale(KeyFrameVector3 value,float scale)
{
    // Keep vector-first multiplication in blend application and endpoint queries
    const KeyFrameVector3 result={value.x*scale,value.y*scale,value.z*scale};return result;
}
static KeyFrameVector3 Node_Read(KeyFrameVectorOut value)
{
    // Read current output coordinates at the original in-place scaling boundary
    const KeyFrameVector3 result={*value.x,*value.y,*value.z};return result;
}
static void Node_Write(KeyFrameVectorOut target,KeyFrameVector3 value)
{
    // Publish each complete vector expression immediately in XYZ order
    *target.x=value.x;*target.y=value.y;*target.z=value.z;
}
static KeyFrameVectorOut Node_Output(KeyFrameVector3 *value)
{
    // Address actual C vector fields for packed translation decoding
    const KeyFrameVectorOut result={&value->x,&value->y,&value->z};return result;
}
'''
    for result,name,args in DECLARATIONS:
        body=method(original,'CAnimBlendNode::'+name);body=body[body.index('{')+1:body.rfind('}')]
        body=body.replace('CAnimBlendSequence::KF_ROT','ANIM_KEYFRAME_ROT').replace('CAnimBlendSequence::KF_TRANS','ANIM_KEYFRAME_TRANS')
        body=body.replace('association->IsRunning()','Node_IsRunning(self)').replace('association->IsRepeating()','Node_IsRepeating(self)').replace('association->timeStep','Node_TimeStep(self)').replace('association->GetBlendAmount(weight)','Node_BlendAmount(self,weight)')
        body=body.replace('sequence->numFrames','Node_Count(self)').replace('sequence->type','Node_Type(self)')
        code=mask(body)
        for match in reversed(list(re.finditer(r'\bsequence->(GetKeyFrameCompressed|GetKeyFrame)\(',code))):
            end=closing(code,match.end()-1);argument=body[match.end():end-1]
            body=body[:match.start()]+('Node_PackedFrame' if match[1]=='GetKeyFrameCompressed' else 'Node_Frame')+'(self,'+argument+')'+body[end:]
        body=body.replace('trans = CVector(0.0f, 0.0f, 0.0f);','Node_Write(trans,(KeyFrameVector3){0.0f,0.0f,0.0f});')
        body=body.replace('CVector transA, transB;','KeyFrameVector3 transA, transB;').replace('CVector pos;','KeyFrameVector3 pos;')
        body=body.replace('KeyFrame_VectorOut(transA)','Node_Output(&transA)').replace('KeyFrame_VectorOut(transB)','Node_Output(&transB)').replace('KeyFrame_VectorOut(pos)','Node_Output(&pos)')
        body=body.replace('trans = KeyFrame_ToNative(((const KeyFrameTrans*)kfB)->translation) + t*(KeyFrame_ToNative(((const KeyFrameTrans*)kfA)->translation) - KeyFrame_ToNative(((const KeyFrameTrans*)kfB)->translation));','Node_Write(trans,Node_Add(((const KeyFrameTrans*)kfB)->translation,Node_LeftScale(t,Node_Sub(((const KeyFrameTrans*)kfA)->translation,((const KeyFrameTrans*)kfB)->translation))));')
        body=body.replace('trans = transB + t*(transA - transB);','Node_Write(trans,Node_Add(transB,Node_LeftScale(t,Node_Sub(transA,transB))));').replace('trans *= blend;','Node_Write(trans,Node_RightScale(Node_Read(trans),blend));')
        body=body.replace('trans = KeyFrame_ToNative(kf->translation) * blend;','Node_Write(trans,Node_RightScale(kf->translation,blend));').replace('trans = pos * blend;','Node_Write(trans,Node_RightScale(pos,blend));')
        body=body.replace('rot = Quaternion_Make(','*rot = Quaternion_Make(').replace('Quaternion_Slerp(&rot,','Quaternion_Slerp(rot,').replace('Quaternion_ScaleAssign(&rot,','Quaternion_ScaleAssign(rot,')
        for operation in OPERATIONS:
            body=re.sub(r'(?<![\w])'+operation+r'\(',lambda match:'AnimBlendNode_'+operation+'(self'+('' if body[match.end():].startswith(')') else ','),body)
        body=re.sub(r'(?<![\w.>])\b(theta|invSin|frameA|frameB|remainingTime|sequence|association)\b',r'self->\1',body)
        body=body.replace('Acos(','acosf(').replace('Sin(','sinf(').replace('nil','NULL')
        source+=result+' AnimBlendNode_'+name+'('+args+')\n{\n    // Retain the complete original animation policy and its lazy field read order\n'+body+'}\n'
    return source+'//- rouz edit (ChatGPT)\n'

def anim_node_calls(source):
    # Remove the complete node class and move all thirteen algorithms into its C producer
    original=source
    if 'class CAnimBlendNode\n' in source:return header()
    if re.search(r'(?m)^CAnimBlendNode::Init\(',source):return kernel(source)
    source=source.replace('class CAnimBlendNode;','struct CAnimBlendNode;')
    if 'typedef struct KeyFrameVector3' in source and 'ANIM_KEYFRAME_ROT' not in source:source=source.replace('#include "Quaternion.h"','#include "Quaternion.h"\nenum {ANIM_KEYFRAME_ROT=1,ANIM_KEYFRAME_TRANS=2};',1)
    source=source.replace('KF_ROT = 1,','KF_ROT = ANIM_KEYFRAME_ROT,').replace('KF_TRANS = 2','KF_TRANS = ANIM_KEYFRAME_TRANS')
    if 'class CAnimBlendAssociation\n' in source and '\n\tASSOC_RUNNING = 1,' in source:
        start=source.index('enum {');end=source.index('\n};',start)+3;source=source[:start]+'#include "AnimBlendAssociation_flags.h"'+source[end:]
    if re.search(r'(?m)^CAnimBlendSequence::CAnimBlendSequence\(',source) and 'AnimBlendNode_BackendSequence(' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "AnimBlendNode_backend.h"',1)+sequence_backend()
    if re.search(r'(?m)^CAnimBlendAssociation::CAnimBlendAssociation\(',source) and 'AnimBlendNode_BackendAssociation(' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "AnimBlendNode_backend.h"',1)+association_backend()
    # Preserve each diagnosed native node receiver and its original argument evaluation boundary
    code=mask(source);pattern=r'(\(\*node\)(?:\[i\])?|nodes\[[^\]\n]+\])(?:->|\.)([A-Za-z0-9_]+)\('
    for match in reversed(list(re.finditer(pattern,code))):
        receiver,operation=source[match.start(1):match.end(1)],match[2]
        if operation not in OPERATIONS:continue
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1]);address=receiver if receiver=='(*node)' else '&'+receiver
        if operation in ['Update','UpdateCompressed']:values=[address,'KeyFrame_VectorOut('+args[0]+')','&'+args[1],args[2]]
        elif 'Translation' in operation:values=[address,'KeyFrame_VectorOut('+args[0]+')',args[1]]
        else:values=[address]+([] if args==[''] else args)
        source=source[:match.start()]+'AnimBlendNode_'+operation+'('+','.join(values)+')'+source[end:]
    if source==original:return source
    # Describe newly added executable calls without changing surrounding control flow
    oldLines=set(original.splitlines());lines=[]
    for line in source.splitlines():
        if line not in oldLines and 'AnimBlendNode_' in line and not line.lstrip().startswith('#') and not any(text in line for text in ['Fields ','BackendSequence(','BackendAssociation(']):lines.append(re.match(r'\s*',line)[0]+'// Use the C blend-node policy at the original native call boundary')
        lines.append(line)
    source='\n'.join(lines)+'\n'
    if 'KeyFrame_VectorOut(' in source and '#include "KeyFrames_native.h"' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "KeyFrames_native.h"',1)
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def anim_node_project(source):
    # Keep the complete renamed producer in C11 and register the field and flag interfaces
    source=source.replace('..\\src\\animation\\AnimBlendNode.cpp','..\\src\\animation\\AnimBlendNode.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\animation\\AnimBlendNode\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve unrelated metadata while requiring the complete node producer to compile as C11
            body=match[1] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            return '<ClCompile Include="..\\src\\animation\\AnimBlendNode.c">'+body+'\n    </ClCompile>'
        source=re.sub(pattern,metadata,source)
    anchor='<ClInclude Include="..\\src\\animation\\AnimBlendNode.h" />'
    for name in ['AnimBlendNode_backend.h','AnimBlendAssociation_flags.h']:
        entry='<ClInclude Include="..\\src\\animation\\'+name+'" />'
        if entry not in source:source=source.replace(anchor,anchor+'\n    '+entry)
    return objects.project(source)



def calls(source):
    # Preserve complete blend-node ownership before C animation links and clump lifetime
    import anim_clump_migration as clumps
    return clumps.calls(anim_node_calls(source))


def project(source):
    # Retain node C11 integration before both complete animation owner producers
    import anim_clump_migration as clumps
    return clumps.project(anim_node_project(source))
