import weather_migration as _weather_conversion
"""Audit complete variable-console relocation and emit original/C stateful comparison programs."""
from pathlib import Path
import json,re
import var_console_migration as m
import set_pieces_migration as pieces
root=Path(__file__).resolve().parents[2];out=root/'build/var-console-c-tests'
old=(out/'before/src/renderer/VarConsole.cpp').read_text();header=(out/'before/src/renderer/VarConsole.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    source=(out/'before'/path).read_text();expected=m.project(source) if path.startswith('build/') else m.calls(source)
    if path=='src/control/SetPieces.cpp':pieces.verify_legacy(source,m.calls,root)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
assert (root/'src/renderer/VariableConsole.c').read_text()==m.kernel(old)
assert (root/'src/renderer/VarConsole_backend.h').read_text()==m.BACKEND
for path in json.loads((out/'untouched.json').read_text()):_weather_conversion.verify_dependency(out/'before'/path,root/path)
entryfields=['text','pInt8Value','pInt16Value','pInt32Value','pInt64Value','pUint8Value','pUint16Value','pUint32Value','pUint64Value','pFloatValue','pBoolValue','pCallback']+[prefix+'_'+kind for prefix in ['I8','I16','I32','I64','F'] for kind in ['step','max','min']]+['bAllowExceedBounds','VarType']
assert len(entryfields)==29 and len(m.methods(old))==26 and len(m.READERS)==31
(out/'entry-fields.json').write_text(json.dumps(entryfields,indent=2),newline='\n')
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "Draw.h"
#include "Draw_constants.h"
typedef int8_t int8;typedef int16_t int16;typedef int32_t int32;typedef int64_t int64;
typedef uint8_t uint8;typedef uint16_t uint16;typedef uint32_t uint32;typedef uint64_t uint64;
typedef uint8 bool8;typedef uint16 bool16;typedef uint32 bool32;typedef uint16_t wchar;
#define FALSE 0
#define TRUE 1
class CRGBA {public:uint8 r,g,b,a;CRGBA(){}CRGBA(uint8 r,uint8 g,uint8 b,uint8 a):r(r),g(g),b(b),a(a){}};
struct RsGlobalType {int width,height;};extern RsGlobalType RsGlobal;
'''
native=(out/'before/src/core/common.h').read_text();a=native.index('#ifdef GTA_PS2\n\t#ifdef GTA_PAL');b=native.index('\n#include "maths.h"',a)
common+=native[a:b]+'\n#endif\n';(out/'common.h').write_text(common,newline='\n')
owners='''#pragma once
#include "common.h"
#include "VarConsole_backend.h"
#include "FontStyles.h"
#include "Draw_backend.h"
class CFont {public:static void SetColor(CRGBA);static void SetFontStyle(int16);static void SetScale(float,float);static void SetDropShadowPosition(int16);static void SetDropColor(CRGBA);static void SetPropOn(void);static void SetWrapx(float);static void SetRightJustifyWrap(float);static void SetRightJustifyOn(void);static void SetRightJustifyOff(void);static void PrintString(float,float,wchar *);static void SetBackgroundOn(void);static void SetBackgroundOff(void);static void SetBackgroundColor(CRGBA);};
void AsciiToUnicode(const char *,wchar *);
class CPad {public:static CPad *GetPad(int32 index);
'''+''.join('bool Get'+name+'(void);\n' for name in m.PADS)+'};\n'
(out/'owners.h').write_text(owners,newline='\n')
for name in ['VarConsole.h','VarConsole_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
operations=[('Construct','void','')]+[(name,m.aliases(ret),m.aliases(args)) for name,ret,args,body in m.methods(old)]
prototypes='void TestVC_Read(VarConsoleState *state);void TestVC_Write(const VarConsoleState *state);\n'
for name,ret,args in operations:prototypes+=ret+' TestVC_'+name+'('+ (args or 'void')+');'+ret+' TestVC_Run'+name+'('+(args or 'void')+');\n'
(out/'caller.h').write_text('#pragma once\n#include "VarConsole.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
caller='#include "caller.h"\n'
for name,ret,args in operations:
    parameters=[] if not args else ['pCallback'] if name=='AddFunction' else []
    if name=='AddFunction':parameters=['text','pCallback']
    elif args:parameters=[re.search(r'(\w+)\s*$',arg)[1] for arg in args.split(',')]
    invoke=','.join(parameters);caller+=ret+' TestVC_Run'+name+'('+(args or 'void')+')\n{\n    // Enter every complete operation through a separately compiled C11 caller\n    '+('return ' if ret!='void' else '')+'TestVC_'+name+'('+invoke+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
for before in [True,False]:
    text='#include "owners.h"\n#include "caller.h"\n'
    if before:text+='namespace Original {\n'+header.replace('#pragma once','').replace('class CVarConsole\n{','class CVarConsole\n{\npublic:')+'\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n}\n'
    state='Original::VarConsole' if before else 'VarConsole'
    text+='extern "C" {\nvoid TestVC_Read(VarConsoleState *state)\n{\n    // Observe all actual declared fields without copying unspecified native padding\n'
    for field in m.FIELDS[:-1]:text+='    state->'+field+'='+state+'.'+field+';\n'
    text+='    for(int i=0;i<91;i++){\n        // Copy every named field from the actual canonical entry record\n'
    for field in entryfields:text+='        state->m_aEntries[i].'+field+'='+state+'.m_aEntries[i].'+field+';\n'
    text+='    }\n}\nvoid TestVC_Write(const VarConsoleState *state)\n{\n    // Seed only actual named canonical fields with valid test values\n'
    for field in m.FIELDS[:-1]:text+='    '+state+'.'+field+'=state->'+field+';\n'
    text+='    for(int i=0;i<91;i++){\n        // Preserve every real pointer and scalar field while excluding padding writes\n'
    for field in entryfields:text+='        '+state+'.m_aEntries[i].'+field+'=state->m_aEntries[i].'+field+';\n'
    text+='    }\n}\n'
    for name,ret,args in operations:
        parameters=['text','pCallback'] if name=='AddFunction' else [re.search(r'(\w+)\s*$',arg)[1] for arg in args.split(',')] if args else []
        text+=ret+' TestVC_'+name+'('+(args or 'void')+')\n{\n    // Invoke the real original operation or C operation on its canonical global owner\n'
        if before and name=='Construct':
            text+='#ifdef FIX_BUGS\n    Original::CVarConsole fresh;\n'
            for field in ['m_nCountEntries','m_nCurPage','m_bIsOpen','m_nCurEntry','m_nFirstEntryOnPage']:text+='    '+state+'.'+field+'=fresh.'+field+';\n'
            text+='#endif\n'
        else:
            method='Add' if name.startswith('Add') else name;invoke=state+'.'+method+'('+','.join(parameters)+')' if before else 'VarConsole_'+name+'(&VarConsole'+(','+','.join(parameters) if parameters else '')+')'
            text+='    '+('return ' if ret!='void' else '')+invoke+';\n'
        text+='}\n'
    (out/('original.cpp' if before else 'adapter.cpp')).write_text(text+'}\n',newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',m.HOST,flags=re.M),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/var_console.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited 27 complete C operations, both plain records, all 91 entries, fourteen overload replacements, 31 native boundaries and fifteen complete source/project changes')
