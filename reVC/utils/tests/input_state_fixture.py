"""Prepare complete original input policies, native device leaves and independent C callers."""
from pathlib import Path
import re,json,sys
import input_state_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/input-state-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/name).read_text()==m.calls((saved/name).read_text()),name
    for name in ['InputState.h','InputState.c','InputState_backend.h']:assert (root/'src/core'/name).read_text()==(out/'overlay/src/core'/name).read_text(),name
for name in ['InputState.h','InputState.c','InputState_backend.h']:(out/name).write_text((tree/'src/core'/name).read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n',newline='\n')
header=(saved/'src/core/Pad.h').read_text();source=(saved/'src/core/Pad.cpp').read_text()
original='#pragma once\n#include <stdint.h>\ntypedef int16_t int16;typedef int32_t int32;\n#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))\n#define nil NULL\n'+'\n'.join(m.record(header,name) for name in m.RECORDS)+'\nextern CMousePointerStateHelper MousePointerStateHelper;\n'
(out/'original.h').write_text(original,newline='\n')
owners='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#ifdef ORIGINAL
#include "original.h"
#else
#include "InputState.h"
#endif
class TestPad {public:
static CMouseControllerState OldMouseControllerState,NewMouseControllerState,PCTempMouseControllerState;
};
''';(out/'owners.h').write_text(owners,newline='\n')
native='''#pragma once
#include "owners.h"
struct DIDEVCAPS {uint32_t dwSize,dwButtons,dwAxes;};
struct TestMouse {void GetCapabilities(DIDEVCAPS *caps);};
extern TestMouse *platformMouse;
extern void *platformWindow;
struct TestFrontend {bool m_bMenuActive;};
extern TestFrontend FrontEndMenuManager;
#define PSGLOBAL(name) platform##name
#define platformmouse platformMouse
#define platformwindow platformWindow
#ifndef nil
#define nil NULL
#endif
bool _InputMouseNeedsExclusive(void);
void _InputInitialiseMouse(bool exclusive);
void glfwGetCursorPos(void *window,double *x,double *y);
''';(out/'native.h').write_text(native,newline='\n')
ops=[('ControllerState_CheckForInput','bool','CControllerState','CheckForInput'),('ControllerState_Clear','void','CControllerState','Clear'),('KeyboardState_Clear','void','CKeyboardState','Clear'),('MouseControllerState_Init','void','CMouseControllerState',None),('MouseControllerState_Clear','void','CMouseControllerState','Clear')]+[('ControllerState_Get'+axis,'float','CControllerState','Get'+axis) for axis in ['LeftStickX','LeftStickY','RightStickX','RightStickY']]
entry='#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\nvoid TestTrace(uint32_t code,uint64_t value);\nvoid TestConfigure(bool hasMouse,bool menu,bool needsExclusive,bool createMouse,uint32_t buttons,uint32_t axes,double cursorX,bool writeCursor);\n'
original='#include "native.h"\n#include "entry.h"\nCMousePointerStateHelper MousePointerStateHelper;\n'+'\n\n'.join(m.method(source,old) for old,new,owner in m.METHODS)+'\n'
caller='#include "entry.h"\n#ifndef ORIGINAL\n#include "InputState.h"\n#endif\n'
for name,result,owner,method in ops:
    for prefix in ['Run','Before']:entry+=result+' '+prefix+'_'+name+'(void *self);\n'
    original+='extern "C" '+result+' Before_'+name+'(void *self)\n{\n    // Invoke the complete original state operation through its native type\n    '+('return ' if result!='void' else '')+(('new(self) '+owner+'();') if method is None else '(('+owner+'*)self)->'+method+'();')+'\n}\n'
    caller+=result+' Run_'+name+'(void *self)\n{\n    // Exercise the C state API through an independent strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'(self);\n#else\n    '+('return ' if result!='void' else '')+name+'(('+owner+'*)self);\n#endif\n}\n'
for prefix in ['Run','Before']:entry+='void '+prefix+'_MousePointerState_GetMouseSetUp(void *self,void *output);\n'
original+='extern "C" void Before_MousePointerState_GetMouseSetUp(void *self,void *output)\n{\n    // Return the complete original capability selection through initialized caller storage\n    *(CMouseControllerState*)output=((CMousePointerStateHelper*)self)->GetMouseSetUp();\n}\n'
caller+='void Run_MousePointerState_GetMouseSetUp(void *self,void *output)\n{\n    // Exercise the C record return ABI and preserve all original platform selection\n#ifdef ORIGINAL\n    Before_MousePointerState_GetMouseSetUp(self,output);\n#else\n    *(CMouseControllerState*)output=MousePointerState_GetMouseSetUp((CMousePointerStateHelper*)self);\n#endif\n}\n'
entry+='#ifdef __cplusplus\n}\n#endif\n'
glob=[]
for originalTree in [saved,tree]:
    text=(originalTree/'src/core/Pad.cpp').read_text();parts=[]
    for name in ['OldMouseControllerState','NewMouseControllerState','PCTempMouseControllerState']:
        definition=re.search(r'CMouseControllerState CPad::'+name+r';\n(?:static const int '+name+r'Initialized=[^\n]+\n)?',text)[0]
        parts.append('static const int Seed'+name+'=SeedGlobalMouse((void*)&TestPad::'+name+');\n'+definition.replace('CPad::','TestPad::'))
    glob.append(''.join(parts))
(out/'globals.cpp').write_text('''#include "owners.h"
static int SeedGlobalMouse(void *storage)
{
    // Modify raw allocated storage before the original or explicit initialization boundary
    memset(storage,0xa5,sizeof(CMouseControllerState));
    return 0;
}
#ifdef ORIGINAL
'''+glob[0]+'#else\n'+glob[1]+'#endif\n',newline='\n')
for name,value in [('entry.h',entry),('original.cpp',original),('caller.c',caller),('platform.cpp',(root/'utils/tests/input_state_platform.cpp.in').read_text()+m.HOST),('fixture.cpp',(root/'utils/tests/input_state.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared all ten input-state operations, six literal native leaves, partial clear contracts and global mouse initialization')
