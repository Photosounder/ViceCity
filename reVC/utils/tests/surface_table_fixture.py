"""Audit all surface-property algorithms and generate complete original/C comparison units."""
from pathlib import Path
import json,re
import surface_table_migration as m
import var_console_migration as variables
import weather_migration as weather
root=Path(__file__).resolve().parents[2];out=root/'build/surface-table-c-tests'
old=(out/'before/src/core/SurfaceTable.cpp').read_text();header=(out/'before/src/core/SurfaceTable.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    before=(out/'before'/path).read_text();expected=m.project(before) if path.startswith('build/') else m.calls(before)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
assert (root/'src/core/SurfaceProperties.c').read_text()==m.kernel(old,header)
assert (root/'src/core/SurfaceTable_backend.h').read_text()==m.BACKEND
for path in ['src/core/common.h','src/core/config.h','src/core/main.h','src/core/main.cpp','src/core/FileMgr.h','src/core/FileMgr.c','src/core/SurfaceTypes.h','src/renderer/Weather.h','src/renderer/Weather.cpp','src/collision/ColPoint.h','src/collision/ColPoint.cpp','src/math/Vector.h']:
    if path=='src/core/main.cpp':
        assert (root/path).read_text()==variables.calls((out/'before'/path).read_text()),path
    elif path in ['src/renderer/Weather.h','src/renderer/Weather.cpp']:
        before=(out/'before'/path).read_text()
        if path.endswith('.cpp'):weather.verify_legacy(before,m.calls,root)
        assert (root/path).read_text()==m.calls(before),path
    else:assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "config.h"
typedef uint8_t uint8;
#define Const const
#endif
'''
(out/'common.h').write_text(common,newline='\n')
(out/'main.h').write_text('#pragma once\nextern uint8_t work_buff[55000];\n',newline='\n')
(out/'ColPoint.h').write_text('#pragma once\n#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/collision/ColPoint.h').as_posix()+'"\n#else\nstruct CColPoint { uint64_t firstGuard;uint8_t surfaceA;uint32_t secondGuard;uint8_t surfaceB;uint64_t lastGuard; };\n#endif\n',newline='\n')
(out/'Weather.h').write_text('#pragma once\nclass CWeather { public:static float WetRoads; };\n',newline='\n')
(out/'owners.h').write_text('#pragma once\n#include "common.h"\n#include "SurfaceTable.h"\n#include "FileMgr.h"\n#include "main.h"\n#include "Weather.h"\n#include "ColPoint.h"\n',newline='\n')
for name in ['SurfaceTable.h','SurfaceTable_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/core'/name).as_posix()+'"\n',newline='\n')
original_header=re.sub(r'^#(?:pragma|include)[^\n]*\n','',header,flags=re.M).replace('struct CColPoint;','')
(out/'original-classes.h').write_text(original_header,newline='\n')
prototypes='''void TestSt_Init(const char *filename);int TestSt_Group(uint8_t surface);float TestSt_Wet(uint8_t surface);float TestSt_Limit(CColPoint *point);
bool TestSt_Soft(uint8_t surface);bool TestSt_See(uint8_t surface);bool TestSt_SeeVertical(uint8_t surface);bool TestSt_Shoot(uint8_t surface);
void TestSt_RunInit(const char *filename);int TestSt_RunGroup(uint8_t surface);float TestSt_RunWet(uint8_t surface);float TestSt_RunLimit(CColPoint *point);
bool TestSt_RunSoft(uint8_t surface);bool TestSt_RunSee(uint8_t surface);bool TestSt_RunSeeVertical(uint8_t surface);bool TestSt_RunShoot(uint8_t surface);
'''
(out/'caller.h').write_text('#pragma once\n#include "SurfaceTable.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
specs=[('Init','void','const char *filename','Initialise','filename'),('Group','int','uint8_t surface','GetAdhesionGroup','surface'),('Wet','float','uint8_t surface','GetWetMultiplier','surface'),('Limit','float','CColPoint *point','GetAdhesiveLimit','*point'),('Soft','bool','uint8_t surface','IsSoftLanding','surface'),('See','bool','uint8_t surface','IsSeeThrough','surface'),('SeeVertical','bool','uint8_t surface','IsSeeThroughVertical','surface'),('Shoot','bool','uint8_t surface','IsShootThrough','surface')]
for original in [True,False]:
    text='#include "owners.h"\n#include "caller.h"\n'
    if original:text+='namespace Original {\n'+original_header+'\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n}\n'
    text+='extern "C" {\n'
    for alias,result,args,method,invoke in specs:
        target=('Original::CSurfaceTable::' if alias not in ['See','SeeVertical','Shoot'] else 'Original::')+method if original else ('SurfaceTable_' if alias not in ['See','SeeVertical','Shoot'] else '')+method
        if alias=='Limit' and not original:invoke='point'
        text+=result+' TestSt_'+alias+'('+args+')\n{\n    // Enter the entire actual original or C surface-property operation\n    '+('return ' if result!='void' else '')+target+'('+invoke+');\n}\n'
    text+='}\n';(out/('original.cpp' if original else 'adapter.cpp')).write_text(text,newline='\n')
caller='#include "caller.h"\n'
for alias,result,args,method,invoke in specs:
    if alias=='Limit':invoke='point'
    caller+=result+' TestSt_Run'+alias+'('+args+')\n{\n    // Preserve the real parameter widths and opaque native point through a separate C11 caller\n    '+('return ' if result!='void' else '')+'TestSt_'+alias+'('+invoke+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
(out/'host.cpp').write_text(m.HOST,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/surface_table.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all eight complete algorithms, symmetric static table, four exact native field/buffer readers and ten complete source/project files')
