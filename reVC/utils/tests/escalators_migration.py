"""Move the complete escalator controller and count into C11 with actual typed records."""
import re
from audio_controls_migration import function
import escalator_record_migration as records
METHODS=['AddOne','Init','Update','Shutdown']
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct EscalatorVector { float x,y,z; } EscalatorVector;
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t Escalators_Num;
void Escalators_Init(void);
void Escalators_Update(void);
void Escalators_Shutdown(void);
void Escalators_AddOne(EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool movingDown);
#ifdef __cplusplus
}
class CEscalator;
const CEscalator &Escalators_GetEscalatorLegacy(int index);
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('bool','Replay','void','return CReplay::IsPlayingBack();'),
('void','Add','int32_t index,EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool movingDown','EscalatorOwners[index].AddThisOne(CVector(first.x,first.y,first.z),CVector(lower.x,lower.y,lower.z),CVector(upper.x,upper.y,upper.z),CVector(last.x,last.y,last.z),movingDown);'),
('void','Update','int32_t index','EscalatorOwners[index].Update();'),
('void','SwitchOff','int32_t index','EscalatorOwners[index].SwitchOff();')]
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "Escalators.h"
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' Escalators_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='//+ rouz edit (ChatGPT)\n#include "Escalators_backend.h"\n'+''.join(result+' Escalators_Backend'+name+'('+args+')\n{\n    // Retain the original typed record operation at its C-selected index\n    '+body+'\n}\n' for result,name,args,body in READERS)+'''const CEscalator &Escalators_GetEscalatorLegacy(int index)
{
    // Preserve the actual const record reference borrowed by the existing audio readers
    return EscalatorOwners[index];
}
//- rouz edit (ChatGPT)
'''

def method(source,name):
    # Extract every original statement without changing its opening-brace formatting
    match=re.search(r'(?m)^void\s+CEscalators::'+name+r'\([^\n]*\)\s*\{',source);assert match,name
    a=source.index('{',match.start());b=a+1;depth=1
    while depth:depth+=(source[b]=='{')-(source[b]=='}');b+=1
    return source[match.start():b]

def controller(source):
    # Keep the original controller declaration for the independent comparison oracle
    a=source.index('class CEscalators\n');b=source.index('\n};',a)+len('\n};');return source[a:b]

def record(source):
    # Delimit the complete unchanged native record and its CMatrix lifetime owner
    a=source.index('class CEscalator\n');b=source.index('\n};',a)+len('\n};');return source[a:b]

def calls(source):
    # Move only the controller and retain native record construction at its original source point
    if 'CEscalators' not in source and 'MovingThings_BackendEscalators' not in source:return records.calls(source)
    protected=None
    if 'cAudioManager::ProcessEscalators(' in source:
        protected=function(source,'cAudioManager::','ProcessEscalators');source=source.replace(protected,'@@ESCALATORS_ORIGINAL_AUDIO_BODY@@')
    if 'CEscalator CEscalators::aEscalators[NUM_ESCALATORS];' in source:
        source=source.replace('CEscalator CEscalators::aEscalators[NUM_ESCALATORS];','//+ rouz edit (ChatGPT)\nstatic CEscalator EscalatorOwners[NUM_ESCALATORS];\n//- rouz edit (ChatGPT)').replace('int32 CEscalators::NumEscalators;','')
        for name in METHODS:source=source.replace(method(source,name),'')
        source+='\n'+HOST
    if 'class CEscalators\n' in source:source=source.replace(controller(source),'//+ rouz edit (ChatGPT)\n#include "Escalators.h"\n//- rouz edit (ChatGPT)')
    for name in ['Init','Shutdown','Update']:
        if re.search(r'MovingThings_BackendEscalators'+name+r'\([^\n]*\)\n\{',source):source=source.replace(function(source,'MovingThings_BackendEscalators',name),'')
        source=re.sub(r'^void MovingThings_BackendEscalators'+name+r'\(void\);\n','',source,flags=re.M)
    if 'MovingThings_BackendEscalators' in source:
        source=source.replace('#include "MovingThings_backend.h"','#include "MovingThings_backend.h"\n#include "Escalators.h"')
        source=source.replace('MovingThings_BackendEscalators','Escalators_')
    lines=[]
    for line in source.splitlines(keepends=True):
        changed=line.replace('CEscalators::GetEscalator','Escalators_GetEscalatorLegacy').replace('CEscalators::NumEscalators','Escalators_Num')
        for name in METHODS:changed=changed.replace('CEscalators::'+name,'Escalators_'+name)
        if changed!=line and not line.lstrip().startswith('//'):lines.append(re.match(r'\s*',line)[0]+'// Enter the C controller or read its canonical count at the original call point\n')
        lines.append(changed)
    result=''.join(lines)
    return records.calls(result.replace('@@ESCALATORS_ORIGINAL_AUDIO_BODY@@',protected) if protected is not None else result)

def kernel(source):
    # Relocate every registration coordinate, replay gate, traversal and count write unchanged
    body='\n\n'.join(method(source,name) for name in METHODS)
    body=body.replace('CEscalators::AddOne(CVector pos0, CVector pos1, CVector pos2, CVector pos3, bool b_isMovingDown)','Escalators_AddOne(EscalatorVector pos0,EscalatorVector pos1,EscalatorVector pos2,EscalatorVector pos3,bool b_isMovingDown)')
    body=body.replace('aEscalators[NumEscalators++].AddThisOne(pos0, pos1, pos2, pos3, b_isMovingDown)','Escalators_BackendAdd(Escalators_Num++,pos0,pos1,pos2,pos3,b_isMovingDown)')
    for name in ['Init','Update','Shutdown']:body=body.replace('CEscalators::'+name+'(void)','Escalators_'+name+'(void)')
    for name in ['AddOne','Shutdown']:body=re.sub(r'(?<!\w)'+name+r'\(',r'Escalators_'+name+'(',body)
    body=re.sub(r'\bNumEscalators\b','Escalators_Num',body)
    body=re.sub(r'CVector\(([^()]*)\)',r'(EscalatorVector){\1}',body)
    body=body.replace('aEscalators[i].SwitchOff()','Escalators_BackendSwitchOff(i)').replace('aEscalators[i].Update()','Escalators_BackendUpdate(i)').replace('CReplay::IsPlayingBack()','Escalators_BackendReplay()')
    body=re.sub(r'\bTRUE\b','true',body);body=re.sub(r'\bFALSE\b','false',body)
    body=re.sub(r'(?m)^((?:void)\s+Escalators_\w+\([^\n]*\)\s*\{\n)',r'\1    // Preserve all original registration, replay and shutdown write ordering\n',body)
    body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Visit every original slot regardless of the live registration count\n',body,flags=re.M)
    assert not re.search(r'\b(?:CEscalators|CVector|CReplay|aEscalators)\b',body)
    return records.calls('//+ rouz edit (ChatGPT)\n#include "Escalators_backend.h"\nint32_t Escalators_Num;\n'+body+'\n//- rouz edit (ChatGPT)\n')

def project(source):
    # Register one uniquely named C11 unit and retain every paired object-name distinction
    entry='<ClCompile Include="..\\src\\renderer\\Fluff.cpp" />'
    if '..\\src\\renderer\\Escalators.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\Escalators.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\Fluff.h" />'
    for name in ['Escalators.h','Escalators_backend.h']:
        if '..\\src\\renderer\\'+name+'"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\'+name+'" />')
    return records.project(source)

HEADER=records.calls(HEADER)
BACKEND_HEADER=records.calls(BACKEND_HEADER)
HOST=records.calls(HOST)
