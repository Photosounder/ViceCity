"""Move INI parsing, clamping and density-limit calculations to C11."""
import re
import directory_migration as directory
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#define DEFAULT_MAX_NUMBER_OF_PEDS 25.0f
#define DEFAULT_MAX_NUMBER_OF_PEDS_INTERIOR 40.0f
#define DEFAULT_MAX_NUMBER_OF_CARS 12.0f
#ifdef __cplusplus
extern "C" {
#endif
void IniFile_LoadIniFile(void);
extern float IniFile_PedNumberMultiplier;
extern float IniFile_CarNumberMultiplier;
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
char *IniFile_BackendString(void);
int32_t *IniFile_BackendPedLimit(void);
int32_t *IniFile_BackendInteriorLimit(void);
int32_t *IniFile_BackendCarLimit(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "CarCtrl.h"
#include "main.h"
#include "Population.h"
#include "IniFile_backend.h"

char *
IniFile_BackendString(void)
{
	// Return the original shared parse buffer without copying or clearing it
	return gString;
}

int32_t *
IniFile_BackendPedLimit(void)
{
	// Return the original live pedestrian limit
	return &CPopulation::MaxNumberOfPedsInUse;
}

int32_t *
IniFile_BackendInteriorLimit(void)
{
	// Return the original live interior pedestrian limit
	return &CPopulation::MaxNumberOfPedsInUseInterior;
}

int32_t *
IniFile_BackendCarLimit(void)
{
	// Return the original live traffic limit
	return &CCarCtrl::MaxNumberOfCarsInUse;
}
//- rouz edit (ChatGPT)
'''
def calls(source):
    # Preserve direct reads, writes and slider addresses while removing the class qualifier
    if 'class CIniFile\n' in source:return HEADER
    if 'void CIniFile::LoadIniFile()' in source:return HOST
    return directory.calls(re.sub(r'\bCIniFile::(\w+)\b',r'IniFile_\1',source))
def kernel(source):
    # Retain the complete original parser, ternary clamps and sequential integer limit writes
    source=source[source.index('float CIniFile::PedNumberMultiplier'):]
    source=re.sub(r'\bCIniFile::(\w+)\b',r'IniFile_\1',source).replace('IniFile_LoadIniFile()','IniFile_LoadIniFile(void)')
    source=re.sub(r'(?<!IniFile_)\b(PedNumberMultiplier|CarNumberMultiplier)\b',r'IniFile_\1',source)
    source=source.replace('if (f){','if (f){\n\t\t// Read and parse into the original shared buffer without resetting stale contents\n\t\tchar *string = IniFile_BackendString();')
    source=source.replace('gString','string').replace('Min(','INIFILE_MIN(').replace('Max(','INIFILE_MAX(')
    source=source.replace('CPopulation::MaxNumberOfPedsInUseInterior','*IniFile_BackendInteriorLimit()').replace('CPopulation::MaxNumberOfPedsInUse','*IniFile_BackendPedLimit()').replace('CCarCtrl::MaxNumberOfCarsInUse','*IniFile_BackendCarLimit()')
    source=source.replace('{\n\tFileMgr_SetDir','{\n\t// Restore the original file-manager directory before opening the INI file\n\tFileMgr_SetDir').replace('\n\t*IniFile_BackendPedLimit()', '\n\t// Apply the original float-to-integer limits in their original write order\n\t*IniFile_BackendPedLimit()')
    source=source.replace('//- rouz edit (ChatGPT)','').rstrip()
    return '''//+ rouz edit (ChatGPT)
#include "IniFile.h"
#include "IniFile_backend.h"
#include "FileMgr.h"
#include <stdio.h>
#define INIFILE_MIN(a,b) ((a) < (b) ? (a) : (b))
#define INIFILE_MAX(a,b) ((a) > (b) ? (a) : (b))

'''+source+'\n//- rouz edit (ChatGPT)\n'
def project(source):
    # Register the C parser beside the four typed C++ address readers
    entry='<ClCompile Include="..\\src\\core\\IniFile.cpp" />'
    source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\core\\IniFile.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClCompile Include="..\\src\\core\\IniFile.cpp">\n      <Filter>core</Filter>\n    </ClCompile>'
    source=source.replace(entry,entry+'\n    '+entry.replace('IniFile.cpp','IniFile.c'))
    entry='<ClInclude Include="..\\src\\core\\IniFile.h" />'
    source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\core\\IniFile_backend.h" />')
    entry='<ClInclude Include="..\\src\\core\\IniFile.h">\n      <Filter>core</Filter>\n    </ClInclude>'
    return directory.project(source.replace(entry,entry+'\n    '+entry.replace('IniFile.h','IniFile_backend.h')))
