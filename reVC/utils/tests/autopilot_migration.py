"""Relocate the complete embedded vehicle autopilot record, initialization and persistence into C11."""
import re
import object_names_migration as objects
from events_migration import arguments
OBSERVER=None
OPERATIONS=['Init','ModifySpeed','RemoveOnePathNode','GetCruiseSpeed','Save','Load']

def fields(header):
    # Capture all original scalar, bitfield, vector and native-pointer declarations
    block=header[header.index('\tint32 m_nCurrentRouteNode;'):header.index('\n\tCAutoPilot(void)')]
    values=re.findall(r'^\s*(int32|uint32|int8|uint8|int16|float|CVector|CPathNode\s*\*|CVehicle\s*\*)\s*(m_\w+)([^;]*);',block,re.M)
    assert len(values)==33,len(values)
    return values

def scalar(source):
    # Preserve every original integer width and replace only native shorthand aliases
    for old,new in [('int32','int32_t'),('uint32','uint32_t'),('int16','int16_t'),('int8','int8_t'),('uint8','uint8_t')]:source=re.sub(r'\b'+old+r'\b',new,source)
    return source

def header(original):
    # Preserve the complete record shape while replacing implicit member construction with an explicit C operation
    enums=original[original.index('enum eCarMission'):original.index('class CAutoPilot')]
    enums=re.sub(r'enum (eCar\w+)(\s*\{[\s\S]*?\n\});',r'typedef enum \1\2 \1;',enums)
    block=original[original.index('\tint32 m_nCurrentRouteNode;'):original.index('\n\tCAutoPilot(void)')]
    block=scalar(block).replace('CVector','AutoPilotVector')
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "Timer.h"
#ifdef __cplusplus
class CVehicle;struct CPathNode;
#else
typedef struct CVehicle CVehicle;typedef struct CPathNode CPathNode;
#endif
'''+enums+'''typedef struct AutoPilotVector {float x,y,z;} AutoPilotVector;
typedef struct CAutoPilot {
'''+block+'''
} CAutoPilot;
#ifdef __cplusplus
extern "C" {
#endif
void AutoPilot_Init(CAutoPilot *self);
void AutoPilot_ModifySpeed(CAutoPilot *self,float speed);
void AutoPilot_RemoveOnePathNode(CAutoPilot *self);
float AutoPilot_GetCruiseSpeed(const CAutoPilot *self);
#ifdef COMPATIBLE_SAVES
void AutoPilot_Save(CAutoPilot *self,uint8_t **buf);
void AutoPilot_Load(CAutoPilot *self,uint8_t **buf);
#endif
#ifdef __cplusplus
}
class CVector;
AutoPilotVector AutoPilot_FromVector(CVector value);
CVector AutoPilot_ToVector(AutoPilotVector value);
#ifdef VALIDATE_SIZE
VALIDATE_SIZE(CAutoPilot,0x70);
#endif
#endif
//- rouz edit (ChatGPT)
'''

LEAVES=[('CCarPathLink *','Link','uint32_t index','return &ThePaths.m_carPathLinks[index];')]+[('float',name,'CCarPathLink *link','return link->Get'+name+'();') for name in ['X','Y','DirX','DirY']]
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "AutoPilot.h"
#ifdef __cplusplus
struct CCarPathLink;
extern "C" {
#else
typedef struct CCarPathLink CCarPathLink;
#endif
'''+''.join(kind+' AutoPilot_Backend'+name+'('+args+');\n' for kind,name,args,code in LEAVES)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def host():
    # Retain only typed path-link reads and vector boundaries for remaining native callers
    source='//+ rouz edit (ChatGPT)\n#include "common.h"\n#include "AutoPilot_backend.h"\n#include "PathFind.h"\n'
    source+='''AutoPilotVector AutoPilot_FromVector(CVector value)
{
    // Copy the original native destination components into the plain C record
    AutoPilotVector result={value.x,value.y,value.z};return result;
}
CVector AutoPilot_ToVector(AutoPilotVector value)
{
    // Copy the plain destination components at the remaining native argument boundary
    return CVector(value.x,value.y,value.z);
}
'''
    return source+'\n'.join(kind+' AutoPilot_Backend'+name+'('+args+')\n{\n    // Read the original typed path-link primitive at its original C policy access point\n    '+code+'\n}\n' for kind,name,args,code in LEAVES)+'//- rouz edit (ChatGPT)\n'

def operation(source,name):
    # Match the complete exact original operation including all compatible-save copies
    start=source.index('CAutoPilot::'+name+'(');opening=source.index('{',start)
    return source[opening:source.index('\n}',opening)+2]

def kernel(source,original):
    # Retain the entire original constructor, getter, speed calculation, path shift and save/load policies
    chunks=[]
    for name in OPERATIONS:
        if name=='Init':
            start=original.index('CAutoPilot(void)');opening=original.index('{',start);body=original[opening:original.index('\n\t}',opening)+3]
        elif name=='GetCruiseSpeed':body='{\n'+re.search(r'float GetCruiseSpeed\(void\)\s*\{([^}]+)\}',original)[1]+'\n}'
        else:body=operation(source,name)
        body=scalar(body)
        for kind,field,suffix in fields(original):body=re.sub(r'\b'+field+r'\b','self->'+field,body)
        body=body.replace('&ThePaths.m_carPathLinks[self->m_nCurrentPathNodeInfo]','AutoPilot_BackendLink(self->m_nCurrentPathNodeInfo)').replace('&ThePaths.m_carPathLinks[self->m_nNextPathNodeInfo]','AutoPilot_BackendLink(self->m_nNextPathNodeInfo)')
        for index in ['Current','Next']:
            for coord in ['DirX','DirY']:body=body.replace('ThePaths.m_carPathLinks[self->m_n'+index+'PathNodeInfo].Get'+coord+'()','AutoPilot_Backend'+coord+'(AutoPilot_BackendLink(self->m_n'+index+'PathNodeInfo))')
        for link in ['pCurrentLink','pNextLink']:
            for coord in ['X','Y']:body=body.replace(link+'->Get'+coord+'()','AutoPilot_Backend'+coord+'('+link+')')
        body=re.sub(r'CVector (position\w+)\(([\s\S]*?)\);',r'CurvesVector \1={\2};',body)
        for position in ['positionOnCurrentLinkIncludingLane','positionOnNextLinkIncludingLane']:body=body.replace('CurvesVector_FromVector(*(&'+position+'))',position)
        if name in ['Save','Load']:body=body.replace('&buf','buf')
        params='const CAutoPilot *self' if name=='GetCruiseSpeed' else 'CAutoPilot *self'+(',float speed' if name=='ModifySpeed' else ',uint8_t **buf' if name in ['Save','Load'] else '')
        body=body.replace('{','{\n    // Preserve the complete original '+name+' operation and all intentionally unwritten fields',1)
        text=('float' if name=='GetCruiseSpeed' else 'void')+' AutoPilot_'+name+'('+params+')\n'+body
        if name in ['Save','Load']:text='#ifdef COMPATIBLE_SAVES\n'+text+'\n#endif'
        chunks.append(text)
    return '''//+ rouz edit (ChatGPT)
#include "AutoPilot_backend.h"
#include "Curves.h"
#include "Timer.h"
#include "SaveBuf.h"
#define LANE_WIDTH 5.0f
#define Max(a,b) ((a)>(b)?(a):(b))
#define BIT(bit) (1u<<(bit))
'''+ '\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def autopilot_calls(source):
    # Remove the entire embedded class and adapt its explicit initialization, methods and vector consumers
    if 'class CAutoPilot {' in source:return header(source)
    if 'void CAutoPilot::ModifySpeed(float speed)' in source:
        if OBSERVER:OBSERVER(source)
        return host()
    original=source
    source=source.replace('CVehicle::CVehicle(uint8 CreatedBy)\n{\n\tint i;','CVehicle::CVehicle(uint8 CreatedBy)\n{\n    // Initialize the plain autopilot before any original vehicle constructor statement\n    AutoPilot_Init(&AutoPilot);\n\tint i;')
    source=source.replace('CVehicle(void) {}\t// FAKE','CVehicle(void) {\n        // Preserve the original autopilot initialization in the fake vehicle constructor\n        AutoPilot_Init(&AutoPilot);\n    }\t// FAKE')
    pattern=r'(?P<receiver>(?:\b\w+->)?AutoPilot)\.(?P<method>ModifySpeed|RemoveOnePathNode|GetCruiseSpeed|Save|Load)\('
    pos=0
    while (match:=re.search(pattern,source[pos:])):
        start=pos+match.start();opening=pos+match.end()-1;end,args=arguments(source,opening);receiver=match['receiver'];name=match['method']
        if name in ['Save','Load']:args=['&('+arg+')' for arg in args]
        value='AutoPilot_'+name+'(&'+receiver+('' if not args else ','+','.join(args))+')';source=source[:start]+value+source[end:];pos=start+len(value)
    vector=r'(?:\b\w+->)?AutoPilot\.m_vecDestinationCoors'
    # Preserve writable component access and wrap only complete native vector reads or assignments
    source=re.sub(r'('+vector+r')\s*=\s*([^;\n]+);',lambda match:match[0] if match[2].lstrip().startswith('AutoPilot_FromVector(') else match[1]+' = AutoPilot_FromVector('+match[2]+');',source)
    pattern=re.compile(vector+r'(?![\w.]|\s*=)');pos=0
    while (match:=pattern.search(source,pos)):
        if re.search(r'AutoPilot_ToVector\s*\(\s*$',source[max(0,match.start()-128):match.start()]):pos=match.end();continue
        start,end=match.span();value='AutoPilot_ToVector('+match[0]+')';source=source[:start]+value+source[end:];pos=start+len(value)
    if source==original:return source
    if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def autopilot_project(source):
    # Register a uniquely named C autopilot object and its typed native-access header
    for tag,old,new in [('ClCompile','AutoPilot.cpp','AutoPilotSystem.c'),('ClInclude','AutoPilot.h','AutoPilot_backend.h')]:
        entry='<'+tag+' Include="..\\src\\control\\'+old+'" />';addition='<'+tag+' Include="..\\src\\control\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\control\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\control\\'+re.escape(old)+r'">\n\s*<Filter>control</Filter>\n\s*</'+tag+'>)';source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove complete relocation of the original autopilot policy and explicit constructor assignments
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/control/AutoPilot.cpp').read_text())
    original=(root/'.diagnostics/autopilot-c-tests/before/src/control/AutoPilot.h').read_text()
    assert normalize(kernel(captured[0],original))==normalize((root/'src/control/AutoPilotSystem.c').read_text())

def verify_dependency(saved,current):
    # Permit only the complete explicit autopilot projection through historical dependency guards
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original);assert expected!=original and expected==current.read_text(),str(current)


def calls(source):
    # Preserve the complete autopilot projection before removing all profiler and cut-feature classes
    import profile_power_migration as profile_power
    return profile_power.calls(autopilot_calls(source))

def project(source):
    # Preserve earlier registrations before renaming both complete producers to C11
    import profile_power_migration as profile_power
    return profile_power.project(autopilot_project(source))
