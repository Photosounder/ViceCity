"""Prepare whole original/C wanted and audio-entry comparisons with real C pool/crime primitives."""
from pathlib import Path
import re,json,sys
import wanted_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/wanted-c-tests';preview=out/'preview';saved=out/'before'
if '--production' in sys.argv:
    for path in json.loads((out/'changes.json').read_text()):assert (root/path).read_text()==m.calls((saved/path).read_text()),path
    for name in ['WantedSystem.c','Wanted_backend.h','WantedShared.h']:assert (root/'src/core'/name).read_text()==(preview/name).read_text(),name
for name,path in [('Vector.h','src/math/Vector.h'),('Pool.h','src/core/Pool.h'),('CrimeTypes.h','src/core/CrimeTypes.h'),('Validate.h','src/core/Validate.h'),('EntityStatus.h','src/entities/EntityStatus.h'),('AudioCrimes.c','src/audio/AudioCrimes.c')]:
    source=(saved/path).read_text() if (saved/path).exists() else (root/path).read_text();(out/name).write_text(source,newline='\n')
(out/'AudioCrimes.h').write_text((saved/'src/audio/AudioCrimes.h').read_text().replace('"../core/CrimeTypes.h"','"CrimeTypes.h"'),newline='\n')
(out/'config.h').write_text('#pragma once\n#define NUMEVENTS 64\n#define NUM_ACCIDENTS 20\n',newline='\n')
(out/'common.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <new>
#include <memory>
#include "config.h"
typedef int32_t int32;typedef uint32_t uint32;typedef int16_t int16;typedef uint16_t uint16;typedef uint8_t uint8;typedef int8_t int8;
#define nil NULL
#define VALIDATE_SIZE(type,size)
#define ARRAY_SIZE(value) (sizeof(value)/sizeof((value)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
struct RwV3d {float x,y,z;};
inline float Sqrt(float v){return sqrtf(v);}
inline float RecipSqrt(float v){return 1.0f/sqrtf(v);}
inline float Atan2(float x,float y){return atan2f(x,y);}
#include "Vector.h"
''',newline='\n')
(out/'original-Crime.h').write_text((saved/'src/core/Crime.h').read_text(),newline='\n')
(out/'original-Wanted.h').write_text((saved/'src/core/Wanted.h').read_text().replace('"Crime.h"','"original-Crime.h"'),newline='\n')
owners='''#pragma once
#include "common.h"
#include "CrimeTypes.h"
#include "Pool.h"
#include "Timer.h"
#include "ModelIndices.h"
#include "ZoneCull.h"
#include "General.h"
#include "EntityStatus.h"
#ifdef TEST_WANTED_ORIGINAL
#include "original-Wanted.h"
typedef CVector WantedVector;
struct WantedPositionRef {const WantedVector *plain;const CVector *native;};
#define GLOBAL(name) CWanted::name
#else
#include "@PREVIEW@/Wanted.h"
#define GLOBAL(name) Wanted_##name
#endif
void Observe(unsigned point,int identity);
template<class T>struct Observed {T value;int identity;unsigned read,write;operator T()const{Observe(read,identity);return value;}Observed &operator=(T v){Observe(write,identity);value=v;return *this;}};
struct EntityPrefix {uintptr_t padding[3];};
class CEntity {public:int identity;Observed<int16> model;CVector position;const CVector &GetPosition();int16 GetModelIndex();};
class CPed:public EntityPrefix,public CEntity {};
class CVehicle:public EntityPrefix,public CEntity {public:Observed<bool> bIsLawEnforcer;Observed<uint8> status;uint8 GetStatus();};
struct CopPrefix {uintptr_t padding[5];};
enum {OBJECTIVE_NONE=0,PED_NONE=0};
class CCopPed:public CopPrefix,public CPed {public:Observed<bool> m_bIsInPursuit;Observed<int> m_objective,m_prevObjective,m_nLastPedState;bool dying;bool DyingOrDead();bool SetWanderPath(int8 path);};
class CPlayerPed {};
class CPools {public:static CPool *GetPedPool();static CPool *GetVehiclePool();};
class CDarkel {public:static bool FrenzyOnGoing();};
class CStats {public:static int32 WantedStarsAttained,WantedStarsEvaded;};
CVector FindPlayerCoors();CVehicle *FindPlayerVehicle();void Error(const char *,...);
#include "entry.h"
'''.replace('@PREVIEW@',preview.as_posix())
(out/'owners.h').write_text(owners,newline='\n')
if 'Darkel_FrenzyOnGoing(' in (root/'src/core/Wanted.cpp').read_text():
    (out/'owners.h').write_text(owners+'''extern "C" inline bool Darkel_FrenzyOnGoing(void)
{
    // Preserve the existing observed rampage dependency while testing the complete wanted policy separately
    return CDarkel::FrenzyOnGoing();
}
''',newline='\n')
api='#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
caller='#include "'+(preview/'Wanted.h').as_posix()+'"\n#include "entry.h"\n'
original='#include "owners.h"\n#include "DMAudio.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/core/Wanted.cpp').read_text(),flags=re.M)+'\n'
for ret,name,args in m.SIGNATURES:
    values=[re.search(r'(\w+)\s*$',a)[1] for a in args.split(',')];actual=list(values)
    if name in m.REF_ARGUMENTS:
        index=m.REF_ARGUMENTS[name]+1;actual[index]='*('+actual[index]+'.native?'+actual[index]+'.native:'+actual[index]+'.plain)'
    if name=='ConstructValue':call='std::allocator<CWanted>().construct(self)'
    elif name=='Copy':call='std::allocator<CWanted>().construct(self,*source)'
    elif name in m.STATIC:call='CWanted::'+name+'('+','.join(actual)+')'
    else:call='self->'+name+'('+','.join(actual[1:])+')'
    api+=ret+' Run_Wanted_'+name+'('+args+');\n'+ret+' Before_Wanted_'+name+'('+args+');\n'
    original+='extern "C" '+ret+' Before_Wanted_'+name+'('+args+')\n{\n    // Invoke the entire original operation with its original live reference or by-value argument\n    '+('return ' if ret!='void' else '')+call+';\n}\n'
    caller+=ret+' Run_Wanted_'+name+'('+args+')\n{\n    // Exercise the complete public operation from a standalone strict C11 caller\n#ifdef TEST_WANTED_ORIGINAL\n    '+('return ' if ret!='void' else '')+'Before_Wanted_'+name+'('+','.join(values)+');\n#else\n    '+('return ' if ret!='void' else '')+'Wanted_'+name+'('+','.join(values)+');\n#endif\n}\n'
api+='#ifdef __cplusplus\n}\n#endif\n';(out/'entry.h').write_text(api,newline='\n');(out/'caller.c').write_text(caller,newline='\n');(out/'original.cpp').write_text(original,newline='\n')
from stats_test_dependencies import aliases
current=m.calls((saved/'src/core/Wanted.cpp').read_text())
(out/'host.cpp').write_text('#include "owners.h"\n#include "'+(preview/'Wanted_backend.h').as_posix()+'"\n'+aliases(current)+re.sub(r'^#include[^\n]*\n','',current,flags=re.M),newline='\n')
(out/'kernel.c').write_text((preview/'WantedSystem.c').read_text(),newline='\n')
for name in ['before','after']:(out/('audio-'+name+'.c')).write_text((saved/'src/audio/AudioGameEntries.c').read_text() if name=='before' else m.audio_entries((saved/'src/audio/AudioGameEntries.c').read_text()),newline='\n')
capi='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AudioCrimes.h"
#ifdef __cplusplus
class CVector;class CEntity;class CVehicle;class CPlayerPed;
extern "C" {
#else
typedef struct CVector CVector;typedef struct CEntity CEntity;typedef struct CVehicle CVehicle;typedef struct CPlayerPed CPlayerPed;
#endif
enum {MUSICMODE_CUTSCENE=2};
typedef struct cAudioManager {bool m_bIsInitialised;uint32_t m_FrameCounter;cAMCrime m_aCrimes[AUDIO_CRIME_COUNT];} cAudioManager;
extern cAudioManager AudioManager;extern uint32_t gMinTimeToNextReport[NUM_CRIME_TYPES];
CVehicle *AudioPoliceHost_PlayerVehicle(void);CPlayerPed *AudioPoliceHost_FindPlayer(void);CEntity *AudioPoliceHost_AttachedTo(const CPlayerPed*);uint8_t AudioPoliceHost_IsVehicle(CEntity*);CVehicle *AudioPoliceHost_AsVehicle(CEntity*);
uint8_t AudioPoliceHost_MusicMode(void);int32_t AudioPoliceHost_WantedLevel(const CPlayerPed*);
void AudioPoliceHost_CrimePosition(const CVector *position,AudioCrimePosition *plain);
void AudioPolice_ReportCrime(cAudioManager *manager,enum eCrimeType type,const CVector *position);
void DMAudio_ReportCrime(enum eCrimeType type,const CVector *position);void DMAudio_ReportCrimeAt(enum eCrimeType type,const AudioCrimePosition *position);
extern uint32_t TestTime;uint32_t *Timer_GetTimeInMilliseconds(void);
int32_t CullZones_GetWantedLevelDrop(void);float General_GetRandomNumberInRangeFloat(float min,float max);
bool IsPolicePedModel(int16_t model);bool IsPoliceVehicleModel(int16_t model);
#ifdef __cplusplus
}
#endif
'''
(out/'capi.h').write_text(capi,newline='\n')
for name in ['Timer.h','ZoneCull.h','General.h','ModelIndices.h','AudioPoliceGame.h','AudioManager.h','DMAudio.h']:(out/name).write_text('#pragma once\n#include "capi.h"\n',newline='\n')
(out/'fixture.cpp').write_text((root/'utils/tests/wanted.cpp.in').read_text(),newline='\n')
print('Prepared all 26 wanted operations, complete original/current audio entry policies and actual C pool/crime primitives')
