"""Move the complete transmission owner and handling flags to C and remove obsolete audio readers."""
import re
import object_names_migration as objects
RENAMES={'src/vehicles/Transmission.cpp':'src/vehicles/Transmission.c'}
OPERATIONS=['InitGearRatios','CalculateGearForSimpleCar','CalculateDriveAcceleration']
READERS={'AudioVehicleRoadHost_MaxVelocity':'fMaxVelocity','AudioVehicleReverseHost_MaxReverse':'fMaxReverseVelocity','AudioVehicleEngineHost_GearCount':'nNumberOfGears','AudioVehicleEngineHost_DriveType':'nDriveType','AudioVehicleEngineHost_ShiftDown':'fShiftDownVelocity','AudioVehicleEngineHost_ShiftUp':'fShiftUpVelocity'}

def flags(original):
    # Relocate the complete unchanged handling flag enum without importing the C++ handling manager
    enum=re.search(r'enum\n\{\n\tHANDLING_1G_BOOST[\s\S]*?\n\};',original)[0]
    return '//+ rouz edit (ChatGPT)\n#pragma once\n'+enum+'\n//- rouz edit (ChatGPT)\n'

def header(original):
    # Keep both records and all native field types with explicit C pointer parameters for references
    gear=re.search(r'struct tGear\n\{[\s\S]*?\n\};',original)[0].replace('struct tGear','typedef struct tGear',1).replace('};','} tGear;')
    fields=original[original.index('\t// Gear 0'):original.index('\n\tvoid InitGearRatios')].replace('int8 ','int8_t ').replace('uint8 ','uint8_t ')
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
'''+gear+'''
typedef struct cTransmission {
'''+fields+'''
} cTransmission;
#ifdef __cplusplus
extern "C" {
#endif
void Transmission_InitGearRatios(cTransmission *self);
void Transmission_CalculateGearForSimpleCar(cTransmission *self,float speed,uint8_t *gear);
float Transmission_CalculateDriveAcceleration(cTransmission *self,const float *gasPedal,uint8_t *gear,float *time,const float *velocity,bool cheat);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Retain complete ordered algorithms, function statics and recursive references in a standalone C11 producer
    source=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
    source=source.replace('cTransmission::InitGearRatios(void)','Transmission_InitGearRatios(cTransmission *self)')
    source=source.replace('cTransmission::CalculateGearForSimpleCar(float speed, uint8 &gear)','Transmission_CalculateGearForSimpleCar(cTransmission *self,float speed,uint8_t *gear)')
    source=source.replace('cTransmission::CalculateDriveAcceleration(const float &gasPedal, uint8 &gear, float &time, const float &velocity, bool cheat)','Transmission_CalculateDriveAcceleration(cTransmission *self,const float *gasPedal,uint8_t *gear,float *time,const float *velocity,bool cheat)')
    source=source.replace('return CalculateDriveAcceleration(gasPedal, gear, time, fVelocity, false);','return Transmission_CalculateDriveAcceleration(self,gasPedal,gear,time,&fVelocity,false);')
    source=source.replace('nil','NULL')
    for field in ['Gears','nDriveType','nEngineType','nNumberOfGears','Flags','fEngineAcceleration','fMaxVelocity','fMaxCruiseVelocity','fMaxReverseVelocity','fCurVelocity']:
        source=re.sub(r'(?<![\w>.])'+field+r'\b','self->'+field,source)
    # Dereference only operation bodies so recursive argument pointers retain their original bindings
    starts=list(re.finditer(r'Transmission_(\w+)\([^\n]+\)\n\{',source))
    for match in reversed(starts):
        begin=match.end();end=source.index('\n}',begin);body=source[begin:end]
        for parameter in (['gear'] if match[1]=='CalculateGearForSimpleCar' else ['gasPedal','gear','velocity'] if match[1]=='CalculateDriveAcceleration' else []):
            body=re.sub(r'\b'+parameter+r'\b','(*'+parameter+')',body)
        body=body.replace('Transmission_CalculateDriveAcceleration(self,(*gasPedal),(*gear),time,&fVelocity,false)','Transmission_CalculateDriveAcceleration(self,gasPedal,gear,time,&fVelocity,false)')
        body='\n\t// Preserve the complete original transmission operation and its ordered live accesses'+body
        if match[1]=='CalculateDriveAcceleration':body='\n\t// Retain the unused time reference without reading or modifying its storage\n\t(void)time;'+body
        source=source[:begin]+body+source[end:]
    prefix='''//+ rouz edit (ChatGPT)
#include "Transmission.h"
#include "HandlingFlags.h"
#include "Timer.h"
#include <string.h>
#include <math.h>
#include <stddef.h>
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
static inline float sq(float value)
{
    // Preserve the original float square helper's expression and return type
    return value*value;
}
#define Abs(value) fabsf(value)
'''
    return prefix+source.strip()+'\n//- rouz edit (ChatGPT)\n'

def transmission_calls(source):
    # Replace the complete owner, known typed callers and every obsolete pure audio field reader
    original=source
    if 'class cTransmission\n' in source:return header(source)
    if 'cTransmission::InitGearRatios(void)' in source:return kernel(source)
    source=source.replace('class cTransmission;','struct cTransmission;')
    if 'struct tHandlingData\n' in source and 'HANDLING_1G_BOOST = 1,' in source:
        source=re.sub(r'enum\n\{\n\tHANDLING_1G_BOOST[\s\S]*?\n\};','#include "HandlingFlags.h"',source,count=1)
    source=re.sub(r'(handling->Transmission|pHandling->Transmission|mod_HandlingManager\.GetHandlingData\(HANDLING_(?:BAGGAGE|GOLFCART)\)->Transmission)\.InitGearRatios\(\)',r'Transmission_InitGearRatios(&\1)',source)
    source=re.sub(r'pHandling->Transmission\.CalculateGearForSimpleCar\(([^,\n]+), (\w+)\)',r'Transmission_CalculateGearForSimpleCar(&pHandling->Transmission, \1, &\2)',source)
    source=re.sub(r'pHandling->Transmission\.CalculateDriveAcceleration\((\w+), (\w+), (\w+), (\w+), (\w+)\)',r'Transmission_CalculateDriveAcceleration(&pHandling->Transmission, &\1, &\2, &\3, &\4, \5)',source)
    if 'typedef struct cAudioScriptObjectManager' in source and '#include "../vehicles/Transmission.h"' not in source:
        source=source.replace('#include <stdint.h>','#include <stdint.h>\n#include "../vehicles/Transmission.h"',1)
        source=source.replace('struct cTransmission;\n','').replace('typedef struct cTransmission cTransmission;\n','')
    for name,field in READERS.items():
        definition=re.search(r'(?:float|int8_t|char)\n'+name+r'\([^)]*\)\n\{([\s\S]*?)\n\}',source)
        if definition:
            body=re.sub(r'//[^\n]*','',definition[1]).strip();expected='return transmission->'+('Gears[gear].' if name.endswith(('ShiftDown','ShiftUp')) else '')+field+';'
            assert body==expected,(name,body)
            source=source.replace(definition[0]+'\n','')
        source=re.sub(r'^(?:float|int8_t|char) '+name+r'\([^)]*\);\n','',source,flags=re.M)
        if name.endswith(('ShiftDown','ShiftUp')):
            source=re.sub(r'\b'+name+r'\((transmission|params->m_pTransmission), ([^()]+)\)',lambda match:'('+match[1]+'->Gears['+match[2]+'].'+field+')',source)
        else:source=re.sub(r'\b'+name+r'\((transmission|params->m_pTransmission)\)',lambda match:'('+match[1]+'->'+field+')',source)
    if source==original:return source
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def transmission_project(source):
    # Preserve the complete producer rename and explicit C11 metadata in historical native projects
    source=source.replace('..\\src\\vehicles\\Transmission.cpp','..\\src\\vehicles\\Transmission.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\vehicles\\Transmission\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Keep unrelated per-file options and require the relocated producer to compile as C11
            body=match[1] or '\n    '
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body=body.rstrip()+'\n      <'+tag+'>'+value+'</'+tag+'>\n    '
            return '<ClCompile Include="..\\src\\vehicles\\Transmission.c">'+body+'</ClCompile>'
        source=re.sub(pattern,metadata,source)
    return objects.project(source)


def calls(source):
    # Preserve complete transmission ownership before removing both complete door classes
    import doors_migration as doors
    return doors.calls(transmission_calls(source))

def project(source):
    # Retain prior registrations before adding the independent C door-system producer
    import doors_migration as doors
    return doors.project(transmission_project(source))
