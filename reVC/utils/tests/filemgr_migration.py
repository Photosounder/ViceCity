"""Remove the file-manager class and move its complete file/directory logic to C11."""
import re
import inifile_migration as inifile

HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T FileMgrSize;
#elif defined(__MWERKS__)
typedef ptrdiff_t FileMgrSize;
#else
#include <sys/types.h>
typedef ssize_t FileMgrSize;
#endif
#ifdef __cplusplus
extern "C" {
#endif
void FileMgr_Initialise(void);
void FileMgr_ChangeDir(const char *dir);
void FileMgr_SetDir(const char *dir);
void FileMgr_SetDirMyDocuments(void);
FileMgrSize FileMgr_LoadFile(const char *file, uint8_t *buf, int maxlen, const char *mode);
int FileMgr_OpenFile(const char *file, const char *mode);
int FileMgr_OpenFileDefault(const char *file);
int FileMgr_OpenFileForWriting(const char *file);
size_t FileMgr_Read(int fd, const char *buf, FileMgrSize len);
size_t FileMgr_Write(int fd, const char *buf, FileMgrSize len);
bool FileMgr_Seek(int fd, int offset, int whence);
bool FileMgr_ReadLine(int fd, char *buf, int len);
int FileMgr_CloseFile(int fd);
int FileMgr_GetErrorReadWrite(int fd);
char *FileMgr_GetRootDirName(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdio.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
const char *FileMgr_BackendUserFilesFolder(void);
#ifndef _WIN32
char *FileMgr_BackendCasePath(const char *path, bool checkPathFirst);
FILE *FileMgr_BackendCaseOpen(const char *filename, const char *mode);
#endif
void re3_assert(const char *expr, const char *filename, unsigned int lineno, const char *func);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "crossplatform.h"
#include "FileMgr_backend.h"

const char *_psGetUserFilesFolder();

const char *
FileMgr_BackendUserFilesFolder(void)
{
	// Query the original platform folder provider at the original call point
	return _psGetUserFilesFolder();
}

#ifndef _WIN32
char *
FileMgr_BackendCasePath(const char *path, bool checkPathFirst)
{
	// Retain the existing case-correcting path helper and its allocated result
	return casepath(path, checkPathFirst);
}

FILE *
FileMgr_BackendCaseOpen(const char *filename, const char *mode)
{
	// Retain the original case-insensitive platform file-open operation
	return fcaseopen(filename, mode);
}
#endif
//- rouz edit (ChatGPT)
'''

def masked(source):
    # Keep exact offsets while excluding comments and quoted contents from call parsing
    return re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\\n])*\'',lambda m:''.join('\n' if c=='\n' else ' ' for c in m[0]),source)

def calls(source):
    # Select the original one-argument binary-open overload by actual argument count
    if 'class CFileMgr\n' in source:return HEADER
    # Finish the missed date-array comparison exposed by compiling the complete frontend caller
    source=source.replace('SlotDate[saveSlot] < SlotDate[i]','Date_IsBefore(&SlotDate[saveSlot], &SlotDate[i])')
    code=masked(source);changes=[]
    for match in re.finditer(r'\bCFileMgr::(\w+)\b',code):
        name=match[1]
        if name=='OpenFile':
            a=code.index('(',match.end());i=a+1;depth=1;commas=0
            while depth:
                c=code[i]
                if c in '([{':depth+=1
                if c in ')]}':depth-=1
                if c==',' and depth==1:commas+=1
                i+=1
            assert commas in [0,1],source[match.start():i]
            if commas==0:name='OpenFileDefault'
        changes.append((match.start(),match.end(),'FileMgr_'+name))
    for a,b,value in reversed(changes):source=source[:a]+value+source[b:]
    return inifile.calls(source)

def kernel(source):
    # Preserve every file-table, text translation and directory-management statement
    source=source[source.index('/*\n * Windows FILE'):]
    source=source.replace('struct myFILE\n','typedef struct myFILE\n').replace('\n};\n','\n} myFILE;\n',1)
    source=source.replace('casepath(path, false)','FileMgr_BackendCasePath(path, false)')
    source=source.replace('fcaseopen(filename, realmode)','FileMgr_CaseOpen(filename, realmode)')
    source=source.replace('CFileMgr::','FileMgr_').replace('ms_rootDirName','FileMgr_rootDirName').replace('ms_dirName','FileMgr_dirName')
    source=source.replace('char FileMgr_FileMgr_rootDirName','static char FileMgr_rootDirName').replace('char FileMgr_FileMgr_dirName','static char FileMgr_dirName')
    source=source.replace('ssize_t','FileMgrSize').replace('uint8 *buf','uint8_t *buf').replace('nil','NULL')
    source=source.replace('\tSetDir("");','\tFileMgr_SetDir("");').replace('_psGetUserFilesFolder()','FileMgr_BackendUserFilesFolder()').replace('return OpenFile(file, "wb");','return FileMgr_OpenFile(file, "wb");')
    source=source.replace('assert(', 'FileMgr_Assert(')
    source=re.sub(r'(\n(?:void|static int|static char\*|static size_t|FileMgrSize|int|size_t|bool)\n?\w*[^\n]*\n\{\n)',r'\1\t// Preserve the original operation order, state updates and return convention\n',source)
    source=source.replace('void mychdir(char const *path)\n{','static void mychdir(char const *path)\n{\n\t// Resolve the original non-Windows path before changing the current directory')
    comments={
        'myfopen':'Reserve the first free slot and force binary CRT mode while remembering text mode',
        'myfclose':'Close an occupied slot and release it for reuse',
        'myfgetc':'Translate CRLF to LF while retaining lone CR and embedded SUB bytes',
        'myfputc':'Insert CR before LF only for text-mode output',
        'myfgets':'Read through the text adapter until newline, capacity or EOF',
        'myfread':'Read translated text bytes individually or delegate binary reads to the CRT',
        'myfwrite':'Write translated text bytes individually or delegate binary writes to the CRT',
        'myfseek':'Return the original raw CRT seek status',
        'myfeof':'Report EOF using the original feof operation',
        'FileMgr_Initialise':'Capture the current root directory and append the original backslash',
        'FileMgr_ChangeDir':'Extend or reset the persistent directory buffer before changing directories',
        'FileMgr_SetDir':'Build the requested directory from the live root buffer',
        'FileMgr_SetDirMyDocuments':'Restore the root before querying and entering the platform user directory',
        'FileMgr_LoadFile':'Read the original fixed-size chunks and append the terminating NUL',
        'FileMgr_OpenFile':'Open the requested mode through the original slot allocator',
        'FileMgr_OpenFileForWriting':'Retain the original binary-write convenience operation',
        'FileMgr_Read':'Read into the original caller buffer despite its const-qualified API',
        'FileMgr_Write':'Write the original caller bytes through the text or binary adapter',
        'FileMgr_Seek':'Retain the original true-on-failure seek convention',
        'FileMgr_ReadLine':'Report whether the original bounded line reader returned a buffer',
        'FileMgr_CloseFile':'Release the original file slot and return its close status',
        'FileMgr_GetErrorReadWrite':'Retain the original EOF query despite the method name'}
    for name,comment in comments.items():
        source=re.sub(r'(\b'+name+r'\([^{}\n]*\)\n\{\n)\t// Preserve the original operation order, state updates and return convention\n',lambda match:match[1]+'\t// '+comment+'\n',source)
    source=source.replace('\t// Resolve the original non-Windows path before changing the current directory\n\t// Preserve the original operation order, state updates and return convention','\t// Resolve the original non-Windows path before changing the current directory')
    extra='''
int
FileMgr_OpenFileDefault(const char *file)
{
	// Preserve the original one-argument overload's binary-read mode
	return FileMgr_OpenFile(file, "rb");
}

char *
FileMgr_GetRootDirName(void)
{
	// Return the original live root buffer without copying it
	return FileMgr_rootDirName;
}
'''
    prefix='''//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "FileMgr.h"
#include "FileMgr_backend.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef _WIN32
#include <direct.h>
#define FileMgr_CaseOpen fopen
#else
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#include <cita_windows.h>
#define FileMgr_CaseOpen FileMgr_BackendCaseOpen
#endif
#ifndef MASTER
#define FileMgr_Assert(expr) (void)((!!(expr)) || (re3_assert(#expr, __FILE__, __LINE__, __func__), 0))
#else
#define FileMgr_Assert(expr) (expr)
#endif

'''
    assert 'CFileMgr' not in source and 'ssize_t' not in source
    return prefix+source.rstrip()+'\n'+extra+'//- rouz edit (ChatGPT)\n'

def project(source):
    # Compile the complete file-manager algorithm as C11 beside its platform bridge
    source=source.replace('<ClCompile Include="..\\src\\core\\FileMgr.cpp" />','<ClCompile Include="..\\src\\core\\FileMgr.cpp" />\n    <ClCompile Include="..\\src\\core\\FileMgr.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClCompile Include="..\\src\\core\\FileMgr.cpp">\n      <Filter>core</Filter>\n    </ClCompile>'
    source=source.replace(entry,entry+'\n    '+entry.replace('FileMgr.cpp','FileMgr.c'))
    source=source.replace('<ClInclude Include="..\\src\\core\\FileMgr.h" />','<ClInclude Include="..\\src\\core\\FileMgr.h" />\n    <ClInclude Include="..\\src\\core\\FileMgr_backend.h" />')
    entry='<ClInclude Include="..\\src\\core\\FileMgr.h">\n      <Filter>core</Filter>\n    </ClInclude>'
    return inifile.project(source.replace(entry,entry+'\n    '+entry.replace('FileMgr.h','FileMgr_backend.h')))
