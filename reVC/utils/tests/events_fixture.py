"""Prepare immutable whole-owner event/accident comparisons outside the disposable build tree."""
from pathlib import Path
import json,re,sys,hashlib
import events_migration as m
root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/events-c-tests'
out.mkdir(parents=True,exist_ok=True)
owners=['src/core/EventList.cpp','src/core/EventList.h','src/core/Accident.cpp','src/core/Accident.h']
if not (out/'changes.json').exists():
    assert 'class CEventList\n' in (root/'src/core/EventList.h').read_text(),'The original comparison snapshots were removed after conversion; restore .diagnostics/events-c-tests/before before running the historical comparison'
    callers=[p.relative_to(root).as_posix() for p in (root/'src').rglob('*') if p.suffix in ['.cpp','.c','.h'] and p.relative_to(root).as_posix() not in owners and any(t in p.read_text(errors='replace') for t in ['CEventList::','gAccidentManager.','class CAccident;'])]
    changes=owners+callers
    for path in changes+['src/math/Vector.h','src/core/CrimeTypes.h','src/core/config.h']:
        saved=out/'before'/path;saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    (out/'changes.json').write_text(json.dumps(changes,indent=2),newline='\n')
    (out/'untouched.json').write_text(json.dumps({p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in (root/'src').rglob('*') if p.suffix in ['.cpp','.c','.h'] and p.relative_to(root).as_posix() not in changes},indent=2),newline='\n')
old=lambda name:(out/'before/src/core'/name).read_text()
preview=out/'preview';preview.mkdir(exist_ok=True)
for name,value in [('EventsShared.h',m.SHARED),('Events_backend.h',m.BACKEND),('EventList.h',m.event_header(old('EventList.h'))),('Accident.h',m.ACCIDENT_HEADER),('GameplayEvents.c',m.kernel(old('EventList.cpp'),old('Accident.cpp'),old('Accident.h')))]:
    (preview/name).write_text(value,newline='\n')
if '--production' in sys.argv:
    for path in json.loads((out/'changes.json').read_text()):assert (root/path).read_text()==m.calls((out/'before'/path).read_text()),path
    for name in ['EventsShared.h','Events_backend.h','GameplayEvents.c']:assert (root/'src/core'/name).read_text()==(preview/name).read_text(),name
(out/'config.h').write_text('#pragma once\n#define NUMEVENTS 64\n#define NUM_ACCIDENTS 20\n',newline='\n')
(out/'CrimeTypes.h').write_text((out/'before/src/core/CrimeTypes.h').read_text(),newline='\n')
(out/'Vector.h').write_text((out/'before/src/math/Vector.h').read_text(),newline='\n')
(out/'common.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <new>
#include "config.h"
typedef int32_t int32;typedef uint32_t uint32;typedef uint16_t uint16;typedef uint8_t uint8;typedef intptr_t intptr;
#define nil NULL
struct RwV3d {float x,y,z;};
inline float Sqrt(float v){return sqrtf(v);}
inline float RecipSqrt(float v){return 1.0f/sqrtf(v);}
inline float Atan2(float x,float y){return atan2f(x,y);}
#include "Vector.h"
''',newline='\n')
for name in ['EventList','Accident']:
    value=old(name+'.h').replace('class CEventList\n{','class CEventList\n{\npublic:').replace('class CAccidentManager\n{','class CAccidentManager\n{\npublic:')
    (out/('original-'+name+'.h')).write_text(value,newline='\n')
(out/'Timer.h').write_text('#pragma once\n#include <stdint.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\nextern uint32_t TestTime;\n#ifdef __cplusplus\n}\n#endif\nstatic inline uint32_t *Timer_GetTimeInMilliseconds(void){return &TestTime;}\n',newline='\n')
(out/'Text.h').write_text('''#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
extern int TheText;
uint16_t *Text_Get(void *text,const char *key);
#ifdef __cplusplus
}
#endif
''',newline='\n')
(out/'Messages.h').write_text('''#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void Messages_AddBigMessage(uint16_t *text,uint32_t duration,uint16_t style);
#ifdef __cplusplus
}
#endif
''',newline='\n')
models='''#pragma once
#include "common.h"
#include "CrimeTypes.h"
#include "Timer.h"
#include "Text.h"
#include "Messages.h"
#ifdef TEST_EVENTS_ORIGINAL
#include "original-EventList.h"
#include "original-Accident.h"
typedef CVector EventsVector;
#define FIRST CEventList::ms_nFirstFreeSlotIndex
#else
#include "@PREVIEW@/EventList.h"
#include "@PREVIEW@/Accident.h"
#define FIRST Events_FirstFreeSlotIndex
#endif
void Observe(unsigned point,int identity);
template<class T>struct Observed {T value;int owner;unsigned point;operator T()const{Observe(point,owner);return value;}};
struct EntityPrefix {uintptr_t padding[3];};
class CEntity {public:int id;CVector position;const CVector &GetPosition();void RegisterReference(CEntity **slot);};
enum {RANDOM_CHAR=1,MISSION_CHAR=2};
class CWanted {public:int id,level;int GetWantedLevel();void RegisterCrime(eCrimeType,const CVector &,uint32,bool);void RegisterCrime_Immediately(eCrimeType,const CVector &,uint32,bool);void SetWantedLevelNoDrop(int);static int WorkOutPolicePresence(const CVector &,float);};
class CPed:public EntityPrefix,public CEntity {public:bool player,dead;Observed<uint8> CharCreatedBy;Observed<bool> bRenderScorched,bBodyPartJustCameOff,bAllowMedicsToReviveMe,bIsInWater,bBeingChasedByPolice;Observed<float> m_fHealth;CEntity *m_pCurrentPhysSurface;CAccident *m_lastAccident;CWanted *m_pWanted;bool IsPlayer();bool DyingOrDead();void SetWantedLevelNoDrop(int);};
struct PlayerPrefix {uintptr_t padding[5];};
class CPlayerPed:public PlayerPrefix,public CPed {};
class CVehicle:public EntityPrefix,public CEntity {};
class CObject:public EntityPrefix,public CEntity {};
class CPools {public:static int GetPedRef(CPed*);static int GetVehicleRef(CVehicle*);static int GetObjectRef(CObject*);static CPed *GetPed(int);};
struct CColPoint {CVector point,normal;int surface;};
struct TestPlayerInfo {int32 m_nMoney;};
class CWorld {public:static TestPlayerInfo Players[2];static uint8 PlayerInFocus;static bool ProcessVerticalLine(const CVector &,float,CColPoint &,CEntity *&,bool,bool,bool,bool,bool,bool,CEntity*);};
class CGame {public:static bool germanGame;};
CPlayerPed *FindPlayerPed();CVector FindPlayerCoors();bool IsPedPointerValid(CPed*);
void debug(const char *,...);void Error(const char *,...);
#include "entry.h"
'''.replace('@PREVIEW@',preview.as_posix())
(out/'owners.h').write_text(models,newline='\n')
if 'Wanted_GetWantedLevel(' in (root/'src/core/EventList.cpp').read_text():
    from wanted_test_dependencies import FACADE
    (out/'owners.h').write_text(models+FACADE,newline='\n')
api='#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
caller='#include "'+(preview/'EventList.h').as_posix()+'"\n#include "'+(preview/'Accident.h').as_posix()+'"\n#include "entry.h"\n'
original='#include "owners.h"\n'+ '\n'.join(re.sub(r'^#include[^\n]*\n','',old(name),flags=re.M) for name in ['EventList.cpp','Accident.cpp'])+'\n'
signatures=re.findall(r'(?m)^(void|bool|uint16_t|CAccident \*)\s*(Events_\w+|AccidentManager_\w+|Accident_Init)\(([^\n]*)\);',m.event_header(old('EventList.h'))+m.ACCIDENT_HEADER)
assert len(signatures)==17,len(signatures)
for ret,name,args in signatures:
    values=[] if args=='void' else [re.search(r'(\w+)\s*$',arg)[1] for arg in args.split(',')]
    api+=ret+' Run_'+name+'('+args+');\n'+ret+' Before_'+name+'('+args+');\n'
    if name=='Accident_Init':call='new(self) CAccident'
    elif name=='AccidentManager_Init':call='new(self) CAccidentManager'
    elif name.startswith('AccidentManager_'):call=values[0]+'->'+name[len('AccidentManager_'):]+'('+','.join(values[1:])+')'
    else:
        member=name[len('Events_'):];member='RegisterEvent' if member in ['RegisterEntity','RegisterPosition'] else member
        call='CEventList::'+member+'('+','.join(values)+')'
    original+='extern "C" '+ret+' Before_'+name+'('+args+')\n{\n    // Invoke the complete original operation with its actual declared argument types\n    '+('return ' if ret!='void' else '')+call+';\n}\n'
    caller+=ret+' Run_'+name+'('+args+')\n{\n    // Exercise the complete public operation from an independent strict C11 caller\n#ifdef TEST_EVENTS_ORIGINAL\n    '+('return ' if ret!='void' else '')+'Before_'+name+'('+','.join(values)+');\n#else\n    '+('return ' if ret!='void' else '')+name+'('+','.join(values)+');\n#endif\n}\n'
api+='#ifdef __cplusplus\n}\n#endif\n'
(out/'entry.h').write_text(api,newline='\n');(out/'caller.c').write_text(caller,newline='\n');(out/'original.cpp').write_text(original,newline='\n')
for name,accident in [('event-host.cpp',False),('accident-host.cpp',True)]:
    current=m.calls(old('Accident.cpp' if accident else 'EventList.cpp'))
    (out/name).write_text('#include "owners.h"\n#include "'+(preview/'Events_backend.h').as_posix()+'"\n'+re.sub(r'^#include[^\n]*\n','',current,flags=re.M),newline='\n')
(out/'fixture.cpp').write_text((root/'utils/tests/events.cpp.in').read_text(),newline='\n')
print('Prepared 17 whole original/C operations and immutable event/accident owners in',out)
