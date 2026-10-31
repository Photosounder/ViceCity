"""Move the full text owner, allocation tables, lifecycle and Unicode helpers into C11."""
import re
import object_names_migration as objects
OBSERVER=None
OPERATIONS=[('CText','Text',name) for name in ['Load','Unload','Get','GetUpperCase','UpperCase','GetNameOfLoadedMissionText','ReadChunkHeader','LoadMissionText']]+[('CKeyArray','TextKeys',name) for name in ['Load','Unload','Update','BinarySearch','Search']]+[('CData','TextData',name) for name in ['Load','Unload']]+[('CMissionTextOffsets','TextOffsets','Load')]
FREE=['UnicodeToAscii','UnicodeToAsciiForSaveLoad','UnicodeToAsciiForMemoryCard','TextCopy']
FIELDS={'CText':['keyArray','data','mission_keyArray','mission_data','encoding','bHasMissionTextOffsets','bIsMissionTextLoaded','szMissionTableName','MissionTextOffsets'],'CKeyArray':['entries','numEntries'],'CData':['chars','numChars'],'CMissionTextOffsets':['data','size']}

def body(source,owner,name):
    # Keep complete preprocessor alternatives and reference/pointer return declarations intact
    if owner=='CKeyArray' and name=='Search':start=source.index('wchar*\n#if defined (FIX_BUGS) || defined(FIX_BUGS_64)\nCKeyArray::Search')
    else:
        pattern=r'(?m)^(?:[\w *]+\n)?(?:[\w *]+ )?'+(re.escape(owner)+'::' if owner else '')+name+r'\([^\n]*\)\n\{'
        match=re.search(pattern,source);assert match,(owner,name);start=match.start()
    return source[start:source.index('\n}',start)+2]

def language_block(source):
    # Preserve the complete existing conditional nested enum while exposing identical shared C values
    start=source.index('\tenum LANGUAGE\n');end=source.index('\n\t};',start)+4
    return source[start:end]

def language_header(source):
    # Share every original conditional language identifier without copying numeric values
    text=language_block(source).replace('\tenum LANGUAGE','enum TextLanguage').replace('LANGUAGE_','TEXT_LANGUAGE_')
    return '//+ rouz edit (ChatGPT)\n#pragma once\n#include "config.h"\n'+text+'\n//- rouz edit (ChatGPT)\n'

def header():
    # Expose the original ownership records and complete explicit C lifecycle
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "config.h"
typedef struct CKeyEntry {
#if defined(FIX_BUGS) || defined(FIX_BUGS_64)
    uint32_t valueOffset;
#else
    uint16_t *value;
#endif
    char key[8];
} CKeyEntry;
typedef struct CKeyArray {CKeyEntry *entries;int numEntries;} CKeyArray;
typedef struct CData {uint16_t *chars;int numChars;} CData;
enum {MAX_MISSION_TEXTS=90};
typedef struct CMissionTextOffsetEntry {char szMissionName[8];uint32_t offset;} CMissionTextOffsetEntry;
typedef struct CMissionTextOffsets {CMissionTextOffsetEntry data[MAX_MISSION_TEXTS];uint16_t size;} CMissionTextOffsets;
typedef struct ChunkHeader {char magic[4];int size;} ChunkHeader;
typedef struct CText {
    CKeyArray keyArray;
    CData data;
    CKeyArray mission_keyArray;
    CData mission_data;
    char encoding;
    bool bHasMissionTextOffsets;
    bool bIsMissionTextLoaded;
    char szMissionTableName[8];
    CMissionTextOffsets MissionTextOffsets;
} CText;
#ifdef __cplusplus
extern "C" {
#endif
extern CText TheText;
extern uint16_t WideErrorString[25];
extern uint16_t UpperCaseTable[128];
extern uint16_t FrenchUpperCaseTable[128];
void Text_Init(CText *self);
void Text_Destroy(CText *self);
void Text_Load(CText *self);
void Text_Unload(CText *self);
uint16_t *Text_Get(CText *self,const char *key);
uint16_t Text_GetUpperCase(CText *self,uint16_t c);
void Text_UpperCase(CText *self,uint16_t *s);
void Text_GetNameOfLoadedMissionText(CText *self,char *output);
void Text_ReadChunkHeader(CText *self,ChunkHeader *buf,int32_t file,size_t *offset);
void Text_LoadMissionText(CText *self,char *name);
void TextKeys_Init(CKeyArray *self);
void TextKeys_Load(CKeyArray *self,size_t length,int file,size_t *offset);
void TextKeys_Unload(CKeyArray *self);
void TextKeys_Update(CKeyArray *self,uint16_t *chars);
CKeyEntry *TextKeys_BinarySearch(CKeyArray *self,const char *key,CKeyEntry *entries,int16_t low,int16_t high);
#if defined(FIX_BUGS) || defined(FIX_BUGS_64)
uint16_t *TextKeys_Search(CKeyArray *self,const char *key,uint16_t *data,uint8_t *result);
#else
uint16_t *TextKeys_Search(CKeyArray *self,const char *key,uint8_t *result);
#endif
void TextData_Init(CData *self);
void TextData_Load(CData *self,size_t length,int file,size_t *offset);
void TextData_Unload(CData *self);
void TextOffsets_Init(CMissionTextOffsets *self);
void TextOffsets_Load(CMissionTextOffsets *self,size_t length,int file,size_t *offset,int unused);
char *UnicodeToAscii(uint16_t *source);
char *UnicodeToAsciiForSaveLoad(uint16_t *source);
char *UnicodeToAsciiForMemoryCard(uint16_t *source);
void TextCopy(uint16_t *output,const uint16_t *source);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "TextLanguages.h"
#ifdef __cplusplus
extern "C" {
#endif
int32_t Text_BackendLanguage(void);
bool Text_BackendRussianGame(void);
bool Text_BackendJapaneseGame(void);
void *Text_BackendAllocate(size_t size);
void Text_BackendFree(void *pointer);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Frontend.h"
#include "Game.h"
#include "Text_backend.h"

int32_t Text_BackendLanguage(void)
{
    // Read the original live language preference at each file-selection switch
    return FrontEndMenuManager.m_PrefsLanguage;
}
bool Text_BackendRussianGame(void)
{
    // Query the original native locale flag only at its original short-circuit position
#ifdef MORE_LANGUAGES
    return CGame::russianGame;
#else
    return false;
#endif
}
bool Text_BackendJapaneseGame(void)
{
    // Query the original native locale flag only after the original Russian-language test
#ifdef MORE_LANGUAGES
    return CGame::japaneseGame;
#else
    return false;
#endif
}
void *Text_BackendAllocate(size_t size)
{
    // Preserve the native allocation provider selected by the existing common header
    return malloc(size);
}
void Text_BackendFree(void *pointer)
{
    // Release text ownership through the same native provider that allocated its buffers
    free(pointer);
}
//- rouz edit (ChatGPT)
'''
LIFECYCLE='''void TextKeys_Init(CKeyArray *self)
{
    // Preserve the original key-array constructor's named-field initialization
    self->entries=NULL;
    self->numEntries=0;
}
void TextData_Init(CData *self)
{
    // Preserve the original character-array constructor's named-field initialization
    self->chars=NULL;
    self->numChars=0;
}
void TextOffsets_Init(CMissionTextOffsets *self)
{
    // Preserve the original offset-table constructor without clearing inactive entries
    self->size=0;
}
void Text_Init(CText *self)
{
    // Construct each original owning subrecord before the original text-constructor body
    TextKeys_Init(&self->keyArray);
    TextData_Init(&self->data);
    TextKeys_Init(&self->mission_keyArray);
    TextData_Init(&self->mission_data);
    TextOffsets_Init(&self->MissionTextOffsets);
    self->encoding='e';
    self->bHasMissionTextOffsets=false;
    self->bIsMissionTextLoaded=false;
    memset(self->szMissionTableName,0,sizeof(self->szMissionTableName));
    memset(WideErrorString,0,sizeof(WideErrorString));
}
void Text_Destroy(CText *self)
{
    // Destroy the four original owning subrecords in reverse member-declaration order
    TextData_Unload(&self->mission_data);
    TextKeys_Unload(&self->mission_keyArray);
    TextData_Unload(&self->data);
    TextKeys_Unload(&self->keyArray);
}
static void Text_ShutdownGlobal(void)
{
    // Retain original process-exit ownership cleanup without a C++ global destructor
    Text_Destroy(&TheText);
}
#if !defined(_MSC_VER) && (defined(__GNUC__) || defined(__clang__))
static void Text_InitialiseGlobal(void) __attribute__((constructor));
#endif
static void Text_InitialiseGlobal(void)
{
    // Retain original pre-main construction and register its matching C exit cleanup
    Text_Init(&TheText);
    atexit(Text_ShutdownGlobal);
}
#if defined(_MSC_VER)
#pragma section(".CRT$XCU",read)
__declspec(allocate(".CRT$XCU")) void (__cdecl *const Text_globalInitializer)(void)=Text_InitialiseGlobal;
#if defined(_M_IX86)
#pragma comment(linker,"/include:_Text_globalInitializer")
#else
#pragma comment(linker,"/include:Text_globalInitializer")
#endif
#elif !defined(__GNUC__) && !defined(__clang__)
#error "Register Text_InitialiseGlobal with the platform C runtime startup"
#endif
'''

def primitives(text):
    # Keep exact original scalar widths, pointer offsets and UTF-16 storage
    for old,new in [('wchar','uint16_t'),('uint8','uint8_t'),('uint16','uint16_t'),('uint32','uint32_t'),('int16','int16_t'),('int32','int32_t'),('uintptr','uintptr_t'),('nil','NULL')]:text=re.sub(r'\b'+old+r'\b',new,text)
    return text

def kernel(source,oldheader):
    # Preserve all complete original loader/search/conversion bodies and named constructor effects
    assert 'CKeyArray(void) : entries(nil), numEntries(0) {}' in oldheader and 'CData(void) : chars(nil), numChars(0) {}' in oldheader
    assert 'CMissionTextOffsets(void) : size(0) {}' in oldheader
    assert '~CKeyArray(void) { Unload(); }' in oldheader and '~CData(void) { Unload(); }' in oldheader
    originalCtor=body(source,'CText','CText');assert re.sub(r'\s+','',originalCtor[originalCtor.index('{'):])=='{encoding=\'e\';bHasMissionTextOffsets=false;bIsMissionTextLoaded=false;memset(szMissionTableName,0,sizeof(szMissionTableName));memset(WideErrorString,0,sizeof(WideErrorString));}'
    chunks=[]
    for owner,prefix,name in OPERATIONS:
        text=body(source,owner,name)
        text=text.replace(owner+'::'+name+'(',prefix+'_'+name+'('+owner+' *self'+('' if re.search(owner+r'::'+name+r'\((?:void)?\)',text) else ','),1)
        if owner=='CKeyArray' and name=='Search':text=text.replace('CKeyArray::Search(',prefix+'_'+name+'('+owner+' *self,')
        text=text.replace('*selfvoid)', '*self)')
        text=text.replace('this->','')
        for field in FIELDS[owner]:
            if owner=='CKeyArray' and name=='BinarySearch' and field=='entries':continue
            text=re.sub(r'(?<![\w.>])'+field+r'\b','self->'+field,text)
        if owner=='CText':
            for member,group in [('keyArray','TextKeys'),('mission_keyArray','TextKeys'),('data','TextData'),('mission_data','TextData'),('MissionTextOffsets','TextOffsets')]:
                text=re.sub(r'(self->'+member+r')\.(\w+)\(',lambda match:group+'_'+match[2]+'(&'+match[1]+',',text)
            text=text.replace(',)',')')
            for method in ['Unload','ReadChunkHeader','GetUpperCase']:text=re.sub(r'(?<![\w])'+method+r'\(', 'Text_'+method+'(self,',text)
            text=text.replace('Text_Unload(self,)','Text_Unload(self)')
        if owner=='CKeyArray':text=re.sub(r'(?<![\w])BinarySearch\(', 'TextKeys_BinarySearch(self,',text)
        text=text.replace('CMissionTextOffsets::Entry','CMissionTextOffsetEntry')
        if owner=='CMissionTextOffsets':text=text.replace('size_t *offset, int)','size_t *offset, int unused)')
        text=text.replace('FrontEndMenuManager.m_PrefsLanguage','Text_BackendLanguage()').replace('CMenuManager::LANGUAGE_','TEXT_LANGUAGE_').replace('CGame::russianGame','Text_BackendRussianGame()').replace('CGame::japaneseGame','Text_BackendJapaneseGame()')
        text=primitives(text).replace(' // rouz edit (ChatGPT)','')
        text=re.sub(r'\bmalloc\(', 'Text_BackendAllocate(',text)
        text=re.sub(r'\bfree\(', 'Text_BackendFree(',text)
        text=text.replace('\n{','\n{\n    // Preserve the complete original '+name+' operation and ordered live accesses',1)
        if owner=='CMissionTextOffsets':text=text.replace('// Preserve the complete original '+name+' operation and ordered live accesses','// Preserve the complete original '+name+' operation and ordered live accesses\n    (void)unused;',1)
        chunks.append(text)
    for name in FREE:
        text=body(source,'',name).replace('CGame::russianGame','Text_BackendRussianGame()').replace('CGame::japaneseGame','Text_BackendJapaneseGame()')
        text=primitives(text).replace('\n{','\n{\n    // Preserve the complete original '+name+' conversion and static-buffer contract',1);chunks.append(text)
    tables='\n\n'.join(primitives(re.search(r'wchar '+name+r'\[128\] = \{[\s\S]*?\n\};',source)[0]) for name in ['UpperCaseTable','FrenchUpperCaseTable'])
    return '//+ rouz edit (ChatGPT)\n#define _CRT_SECURE_NO_WARNINGS\n#include "Text.h"\n#include "Text_backend.h"\n#include "Messages.h"\n#include "FileMgr.h"\n#include "Timer.h"\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\nCText TheText;\nuint16_t WideErrorString[25];\n'+tables+'\n\n'+LIFECYCLE+'\n\n'+'\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def text_calls(source):
    # Remove all four complete classes and update every exact canonical text-object caller
    if 'class CKeyArray\n' in source:return header()
    if 'CText::CText(void)' in source:
        if OBSERVER:OBSERVER(source)
        return HOST
    old=source
    if '\tenum LANGUAGE\n' in source and ('class CMenuManager' in source):
        block=language_block(source);alias=re.sub(r'(?m)^(\s*)(LANGUAGE_\w+)(,)',lambda match:match[1]+match[2]+' = TEXT_'+match[2]+match[3],block)
        source=source.replace(block,alias).replace('#pragma once\n','#pragma once\n#include "TextLanguages.h"\n',1)
    for owner,prefix,name in OPERATIONS:
        if owner=='CText':source=re.sub(r'\bTheText\.'+name+r'\(', 'Text_'+name+'(&TheText'+('' if name in ['Load','Unload'] else ','),source)
    if source==old:return source
    if old.lstrip().startswith('//+ rouz edit (ChatGPT)') and old.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def text_project(source):
    # Keep a distinct C object and register its shared enum without a C++ constructor trampoline
    for tag,directory,old,new in [('ClCompile','text','Text.cpp','TextSystem.c'),('ClInclude','text','Text.h','Text_backend.h'),('ClInclude','core','Frontend.h','TextLanguages.h')]:
        entry='<'+tag+' Include="..\\src\\'+directory+'\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\'+directory+'\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\'+directory+'\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\'+directory+r'\\'+re.escape(old)+r'">\n\s*<Filter>'+directory+r'</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove whole historical producer relocation including all ownership and destructor replacements
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/text/Text.cpp').read_text())
    oldheader=(root/'build/text-c-tests/before/src/text/Text.h').read_text()
    assert normalize(kernel(captured[0],oldheader))==normalize((root/'src/text/TextSystem.c').read_text())

def verify_dependency(saved,current):
    # Permit only the exact whole text-object and shared-language conversion through existing byte guards
    if saved.read_bytes()==current.read_bytes():return
    old=saved.read_text();expected=calls(old);assert expected!=old and expected==current.read_text(),str(current)


def calls(source):
    # Retain the complete text conversion before forwarding through all C time-cycle storage and operations
    import timecycle_migration as cycle
    return cycle.calls(text_calls(source))

def project(source):
    # Retain prior project registrations before adding the unique C lighting object
    import timecycle_migration as cycle
    return cycle.project(text_project(source))
