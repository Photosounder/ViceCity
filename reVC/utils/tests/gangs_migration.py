"""Remove gang classes, their global constructors and all static member operations."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import fire_migration as fire
METHODS=['Initialise','HaveGangModelsLoaded','SetGangPedModels','SetWillAttackPlayerWithCops','GetWillAttackPlayerWithCops','ChooseGangPedModel','SetGangVehicleModel','SetGangWeapons','SetGangPedModelOverride','GetGangPedModelOverride','SaveAllGangData','LoadAllGangData']
ACCESSORS=[('int32_t','GetGangPedModel1','return Gangs_records[gang].m_nPedModel1MI;'),('int32_t','GetGangPedModel2','return Gangs_records[gang].m_nPedModel2MI;'),('int32_t','GetGangVehicleModel','return Gangs_records[gang].m_nVehicleMI;'),('GangInfo *','GetGangInfo','return &Gangs_records[gang];')]
FIELDS=['m_nVehicleMI','m_nPedModel1MI','m_nPedModel2MI','m_nPedModelOverride','m_Weapon1','m_Weapon2']
OBSERVER=None
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "PedType.h"
#include "GangTypes.h"
typedef struct GangInfo {
    int32_t m_nVehicleMI;
    int32_t m_nPedModel1MI;
    int32_t m_nPedModel2MI;
    int8_t m_nPedModelOverride;
    int32_t m_Weapon1;
    int32_t m_Weapon2;
} GangInfo;
#ifdef __cplusplus
extern "C" {
#endif
void Gangs_Initialise(void);
bool Gangs_HaveGangModelsLoaded(int16_t gang);
void Gangs_SetGangPedModels(int16_t gang,int32_t first,int32_t second);
void Gangs_SetWillAttackPlayerWithCops(ePedType type,bool will);
bool Gangs_GetWillAttackPlayerWithCops(ePedType type);
int32_t Gangs_ChooseGangPedModel(int16_t gang);
void Gangs_SetGangVehicleModel(int16_t gang,int32_t model);
void Gangs_SetGangWeapons(int16_t gang,int32_t first,int32_t second);
void Gangs_SetGangPedModelOverride(int16_t gang,int8_t overrideModel);
int8_t Gangs_GetGangPedModelOverride(int16_t gang);
void Gangs_SaveAllGangData(uint8_t *buffer,uint32_t *size);
void Gangs_LoadAllGangData(uint8_t *buffer,uint32_t size);
'''+''.join(kind+' Gangs_'+name+'(int16_t gang);\n' for kind,name,body in ACCESSORS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
bool Gangs_BackendHasModelLoaded(int32_t model);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "ModelIndices.h"
#include "Streaming.h"
#include "Gangs_backend.h"

bool Gangs_BackendHasModelLoaded(int32_t model)
{
    // Query the original streaming table at each conditional lookup in the C controller
    return CStreaming::HasModelLoaded(model);
}
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Preserve every complete original method and the original record-assignment save semantics
    assert re.search(r'CGangInfo::CGangInfo\(\)\s*:\s*m_nVehicleMI\(-1\),\s*m_nPedModel1MI\(-1\),\s*m_nPedModel2MI\(-1\),\s*m_nPedModelOverride\(-1\),\s*m_Weapon1\(WEAPONTYPE_UNARMED\),\s*m_Weapon2\(WEAPONTYPE_UNARMED\)\s*\{\}',source)
    chunks=[]
    comments={'Initialise':'Restore the original eight gang defaults while retaining unrelated live fields','HaveGangModelsLoaded':'Query the second live model only when the first streaming query succeeds','SetGangPedModels':'Write both original model fields through the same canonical record','SetWillAttackPlayerWithCops':'Change attack policy only for the original nine gang pedestrian types','GetWillAttackPlayerWithCops':'Read attack policy only for the original valid gang pedestrian range','ChooseGangPedModel':'Preserve override short-circuiting and the original random draw','SetGangVehicleModel':'Update the live vehicle model without narrowing its signed value','SetGangWeapons':'Write both original signed weapon fields in order','SetGangPedModelOverride':'Store the original signed-byte override without interpreting its value','GetGangPedModelOverride':'Read the live signed-byte override unchanged','SaveAllGangData':'Preserve the original complete record assignments and save-header order','LoadAllGangData':'Initialize at the original point before validating and assigning saved records'}
    for name in METHODS:
        text=function(source,'CGangs::',name)
        text=re.sub(r'(?m)^\s*//[+-] rouz edit \(ChatGPT\)\n','\n',text).replace(' // rouz edit (ChatGPT)','')
        text=text.replace('CGangs::','Gangs_').replace('CGangInfo','GangInfo').replace('CStreaming::HasModelLoaded','Gangs_BackendHasModelLoaded')
        for method in METHODS+[entry[1] for entry in ACCESSORS]:text=re.sub(r'(?<![\w])'+method+r'\(', 'Gangs_'+method+'(',text)
        text=re.sub(r'\bGang\b','Gangs_records',text);text=re.sub(r'\bGangAttackWithCops\b','Gangs_attackWithCops',text)
        for old,new in [('int32','int32_t'),('int16','int16_t'),('int8','int8_t'),('uint8','uint8_t'),('uint32','uint32_t')]:text=re.sub(r'\b'+old+r'\b',new,text)
        text=text.replace('\n{','\n{\n    // '+comments[name],1);chunks.append(text)
    for kind,name,body in ACCESSORS:chunks.append(kind+' Gangs_'+name+'(int16_t gang)\n{\n    // Access the original canonical gang slot without changing its index contract\n    '+body+'\n}')
    initializers=',\n'.join('    {-1,-1,-1,-1,WEAPONTYPE_UNARMED,WEAPONTYPE_UNARMED}' for i in range(9))
    return '''//+ rouz edit (ChatGPT)
#include "Gangs.h"
#include "Gangs_backend.h"
#include "General.h"
#include "ModelIndexData.h"
#include "WeaponType.h"
#undef assert
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "SaveBuf.h"
_Static_assert(NUM_GANGS==9,"Original gang count for constructor-equivalent static initialization");
_Static_assert(sizeof(GangInfo)==24,"Current original gang record layout");
static GangInfo Gangs_records[NUM_GANGS]={
'''+initializers+'''
};
static bool Gangs_attackWithCops[NUM_GANGS];

'''+ '\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def gang_calls(source):
    # Replace the complete class files and all qualified callers without wrapping existing outer markers twice
    if 'class CGangs\n' in source:return HEADER
    if re.search(r'(?m)^CGangInfo CGangs::Gang\[NUM_GANGS\];',source):
        if OBSERVER:OBSERVER(source)
        return HOST
    changed=source.replace('CGangs::','Gangs_');changed=re.sub(r'\bCGangInfo\b','GangInfo',changed)
    if changed==source:return source
    if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)'):return changed
    return '//+ rouz edit (ChatGPT)\n'+changed.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Forward complete historical owners through the separately verified fire class removal
    return fire.calls(gang_calls(source))
def project(source):
    # Give the new C controller a distinct basename while retaining the single native streaming adapter
    for tag,old,new in [('ClCompile','Gangs.cpp','GangData.c'),('ClInclude','Gangs.h','Gangs_backend.h')]:
        entry='<'+tag+' Include="..\\src\\peds\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\peds\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\peds\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\peds\\'+re.escape(old)+r'">\n\s*<Filter>peds</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return fire.project(source)

def verify_legacy(source,transform,root):
    # Prove both complete relocated gang logic and its native streaming binding from earlier snapshots
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/peds/Gangs.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/peds/GangData.c').read_text())

def verify_dependency(saved,current):
    # Preserve byte guards unless the entire dependency follows the exact gang conversion
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original)
    assert expected!=original and expected==current.read_text(),str(current)
