"""Replace the complete static cull-zone owner with C11 state and operations."""
import re
import zones_migration as zones

FIELDS={'NumAttributeZones':('int32_t','CullZones_numAttributeZones'),
 'aAttributeZones':('CAttributeZone','CullZones_attributeZones'),
 'CurrentWantedLevelDrop_Player':('int32_t','CullZones_playerWantedLevelDrop'),
 'CurrentFlags_Camera':('int32_t','CullZones_cameraFlags'),
 'CurrentFlags_Player':('int32_t','CullZones_playerFlags'),
 'bCurrentSubwayIsInvisible':('bool','CullZones_subwayInvisible'),
 'bAtBeachForAudio':('bool','CullZones_atBeachForAudio')}
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Building.h"
#include "Treadable.h"
#include "Train.h"
#include "Pools.h"
#include "Camera.h"
#include "World.h"
#include "ZoneCull.h"

'''
READERS=[('CullZonesPosition','GameCamera','void','CullZonesPosition_FromVector(TheCamera.GetGameCamPosition())'),
 ('CullZonesPosition','Player','void','CullZonesPosition_FromVector(FindPlayerCoors())'),
 ('float','CameraX','void','TheCamera.GetPosition().x'),
 ('float','CameraY','void','TheCamera.GetPosition().y'),
 ('CPool *','BuildingPool','void','CPools::GetBuildingPool()'),
 ('CPool *','TreadablePool','void','CPools::GetTreadablePool()'),
 ('CPool *','VehiclePool','void','CPools::GetVehiclePool()'),
 ('CEntity *','Building','void *slot','(CBuilding *)slot'),
 ('CEntity *','Treadable','void *slot','(CTreadable *)slot'),
 ('CVehicle *','Vehicle','void *slot','(CVehicle *)slot'),
 ('bool','IsSubway','CEntity *entity','entity->bIsSubway'),
 ('bool','IsTrain','CVehicle *vehicle','vehicle->IsTrain()'),
 ('uint8_t','Track','CVehicle *vehicle','((CTrain *)vehicle)->m_nTrackId')]
for kind,name,args,value in READERS:
    HOST+='extern "C" '+kind+'\nCullZonesHost_'+name+'('+args+')\n{\n    // Read the original typed game input at its existing access point\n    return '+value+';\n}\n\n'
for kind,name in [('CEntity','Entity'),('CVehicle','Vehicle')]:
    HOST+='extern "C" void\nCullZonesHost_Set'+name+'Visible('+kind+' *entity, bool visible)\n{\n    // Update the original visibility bit after the C eligibility checks\n    entity->bIsVisible = visible;\n}\n\n'
HOST+='//- rouz edit (ChatGPT)\n'

def vector_calls(source,name,pointer=False):
    # Wrap only the original coordinate argument while retaining nested constructor expressions
    token='CCullZones::'+name+'('
    while token in source:
        a=source.index(token);start=a+len(token);end=start;depth=0
        while end<len(source):
            if source[end] in ',)' and depth==0:break
            depth+=(source[end]=='(')-(source[end]==')');end+=1
        arg=source[start:end]
        replacement='&'+arg if pointer else 'CullZonesPosition_FromVector('+arg+')'
        source=source[:a]+'CullZones_'+name+'('+replacement+source[end:]
    return source

def calls(source):
    # Replace the complete class declaration or implementation before rewriting ordinary callers
    if 'class CCullZones' in source:return header(source)
    if 'CCullZones::Init(void)\n{' in source:return HOST
    for name in ['FindAttributesForCoors','ForceCullZoneCoors']:source=vector_calls(source,name)
    source=vector_calls(source,'AddCullZone',True)
    for name,(_,target) in FIELDS.items():source=re.sub(r'CCullZones::'+name+r'\b',target,source)
    return zones.calls(re.sub(r'CCullZones::(\w+)\b',r'CullZones_\1',source))

def header(source):
    # Retain attribute values and all eight signed 16-bit record fields with a C typedef
    oldh=source
    a=source.index('enum eZoneAttribs');b=source.index('class CCullZones')
    source=source[a:b].replace('struct CAttributeZone','typedef struct CAttributeZone').replace('int16 ','int16_t ').replace('\n};\n\n','\n};\n\n',1)
    i=source.rfind('\n};');source=source[:i]+source[i:].replace('\n};','\n} CAttributeZone;',1)
    result='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#include "config.h"\n#include "Pool.h"\n'+source
    result+='typedef struct CullZonesPosition { float x, y, z; } CullZonesPosition;\n'
    result+='''#ifdef __cplusplus
#include "Vector.h"
class CEntity;
class CVehicle;
static inline CullZonesPosition
CullZonesPosition_FromVector(const CVector &position)
{
    // Copy the original vector coordinates into the C value argument
    CullZonesPosition result = { position.x, position.y, position.z };
    return result;
}
extern "C" {
#else
typedef struct CEntity CEntity;
typedef struct CVehicle CVehicle;
#endif
'''
    for name,(kind,target) in FIELDS.items():result+='extern '+kind+' '+target+('[NUMATTRIBZONES]' if name=='aAttributeZones' else '')+';\n'
    result+='''void CullZones_Init(void);
void CullZones_Update(void);
void CullZones_UpdateAtBeachForAudio(void);
void CullZones_ForceCullZoneCoors(CullZonesPosition coors);
int32_t CullZones_FindAttributesForCoors(CullZonesPosition coors, int32_t *wantedLevel);
CAttributeZone *CullZones_FindZoneWithStairsAttributeForPlayer(void);
void CullZones_MarkSubwayAsInvisible(bool visible);
void CullZones_AddCullZone(const void *position, float minx, float maxx, float miny, float maxy, float minz, float maxz, uint16_t flag, int16_t wantedLevel);
bool IsPointWithinArbitraryArea(float px, float py, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
'''
    for kind,name,args,_ in READERS:result+=kind+' CullZonesHost_'+name+'('+args+');\n'
    result+='void CullZonesHost_SetEntityVisible(CEntity *entity, bool visible);\nvoid CullZonesHost_SetVehicleVisible(CVehicle *entity, bool visible);\n'
    for kind,name,body in re.findall(r'static (bool|int32) (\w+)\(void\) \{([^}]+)\}',oldh):
        for field,(_,target) in FIELDS.items():body=re.sub(r'\b'+field+r'\b',target,body)
        result+='static inline '+('int32_t' if kind=='int32' else kind)+' CullZones_'+name+'(void)\n{\n    // Read the original live zone flags or wanted-level drop\n    '+body.strip()+'\n}\n'
    return result+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def kernel(source):
    # Preserve the complete original operation bodies and descending pool traversal
    source=source[source.index('int32     CCullZones::NumAttributeZones;'):]
    source=source.replace('CVector const &position','const void *position').replace('CVector','CullZonesPosition')
    source=source.replace('TheCamera.GetGameCamPosition()','CullZonesHost_GameCamera()').replace('FindPlayerCoors()','CullZonesHost_Player()')
    source=source.replace('TheCamera.GetPosition().x','CullZonesHost_CameraX()').replace('TheCamera.GetPosition().y','CullZonesHost_CameraY()')
    for name in ['Building','Treadable','Vehicle']:
        source=source.replace('CPools::Get'+name+'Pool()', 'CullZonesHost_'+name+'Pool()')
        typ='C'+name
        source=source.replace('(('+typ+'*)CPool_GetSlot(CullZonesHost_'+name+'Pool(), i))','CullZonesHost_'+name+'(CPool_GetSlot(CullZonesHost_'+name+'Pool(), i))')
    source=source.replace('e->bIsSubway','CullZonesHost_IsSubway(e)').replace('e->bIsVisible = visible','CullZonesHost_SetEntityVisible(e, visible)')
    source=source.replace('v->IsTrain()','CullZonesHost_IsTrain(v)').replace('((CTrain*)v)->m_nTrackId != TRACK_ELTRAIN','CullZonesHost_Track(v) != 0').replace('v->bIsVisible = visible','CullZonesHost_SetVehicleVisible(v, visible)')
    for name,(_,target) in FIELDS.items():source=re.sub(r'CCullZones::'+name+r'\b',target,source)
    source=re.sub(r'CCullZones::(\w+)\b',r'CullZones_\1',source)
    for name,(_,target) in FIELDS.items():source=re.sub(r'\b'+name+r'\b',target,source)
    for name in ['UpdateAtBeachForAudio','FindAttributesForCoors','MarkSubwayAsInvisible']:
        source=re.sub(r'(?<![\w])'+name+r'\(', 'CullZones_'+name+'(',source)
    for a,b in [('int32','int32_t'),('int16','int16_t'),('uint16','uint16_t'),('nil','NULL'),('Max','CULL_ZONES_MAX')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=re.sub(r'(\n\{\n)',r'\1\t// Preserve the original zone operation and live state access order\n',source)
    pre='''//+ rouz edit (ChatGPT)
#include "ZoneCull.h"
#include "Timer.h"
#undef assert
#ifndef MASTER
void re3_assert(const char *expression, const char *filename, unsigned int line, const char *function);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression, __FILE__, __LINE__, __func__), 0)))
#else
#define assert(expression) (expression)
#endif
#define CULL_ZONES_MAX(a,b) ((a) > (b) ? (a) : (b))

'''
    return pre+source

def project(source):
    # Register the complete zone operations as C11 beside their typed C++ adapters
    if 'ZoneCull.c"' in source:return zones.project(source)
    entry='<ClCompile Include="..\\src\\core\\ZoneCull.cpp" />'
    if entry in source:return zones.project(source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\core\\ZoneCull.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'))
    a=source.index('    <ClCompile Include="..\\src\\core\\ZoneCull.cpp"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return zones.project(source[:b]+'\n'+source[a:b].replace('ZoneCull.cpp','ZoneCull.c')+source[b:])
