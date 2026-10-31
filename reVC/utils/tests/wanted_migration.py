"""Relocate the complete wanted owner and queued crimes while preserving live vector references."""
import re
import object_names_migration as objects
from events_migration import arguments,body as event_body
OBSERVER=None
OPERATIONS=['Initialise','AreMiamiViceRequired','AreSwatRequired','AreFbiRequired','AreArmyRequired','NumOfHelisRequired','SetWantedLevel','SetWantedLevelNoDrop','GetWantedLevel','CheatWantedLevel','RegisterCrime','RegisterCrime_Immediately','ClearQdCrimes','AddCrimeToQ','ReportCrimeNow','UpdateWantedLevel','Reset','ResetPolicePursuit','UpdateCrimesQ','Update','Suspend','IsIgnored','WorkOutPolicePresence','SetMaximumWantedLevel']
STATIC={'WorkOutPolicePresence','SetMaximumWantedLevel'}
REF_ARGUMENTS={'RegisterCrime':1,'RegisterCrime_Immediately':1,'AddCrimeToQ':2,'ReportCrimeNow':1}
SHARED='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "AudioCrimes.h"
#ifdef __cplusplus
class CVector;class CPed;class CVehicle;class CCopPed;
#else
typedef struct CVector CVector;typedef struct CPed CPed;typedef struct CVehicle CVehicle;typedef struct CCopPed CCopPed;
#endif
typedef AudioCrimePosition WantedVector;
typedef struct WantedPositionRef {const WantedVector *plain;const CVector *native;} WantedPositionRef;
#ifdef __cplusplus
WantedVector Wanted_VectorFromNative(const CVector &value);
WantedPositionRef Wanted_ReferenceNative(const CVector &value);
#endif
//- rouz edit (ChatGPT)
'''
CRIME_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "WantedShared.h"
#include "CrimeTypes.h"
typedef struct CCrimeBeingQd {
    enum eCrimeType m_nType;
    int32_t m_nId;
    uint32_t m_nTime;
    WantedVector m_vecPosn;
    bool m_bReported;
    bool m_bPoliceDoesntCare;
} CCrimeBeingQd;
//- rouz edit (ChatGPT)
'''
SIGNATURES=[
('void','Initialise','CWanted *self'),('bool','AreMiamiViceRequired','CWanted *self'),('bool','AreSwatRequired','CWanted *self'),('bool','AreFbiRequired','CWanted *self'),('bool','AreArmyRequired','CWanted *self'),('int32_t','NumOfHelisRequired','CWanted *self'),('void','SetWantedLevel','CWanted *self,int32_t level'),('void','SetWantedLevelNoDrop','CWanted *self,int32_t level'),('int32_t','GetWantedLevel','CWanted *self'),('void','CheatWantedLevel','CWanted *self,int32_t level'),('void','RegisterCrime','CWanted *self,enum eCrimeType type,WantedPositionRef coors,uint32_t id,bool policeDoesntCare'),('void','RegisterCrime_Immediately','CWanted *self,enum eCrimeType type,WantedPositionRef coors,uint32_t id,bool policeDoesntCare'),('void','ClearQdCrimes','CWanted *self'),('bool','AddCrimeToQ','CWanted *self,enum eCrimeType type,int32_t id,WantedPositionRef coors,bool reported,bool policeDoesntCare'),('void','ReportCrimeNow','CWanted *self,enum eCrimeType type,WantedPositionRef coors,bool policeDoesntCare'),('void','UpdateWantedLevel','CWanted *self'),('void','Reset','CWanted *self'),('void','ResetPolicePursuit','CWanted *self'),('void','UpdateCrimesQ','CWanted *self'),('void','Update','CWanted *self'),('void','Suspend','CWanted *self'),('bool','IsIgnored','CWanted *self'),('int32_t','WorkOutPolicePresence','WantedVector posn,float radius'),('void','SetMaximumWantedLevel','int32_t level'),('void','ConstructValue','CWanted *self'),('void','Copy','CWanted *self,const CWanted *source')]

def scalar(source):
    # Keep all original integer widths and enum tags explicit in the C translation
    for old,new in [('int32','int32_t'),('uint32','uint32_t'),('int16','int16_t'),('uint8','uint8_t'),('nil','NULL')]:source=re.sub(r'\b'+old+r'\b',new,source)
    return source

def fields(header):
    # Preserve the complete original nonstatic representation including the five native bit fields
    return header[header.index('\tint32 m_nChaos;'):header.index('\n\tstatic int32 MaximumWantedLevel;')]

def header(original):
    # Expose the original complete wanted representation and every operation as plain C
    return '//+ rouz edit (ChatGPT)\n#pragma once\n#include "Crime.h"\n#include "Validate.h"\ntypedef struct CWanted {\n'+scalar(fields(original))+'\n} CWanted;\n#ifdef __cplusplus\nextern "C" {\n#endif\nextern int32_t Wanted_MaximumWantedLevel,Wanted_nMaximumWantedLevel;\n'+''.join(ret+' Wanted_'+name+'('+args+');\n' for ret,name,args in SIGNATURES)+'#ifdef __cplusplus\n}\n#endif\n#ifdef CHECK_STRUCT_SIZES\nVALIDATE_SIZE(CWanted,0x204);\n#endif\n//- rouz edit (ChatGPT)\n'

LEAVES=[
('bool','Frenzy','void','return CDarkel::FrenzyOnGoing();'),
('int32_t *','Attained','void','return &CStats::WantedStarsAttained;'),('int32_t *','Evaded','void','return &CStats::WantedStarsEvaded;'),
('CPool *','PedPool','void','return CPools::GetPedPool();'),('CPool *','VehiclePool','void','return CPools::GetVehiclePool();'),
('int16_t','PedModel','CPed *ped','return ped->GetModelIndex();'),('int16_t','VehicleModel','CVehicle *vehicle','return vehicle->GetModelIndex();'),
('WantedVector','PedPosition','CPed *ped','const CVector &v=ped->GetPosition();WantedVector result={v.x,v.y,v.z};return result;'),
('WantedVector','VehiclePosition','CVehicle *vehicle','const CVector &v=vehicle->GetPosition();WantedVector result={v.x,v.y,v.z};return result;'),
('bool','LawEnforcer','CVehicle *vehicle','return vehicle->bIsLawEnforcer;'),('uint8_t','VehicleStatus','CVehicle *vehicle','return vehicle->GetStatus();'),
('CVehicle *','PlayerVehicle','void','return FindPlayerVehicle();'),
('WantedVector','PlayerCoors','void','CVector v=FindPlayerCoors();WantedVector result={v.x,v.y,v.z};return result;'),
('void','ClearCopState','CCopPed *cop','cop->m_bIsInPursuit=false;cop->m_objective=OBJECTIVE_NONE;cop->m_prevObjective=OBJECTIVE_NONE;cop->m_nLastPedState=PED_NONE;'),
('bool','CopDying','CCopPed *cop','return cop->DyingOrDead();'),('void','CopWander','CCopPed *cop,float path','cop->SetWanderPath(path);'),
('WantedVector','NativePosition','const CVector *value','WantedVector result={value->x,value->y,value->z};return result;'),
('void','Error','const char *message','Error("%s",message);')]
BACKEND='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Wanted.h"\n#include "Pool.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(ret+' Wanted_Backend'+name+'('+args+');\n' for ret,name,args,code in LEAVES)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def host():
    # Retain only typed accesses to owners whose implementation still uses native C++ records
    return '''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Wanted_backend.h"
#include "Pools.h"
#include "CopPed.h"
#include "Darkel.h"
#include "Stats.h"
#include "main.h"
#include "World.h"
WantedVector Wanted_VectorFromNative(const CVector &value)
{
    // Copy the original by-value police-presence input at the native caller boundary
    WantedVector result={value.x,value.y,value.z};return result;
}
WantedPositionRef Wanted_ReferenceNative(const CVector &value)
{
    // Retain the actual native coordinate object until its original later read point
    WantedPositionRef result={NULL,&value};return result;
}
'''+ '\n'.join(ret+' Wanted_Backend'+name+'('+args+')\n{\n    // Access only the original typed native primitive at its original policy access point\n    '+code+'\n}\n' for ret,name,args,code in LEAVES)+'//- rouz edit (ChatGPT)\n'

def kernel(source,oldheader):
    # Relocate every complete wanted operation while retaining all original branches and timing expressions
    memberFields=re.findall(r'\b(m_\w+)\s*(?:\[\d+\])?\s*(?::\s*1\s*)?;',fields(oldheader));assert len(memberFields)==20,len(memberFields)
    chunks=[]
    for ret,name,args in SIGNATURES[:24]:
        if name in ['GetWantedLevel','IsIgnored']:
            expression='m_nWantedLevel' if name=='GetWantedLevel' else 'm_bIgnoredByCops || m_bIgnoredByEveryone'
            text=ret+' Wanted_'+name+'('+args+')\n{\n\treturn '+expression+';\n}'
        else:
            text=event_body(source,'CWanted',name+'(');assert re.search(r'CWanted::'+name+r'\(',text)
            opening=text.index('\n{');text=ret+' Wanted_'+name+'('+args+')'+text[opening:]
        for field in memberFields:text=re.sub(r'\b'+field+r'\b','self->'+field,text)
        text=re.sub(r'\bnMaximumWantedLevel\b','Wanted_nMaximumWantedLevel',text);text=re.sub(r'\bMaximumWantedLevel\b','Wanted_MaximumWantedLevel',text)
        for called in OPERATIONS:
            if called in STATIC:continue
            text=re.sub(r'(?<![\w])'+called+r'\(', 'Wanted_'+called+'(self,',text)
        text=text.replace('(self,)','(self)')
        text=text.replace('WorkOutPolicePresence(playerPos, radius)','Wanted_WorkOutPolicePresence(playerPos, radius)')
        text=text.replace('CCrimeBeingQd &crime = self->m_aCrimes[i];','CCrimeBeingQd *crime = &self->m_aCrimes[i];').replace('crime.','crime->')
        text=text.replace('Wanted_ReportCrimeNow(self,crime->m_nType, crime->m_vecPosn,','Wanted_ReportCrimeNow(self,crime->m_nType, Wanted_ReferencePlain(&crime->m_vecPosn),')
        text=text.replace('self->m_aCrimes[i].m_vecPosn = coors;','self->m_aCrimes[i].m_vecPosn = Wanted_ReadPosition(coors);')
        text=text.replace('CDarkel::FrenzyOnGoing()','Wanted_BackendFrenzy()').replace('CStats::WantedStarsAttained','(*Wanted_BackendAttained())').replace('CStats::WantedStarsEvaded','(*Wanted_BackendEvaded())')
        text=text.replace('DMAudio_ReportCrime(type, &coors);','Wanted_ReportAudio(type,coors);').replace('Error(','Wanted_BackendError(')
        text=text.replace('CPools::GetPedPool()','Wanted_BackendPedPool()').replace('CPools::GetVehiclePool()','Wanted_BackendVehiclePool()')
        text=text.replace('ped->GetModelIndex()','Wanted_BackendPedModel(ped)').replace('vehicle->GetModelIndex()','Wanted_BackendVehicleModel(vehicle)').replace('vehicle->bIsLawEnforcer','Wanted_BackendLawEnforcer(vehicle)').replace('vehicle->GetStatus()','Wanted_BackendVehicleStatus(vehicle)').replace('FindPlayerVehicle()','Wanted_BackendPlayerVehicle()')
        text=text.replace('(posn - ped->GetPosition()).Magnitude()','Wanted_Magnitude(Wanted_Sub(posn,Wanted_BackendPedPosition(ped)))').replace('(posn - vehicle->GetPosition()).Magnitude()','Wanted_Magnitude(Wanted_Sub(posn,Wanted_BackendVehiclePosition(vehicle)))').replace('CVector playerPos = FindPlayerCoors();','WantedVector playerPos = Wanted_BackendPlayerCoors();')
        text=text.replace('cop->m_bIsInPursuit = false;\n\t\tcop->m_objective = OBJECTIVE_NONE;\n\t\tcop->m_prevObjective = OBJECTIVE_NONE;\n\t\tcop->m_nLastPedState = PED_NONE;','Wanted_BackendClearCopState(cop);').replace('cop->DyingOrDead()','Wanted_BackendCopDying(cop)').replace('cop->SetWanderPath(','Wanted_BackendCopWander(cop,')
        text=scalar(text).replace('\n{','\n{\n    // Preserve the complete original '+name+' operation and every ordered external access',1);chunks.append(text)
    return '''//+ rouz edit (ChatGPT)
#include "Wanted_backend.h"
#include "Timer.h"
#include "ZoneCull.h"
#include "General.h"
#include "ModelIndices.h"
#include "EntityStatus.h"
#include "DMAudio.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define ARRAY_SIZE(value) (sizeof(value)/sizeof((value)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
int32_t Wanted_MaximumWantedLevel=6;
int32_t Wanted_nMaximumWantedLevel=9600;
static WantedVector Wanted_Sub(WantedVector a,WantedVector b)
{
    // Preserve all three original vector subtractions before the distance calculation
    WantedVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static float Wanted_Magnitude(WantedVector value)
{
    // Preserve the original ordered squared length and single-precision square root
    return sqrtf(value.x*value.x+value.y*value.y+value.z*value.z);
}
static WantedPositionRef Wanted_ReferencePlain(const WantedVector *value)
{
    // Retain a canonical queued-crime coordinate until its original later read point
    WantedPositionRef result={value,NULL};return result;
}
static WantedVector Wanted_ReadPosition(WantedPositionRef value)
{
    // Read either actual native coordinates or the canonical C record at the original copy statement
    return value.native?Wanted_BackendNativePosition(value.native):*value.plain;
}
static void Wanted_ReportAudio(enum eCrimeType type,WantedPositionRef position)
{
    // Preserve deferred audio eligibility checks before reading either live coordinate representation
    if(position.native)DMAudio_ReportCrime(type,position.native);
    else DMAudio_ReportCrimeAt(type,position.plain);
}
void Wanted_ConstructValue(CWanted *self)
{
    // Preserve the original allocator value-initialization before explicit gameplay initialization
    static const CWanted zero;memcpy(self,&zero,sizeof(*self));
}
void Wanted_Copy(CWanted *self,const CWanted *source)
{
    // Copy every original named record field and pointer for replay restoration
    *self=*source;
}
'''+ '\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def audio_entries(source):
    # Share the exact original audio eligibility policy while permitting a deferred C coordinate read
    source=source.replace('#include <stddef.h>','#include <stddef.h>\n#include <stdbool.h>')
    original='''\tif (manager->m_bIsInitialised && AudioPoliceHost_MusicMode() != MUSICMODE_CUTSCENE && AudioPoliceHost_WantedLevel(AudioPoliceHost_FindPlayer()) > 0 &&
\t\t(type > CRIME_NONE || type < NUM_CRIME_TYPES) && manager->m_FrameCounter >= gMinTimeToNextReport[type]) {'''
    assert original in source
    position=source.index('void\nAudioPolice_ReportCrime(')
    helper='''static bool
AudioPolice_CanReportCrime(cAudioManager *manager,enum eCrimeType type)
{
    // Preserve every original short-circuited eligibility and cooldown read before reading coordinates
    return manager->m_bIsInitialised && AudioPoliceHost_MusicMode() != MUSICMODE_CUTSCENE && AudioPoliceHost_WantedLevel(AudioPoliceHost_FindPlayer()) > 0 &&
        (type > CRIME_NONE || type < NUM_CRIME_TYPES) && manager->m_FrameCounter >= gMinTimeToNextReport[type];
}

'''
    source=source[:position]+helper+source[position:];source=source.replace(original,'\tif (AudioPolice_CanReportCrime(manager,type)) {')
    addition='''void
DMAudio_ReportCrimeAt(enum eCrimeType crime,const AudioCrimePosition *position)
{
    // Read the live canonical C position only after the original audio eligibility callbacks
    if(AudioPolice_CanReportCrime(&AudioManager,crime))
        AudioCrimes_Report(AudioManager.m_aCrimes,gMinTimeToNextReport,crime,AudioManager.m_FrameCounter,position->x,position->y,position->z);
}
'''
    return source.replace('//- rouz edit (ChatGPT)',addition+'//- rouz edit (ChatGPT)')

def atom_start(source,end):
    # Recover a complete postfix receiver including nested casts, function calls, subscripts and member chains
    i=end
    while i>0 and source[i-1].isspace():i-=1
    if source[i-1] in ')]':
        close=source[i-1];opening='(' if close==')' else '[';depth=1;i-=1
        while depth:
            i-=1
            if source[i]==close:depth+=1
            elif source[i]==opening:depth-=1
        if i and (source[i-1].isalnum() or source[i-1]=='_'):i=atom_start(source,i)
    else:
        while i and (source[i-1].isalnum() or source[i-1]=='_'):i-=1
    while i:
        j=i
        while j and source[j-1].isspace():j-=1
        size=2 if source[max(0,j-2):j] in ['->','::'] else 1 if source[j-1:j]=='.' else 0
        if not size:break
        i=atom_start(source,j-size)
    return i

def wanted_calls(source):
    # Remove both complete classes and adapt only receivers proven to have the original wanted type
    if 'class CWanted\n' in source:return header(source)
    if 'class CCrimeBeingQd\n' in source:return CRIME_HEADER
    if 'int32 CWanted::MaximumWantedLevel = 6;' in source:
        if OBSERVER:OBSERVER(source)
        return host()
    if 'AudioPolice_ReportCrime(cAudioManager *manager, enum eCrimeType type, const CVector *position)\n{' in source:return audio_entries(source)
    if 'void DMAudio_ReportCrime(enum eCrimeType crime, const CVector *pos);' in source:
        source=source.replace('#include "../core/CrimeTypes.h"','#include "../core/CrimeTypes.h"\n#include "AudioCrimes.h"').replace('void DMAudio_ReportCrime(enum eCrimeType crime, const CVector *pos);','void DMAudio_ReportCrime(enum eCrimeType crime, const CVector *pos);\n\tvoid DMAudio_ReportCrimeAt(enum eCrimeType crime,const AudioCrimePosition *pos);')
    original=source
    known=set(re.findall(r'CWanted\s*\*\s*(\w+)',source))|{'m_pWanted'}
    mask=re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"',lambda match:''.join('\n' if char=='\n' else ' ' for char in match[0]),source)
    matches=list(re.finditer(r'(?:->|CWanted::)('+ '|'.join(OPERATIONS)+r')\s*\(',mask))
    for match in reversed(matches):
        end,args=arguments(source,match.end()-1);name=match[1]
        if match[0].startswith('CWanted::'):start=match.start();receiver=None
        else:
            start=atom_start(source,match.start());receiver=source[start:match.start()].strip()
            tail=re.search(r'(\w+)\s*$',receiver)
            if not tail or tail[1] not in known:continue
        if name in REF_ARGUMENTS:args[REF_ARGUMENTS[name]]='Wanted_ReferenceNative('+args[REF_ARGUMENTS[name]]+')'
        if name=='WorkOutPolicePresence':args[0]='Wanted_VectorFromNative('+args[0]+')'
        if receiver is not None and name not in STATIC:args.insert(0,receiver)
        replacement='Wanted_'+name+'('+','.join(args)+')'
        if receiver is not None and name in STATIC:replacement='((void)('+receiver+'),'+replacement+')'
        source=source[:start]+replacement+source[end:]
    source=source.replace('CWanted::MaximumWantedLevel','Wanted_MaximumWantedLevel').replace('CWanted::nMaximumWantedLevel','Wanted_nMaximumWantedLevel').replace('class CWanted;','struct CWanted;')
    source=source.replace('std::allocator<CWanted>().construct(m_pWanted);','Wanted_ConstructValue(m_pWanted);').replace('std::allocator<CWanted>().construct(FindPlayerPed()->m_pWanted, PlayerWanted);','Wanted_Copy(FindPlayerPed()->m_pWanted,&PlayerWanted);').replace('\t\tm_pWanted->~CWanted();\n','')
    source=source.replace('// Destroy and release the wanted data without invoking C++ delete.','// Release the plain wanted record through the original allocator').replace('// Allocate wanted data without invoking C++ new.','// Allocate and value-initialize the plain wanted record through the original allocator').replace('// Allocate the restored wanted state without invoking C++ new.','// Allocate and copy the complete plain wanted record for replay restoration')
    if source==original:return source
    if original.lstrip().startswith('//+ rouz edit (ChatGPT)') and original.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def wanted_project(source):
    # Register one unique C policy object while preserving all prior object disambiguation
    for tag,old,new in [('ClCompile','Wanted.cpp','WantedSystem.c'),('ClInclude','Wanted.h','Wanted_backend.h'),('ClInclude','Crime.h','WantedShared.h')]:
        entry='<'+tag+' Include="..\\src\\core\\'+old+'" />';addition='<'+tag+' Include="..\\src\\core\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\core\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\core\\'+re.escape(old)+r'">\n\s*<Filter>core</Filter>\n\s*</'+tag+'>)';source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove complete relocation of a historical wanted producer and all original policy statements
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/core/Wanted.cpp').read_text())
    original=(root/'.diagnostics/wanted-c-tests/before/src/core/Wanted.h').read_text()
    assert normalize(kernel(captured[0],original))==normalize((root/'src/core/WantedSystem.c').read_text())

def verify_dependency(saved,current):
    # Keep complete dependency byte guards unless the entire file follows this exact wanted conversion
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original);assert expected!=original and expected==current.read_text(),str(current)


def calls(source):
    # Retain the complete wanted and queued-crime conversion before removing the complete rampage class
    import darkel_migration as darkel
    return darkel.calls(wanted_calls(source))

def project(source):
    # Preserve every earlier registration before adding the unique rampage-system C object
    import darkel_migration as darkel
    return darkel.project(wanted_project(source))
