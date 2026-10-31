"""Move pedestrian type and statistic records, parsers and save operations into C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import projectile_migration as projectiles

TYPES=['Initialise','Shutdown','LoadPedData','FindPedType','FindPedFlag','Save','Load']
STATS=['Initialise','Shutdown','LoadPedStats','GetPedStatType']
ACCESSORS=[('uint32_t','GetFlag','int type','return PedType_records[type]->m_flag;'),
 ('uint32_t','GetAvoid','int type','return PedType_records[type]->m_avoid;'),
 ('uint32_t','GetThreats','int type','return PedType_records[type]->m_threats;'),
 ('void','SetThreats','int type, uint32_t threat','PedType_records[type]->m_threats = threat;'),
 ('void','AddThreat','int type, int threat','PedType_records[type]->m_threats |= threat;'),
 ('void','RemoveThreat','int type, int threat','PedType_records[type]->m_threats &= ~threat;'),
 ('bool','IsThreat','int type, int threat','return PedType_records[type]->m_threats & threat;')]
OBSERVER=None

BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
void *PedData_BackendAllocate(size_t size);
void PedData_BackendFree(void *memory);
void PedData_BackendDebug(const char *message);
void PedData_BackendUnknownType(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "PedData_backend.h"

void *PedData_BackendAllocate(size_t size)
{
    // Retain the existing native allocation provider for records and parser buffers
    return malloc(size);
}

void PedData_BackendFree(void *memory)
{
    // Release each allocation through the matching native provider
    free(memory);
}

void PedData_BackendDebug(const char *message)
{
    // Preserve the native debug prefix and message text
    debug("%s", message);
}

void PedData_BackendUnknownType(void)
{
    // Report the original unknown-type diagnostic before returning its sentinel
    Error("Unknown ped type, Pedtype.cpp");
}
//- rouz edit (ChatGPT)
'''

def header(source):
    # Preserve every original enumeration and stored member while removing class-only declarations
    source=re.sub(r'enum (ePedType|ePedStats)\n\{',r'typedef enum \1\n{',source)
    for name in ['ePedType','ePedStats']:
        source=re.sub(r'(typedef enum '+name+r'\n\{[\s\S]*?)\n};',r'\1\n} '+name+';',source,count=1)
    # Match the complete original class and validation without altering its neighboring enums
    pattern=r'class CPedType\n\{[\s\S]*?VALIDATE_SIZE\(CPedType, 0x20\);'
    fields='''typedef struct PedType
{
    uint32_t m_flag;
    float unknown1, unknown2, unknown3, unknown4, unknown5;
    uint32_t m_threats, m_avoid;
} PedType;'''
    source,count=re.subn(pattern,fields,source);assert count==1
    pattern=r'class CPedStats\n\{\npublic:\n([\s\S]*?)\n\tstatic CPedStats \*ms_apPedStats[\s\S]*?VALIDATE_SIZE\(CPedStats, 0x34\);'
    source,count=re.subn(pattern,lambda m:'typedef struct PedStats\n{\n'+m[1]+'\n} PedStats;',source);assert count==1
    for old,new in [('int8','int8_t'),('int16','int16_t')]:source=re.sub(r'\b'+old+r'\b',new,source)
    source=source.replace('#pragma once','#pragma once\n#include <stdint.h>\n#include <stdbool.h>')
    prototypes='''
#ifdef __cplusplus
extern "C" {
#endif
extern PedType *PedType_records[NUM_PEDTYPES];
extern PedStats *PedStats_records[NUM_PEDSTATS];
void PedType_Initialise(void);
void PedType_Shutdown(void);
void PedType_LoadPedData(void);
ePedType PedType_FindPedType(char *type);
uint32_t PedType_FindPedFlag(char *type);
void PedType_Save(uint8_t *buf, uint32_t *size);
void PedType_Load(uint8_t *buf, uint32_t size);
'''
    prototypes+=''.join(kind+' PedType_'+name+'('+args+');\n' for kind,name,args,body in ACCESSORS)
    prototypes+='''void PedStats_Initialise(void);
void PedStats_Shutdown(void);
void PedStats_LoadPedStats(void);
ePedStats PedStats_GetPedStatType(char *name);
#ifdef __cplusplus
}
#endif
'''
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip()+'\n'+prototypes+'//- rouz edit (ChatGPT)\n'

def rename(source):
    # Translate only original pedestrian class operations, canonical arrays and record names
    source=source.replace('CPedType::ms_apPedType','PedType_records').replace('CPedStats::ms_apPedStats','PedStats_records')
    source=source.replace('CPedType::','PedType_').replace('CPedStats::','PedStats_')
    source=re.sub(r'\bCPedType\b','PedType',source);source=re.sub(r'\bCPedStats\b','PedStats',source)
    return source

def kernel(source):
    # Relocate each complete original body and explicitly retain trivial C++ value initialization
    chunks=[]
    for cls,names in [('CPedType',TYPES),('CPedStats',STATS)]:
        for name in names:
            body=function(source,cls+'::',name)
            body=re.sub(r'(?m)^\s*//[+-] rouz edit \(ChatGPT\)\n','\n',body)
            body=body.replace(' // rouz edit (ChatGPT)','')
            body=body.replace('debug(', 'PedData_BackendDebug(').replace('Error("Unknown ped type, Pedtype.cpp")','PedData_BackendUnknownType()')
            body=body.replace('malloc(', 'PedData_BackendAllocate(').replace('free(', 'PedData_BackendFree(')
            array='ms_apPedType' if cls=='CPedType' else 'ms_apPedStats'
            body=re.sub(r'std::allocator<'+cls+r'>\(\)\.construct\('+array+r'\[i\]\);',
                '// Preserve value initialization, including the previously implicit zero fields\n\t\tmemset('+array+'[i], 0, sizeof(*'+array+'[i]));',body)
            body=re.sub(r'\s*std::allocator<'+cls+r'>\(\)\.destroy\('+array+r'\[i\]\);','',body)
            body=rename(body)
            body=body.replace('ms_apPedType','PedType_records').replace('ms_apPedStats','PedStats_records')
            for method in (TYPES if cls=='CPedType' else STATS):body=re.sub(r'(?<![\w])'+method+r'\(',('PedType_' if cls=='CPedType' else 'PedStats_')+method+'(',body)
            for old,new in [('uint32','uint32_t'),('uint8','uint8_t'),('uint16','uint16_t'),('ssize_t','FileMgrSize')]:body=re.sub(r'\b'+old+r'\b',new,body)
            # Preserve original diagnostic spelling even though the stored records have C names
            body=body.replace('Initialising PedType','Initialising CPedType').replace('PedType ready','CPedType ready').replace('Shutting down PedType','Shutting down CPedType').replace('PedType shut down','CPedType shut down')
            body=body.replace('Initialising PedStats','Initialising CPedStats').replace('PedStats ready','CPedStats ready').replace('Shutting down PedStats','Shutting down CPedStats').replace('PedStats shut down','CPedStats shut down')
            body=body.replace('// Construct the ped type without invoking C++ new.','// Allocate each record through the existing native provider').replace('// Construct the ped stats without invoking C++ new.','// Allocate each record through the existing native provider')
            body=body.replace('// unknown3 not initialized','// Unknown3 retains the original implicit value initialization')
            body=body.replace('// Destroy and release the ped type without invoking C++ delete.','// Release each trivially destructible record through the native provider').replace('// Destroy and release the ped stats without invoking C++ delete.','// Release each trivially destructible record through the native provider')
            body=body.replace('\n{','\n{\n    // Execute the complete original '+('pedestrian type' if cls=='CPedType' else 'pedestrian statistics')+' '+name+' operation',1)
            chunks.append(body)
    for kind,name,args,body in ACCESSORS:chunks.append(kind+' PedType_'+name+'('+args+')\n{\n    // Preserve the original live record access and bit-mask conversion\n    '+body+'\n}')
    return '''//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "PedType.h"
#include "PedData_backend.h"
#include "General.h"
#include "FileMgr.h"
#undef assert
#ifndef MASTER
void re3_assert(const char *, const char *, unsigned int, const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression, __FILE__, __LINE__, __func__), 0)))
#else
#define assert(expression) (expression)
#endif
#include "SaveBuf.h"
#include <stdio.h>
#include <string.h>
_Static_assert(sizeof(PedType) == 0x20, "Original pedestrian type record layout");
_Static_assert(sizeof(PedStats) == 0x34, "Original pedestrian statistics record layout");
PedType *PedType_records[NUM_PEDTYPES];
PedStats *PedStats_records[NUM_PEDSTATS];

'''+ '\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def pedestrian_calls(source):
    # Retain full caller text and existing outer edit markers when forwarding older snapshots
    if 'CPedType *CPedType::ms_apPedType' in source:
        if OBSERVER:OBSERVER(source)
        return HOST
    if 'class CPedType\n' in source:return header(source)
    changed=rename(source)
    if changed==source:return source
    if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)'):return changed
    return '//+ rouz edit (ChatGPT)\n'+changed.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Forward complete historical callers through the separately verified projectile-controller removal
    return projectiles.calls(pedestrian_calls(source))
def project(source):
    # Give the C kernel a distinct basename and register both public C boundary files
    entry='<ClCompile Include="..\\src\\peds\\PedType.cpp" />'
    if '..\\src\\peds\\PedestrianData.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\peds\\PedestrianData.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\peds\\PedType.h" />'
    if '..\\src\\peds\\PedData_backend.h"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\peds\\PedData_backend.h" />')
    for tag,old,new in [('ClCompile','PedType.cpp','PedestrianData.c'),('ClInclude','PedType.h','PedData_backend.h')]:
        pattern=r'(<'+tag+r' Include="\.\.\\src\\peds\\'+re.escape(old)+r'">\n\s*<Filter>peds</Filter>\n\s*</'+tag+'>)'
        if '..\\src\\peds\\'+new+'"' not in source:source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return projectiles.project(source)

def verify_legacy(source,transform,root):
    # Prove both complete relocated algorithms and their native bridge from immutable earlier snapshots
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/peds/PedType.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/peds/PedestrianData.c').read_text())

def verify_dependency(saved,current):
    # Preserve byte-exact dependency guards unless the entire file matches the known pedestrian conversion
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original)
    assert expected!=original and expected==current.read_text(),str(current)
