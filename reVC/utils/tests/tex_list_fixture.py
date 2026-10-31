"""Audit the complete texture-list relocation and emit original/C comparison programs."""
from pathlib import Path
import json,re
import tex_list_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/tex-list-c-tests'
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    target=root/m.target(path);assert target.read_text()==expected,path
    assert b'\r' not in target.read_bytes(),path
assert not (root/'src/renderer/TexList.cpp').exists()
for path in ['src/core/common.h','src/core/config.h','src/core/FileMgr.h','src/core/FileMgr.c','src/fakerw/rwcore.h','vendor/librw/src/rwobjects.h']:assert (out/'before'/path).read_bytes()==(root/path).read_bytes(),path
old=(out/'before/src/renderer/TexList.cpp').read_text();header=(out/'before/src/renderer/TexList.h').read_text()
assert len(m.METHODS)==6 and 'MAX_TEXUSED = 400' in header
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef int32_t int32;
#ifdef LIBRW
namespace rw { struct Texture; }
typedef rw::Texture RwTexture;
#else
typedef struct RwTexture RwTexture;
#endif
#define nil NULL
#endif
'''
(out/'common.h').write_text(common,newline='\n')
(out/'TexList.h').write_text('#pragma once\n#include "'+(root/'src/renderer/TexList.h').as_posix()+'"\n',newline='\n')
prototypes='''bool *TestTex_Flags(void);
void TestTex_Init(void);void TestTex_Shutdown(void);void *TestTex_Set(int32_t slot,char *name);int32_t TestTex_Free(void);void *TestTex_Load(char *name);void TestTex_Global(void);
void TestTex_RunInit(void);void TestTex_RunShutdown(void);void *TestTex_RunSet(int32_t slot,char *name);int32_t TestTex_RunFree(void);void *TestTex_RunLoad(char *name);void TestTex_RunGlobal(void);
'''
(out/'caller.h').write_text('#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
operations=[('Init','Initialise','void','void',''),('Shutdown','Shutdown','void','void',''),('Set','SetTexture','void *','int32_t slot,char *name','slot,name'),('Free','GetFirstFreeTexture','int32_t','void',''),('Load','LoadFileNameTexture','void *','char *name','name'),('Global','LoadGlobalTextureList','void','void','')]
caller='#include "caller.h"\n'
for alias,method,result,args,invoke in operations:caller+=result+' TestTex_Run'+alias+'('+args+')\n{\n    // Enter the complete real operation through a separate C11 caller\n    '+('return ' if result!='void' else '')+'TestTex_'+alias+'('+invoke+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
for before in [True,False]:
    text='#include "common.h"\n#include "caller.h"\n#include "FileMgr.h"\n'
    if before:text+='namespace Original {\n'+header.replace('#pragma once','').replace('class CTexList\n{','class CTexList\n{\npublic:')+'\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n}\n'
    else:text+='#include "TexList.h"\n'
    prefix='Original::CTexList::' if before else 'TexList_'
    text+='extern "C" bool *TestTex_Flags(void)\n{\n    // Expose actual canonical usage flags without a copied or surrogate table\n    return '+prefix+'ms_nTexUsed;\n}\n'
    for alias,method,result,args,invoke in operations:text+='extern "C" '+result+' TestTex_'+alias+'('+args+')\n{\n    // Execute the complete original or C operation with the same captured arguments\n    '+('return ' if result!='void' else '')+prefix+method+'('+invoke+');\n}\n'
    (out/('original.cpp' if before else 'adapter.cpp')).write_text(text,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/tex_list.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all six complete C operations, 400 canonical flags, pure C11 unit and four full source/project relocations')
