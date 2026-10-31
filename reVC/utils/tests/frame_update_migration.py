"""Project all nine complete animation frame accumulation callbacks into C11."""
import re
from anim_association_migration import original_body
import object_names_migration as objects
RENAMES={'src/animation/FrameUpdate.cpp':'src/animation/FrameUpdate.c'}
OPERATIONS=['NonSkinned','WithVelocityExtractionNonSkinned','With3dVelocityExtractionNonSkinned','Skinned','WithVelocityExtractionSkinned','With3dVelocityExtractionSkinned','Offscreen','NonSkinnedCompressed','SkinnedCompressed']

def header():
    # Keep the original callback data layout and names while exposing every callback and its global through C linkage
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "AnimBlendNode.h"
#include "AnimBlendFrameData.h"
struct CAnimBlendClumpData;
typedef struct AnimBlendFrameUpdateData {
    int foobar;
    CAnimBlendNode *nodes[16];
} AnimBlendFrameUpdateData;
#ifdef __cplusplus
extern "C" {
#endif
extern struct CAnimBlendClumpData *gpAnimBlendClump;
'''+''.join('void FrameUpdateCallBack'+name+'(AnimBlendFrameData *frame,void *arg);\n' for name in OPERATIONS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Borrow actual live SDK and native vector fields without exposing synthetic SDK record layouts to C
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "AnimBlendFrameUpdate.h"
#include "Quaternion.h"
typedef struct FrameUpdateVector2Out {float *x,*y;} FrameUpdateVector2Out;
typedef struct FrameUpdateQuaternionOut {float *x,*y,*z,*w;} FrameUpdateQuaternionOut;
#ifdef __cplusplus
extern "C" {
#endif
void *FrameUpdate_HostFrameMatrix(AnimBlendFrameData *frame);
void FrameUpdate_HostMatrixIdentity(void *matrix);
void FrameUpdate_HostMatrixUpdate(void *matrix);
QuaternionMatrixOut FrameUpdate_HostMatrixRotation(void *matrix);
KeyFrameVectorOut FrameUpdate_HostMatrixPosition(void *matrix);
FrameUpdateQuaternionOut FrameUpdate_HostPoseRotation(void *pose);
KeyFrameVectorOut FrameUpdate_HostPoseTranslation(void *pose);
FrameUpdateVector2Out FrameUpdate_HostVelocity2d(struct CAnimBlendClumpData *owner);
KeyFrameVectorOut FrameUpdate_HostVelocity3d(struct CAnimBlendClumpData *owner);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def native():
    # Keep only SDK matrix operations and borrowing of the actual existing native field addresses
    return '''//+ rouz edit (ChatGPT)
#include "common.h"
#include "AnimBlendFrameData_native.h"
#include "AnimBlendClumpData.h"
#include "Quaternion_native.h"
#include "FrameUpdate_backend.h"
void *FrameUpdate_HostFrameMatrix(AnimBlendFrameData *frame)
{
    // Recover the same live SDK matrix at the original callback lookup boundary
    return RwFrameGetMatrix(AnimBlendFrame_GetFrame(frame));
}
void FrameUpdate_HostMatrixIdentity(void *matrix)
{
    // Invoke the original native identity operation on the actual SDK matrix
    RwMatrixSetIdentity(static_cast<RwMatrix*>(matrix));
}
void FrameUpdate_HostMatrixUpdate(void *matrix)
{
    // Preserve the final SDK matrix update at the original callback position
    RwMatrixUpdate(static_cast<RwMatrix*>(matrix));
}
QuaternionMatrixOut FrameUpdate_HostMatrixRotation(void *matrix)
{
    // Borrow actual SDK rotation components for the existing complete C quaternion operation
    return Quaternion_MatrixOut(*static_cast<RwMatrix*>(matrix));
}
KeyFrameVectorOut FrameUpdate_HostMatrixPosition(void *matrix)
{
    // Return references to the actual live matrix position without making a temporary view
    RwMatrix *value=static_cast<RwMatrix*>(matrix);KeyFrameVectorOut out={&value->pos.x,&value->pos.y,&value->pos.z};return out;
}
FrameUpdateQuaternionOut FrameUpdate_HostPoseRotation(void *pose)
{
    // Borrow the actual SDK pose quaternion fields while leaving all blending policy in C
    RpHAnimStdInterpFrame *value=static_cast<RpHAnimStdInterpFrame*>(pose);FrameUpdateQuaternionOut out={&value->q.imag.x,&value->q.imag.y,&value->q.imag.z,&value->q.real};return out;
}
KeyFrameVectorOut FrameUpdate_HostPoseTranslation(void *pose)
{
    // Borrow actual live SDK pose translation fields without copying their values
    RpHAnimStdInterpFrame *value=static_cast<RpHAnimStdInterpFrame*>(pose);KeyFrameVectorOut out={&value->t.x,&value->t.y,&value->t.z};return out;
}
FrameUpdateVector2Out FrameUpdate_HostVelocity2d(CAnimBlendClumpData *owner)
{
    // Borrow the real native velocity vector fields through the current owner pointer
    FrameUpdateVector2Out out={&owner->velocity2d->x,&owner->velocity2d->y};return out;
}
KeyFrameVectorOut FrameUpdate_HostVelocity3d(CAnimBlendClumpData *owner)
{
    // Preserve the original union member and live native velocity storage
    KeyFrameVectorOut out={&owner->velocity3d->x,&owner->velocity3d->y,&owner->velocity3d->z};return out;
}
//- rouz edit (ChatGPT)
'''

def helpers():
    # Replace native vector constructors and operators by equivalent ordered C component operations
    return '''
static KeyFrameVectorOut FrameUpdate_VectorOut(KeyFrameVector3 *value)
{
    // Borrow actual local C vector components for the existing interpolation functions
    KeyFrameVectorOut out={&value->x,&value->y,&value->z};return out;
}
static void FrameUpdate_AddOut(KeyFrameVectorOut destination,const KeyFrameVector3 *value)
{
    // Retain original native vector addition order and live component reads
    *destination.x+=value->x;*destination.y+=value->y;*destination.z+=value->z;
}
static void FrameUpdate_Add(KeyFrameVector3 *destination,const KeyFrameVector3 *value)
{
    // Apply complete local vector addition without C++ operators
    FrameUpdate_AddOut(FrameUpdate_VectorOut(destination),value);
}
static KeyFrameVector3 FrameUpdate_Subtract(const KeyFrameVector3 *left,const KeyFrameVector3 *right)
{
    // Preserve each original vector subtraction before selecting or storing components
    KeyFrameVector3 value={left->x-right->x,left->y-right->y,left->z-right->z};return value;
}
static void FrameUpdate_Store(KeyFrameVectorOut destination,KeyFrameVector3 value)
{
    // Assign all actual destination components in the original native vector field order
    *destination.x=value.x;*destination.y=value.y;*destination.z=value.z;
}
'''

def kernel(original):
    # Preserve the complete callback algorithms and native field read/write ordering in a C-only producer
    source='''//+ rouz edit (ChatGPT)
#include "config.h"
#include "AnimBlendFrameUpdate.h"
#include "FrameUpdate_backend.h"
#include "AnimBlendClumpData.h"
#include "AnimBlendAssociation.h"
CAnimBlendClumpData *gpAnimBlendClump;
'''+helpers()
    for operation in OPERATIONS:
        # Select entire exact callback definitions rather than top-of-file prototypes or internal calls
        signature='FrameUpdateCallBack'+operation+'(AnimBlendFrameData *frame, void *arg)'
        body=original_body(original,signature)
        body=body.replace('CVector vec, pos(0.0f, 0.0f, 0.0f);','KeyFrameVector3 vec, pos={0.0f, 0.0f, 0.0f};')
        for name in ['trans','cur','end']:
            # Keep local zero initialization at each original constructor boundary
            body=body.replace('CVector '+name+'(0.0f, 0.0f, 0.0f);','KeyFrameVector3 '+name+'={0.0f, 0.0f, 0.0f};')
        body=body.replace('RwMatrix *mat = RwFrameGetMatrix(AnimBlendFrame_GetFrame(frame));','void *mat = FrameUpdate_HostFrameMatrix(frame);')
        body=body.replace('RpHAnimStdInterpFrame *xform = AnimBlendFrame_GetHAnim(frame);','void *xform = frame->hanimFrame;')
        body=body.replace('KeyFrame_VectorOut(vec)','FrameUpdate_VectorOut(&vec)')
        body=re.sub(r'\b(pos|trans|cur|end) \+= vec;',r'FrameUpdate_Add(&\1,&vec);',body)
        body=body.replace('*gpAnimBlendClump->velocity3d = trans - cur;','FrameUpdate_Store(FrameUpdate_HostVelocity3d(gpAnimBlendClump),FrameUpdate_Subtract(&trans,&cur));')
        body=body.replace('*gpAnimBlendClump->velocity3d += end;','FrameUpdate_AddOut(FrameUpdate_HostVelocity3d(gpAnimBlendClump),&end);')
        body=body.replace('(pos - trans).','FrameUpdate_Subtract(&pos,&trans).')
        body=body.replace('RwMatrixSetIdentity(mat)','FrameUpdate_HostMatrixIdentity(mat)').replace('RwMatrixUpdate(mat)','FrameUpdate_HostMatrixUpdate(mat)').replace('Quaternion_MatrixOut(*mat)','FrameUpdate_HostMatrixRotation(mat)')
        body=re.sub(r'mat->pos\.([xyz])',r'(*FrameUpdate_HostMatrixPosition(mat).\1)',body)
        body=re.sub(r'xform->t\.([xyz])',r'(*FrameUpdate_HostPoseTranslation(xform).\1)',body)
        body=re.sub(r'xform->q\.imag\.([xyz])',r'(*FrameUpdate_HostPoseRotation(xform).\1)',body).replace('xform->q.real','(*FrameUpdate_HostPoseRotation(xform).w)')
        body=re.sub(r'gpAnimBlendClump->velocity2d->([xy])',r'(*FrameUpdate_HostVelocity2d(gpAnimBlendClump).\1)',body)
        source+='void '+signature+'\n{\n    // Preserve the full original frame accumulation policy and live field access order\n'+body+'}\n'
    return source+'//- rouz edit (ChatGPT)\n'

def frame_update_calls(source):
    # Move complete callback ownership and its data record while leaving unrelated SDK functions native
    if 'FrameUpdateCallBackWith3dVelocityExtractionNonSkinned(AnimBlendFrameData *frame, void *arg)\n{' in source and 'CVector vec' in source:return kernel(source)
    if 'struct AnimBlendFrameUpdateData\n{' in source and 'RPANIMBLENDCLUMPDATA' in source:
        # Share the original callback layout and C linkage with all remaining native consumers
        start=source.index('struct AnimBlendFrameUpdateData\n{');end=source.index('\n};',start)+3;source=source[:start]+'#include "AnimBlendFrameUpdate.h"'+source[end:]
        source=re.sub(r'(?m)^extern CAnimBlendClumpData \*gpAnimBlendClump;\n|^void FrameUpdateCallBack\w+\(AnimBlendFrameData \*frame, void \*arg\);\n','',source)
        return source
    return source

def frame_update_project(source):
    # Select the C callback owner and its separate SDK primitive unit without changing other source metadata
    source=source.replace('src\\animation\\FrameUpdate.cpp','src\\animation\\FrameUpdate.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\animation\\FrameUpdate\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Retain existing settings while explicitly compiling the full callback producer as C11
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';native=match[1][:-len('FrameUpdate.c')]+'FrameUpdate_native.cpp'
            if '<ClCompile Include="'+native+'"' not in source:result+='\n    <ClCompile Include="'+native+'" />'
            return result
        source=re.sub(pattern,metadata,source)
    anchor=re.search(r'<ClInclude Include="([^"]*src\\animation\\RpAnimBlend\.h)"\s*/>',source)
    if anchor:
        # Expose the C dispatch data and SDK field reference contracts beside their original native API header
        prefix=anchor[1][:-len('RpAnimBlend.h')];entries=''
        for name in ['AnimBlendFrameUpdate.h','FrameUpdate_backend.h']:
            entry='<ClInclude Include="'+prefix+name+'" />'
            if entry not in source:entries+='\n    '+entry
        source=source[:anchor.end()]+entries+source[anchor.end():]
    return objects.project(source)



def calls(source):
    # Preserve complete C frame callbacks before converting the complete animation plugin owner
    import anim_plugin_migration as plugin
    return plugin.calls(frame_update_calls(source))


def project(source):
    # Retain C callback metadata before selecting the C plugin and native SDK primitive unit
    import anim_plugin_migration as plugin
    return plugin.project(frame_update_project(source))
