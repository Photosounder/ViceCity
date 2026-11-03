"""Move zone records, shared enum definitions and the complete zone owner into C11."""
import re
import general_migration as general

FIELDS={'m_CurrLevel':'Zones_currentLevel','FindIndex':'Zones_findIndex',
 'NumberOfAudioZones':'Zones_audioCount','AudioZoneArray':'Zones_audioIndices',
 'TotalNumberOfMapZones':'Zones_mapCount','TotalNumberOfInfoZones':'Zones_infoCount',
 'TotalNumberOfNavigationZones':'Zones_navigationCount','InfoZoneArray':'Zones_info',
 'MapZoneArray':'Zones_map','NavigationZoneArray':'Zones_navigation',
 'TotalNumberOfZoneInfos':'Zones_zoneInfoCount','ZoneInfoArray':'Zones_zoneInfo'}
CAR_NAMES=['NORMAL','POOR','RICH','EXEC','WORKER','BIG','TAXI','MOPED','MOTORBIKE','LEISUREBOAT','WORKERBOAT','COPS','CUBAN','HAITIAN','STREET','DIAZ','BIKER','SECURITY','PLAYER','GOLFERS','GANG9','COPS_BOAT','FIRST_CAR_RATING','FIRST_BOAT_RATING','FIRST_GANG_CAR_RATING','NUM_CAR_CLASSES','NUM_BOAT_CLASSES','NUM_GANG_CAR_CLASSES','TOTAL_CUSTOM_CLASSES']
POSITIONS=['PointLiesWithinZone','GetLevelFromPosition','FindInformationZoneForPosition','FindSmallestNavigationZoneForPosition','GetZoneInfo','GetZoneInfoForTimeOfDay','FindAudioZone']
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Zones.h"
#include "Text.h"
#include "World.h"

extern "C" ZonesPosition
ZonesHost_Player(void)
{
    // Copy the original player coordinates at the zone update's query point
    return ZonesPosition_FromVector(FindPlayerCoors());
}

//- rouz edit (ChatGPT)
'''

def enum_body(source,start):
    # Extract the complete enum and its original relative-value expressions
    a=source.index(start);b=source.index('};',a)+2
    return source[a:b]

def car_enum(source):
    # Expose every former nested car-class value under C names without compatibility aliases
    source=enum_body(source,'enum eCarClass {').replace('enum eCarClass {','typedef enum CarClass {').replace('};','} CarClass;')
    for name in CAR_NAMES:source=re.sub(r'\b'+name+r'\b','CARCLASS_'+name,source)
    return '//+ rouz edit (ChatGPT)\n#pragma once\n'+source+'\n//- rouz edit (ChatGPT)\n'

def level_enum(source):
    # Preserve all existing global level values in a C enum typedef
    return '//+ rouz edit (ChatGPT)\n#pragma once\n'+enum_body(source,'enum eLevelName {').replace('enum eLevelName {','typedef enum eLevelName {').replace('};','} eLevelName;')+'\n//- rouz edit (ChatGPT)\n'

def gang_enum(source):
    # Share the original gang order and count between C records and existing C++ users
    a=source.index('enum {\n\tGANG_CUBAN');body=enum_body(source[a:],'enum {')
    return '//+ rouz edit (ChatGPT)\n#pragma once\n'+body+'\n//- rouz edit (ChatGPT)\n'

def scalars(source):
    # Use exact-width C integers and preserve the original pointer-difference return type
    for a,b in [('uint8','uint8_t'),('uint16','uint16_t'),('uint32','uint32_t'),('uint64','uint64_t'),('int16','int16_t'),('int32','int32_t'),('ssize_t','ptrdiff_t'),('wchar','uint16_t'),('nil','NULL')]:source=re.sub(r'\b'+a+r'\b',b,source)
    return source

def car_calls(source):
    # Replace scoped enum uses while leaving car-control operation names unchanged
    source=source.replace('CCarCtrl::eCarClass','CarClass')
    for name in CAR_NAMES:source=re.sub(r'CCarCtrl::'+name+r'\b','CARCLASS_'+name,source)
    if 'class CCarCtrl' in source or 'CCarCtrl::Init(' in source:
        # Rewrite implicit class-scope uses without modifying diagnostic strings or other qualified names
        pieces=re.split(r'("(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])\')',source)
        for i in range(0,len(pieces),2):
            for name in CAR_NAMES:pieces[i]=re.sub(r'(?<![\w:.>])'+name+r'\b','CARCLASS_'+name,pieces[i])
        source=''.join(pieces)
    return source

def position_calls(source,name):
    # Copy the pointed-to vector into the C value argument without evaluating its expression twice
    token='CTheZones::'+name+'('
    while token in source:
        a=source.index(token);start=a+len(token);b=start;depth=0
        while b<len(source):
            if source[b] in ',)' and depth==0:break
            depth+=(source[b]=='(')-(source[b]==')');b+=1
        argument=source[start:b]
        source=source[:a]+'Zones_'+name+'(ZonesPosition_FromVector(*('+argument+'))'+source[b:]
    return source

def calls(source):
    # Remove complete owner features and share the prerequisite enums at their existing declarations
    if 'class CTheZones' in source:return header(source)
    if 'CTheZones::Init(void)\n{' in source:return general.calls(HOST)
    if 'enum eCarClass {' in source:
        source=source.replace(enum_body(source,'enum eCarClass {'),'')
        source=source.replace('#include "PathFind.h"','#include "CarClasses.h"\n#include "PathFind.h"',1)
    if 'enum eLevelName {' in source:source=source.replace(enum_body(source,'enum eLevelName {'),'#include "GameLevels.h"')
    if 'enum {\n\tGANG_CUBAN' in source:source=source.replace(enum_body(source,'enum {\n\tGANG_CUBAN'),'#include "GangTypes.h"')
    # Import the gang owner explicitly when a caller previously received it through Zones.h
    if 'CGangs::' in source and '#include "Gangs.h"' not in source and '#include "Zones.h"' in source:source=source.replace('#include "Zones.h"','#include "Zones.h"\n#include "Gangs.h"',1)
    # Import car-control declarations explicitly when a caller previously received them through Zones.h
    if ('CCarCtrl::' in source or 'GAME_SPEED_TO_METERS_PER_SECOND' in source) and '#include "CarCtrl.h"' not in source and '#include "Zones.h"' in source:source=source.replace('#include "Zones.h"','#include "Zones.h"\n#include "CarCtrl.h"',1)
    # Import the vehicle declaration used by pointer validation after removing transitive zone includes
    if 'IsVehiclePointerValid(' in source and '#include "Vehicle.h"' not in source and '#include "Zones.h"' in source:source=source.replace('#include "Zones.h"','#include "Zones.h"\n#include "Vehicle.h"',1)
    source=car_calls(source)
    for name in POSITIONS:source=position_calls(source,name)
    for name,target in FIELDS.items():source=re.sub(r'CTheZones::'+name+r'\b',target,source)
    source=re.sub(r'CTheZones::(\w+)\b',r'Zones_\1',source)
    source=source.replace('class CZone;','struct CZone;').replace('class CZoneInfo;','struct CZoneInfo;')
    return general.calls(re.sub(r'\b(m_pZone2?)\->GetTranslatedName\(\)',r'Zone_GetTranslatedName(\1)',source))

def records(source):
    # Preserve record field order while removing data classes and the translated-name member
    a=source.index('enum eZoneType');b=source.index('class CTheZones')
    source=source[a:b].replace('enum eZoneType','typedef enum eZoneType',1)
    i=source.index('};');source=source[:i]+source[i:].replace('};','} eZoneType;',1)
    for name in ['CZone','CZoneInfo']:
        source=source.replace('class '+name+'\n{\npublic:','typedef struct '+name+'\n{')
        a=source.index('typedef struct '+name+'\n');b=source.index('};',a)
        source=source[:b]+source[b:].replace('};','} '+name+';',1)
    source=source.replace('\twchar *GetTranslatedName(void);\n','').replace('CZone     *','struct CZone *')
    source=scalars(car_calls(source))
    return '//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#include "GameLevels.h"\n#include "../peds/GangTypes.h"\n#include "../control/CarClasses.h"\n'+source+'\n//- rouz edit (ChatGPT)\n'

def header(source):
    # Expose all original state, complete operations and six live record/index accessors in C
    owner=source[source.index('class CTheZones'):]
    result='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "config.h"
#include "ZoneRecords.h"
typedef struct ZonesPosition { float x, y, z; } ZonesPosition;
#ifdef __cplusplus
#include "Vector.h"
static inline ZonesPosition
ZonesPosition_FromVector(const CVector &position)
{
    // Copy the original vector coordinates into the C value argument
    ZonesPosition result = { position.x, position.y, position.z };
    return result;
}
extern "C" {
#endif
uint16_t *Zone_GetTranslatedName(CZone *zone);
ZonesPosition ZonesHost_Player(void);
'''
    for declaration in re.findall(r'\bstatic\s+([^;{}]+);',owner):
        # Retain exact scalar widths and array dimensions for every original static field
        if '(' in declaration:continue
        for name,target in FIELDS.items():declaration=re.sub(r'\b'+name+r'\b',target,declaration)
        result+='extern '+scalars(declaration.strip())+';\n'
    for signature,body in re.findall(r'\bstatic\s+([^;{}]+\([^{};]*\))\s*(?:;|\{([^}]+)\})',owner):
        signature=scalars(signature.strip())
        match=re.search(r'\b(\w+)\(',signature);name=match.group(1)
        signature=signature[:match.start()]+signature[match.start():].replace(name+'(', 'Zones_'+name+'(',1)
        signature=re.sub(r'(?:const CVector|CVector const)\s*\*\s*(\w+)',r'ZonesPosition \1',signature)
        signature=re.sub(r'CVector\s*\*\s*(\w+)',r'ZonesPosition \1',signature)
        if body:
            for field,target in FIELDS.items():body=re.sub(r'\b'+field+r'\b',target,body)
            result+='static inline '+signature+'\n{\n    // Preserve the original live record address or navigation pointer index\n    '+scalars(body.strip())+'\n}\n'
        else:result+=signature+';\n'
    return result+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def kernel(source):
    # Retain all hierarchy, density, label, audio traversal and serialization statements
    methods=re.findall(r'CTheZones::(\w+)\(',source)
    source=source[source.index('eLevelName CTheZones::m_CurrLevel;'):]
    source=source.replace('CZone::GetTranslatedName(void)','Zone_GetTranslatedName(CZone *zone)').replace('TheText.Get(name)','Text_Get(&TheText,zone->name)')
    source=source.replace('pos = FindPlayerCoors();','pos = ZonesHost_Player();')
    source=re.sub(r'(?:const CVector|CVector const)\s*\*\s*(\w+)',r'ZonesPosition \1',source)
    source=re.sub(r'CVector\s*\*\s*(\w+)',r'ZonesPosition \1',source)
    source=source.replace('v->','v.')
    source=re.sub(r'CVector (vmin|vmax)\(([^;]+)\);',r'ZonesPosition \1 = {\2};',source).replace('CVector pos;','ZonesPosition pos;')
    for name in ['vmin','vmax','pos']:source=source.replace('PointLiesWithinZone(&'+name+',','PointLiesWithinZone('+name+',')
    source=source.replace('GetLevelFromPosition(&pos)','GetLevelFromPosition(pos)')
    for name,target in FIELDS.items():source=re.sub(r'CTheZones::'+name+r'\b',target,source)
    source=re.sub(r'CTheZones::(\w+)\b',r'Zones_\1',source)
    for name,target in FIELDS.items():source=re.sub(r'\b'+name+r'\b',target,source)
    methods+=['GetInfoZone','GetNavigationZone','GetMapZone','GetAudioZone','GetPointerForNavigationZoneIndex','GetIndexForNavigationZonePointer']
    for name in methods:source=re.sub(r'(?<![\w])'+name+r'\(', 'Zones_'+name+'(',source)
    source=scalars(car_calls(source)).replace('ARRAY_SIZE(', 'ZONES_ARRAY_SIZE(')
    source=re.sub(r'\n\{\n', '\n{\n\t// Preserve the original zone operation and live state access order\n',source)
    return '''//+ rouz edit (ChatGPT)
#include "Zones.h"
#include "Text.h"
#include "Clock.h"
#include "Timer.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#undef assert
#ifndef MASTER
void re3_assert(const char *expression, const char *filename, unsigned int line, const char *function);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression, __FILE__, __LINE__, __func__), 0)))
#else
#define assert(expression) (expression)
#endif
#include "../save/SaveBuf.h"
#define ZONES_ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

'''+source

def project(source):
    # Register the complete C kernel and shared enum/record headers in the existing Visual Studio project
    if 'Zones.c"' not in source:
        entry='<ClCompile Include="..\\src\\core\\Zones.cpp" />'
        if entry in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\core\\Zones.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
        else:
            a=source.index('    <ClCompile Include="..\\src\\core\\Zones.cpp"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
            source=source[:b]+'\n'+source[a:b].replace('Zones.cpp','Zones.c')+source[b:]
    for path in ['core\\ZoneRecords.h','core\\GameLevels.h','peds\\GangTypes.h','control\\CarClasses.h']:
        if path in source:continue
        token='<ClInclude Include="..\\src\\core\\Zones.h" />'
        if token in source:source=source.replace(token,token+'\n    <ClInclude Include="..\\src\\'+path+'" />')
        else:
            a=source.index('    <ClInclude Include="..\\src\\core\\Zones.h"');b=source.index('</ClInclude>',a)+len('</ClInclude>')
            block=source[a:b].replace('core\\Zones.h',path)
            if path.startswith('control'):block=block.replace('core</Filter>','control</Filter>')
            if path.startswith('peds'):block=block.replace('core</Filter>','peds</Filter>')
            source=source[:b]+'\n'+block+source[b:]
    return general.project(source)
