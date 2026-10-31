"""Move every restart-point operation and canonical save-compatible point table into C11."""
import re
from audio_controls_migration import function
import remote_migration as remote
METHODS=['Initialise','AddHospitalRestartPoint','AddPoliceRestartPoint','OverrideNextRestart','CancelOverrideRestart','FindClosestHospitalRestartPoint','FindClosestPoliceRestartPoint','LoadAllRestartPoints','SaveAllRestartPoints']
SCALARS=[('uint8_t','OverrideHospitalLevel'),('uint8_t','OverridePoliceStationLevel'),('bool','bFadeInAfterNextArrest'),('bool','bFadeInAfterNextDeath'),('bool','bOverrideRestart'),('RestartPoint','OverridePosition'),('float','OverrideHeading'),('uint16_t','NumberOfHospitalRestarts'),('uint16_t','NumberOfPoliceRestarts')]
OBSERVER=None
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "GameLevels.h"
#define NUM_RESTART_POINTS 8
typedef struct RestartPoint {float x,y,z;} RestartPoint;
#ifdef __cplusplus
class CVector;
typedef CVector RestartPosition;
extern "C" {
#else
typedef struct CVector RestartPosition;
#endif
'''+''.join('extern '+kind+' Restart_'+name+';\n' for kind,name in SCALARS)+'''extern RestartPoint Restart_HospitalRestartPoints[NUM_RESTART_POINTS];
extern float Restart_HospitalRestartHeadings[NUM_RESTART_POINTS];
extern RestartPoint Restart_PoliceRestartPoints[NUM_RESTART_POINTS];
extern float Restart_PoliceRestartHeadings[NUM_RESTART_POINTS];
void Restart_Initialise(void);
void Restart_AddHospitalRestartPoint(const RestartPosition *position,float heading);
void Restart_AddPoliceRestartPoint(const RestartPosition *position,float heading);
void Restart_OverrideNextRestart(const RestartPosition *position,float heading);
void Restart_CancelOverrideRestart(void);
void Restart_FindClosestHospitalRestartPoint(const RestartPosition *position,RestartPosition *outPosition,float *outHeading);
void Restart_FindClosestPoliceRestartPoint(const RestartPosition *position,RestartPosition *outPosition,float *outHeading);
void Restart_LoadAllRestartPoints(uint8_t *buffer,uint32_t size);
void Restart_SaveAllRestartPoints(uint8_t *buffer,uint32_t *size);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "Restart.h"
#ifdef __cplusplus
extern "C" {
#endif
RestartPoint Restart_BackendPosition(const RestartPosition *position);
void Restart_BackendSetPosition(RestartPosition *position,RestartPoint value);
RestartPoint Restart_BackendClosestNode(const RestartPosition *position);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Restart_backend.h"
#include "PathFind.h"
RestartPoint Restart_BackendPosition(const RestartPosition *position)
{
    // Read the original still-live native position without assuming its object layout
    RestartPoint result={position->x,position->y,position->z};return result;
}
void Restart_BackendSetPosition(RestartPosition *position,RestartPoint value)
{
    // Assign the original native output object with explicit coordinate transfers
    *position=CVector(value.x,value.y,value.z);
}
RestartPoint Restart_BackendClosestNode(const RestartPosition *position)
{
    // Preserve the actual native path search and selected node's original fixed-point conversion
    CVector result=ThePaths.m_pathNodes[ThePaths.FindNodeClosestToCoors(*position,PATH_PED,999999.9f)].GetPosition();
    RestartPoint point={result.x,result.y,result.z};return point;
}
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Transform every complete operation while retaining all original decisions and save transfers
    text='\n\n'.join(function(source,'CRestart::',name) for name in METHODS).replace('CRestart::','Restart_')
    text=text.replace('const CVector &pos','const RestartPosition *pos').replace('CVector *outPos','RestartPosition *outPos')
    text=text.replace('= CVector(0.0f, 0.0f, 0.0f);','= (RestartPoint){0.0f,0.0f,0.0f};')
    text=text.replace(' = pos;',' = Restart_BackendPosition(pos);')
    text=text.replace('ZonesPosition_FromVector(*(&pos))','Restart_AsZones(Restart_BackendPosition(pos))')
    for array in ['HospitalRestartPoints','PoliceRestartPoints']:
        text=text.replace('ZonesPosition_FromVector(*(&'+array+'[i]))','Restart_AsZones('+array+'[i])')
        text=text.replace('(pos - '+array+'[i]).MagnitudeSqr()','Restart_DistanceSqr(Restart_BackendPosition(pos),'+array+'[i])')
    text=text.replace('*outPos = ThePaths.m_pathNodes[ThePaths.FindNodeClosestToCoors(pos, PATH_PED, 999999.9f)].GetPosition();','Restart_BackendSetPosition(outPos,Restart_BackendClosestNode(pos));')
    text=re.sub(r'\*outPos = ([^;]+);',r'Restart_BackendSetPosition(outPos,\1);',text)
    old='printf("Couldn\'t find a hospital restart zone near the player %f %f %f->%f %f %f\\n", pos.x, pos.y, pos.z, outPos->x, outPos->y, outPos->z);'
    new='RestartPoint diagnosticPosition=Restart_BackendPosition(pos),diagnosticOutput=Restart_BackendPosition(outPos);\n\t\tprintf("Couldn\'t find a hospital restart zone near the player %f %f %f->%f %f %f\\n", diagnosticPosition.x,diagnosticPosition.y,diagnosticPosition.z,diagnosticOutput.x,diagnosticOutput.y,diagnosticOutput.z);'
    assert old in text;text=text.replace(old,new)
    for name in METHODS:text=re.sub(r'(?<![\w])'+name+r'\(', 'Restart_'+name+'(',text)
    names=[name for kind,name in SCALARS]+['HospitalRestartPoints','HospitalRestartHeadings','PoliceRestartPoints','PoliceRestartHeadings']
    for name in names:text=re.sub(r'\b'+name+r'\b','Restart_'+name,text)
    for old,new in [('uint8','uint8_t'),('uint16','uint16_t'),('uint32','uint32_t')]:text=re.sub(r'\b'+old+r'\b',new,text)
    for name in ['Initialise','CancelOverrideRestart']:text=text.replace('Restart_'+name+'()\n','Restart_'+name+'(void)\n')
    text=text.replace('SQR(4000.0f)','(4000.0f*4000.0f)')
    text=re.sub(r'(?m)^(Restart_\w+\([^\n]*\)\n\{\n)',r'\1    // Preserve every original state access, branch decision and transfer order\n',text)
    code=re.sub(r'//[^\n]*|"(?:\\.|[^"\\])*"','',text);assert not re.search(r'\b(?:CRestart|CVector|ThePaths|ZonesPosition_FromVector)\b|pos\.|outPos->',code),code
    return '''//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "Restart_backend.h"
#include "Zones.h"
#include <stdio.h>
#undef assert
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "SaveBuf.h"
'''+''.join(kind+' Restart_'+name+';\n' for kind,name in SCALARS)+'''RestartPoint Restart_HospitalRestartPoints[NUM_RESTART_POINTS];
float Restart_HospitalRestartHeadings[NUM_RESTART_POINTS];
RestartPoint Restart_PoliceRestartPoints[NUM_RESTART_POINTS];
float Restart_PoliceRestartHeadings[NUM_RESTART_POINTS];
static ZonesPosition Restart_AsZones(RestartPoint point)
{
    // Copy every captured coordinate into the actual C zone-query argument
    ZonesPosition result={point.x,point.y,point.z};return result;
}
static float Restart_DistanceSqr(RestartPoint a,RestartPoint b)
{
    // Retain the original component subtraction and ordered sum of squared differences
    RestartPoint delta={a.x-b.x,a.y-b.y,a.z-b.z};return delta.x*delta.x+delta.y*delta.y+delta.z*delta.z;
}

'''+text+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Replace the whole class and every caller while retaining actual native input/output objects
    if 'CRestart' not in source:return remote.calls(source)
    if 'uint8 CRestart::OverrideHospitalLevel;' in source:
        if OBSERVER:OBSERVER(source)
        return HOST
    if 'class CRestart\n' in source:return HEADER
    lines=[]
    for line in source.splitlines(keepends=True):
        changed=line.replace('CRestart::','Restart_')
        for name in ['AddHospitalRestartPoint','AddPoliceRestartPoint','OverrideNextRestart']:changed=changed.replace('Restart_'+name+'(pos,','Restart_'+name+'(&pos,')
        for name in ['FindClosestHospitalRestartPoint','FindClosestPoliceRestartPoint']:changed=changed.replace('Restart_'+name+'(pPlayerInfo.GetPos(),','Restart_'+name+'(&pPlayerInfo.GetPos(),')
        if changed!=line:changed=changed.rstrip('\n')+' // rouz edit (ChatGPT)\n'
        lines.append(changed)
    return remote.calls(''.join(lines))

def project(source):
    # Register the unique C11 point-table controller and its native boundary declarations
    entry='<ClCompile Include="..\\src\\control\\Restart.cpp" />'
    if '..\\src\\control\\RestartPoints.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\control\\RestartPoints.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\control\\Restart.h" />'
    if '..\\src\\control\\Restart_backend.h"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\control\\Restart_backend.h" />')
    return remote.project(source)

def verify_legacy(source,transform,root):
    # Retain the complete relocated algorithm as well as the typed boundary in older whole-caller audits
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/control/Restart.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/control/RestartPoints.c').read_text())
