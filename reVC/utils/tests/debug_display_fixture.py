"""Prepare complete original/C debug-display policies with live native font and renderer models."""
from pathlib import Path
import re,json,sys
import debug_display_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/debug-display-c-tests';saved=out/'before';preview=out/'preview'
if '--production' in sys.argv:
    for path in json.loads((out/'changes.json').read_text()):assert (root/path).read_text()==m.calls((saved/path).read_text()),path
    for name in ['DebugDisplaySystem.c','Debug_backend.h']:assert (root/'src/core'/name).read_text()==(preview/name).read_text(),name
for name,path in [('FontStyles.h','src/renderer/FontStyles.h'),('Lines.h','src/renderer/Lines.h')]:(out/name).write_text((saved/path).read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n',newline='\n')
(out/'original-Debug.h').write_text((saved/'src/core/Debug.h').read_text().replace('class CDebug\n{','class CDebug\n{\npublic:'),newline='\n')
font=(saved/'src/renderer/Font.cpp').read_text();asciiBody=re.search(r'AsciiToUnicode\(const char \*src, wchar \*dst\)\s*(\{[^}]+\})',font)[1]
owners='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
typedef int16_t int16;typedef int32_t int32;typedef uint32_t uint32;typedef uint16_t wchar;
#define nil NULL
#define FALSE 0
struct CVector {float x,y,z;CVector(){} CVector(float a,float b,float c):x(a),y(b),z(c){}};
struct CRGBA {uint8_t r,g,b,a;CRGBA(uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha):r(red),g(green),b(blue),a(alpha){}};
#include "FontStyles.h"
#include "Lines.h"
class CFont {public:
@FONT@
static void SetScale(float,float);static void SetRightJustifyWrap(float);static void SetWrapx(float);static void SetFontStyle(int16);static void SetColor(CRGBA);static void PrintString(float,float,wchar*);
};
extern uint16_t gUString[256];void AsciiToUnicode(const char*,wchar*);void ObrsPrintfString(const char*,short,short);
enum {rwRENDERSTATETEXTURERASTER=1,rwRENDERSTATEZTESTENABLE=2};bool RwRenderStateSet(int,void*);
#ifdef TEST_DEBUG_DISPLAY_ORIGINAL
#include "original-Debug.h"
#define GLOBAL(name) CDebug::name
typedef CDebug::ScreenStr Debug_ScreenStr;typedef CDebug::Line Debug_Line;
#else
#include "@PREVIEW@/Debug.h"
#define GLOBAL(name) Debug_##name
#endif
#include "entry.h"
'''.replace('@PREVIEW@',preview.as_posix()).replace('@FONT@',''.join('static void '+name+'();\n' for kind,name,args,code in m.LEAVES if args=='void' and name not in ['Unicode','TextureNone','DepthTestOff']))
(out/'owners.h').write_text(owners,newline='\n');(out/'common.h').write_text('#pragma once\n#include "owners.h"\n',newline='\n')
api='''#pragma once
#include <stdint.h>
typedef struct DebugTestVector {float x,y,z;} DebugTestVector;
#ifdef __cplusplus
extern "C" {
#endif
'''
signatures=[('DebugInitTextBuffer','void',[]),('DebugAddText','const char *str',['str']),('DebugDisplayTextBuffer','void',[]),('DisplayScreenStrings','void',[]),('PrintAt','const char *str,int x,int y',['str','x','y']),('AddLine','DebugTestVector p1,DebugTestVector p2,uint32_t c1,uint32_t c2',['p1','p2','c1','c2']),('DrawLines','void',[])]
original='#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/core/Debug.cpp').read_text(),flags=re.M)+'\n'
caller='#include "entry.h"\n#ifndef TEST_DEBUG_DISPLAY_ORIGINAL\n#include "'+(preview/'Debug.h').as_posix()+'"\n#endif\n'
for name,args,values in signatures:
    api+='void Before_Debug_'+name+'('+args+');\nvoid Run_Debug_'+name+'('+args+');\n'
    native=','.join('CVector('+v+'.x,'+v+'.y,'+v+'.z)' if name=='AddLine' and v in ['p1','p2'] else v for v in values)
    plain=','.join('(Debug_Vector){'+v+'.x,'+v+'.y,'+v+'.z}' if name=='AddLine' and v in ['p1','p2'] else v for v in values)
    original+='extern "C" void Before_Debug_'+name+'('+args+')\n{\n    // Invoke the entire original operation with its original native argument copies\n    CDebug::'+name+'('+native+');\n}\n'
    caller+='void Run_Debug_'+name+'('+args+')\n{\n    // Exercise the complete public operation through a standalone strict C11 caller\n#ifdef TEST_DEBUG_DISPLAY_ORIGINAL\n    Before_Debug_'+name+'('+','.join(values)+');\n#else\n    Debug_'+name+'('+plain+');\n#endif\n}\n'
api+='#ifdef __cplusplus\n}\n#endif\n';(out/'entry.h').write_text(api,newline='\n');(out/'caller.c').write_text(caller,newline='\n');(out/'original.cpp').write_text(original,newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n#include "'+(preview/'Debug_backend.h').as_posix()+'"\n'+re.sub(r'^#include[^\n]*\n','',m.host(),flags=re.M),newline='\n');(out/'kernel.c').write_text((preview/'DebugDisplaySystem.c').read_text(),newline='\n')
fixture=(root/'utils/tests/debug_display.cpp.in').read_text().replace('@ASCII_BODY@',asciiBody)
(out/'fixture.cpp').write_text(fixture,newline='\n')
print('Prepared all seven original/C queue operations, every state byte, native font effects and the actual Unicode conversion body')
