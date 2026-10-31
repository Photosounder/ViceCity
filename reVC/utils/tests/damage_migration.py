"""Move the complete vehicle damage manager and replay bit reader into C11."""
import re
import object_names_migration as objects
RENAMES={'src/vehicles/DamageManager.cpp':'src/vehicles/DamageManager.c'}
OPERATIONS=['ResetDamageStatus','FuckCarCompletely','ApplyDamage','GetComponentGroup','SetDoorStatus','GetDoorStatus','ProgressDoorDamage','SetPanelStatus','GetPanelStatus','ProgressPanelDamage','SetLightStatus','GetLightStatus','SetWheelStatus','GetWheelStatus','ProgressWheelDamage','SetEngineStatus','GetEngineStatus','ProgressEngineDamage']
INIT='''
    // Initialize damage before the former door member constructors
    Damage_Init(&Damage);'''

def parts(original):
    # Relocate both unchanged vehicle part enums without importing the C++ vehicle owner
    blocks=[re.search(r'enum '+name+r'\n\{[\s\S]*?\n\};',original)[0] for name in ['ePanels','eLights']]
    return '//+ rouz edit (ChatGPT)\n#pragma once\n'+'\n\n'.join(blocks)+'\n//- rouz edit (ChatGPT)\n'

def header(original):
    # Keep the exact record fields and expose explicit signed indices for the original component/light arithmetic
    enums=original[original.index('// TODO:'):original.index('enum eLights;')]
    for name in ['tComponent','tComponentGroup']:enums=re.sub(r'enum '+name+r'\n(\{[\s\S]*?)\n\};','typedef enum '+name+r'\n\1\n} '+name+';',enums,count=1)
    fields=original[original.index('\tfloat m_fWheelDamageEffect;'):original.index('\n\tCDamageManager(void);')].replace('uint8 ','uint8_t ').replace('uint32 ','uint32_t ')
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "VehicleDamageParts.h"
'''+enums+'''
typedef struct CDamageManager {
'''+fields+'''
} CDamageManager;
#ifdef __cplusplus
extern "C" {
#endif
extern float G_aComponentDamage[7];
void Damage_Init(CDamageManager *self);
void Damage_ResetDamageStatus(CDamageManager *self);
void Damage_FuckCarCompletely(CDamageManager *self);
bool Damage_ApplyDamage(CDamageManager *self,int32_t component,float damage,float unused);
bool Damage_GetComponentGroup(CDamageManager *self,int32_t component,tComponentGroup *componentGroup,uint8_t *subComp);
void Damage_SetDoorStatus(CDamageManager *self,int32_t door,uint32_t status);
int32_t Damage_GetDoorStatus(CDamageManager *self,int32_t door);
bool Damage_ProgressDoorDamage(CDamageManager *self,uint8_t door);
void Damage_SetPanelStatus(CDamageManager *self,int32_t panel,uint32_t status);
int32_t Damage_GetPanelStatus(CDamageManager *self,int32_t panel);
bool Damage_ProgressPanelDamage(CDamageManager *self,uint8_t panel);
int32_t Damage_GetPanelStatusFromWord(uint32_t panelstatus,int32_t panel);
void Damage_SetLightStatus(CDamageManager *self,int32_t light,uint32_t status);
int32_t Damage_GetLightStatus(CDamageManager *self,int32_t light);
void Damage_SetWheelStatus(CDamageManager *self,int32_t wheel,uint32_t status);
int32_t Damage_GetWheelStatus(CDamageManager *self,int32_t wheel);
bool Damage_ProgressWheelDamage(CDamageManager *self,uint8_t wheel);
void Damage_SetEngineStatus(CDamageManager *self,uint32_t status);
int32_t Damage_GetEngineStatus(CDamageManager *self);
bool Damage_ProgressEngineDamage(CDamageManager *self);
#ifdef __cplusplus
}
#if defined(VALIDATE_SIZE)
VALIDATE_SIZE(CDamageManager,0x1C);
#endif
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Preserve all original bodies while representing packed shifts with unsigned C arithmetic
    source=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
    source=source.replace('CDamageManager::CDamageManager(void)','Damage_Init(CDamageManager *self)')
    source=source.replace('Damage_Init(CDamageManager *self)','void\nDamage_Init(CDamageManager *self)',1)
    for name in OPERATIONS:
        source=source.replace('CDamageManager::'+name+'(void)','Damage_'+name+'(CDamageManager *self)')
        source=source.replace('CDamageManager::'+name+'(','Damage_'+name+'(CDamageManager *self,')
    source=source.replace('tComponent component','int32_t component').replace('eLights light','int32_t light').replace('(eLights)subComp','(int32_t)subComp')
    for old,new in [('uint8','uint8_t'),('uint32','uint32_t'),('int32','int32_t')]:source=re.sub(r'\b'+old+r'\b',new,source)
    source=re.sub(r'(?<![\w>.])m_(\w+)\b',r'self->m_\1',source);source=re.sub(r'(?<![\w>.])field_18\b','self->field_18',source)
    for name in OPERATIONS:
        source=re.sub(r'(?<![\w_])'+name+r'\((?!CDamageManager)',lambda match:'Damage_'+name+'(self'+('' if source[match.end():].startswith(')') else ', '),source)
    source=source.replace('dpb(','Damage_DepositBits(').replace('ldb(','Damage_LoadBits(')
    source=re.sub(r'(Damage_\w+\([^\n]*\)\n\{)',r'\1\n    // Preserve the complete original damage operation and its ordered field accesses',source)
    source=source.replace('bool\nDamage_GetComponentGroup(CDamageManager *self,','bool\nDamage_GetComponentGroup(CDamageManager *self,')
    source=source.replace('Damage_ApplyDamage(CDamageManager *self,int32_t component, float damage, float unused)\n{','Damage_ApplyDamage(CDamageManager *self,int32_t component, float damage, float unused)\n{\n    // Retain the unused scalar argument without changing the damage calculation\n    (void)unused;')
    source=source.replace('Damage_GetComponentGroup(CDamageManager *self,int32_t component, tComponentGroup *componentGroup, uint8_t *subComp)\n{','Damage_GetComponentGroup(CDamageManager *self,int32_t component, tComponentGroup *componentGroup, uint8_t *subComp)\n{\n    // Keep this stateless operation independent of the record\n    (void)self;')
    source=source.replace('Damage_ProgressEngineDamage(CDamageManager *self)\n{','Damage_ProgressEngineDamage(CDamageManager *self)\n{\n    // Preserve the original cut-feature no-op without reading the record\n    (void)self;')
    prefix='''//+ rouz edit (ChatGPT)
#include "DamageManager.h"
#include "config.h"
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
static inline uint32_t Damage_DepositBits(uint32_t bits,uint32_t position,uint32_t size,uint32_t word)
{
    // Preserve the original packed bit pattern with unsigned C shifts
    uint32_t mask=((UINT32_C(1)<<size)-UINT32_C(1))<<position;
    return (word & ~mask) | ((bits<<position) & mask);
}
static inline uint32_t Damage_LoadBits(uint32_t position,uint32_t size,uint32_t word)
{
    // Extract the same low-width field from the packed word
    return word>>position & ((UINT32_C(1)<<size)-UINT32_C(1));
}
'''
    return prefix+source.strip()+'''

int32_t Damage_GetPanelStatusFromWord(uint32_t panelstatus,int32_t panel)
{
    // Preserve the original replay accessor without requiring a damage record
    return Damage_LoadBits(panel*4,4,panelstatus);
}
//- rouz edit (ChatGPT)
'''

def atom_start(source,end):
    # Find one complete receiver atom including casts, calls and member chains
    while end and source[end-1].isspace():end-=1
    if source[end-1] in ')]':
        closing=source[end-1];opening='(' if closing==')' else '[';depth=1;start=end-1
        while depth:
            start-=1
            if source[start]==closing:depth+=1
            elif source[start]==opening:depth-=1
        before=start
        while before and source[before-1].isspace():before-=1
        groupedCast=bool(re.match(r'\(\s*\(\s*CAutomobile\b',source[start:end]))
        if not groupedCast and '\n' not in source[before:start] and before and (source[before-1].isalnum() or source[before-1]=='_'):
            candidate=atom_start(source,before)
            if source[candidate:before] not in ['if','for','while','switch','return','else','sizeof']:start=candidate
    else:
        start=end
        while start and (source[start-1].isalnum() or source[start-1]=='_'):start-=1
    before=start
    while before and source[before-1].isspace():before-=1
    if source[max(0,before-2):before] in ['->','::']:return atom_start(source,before-2)
    if before and source[before-1]=='.':return atom_start(source,before-1)
    return start

def damage_calls(source):
    # Replace the complete producer and every damage receiver without splitting cast expressions
    if 'class CDamageManager\n' in source:return header(source)
    if 'CDamageManager::CDamageManager(void)' in source:return kernel(source)
    original=source
    if 'class CVehicle :' in source and 'enum ePanels\n' in source and 'enum eLights\n' in source:
        source=re.sub(r'enum ePanels\n\{[\s\S]*?\n\};','#include "VehicleDamageParts.h"',source,count=1)
        source=re.sub(r'enum eLights\n\{[\s\S]*?\n\};\n','',source,count=1)
    source=source.replace('CDamageManager::GetPanelStatus(','Damage_GetPanelStatusFromWord(').replace('class CDamageManager;','struct CDamageManager;')
    matches=list(re.finditer(r'\bDamage\.('+ '|'.join(OPERATIONS)+r')\(',source))
    for match in reversed(matches):
        start=match.start();before=start
        while before and source[before-1].isspace():before-=1
        if source[max(0,before-2):before]=='->':start=atom_start(source,before-2)
        elif before and source[before-1]=='.':start=atom_start(source,before-1)
        receiver=source[start:match.start()]+ 'Damage'
        source=source[:start]+'Damage_'+match[1]+'(&'+receiver+('' if source[match.end():].lstrip().startswith(')') else ', ')+source[match.end():]
    marker='CAutomobile::CAutomobile(int32 id, uint8 CreatedBy)\n : CVehicle(CreatedBy)\n{'
    if marker in source and INIT not in source:source=source.replace(marker,marker+INIT,1)
    if source==original:return source
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def damage_project(source):
    # Retain the full producer rename and explicit C11 metadata in historical native projects
    source=source.replace('..\\src\\vehicles\\DamageManager.cpp','..\\src\\vehicles\\DamageManager.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\vehicles\\DamageManager\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve unrelated options while requiring the relocated producer to compile as C11
            body=match[1] or '\n    '
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body=body.rstrip()+'\n      <'+tag+'>'+value+'</'+tag+'>\n    '
            return '<ClCompile Include="..\\src\\vehicles\\DamageManager.c">'+body+'</ClCompile>'
        source=re.sub(pattern,metadata,source)
    return objects.project(source)


def calls(source):
    # Preserve complete damage ownership before moving the full pedestrian route table to C
    import ped_routes_migration as routes
    return routes.calls(damage_calls(source))

def project(source):
    # Retain the damage producer rename before the complete C11 route producer
    import ped_routes_migration as routes
    return routes.project(damage_project(source))
