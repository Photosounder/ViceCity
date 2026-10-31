"""Move both sea-life classes, every operation and all canonical records into C11."""
import re
import tex_list_migration as textures
METHODS=[('CWaterCreature',name) for name in ['Initialise','Allocate','Free']]+[('CWaterCreatures',name) for name in ['GetFishStructSlot','CreateSeaLifeForm','IsSpaceForMoreWaterCreatures','CalculateFishHeading','CreateOne','FreeFishStructSlot','UpdateAll','RemoveAll']]
FIELDS=['m_pObj','m_fFwdSpeed','m_fZTurnSpeed','m_alpha','m_fWaterDepth','m_state']
OBSERVER=None
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CObject;
class CVector;
typedef CVector WaterCreaturePosition;
#else
typedef struct CObject CObject;
typedef struct CVector WaterCreaturePosition;
#endif
enum eFishSlotState { WATER_CREATURE_INIT=0,WATER_CREATURE_ACTIVE,WATER_CREATURE_FADE_OUT,WATER_CREATURE_REMOVE,WATER_CREATURE_DISABLED };
typedef struct WaterCreatureVector { float x,y,z; } WaterCreatureVector;
typedef struct WaterCreature {
    CObject *m_pObj;
    float m_fFwdSpeed,m_fZTurnSpeed;
    int32_t m_alpha;
    float m_fWaterDepth;
    int32_t m_state;
} WaterCreature;
typedef struct WaterCreatureProperties {
    int16_t *modelID;
    float fFwdSpeed,fLevel,fUnknown,fWaterDepth;
} WaterCreatureProperties;
#ifdef __cplusplus
extern "C" {
#endif
extern WaterCreature WaterCreatures_aWaterCreatures[NUM_WATER_CREATURES];
extern int32_t WaterCreatures_nNumActiveSeaLifeForms;
extern WaterCreatureProperties WaterCreatures_Properties[65];
void WaterCreature_Construct(WaterCreature *record);
void WaterCreature_Initialise(WaterCreature *record,CObject *object,float forward,float turn,float depth,uint32_t alpha,enum eFishSlotState state);
void WaterCreature_Allocate(WaterCreature *record,CObject *object,float forward,float turn,float depth,uint32_t alpha,enum eFishSlotState state);
void WaterCreature_Free(WaterCreature *record);
WaterCreature *WaterCreatures_GetFishStructSlot(void);
CObject *WaterCreatures_CreateSeaLifeForm(const WaterCreaturePosition *position,int16_t modelID,int32_t rotation);
bool WaterCreatures_IsSpaceForMoreWaterCreatures(void);
float WaterCreatures_CalculateFishHeading(const WaterCreaturePosition *position1,const WaterCreaturePosition *position2);
void WaterCreatures_CreateOne(const WaterCreaturePosition *position,int32_t modelID);
void WaterCreatures_FreeFishStructSlot(WaterCreature *record);
void WaterCreatures_UpdateAll(void);
void WaterCreatures_RemoveAll(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('int16_t *','TempCount','void','return &CObject::nNoTempObjects;'),
('CPool *','ObjectPool','void','return CPools::GetObjectPool();'),
('int8_t','CreatedBy','CObject *object','return object->ObjectCreatedBy;'),
('void','Construct','CObject *object,int16_t modelID','std::allocator<CObject>().construct(object,modelID,true);'),
('void','Destroy','CObject *object','object->~CObject();'),
('void','WorldRemove','CObject *object','CWorld::Remove(object);'),
('void','WorldAdd','CObject *object','CWorld::Add(object);'),
('void','SetPosition','CObject *object,const WaterCreaturePosition *position','object->SetPosition(*position);'),
('void','UpdateRW','CObject *object','object->GetMatrix().UpdateRW();'),
('void','RotateZ','CObject *object,float angle','object->GetMatrix().SetRotateZOnly(angle);'),
('void','SetCreatedBy','CObject *object,int8_t value','object->ObjectCreatedBy=value;'),
('void','SetStatic','CObject *object,bool value','object->bIsStatic=value;'),
('void','SetTouchingWater','CObject *object,bool value','object->bTouchingWater=value;'),
('void','SetUnderwater','CObject *object,bool value','object->bUnderwater=value;'),
('uint32_t *','EndTime','CObject *object','return &object->m_nEndOfLifeTime;'),
('void','SetAlpha','CObject *object,int32_t value','object->SetRwObjectAlpha(value);'),
('bool','OnScreen','CObject *object','return object->GetIsOnScreen();'),
('WaterCreatureVector','PlayerPosition','void','const CVector &value=FindPlayerPed()->GetPosition();WaterCreatureVector result={value.x,value.y,value.z};return result;'),
('WaterCreatureVector','Vector','const WaterCreaturePosition *position','WaterCreatureVector result={position->x,position->y,position->z};return result;'),
('WaterCreatureVector','Position','CObject *object','const CVector &value=object->GetPosition();WaterCreatureVector result={value.x,value.y,value.z};return result;'),
('WaterCreatureVector','Right','CObject *object','const CVector &value=object->GetRight();WaterCreatureVector result={value.x,value.y,value.z};return result;'),
('const WaterCreaturePosition *','ObjectPosition','CObject *object','return &object->GetPosition();'),
('bool','Visible','WaterCreaturePosition *position,float radius','return TheCamera.IsSphereVisible(*position,radius);'),
('bool','Depth','const WaterCreaturePosition *position,float *depth,float *level,float *ground','return CWaterLevel::GetWaterDepth(*position,depth,level,ground);'),
('void','SetZ','WaterCreaturePosition *position,float value','position->z=value;')]
for name,field in [('MoveSpeed','m_vecMoveSpeed'),('TurnSpeed','m_vecTurnSpeed')]:READERS.append(('void','Set'+name,'CObject *object,WaterCreatureVector value','object->'+field+'=CVector(value.x,value.y,value.z);'))
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "WaterCreatures.h"
#include "Pool.h"
enum { WATER_CREATURE_TEMP_OBJECT=3,WATER_CREATURE_CONTROLLED_SUB_OBJECT=5 };
typedef void (*WaterCreatureSpawnCallback)(WaterCreatureVector,WaterCreaturePosition *,int32_t);
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' WaterCreatures_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'''void WaterCreatures_BackendWithFishPosition(const WaterCreaturePosition *position,WaterCreatureVector player,int32_t modelID,WaterCreatureSpawnCallback callback);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "WaterCreatures_backend.h"
#include "World.h"
#include "WaterLevel.h"
#include "Camera.h"
#include "PlayerPed.h"
#include "Object.h"
#include "Pools.h"
static_assert(TEMP_OBJECT==WATER_CREATURE_TEMP_OBJECT && CONTROLLED_SUB_OBJECT==WATER_CREATURE_CONTROLLED_SUB_OBJECT,"Original native creation tags");
'''+''.join(result+' WaterCreatures_Backend'+name+'('+args+')\n{\n    // Preserve the actual typed owner operation and original borrowed-field access\n    '+body+'\n}\n' for result,name,args,body in READERS)+'''void WaterCreatures_BackendWithFishPosition(const WaterCreaturePosition *position,WaterCreatureVector player,int32_t modelID,WaterCreatureSpawnCallback callback)
{
    // Keep the original native fish-position copy alive through visibility, depth, allocation and publication
    CVector fishPos=*position;callback(player,&fishPos,modelID);
}
//- rouz edit (ChatGPT)
'''
HELPERS='''static WaterCreatureVector WaterCreatures_Subtract(WaterCreatureVector a,WaterCreatureVector b)
{
    // Preserve every original component subtraction
    WaterCreatureVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static float WaterCreatures_MagnitudeSqr(WaterCreatureVector value)
{
    // Preserve the original ordered sum of squared components
    return value.x*value.x+value.y*value.y+value.z*value.z;
}
static void WaterCreatures_Normalise(WaterCreatureVector *value)
{
    // Preserve the original reciprocal-square-root normalization and x-only zero fallback
    float sq=WaterCreatures_MagnitudeSqr(*value);if(sq>0.0f){
        // Apply the same captured reciprocal to each original component
        float inverse=1.0f/sqrtf(sq);value->x*=inverse;value->y*=inverse;value->z*=inverse;
    }else value->x=1.0f;
}
static WaterCreatureVector WaterCreatures_Multiply(WaterCreatureVector value,float scalar)
{
    // Preserve the original vector-left multiplication operand order
    WaterCreatureVector result={value.x*scalar,value.y*scalar,value.z*scalar};return result;
}
static float WaterCreatures_HeadingValues(WaterCreatureVector position1,WaterCreatureVector position2)
{
    // Normalize the same captured difference before the original integer random heading offset
    WaterCreatureVector delta=WaterCreatures_Subtract(position1,position2);WaterCreatures_Normalise(&delta);
    return General_GetRandomNumberInRangeInt(-90,90)+WATER_CREATURE_RADTODEG(atan2f(-delta.x,delta.y)+WATER_CREATURE_HALFPI+WATER_CREATURE_PI);
}
static void WaterCreatures_CreateOneWithPosition(WaterCreatureVector playerPos,WaterCreaturePosition *fishPos,int32_t modelID);
'''

def method(source,owner,name):
    # Delimit the entire actual method without depending on edit-marker footers
    match=re.search(r'(?m)^(?:(?:CWaterCreature|CObject)\s*\*|void|bool|float)\s*'+owner+r'::'+name+r'\([^\n]*\)\s*\{',source);assert match,(owner,name)
    a=source.index('{',match.start());b=a+1;depth=1
    while depth:depth+=(source[b]=='{')-(source[b]=='}');b+=1
    return source[match.start():b]

def kernel(source):
    # Move every complete record/controller method and all sixty-five real model-pointer property rows
    normalized=re.sub(r'\s+','',re.sub(r'//[^\n]*','',source))
    assert 'CWaterCreature::CWaterCreature(){Free();}' in normalized
    a=source.index('struct WaterCreatureProperties aProperties[65] = {');b=source.index('\n};',a)+3
    properties=source[a:b].replace('struct WaterCreatureProperties aProperties','WaterCreatureProperties WaterCreatures_Properties')
    chunks=[]
    for owner,name in METHODS:
        text=method(source,owner,name)
        if name=='CreateOne':
            a=text.index('\tfloat fDepth, fLevelNoWaves;');callback=text[a:]
            text='''void WaterCreatures_CreateOne(const WaterCreaturePosition *pos,int32_t modelID)
{
    // Capture the original player value before copying the caller's still-live native position
    if(!WaterCreatures_IsSpaceForMoreWaterCreatures())return;
    WaterCreatureVector player=WaterCreatures_BackendPlayerPosition();
    WaterCreatures_BackendWithFishPosition(pos,player,modelID,WaterCreatures_CreateOneWithPosition);
}
static void WaterCreatures_CreateOneWithPosition(WaterCreatureVector playerPos,WaterCreaturePosition *fishPos,int32_t modelID)
{
    // Retain the native fish-position reference throughout the complete original creation sequence
'''+callback
        elif name=='CalculateFishHeading':
            text='''float WaterCreatures_CalculateFishHeading(const WaterCreaturePosition *pos1,const WaterCreaturePosition *pos2)
{
    // Read the same two borrowed native positions before the complete C heading calculation
    return WaterCreatures_HeadingValues(WaterCreatures_BackendVector(pos1),WaterCreatures_BackendVector(pos2));
}'''
        elif owner=='CWaterCreature':
            text=text.replace('CWaterCreature::'+name+'(','WaterCreature_'+name+'(WaterCreature *record,').replace('(WaterCreature *record,)','(WaterCreature *record)')
            text=text.replace('this->','record->').replace('CWaterCreature::Initialise(','WaterCreature_Initialise(record,')
        else:text=text.replace('CWaterCreatures::'+name+'(','WaterCreatures_'+name+'(')
        chunks.append(text)
    body='\n\n'.join(chunks)
    body=body.replace('CWaterCreatures::CalculateFishHeading(playerPos, fishPos)','WaterCreatures_HeadingValues(playerPos,WaterCreatures_BackendVector(fishPos))')
    body=body.replace('CVector playerPos = FindPlayerPed()->GetPosition();','WaterCreatureVector playerPos=WaterCreatures_BackendPlayerPosition();')
    body=body.replace('TheCamera.IsSphereVisible(fishPos, 3.0f)','WaterCreatures_BackendVisible(fishPos,3.0f)').replace('CWaterLevel::GetWaterDepth(fishPos,','WaterCreatures_BackendDepth(fishPos,')
    body=body.replace('fishPos.z = fLevelNoWaves - creature->fLevel;','WaterCreatures_BackendSetZ(fishPos,fLevelNoWaves-creature->fLevel);')
    body=body.replace('CObject::nNoTempObjects','(*WaterCreatures_BackendTempCount())').replace('CPools::GetObjectPool()','WaterCreatures_BackendObjectPool()').replace('CObjectPool *','CPool *')
    body=body.replace('std::allocator<CObject>().construct(pObj, modelID, true)','WaterCreatures_BackendConstruct(pObj,modelID)')
    for object in ['aWaterCreatures[i].m_pObj','existing','pObj']:
        body=body.replace('CWorld::Remove('+object+')','WaterCreatures_BackendWorldRemove('+object+')').replace('CWorld::Add('+object+')','WaterCreatures_BackendWorldAdd('+object+')').replace(object+'->~CObject()','WaterCreatures_BackendDestroy('+object+')')
        body=body.replace(object+'->ObjectCreatedBy == TEMP_OBJECT','WaterCreatures_BackendCreatedBy('+object+') == WATER_CREATURE_TEMP_OBJECT')
        body=body.replace(object+'->ObjectCreatedBy = CONTROLLED_SUB_OBJECT','WaterCreatures_BackendSetCreatedBy('+object+',WATER_CREATURE_CONTROLLED_SUB_OBJECT)')
        for field,name,value in [('bIsStatic','Static','false'),('bTouchingWater','TouchingWater','true'),('bUnderwater','Underwater','true')]:body=body.replace(object+'->'+field+' = '+value,'WaterCreatures_BackendSet'+name+'('+object+','+value+')')
        body=body.replace(object+'->m_nEndOfLifeTime','(*WaterCreatures_BackendEndTime('+object+'))').replace(object+'->GetMatrix().UpdateRW()','WaterCreatures_BackendUpdateRW('+object+')')
    body=body.replace('pObj->SetPosition(pos)','WaterCreatures_BackendSetPosition(pObj,pos)').replace('pObj->GetMatrix().SetRotateZOnly(DEGTORAD(zRotAngle))','WaterCreatures_BackendRotateZ(pObj,WATER_CREATURE_DEGTORAD(zRotAngle))')
    for field,name in [('m_vecMoveSpeed','MoveSpeed'),('m_vecTurnSpeed','TurnSpeed')]:body=body.replace('pObj->'+field+' = CVector(0.0f, 0.0f, 0.0f)','WaterCreatures_BackendSet'+name+'(pObj,(WaterCreatureVector){0.0f,0.0f,0.0f})')
    obj='aWaterCreatures[i].m_pObj'
    body=body.replace('fish->SetRwObjectAlpha(255)','WaterCreatures_BackendSetAlpha(fish,255)')
    body=re.sub(re.escape(obj)+r'->SetRwObjectAlpha\(([^\n;]+)\)',lambda match:'WaterCreatures_BackendSetAlpha('+obj+','+match[1]+')',body)
    body=body.replace(obj+'->GetIsOnScreen()','WaterCreatures_BackendOnScreen('+obj+')')
    body=body.replace('(playerPos - '+obj+'->GetPosition()).MagnitudeSqr()','WaterCreatures_MagnitudeSqr(WaterCreatures_Subtract(playerPos,WaterCreatures_BackendPosition('+obj+')))')
    for value in ['fwd','speed']:
        body=body.replace('CVector '+value+' = '+obj+'->GetRight()','WaterCreatureVector '+value+'=WaterCreatures_BackendRight('+obj+')').replace(value+'.Normalise()','WaterCreatures_Normalise(&'+value+')')
    body=body.replace(obj+'->m_vecMoveSpeed = fwd * aWaterCreatures[i].m_fFwdSpeed','WaterCreatures_BackendSetMoveSpeed('+obj+',WaterCreatures_Multiply(fwd,aWaterCreatures[i].m_fFwdSpeed))')
    body=body.replace(obj+'->m_vecTurnSpeed = CVector(0.0f, 0.0f, aWaterCreatures[i].m_fZTurnSpeed)','WaterCreatures_BackendSetTurnSpeed('+obj+',(WaterCreatureVector){0.0f,0.0f,aWaterCreatures[i].m_fZTurnSpeed})')
    body=body.replace(obj+'->m_vecMoveSpeed = speed','WaterCreatures_BackendSetMoveSpeed('+obj+',speed)')
    body=body.replace('CWaterLevel::GetWaterDepth('+obj+'->GetPosition(),','WaterCreatures_BackendDepth(WaterCreatures_BackendObjectPosition('+obj+'),')
    body=body.replace('wc->Allocate(','WaterCreature_Allocate(wc,').replace('wc->Free()','WaterCreature_Free(wc)')
    body=re.sub(r'\bCWaterCreatures::','WaterCreatures_',body)
    for name in ['GetFishStructSlot','CreateSeaLifeForm','IsSpaceForMoreWaterCreatures','FreeFishStructSlot']:body=re.sub(r'(?<![\w])'+name+r'\(', 'WaterCreatures_'+name+'(',body)
    body=body.replace('nNumActiveSeaLifeForms','WaterCreatures_nNumActiveSeaLifeForms').replace('aWaterCreatures','WaterCreatures_aWaterCreatures').replace('aProperties','WaterCreatures_Properties')
    for old,new in [('CWaterCreature','WaterCreature'),('int32','int32_t'),('int16','int16_t'),('uint32','uint32_t')]:body=re.sub(r'\b'+old+r'\b',new,body)
    body=body.replace('eFishSlotState state','enum eFishSlotState state').replace('CVector const& pos,','const WaterCreaturePosition *pos,').replace('nil','NULL').replace('SQR(75.0f)','(75.0f*75.0f)')
    body=re.sub(r'(?m)^((?:WaterCreature\s*\*|bool|void)\s*WaterCreatures_(?:GetFishStructSlot|IsSpaceForMoreWaterCreatures|UpdateAll|RemoveAll))\(\)',r'\1(void)',body)
    body=re.sub(r'(?m)^((?:static\s+)?(?:void|bool|float|CObject\s*\*|WaterCreature\s*\*)\s*WaterCreatures?_\w+\([^\n]*\)\s*\{\n)',r'\1    // Preserve every original state write, native call and traversal decision\n',body)
    body=re.sub(r'(?m)^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original branch and side-effect ordering\n',body)
    code=re.sub(r'//[^\n]*|"(?:\\.|[^"\\])*"','',body);assert not re.search(r'\b(?:CVector|CWaterCreature|CWaterCreatures|CWaterLevel|CWorld|CPools|std|this)\b|->Get|\.Normalise\(',code)
    initial='{NULL,0.0f,0.0f,0,0.0f,WATER_CREATURE_DISABLED}'
    return '''//+ rouz edit (ChatGPT)
#include "WaterCreatures_backend.h"
#include "ModelIndexData.h"
#include "General.h"
#include "Timer.h"
#include <math.h>
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define WATER_CREATURE_PI 3.14159265358979323846f
#define WATER_CREATURE_HALFPI (WATER_CREATURE_PI/2)
#define WATER_CREATURE_DEGTORAD(value) ((value)*WATER_CREATURE_PI/180.0f)
#define WATER_CREATURE_RADTODEG(value) ((value)*180.0f/WATER_CREATURE_PI)
_Static_assert(NUM_WATER_CREATURES==8,"Original complete statically constructed slot pool");
int32_t WaterCreatures_nNumActiveSeaLifeForms;
WaterCreature WaterCreatures_aWaterCreatures[NUM_WATER_CREATURES]={
'''+',\n'.join('    '+initial for i in range(8))+'''\n};
'''+properties+'\n\n'+HELPERS+'''void WaterCreature_Construct(WaterCreature *record)
{
    // Execute the same complete clearing operation as the original constructor
    WaterCreature_Free(record);
}
'''+body+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Replace both whole classes and preserve each existing native argument by passing its actual address
    if 'CWaterCreature' not in source:return textures.calls(source)
    if 'int CWaterCreatures::nNumActiveSeaLifeForms;' in source:
        if OBSERVER:OBSERVER(source)
        return textures.calls(HOST)
    if 'class CWaterCreature {' in source:return textures.calls(HEADER)
    result=[]
    for line in source.splitlines(keepends=True):
        changed=line.replace('CWaterCreatures::','WaterCreatures_')
        if 'CWaterCreatures::CreateOne(' in line:changed=re.sub(r'WaterCreatures_CreateOne\((\w+),',r'WaterCreatures_CreateOne(&\1,',changed)
        if changed!=line:changed=changed.rstrip('\n')+' // rouz edit (ChatGPT)\n'
        result.append(changed)
    return textures.calls(''.join(result))

def project(source):
    # Register a unique C11 sea-life unit and retain the existing paired-source object distinctions
    entry='<ClCompile Include="..\\src\\renderer\\WaterCreatures.cpp" />'
    if '..\\src\\renderer\\SeaLife.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\SeaLife.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\WaterCreatures.h" />'
    if '..\\src\\renderer\\WaterCreatures_backend.h"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\WaterCreatures_backend.h" />')
    return textures.project(source)

def verify_legacy(source,transform,root):
    # Keep whole historical caller evidence by verifying the complete relocated algorithm after prior migrations
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/renderer/WaterCreatures.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/renderer/SeaLife.c').read_text())
