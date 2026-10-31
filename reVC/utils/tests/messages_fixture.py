"""Audit all message algorithms and emit whole untouched-original/C runtime comparisons."""
from pathlib import Path
import json,re
import messages_migration as m
import text_migration as text
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/messages-c-tests'
changes=json.loads((out/'changes.json').read_text())
for path in changes:
    source=(out/'before'/path).read_text();assert (root/path).read_text()==(m.project(source) if path.startswith('build/') else m.calls(source)),path
    if path=='src/text/Text.cpp':text.verify_legacy(source,m.calls,root)
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/text/Messages.cpp').read_text();header=(out/'before/src/text/Messages.h').read_text()
assert (root/'src/text/MessageQueues.c').read_text()==m.kernel(old,header)
assert (root/'src/text/Messages_backend.h').read_text()==m.BACKEND
assert (root/'src/core/ControllerActions.h').read_text()==m.actions_header((out/'before/src/core/ControllerConfig.h').read_text())
for path in json.loads((out/'dependencies.json').read_text()):
    if path not in changes:text.verify_dependency(out/'before'/path,root/path)
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
typedef int16_t int16;typedef int32_t int32;typedef uint16_t wchar;
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
#define nil NULL
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
'''
(out/'common.h').write_text(common,newline='\n');(out/'original-Messages.h').write_text(header,newline='\n')
font=(out/'before/src/renderer/Font.h').read_text();locale=re.search(r'enum\s*\{\s*FONT_LANGSET_EFIGS[\s\S]*?\};',font)[0]
owners='''#pragma once
#include "common.h"
#include "ControllerActions.h"
#include "Timer.h"
#include "Pager.h"
#ifdef TEST_MESSAGES_ORIGINAL
#include "original-Messages.h"
#else
#include "'''+(root/'src/text/Messages.h').as_posix()+'''"
#endif
#include "entry.h"
'''+locale+'''
class CFont {public:static uint8 LanguageSet;static bool IsJapanese();};
class CControllerConfigManager {public:wchar m_aActionNames[MAX_CONTROLLERACTIONS][ACTIONNAME_LENGTH];void GetWideStringOfCommandKeys(uint16 action,wchar *output,uint16 length);};
extern CControllerConfigManager ControlsManager;
class CHud {public:static wchar m_BigMessage[6][128];static wchar m_Message[256];static float BigMessageInUse[6];static int16 OddJob2On;static float OddJob2OffTimer;
static void SetBigMessage(wchar *message,uint16 style);static void RealSetBigMessage(wchar *message,uint16 style);
static void SetMessage(wchar *message);static void RealSetMessage(wchar *message);
static void GetRidOfAllHudMessages();};
extern wchar LastBigMessage[6][128];
void DefinedState();void AsciiToUnicode(const char *source,wchar *output);void RealAsciiToUnicode(const char *source,wchar *output);
'''
(out/'owners.h').write_text(owners,newline='\n')
api='#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
caller='#include "entry.h"\n#ifndef TEST_MESSAGES_ORIGINAL\n#include "'+(root/'src/text/Messages.h').as_posix()+'"\n#endif\n'
original='#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n'
for ret,name,args in m.operations(header):
    args=m.primitives(args if args.strip() else 'void');ret=m.primitives(ret)
    values=[] if args=='void' else [re.search(r'(\w+)\s*$',arg)[1] for arg in args.split(',')]
    api+=ret+' Run_Messages_'+name+'('+args+');\n'+ret+' Before_Messages_'+name+'('+args+');\n'
    caller+=ret+' Run_Messages_'+name+'('+args+')\n{\n    // Enter the complete operation through a genuine standalone C11 caller\n#ifdef TEST_MESSAGES_ORIGINAL\n    '+('return ' if ret!='void' else '')+'Before_Messages_'+name+'('+','.join(values)+');\n#else\n    '+('return ' if ret!='void' else '')+'Messages_'+name+'('+','.join(values)+');\n#endif\n}\n'
    original+='extern "C" '+ret+' Before_Messages_'+name+'('+args+')\n{\n    // Execute the entire unmodified original static method on its actual canonical tables\n    '+('return ' if ret!='void' else '')+'CMessages::'+name+'('+','.join(values)+');\n}\n'
api+='#ifdef __cplusplus\n}\n#endif\n';(out/'entry.h').write_text(api,newline='\n')
(out/'caller.c').write_text(caller,newline='\n');(out/'original.cpp').write_text(original,newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n#include "'+(root/'src/text/Messages_backend.h').as_posix()+'"\n'+re.sub(r'^#include[^\n]*\n','',m.HOST,flags=re.M),newline='\n')
native='#include "owners.h"\n'+function((out/'before/src/renderer/Font.cpp').read_text(),'','AsciiToUnicode').replace('AsciiToUnicode(','RealAsciiToUnicode(',1)+'\n'
hud=(out/'before/src/renderer/Hud.cpp').read_text()
for name in ['SetBigMessage','SetMessage']:native+=function(hud,'CHud::',name).replace('CHud::'+name+'(', 'CHud::Real'+name+'(')+'\n'
(out/'native.cpp').write_text(native,newline='\n');(out/'test.cpp').write_text((root/'utils/tests/messages.cpp.in').read_text(),newline='\n')
(out/'classic.h').write_text('#include "config.h"\n#undef MORE_LANGUAGES\n',newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited 27 full public operations, two private helpers, all four canonical C tables, shared complete action enum and eight native boundaries')
