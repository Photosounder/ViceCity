"""Audit the complete registry relocation and emit original/C scalar and model-owner oracles."""
from pathlib import Path
import json,re
import model_indices_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/model-indices-c-tests'
header=(out/'before/src/modelinfo/ModelIndices.h').read_text();original=(out/'before/src/modelinfo/ModelIndices.cpp').read_text()
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
for name,text in [('ModelIndexData.h',m.data_header(header)),('ModelIndexData.c',m.kernel(original,header)),('ModelIndices_backend.h',m.BACKEND)]:assert (root/'src/modelinfo'/name).read_text()==text,name
for path in ['src/modelinfo/ModelIds.h','src/modelinfo/ModelInfo.h','src/modelinfo/ModelInfo.cpp','src/modelinfo/SimpleModelInfo.h','src/modelinfo/BaseModelInfo.h','src/core/General.c','src/core/General.h','src/core/config.h','src/core/common.h']:assert (out/'before'/path).read_bytes()==(root/path).read_bytes(),path
names=re.findall(r'(?m)^\s*X\("([^"]+)",\s*(\w+)\)',header);assert len(names)==136 and len({name for name,var in names})==133 and len({var for name,var in names})==136
classifiers=m.helper_names(header)
(out/'symbols.json').write_text(json.dumps(names,indent=2),newline='\n');(out/'classifiers.json').write_text(json.dumps(classifiers,indent=2),newline='\n')
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include "config.h"
typedef int16_t int16;
#endif
'''
(out/'common.h').write_text(common,newline='\n')
(out/'ModelInfo.h').write_text('''#pragma once
#include <stdint.h>
class CBaseModelInfo;
extern "C" bool TestModel_Building(CBaseModelInfo *model);
struct ModelPrefix { uint64_t guard[2]; };
class CBaseModelInfo { public:int identity;bool building;bool IsBuilding(void){return TestModel_Building(this);} };
class CSimpleModelInfo:public ModelPrefix,public CBaseModelInfo { public:uint16_t m_isCodeGlass:1;uint16_t m_isArtistGlass:1; };
class CModelInfo { public:static CBaseModelInfo *GetModelInfo(int id); };
''',newline='\n')
(out/'owners.h').write_text('#pragma once\n#include "common.h"\n#include "ModelInfo.h"\n#include "ModelIndexData.h"\n#include "General.h"\n',newline='\n')
for name in ['ModelIndexData.h','ModelIndices_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/modelinfo'/name).as_posix()+'"\n',newline='\n')
prototypes='''void TestModel_Init(void);void TestModel_Match(const char *name,int16_t id);void TestModel_Test(void);
bool TestModel_Classify(int which,int16_t id);int16_t *TestModel_Variable(unsigned index);const char *TestModel_Name(unsigned index);
void TestModel_RunInit(void);void TestModel_RunMatch(const char *name,int16_t id);void TestModel_RunTest(void);bool TestModel_RunClassify(int which,int16_t id);
'''
(out/'caller.h').write_text('#pragma once\n#include "ModelIndexData.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
oldheader=re.sub(r'^#(?:pragma|include)[^\n]*\n','',header,flags=re.M)
(out/'original-declarations.h').write_text(oldheader,newline='\n')
for before in [True,False]:
    text='#include "owners.h"\n#include "caller.h"\n'
    if before:text+='namespace Original {\n'+oldheader+'\n'+re.sub(r'^#include[^\n]*\n','',original,flags=re.M)+'\n}\n'
    prefix='Original::' if before else '';text+='extern "C" {\n'
    for alias,target,args,invoke in [('Init','InitModelIndices','void',''),('Match','MatchModelString','const char *name,int16_t id','name,id'),('Test','TestModelIndices','void','')]:text+='void TestModel_'+alias+'('+args+')\n{\n    // Enter the complete actual original or C registry operation\n    '+prefix+target+'('+invoke+');\n}\n'
    text+='bool TestModel_Classify(int which,int16_t id)\n{\n    // Enter every complete original inline or actual C classification operation\n    switch(which){\n'
    for i,name in enumerate(classifiers):text+='    case '+str(i)+':return '+prefix+name+'(id);\n'
    text+='    default:abort();\n    }\n}\n'
    text+='int16_t *TestModel_Variable(unsigned index)\n{\n    // Borrow each actual canonical global variable address without a shadow registry\n    static int16_t *variables[]={\n#define X(name,var) &'+prefix+'var,\nMODELINDICES\n#undef X\n    };return variables[index];\n}\n'
    text+='const char *TestModel_Name(unsigned index)\n{\n    // Retain every original name and its exact macro declaration order\n    static const char *names[]={\n#define X(name,var) name,\nMODELINDICES\n#undef X\n    };return names[index];\n}\n}\n'
    (out/('original.cpp' if before else 'adapter.cpp')).write_text(text,newline='\n')
caller='#include "caller.h"\n'
for alias,result,args,invoke in [('Init','void','void',''),('Match','void','const char *name,int16_t id','name,id'),('Test','void','void',''),('Classify','bool','int which,int16_t id','which,id')]:caller+=result+' TestModel_Run'+alias+'('+args+')\n{\n    // Retain the actual signed scalar and borrowed string through a separate C11 caller\n    '+('return ' if result!='void' else '')+'TestModel_'+alias+'('+invoke+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n');(out/'host.cpp').write_text(m.HOST,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/model_indices.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all 136 name/variable pairs, all eighteen complete operations, four typed native reads and four complete source/project files')
