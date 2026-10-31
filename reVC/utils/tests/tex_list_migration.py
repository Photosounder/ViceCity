"""Move the complete texture-list class and its canonical usage flags into C11."""
import re
from audio_controls_migration import function
import player_skin_migration as skins
METHODS=['Initialise','Shutdown','SetTexture','GetFirstFreeTexture','LoadFileNameTexture','LoadGlobalTextureList']
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
#ifdef LIBRW
namespace rw { struct Texture; }
typedef rw::Texture TexListTexture;
#else
typedef struct RwTexture TexListTexture;
#endif
extern "C" {
#else
typedef struct TexListTexture TexListTexture;
#endif
enum { TEXLIST_MAX_TEXUSED=400 };
extern bool TexList_ms_nTexUsed[TEXLIST_MAX_TEXUSED];
void TexList_Initialise(void);
void TexList_Shutdown(void);
TexListTexture *TexList_SetTexture(int32_t slot,char *name);
int32_t TexList_GetFirstFreeTexture(void);
TexListTexture *TexList_LoadFileNameTexture(char *name);
void TexList_LoadGlobalTextureList(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Transform every complete actual operation without filling the original texture-loading stubs
    assert 'bool CTexList::ms_nTexUsed[MAX_TEXUSED];' in source
    chunks=[]
    comments=['Preserve the original empty initialization without resetting usage flags','Preserve the original empty shutdown without changing usage flags','Preserve the original null texture result without reading the name or changing slot flags','Find the first unused flag in the original ascending slot order','Keep the original free-slot lookup before the unchanged texture setter','Enter the same file-manager operation with the original directory string']
    for name,comment in zip(METHODS,comments):
        text=function(source,'CTexList::',name).replace('CTexList::','TexList_')
        for method in METHODS:text=re.sub(r'(?<![\w])'+method+r'\(', 'TexList_'+method+'(',text)
        text=re.sub(r'\bRwTexture\b','TexListTexture',text);text=re.sub(r'\bint32\b','int32_t',text)
        text=text.replace('ms_nTexUsed','TexList_ms_nTexUsed').replace('MAX_TEXUSED','TEXLIST_MAX_TEXUSED').replace('nil','NULL')
        text=text.replace('()\n','(void)\n').replace('\n{}','\n{\n}')
        text=text.replace('\n{','\n{\n    // '+comment,1)
        if name=='SetTexture':text=text.replace('\n\treturn NULL;','\n    (void)slot;(void)name;\n\treturn NULL;')
        chunks.append(text)
    return '''//+ rouz edit (ChatGPT)
#include "TexList.h"
#include "FileMgr.h"
#include <stddef.h>
bool TexList_ms_nTexUsed[TEXLIST_MAX_TEXUSED];

'''+ '\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Replace both complete class files and forward any future ordinary call sites consistently
    if 'CTexList' not in source:return skins.calls(source)
    if 'bool CTexList::ms_nTexUsed[MAX_TEXUSED];' in source:return skins.calls(kernel(source))
    if 'class CTexList\n' in source:return skins.calls(HEADER)
    lines=[]
    for line in source.splitlines(keepends=True):
        changed=line.replace('CTexList::','TexList_')
        if changed!=line:changed=changed.rstrip('\n')+' // rouz edit (ChatGPT)\n'
        lines.append(changed)
    return skins.calls(''.join(lines))

def project(source):
    # Replace the C++ compilation unit with C11 while retaining every existing project entry
    entry='<ClCompile Include="..\\src\\renderer\\TexList.cpp" />'
    source=source.replace(entry,'<ClCompile Include="..\\src\\renderer\\TexList.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    source=source.replace('Include="..\\src\\renderer\\TexList.cpp"','Include="..\\src\\renderer\\TexList.c"')
    return skins.project(source)

def target(path):
    # Keep whole historical file-manager caller checks after the complete source relocation
    return 'src/renderer/TexList.c' if path=='src/renderer/TexList.cpp' else path
