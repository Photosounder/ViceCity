"""Prepare whole original/C profiler comparisons and direct cut-feature no-op checks."""
from pathlib import Path
import re,json,sys
import profile_power_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/profile-power-c-tests';saved=out/'before';preview=out/'preview'
if '--production' in sys.argv:
    for path in json.loads((out/'changes.json').read_text()):assert (root/m.RENAMES.get(path,path)).read_text()==m.calls((saved/path).read_text()),path
    for old,new in m.RENAMES.items():assert not (root/old).exists() and (root/new).exists()
(out/'config.h').write_text('#pragma once\n',newline='\n')
(out/'original-Profile.h').write_text((saved/'src/core/Profile.h').read_text().replace('class CProfile\n{','class CProfile\n{\npublic:'),newline='\n')
(out/'original-PowerPoints.h').write_text((saved/'src/control/PowerPoints.h').read_text(),newline='\n')
owners='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef uint8_t uint8;typedef uint32_t uint32;
#define Const const
#define Max(a,b) ((a)>(b)?(a):(b))
struct RwRGBA {uint8_t red,green,blue,alpha;};
#ifdef TEST_PROFILE_POWER_ORIGINAL
#include "original-Profile.h"
#include "original-PowerPoints.h"
#define GLOBAL(name) CProfile::name
typedef RwRGBA TestProfileColor;
#else
#include "@PREVIEW@/Profile.h"
#include "@PREVIEW@/PowerPoints.h"
#define GLOBAL(name) Profile_##name
typedef ProfileColor TestProfileColor;
#endif
#include "entry.h"
'''.replace('@PREVIEW@',preview.as_posix());(out/'owners.h').write_text(owners,newline='\n')
api='#pragma once\n#include <stdint.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
signatures=[('Profile_Initialise','void',[]),('Profile_SuspendProfile','int32_t profile',['profile']),('Profile_ShowResults','void',[])]+[(name,args.replace('CPowerPoint *self','void *self'),values) for name,args,values in m.POWER_OPERATIONS]
original='#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/core/Profile.cpp').read_text(),flags=re.M)+'\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/control/PowerPoints.cpp').read_text(),flags=re.M)+'\n'
caller='#include "entry.h"\n#ifndef TEST_PROFILE_POWER_ORIGINAL\n#include "'+(preview/'Profile.h').as_posix()+'"\n#include "'+(preview/'PowerPoints.h').as_posix()+'"\n#endif\n'
for name,args,values in signatures:
    guard='#ifndef MASTER\n' if name.startswith('Profile_') else '';tail='#endif\n' if guard else ''
    api+=guard+'void Before_'+name+'('+args+');\nvoid Run_'+name+'('+args+');\n'+tail
    if name.startswith('Profile_'):
        native='CProfile::'+name[len('Profile_'):]+'('+('(eProfile)profile' if values else '')+');';plain=name+'('+('(eProfile)profile' if values else '')+');'
    elif name=='PowerPoint_Update':native='((CPowerPoint*)self)->Update();';plain='PowerPoint_Update((CPowerPoint*)self);'
    else:native='CPowerPoints::'+name[len('PowerPoints_'):]+'('+','.join(values)+');';plain=name+'('+','.join(values)+');'
    original+=guard+'extern "C" void Before_'+name+'('+args+')\n{\n    // Invoke the complete original operation with its original native parameter types\n    '+native+'\n}\n'+tail
    caller+=guard+'void Run_'+name+'('+args+')\n{\n    // Exercise the complete public operation through a standalone strict C11 caller\n#ifdef TEST_PROFILE_POWER_ORIGINAL\n    Before_'+name+'('+','.join(values)+');\n#else\n    '+plain+'\n#endif\n}\n'+tail
api+='#ifdef __cplusplus\n}\n#endif\n';(out/'entry.h').write_text(api,newline='\n');(out/'caller.c').write_text(caller,newline='\n');(out/'original.cpp').write_text(original,newline='\n')
for name in ['Profile.c','PowerPoints.c']:(out/name).write_text((preview/name).read_text(),newline='\n')
(out/'fixture.cpp').write_text((root/'utils/tests/profile_power.cpp.in').read_text(),newline='\n')
print('Prepared all 3 original/C profiler policies and 6 direct no-op contracts with standalone C callers')
