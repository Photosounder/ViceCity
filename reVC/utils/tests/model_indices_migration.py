"""Move the complete dynamic model registry and all classification algorithms into C11."""
import re
import water_creatures_migration as water
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#ifdef __cplusplus
#include "ModelInfo.h"
#endif
#include "ModelIndexData.h"
//- rouz edit (ChatGPT)
'''
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "ModelIndexData.h"
#ifdef __cplusplus
class CSimpleModelInfo;
typedef CSimpleModelInfo ModelIndexModel;
extern "C" {
#else
typedef struct ModelIndexModel ModelIndexModel;
#endif
ModelIndexModel *ModelIndices_BackendModel(int16_t id);
bool ModelIndices_BackendBuilding(ModelIndexModel *model);
bool ModelIndices_BackendCodeGlass(const ModelIndexModel *model);
bool ModelIndices_BackendArtistGlass(const ModelIndexModel *model);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "ModelInfo.h"
#include "ModelIndices_backend.h"
ModelIndexModel *ModelIndices_BackendModel(int16_t id)
{
    // Capture the same actual model entry with the original typed native downcast
    return (CSimpleModelInfo*)CModelInfo::GetModelInfo(id);
}
bool ModelIndices_BackendBuilding(ModelIndexModel *model)
{
    // Read the original native building predicate without copying model state
    return model->IsBuilding();
}
bool ModelIndices_BackendCodeGlass(const ModelIndexModel *model)
{
    // Read the same captured model's original code-glass flag
    return model->m_isCodeGlass;
}
bool ModelIndices_BackendArtistGlass(const ModelIndexModel *model)
{
    // Read the same captured model's original artist-glass flag
    return model->m_isArtistGlass;
}
//- rouz edit (ChatGPT)
'''

def helper_names(source):
    # Retain every complete original inline classifier in declaration order
    names=re.findall(r'\binline\s+bool\s+(\w+)\(int16 id\)',source)
    assert len(names)==15 and len(names)==len(set(names)),names
    return names

def data_header(source):
    # Preserve the entire name-to-variable macro, hardcoded IDs and all public scalar contracts
    a=source.index('#define MODELINDICES \\');b=source.index('\n#define X(',a)
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ModelIds.h"
'''+source[a:b].rstrip()+'''
#ifdef __cplusplus
extern "C" {
#endif
#define X(name,var) extern int16_t var;
MODELINDICES
#undef X
void InitModelIndices(void);
void MatchModelString(const char *name,int16_t id);
void TestModelIndices(void);
'''+''.join('bool '+name+'(int16_t id);\n' for name in helper_names(source))+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def helpers(source):
    # Move all native-independent decisions into C while retaining the captured model's live field reads
    a=re.search(r'\binline\s+bool\s+IsGlass\(',source).start()
    text=source[a:source.rfind('}')+1].strip()
    text=re.sub(r'inline\s+bool\s*\n','bool\n',text).replace('int16 id','int16_t id')
    text=text.replace('CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(id);','ModelIndexModel *mi = ModelIndices_BackendModel(id);')
    text=text.replace('mi->IsBuilding()','ModelIndices_BackendBuilding(mi)').replace('mi->m_isCodeGlass','ModelIndices_BackendCodeGlass(mi)').replace('mi->m_isArtistGlass','ModelIndices_BackendArtistGlass(mi)')
    text=text.replace('\n{\n','\n{\n    // Preserve the original classifier and its short-circuit read order\n')
    assert len(re.findall(r'(?m)^bool\n\w+\(',text))==15
    return text

def kernel(source,header):
    # Retain all variable initialization, first-match assignment and reset behavior in C
    a=source.index('#define X(name, var) int16 var = -1;');b=source.rindex('//- rouz edit (ChatGPT)')
    text=source[a:b].strip().replace('int16','int16_t')
    text=text.replace('\n{\n','\n{\n    // Preserve the complete original model-registry operation and assignment order\n')
    text=text.replace('if(!General_faststrcmp(name, modelname)){ \\','if(!General_faststrcmp(name, modelname)){ \\')
    return '''//+ rouz edit (ChatGPT)
#include "ModelIndices_backend.h"
#include "General.h"
'''+text+'\n\n'+helpers(header)+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Replace only the complete owning C++ unit or its complete public header
    if '#define X(name, var) int16 var = -1;' in source and 'InitModelIndices(void)' in source:return water.calls(HOST)
    if '#define MODELINDICES \\' in source and 'inline bool' in source and 'IsGlass(int16 id)' in source:return water.calls(HEADER)
    return water.calls(source)

def project(source):
    # Register one uniquely named C11 registry unit and both plain-data/native boundary headers
    entry='<ClCompile Include="..\\src\\modelinfo\\ModelIndices.cpp" />'
    if '..\\src\\modelinfo\\ModelIndexData.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\modelinfo\\ModelIndexData.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\modelinfo\\ModelIndices.h" />'
    for name in ['ModelIndexData.h','ModelIndices_backend.h']:
        if '..\\src\\modelinfo\\'+name+'"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\modelinfo\\'+name+'" />')
    return water.project(source)

def verify_header(snapshot,root):
    # Preserve complete historical header evidence after moving data declarations and every helper body
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize((root/'src/modelinfo/ModelIndices.h').read_text())==normalize(HEADER)
    assert normalize((root/'src/modelinfo/ModelIndexData.h').read_text())==normalize(data_header(snapshot))
    assert normalize(helpers(snapshot)) in normalize((root/'src/modelinfo/ModelIndexData.c').read_text())
