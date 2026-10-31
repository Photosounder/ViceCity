"""Move weapon metadata, tables, parsing and all record/static operations into C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
METHODS=['GetWeaponInfo','Initialise','LoadWeaponData','FindWeaponType','FindWeaponFireType','Shutdown']
OBSERVER=None
FIELDS=['m_eWeaponFire','m_fRange','m_nFiringRate','m_nReload','m_nAmountofAmmunition','m_nDamage','m_fSpeed','m_fRadius','m_fLifespan','m_fSpread','m_vecFireOffset','m_AnimToPlay','m_fAnimLoopStart','m_fAnimLoopEnd','m_fAnimFrameFire','m_fAnim2LoopStart','m_fAnim2LoopEnd','m_fAnim2FrameFire','m_fAnimBreakout','m_nModelId','m_nModel2Id','m_Flags','m_nWeaponSlot']

def groups(source):
    # Share the complete original conditional animation-group enum without changing values or order
    return re.search(r'enum AssocGroupId\n\{[\s\S]*?\n\};',source)[0]

def groups_header(source):
    # Supply the same canonical enum to both native animation owners and the C metadata record
    return '//+ rouz edit (ChatGPT)\n#pragma once\n#include "config.h"\n'+groups(source)+'\n//- rouz edit (ChatGPT)\n'

def header(source):
    # Preserve every record member and flag while replacing methods with explicit C operations
    flags=re.search(r'enum\n\{[\s\S]*?\n\};',source)[0]
    fields=source[source.index('\teWeaponFire m_eWeaponFire;'):source.index('\n\tstatic void Initialise')]
    for old,new in [('eWeaponFire','enum eWeaponFire'),('AssocGroupId','enum AssocGroupId'),('CVector','WeaponInfoVector'),('uint32','uint32_t'),('int32','int32_t')]:fields=re.sub(r'\b'+old+r'\b',new,fields)
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AnimGroups.h"
#include "WeaponType.h"
#ifdef __cplusplus
#include "AnimManager.h"
#include "AnimationId.h"
#endif
'''+flags+'''
typedef struct WeaponInfoVector {float x,y,z;} WeaponInfoVector;
typedef struct CWeaponInfo {
'''+fields+'''
} CWeaponInfo;
#ifdef __cplusplus
extern "C" {
#endif
extern uint16_t WeaponInfo_ms_aReloadSampleTime[WEAPONTYPE_TOTALWEAPONS];
extern int32_t WeaponInfo_ms_aMaxAmmoForWeapon[WEAPONTYPE_TOTALWEAPONS];
void WeaponInfo_Initialise(void);
void WeaponInfo_LoadWeaponData(void);
CWeaponInfo *WeaponInfo_GetWeaponInfo(enum eWeaponType weapon);
enum eWeaponFire WeaponInfo_FindWeaponFireType(char *name);
enum eWeaponType WeaponInfo_FindWeaponType(char *name);
void WeaponInfo_Shutdown(void);
bool WeaponInfo_IsWeaponSlotAmmoMergeable(uint32_t slot);
bool WeaponInfo_IsFlagSet(const CWeaponInfo *info,uint32_t flag);
#ifdef __cplusplus
}
class CVector;
CVector WeaponInfo_VectorToNative(WeaponInfoVector value);
#endif
//- rouz edit (ChatGPT)
'''

BOUNDARIES=[
('WeaponInfoWorkBuffer *','Buffer','void','return &work_buff;'),
('bool','IsShotgun','int32_t weapon','return CWeapon::IsShotgun(weapon);'),
('void','SetModelWeapon','int32_t model,int32_t weapon','((CWeaponModelInfo *)CModelInfo::GetModelInfo(model))->SetWeaponInfo(weapon);'),
('const char *','AnimGroupName','enum AssocGroupId group','return CAnimManager::GetAnimGroupName(group);'),
('void','Initialising','void','debug("Initialising CWeaponInfo...\\n");'),
('void','Loading','void','debug("Loading weapon data...\\n");'),
('void','Ready','void','debug("CWeaponInfo ready\\n");'),
('void','ShuttingDown','void','debug("Shutting down CWeaponInfo...\\n");'),
('void','ShutDown','void','debug("CWeaponInfo shut down\\n");'),
('void','UnknownFireType','void','Error("Unknown weapon fire type, WeaponInfo.cpp");')]
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "WeaponInfo.h"
typedef uint8_t WeaponInfoWorkBuffer[55000];
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(kind+' WeaponInfo_Backend'+name+'('+args+');\n' for kind,name,args,body in BOUNDARIES)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "WeaponInfo_backend.h"
#include "main.h"
#include "AnimManager.h"
#include "Weapon.h"
#include "ModelInfo.h"
CVector WeaponInfo_VectorToNative(WeaponInfoVector value)
{
    // Supply the original native vector value to each remaining native consumer
    return CVector(value.x,value.y,value.z);
}
'''+''.join(kind+' WeaponInfo_Backend'+name+'('+args+')\n{\n    // Enter the original typed native operation or exact diagnostic call\n    '+body+'\n}\n' for kind,name,args,body in BOUNDARIES)+'//- rouz edit (ChatGPT)\n'

def kernel(source,oldheader):
    # Preserve all original initializers and complete loader/lifecycle bodies before outlining both accessors
    start=source.index('uint16 CWeaponInfo::ms_aReloadSampleTime');end=source.index('CWeaponInfo*\nCWeaponInfo::GetWeaponInfo')
    initializers=source[start:end].replace('CWeaponInfo::ms_aWeaponNames','WeaponInfo_names').replace('CWeaponInfo::','WeaponInfo_')
    initializers=initializers.replace('char WeaponInfo_names','static char WeaponInfo_names').replace('uint16 ','uint16_t ').replace('int32 ','int32_t ')
    chunks=[]
    for name in METHODS:
        text=function(source,'CWeaponInfo::',name).replace('CWeaponInfo::','WeaponInfo_')
        text=text.replace('CVector vecFireOffset(fireOffsetX, fireOffsetY, fireOffsetZ);','WeaponInfoVector vecFireOffset={fireOffsetX,fireOffsetY,fireOffsetZ};')
        text=text.replace('CVector(0.0f, 0.0f, 0.0f)','(WeaponInfoVector){0.0f,0.0f,0.0f}')
        for operation in METHODS:text=re.sub(r'(?<![\w])'+operation+r'\(', 'WeaponInfo_'+operation+'(',text)
        text=text.replace('ms_aWeaponNames','WeaponInfo_names').replace('static_cast<eWeaponType>(i)','(enum eWeaponType)i')
        text=text.replace('CWeapon::IsShotgun(','WeaponInfo_BackendIsShotgun(')
        text=text.replace('((CWeaponModelInfo*)CModelInfo::GetModelInfo(modelId))->SetWeaponInfo(weaponType)','WeaponInfo_BackendSetModelWeapon(modelId,weaponType)')
        text=text.replace('CAnimManager::GetAnimGroupName(','WeaponInfo_BackendAnimGroupName(')
        for old,new in [('eWeaponType','enum eWeaponType'),('eWeaponFire','enum eWeaponFire'),('AssocGroupId','enum AssocGroupId')]:text=re.sub(r'(?<!enum )\b'+old+r'\b',new,text)
        for key,message in [('Initialising','Initialising CWeaponInfo...\\n'),('Loading','Loading weapon data...\\n'),('Ready','CWeaponInfo ready\\n'),('ShuttingDown','Shutting down CWeaponInfo...\\n'),('ShutDown','CWeaponInfo shut down\\n')]:text=text.replace('debug("'+message+'");','WeaponInfo_Backend'+key+'();')
        text=text.replace('Error("Unknown weapon fire type, WeaponInfo.cpp");','WeaponInfo_BackendUnknownFireType();')
        if name=='LoadWeaponData':
            text=text.replace('sizeof(work_buff)','sizeof(*buffer)').replace('work_buff[bp]','(*buffer)[bp]').replace('"WEAPON.DAT", work_buff,','"WEAPON.DAT", *buffer,')
            text=text.replace('\n{','\n{\n    WeaponInfoWorkBuffer *buffer=WeaponInfo_BackendBuffer();',1)
        text=text.replace('\n{','\n{\n    // Preserve the complete original '+name+' operation and ordered live accesses',1)
        chunks.append(text)
    inline=re.search(r'static bool IsWeaponSlotAmmoMergeable\(uint32 slot\) \{([^}]+)\}',oldheader)[1]
    flag=re.search(r'bool IsFlagSet\(uint32 flag\) const \{([^}]+)\}',oldheader)[1].replace('m_Flags','info->m_Flags')
    return '''//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "WeaponInfo_backend.h"
#include "FileMgr.h"
#include "ModelIndexData.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
_Static_assert(sizeof(CWeaponInfo)==100,"Original complete weapon metadata record");
'''+initializers+'\n\n'.join(chunks)+'''
bool WeaponInfo_IsWeaponSlotAmmoMergeable(uint32_t slot)
{
    // Preserve the complete original static inline slot predicate
'''+inline+'''
}
bool WeaponInfo_IsFlagSet(const CWeaponInfo *info,uint32_t flag)
{
    // Preserve the complete original const record predicate with an explicit record pointer
'''+flag+'''
}
//- rouz edit (ChatGPT)
'''

def arguments(source,opening):
    # Keep nested expressions together while replacing each complete actual member call
    depth=1;i=opening+1;start=i;parts=[]
    while depth:
        if source[i] in '([{':depth+=1
        elif source[i] in ')]}':depth-=1
        if source[i]==',' and depth==1:parts.append(source[start:i]);start=i+1
        i+=1
    parts.append(source[start:i-1]);return i,parts

def receiver_start(source,end):
    # Capture each actual variable or nested metadata lookup once as the explicit C receiver
    i=end-1;depth=0
    while i>=0:
        value=source[i]
        if value in ')]':depth+=1
        elif value in '([':
            if depth:depth-=1
            else:break
        elif not depth and not (value.isalnum() or value in '_:'):break
        i-=1
    return i+1

def weapon_calls(source):
    # Remove methods while retaining the record name, exact caller expressions and native vector values
    if 'class CWeaponInfo {' in source:return header(source)
    if re.search(r'(?m)^CWeaponInfo aWeaponInfo\[',source):
        if OBSERVER:OBSERVER(source)
        return HOST
    old=source
    if 'class CAnimManager\n' in source and 'enum AssocGroupId\n' in source:source=source.replace(groups(source),'#include "AnimGroups.h"')
    source=source.replace('class CWeaponInfo;','struct CWeaponInfo;').replace('CWeaponInfo::','WeaponInfo_')
    changes=[]
    for match in re.finditer(r'->IsFlagSet\(',source):
        start=receiver_start(source,match.start());end,args=arguments(source,match.end()-1);assert len(args)==1
        changes.append((start,end,'WeaponInfo_IsFlagSet('+source[start:match.start()]+','+args[0]+')'))
    for start,end,text in reversed(changes):source=source[:start]+text+source[end:]
    source=re.sub(r'(?<!WeaponInfo_VectorToNative\()\b(info|weaponInfo|ourWeapon)->m_vecFireOffset\b(?!\.)',lambda match:'WeaponInfo_VectorToNative('+match[0]+')',source)
    if source==old:return source
    if old.lstrip().startswith('//+ rouz edit (ChatGPT)') and old.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def weapon_project(source):
    # Register uniquely named C metadata storage and its shared C enum beside typed native operations
    for tag,directory,old,new in [('ClCompile','weapons','WeaponInfo.cpp','WeaponData.c'),('ClInclude','weapons','WeaponInfo.h','WeaponInfo_backend.h'),('ClInclude','animation','AnimManager.h','AnimGroups.h')]:
        entry='<'+tag+' Include="..\\src\\'+directory+'\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\'+directory+'\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\'+directory+'\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\'+directory+r'\\'+re.escape(old)+r'">\n\s*<Filter>'+directory+r'</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove the complete relocated loader and native bindings from earlier immutable sources
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/weapons/WeaponInfo.cpp').read_text())
    original=(root/'build/weapon-info-c-tests/before/src/weapons/WeaponInfo.h').read_text()
    assert normalize(kernel(captured[0],original))==normalize((root/'src/weapons/WeaponData.c').read_text())

def verify_dependency(saved,current):
    # Keep byte guards unless the whole dependency follows this exact class and enum conversion
    if saved.read_bytes()==current.read_bytes():return
    old=saved.read_text();expected=calls(old)
    assert expected!=old and expected==current.read_text(),str(current)


def calls(source):
    # Forward historical complete sources through the verified on-screen display conversion
    import onscreen_timer_migration as onscreen
    return onscreen.calls(weapon_calls(source))

def project(source):
    # Preserve all earlier C units before registering the on-screen controller
    import onscreen_timer_migration as onscreen
    return onscreen.project(weapon_project(source))
