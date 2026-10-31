"""Move the complete event list and mutually dependent accident manager into C11."""
import re
import object_names_migration as objects
OBSERVER=None
EVENT_OPERATIONS=['Initialise','Update','RegisterEntity','RegisterPosition','GetEvent','ClearEvent','FindClosestEvent','ReportCrimeForEvent']
ACCIDENT_OPERATIONS=['GetNextFreeAccident','ReportAccident','Update','FindNearestAccident','CountActiveAccidents','WorkToDoForMedics','UnattendedAccidents']
SHARED='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CEntity;class CPed;
#else
typedef struct CEntity CEntity;typedef struct CPed CPed;
#endif
typedef struct EventsVector {float x,y,z;} EventsVector;
#ifdef __cplusplus
class CVector;
EventsVector Events_VectorFromNative(const CVector &value);
#endif
//- rouz edit (ChatGPT)
'''
def event_header(original):
    # Preserve both complete event enums and the exact pointer/scalar/vector record order
    enums=[]
    for name in ['eEventType','eEventEntity']:
        text=re.search(r'enum '+name+r'\s*\{[\s\S]*?\n\};',original)[0];enums.append(text.replace('enum '+name,'typedef enum '+name,1)[:-2]+'} '+name+';')
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "EventsShared.h"
'''+ '\n'.join(enums)+'''
typedef struct CEvent {eEventType type;eEventEntity entityType;int32_t entityRef;CPed *criminal;EventsVector posn;uint32_t timeout;int32_t state;} CEvent;
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t Events_FirstFreeSlotIndex;
extern CEvent gaEvent[NUMEVENTS];
void Events_Initialise(void);
void Events_Update(void);
void Events_RegisterEntity(eEventType type,eEventEntity entityType,CEntity *entity,CPed *criminal,int32_t timeout);
void Events_RegisterPosition(eEventType type,EventsVector position,int32_t timeout);
bool Events_GetEvent(eEventType type,int32_t *event);
void Events_ClearEvent(int32_t event);
bool Events_FindClosestEvent(eEventType type,EventsVector position,int32_t *event);
void Events_ReportCrimeForEvent(eEventType type,intptr_t crimeId,bool copsDontCare);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
ACCIDENT_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "EventsShared.h"
typedef struct CAccident {CPed *m_pVictim;uint32_t m_nMedicsAttending;uint32_t m_nMedicsPerformingCPR;} CAccident;
typedef struct CAccidentManager {CAccident m_aAccidents[NUM_ACCIDENTS];} CAccidentManager;
enum {ACCIDENT_MAX_MEDICS_TO_ATTEND=2};
#ifdef __cplusplus
extern "C" {
#endif
extern CAccidentManager gAccidentManager;
void Accident_Init(CAccident *self);
void AccidentManager_Init(CAccidentManager *self);
CAccident *AccidentManager_GetNextFreeAccident(CAccidentManager *self);
void AccidentManager_ReportAccident(CAccidentManager *self,CPed *ped);
void AccidentManager_Update(CAccidentManager *self);
CAccident *AccidentManager_FindNearestAccident(CAccidentManager *self,EventsVector position,float *distance);
uint16_t AccidentManager_CountActiveAccidents(CAccidentManager *self);
bool AccidentManager_WorkToDoForMedics(CAccidentManager *self);
bool AccidentManager_UnattendedAccidents(CAccidentManager *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('void','Debug','const char *message','debug("%s",message);'),
('void','Error','const char *message','Error("%s",message);'),
('int32_t','PedRef','CEntity *entity','return CPools::GetPedRef((CPed*)entity);'),
('int32_t','VehicleRef','CEntity *entity','return CPools::GetVehicleRef((CVehicle*)entity);'),
('int32_t','ObjectRef','CEntity *entity','return CPools::GetObjectRef((CObject*)entity);'),
('CPed *','PedFromEntity','CEntity *entity','return (CPed*)entity;'),
('EventsVector','EntityPosition','CEntity *entity','const CVector &v=entity->GetPosition();EventsVector result={v.x,v.y,v.z};return result;'),
('void','RegisterReference','CPed *ped,CPed **slot','ped->RegisterReference((CEntity**)slot);'),
('CPed *','Player','void','return FindPlayerPed();'),
('bool','PedValid','CPed *ped','return IsPedPointerValid(ped);'),
('int32_t','WantedLevel','void','return FindPlayerPed()->m_pWanted->GetWantedLevel();'),
('bool','Chased','CPed *ped','return ped->bBeingChasedByPolice;'),
('bool','DyingOrDead','CPed *ped','return ped->DyingOrDead();'),
('int32_t *','Money','void','return &CWorld::Players[CWorld::PlayerInFocus].m_nMoney;'),
('EventsVector','PlayerCoors','void','CVector v=FindPlayerCoors();EventsVector result={v.x,v.y,v.z};return result;'),
('EventsVector','PlayerPedPosition','void','const CVector &v=FindPlayerPed()->GetPosition();EventsVector result={v.x,v.y,v.z};return result;'),
('int32_t','PolicePresence','EventsVector position,float radius','return CWanted::WorkOutPolicePresence(CVector(position.x,position.y,position.z),radius);'),
('bool','GermanGame','void','return CGame::germanGame;'),
('void','Crime','enum eCrimeType type,EventsVector position,uint32_t id,bool copsDontCare','FindPlayerPed()->m_pWanted->RegisterCrime(type,CVector(position.x,position.y,position.z),id,copsDontCare);'),
('void','ImmediateCrime','enum eCrimeType type,EventsVector position,uint32_t id,bool copsDontCare','FindPlayerPed()->m_pWanted->RegisterCrime_Immediately(type,CVector(position.x,position.y,position.z),id,copsDontCare);'),
('void','WantedNoDrop','int32_t level','FindPlayerPed()->m_pWanted->SetWantedLevelNoDrop(level);'),
('void','PedWantedNoDrop','int32_t level','FindPlayerPed()->SetWantedLevelNoDrop(level);'),
('bool','IsPlayer','CPed *ped','return ped->IsPlayer();'),
('uint8_t','CreatedBy','CPed *ped','return ped->CharCreatedBy;'),
('bool','Scorched','CPed *ped','return ped->bRenderScorched;'),
('bool','BodyPart','CPed *ped','return ped->bBodyPartJustCameOff;'),
('bool','AllowRevive','CPed *ped','return ped->bAllowMedicsToReviveMe;'),
('bool','InWater','CPed *ped','return ped->bIsInWater;'),
('bool','HasSurface','CPed *ped','return ped->m_pCurrentPhysSurface!=nil;'),
('EventsVector','PedPosition','CPed *ped','const CVector &v=ped->GetPosition();EventsVector result={v.x,v.y,v.z};return result;'),
('float','PedZ','CPed *ped','return ped->GetPosition().z;'),
('bool','VerticalLine','EventsVector position,float endZ','CColPoint collision;CEntity *entity;return CWorld::ProcessVerticalLine(CVector(position.x,position.y,position.z),endZ,collision,entity,true,false,false,false,false,false,nil);'),
('void','SetLastAccident','CPed *ped,CAccident *accident','ped->m_lastAccident=accident;'),
('CPed *','Ped','int32_t reference','return CPools::GetPed(reference);'),
('float','Health','CPed *ped','return ped->m_fHealth;')]
BACKEND='//+ rouz edit (ChatGPT)\n#pragma once\n#include "EventList.h"\n#include "Accident.h"\n#include "CrimeTypes.h"\nenum {EVENTS_MISSION_CHAR=2};\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(ret+' Events_Backend'+name+'('+args+');\n' for ret,name,args,body in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def host(accident=False):
    # Keep only typed native primitives outside the complete C event and accident policies
    chosen=READERS[22:] if accident else READERS[:22]
    prefix='//+ rouz edit (ChatGPT)\n#include "common.h"\n#include "Events_backend.h"\n#include "main.h"\n#include "Game.h"\n#include "Ped.h"\n#include "PlayerPed.h"\n#include "Pools.h"\n#include "World.h"\n#include "Wanted.h"\n'
    if accident:prefix+='static_assert(int(EVENTS_MISSION_CHAR)==int(MISSION_CHAR),"Original native pedestrian creation code");\n'
    else:prefix+='EventsVector Events_VectorFromNative(const CVector &v)\n{\n    // Capture original caller vector values explicitly without assuming a native object representation\n    EventsVector result={v.x,v.y,v.z};return result;\n}\n'
    return prefix+'\n'.join(ret+' Events_Backend'+name+'('+args+')\n{\n    // Access the original typed native primitive only at its original C policy access point\n    '+body+'\n}\n' for ret,name,args,body in chosen)+'//- rouz edit (ChatGPT)\n'

def body(source,owner,name):
    # Extract each entire original operation including complete conditional alternatives
    signature=name
    if owner=='CEventList' and name=='RegisterEntity':signature='RegisterEvent(eEventType type, eEventEntity'
    elif owner=='CEventList' and name=='RegisterPosition':signature='RegisterEvent(eEventType type, CVector'
    start=source.index(owner+'::'+signature);start=source.rfind('\n',0,start-1)+1
    return source[start:source.index('\n}',start)+2]

def scalars(text):
    # Preserve the actual original integer widths, vector components and pointer-sized crime IDs
    for old,new in [('uint8','uint8_t'),('uint16','uint16_t'),('uint32','uint32_t'),('int32','int32_t'),('intptr','intptr_t'),('CVector','EventsVector'),('nil','NULL')]:text=re.sub(r'\b'+old+r'\b',new,text)
    return text

def kernel(eventSource,accidentSource,accidentHeader):
    # Preserve both complete mutually dependent owners instead of hiding their decisions inside native adapters
    assert 'CAccident() : m_pVictim(nil), m_nMedicsAttending(0), m_nMedicsPerformingCPR(0) {}' in accidentHeader
    chunks=[]
    for owner,names,prefix,source in [('CEventList',EVENT_OPERATIONS,'Events',eventSource),('CAccidentManager',ACCIDENT_OPERATIONS,'AccidentManager',accidentSource)]:
        for name in names:
            text=body(source,owner,name);oldname='RegisterEvent' if name in ['RegisterEntity','RegisterPosition'] else name
            text=text.replace(owner+'::'+oldname+'(',prefix+'_'+name+'('+('CAccidentManager *self,' if owner=='CAccidentManager' else ''),1)
            text=text.replace('CAccidentManager *self,)','CAccidentManager *self)')
            if owner=='CAccidentManager':
                text=re.sub(r'\bm_aAccidents\b','self->m_aAccidents',text)
                for method in ['GetNextFreeAccident','ReportAccident','WorkToDoForMedics']:text=re.sub(r'(?<![\w])'+method+r'\(', 'AccidentManager_'+method+'(self,',text)
                text=text.replace('(self,)','(self)')
            text=text.replace('ms_nFirstFreeSlotIndex','Events_FirstFreeSlotIndex').replace('MAX_MEDICS_TO_ATTEND_ACCIDENT','ACCIDENT_MAX_MEDICS_TO_ATTEND')
            text=text.replace('gAccidentManager.ReportAccident((CPed*)ent)','AccidentManager_ReportAccident(&gAccidentManager,Events_BackendPedFromEntity(ent))')
            text=text.replace('CEventList::GetEvent(','Events_GetEvent(').replace('CEventList::ClearEvent(','Events_ClearEvent(')
            text=re.sub(r'(?<![\w])ReportCrimeForEvent\(', 'Events_ReportCrimeForEvent(',text)
            text=text.replace('CPools::GetPedRef((CPed*)ent)','Events_BackendPedRef(ent)').replace('CPools::GetVehicleRef((CVehicle*)ent)','Events_BackendVehicleRef(ent)').replace('CPools::GetObjectRef((CObject*)ent)','Events_BackendObjectRef(ent)').replace('CPools::GetPed(gaEvent[e].entityRef)','Events_BackendPed(gaEvent[e].entityRef)')
            text=text.replace('gaEvent[i].posn = ent->GetPosition();','gaEvent[i].posn = Events_BackendEntityPosition(ent);').replace('gaEvent[i].criminal->RegisterReference((CEntity**)&gaEvent[i].criminal)','Events_BackendRegisterReference(gaEvent[i].criminal,&gaEvent[i].criminal)').replace('ped->RegisterReference((CEntity**)&accident->m_pVictim)','Events_BackendRegisterReference(ped,&accident->m_pVictim)')
            text=text.replace('(posn - gaEvent[i].posn).Magnitude()','Events_Magnitude(Events_Sub(posn,gaEvent[i].posn))').replace('(pVictim->GetPosition() - vecPos).Magnitude2D()','Events_Magnitude2D(Events_Sub(Events_BackendPedPosition(pVictim),vecPos))').replace('pVictim->GetPosition().z','Events_BackendPedZ(pVictim)').replace('ped->GetPosition()','Events_BackendPedPosition(ped)')
            text=text.replace('IsPedPointerValid((CPed*)crimeId)','Events_BackendPedValid((CPed*)crimeId)').replace('FindPlayerPed()->m_pWanted->GetWantedLevel()','Events_BackendWantedLevel()').replace('((CPed*)crimeId)->bBeingChasedByPolice','Events_BackendChased((CPed*)crimeId)').replace('((CPed*)crimeId)->DyingOrDead()','Events_BackendDyingOrDead((CPed*)crimeId)').replace('CWorld::Players[CWorld::PlayerInFocus].m_nMoney','(*Events_BackendMoney())')
            text=text.replace('FindPlayerPed()->GetPosition()','Events_BackendPlayerPedPosition()').replace('FindPlayerCoors()','Events_BackendPlayerCoors()').replace('CWanted::WorkOutPolicePresence(','Events_BackendPolicePresence(').replace('CGame::germanGame','Events_BackendGermanGame()')
            text=text.replace('FindPlayerPed()->m_pWanted->RegisterCrime_Immediately(','Events_BackendImmediateCrime(').replace('FindPlayerPed()->m_pWanted->RegisterCrime(','Events_BackendCrime(').replace('FindPlayerPed()->m_pWanted->SetWantedLevelNoDrop(','Events_BackendWantedNoDrop(').replace('FindPlayerPed()->SetWantedLevelNoDrop(','Events_BackendPedWantedNoDrop(').replace('FindPlayerPed()','Events_BackendPlayer()')
            for expression,reader in [('ped->IsPlayer()','IsPlayer'),('ped->CharCreatedBy','CreatedBy'),('ped->bRenderScorched','Scorched'),('ped->bBodyPartJustCameOff','BodyPart'),('ped->bAllowMedicsToReviveMe','AllowRevive'),('ped->bIsInWater','InWater')]:text=text.replace(expression,'Events_Backend'+reader+'(ped)')
            text=text.replace('ped->m_pCurrentPhysSurface == nil','!Events_BackendHasSurface(ped)').replace('ped->m_lastAccident = accident','Events_BackendSetLastAccident(ped,accident)')
            text=text.replace('pVictim->CharCreatedBy','Events_BackendCreatedBy(pVictim)').replace('pVictim->m_fHealth','Events_BackendHealth(pVictim)').replace('MISSION_CHAR','EVENTS_MISSION_CHAR')
            text=text.replace('\t\t\tCColPoint colPoint;\n\t\t\tCEntity *pEntity; \n','').replace('CWorld::ProcessVerticalLine(point, -100.0f, colPoint, pEntity, true, false, false, false, false, false, nil)','Events_BackendVerticalLine(point,-100.0f)')
            text=text.replace('eCrimeType crime;','enum eCrimeType crime;').replace('debug(','Events_BackendDebug(').replace('Error(','Events_BackendError(')
            text=scalars(text).replace('\n{','\n{\n    // Preserve the complete original '+name+' policy and all ordered native accesses',1);chunks.append(text)
    prefix='//+ rouz edit (ChatGPT)\n#include "Events_backend.h"\n#include "Timer.h"\n#include "Messages.h"\n#include "Text.h"\n#include <math.h>\n#include <stddef.h>\nint32_t Events_FirstFreeSlotIndex;\nCEvent gaEvent[NUMEVENTS];\nCAccidentManager gAccidentManager;\nenum {EVENT_STATE_0,EVENT_STATE_CANDELETE,EVENT_STATE_CLEAR};\n'
    helpers='''static EventsVector Events_Sub(EventsVector a,EventsVector b)
{
    // Preserve all three original vector subtractions before selecting a distance norm
    EventsVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static float Events_Magnitude(EventsVector value)
{
    // Preserve the original ordered three-component squared length and float square root
    return sqrtf(value.x*value.x+value.y*value.y+value.z*value.z);
}
static float Events_Magnitude2D(EventsVector value)
{
    // Preserve the original two-component squared length without changing the vertical eligibility test
    return sqrtf(value.x*value.x+value.y*value.y);
}
void Accident_Init(CAccident *self)
{
    // Preserve the original constructor's named-field initialization in declaration order
    self->m_pVictim=NULL;self->m_nMedicsAttending=0;self->m_nMedicsPerformingCPR=0;
}
void AccidentManager_Init(CAccidentManager *self)
{
    // Construct all twenty original accident subrecords without clearing unrelated bytes
    for(int i=0;i<NUM_ACCIDENTS;i++)Accident_Init(&self->m_aAccidents[i]);
}
'''
    return prefix+helpers+'\n\n'+'\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def arguments(source,opening):
    # Keep nested expressions and comments intact when distinguishing the two original overloads
    parts=[];start=opening+1;depth=1;i=start
    while depth:
        if source.startswith('//',i):i=source.index('\n',i);continue
        if source.startswith('/*',i):i=source.index('*/',i)+2;continue
        if source[i] in ['"',"'"]:
            quote=source[i];i+=1
            while source[i]!=quote:i+=2 if source[i]=='\\' else 1
            i+=1;continue
        if source[i] in '([{':depth+=1
        elif source[i] in ')]}':depth-=1
        if source[i]==',' and depth==1:parts.append(source[start:i].strip());start=i+1
        i+=1
    parts.append(source[start:i-1].strip());return i,([] if parts==[''] else parts)

def event_calls(source):
    # Remove all three whole classes and adapt only actual event/manager calls and record forward declarations
    if 'class CEventList\n' in source:return event_header(source)
    if 'class CAccidentManager\n' in source:return ACCIDENT_HEADER
    if 'int32 CEventList::ms_nFirstFreeSlotIndex;' in source:
        if OBSERVER:OBSERVER('events',source)
        return host(False)
    if 'CAccidentManager gAccidentManager;' in source:
        if OBSERVER:OBSERVER('accidents',source)
        return host(True)
    original=source
    pattern=r'CEventList::(\w+)\s*\(|gAccidentManager\.(\w+)\s*\('
    for match in reversed(list(re.finditer(pattern,source))):
        end,args=arguments(source,match.end()-1);assert not re.search(pattern,source[match.end():end-1])
        name=match[1] or match[2]
        if match[1]:
            prefix='Events_'
            if name=='RegisterEvent':
                assert len(args) in [3,5]
                if len(args)==3:name='RegisterPosition';args[1]='Events_VectorFromNative('+args[1]+')'
                else:name='RegisterEntity'
            elif name=='FindClosestEvent':args[1]='Events_VectorFromNative('+args[1]+')'
        else:
            prefix='AccidentManager_'
            if name=='FindNearestAccident':args[0]='Events_VectorFromNative('+args[0]+')'
            args.insert(0,'&gAccidentManager')
        source=source[:match.start()]+prefix+name+'('+','.join(args)+')'+source[end:]
    source=source.replace('class CAccident;','struct CAccident;')
    if source==original:return source
    if original.lstrip().startswith('//+ rouz edit (ChatGPT)') and original.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def event_project(source):
    # Register one uniquely named C policy object and its shared record/native boundaries
    for tag,old,new in [('ClCompile','EventList.cpp','GameplayEvents.c'),('ClInclude','EventList.h','Events_backend.h'),('ClInclude','Accident.h','EventsShared.h')]:
        entry='<'+tag+' Include="..\\src\\core\\'+old+'" />';addition='<'+tag+' Include="..\\src\\core\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\core\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\core\\'+re.escape(old)+r'">\n\s*<Filter>core</Filter>\n\s*</'+tag+'>)';source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove full relocation of either historical producer along with the complete mutually dependent C policies
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=lambda kind,value:captured.append((kind,value))
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1;kind,producer=captured[0]
    normal=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    name='EventList.cpp' if kind=='events' else 'Accident.cpp';assert normal(expected)==normal((root/'src/core'/name).read_text())
    folder=root/'.diagnostics/events-c-tests/before/src/core';event=producer if kind=='events' else (folder/'EventList.cpp').read_text();accident=producer if kind=='accidents' else (folder/'Accident.cpp').read_text()
    assert normal(kernel(event,accident,(folder/'Accident.h').read_text()))==normal((root/'src/core/GameplayEvents.c').read_text())

def verify_dependency(saved,current):
    # Retain complete dependency byte guards unless the whole file follows exactly this class/overload conversion
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original);assert expected!=original and expected==current.read_text(),str(current)


def calls(source):
    # Preserve the complete event and accident conversion before removing the complete wanted owner
    import wanted_migration as wanted
    return wanted.calls(event_calls(source))

def project(source):
    # Retain every prior source registration before adding the unique wanted-system C object
    import wanted_migration as wanted
    return wanted.project(event_project(source))
