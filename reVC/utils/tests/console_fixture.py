"""Audit complete console relocation and compare real original and C record behavior."""
from pathlib import Path
import re,json
import console_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/console-c-tests'

def normalize(source):
    # Compare all complete statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/Console.cpp').read_text();header=(out/'before/src/renderer/Console.h').read_text()
for name,value in [('Console.c',m.kernel(old,header)),('Console.cpp',m.HOST),('Console.h',m.HEADER),('Console_backend.h',m.BACKEND_HEADER)]:assert (root/'src/renderer'/name).read_text()==value
assert len(re.findall(r'\nCConsole::\w+\(',m.section(old)))==3 and 'void Init()' in header
for path in ['src/core/common.h','src/core/config.h','src/renderer/Font.h']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
portable=(root/'build/special-fx-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(portable,newline='\n')
assert saved.read_text()==portable
(out/'common.h').write_text(portable,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
(out/'console-view.h').write_text('#pragma once\n#include <stdint.h>\n'+m.TYPE,newline='\n')
(out/'original-Console.h').write_text(header.replace('class CConsole\n{','class CConsole\n{\npublic:'),newline='\n')
owners='''#pragma once
#include "Timer.h"
#include "FontStyles.h"
class CFont { public:
static void SetScale(float,float);static void SetColor(CRGBA);static void PrintString(float,float,wchar *);
static void SetFontStyle(int16);static void SetRightJustifyWrap(float);static void SetWrapx(float);
static void SetPropOn(void);static void SetBackgroundOff(void);static void SetCentreOff(void);static void SetRightJustifyOff(void);
static void SetJustifyOn(void);static void SetBackGroundOnlyTextOff(void);static void SetPropOff(void);
};
'''
(out/'owners.h').write_text(owners,newline='\n');(out/'Font.h').write_text('#pragma once\n#include "owners.h"\n',newline='\n')
prototypes='''Console TestCo_Get(int32_t index);void TestCo_Set(int32_t index,const Console *record);
void *TestCo_Row(int32_t index,int32_t row);
void TestCo_Init(int32_t index);void TestCo_Display(int32_t index);
void TestCo_Add(int32_t index,char *text,uint8_t r,uint8_t g,uint8_t b,bool split);
void TestCo_Alias(int32_t index,int32_t row,int32_t offset,bool split);
void TestCo_Printf(int32_t pattern);
'''
(out/'caller.h').write_text('#pragma once\n#include <stdbool.h>\n#ifdef TEST_ORIGINAL\n#include "console-view.h"\n#else\n#include "Console.h"\n#endif\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
printf='''void TestCo_Printf(int32_t pattern)
{
    // Enter the original variadic function with identical promoted arguments from each language
    char a[]="n=%d u=%u hex=%08x text=%s c=%c f=%.2f";char b[]="wide=%lld %llu percent=%%";char c[]="%s";
    if(pattern==0)cprintf(a,-12345,4000000000u,0x1234abcdu,"console",'Q',12.375);
    else if(pattern==1)cprintf(b,-1234567890123ll,123456789012345ull);
    else{char text[230];for(int i=0;i<229;i++)text[i]=(char)('a'+i%26);text[229]=0;cprintf(c,text);}
}
'''
for original in [True,False]:
    body=('#include "common.h"\n#include "owners.h"\n#include "original-Console.h"\n#include <stdarg.h>\n'+m.section(old)+'\n') if original else '#include "Console.h"\n'
    body+='#include "caller.h"\n'+('static CConsole ExtraConsoles[2];\n' if original else 'static Console ExtraConsoles[2];\n')
    kind='CConsole' if original else 'Console'
    body+='static '+kind+' *Object(int32_t index)\n{\n    // Address actual typed instances without assuming any cross-language layout\n    return index==0?&TheConsole:&ExtraConsoles[index-1];\n}\n'
    if original:body+='extern "C" {\n'
    body+='Console TestCo_Get(int32_t index)\n{\n    // Copy all actual declared fields into the same legal C observation record\n    '+kind+' *v=Object(index);Console result;\n'
    for name in m.FIELDS:body+=('    result.'+name+'=v->'+name+';\n') if name.startswith('m_n') else '    memcpy(result.'+name+',v->'+name+',sizeof(result.'+name+'));\n'
    body+='    return result;\n}\nvoid TestCo_Set(int32_t index,const Console *record)\n{\n    // Seed named real fields while preserving the actual owner instance identity\n    '+kind+' *v=Object(index);\n'
    for name in m.FIELDS:body+=('    v->'+name+'=record->'+name+';\n') if name.startswith('m_n') else '    memcpy(v->'+name+',record->'+name+',sizeof(record->'+name+'));\n'
    body+='}\nvoid *TestCo_Row(int32_t index,int32_t row)\n{\n    // Expose only the actual legal mutable Unicode row identity\n    return Object(index)->Buffers[row];\n}\n'
    for name in ['Init','Display']:body+='void TestCo_'+name+'(int32_t index)\n{\n    // Enter the complete actual instance operation\n    '+('Object(index)->'+name+'()' if original else 'Console_'+name+'(Object(index))')+';\n}\n'
    body+='void TestCo_Add(int32_t index,char *text,uint8_t r,uint8_t g,uint8_t b,bool split)\n{\n    // Preserve the borrowed input pointer across the complete split or single-line operation\n'
    if original:body+='    if(split)Object(index)->AddLine(text,r,g,b);else Object(index)->AddOneLine(text,r,g,b);\n}\n'
    else:body+='    if(split)Console_AddLine(Object(index),text,r,g,b);else Console_AddOneLine(Object(index),text,r,g,b);\n}\n'
    body+='void TestCo_Alias(int32_t index,int32_t row,int32_t offset,bool split)\n{\n    // Borrow legal character bytes of the actual Unicode array without copying them\n    TestCo_Add(index,((char *)Object(index)->Buffers[row])+offset,31,127,241,split);\n}\n'+printf
    if original:body+='}\n'
    if not original:body='#include <string.h>\n'+body
    (out/('original.cpp' if original else 'caller.c')).write_text(body,newline='\n')
(out/'observed-kernel.c').write_text('#include "'+(root/'src/renderer/Console.c').as_posix()+'"\n_Static_assert(sizeof(Console)==700,"Original complete C record width");\n',newline='\n')
(out/'host.cpp').write_text(m.HOST,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/console.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited four complete methods, variadic formatting, all C record fields and six entire source/project files')
