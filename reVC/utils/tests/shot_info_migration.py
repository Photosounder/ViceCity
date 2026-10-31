"""Project complete shot allocation and fire propagation policy into a genuine C owner."""
import re
from rect_migration import mask
RENAMES={'src/weapons/ShotInfo.cpp':'src/weapons/ShotInfo.c'}


def header():
    # Expose genuine C state while retaining native entities as opaque separately owned records
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "WeaponType.h"
#ifdef __cplusplus
class CEntity;
extern "C" {
#else
typedef struct CEntity CEntity;
#endif
typedef struct ShotInfoVector {float x,y,z;} ShotInfoVector;
typedef struct ShotInfo {
    enum eWeaponType m_weapon;
    ShotInfoVector m_startPos,m_areaAffected;
    float m_radius;
    CEntity *m_sourceEntity;
    float m_timeout;
    bool m_inUse;
} ShotInfo;
extern ShotInfo gaShotInfo[NUMSHOTINFOS];
extern float ShotInfo_ms_afRandTable[20];
#ifdef SQUEEZE_PERFORMANCE
extern uint32_t shotInfoInUse;
#endif
void ShotInfo_Initialise(void);
bool ShotInfo_AddShot(CEntity *sourceEntity,enum eWeaponType weapon,ShotInfoVector startPos,ShotInfoVector endPos);
void ShotInfo_Shutdown(void);
void ShotInfo_Update(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''


def boundary():
    # Keep only separately owned entity operations and vector normalization on native objects
    rows=[('bool','IsPed','CEntity *entity','return entity->IsPed();'),('bool','Valid','CEntity *entity','return ((CPed*)entity)->IsPointerValid();'),('int32_t','NearCount','CEntity *entity','return ((CPed*)entity)->m_numNearPeds;'),('CEntity *','NearPed','CEntity *entity,int32_t index','return ((CPed*)entity)->m_nearPeds[index];'),('bool','InControl','CEntity *entity','return ((CPed*)entity)->IsPedInControl();'),('ShotInfoVector','Position','CEntity *entity','const CVector &position=entity->GetPosition();ShotInfoVector result={position.x,position.y,position.z};return result;'),('bool','FireProof','CEntity *entity','return entity->bFireProof;'),('bool','IsPlayer','CEntity *entity','return ((CPed*)entity)->IsPlayer();'),('void','Flee','CEntity *entity,CEntity *source,uint32_t duration','((CPed*)entity)->SetFindPathAndFlee(source,duration);'),('void','Sprint','CEntity *entity','((CPed*)entity)->SetMoveState(PEDMOVE_SPRINT);'),('void','CarsOnFire','float x,float y,float z,float radius,CEntity *source','CWorld::SetCarsOnFire(x,y,z,radius,source);'),('void','Normalise','ShotInfoVector *value','CVector vector(value->x,value->y,value->z);vector.Normalise();value->x=vector.x;value->y=vector.y;value->z=vector.z;'),('void','Debug','const char *message','debug("%s",message);'),('void','Assert','const char *expression,const char *file,uint32_t line,const char *function','re3_assert(expression,file,line,function);')]
    h='//+ rouz edit (ChatGPT)\n#pragma once\n#include "ShotInfo.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n';native='//+ rouz edit (ChatGPT)\n#include "common.h"\n#include "ShotInfo_backend.h"\n#include "Ped.h"\n#include "World.h"\n'
    for kind,name,args,expression in rows:
        # Bind one native primitive without adding shot selection or fire propagation decisions
        declaration=kind+' ShotInfo_Host'+name+'('+args+')';h+=declaration+';\n';native+=declaration+'\n{\n    // Use the actual separately owned native entity or vector operation\n    '+expression+'\n}\n'
    return h+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n',native+'//- rouz edit (ChatGPT)\n'


def producer(original):
    # Preserve every complete original operation while replacing vector operators and native field syntax
    source=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
    source=source.replace('CShotInfo::','ShotInfo_')
    for token in reversed(list(re.finditer(r'\bCShotInfo\b',mask(source)))):
        # Change only actual class tokens while retaining original diagnostic strings
        source=source[:token.start()]+'ShotInfo'+source[token.end():]
    source=source.replace('CVector(0.0f, 0.0f, 0.0f)','(ShotInfoVector){0.0f, 0.0f, 0.0f}').replace('CVector startPos, CVector endPos','ShotInfoVector startPos, ShotInfoVector endPos').replace('eWeaponType weapon','enum eWeaponType weapon')
    source=re.sub(r'\bms_afRandTable\b','ShotInfo_ms_afRandTable',source)
    source=re.sub(r'\buint32\b','uint32_t',source).replace('nil','NULL').replace('debug(','ShotInfo_HostDebug(')
    source=source.replace('ShotInfo &shot = gaShotInfo[slot];','ShotInfo *shot = &gaShotInfo[slot];').replace('shot.','shot->')
    source=source.replace('endPos - startPos','ShotInfo_Difference(endPos,startPos)').replace('gaShotInfo[slot].m_areaAffected.Normalise();','ShotInfo_HostNormalise(&gaShotInfo[slot].m_areaAffected);')
    source=re.sub(r'gaShotInfo\[slot\]\.m_areaAffected \*= ([^;]+);',r'ShotInfo_Scale(&gaShotInfo[slot].m_areaAffected,\1);',source)
    source=source.replace('shot->m_areaAffected *= pow(0.96, (*Timer_GetTimeStep()));','ShotInfo_Scale(&shot->m_areaAffected,(float)pow(0.96, (*Timer_GetTimeStep())));')
    source=source.replace('assert(','ShotInfo_Assert(')
    source=source.replace('shot->m_startPos += (*Timer_GetTimeStep()) * shot->m_areaAffected;','ShotInfo_Advance(&shot->m_startPos,(*Timer_GetTimeStep()),&shot->m_areaAffected);')
    source=source.replace('shot->m_sourceEntity->IsPed()','ShotInfo_HostIsPed(shot->m_sourceEntity)').replace('((CPed*)shot->m_sourceEntity)->IsPointerValid()','ShotInfo_HostValid(shot->m_sourceEntity)').replace('CPed *ped = (CPed*) shot->m_sourceEntity;','CEntity *ped = shot->m_sourceEntity;')
    source=source.replace('ped->m_numNearPeds','ShotInfo_HostNearCount(ped)').replace('CPed *nearPed = ped->m_nearPeds[i];','CEntity *nearPed = ShotInfo_HostNearPed(ped,i);')
    source=source.replace('nearPed->IsPointerValid()','ShotInfo_HostValid(nearPed)').replace('nearPed->IsPedInControl()','ShotInfo_HostInControl(nearPed)').replace('(nearPed->GetPosition() - shot->m_startPos).MagnitudeSqr()','ShotInfo_DistanceSquared(ShotInfo_HostPosition(nearPed),&shot->m_startPos)').replace('nearPed->bFireProof','ShotInfo_HostFireProof(nearPed)').replace('nearPed->IsPlayer()','ShotInfo_HostIsPlayer(nearPed)').replace('nearPed->SetFindPathAndFlee(shot->m_sourceEntity, 10000)','ShotInfo_HostFlee(nearPed,shot->m_sourceEntity,10000)').replace('nearPed->SetMoveState(PEDMOVE_SPRINT)','ShotInfo_HostSprint(nearPed)').replace('CWorld::SetCarsOnFire','ShotInfo_HostCarsOnFire')
    source=re.sub(r'(ShotInfo_(?:Initialise|Shutdown|Update))\(\)',r'\1(void)',source)
    source=re.sub(r'((?:void|bool)\s+ShotInfo_(?:Initialise|Shutdown|Update|AddShot)\([^{}]*\)\s*\{)',r'\1\n    // Execute the complete original shot operation with C-owned state',source)
    helpers='''
static ShotInfoVector ShotInfo_Difference(ShotInfoVector end,ShotInfoVector start)
{
    // Preserve the original component subtraction before storing the entire vector
    ShotInfoVector result={end.x-start.x,end.y-start.y,end.z-start.z};return result;
}
static void ShotInfo_Scale(ShotInfoVector *value,float scale)
{
    // Convert the scalar once before the original component multiplication order
    value->x*=scale;value->y*=scale;value->z*=scale;
}
static void ShotInfo_Advance(ShotInfoVector *position,float step,const ShotInfoVector *speed)
{
    // Form the original complete temporary vector before adding it into position
    ShotInfoVector delta={step*speed->x,step*speed->y,step*speed->z};position->x+=delta.x;position->y+=delta.y;position->z+=delta.z;
}
static float ShotInfo_DistanceSquared(ShotInfoVector position,const ShotInfoVector *center)
{
    // Retain float subtraction and squared magnitude without changing the caller threshold
    ShotInfoVector delta=ShotInfo_Difference(position,*center);return delta.x*delta.x+delta.y*delta.y+delta.z*delta.z;
}
'''
    return '//+ rouz edit (ChatGPT)\n#include "ShotInfo.h"\n#include "ShotInfo_backend.h"\n#include "WeaponInfo.h"\n#include "General.h"\n#include "Timer.h"\n#include "Fire.h"\n#include <math.h>\n#include <stddef.h>\n#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))\n#define Max(a,b) ((a)>(b)?(a):(b))\n#ifndef MASTER\n#define ShotInfo_Assert(expression) ((void)((!!(expression)) || (ShotInfo_HostAssert(#expression,__FILE__,__LINE__,__func__),0)))\n#else\n#define ShotInfo_Assert(expression) (expression)\n#endif\n'+helpers+source+'\n//- rouz edit (ChatGPT)\n'


def calls(source):
    # Convert the complete owner and the sole native weapon caller without retaining a class adapter
    if not re.search(r'\bCShotInfo\b',mask(source)):return source
    if re.search(r'class\s+CShotInfo\s*\{',mask(source)):return header()
    if 'CShotInfo::AddShot(CEntity *sourceEntity' in source:return producer(source)
    adapter='''([](CEntity *source,enum eWeaponType weapon,CVector start,CVector end){
    // Preserve original by-value native vector arguments before entering complete C policy
    ShotInfoVector first={start.x,start.y,start.z},last={end.x,end.y,end.z};return ShotInfo_AddShot(source,weapon,first,last);
})'''
    source=source.replace('CShotInfo::AddShot',adapter)
    source=re.sub(r'\bCShotInfo::(\w+)',r'ShotInfo_\1',source)
    lines=source.splitlines(keepends=True)
    for i in reversed(range(len(lines))):
        # Explain each converted executable caller at its original operation boundary
        if re.search(r'ShotInfo_(?:Initialise|Shutdown|Update)\(',lines[i]):lines.insert(i,re.match(r'[ \t]*',lines[i])[0]+'// Enter the complete C shot operation\n')
    return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n'


def project(source):
    # Select the complete C owner and only native primitive operations in generated projects
    import object_names_migration as objects
    source=source.replace('src\\weapons\\ShotInfo.cpp','src\\weapons\\ShotInfo.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"\n]*src\\weapons\\ShotInfo\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve existing options while explicitly selecting genuine C11 compilation
            value=match[2] or ''
            for tag,text in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in value:value=re.sub('<'+tag+'>[^<]*</'+tag+'>','<'+tag+'>'+text+'</'+tag+'>',value)
                else:value+='\n      <'+tag+'>'+text+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+value.rstrip()+'\n    </ClCompile>';native=match[1][:-len('ShotInfo.c')]+'ShotInfo_native.cpp'
            if '<ClCompile Include="'+native+'"' not in source:result+='\n    <ClCompile Include="'+native+'" />'
            return result
        source=re.sub(pattern,metadata,source)
    return objects.project(source)
