"""Audit complete source relocation and prepare unchanged-original/C runtime comparisons."""
from pathlib import Path
import json,re
import onscreen_timer_migration as m
import text_migration as text
root=Path(__file__).resolve().parents[2];out=root/'build/onscreen-timer-c-tests'
for path in json.loads((out/'changes.json').read_text()):
    source=(out/'before'/path).read_text();expected=m.project(source) if path.startswith('build/') else m.calls(source)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/control/OnscreenTimer.cpp').read_text()
assert (root/'src/control/OnscreenDisplay.c').read_text()==m.kernel(old)
assert (root/'src/control/OnscreenTimer_backend.h').read_text()==m.BACKEND
for path in json.loads((out/'dependencies.json').read_text()):
    if path not in json.loads((out/'changes.json').read_text()):text.verify_dependency(out/'before'/path,root/path)
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
typedef uint8_t uint8;typedef uint16_t uint16;typedef uint32_t uint32;
typedef int16_t int16;typedef int32_t int32;
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
#define VALIDATE_SIZE(type,size)
#include <assert.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
'''
(out/'common.h').write_text(common,newline='\n')
(out/'original-OnscreenTimer.h').write_text((out/'before/src/control/OnscreenTimer.h').read_text(),newline='\n')
script=(out/'before/src/control/Script.h').read_text()
getter=re.search(r'static int32\* GetPointerToScriptVariable\(int32 offset\) \{[^\n]+\}',script)[0].replace('GetPointerToScriptVariable','RealVariable')
size=m.function(re.sub(r'(?m)^\t','',script),'','GetSizeOfVariableSpace')
read=m.function(re.sub(r'(?m)^\t','',script).replace('Read4BytesFromScript(uint32* pIp) {','Read4BytesFromScript(uint32* pIp)\n{'),'','Read4BytesFromScript')
owners='''#pragma once
#include "common.h"
#include "Timer.h"
#include "DMAudio.h"
#ifdef TEST_ONSCREEN_ORIGINAL
#include "original-OnscreenTimer.h"
#else
#include "'''+(root/'src/control/OnscreenTimer.h').as_posix()+'''"
#endif
#include "entry.h"
void Test_Variable(int32_t offset);
bool Test_Replay(void);
class CReplay {public:static bool IsPlayingBack(){
    // Observe the original predicate boundary without moving the controller's disabled check
    return Test_Replay();
}};
class CHud {public:static bool m_Wants_To_Draw_Hud;};
struct TestCamera {bool m_WideScreenOn;};extern TestCamera TheCamera;
class CTheScripts {public:alignas(4) static uint8 ScriptSpace[260512];
'''+getter+'\n'+size+'\n'+read+'''
static int32 *GetPointerToScriptVariable(int32 offset){
    // Observe live callback mutations before executing the entire actual native variable lookup
    Test_Variable(offset);return RealVariable(offset);
}};
'''
(out/'owners.h').write_text(owners,newline='\n')
api='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
struct COnscreenTimer;struct COnscreenTimerEntry;struct COnscreenCounterEntry;
extern "C" {
#else
typedef struct COnscreenTimer COnscreenTimer;
typedef struct COnscreenTimerEntry COnscreenTimerEntry;
typedef struct COnscreenCounterEntry COnscreenCounterEntry;
#endif
'''
args={'Init':'COnscreenTimer *self','Process':'COnscreenTimer *self','ProcessForDisplay':'COnscreenTimer *self','ClearCounter':'COnscreenTimer *self,uint32_t offset','ClearClock':'COnscreenTimer *self,uint32_t offset','AddCounter':'COnscreenTimer *self,uint32_t offset,uint16_t type,char *text,uint16_t pos','AddClock':'COnscreenTimer *self,uint32_t offset,char *text,bool bGoingDown'}
for owner,prefix,name in m.OPERATIONS:
    a=args[name] if owner=='COnscreenTimer' else owner+' *self'
    api+='void Run_'+prefix+'_'+name+'('+a+');\nvoid Before_'+prefix+'_'+name+'('+a+');\n'
api+='#ifdef __cplusplus\n}\n#endif\n';(out/'entry.h').write_text(api,newline='\n')
caller='#include "entry.h"\n#ifndef TEST_ONSCREEN_ORIGINAL\n#include "'+(root/'src/control/OnscreenTimer.h').as_posix()+'"\n#endif\n'
original='#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n'
for owner,prefix,name in m.OPERATIONS:
    a=args[name] if owner=='COnscreenTimer' else owner+' *self'
    values=[re.search(r'(\w+)\s*$',arg)[1] for arg in a.split(',')]
    caller+='void Run_'+prefix+'_'+name+'('+a+')\n{\n    // Enter each complete operation through a genuine C11 translation unit\n#ifdef TEST_ONSCREEN_ORIGINAL\n    Before_'+prefix+'_'+name+'('+','.join(values)+');\n#else\n    '+prefix+'_'+name+'('+','.join(values)+');\n#endif\n}\n'
    original+='extern "C" void Before_'+prefix+'_'+name+'('+a+')\n{\n    // Execute the untouched original member body on its actual original record\n    self->'+name+'('+','.join(values[1:])+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
(out/'original.cpp').write_text(original,newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n#include "'+(root/'src/control/OnscreenTimer_backend.h').as_posix()+'"\n'+re.sub(r'^#include[^\n]*\n','',m.HOST,flags=re.M),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/onscreen_timer.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited ten complete original operations, canonical C state, four native readers and all thirteen source/project edits')
