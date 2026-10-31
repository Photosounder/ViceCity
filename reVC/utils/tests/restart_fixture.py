"""Audit all restart algorithms and generate real-vector original/C runtime programs."""
from pathlib import Path
import json,re
import restart_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/restart-c-tests'
old=(out/'before/src/control/Restart.cpp').read_text();header=(out/'before/src/control/Restart.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    source=(out/'before'/path).read_text();expected=m.project(source) if path.startswith('build/') else m.calls(source)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
assert (root/'src/control/RestartPoints.c').read_text()==m.kernel(old)
assert (root/'src/control/Restart_backend.h').read_text()==m.BACKEND
for path in json.loads((out/'untouched.json').read_text()):assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
common=(root/'build/water-creatures-c-tests/common.h').read_text()
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
for name in ['Restart.h','Restart_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/control'/name).as_posix()+'"\n',newline='\n')
path=(out/'before/src/control/PathFind.h').read_text();a=path.index('struct CPathNode');b=path.index('\n};',a)+3;node=path[a:b]
owners='''#pragma once
#include "common.h"
#include "Restart_backend.h"
#include "Zones.h"
#include "SaveBuf.h"
'''+next(line for line in path.splitlines() if line.startswith('#define WIDTH_TO_PED_NODE_WIDTH'))+'\n'+node+'''
enum {PATH_CAR=0,PATH_PED=1};
class CPathFind {public:CPathNode m_pathNodes[4];int32 FindNodeClosestToCoors(CVector position,uint8 type,float limit,bool disabled=false,bool between=false,bool selected=false,bool water=false);};extern CPathFind ThePaths;
extern "C" int TestRestart_Printf(const char *,...);
extern "C" eLevelName TestRestart_RealLevel(ZonesPosition position);
'''
(out/'owners.h').write_text(owners,newline='\n')
operations=[('Initialise','void',''),('AddHospitalRestartPoint','void','const RestartPosition *position,float heading'),('AddPoliceRestartPoint','void','const RestartPosition *position,float heading'),('OverrideNextRestart','void','const RestartPosition *position,float heading'),('CancelOverrideRestart','void',''),('FindClosestHospitalRestartPoint','void','const RestartPosition *position,RestartPosition *output,float *heading'),('FindClosestPoliceRestartPoint','void','const RestartPosition *position,RestartPosition *output,float *heading'),('LoadAllRestartPoints','void','uint8_t *buffer,uint32_t size'),('SaveAllRestartPoints','void','uint8_t *buffer,uint32_t *size')]
state='typedef struct RestartTestState {\n'+''.join(kind+' '+name+';\n' for kind,name in m.SCALARS)+''.join('RestartPoint '+kind+'RestartPoints[NUM_RESTART_POINTS];\nfloat '+kind+'RestartHeadings[NUM_RESTART_POINTS];\n' for kind in ['Hospital','Police'])+'} RestartTestState;\n'
prototypes=state+'void TestRestart_Read(RestartTestState *state);void TestRestart_Write(const RestartTestState *state);\n'
for name,ret,args in operations:prototypes+=ret+' TestRestart_'+name+'('+(args or 'void')+');'+ret+' TestRestart_Run'+name+'('+(args or 'void')+');\n'
(out/'caller.h').write_text('#pragma once\n#include "Restart.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
caller='#include "caller.h"\n'
for name,ret,args in operations:
    values=[re.search(r'(\w+)\s*$',arg)[1] for arg in args.split(',')] if args else []
    caller+=ret+' TestRestart_Run'+name+'('+(args or 'void')+')\n{\n    // Enter the complete actual operation through its standalone C11 caller\n    TestRestart_'+name+'('+','.join(values)+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
for before in [True,False]:
    text='#include "owners.h"\n#include "caller.h"\n'
    if before:text+='namespace Original {\n'+header.replace('#pragma once','')+'\n#define printf TestRestart_Printf\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n#undef printf\n}\n'
    prefix='Original::CRestart::' if before else 'Restart_'
    text+='extern "C" {\nvoid TestRestart_Read(RestartTestState *state)\n{\n    // Observe every actual canonical scalar and vector component without a representation cast\n'
    for kind,name in m.SCALARS:
        if kind=='RestartPoint':
            for axis in 'xyz':text+='    state->'+name+'.'+axis+'='+prefix+name+'.'+axis+';\n'
        else:text+='    state->'+name+'='+prefix+name+';\n'
    text+='    for(int i=0;i<NUM_RESTART_POINTS;i++){\n        // Observe every real point and heading slot including currently inactive entries\n'
    for category in ['Hospital','Police']:
        for axis in 'xyz':text+='        state->'+category+'RestartPoints[i].'+axis+'='+prefix+category+'RestartPoints[i].'+axis+';\n'
        text+='        state->'+category+'RestartHeadings[i]='+prefix+category+'RestartHeadings[i];\n'
    text+='    }\n}\nvoid TestRestart_Write(const RestartTestState *state)\n{\n    // Seed only actual canonical named fields with valid test values\n'
    for kind,name in m.SCALARS:
        if kind=='RestartPoint':
            for axis in 'xyz':text+='    '+prefix+name+'.'+axis+'=state->'+name+'.'+axis+';\n'
        else:text+='    '+prefix+name+'=state->'+name+';\n'
    text+='    for(int i=0;i<NUM_RESTART_POINTS;i++){\n        // Preserve the actual canonical table addresses while changing their named contents\n'
    for category in ['Hospital','Police']:
        for axis in 'xyz':text+='        '+prefix+category+'RestartPoints[i].'+axis+'=state->'+category+'RestartPoints[i].'+axis+';\n'
        text+='        '+prefix+category+'RestartHeadings[i]=state->'+category+'RestartHeadings[i];\n'
    text+='    }\n}\n'
    for name,ret,args in operations:
        values=[re.search(r'(\w+)\s*$',arg)[1] for arg in args.split(',')] if args else []
        if before and values and values[0]=='position':values[0]='*position'
        text+=ret+' TestRestart_'+name+'('+(args or 'void')+')\n{\n    // Execute the entire preserved original or C operation with the same native object aliases\n    '+prefix+name+'('+','.join(values)+');\n}\n'
    (out/('original.cpp' if before else 'adapter.cpp')).write_text(text+'}\n',newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',m.HOST,flags=re.M),newline='\n')
zones=(out/'before/src/core/Zones.c').read_text()
real='#include "Zones.h"\nCZone Zones_map[NUMMAPZONES];\nuint16_t Zones_mapCount;\n'+function(zones,'','Zones_PointLiesWithinZone')+'\n'+function(zones,'','Zones_GetLevelFromPosition').replace('Zones_GetLevelFromPosition(','TestRestart_RealLevel(')+'\n'
(out/'real-zones.c').write_text(real,newline='\n')
kernel=(root/'src/control/RestartPoints.c').read_text().replace('#include <stdio.h>','#include <stdio.h>\nint TestRestart_Printf(const char *,...);\n#define printf TestRestart_Printf')
(out/'kernel.c').write_text(kernel,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/restart.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all nine restart operations, thirteen canonical C variables, two complete point tables, actual zone algorithms, native-vector aliases and eleven full source/project changes')
