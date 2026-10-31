"""Move all four keyframe records and their six packed conversion methods into C11."""
import re
from rect_migration import mask,closing,arguments
import object_names_migration as objects
from doors_migration import method
RENAMES={}
OPERATIONS=['KeyFrameCompressed_GetRotation','KeyFrameCompressed_SetRotation','KeyFrameCompressed_GetDeltaTime','KeyFrameCompressed_SetTime','KeyFrameTransCompressed_GetTranslation','KeyFrameTransCompressed_SetTranslation']

def header():
    # Preserve real base subobjects and all four original packed/unpacked strides in C
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "Quaternion.h"
typedef struct KeyFrameVector3 {float x,y,z;} KeyFrameVector3;
typedef struct KeyFrameVectorRef {const float *x,*y,*z;} KeyFrameVectorRef;
typedef struct KeyFrameVectorOut {float *x,*y,*z;} KeyFrameVectorOut;
typedef struct KeyFrame {CQuaternion rotation;float deltaTime;} KeyFrame;
typedef struct KeyFrameTrans {KeyFrame base;KeyFrameVector3 translation;} KeyFrameTrans;
typedef struct KeyFrameCompressed {int16_t rot[4];int16_t deltaTime;} KeyFrameCompressed;
typedef struct KeyFrameTransCompressed {KeyFrameCompressed base;int16_t trans[3];} KeyFrameTransCompressed;
#ifdef __cplusplus
extern "C" {
#endif
void KeyFrameCompressed_GetRotation(const KeyFrameCompressed *self,CQuaternion *value);
void KeyFrameCompressed_SetRotation(KeyFrameCompressed *self,const CQuaternion *value);
float KeyFrameCompressed_GetDeltaTime(const KeyFrameCompressed *self);
void KeyFrameCompressed_SetTime(KeyFrameCompressed *self,float value);
void KeyFrameTransCompressed_GetTranslation(const KeyFrameTransCompressed *self,KeyFrameVectorOut value);
void KeyFrameTransCompressed_SetTranslation(KeyFrameTransCompressed *self,KeyFrameVectorRef value);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel():
    # Keep original float scaling, truncation, signed values and component write order
    return '''//+ rouz edit (ChatGPT)
#include "KeyFrames.h"
void KeyFrameCompressed_GetRotation(const KeyFrameCompressed *self,CQuaternion *value)
{
    // Decode signed rotation components with the original reciprocal multiplication
    const float scale=1.0f/4096.0f;
    value->x=self->rot[0]*scale;value->y=self->rot[1]*scale;value->z=self->rot[2]*scale;value->w=self->rot[3]*scale;
}
void KeyFrameCompressed_SetRotation(KeyFrameCompressed *self,const CQuaternion *value)
{
    // Retain the original component multiplication and signed narrowing at each store
    self->rot[0]=value->x*4096.0f;self->rot[1]=value->y*4096.0f;self->rot[2]=value->z*4096.0f;self->rot[3]=value->w*4096.0f;
}
float KeyFrameCompressed_GetDeltaTime(const KeyFrameCompressed *self)
{
    // Keep division by sixty instead of changing its rounding to reciprocal multiplication
    return self->deltaTime/60.0f;
}
void KeyFrameCompressed_SetTime(KeyFrameCompressed *self,float value)
{
    // Retain the original positive half bias before signed truncation
    self->deltaTime=value*60.0f+0.5f;
}
void KeyFrameTransCompressed_GetTranslation(const KeyFrameTransCompressed *self,KeyFrameVectorOut value)
{
    // Decode signed translation coordinates into their original destination fields in order
    const float scale=1.0f/1024.0f;
    *value.x=self->trans[0]*scale;*value.y=self->trans[1]*scale;*value.z=self->trans[2]*scale;
}
void KeyFrameTransCompressed_SetTranslation(KeyFrameTransCompressed *self,KeyFrameVectorRef value)
{
    // Keep live coordinate references and the original ordered signed narrowing stores
    self->trans[0]=*value.x*1024.0f;self->trans[1]=*value.y*1024.0f;self->trans[2]=*value.z*1024.0f;
}
//- rouz edit (ChatGPT)
'''

def native():
    # Adapt only actual native vector fields while keeping all conversion arithmetic in C
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "KeyFrames.h"
#include <string.h>
static inline KeyFrameVectorRef KeyFrame_VectorRef(const CVector &value)
{
    // Borrow actual native coordinates for the complete surrounding packed conversion
    const KeyFrameVectorRef result={&value.x,&value.y,&value.z};return result;
}
static inline KeyFrameVectorOut KeyFrame_VectorOut(CVector &value)
{
    // Preserve actual native output fields without introducing a record view
    const KeyFrameVectorOut result={&value.x,&value.y,&value.z};return result;
}
static inline KeyFrameVector3 KeyFrame_FromNative(const CVector &value)
{
    // Transfer exact XYZ representations before storing the unpacked C translation
    KeyFrameVector3 result;memcpy(&result.x,&value.x,4);memcpy(&result.y,&value.y,4);memcpy(&result.z,&value.z,4);return result;
}
static inline CVector KeyFrame_ToNative(const KeyFrameVector3 &value)
{
    // Create an actual native value for the remaining native vector expressions
    CVector result;memcpy(&result.x,&value.x,4);memcpy(&result.y,&value.y,4);memcpy(&result.z,&value.z,4);return result;
}
//- rouz edit (ChatGPT)
'''

def records(source):
    # Track each diagnosed pointer declaration in source order across native scopes
    return [(match.start(),match[2],match[1]) for match in re.finditer(r'\b(KeyFrameTransCompressed|KeyFrameTrans|KeyFrameCompressed|KeyFrame)\s*\*\s*(\w+)\b',mask(source))]

def kind(name,position,rows):
    # Select the nearest preceding typed pointer declaration for the original consumer
    return next((record for offset,variable,record in reversed(rows) if variable==name and offset<position),None)

def keyframes_calls(source):
    # Keep completed C node and manager producers free of legacy native vector adapters
    if re.search(r'(?m)^void (?:AnimBlendNode_Init|AnimManager_Initialise)\(',source):return source
    # Replace both inheritance relationships with real first-member C base records
    original=source
    # Keep common-only animation queries on real base objects instead of requiring translation containers
    for name,packed in [('Update',False),('GetCurrentTranslation',False),('UpdateCompressed',True),('GetCurrentTranslationCompressed',True)]:
        if not re.search(r'(?m)^CAnimBlendNode::'+name+r'\(',source):continue
        body=method(source,'CAnimBlendNode::'+name);updated=body
        parent='KeyFrameCompressed' if packed else 'KeyFrame';child='KeyFrameTransCompressed' if packed else 'KeyFrameTrans';getter='GetKeyFrameCompressed' if packed else 'GetKeyFrame'
        for variable,index in [('kfA','frameA'),('kfB','frameB')]:
            old=child+' *'+variable+' = ('+child+'*)sequence->'+getter+'('+index+');'
            updated=updated.replace(old,parent+' *'+variable+' = sequence->'+getter+'('+index+');')
            if packed:
                updated=re.sub(re.escape(variable)+r'->GetTranslation\(&([A-Za-z0-9_]+)\)',lambda m:'KeyFrameTransCompressed_GetTranslation((const KeyFrameTransCompressed*)'+variable+',KeyFrame_VectorOut('+m[1]+'))',updated)
            else:updated=updated.replace(variable+'->translation','KeyFrame_ToNative(((const KeyFrameTrans*)'+variable+')->translation)')
        if updated!=body:
            updated=updated.replace(parent+' *kfA =','// Borrow actual common keyframe records before reading optional translation data\n\t\t'+parent+' *kfA =',1)
            source=source.replace(body,updated)
    if 'struct KeyFrame {\n' in source and 'struct KeyFrameTrans : KeyFrame' in source:
        begin=source.index('// TODO: put them somewhere else?');end=source.index('// The sequence of key frames',begin)
        source=source[:begin]+'#include "KeyFrames.h"\n\n'+source[end:]
        source=source.replace('&((KeyFrameTrans*)keyFrames)[n] :','&((KeyFrameTrans*)keyFrames)[n].base :').replace('&((KeyFrameTransCompressed*)keyFramesCompressed)[n] :','&((KeyFrameTransCompressed*)keyFramesCompressed)[n].base :')
    if not any(name in source for name in ['KeyFrameTrans','KeyFrameCompressed']):return source
    # Redirect the complete nested packed-time queries before adapting named record pointers
    code=mask(source)
    for match in reversed(list(re.finditer(r'\b(?:sequence->|sequences\[i\]\.)GetKeyFrameCompressed\(',code))):
        end=closing(code,match.end()-1);suffix='->GetDeltaTime()'
        if source[end:end+len(suffix)]==suffix:source=source[:match.start()]+'KeyFrameCompressed_GetDeltaTime('+source[match.start():end]+')'+source[end+len(suffix):]
    code=mask(source);rows=records(source)
    for match in reversed(list(re.finditer(r'\b(\w+)->(GetRotation|SetRotation|GetDeltaTime|SetTime|GetTranslation|SetTranslation)\(',code))):
        receiver,operation=match[1],match[2];record=kind(receiver,match.start(),rows)
        if record not in ['KeyFrameCompressed','KeyFrameTransCompressed']:continue
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1]);owner='&'+receiver+'->base' if record=='KeyFrameTransCompressed' and operation not in ['GetTranslation','SetTranslation'] else receiver
        name=('KeyFrameTransCompressed_' if operation in ['GetTranslation','SetTranslation'] else 'KeyFrameCompressed_')+operation
        if operation=='GetDeltaTime':value=''
        elif operation=='SetRotation':value=',&'+args[0]
        elif operation=='SetTranslation':value=',KeyFrame_VectorRef('+args[0]+')'
        elif operation=='GetTranslation':value=',KeyFrame_VectorOut('+('*'+args[0] if not args[0].startswith('&') else args[0][1:])+')'
        else:value=','+args[0]
        source=source[:match.start()]+name+'('+owner+value+')'+source[end:]
    # Retain all inherited field accesses through actual embedded base subobjects
    code=mask(source);rows=records(source)
    for match in reversed(list(re.finditer(r'\b(\w+)->(rotation|deltaTime|rot)\b',code))):
        record=kind(match[1],match.start(),rows)
        if record in ['KeyFrameTrans','KeyFrameTransCompressed']:source=source[:match.start()]+match[1]+'->base.'+match[2]+source[match.end():]
    # Capture native translation assignment before adapting complete native vector reads
    pattern=r'\b(\w+)->translation = ([^;\n]+);';rows=records(source)
    for match in reversed(list(re.finditer(pattern,source))):
        if kind(match[1],match.start(),rows)=='KeyFrameTrans' and not match[2].startswith('KeyFrame_FromNative('):source=source[:match.start()]+match[1]+'->translation = KeyFrame_FromNative('+match[2]+');'+source[match.end():]
    code=mask(source);rows=records(source);protected=[]
    for match in re.finditer(r'\bKeyFrame_ToNative\(',code):protected.append((match.start(),closing(code,match.end()-1)))
    for match in reversed(list(re.finditer(r'\b(\w+)->translation\b',code))):
        if kind(match[1],match.start(),rows)!='KeyFrameTrans' or re.match(r'\s*(?:\.[xyz]\b|=)',source[match.end():]) or any(a<=match.start()<b for a,b in protected):continue
        source=source[:match.start()]+'KeyFrame_ToNative('+match[0]+')'+source[match.end():]
    if source==original:return source
    # Describe changed executable statements at their original native call boundaries
    oldLines=set(original.splitlines());lines=[]
    for line in source.splitlines():
        if line not in oldLines and not line.lstrip().startswith('#') and any(text in line for text in ['KeyFrame_','KeyFrameCompressed_','KeyFrameTransCompressed_','->base.','[n].base']):
            lines.append(re.match(r'\s*',line)[0]+'// Use the C keyframe operation and its actual embedded base fields')
        lines.append(line)
    source='\n'.join(lines)+'\n'
    if 'KeyFrame_ToNative(' in source or 'KeyFrame_FromNative(' in source or 'KeyFrame_Vector' in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "KeyFrames_native.h"',1)
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def keyframes_project(source):
    # Register the complete C producer and both public/native headers in legacy projections
    anchor='<ClInclude Include="..\\src\\animation\\AnimBlendSequence.h" />'
    for name in ['KeyFrames.h','KeyFrames_native.h']:
        entry='<ClInclude Include="..\\src\\animation\\'+name+'" />'
        if entry not in source:source=source.replace(anchor,anchor+'\n    '+entry)
    anchor='<ClCompile Include="..\\src\\animation\\AnimBlendSequence.cpp"'
    if 'Microsoft.Cpp.targets' in source and '..\\src\\animation\\KeyFrames.c"' not in source:source=source.replace(anchor,'<ClCompile Include="..\\src\\animation\\KeyFrames.c"><CompileAs>CompileAsC</CompileAs><LanguageStandard_C>stdc11</LanguageStandard_C></ClCompile>\n    '+anchor,1)
    return objects.project(source)



def calls(source):
    # Preserve all four keyframe records before complete C blend-node ownership
    import anim_node_migration as nodes
    return nodes.calls(keyframes_calls(source))


def project(source):
    # Retain keyframe C11 integration before the complete node producer rename
    import anim_node_migration as nodes
    return nodes.project(keyframes_project(source))
