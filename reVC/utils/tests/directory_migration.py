"""Remove directory constructors, methods, overloads and reference parameters."""
import re
import draw_migration as draw
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct DirectoryInfo {
	uint32_t offset;
	uint32_t size;
	char name[24];
} DirectoryInfo;
typedef struct CDirectory {
	DirectoryInfo *entries;
	int32_t maxEntries;
	int32_t numEntries;
} CDirectory;
#ifdef __cplusplus
extern "C" {
#endif
void Directory_Initialize(CDirectory *directory, int32_t capacity);
void Directory_Shutdown(CDirectory *directory);
void Directory_ReadDirFile(CDirectory *directory, const char *filename);
bool Directory_WriteDirFile(CDirectory *directory, const char *filename);
void Directory_AddItem(CDirectory *directory, const DirectoryInfo *dirinfo);
void Directory_AddItemWithImage(CDirectory *directory, const DirectoryInfo *dirinfo, int32_t imgId);
bool Directory_FindItem(CDirectory *directory, const char *name, uint32_t *offset, uint32_t *size);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
def calls(source):
    # Replace only actual directory ownership, record scopes and method call forms
    if 'class CDirectory\n' in source:return HEADER
    if 'CDirectory::CDirectory(int32 maxEntries)' in source:return kernel(source)
    source=source.replace('class CDirectory;','struct CDirectory;').replace('CDirectory::DirectoryInfo','DirectoryInfo')
    source=re.sub(r'std::allocator<CDirectory>\(\)\.construct\((\w+), (\w+)\)',r'Directory_Initialize(\1, \2)',source)
    source=re.sub(r'(ms_pCutsceneDir|ms_pExtraObjectsDir|pDir)->~CDirectory\(\)',r'Directory_Shutdown(\1)',source)
    source=re.sub(r'(ms_pCutsceneDir|ms_pExtraObjectsDir|pDir)->(ReadDirFile|WriteDirFile)\(([^\n]*)\)',r'Directory_\2(\1, \3)',source)
    source=re.sub(r'(ms_pExtraObjectsDir|pDir)->AddItem\((\w+), (\w+)\)',r'Directory_AddItemWithImage(\1, &\2, \3)',source)
    source=re.sub(r'(ms_pExtraObjectsDir|pDir)->AddItem\((\w+)\)',r'Directory_AddItem(\1, &\2)',source)
    source=re.sub(r'(ms_pCutsceneDir|ms_pExtraObjectsDir)->FindItem\((\w+), (\w+), (\w+)\)',r'Directory_FindItem(\1, \2, &\3, &\4)',source)
    source=source.replace('Construct the directory in malloc storage without invoking C++ new.','Initialize the directory record in its allocated storage').replace('Destroy and release the directory without invoking C++ delete.','Release the directory entries and then its owner storage')
    return draw.calls(source)
def kernel(source):
    # Preserve all seven complete original operations and the real member initialization order
    source=source[source.index('CDirectory::CDirectory(int32 maxEntries)'):]
    source=source.replace('CDirectory::CDirectory(int32 maxEntries)\n : numEntries(0), maxEntries(maxEntries)\n{','void\nDirectory_Initialize(CDirectory *directory, int32_t capacity)\n{\n\tdirectory->maxEntries = capacity;\n\tdirectory->numEntries = 0;')
    source=source.replace('CDirectory::~CDirectory(void)','void\nDirectory_Shutdown(CDirectory *directory)')
    source=source.replace('CDirectory::ReadDirFile(const char *filename)','Directory_ReadDirFile(CDirectory *directory, const char *filename)').replace('CDirectory::WriteDirFile(const char *filename)','Directory_WriteDirFile(CDirectory *directory, const char *filename)')
    source=source.replace('CDirectory::AddItem(const DirectoryInfo &dirinfo, int32 imgId)','Directory_AddItemWithImage(CDirectory *directory, const DirectoryInfo *dirinfo, int32_t imgId)').replace('CDirectory::AddItem(const DirectoryInfo &dirinfo)','Directory_AddItem(CDirectory *directory, const DirectoryInfo *dirinfo)')
    source=source.replace('CDirectory::FindItem(const char *name, uint32 &offset, uint32 &size)','Directory_FindItem(CDirectory *directory, const char *name, uint32_t *offset, uint32_t *size)')
    source=re.sub(r'(?<![>\w])\b(entries|maxEntries|numEntries)\b',r'directory->\1',source)
    source=source.replace('uint32 offset, size;','uint32_t offset, size;').replace('FindItem(dirinfo.name, offset, size)','Directory_FindItem(directory, dirinfo->name, &offset, &size)')
    source=source.replace('AddItem(dirinfo);','Directory_AddItem(directory, &dirinfo);').replace('AddItem(di);','Directory_AddItem(directory, &di);')
    source=source.replace('= dirinfo;','= *dirinfo;').replace('offset = directory->','*offset = directory->').replace('size = directory->','*size = directory->')
    source=source.replace('assert(','DIRECTORY_ASSERT(').replace('//- rouz edit (ChatGPT)','').rstrip()
    comments={'Initialize':'Initialize capacity before count and allocate the original entry array','Shutdown':'Release only the original entry allocation','ReadDirFile':'Read the original fixed-size records and add every positive-length read','WriteDirFile':'Write the original contiguous records and compare the returned byte count','AddItem':'Retain the capacity assertion, duplicate check and record-copy order','AddItemWithImage':'Copy the input record before applying the original image bits','FindItem':'Find the first case-insensitive match and write offset before size'}
    for name,comment in comments.items():
        a=source.index('Directory_'+name+'(CDirectory');brace=source.index('{',a)+1;source=source[:brace]+'\n\t// '+comment+source[brace:]
    assert 'CDirectory::' not in source and '&offset' not in source[source.index('Directory_FindItem(CDirectory'):]
    prefix='''//+ rouz edit (ChatGPT)
#include "Directory.h"
#include "General.h"
#include "FileMgr.h"
#include <stdlib.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#include <cita_windows.h>
void re3_assert(const char *expr, const char *filename, unsigned int lineno, const char *func);
#ifndef MASTER
#define DIRECTORY_ASSERT(expr) (void)((!!(expr)) || (re3_assert(#expr, __FILE__, __LINE__, __func__), 0))
#else
#define DIRECTORY_ASSERT(expr) (expr)
#endif

'''
    return prefix+source+'\n//- rouz edit (ChatGPT)\n'
def project(source):
    # Rename the complete implementation to a C11 compilation unit
    entry='<ClCompile Include="..\\src\\core\\Directory.cpp" />'
    if entry in source:source=source.replace(entry,'<ClCompile Include="..\\src\\core\\Directory.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    else:source=source.replace('Directory.cpp"','Directory.c"')
    return draw.project(source)
