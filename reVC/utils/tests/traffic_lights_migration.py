"""Move complete traffic-light phases, rendering decisions and path/vehicle algorithms into C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import weapon_info_migration as weapons
METHODS=['DisplayActualLight','ScanForLightsOnMap','FindTrafficLightType','LightForPeds','LightForCars1','LightForCars2','LightForCars1_Visual','LightForCars2_Visual','ShouldCarStopForLight','ShouldCarStopForBridge']
OBSERVER=None
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CEntity;class CVehicle;
extern "C" {
#else
typedef struct CEntity CEntity;
typedef struct CVehicle CVehicle;
#endif
enum {
    PED_LIGHTS_WALK,PED_LIGHTS_WALK_BLINK,PED_LIGHTS_DONT_WALK,
    CAR_LIGHTS_GREEN=0,CAR_LIGHTS_YELLOW,CAR_LIGHTS_RED,CAR_LIGHTS_NONE
};
extern bool TrafficLights_bGreenLightsCheat;
void TrafficLights_DisplayActualLight(CEntity *entity);
void TrafficLights_ScanForLightsOnMap(void);
int32_t TrafficLights_FindTrafficLightType(CEntity *entity);
uint8_t TrafficLights_LightForPeds(void);
uint8_t TrafficLights_LightForCars1(void);
uint8_t TrafficLights_LightForCars2(void);
uint8_t TrafficLights_LightForCars1_Visual(void);
uint8_t TrafficLights_LightForCars2_Visual(void);
bool TrafficLights_ShouldCarStopForLight(CVehicle *vehicle,bool alwaysStop);
bool TrafficLights_ShouldCarStopForBridge(CVehicle *vehicle);
bool TrafficLights_DoesLineSegmentIntersect(float x1,float y1,float x2,float y2,float u1,float v1,float u2,float v2);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BOUNDARIES=[
('float','UpZ','CEntity *entity','return entity->GetUp().z;'),
('bool','Damaged','CEntity *entity','return entity->bRenderDamaged;'),
('int32_t','Model','CEntity *entity','return entity->GetModelIndex();'),
('TrafficLightModel *','ModelInfo','int32_t model','return CModelInfo::GetModelInfo(model);'),
('bool','HasEffect','TrafficLightModel *model,int32_t index','return model->Get2dEffect(index)!=nil;'),
('TrafficLightVector','EffectPosition','TrafficLightModel *model,int32_t index','return TrafficLights_FromNative(model->Get2dEffect(index)->pos);'),
('TrafficLightVector','Transform','CEntity *entity,TrafficLightVector point','return TrafficLights_FromNative(entity->GetMatrix()*TrafficLights_ToNative(point));'),
('TrafficLightVector','CameraForward','void','return TrafficLights_FromNative(TheCamera.GetForward());'),
('float','GroundBrightness','void','return CTimeCycle::GetLightOnGroundBrightness();'),
('float','SpriteBrightness','void','return CTimeCycle::GetSpriteBrightness();'),
('float','SpriteSize','void','return CTimeCycle::GetSpriteSize();'),
('void','Shadow','uintptr_t id,TrafficLightVector position,uint8_t red,uint8_t green,uint8_t blue','CVector native=TrafficLights_ToNative(position);CShadows::StoreStaticShadow(id,SHADOWTYPE_ADDITIVE,gpShadowExplosionTex,&native,8.0f,0.0f,0.0f,-8.0f,128,red,green,blue,12.0f,1.0f,40.0f,false,0.0f);'),
('void','Corona','uintptr_t id,uint8_t red,uint8_t green,uint8_t blue,TrafficLightVector position,float size','CCoronas::RegisterCorona(id,red,green,blue,255,TrafficLights_ToNative(position),size,50.0f,CCoronas::TYPE_STAR,CCoronas::FLARE_NONE,CCoronas::REFLECTION_ON,CCoronas::LOSCHECK_OFF,CCoronas::STREAK_OFF,0.0f);'),
('int32_t','NumSectorsX','void','return NUMSECTORS_X;'),
('int32_t','NumSectorsY','void','return NUMSECTORS_Y;'),
('TrafficLightList *','SectorList','int32_t x,int32_t y','return &CWorld::GetSector(x,y)->m_lists[ENTITYLIST_DUMMIES];'),
('TrafficLightNode *','First','TrafficLightList *list','return list->first;'),
('TrafficLightNode *','Next','TrafficLightNode *node','return node->next;'),
('CEntity *','NodeEntity','TrafficLightNode *node','return (CEntity *)node->item;'),
('int32_t','CarNodeCount','void','return ThePaths.m_numCarPathNodes;'),
('int32_t','NodeCount','void','return ThePaths.m_numPathNodes;'),
('TrafficLightVector','PathPosition','int32_t index','return TrafficLights_FromNative(ThePaths.m_pathNodes[index].GetPosition());'),
('int32_t','NumLinks','int32_t index','return ThePaths.m_pathNodes[index].numLinks;'),
('int32_t','FirstLink','int32_t index','return ThePaths.m_pathNodes[index].firstLink;'),
('int32_t','ConnectedNode','int32_t index','return ThePaths.ConnectedNode(index);'),
('int32_t','CarConnection','int32_t index','return ThePaths.m_carPathConnections[index];'),
('uint8_t','LightType','int32_t index','return ThePaths.m_carPathLinks[index].trafficLightType;'),
('void','SetLightType','int32_t index,int32_t type','ThePaths.m_carPathLinks[index].trafficLightType=type;'),
('bool','LightDirection','int32_t index','return ThePaths.m_carPathLinks[index].trafficLightDirection;'),
('void','SetLightDirection','int32_t index,bool direction','ThePaths.m_carPathLinks[index].trafficLightDirection=direction;'),
('int32_t','LinkNode','int32_t index','return ThePaths.m_carPathLinks[index].pathNodeIndex;'),
('bool','CrossesRoad','int32_t index','return ThePaths.ConnectionCrossesRoad(index);'),
('void','SetConnectionLight','int32_t index','ThePaths.ConnectionSetTrafficLight(index);'),
('bool','BridgeLight','int32_t index','return ThePaths.m_carPathLinks[index].bBridgeLights;'),
('TrafficLightVector','LinkPosition','int32_t index','CVector2D v=ThePaths.m_carPathLinks[index].GetPosition();return TrafficLightVector{v.x,v.y,0.0f};'),
('TrafficLightVector','LinkDirection','int32_t index','CVector2D v=ThePaths.m_carPathLinks[index].GetDirection();return TrafficLightVector{v.x,v.y,0.0f};'),
('TrafficLightVector','VehiclePosition','CVehicle *vehicle','const CVector &value=vehicle->GetPosition();return TrafficLightVector{value.x,value.y,0.0f};'),
('int32_t','VehicleStatus','CVehicle *vehicle','return vehicle->GetStatus();')]
for axis in ['x','y','z']:BOUNDARIES.append(('float','Effect'+axis.upper(),'TrafficLightModel *model,int32_t index','return model->Get2dEffect(index)->pos.'+axis+';'))
for part in ['Up','Right','Forward']:BOUNDARIES.append(('TrafficLightVector',part,'CEntity *entity','return TrafficLights_FromNative(entity->Get'+part+'());'))
for axis in ['X','Y']:BOUNDARIES.append(('float','Forward'+axis,'CEntity *entity','return entity->GetForward().'+axis.lower()+';'))
for axis in ['X','Y']:BOUNDARIES.append(('float','Position'+axis,'CEntity *entity','return entity->GetPosition().'+axis.lower()+';'))
for axis in ['X','Y']:BOUNDARIES.append(('float','Path'+axis,'int32_t index','return ThePaths.m_pathNodes[index].Get'+axis+'();'))
for field in ['NextPathNodeInfo','CurrentPathNodeInfo','PreviousPathNodeInfo','NextRouteNode','CurrentRouteNode','PrevRouteNode','NextDirection','CurrentDirection','PreviousDirection']:
    BOUNDARIES.append(('int32_t',field,'CVehicle *vehicle','return vehicle->AutoPilot.m_n'+field+';'))
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "TrafficLights.h"
typedef struct TrafficLightVector {float x,y,z;} TrafficLightVector;
#ifdef __cplusplus
class CBaseModelInfo;class CPtrList;class CPtrNode;
typedef CBaseModelInfo TrafficLightModel;
typedef CPtrList TrafficLightList;
typedef CPtrNode TrafficLightNode;
extern "C" {
#else
typedef struct CBaseModelInfo TrafficLightModel;
typedef struct CPtrList TrafficLightList;
typedef struct CPtrNode TrafficLightNode;
#endif
'''+''.join(kind+' TrafficLights_Backend'+name+'('+args+');\n' for kind,name,args,body in BOUNDARIES)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "TrafficLights_backend.h"
#include "ModelInfo.h"
#include "Camera.h"
#include "Coronas.h"
#include "PathFind.h"
#include "Shadows.h"
#include "Timecycle.h"
#include "Vehicle.h"
#include "World.h"
static TrafficLightVector TrafficLights_FromNative(const CVector &value)
{
    // Materialize the original native vector at its value boundary
    TrafficLightVector result={value.x,value.y,value.z};return result;
}
static CVector TrafficLights_ToNative(TrafficLightVector value)
{
    // Supply the same three original coordinates to the still-native engine operation
    return CVector(value.x,value.y,value.z);
}
'''+''.join(kind+' TrafficLights_Backend'+name+'('+args+')\n{\n    // Enter the original typed native field or action at the C decision point\n    '+body+'\n}\n' for kind,name,args,body in BOUNDARIES)+'//- rouz edit (ChatGPT)\n'

def arguments(source,opening):
    # Keep nested vector, arithmetic and function expressions together at each original call boundary
    parts=[];start=opening+1;depth=1;i=start
    while depth:
        if source[i] in '([{':depth+=1
        elif source[i] in ')]}':depth-=1
        if source[i]==',' and depth==1:parts.append(source[start:i]);start=i+1
        i+=1
    parts.append(source[start:i-1]);return i,parts

def rewrite_calls(source,name,replacement):
    # Rewrite complete actual calls while leaving their original argument expressions and evaluation counts intact
    changes=[]
    for match in re.finditer(re.escape(name)+r'\(',source):
        end,args=arguments(source,match.end()-1);changes.append((match.start(),end,replacement([arg.strip() for arg in args])))
    for start,end,text in reversed(changes):source=source[:start]+text+source[end:]
    return source

def kernel(source):
    # Relocate all ten complete methods and the intersection helper without reducing their decision scope
    chunks=[]
    for name in ['DoesLineSegmentIntersect',*METHODS]:
        text=function(source,'' if name=='DoesLineSegmentIntersect' else 'CTrafficLights::',name)
        text=re.sub(r'(?m)^\s*//[+-] rouz edit \(ChatGPT\)\n','\n',text).replace(' // rouz edit (ChatGPT)','')
        text=text.replace('CTrafficLights::','TrafficLights_').replace('CBaseModelInfo* mi','TrafficLightModel *mi')
        text=text.replace('CPtrNode *node','TrafficLightNode *node').replace('CPtrList &list = CWorld::GetSector(x, y)->m_lists[ENTITYLIST_DUMMIES]','TrafficLightList *list = TrafficLights_BackendSectorList(x,y)')
        text=text.replace('list.first','TrafficLights_BackendFirst(list)').replace('node->next','TrafficLights_BackendNext(node)').replace('(CEntity*)node->item','TrafficLights_BackendNodeEntity(node)')
        text=text.replace('NUMSECTORS_X','TrafficLights_BackendNumSectorsX()').replace('NUMSECTORS_Y','TrafficLights_BackendNumSectorsY()')
        text=text.replace('CModelInfo::GetModelInfo(','TrafficLights_BackendModelInfo(')
        for axis in ['x','y','z']:text=re.sub(r'mi->Get2dEffect\(([^()]*)\)->pos\.'+axis,r'TrafficLights_BackendEffect'+axis.upper()+r'(mi,\1)',text)
        text=re.sub(r'mi->Get2dEffect\(([^()]*)\)->pos',r'TrafficLights_BackendEffectPosition(mi,\1)',text)
        text=re.sub(r'mi->Get2dEffect\(([^()]*)\)',r'TrafficLights_BackendHasEffect(mi,\1)',text)
        for entity in ['ent','light']:
            text=text.replace(entity+'->GetModelIndex()','TrafficLights_BackendModel('+entity+')').replace(entity+'->bRenderDamaged','TrafficLights_BackendDamaged('+entity+')')
            for axis in ['x','y']:
                text=text.replace(entity+'->GetForward().'+axis,'TrafficLights_BackendForward'+axis.upper()+'('+entity+')').replace(entity+'->GetPosition().'+axis,'TrafficLights_BackendPosition'+axis.upper()+'('+entity+')')
            text=text.replace(entity+'->GetUp().z','TrafficLights_BackendUpZ('+entity+')')
            for part in ['Up','Right','Forward']:text=text.replace(entity+'->Get'+part+'()','TrafficLights_Backend'+part+'('+entity+')')
            text=rewrite_calls(text,entity+'->GetMatrix() * CVector',lambda args:'TrafficLights_BackendTransform('+entity+',TrafficLights_Make('+','.join(args)+'))')
            text=re.sub(re.escape(entity)+r'->GetMatrix\(\) \* (TrafficLights_BackendEffectPosition\(mi,[^()]*\))',r'TrafficLights_BackendTransform('+entity+r',\1)',text)
        text=text.replace('TheCamera.GetForward()','TrafficLights_BackendCameraForward()')
        text=text.replace('CVector(0.0f, 0.0f, 0.0f)','TrafficLights_Make(0.0f,0.0f,0.0f)')
        text=text.replace('-TrafficLights_BackendRight(ent)','TrafficLights_Negate(TrafficLights_BackendRight(ent))')
        text=text.replace('(pos1 + pos2) / 2','TrafficLights_Divide(TrafficLights_Add(pos1,pos2),2)')
        text=text.replace('DotProduct(','TrafficLights_Dot(')
        text=text.replace('CVector2D(vehicle->GetPosition()) - ThePaths.m_carPathLinks[node].GetPosition()','TrafficLights_Sub(TrafficLights_BackendVehiclePosition(vehicle),TrafficLights_BackendLinkPosition(node))')
        text=text.replace('ThePaths.m_carPathLinks[node].GetDirection()','TrafficLights_BackendLinkDirection(node)').replace('DotProduct2D(','TrafficLights_Dot2D(')
        text=text.replace('(ThePaths.m_pathNodes[i].GetPosition() - pos1).MagnitudeSqr()','TrafficLights_MagnitudeSqr(TrafficLights_Sub(TrafficLights_BackendPathPosition(i),pos1))')
        text=re.sub(r'ThePaths.m_pathNodes\[([^\]]+)\].GetPosition\(\)',r'TrafficLights_BackendPathPosition(\1)',text)
        for axis in ['X','Y']:text=re.sub(r'ThePaths.m_pathNodes\[([^\]]+)\].Get'+axis+r'\(\)',r'TrafficLights_BackendPath'+axis+r'(\1)',text)
        for field,target in [('numLinks','NumLinks'),('firstLink','FirstLink')]:text=re.sub(r'ThePaths.m_pathNodes\[([^\]]+)\].'+field,r'TrafficLights_Backend'+target+r'(\1)',text)
        for field,target in [('m_numCarPathNodes','CarNodeCount'),('m_numPathNodes','NodeCount')]:text=text.replace('ThePaths.'+field,'TrafficLights_Backend'+target+'()')
        text=text.replace('ThePaths.ConnectedNode(','TrafficLights_BackendConnectedNode(').replace('ThePaths.ConnectionCrossesRoad(','TrafficLights_BackendCrossesRoad(').replace('ThePaths.ConnectionSetTrafficLight(','TrafficLights_BackendSetConnectionLight(')
        text=re.sub(r'ThePaths.m_carPathConnections\[([^\]]+)\]',r'TrafficLights_BackendCarConnection(\1)',text)
        text=text.replace('ThePaths.m_carPathLinks[link].trafficLightType = FindTrafficLightType(light)','TrafficLights_BackendSetLightType(link,FindTrafficLightType(light))')
        text=text.replace('ThePaths.m_carPathLinks[link].trafficLightDirection = true','TrafficLights_BackendSetLightDirection(link,true)')
        for field,target in [('trafficLightType','LightType'),('trafficLightDirection','LightDirection'),('pathNodeIndex','LinkNode'),('bBridgeLights','BridgeLight')]:text=re.sub(r'ThePaths.m_carPathLinks\[([^\]]+)\].'+field,r'TrafficLights_Backend'+target+r'(\1)',text)
        for field in ['NextPathNodeInfo','CurrentPathNodeInfo','PreviousPathNodeInfo','NextRouteNode','CurrentRouteNode','PrevRouteNode','NextDirection','CurrentDirection','PreviousDirection']:text=text.replace('vehicle->AutoPilot.m_n'+field,'TrafficLights_Backend'+field+'(vehicle)')
        text=text.replace('vehicle->GetStatus()','TrafficLights_BackendVehicleStatus(vehicle)')
        for field,target in [('GetLightOnGroundBrightness','GroundBrightness'),('GetSpriteBrightness','SpriteBrightness'),('GetSpriteSize','SpriteSize')]:text=text.replace('CTimeCycle::'+field+'()','TrafficLights_Backend'+target+'()')
        text=rewrite_calls(text,'PointLights_AddLightLegacy',lambda args:'PointLights_AddLight('+','.join([args[0],'TrafficLights_PointVector('+args[1]+')','TrafficLights_PointVector('+args[2]+')',*args[3:]])+')')
        text=rewrite_calls(text,'BrightLights_RegisterOneLegacy',lambda args:'BrightLights_RegisterOne('+','.join(['TrafficLights_BrightVector('+arg+')' for arg in args[:4]]+args[4:]+['0','0','0'])+')')
        text=rewrite_calls(text,'CShadows::StoreStaticShadow',lambda args:'TrafficLights_BackendShadow('+','.join([args[0].replace('(uintptr)','(uintptr_t)'),args[3].removeprefix('&'),*args[9:12]])+')')
        text=rewrite_calls(text,'CCoronas::RegisterCorona',lambda args:'TrafficLights_BackendCorona('+','.join([args[0].replace('(uintptr)','(uintptr_t)'),*args[1:4],args[5],args[6]])+')')
        text=text.replace('CVector','TrafficLightVector').replace('bGreenLightsCheat','TrafficLights_bGreenLightsCheat')
        text=text.replace('RADTODEG(','TRAFFICLIGHTS_RADTODEG(').replace('Abs(','fabsf(')
        for operation in ['DoesLineSegmentIntersect',*METHODS]:text=re.sub(r'(?<![\w])'+operation+r'\(', 'TrafficLights_'+operation+'(',text)
        for old,new in [('uint32','uint32_t'),('uint8','uint8_t')]:text=re.sub(r'\b'+old+r'\b',new,text)
        if name=='FindTrafficLightType':text=re.sub(r'^int\b','int32_t',text)
        text=text.replace('\n{','\n{\n    // Preserve the complete original '+name+' operation and ordered live accesses',1)
        chunks.append(text)
    return PRELUDE+'\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

PRELUDE='''//+ rouz edit (ChatGPT)
#include "TrafficLights_backend.h"
#include "Timer.h"
#include "General.h"
#include "Weather.h"
#include "ModelIndexData.h"
#include "PointLights.h"
#include "BrightLights.h"
#include "EntityStatus.h"
#include <math.h>
#define TRAFFICLIGHTS_PI ((float)3.14159265358979323846)
#define TRAFFICLIGHTS_RADTODEG(x) ((x)*180.0f/TRAFFICLIGHTS_PI)
#define SQR(x) ((x)*(x))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
bool TrafficLights_bGreenLightsCheat;
static TrafficLightVector TrafficLights_Make(float x,float y,float z)
{
    // Materialize the original three-component value
    TrafficLightVector value={x,y,z};return value;
}
static TrafficLightVector TrafficLights_Sub(TrafficLightVector a,TrafficLightVector b)
{
    // Preserve the original component-wise subtraction
    return TrafficLights_Make(a.x-b.x,a.y-b.y,a.z-b.z);
}
static TrafficLightVector TrafficLights_Add(TrafficLightVector a,TrafficLightVector b)
{
    // Preserve the original component-wise addition
    return TrafficLights_Make(a.x+b.x,a.y+b.y,a.z+b.z);
}
static TrafficLightVector TrafficLights_Divide(TrafficLightVector value,float divisor)
{
    // Preserve each original component division rather than introducing reciprocal multiplication
    return TrafficLights_Make(value.x/divisor,value.y/divisor,value.z/divisor);
}
static TrafficLightVector TrafficLights_Negate(TrafficLightVector value)
{
    // Preserve the opposite side vector used for the second traffic-light face
    return TrafficLights_Make(-value.x,-value.y,-value.z);
}
static float TrafficLights_Dot(TrafficLightVector a,TrafficLightVector b)
{
    // Preserve the original ordered three-component dot product
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static float TrafficLights_Dot2D(TrafficLightVector a,TrafficLightVector b)
{
    // Preserve the original two-component stopping distance
    return a.x*b.x+a.y*b.y;
}
static float TrafficLights_MagnitudeSqr(TrafficLightVector value)
{
    // Preserve the original three-term squared scan distance
    return value.x*value.x+value.y*value.y+value.z*value.z;
}
static PointLightsVector TrafficLights_PointVector(TrafficLightVector value)
{
    // Enter the existing C point-light registration with the same coordinates
    PointLightsVector result={value.x,value.y,value.z};return result;
}
static BrightLightVector TrafficLights_BrightVector(TrafficLightVector value)
{
    // Enter the existing C bright-light registration with the same coordinates
    BrightLightVector result={value.x,value.y,value.z};return result;
}
'''

def traffic_calls(source):
    # Replace the complete static class and every actual caller with the C API or canonical flag
    if 'class CTrafficLights\n' in source:return HEADER
    if re.search(r'(?m)^bool CTrafficLights::bGreenLightsCheat;',source):
        if OBSERVER:OBSERVER(source)
        return HOST
    updated=source.replace('CTrafficLights::','TrafficLights_')
    if updated==source:return source
    if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)'):return updated
    return '//+ rouz edit (ChatGPT)\n'+updated.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Forward historical owners through the separately verified weapon metadata record conversion
    return weapons.calls(traffic_calls(source))

def project(source):
    # Register the complete C11 controller with a unique basename beside typed native boundaries
    for tag,old,new in [('ClCompile','TrafficLights.cpp','TrafficControl.c'),('ClInclude','TrafficLights.h','TrafficLights_backend.h')]:
        entry='<'+tag+' Include="..\\src\\control\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\control\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\control\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\control\\'+re.escape(old)+r'">\n\s*<Filter>control</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return weapons.project(source)

def verify_legacy(source,transform,root):
    # Prove the full relocated controller and all native field/action bindings from earlier immutable sources
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/control/TrafficLights.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/control/TrafficControl.c').read_text())

def verify_dependency(saved,current):
    # Preserve byte guards unless the complete dependency follows this exact traffic-light conversion
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original)
    assert expected!=original and expected==current.read_text(),str(current)
