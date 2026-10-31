"""Move the complete surface-property table, parser and eight operations to C11."""
import re
import ranges_migration as ranges
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "SurfaceTypes.h"
#ifdef __cplusplus
struct CColPoint;
extern "C" {
#else
typedef struct CColPoint CColPoint;
#endif
void SurfaceTable_Initialise(const char *filename);
int SurfaceTable_GetAdhesionGroup(uint8_t surfaceType);
float SurfaceTable_GetWetMultiplier(uint8_t surfaceType);
float SurfaceTable_GetAdhesiveLimit(const CColPoint *colpoint);
bool SurfaceTable_IsSoftLanding(uint8_t surfaceType);
bool IsSeeThrough(uint8_t surfaceType);
bool IsSeeThroughVertical(uint8_t surfaceType);
bool IsShootThrough(uint8_t surfaceType);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "SurfaceTable.h"
enum { SURFACE_TABLE_WORK_BUFFER_SIZE=55000 };
#ifdef __cplusplus
extern "C" {
#endif
uint8_t *SurfaceTable_BackendWorkBuffer(void);
float SurfaceTable_BackendWetRoads(void);
uint8_t SurfaceTable_BackendSurfaceA(const CColPoint *point);
uint8_t SurfaceTable_BackendSurfaceB(const CColPoint *point);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Weather.h"
#include "ColPoint.h"
#include "SurfaceTable_backend.h"
static_assert(sizeof(work_buff)==SURFACE_TABLE_WORK_BUFFER_SIZE,"Actual shared work buffer extent");
uint8_t *SurfaceTable_BackendWorkBuffer(void)
{
    // Borrow the original main-module work buffer without a copy or ownership change
    return work_buff;
}
float SurfaceTable_BackendWetRoads(void)
{
    // Read the actual live native wetness value at the original branch
    return CWeather::WetRoads;
}
uint8_t SurfaceTable_BackendSurfaceA(const CColPoint *point)
{
    // Read the original first native collision surface without a layout cast
    return point->surfaceA;
}
uint8_t SurfaceTable_BackendSurfaceB(const CColPoint *point)
{
    // Read the original second native collision surface without a layout cast
    return point->surfaceB;
}
//- rouz edit (ChatGPT)
'''
def kernel(source,header):
    # Translate every complete parser and lookup operation while retaining both symmetric table writes
    body=source[source.index('void\nCSurfaceTable::Initialise'):source.rindex('//- rouz edit (ChatGPT)')].rstrip()
    body=body.replace('CSurfaceTable::','SurfaceTable_').replace('ms_aAdhesiveLimitTable','SurfaceTable_AdhesiveLimits').replace('Const char','const char').replace('uint8','uint8_t').replace('CColPoint &colpoint','const CColPoint *colpoint')
    body=body.replace('work_buff, sizeof(work_buff)','SurfaceTable_BackendWorkBuffer(), SURFACE_TABLE_WORK_BUFFER_SIZE').replace('(char*)work_buff','(char*)SurfaceTable_BackendWorkBuffer()').replace('CWeather::WetRoads','SurfaceTable_BackendWetRoads()')
    body=body.replace('GetAdhesionGroup(colpoint.surfaceB)','SurfaceTable_GetAdhesionGroup(SurfaceTable_BackendSurfaceB(colpoint))').replace('GetAdhesionGroup(colpoint.surfaceA)','SurfaceTable_GetAdhesionGroup(SurfaceTable_BackendSurfaceA(colpoint))')
    a=header.index('inline bool\n');b=header.index('\nclass CSurfaceTable');helpers=header[a:b].replace('inline bool','bool').replace('uint8','uint8_t')
    body+='\n\n'+helpers.strip()
    body=re.sub(r'(?m)^\{\n','{\n    // Preserve the complete original surface parser or lookup decision\n',body)
    body=re.sub(r'(?m)^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original scan, branch and assignment order\n',body)
    return '''//+ rouz edit (ChatGPT)
#include "SurfaceTable_backend.h"
#include "FileMgr.h"
#include <stdio.h>
static float SurfaceTable_AdhesiveLimits[NUMADHESIVEGROUPS][NUMADHESIVEGROUPS];
'''+body+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Replace reference arguments with pointers once and retain every native caller expression
    if 'CSurfaceTable' not in source:return ranges.calls(source)
    if 'float CSurfaceTable::ms_aAdhesiveLimitTable' in source:return ranges.calls(HOST)
    if 'class CSurfaceTable\n' in source:return ranges.calls(HEADER)
    lines=[]
    for line in source.splitlines(keepends=True):
        changed=line.replace('CSurfaceTable::','SurfaceTable_')
        if 'CSurfaceTable::GetAdhesiveLimit(' in line:changed=re.sub(r'SurfaceTable_GetAdhesiveLimit\(([^()\n]+)\)',r'SurfaceTable_GetAdhesiveLimit(&(\1))',changed)
        if changed!=line:changed=changed.rstrip('\n')+' // rouz edit (ChatGPT)\n'
        lines.append(changed)
    return ranges.calls(''.join(lines))

def project(source):
    # Register one unique C11 surface-properties unit and its typed native boundary declarations
    entry='<ClCompile Include="..\\src\\core\\SurfaceTable.cpp" />'
    if '..\\src\\core\\SurfaceProperties.c"' not in source:
        if entry in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\core\\SurfaceProperties.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
        else:
            entry='<ClCompile Include="..\\src\\core\\SurfaceTable.cpp">\n      <Filter>core</Filter>\n    </ClCompile>'
            assert entry in source
            source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\core\\SurfaceProperties.c">\n      <Filter>core</Filter>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\core\\SurfaceTable.h" />'
    if '..\\src\\core\\SurfaceTable_backend.h"' not in source:
        if entry in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\core\\SurfaceTable_backend.h" />')
        else:
            entry='<ClInclude Include="..\\src\\core\\SurfaceTable.h">\n      <Filter>core</Filter>\n    </ClInclude>'
            assert entry in source
            source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\core\\SurfaceTable_backend.h">\n      <Filter>core</Filter>\n    </ClInclude>')
    return ranges.project(source)
