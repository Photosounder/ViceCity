"""Compare the complete original text owner with its staged or production C owner."""
from pathlib import Path
import re,sys,json
import text_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/text-c-tests'
production='--production' in sys.argv
include=root/'src/text' if production else out/'preview'
language=root/'src/core' if production else out/'preview'
old=(out/'before/src/text/Text.cpp').read_text();header=(out/'before/src/text/Text.h').read_text()
if production:
    for path in json.loads((out/'changes.json').read_text()):
        saved=(out/'before'/path).read_text();assert (root/path).read_text()==(m.project(saved) if path.startswith('build/') else m.calls(saved)),path
        assert b'\r' not in (root/path).read_bytes(),path
    assert (include/'TextSystem.c').read_text()==m.kernel(old,header)
    assert (include/'Text_backend.h').read_text()==m.BACKEND
    assert (language/'TextLanguages.h').read_text()==m.language_header((out/'before/src/core/Frontend.h').read_text())
(out/'original-Text.h').write_text(header.replace('class CText\n{','class CText\n{\npublic:'),newline='\n')
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
typedef int16_t int16;typedef int32_t int32;typedef uintptr_t uintptr;typedef uint16_t wchar;
#define nil NULL
#define VALIDATE_SIZE(type,size)
#endif
'''
(out/'common.h').write_text(common,newline='\n')
owners='''#pragma once
#include "common.h"
#include <new>
#include "FileMgr.h"
#include "Timer.h"
#include "Messages.h"
#ifdef TEST_TEXT_ORIGINAL
#include "original-Text.h"
#else
#include "'''+(include/'Text.h').as_posix()+'''"
#endif
struct TextObservedInteger {int value;operator int()const;TextObservedInteger &operator=(int);};
struct TextObservedFlag {bool value;operator bool()const;TextObservedFlag &operator=(bool);};
class CMenuManager {public:
'''+m.language_block((out/'before/src/core/Frontend.h').read_text())+'''
TextObservedInteger m_PrefsLanguage;};
extern CMenuManager FrontEndMenuManager;
class CGame {public:static TextObservedFlag russianGame;static TextObservedFlag japaneseGame;};
extern wchar WideErrorString[25];extern wchar UpperCaseTable[128];extern wchar FrenchUpperCaseTable[128];
#include "entry.h"
'''
(out/'owners.h').write_text(owners,newline='\n')
api='#pragma once\n#include <stdint.h>\n#include <stddef.h>\n#include "config.h"\n#ifndef __cplusplus\ntypedef struct CText CText;typedef struct CKeyArray CKeyArray;typedef struct CData CData;typedef struct CKeyEntry CKeyEntry;typedef struct ChunkHeader ChunkHeader;typedef struct CMissionTextOffsets CMissionTextOffsets;\n#endif\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
caller='#include "'+(include/'Text.h').as_posix()+'"\n#include "entry.h"\n'
original='#include "owners.h"\n#include "allocation.h"\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n'
signatures=[]
for match in re.finditer(r'(?m)^(void|uint16_t \*|uint16_t|CKeyEntry \*|char \*)\s*((?:Text|TextKeys|TextData|TextOffsets)_\w+|UnicodeToAscii\w*|TextCopy)\(([^\n]*)\);',m.header()):
    ret,name,args=match.groups();signatures.append((ret,name,args))
assert len(signatures)==26 # Both conditional Search declarations are retained
for ret,name,args in signatures:
    condition=''
    if name=='TextKeys_Search':condition='#if defined(FIX_BUGS) || defined(FIX_BUGS_64)\n' if 'uint16_t *data' in args else '#if !defined(FIX_BUGS) && !defined(FIX_BUGS_64)\n'
    values=[re.search(r'(\w+)\s*$',arg)[1] for arg in args.split(',')];tail=','.join(values[1:]);isfree=name in m.FREE
    if isfree:expression=name+'('+','.join(values)+')'
    elif name.endswith('_Init'):
        owner={'Text':'CText','TextKeys':'CKeyArray','TextData':'CData','TextOffsets':'CMissionTextOffsets'}[name[:-5]];expression='new (self) '+owner+'()'
    elif name=='Text_Destroy':expression='self->~CText()'
    else:expression='self->'+name.split('_',1)[1]+'('+tail+')'
    api+=condition+ret+' Run_'+name+'('+args+');\n'+ret+' Before_'+name+'('+args+');\n'+('#endif\n' if condition else '')
    original+=condition+'extern "C" '+ret+' Before_'+name+'('+args+')\n{\n    // Enter the complete original operation on its actual receiver and owned buffers\n    '+('return ' if ret!='void' else '')+expression+';\n}\n'+('#endif\n' if condition else '')
    caller+=condition+ret+' Run_'+name+'('+args+')\n{\n    // Invoke the full operation through a standalone C11 translation unit\n#ifdef TEST_TEXT_ORIGINAL\n    '+('return ' if ret!='void' else '')+'Before_'+name+'('+','.join(values)+');\n#else\n    '+('return ' if ret!='void' else '')+name+'('+','.join(values)+');\n#endif\n}\n'+('#endif\n' if condition else '')
api+='#ifdef __cplusplus\n}\n#endif\n'
(out/'entry.h').write_text(api,newline='\n');(out/'caller.c').write_text(caller,newline='\n');(out/'original.cpp').write_text(original,newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n#include "'+(include/'Text_backend.h').as_posix()+'"\n#include "allocation.h"\n'+re.sub(r'^#include[^\n]*\n','',m.HOST,flags=re.M),newline='\n')
(out/'allocation.h').write_text('''#pragma once
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
void *TestTextMalloc(size_t size);
void TestTextFree(void *pointer);
#ifdef __cplusplus
}
#endif
#undef malloc
#undef free
#define malloc TestTextMalloc
#define free TestTextFree
''',newline='\n')
(out/'kernel.c').write_text('#include "allocation.h"\n#include "'+(include/'TextSystem.c').as_posix()+'"\n',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/text.cpp.in').read_text(),newline='\n')
for name,text in [('classic','#include "config.h"\n#undef MORE_LANGUAGES\n'),('unfixed','#include "config.h"\n#undef FIX_BUGS\n'),('byte','#include "config.h"\n#define THIS_IS_STUPID\n')]: (out/(name+'.h')).write_text(text,newline='\n')
print('Prepared complete original/C text operations and allocation/lifecycle observation:', 'production' if production else 'staged')
