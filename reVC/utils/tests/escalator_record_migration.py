"""Move all escalator record state and algorithms into C11 with separate native matrices."""
import re
from audio_controls_migration import function
import script_paths_migration as script_paths
FIELDS=['m_pos0','m_pos1','m_pos2','m_pos3','m_bIsActive','m_bIsMovingDown','m_stepsCount','m_lowerEnd','m_upperEnd','m_midPoint','m_radius','m_pSteps']
TYPES='''typedef struct EscalatorVector { float x,y,z; } EscalatorVector;
#ifdef __cplusplus
class CObject;
#else
typedef struct CObject CObject;
#endif
typedef struct Escalator {
    EscalatorVector m_pos0,m_pos1,m_pos2,m_pos3;
    bool m_bIsActive,m_bIsMovingDown;
    int32_t m_stepsCount;
    float m_lowerEnd,m_upperEnd;
    EscalatorVector m_midPoint;
    float m_radius;
    CObject *m_pSteps[24];
} Escalator;
'''
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
'''+TYPES+'''#ifdef __cplusplus
extern "C" {
#endif
extern Escalator Escalators_aEscalators[NUM_ESCALATORS];
extern bool deletingEscalator;
void Escalator_AddThisOne(int32_t index,EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool movingDown);
void Escalator_Update(int32_t index);
void Escalator_SwitchOff(int32_t index);
bool Escalator_IsActive(int32_t index);
EscalatorVector Escalator_GetPosition(int32_t index);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('float','StepHeight','void','return CModelInfo::GetColModel(MI_ESCALATORSTEP)->boundingBox.max.z;'),
('EscalatorVector','CameraPosition','void','const CVector &v=TheCamera.GetPosition();EscalatorVector result={v.x,v.y,v.z};return result;'),
('bool','SphereVisible','EscalatorVector center,float radius','return TheCamera.IsSphereVisible(CVector(center.x,center.y,center.z),radius);'),
('CPool *','ObjectPool','void','return CPools::GetObjectPool();'),
('int8_t','CreatedBy','CObject *object','return object->ObjectCreatedBy;'),
('void','Construct','CObject *object','std::allocator<CObject>().construct(object,MI_ESCALATORSTEP,TRUE);'),
('void','Destroy','CObject *object','object->~CObject();'),
('void','WorldAdd','CObject *object','CWorld::Add(object);'),
('void','WorldRemove','CObject *object','CWorld::Remove(object);'),
('EscalatorVector','Position','CObject *object','const CVector &v=object->GetPosition();EscalatorVector result={v.x,v.y,v.z};return result;'),
('void','SetPosition','CObject *object,EscalatorVector value','object->SetPosition(CVector(value.x,value.y,value.z));'),
('void','SetCreatedBy','CObject *object,int8_t value','object->ObjectCreatedBy=value;'),
('void','AssignMatrix','CObject *object,int32_t index','object->GetMatrix()=EscalatorMatrices[index];'),
('void','SetMoveSpeed','CObject *object,EscalatorVector value','object->m_vecMoveSpeed=CVector(value.x,value.y,value.z);'),
('void','UpdateRW','CObject *object','object->GetMatrix().UpdateRW();'),
('void','UpdateFrame','CObject *object','object->UpdateRwFrame();')]
for axis in ['Up','Forward','Right','Position']:READERS.append(('void','Matrix'+axis,'int32_t index,EscalatorVector value','EscalatorMatrices[index].Get'+axis+'()=CVector(value.x,value.y,value.z);'))
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "Escalator.h"
#include "Pool.h"
enum { ESCALATOR_TEMP_OBJECT=3,ESCALATOR_CONTROLLED_SUB_OBJECT=5 };
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' Escalator_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='//+ rouz edit (ChatGPT)\n#include "Escalator_backend.h"\nstatic_assert(ESCALATOR_TEMP_OBJECT==TEMP_OBJECT && ESCALATOR_CONTROLLED_SUB_OBJECT==CONTROLLED_SUB_OBJECT,"Original object creation tags");\n'+''.join(result+' Escalator_Backend'+name+'('+args+')\n{\n    // Preserve the actual typed object or matrix operation at its original call point\n    '+body+'\n}\n' for result,name,args,body in READERS)+'//- rouz edit (ChatGPT)\n'
HELPERS='''static EscalatorVector Escalator_Subtract(EscalatorVector a,EscalatorVector b)
{
    // Keep each original component subtraction before vector magnitude or speed calculation
    EscalatorVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static EscalatorVector Escalator_Add(EscalatorVector a,EscalatorVector b)
{
    // Preserve the original left-associated component additions
    EscalatorVector result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static EscalatorVector Escalator_Divide(EscalatorVector a,float divisor)
{
    // Divide each original component directly instead of multiplying by a reciprocal
    EscalatorVector result={a.x/divisor,a.y/divisor,a.z/divisor};return result;
}
static EscalatorVector Escalator_LeftScale(float factor,EscalatorVector a)
{
    // Retain scalar-left multiplication in the original interpolation expressions
    EscalatorVector result={factor*a.x,factor*a.y,factor*a.z};return result;
}
static float Escalator_Magnitude(EscalatorVector a)
{
    // Preserve the exact original square-root argument and component-product order
    return sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
}
static void Escalator_Normalise(EscalatorVector *a)
{
    // Preserve the native normalizer including its x-only zero-vector fallback
    float sq=a->x*a->x+a->y*a->y+a->z*a->z;if(sq>0.0f){
        // Multiply each original component by the reciprocal square root
        float inverse=1.0f/sqrtf(sq);a->x*=inverse;a->y*=inverse;a->z*=inverse;
    }else a->x=1.0f;
}
'''

def record(source):
    # Retain the full original private declaration for named-field oracle observations
    a=source.index('class CEscalator\n');b=source.index('\n};',a)+len('\n};');return source[a:b]

def method(source,name):
    # Delimit all four original bodies with their original opening-brace style
    match=re.search(r'(?m)^(?:void\s+)?CEscalator::'+name+r'\([^\n]*\)\s*\{',source);assert match,name
    a=source.index('{',match.start());b=a+1;depth=1
    while depth:depth+=(source[b]=='{')-(source[b]=='}');b+=1
    return source[match.start():b]

def calls(source):
    # Replace the complete private record and preserve matrix construction at the original source point
    if not any(token in source for token in ['CEscalator','Escalators_BackendAdd','Escalators_BackendUpdate','Escalators_BackendSwitchOff','Escalators_GetEscalatorLegacy','typedef struct EscalatorVector']):return script_paths.calls(source)
    if 'static CEscalator EscalatorOwners[NUM_ESCALATORS];' in source:
        source=source.replace('static CEscalator EscalatorOwners[NUM_ESCALATORS];','static CMatrix EscalatorMatrices[NUM_ESCALATORS];').replace('bool deletingEscalator;','')
        for name in ['CEscalator','AddThisOne','Update','SwitchOff']:source=source.replace(method(source,name),'')
        source+='\n'+HOST
    if 'class CEscalator\n' in source:source=source.replace(record(source),'//+ rouz edit (ChatGPT)\n#include "Escalator.h"\n//- rouz edit (ChatGPT)')
    source=source.replace('typedef struct EscalatorVector { float x,y,z; } EscalatorVector;','#include "Escalator.h"') if 'void Escalators_AddOne(' in source else source
    source=source.replace('class CEscalator;\nconst CEscalator &Escalators_GetEscalatorLegacy(int index);\n','')
    for name in ['Add','Update','SwitchOff']:
        if re.search(r'void Escalators_Backend'+name+r'\([^\n]*\)\n\{',source):source=source.replace(function(source,'Escalators_Backend',name),'')
        source=re.sub(r'^void Escalators_Backend'+name+r'\([^\n]*\);\n','',source,flags=re.M)
    if 'const CEscalator &Escalators_GetEscalatorLegacy(int index)\n{' in source:
        a=source.index('const CEscalator &Escalators_GetEscalatorLegacy(int index)\n{');b=source.index('\n}',a)+2;source=source[:a]+source[b:]
    for old,new in [('Add','AddThisOne'),('Update','Update'),('SwitchOff','SwitchOff')]:source=source.replace('Escalators_Backend'+old+'(','Escalator_'+new+'(')
    source=source.replace('return Escalators_GetEscalatorLegacy(index).IsActive();','return Escalator_IsActive(index);')
    source=source.replace('*position = AudioSoundPosition_FromVector(Escalators_GetEscalatorLegacy(index).GetPosition());','// Copy the C midpoint before publishing the complete original audio position value\n    EscalatorVector value=Escalator_GetPosition(index);\n    AudioSoundPosition result={value.x,value.y,value.z};*position=result;')
    return script_paths.calls(source)

def kernel(source):
    # Translate complete registration, allocation/recovery, motion and teardown without narrowing their scope
    body='\n\n'.join(method(source,name) for name in ['AddThisOne','Update','SwitchOff'])
    body=body.replace('CEscalator::AddThisOne(CVector pos0, CVector pos1, CVector pos2, CVector pos3, bool b_isMovingDown)','Escalator_AddThisOne(int32_t index,EscalatorVector pos0,EscalatorVector pos1,EscalatorVector pos2,EscalatorVector pos3,bool b_isMovingDown)')
    for name in ['Update','SwitchOff']:body=body.replace('CEscalator::'+name+'(void)','Escalator_'+name+'(int32_t index)')
    body=body.replace('CModelInfo::GetColModel(MI_ESCALATORSTEP)->boundingBox.max.z','Escalator_BackendStepHeight()')
    for a,b in [('m_pos0','m_pos1'),('m_pos1','m_pos2'),('m_pos2','m_pos3'),('m_pos0','m_midPoint')]:body=body.replace('('+a+' - '+b+').Magnitude()','Escalator_Magnitude(Escalator_Subtract('+a+','+b+'))')
    body=body.replace('CVector direction(m_pos0.x - m_pos1.x, m_pos0.y - m_pos1.y, 0.0f);','EscalatorVector direction={m_pos0.x-m_pos1.x,m_pos0.y-m_pos1.y,0.0f};').replace('direction.Normalise()','Escalator_Normalise(&direction)')
    for axis in ['Up','Forward','Right','Position']:body=re.sub(r'm_matrix.Get'+axis+r'\(\) = CVector\(([^()]*)\)',r'Escalator_BackendMatrix'+axis+r'(index,(EscalatorVector){\1})',body)
    body=body.replace('(m_pos0 + m_pos3) / 2.0f','Escalator_Divide(Escalator_Add(m_pos0,m_pos3),2.0f)')
    body=body.replace('(TheCamera.GetPosition() - m_midPoint).Magnitude()','Escalator_Magnitude(Escalator_Subtract(Escalator_BackendCameraPosition(),m_midPoint))').replace('TheCamera.IsSphereVisible(','Escalator_BackendSphereVisible(')
    body=body.replace('CPools::GetObjectPool()','Escalator_BackendObjectPool()').replace('CObjectPool *objectPool','CPool *objectPool')
    body=body.replace('existing->ObjectCreatedBy == TEMP_OBJECT','Escalator_BackendCreatedBy(existing) == ESCALATOR_TEMP_OBJECT')
    body=body.replace('existing->~CObject()','Escalator_BackendDestroy(existing)').replace('m_pSteps[i]->~CObject()','Escalator_BackendDestroy(m_pSteps[i])')
    for name in ['Add','Remove']:body=body.replace('CWorld::'+name+'(','Escalator_BackendWorld'+name+'(')
    body=body.replace('std::allocator<CObject>().construct(m_pSteps[i], MI_ESCALATORSTEP, TRUE)','Escalator_BackendConstruct(m_pSteps[i])')
    body=body.replace('m_pSteps[i]->SetPosition(m_pos1)','Escalator_BackendSetPosition(m_pSteps[i],m_pos1)').replace('m_pSteps[i]->SetPosition(newPosition)','Escalator_BackendSetPosition(m_pSteps[i],newPosition)')
    body=body.replace('m_pSteps[i]->ObjectCreatedBy = CONTROLLED_SUB_OBJECT','Escalator_BackendSetCreatedBy(m_pSteps[i],ESCALATOR_CONTROLLED_SUB_OBJECT)')
    body=body.replace('CVector oldPosition = m_pSteps[i]->GetPosition()','EscalatorVector oldPosition=Escalator_BackendPosition(m_pSteps[i])').replace('m_pSteps[i]->GetMatrix() = m_matrix','Escalator_BackendAssignMatrix(m_pSteps[i],index)').replace('CVector newPosition;','EscalatorVector newPosition;')
    for a,b in [('m_pos1','m_pos0'),('m_pos2','m_pos1'),('m_pos3','m_pos2')]:body=body.replace('(ratio * '+a+') + ((1.0f - ratio) * '+b+')','Escalator_Add(Escalator_LeftScale(ratio,'+a+'),Escalator_LeftScale(1.0f-ratio,'+b+'))')
    body=body.replace('m_pSteps[i]->m_vecMoveSpeed = (newPosition - oldPosition) / Max((*Timer_GetTimeStep()), 1.0f)','Escalator_BackendSetMoveSpeed(m_pSteps[i],Escalator_Divide(Escalator_Subtract(newPosition,oldPosition),Max((*Timer_GetTimeStep()),1.0f)))')
    body=body.replace('m_pSteps[i]->GetMatrix().UpdateRW()','Escalator_BackendUpdateRW(m_pSteps[i])').replace('m_pSteps[i]->UpdateRwFrame()','Escalator_BackendUpdateFrame(m_pSteps[i])')
    body=re.sub(r'(?<![\w_])SwitchOff\(\)',r'Escalator_SwitchOff(index)',body)
    for field in FIELDS:body=re.sub(r'\b'+field+r'\b','escalator->'+field,body)
    body=re.sub(r'\bint32\b','int32_t',body).replace('nil','NULL')
    body=re.sub(r'(?m)^(void\s+Escalator_\w+\([^\n]*\)\s*\{\n)',r'\1    // Preserve every original live field read, allocation decision and state-write order\n    Escalator *escalator=&Escalators_aEscalators[index];\n',body)
    body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original branch, callback and traversal order\n',body,flags=re.M)
    code=re.sub(r'//[^\n]*','',body);assert not re.search(r'\b(?:CVector|CMatrix|CEscalator|CPools|CWorld|CModelInfo|TheCamera|m_matrix|std)\b',code)
    return '''//+ rouz edit (ChatGPT)
#include "Escalator_backend.h"
#include "Timer.h"
#include <math.h>
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
Escalator Escalators_aEscalators[NUM_ESCALATORS];
bool deletingEscalator;
'''+HELPERS+body+'''
bool Escalator_IsActive(int32_t index)
{
    // Read the same live activation field used by the former inline audio getter
    return Escalators_aEscalators[index].m_bIsActive;
}
EscalatorVector Escalator_GetPosition(int32_t index)
{
    // Capture the same three midpoint components copied by the original audio caller
    return Escalators_aEscalators[index].m_midPoint;
}
//- rouz edit (ChatGPT)
'''

def project(source):
    # Add the uniquely named C11 record unit and retain every existing object-name distinction
    entry='<ClCompile Include="..\\src\\renderer\\Fluff.cpp" />'
    if '..\\src\\renderer\\Escalator.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\Escalator.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\Fluff.h" />'
    for name in ['Escalator.h','Escalator_backend.h']:
        if '..\\src\\renderer\\'+name+'"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\'+name+'" />')
    return script_paths.project(source)
