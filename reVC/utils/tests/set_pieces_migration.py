"""Move the complete set-piece records, controller and spawn decisions into C11."""
import re
from audio_controls_migration import function
import weather_migration as weather
OBSERVER=None
FIELDS=['m_nType','m_nLastTimeCreated','m_vTriggerInf','m_vTriggerSup','m_vSpawn1','m_vSpawn2','m_vTarget1','m_vTarget2']
CONTROLLER=['Init','AddOne','Update','Save','Load'];RECORD=['Update','TryToGenerateCopCar','TryToGenerateCopPed']
ENUM='''enum eSetPieceType {SETPIECE_NONE=0,SETPIECE_TWOCOPCARSINALLEY,SETPIECE_CARBLOCKINGPLAYERFROMSIDE,SETPIECE_CARRAMMINGPLAYERFROMSIDE,SETPIECE_CREATECOPPERONFOOT,SETPIECE_CREATETWOCOPPERSONFOOT,SETPIECE_TWOCARSBLOCKINGPLAYERFROMSIDE,SETPIECE_TWOCARSRAMMINGPLAYERFROMSIDE};
'''
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CVehicle;class CCopPed;
extern "C" {
#else
typedef struct CVehicle CVehicle;typedef struct CCopPed CCopPed;
#endif
'''+ENUM+'''typedef struct SetPiecePoint {float x,y;} SetPiecePoint;
typedef struct SetPieceVector {float x,y,z;} SetPieceVector;
typedef struct SetPiece {
    uint8_t m_nType;
    uint32_t m_nLastTimeCreated;
    SetPiecePoint m_vTriggerInf,m_vTriggerSup,m_vSpawn1,m_vSpawn2,m_vTarget1,m_vTarget2;
} SetPiece;
extern bool SetPieces_bDebug;
extern uint32_t SetPieces_NumSetPieces;
extern SetPiece SetPieces_aSetPieces[NUM_SETPIECES];
void SetPieces_Init(void);
void SetPieces_AddOne(uint8_t type,SetPiecePoint triggerInf,SetPiecePoint triggerSup,SetPiecePoint spawn1,SetPiecePoint target1,SetPiecePoint spawn2,SetPiecePoint target2);
void SetPieces_Update(void);
void SetPieces_Save(uint8_t *buffer,uint32_t *size);
void SetPieces_Load(uint8_t *buffer,uint32_t size);
void SetPiece_Update(SetPiece *record);
CVehicle *SetPiece_TryToGenerateCopCar(SetPiece *record,SetPiecePoint spawn,SetPiecePoint target);
CCopPed *SetPiece_TryToGenerateCopPed(SetPiece *record,SetPiecePoint spawn);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
CONSTANTS={'DRIVINGSTYLE_SLOW_DOWN_FOR_CARS':1,'DRIVINGSTYLE_PLOUGH_THROUGH':3,'DRIVINGSTYLE_AVOID_CARS':2,'MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_1':24,'MISSION_BLOCKPLAYER_FORWARDANDBACK':26,'MISSION_RAMCAR_CLOSE':16,'TEMPACT_GOFORWARD':8}
BOUNDARIES=[
('void','RegisterDebug','bool *flag','VarConsole_AddBool(&VarConsole,"Show set pieces",flag,true);'),
('SetPieceVector','PlayerCoors','void','CVector value=FindPlayerCoors();return SetPieceVector{value.x,value.y,value.z};'),
('SetPiecePoint','PlayerSpeed','void','CVector2D value=FindPlayerSpeed();return SetPiecePoint{value.x,value.y};'),
('int32_t','Wanted','void','return FindPlayerPed()->m_pWanted->GetWantedLevel();'),
('CVehicle *','PlayerVehicle','void','return FindPlayerVehicle();'),
('void','VehicleStatus','CVehicle *car,uint8_t status','car->SetStatus(status);'),
('void','Occupants','CVehicle *car','CCarAI::AddPoliceCarOccupants(car);'),
('void','RemoveVehicle','CVehicle *car','CWorld::Remove(car);'),
('void','RemoveCop','CCopPed *cop','CWorld::Remove(cop);'),
('void','DestroyVehicle','CVehicle *car','car->~CVehicle();CPool_Delete(CPools::GetVehiclePool(),car);'),
('void','DestroyCop','CCopPed *cop','cop->~CCopPed();CPool_Delete(CPools::GetPedPool(),cop);'),
('uint8_t *','CruiseSpeed','CVehicle *car','return &car->AutoPilot.m_nCruiseSpeed;'),
('float *','MaxSpeed','CVehicle *car','return &car->AutoPilot.m_fMaxTrafficSpeed;'),
('uint8_t *','DrivingStyle','CVehicle *car','return &car->AutoPilot.m_nDrivingStyle;'),
('uint8_t *','CarMission','CVehicle *car','return &car->AutoPilot.m_nCarMission;'),
('uint8_t *','TempAction','CVehicle *car','return &car->AutoPilot.m_nTempAction;'),
('uint32_t *','TempTime','CVehicle *car','return &car->AutoPilot.m_nTimeTempAction;'),
('uint32_t *','VehicleRangeTime','CVehicle *car','return &car->m_nSetPieceExtendedRangeTime;'),
('float *','DestinationX','CVehicle *car','return &car->AutoPilot.m_vecDestinationCoors.x;'),
('float *','DestinationY','CVehicle *car','return &car->AutoPilot.m_vecDestinationCoors.y;'),
('float *','DestinationZ','CVehicle *car','return &car->AutoPilot.m_vecDestinationCoors.z;'),
('SetPieceVector','Forward','CVehicle *car','const CVector &value=car->GetForward();return SetPieceVector{value.x,value.y,value.z};'),
('void','MoveSpeed','CVehicle *car,SetPieceVector value','car->SetMoveSpeed(CVector(value.x,value.y,value.z));'),
('float','GroundZ','float x,float y','return CWorld::FindGroundZForCoord(x,y);'),
('void','ObjectiveCompleted','CCopPed *cop,bool value','cop->bScriptObjectiveCompleted=value;'),
('void','Objective','CCopPed *cop,SetPieceVector value','cop->SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT,CVector(value.x,value.y,value.z));'),
('uint32_t *','CopRangeTime','CCopPed *cop','return &cop->m_nExtendedRangeTimer;'),
('CVehicle *','ConstructVehicle','void','CVehicle *car=(CVehicle*)CPool_New(CPools::GetVehiclePool());assert(car);std::allocator<CAutomobile>().construct((CAutomobile*)car,MI_POLICE,RANDOM_VEHICLE);return car;'),
('CCopPed *','ConstructCop','void','CCopPed *cop=(CCopPed*)((CPed*)CPool_New(CPools::GetPedPool()));assert(cop);std::allocator<CCopPed>().construct(cop,COP_STREET);return cop;'),
('bool','VerticalLine','SetPieceVector value,float *height','CVector position(value.x,value.y,value.z);CColPoint point;CEntity *entity;bool hit=CWorld::ProcessVerticalLine(position,-1000.0f,point,entity,true,false,false,false,true,false,nil);\n    if(hit){\n        // Copy only the original valid hit-height output before the C controller decides placement\n        *height=point.point.z;\n    }\n    return hit;'),
('float','HeightAboveRoad','CVehicle *car','return car->GetHeightAboveRoad();'),
('void','SetForward','CVehicle *car,SetPieceVector value','car->GetForward()=CVector(value.x,value.y,value.z);'),
('void','SetRight','CVehicle *car,SetPieceVector value','car->GetRight()=CVector(value.x,value.y,value.z);'),
('void','SetUp','CVehicle *car,SetPieceVector value','car->GetUp()=CVector(value.x,value.y,value.z);'),
('void','VehiclePosition','CVehicle *car,SetPieceVector value','car->SetPosition(CVector(value.x,value.y,value.z));'),
('void','CopPosition','CCopPed *cop,SetPieceVector value','cop->SetPosition(CVector(value.x,value.y,value.z));'),
('float','VehicleRadius','CVehicle *car','return car->GetColModel()->spheres->radius;'),
('float','CopRadius','CCopPed *cop','return cop->GetColModel()->spheres->radius;'),
('void','Colliding','SetPieceVector value,float radius,int16_t *total','CVector position(value.x,value.y,value.z);CWorld::FindObjectsKindaColliding(position,radius,false,total,16,nil,false,true,true,false,false);'),
('void','LawEnforcer','CVehicle *car,uint8_t enabled','car->ChangeLawEnforcerState(enabled);'),
('void','AddVehicle','CVehicle *car','CWorld::Add(car);'),
('void','AddCop','CCopPed *cop','CWorld::Add(cop);')]
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "SetPieces.h"
enum {'''+','.join('SETPIECE_'+name+'='+str(value) for name,value in CONSTANTS.items())+'''};
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(('#ifndef MASTER\n' if name=='RegisterDebug' else '')+ret+' SetPieces_Backend'+name+'('+args+');\n'+('#endif\n' if name=='RegisterDebug' else '') for ret,name,args,body in BOUNDARIES)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "SetPieces_backend.h"
#include "Automobile.h"
#include "CarAI.h"
#include "CopPed.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "World.h"
#include "Pools.h"
#include "VarConsole.h"
'''+''.join('static_assert(int(SETPIECE_'+name+')==int('+name+'),"Original set-piece constant");\n' for name in CONSTANTS)+''.join(('#ifndef MASTER\n' if name=='RegisterDebug' else '')+ret+' SetPieces_Backend'+name+'('+args+')\n{\n    // Access the original typed native operation or canonical named field\n    '+body+'\n}\n'+('#endif\n' if name=='RegisterDebug' else '') for ret,name,args,body in BOUNDARIES)+'//- rouz edit (ChatGPT)\n'

def kernel(source):
    # Transform all eight complete original methods and preserve every branch and ordered write
    chunks=[]
    for name in CONTROLLER:
        text=function(source,'CSetPieces::',name).replace('CSetPieces::','SetPieces_').replace('CVector2D','SetPiecePoint')
        for field in ['bDebug','NumSetPieces','aSetPieces']:text=re.sub(r'\b'+field+r'\b','SetPieces_'+field,text)
        text=text.replace('VarConsole_AddBool(&VarConsole,"Show set pieces", &SetPieces_bDebug, true)','SetPieces_BackendRegisterDebug(&SetPieces_bDebug)')
        text=text.replace('SetPieces_aSetPieces[i].Update()','SetPiece_Update(&SetPieces_aSetPieces[i])').replace('CSetPiece','SetPiece')
        chunks.append(text)
    for name in RECORD:
        text=function(source,'CSetPiece::',name).replace('CSetPiece::','SetPiece_').replace('CVector2D','SetPiecePoint')
        text=text.replace('SetPiece_'+name+'(', 'SetPiece_'+name+'(SetPiece *record,',1).replace('(SetPiece *record,void)','(SetPiece *record)')
        for field in FIELDS:text=re.sub(r'\b'+field+r'\b','record->'+field,text)
        text=re.sub(r'(?<!_)\bTryToGenerateCop(Car|Ped)\(',r'SetPiece_TryToGenerateCop\1(record,',text)
        text=text.replace('CVector pos = FindPlayerCoors()','SetPieceVector pos = SetPieces_BackendPlayerCoors()').replace('FindPlayerPed()->m_pWanted->GetWantedLevel()','SetPieces_BackendWanted()').replace('FindPlayerVehicle()','SetPieces_BackendPlayerVehicle()')
        text=text.replace('DotProduct2D(FindPlayerSpeed(), (SetPiecePoint)FindPlayerCoors() - record->m_vSpawn1)','SetPiece_Dot2(SetPieces_BackendPlayerSpeed(),SetPiece_Offset(SetPieces_BackendPlayerCoors(),record->m_vSpawn1))')
        text=re.sub(r'(pVehicle[12])->SetStatus\(STATUS_PHYSICS\)',r'SetPieces_BackendVehicleStatus(\1,STATUS_PHYSICS)',text)
        for field,reader in [('m_nCruiseSpeed','CruiseSpeed'),('m_fMaxTrafficSpeed','MaxSpeed'),('m_nDrivingStyle','DrivingStyle'),('m_nCarMission','CarMission'),('m_nTempAction','TempAction'),('m_nTimeTempAction','TempTime'),('m_vecDestinationCoors.x','DestinationX'),('m_vecDestinationCoors.y','DestinationY'),('m_vecDestinationCoors.z','DestinationZ')]:text=re.sub(r'(pVehicle[12])->AutoPilot\.'+re.escape(field),r'*SetPieces_Backend'+reader+r'(\1)',text)
        text=re.sub(r'(pVehicle[12])->m_nSetPieceExtendedRangeTime',r'*SetPieces_BackendVehicleRangeTime(\1)',text)
        text=re.sub(r'(pVehicle[12])->SetMoveSpeed\(2.0f \* \1->GetForward\(\) / 3.0f\)',r'SetPieces_BackendMoveSpeed(\1,SetPiece_ScaledForward(SetPieces_BackendForward(\1)))',text)
        text=text.replace('CCarAI::AddPoliceCarOccupants(','SetPieces_BackendOccupants(').replace('CWorld::FindGroundZForCoord(','SetPieces_BackendGroundZ(')
        for car in ['pVehicle','pVehicle1']:
            text=text.replace('CWorld::Remove('+car+')','SetPieces_BackendRemoveVehicle('+car+')')
            text=re.sub(re.escape(car)+r'->~CVehicle\(\);[\s\S]*?CPool_Delete\(CPools::GetVehiclePool\(\), '+car+r'\);','SetPieces_BackendDestroyVehicle('+car+');',text)
        for cop in ['pCop','pCop2']:
            text=text.replace(cop+'->bScriptObjectiveCompleted = false','SetPieces_BackendObjectiveCompleted('+cop+',false)').replace(cop+'->SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT, CVector(record->m_vTarget'+('2' if cop=='pCop2' else '1')+'.x, record->m_vTarget'+('2' if cop=='pCop2' else '1')+'.y, z))','SetPieces_BackendObjective('+cop+', (SetPieceVector){record->m_vTarget'+('2' if cop=='pCop2' else '1')+'.x,record->m_vTarget'+('2' if cop=='pCop2' else '1')+'.y,z})')
            text=text.replace(cop+'->m_nExtendedRangeTimer','*SetPieces_BackendCopRangeTime('+cop+')')
            text=text.replace('CWorld::Remove('+cop+')','SetPieces_BackendRemoveCop('+cop+')')
            text=re.sub(re.escape(cop)+r'->~CCopPed\(\);[\s\S]*?CPool_Delete\(CPools::GetPedPool\(\), '+cop+r'\);','SetPieces_BackendDestroyCop('+cop+');',text)
        if name!='Update':
            start=text.index('{');end=text.index('\tCVector pos',start)
            kind='Vehicle' if name.endswith('Car') else 'Cop';var='pVehicle' if kind=='Vehicle' else 'pCop';typ='CVehicle' if kind=='Vehicle' else 'CCopPed'
            text=text[:start+1]+'\n    // Construct the original native spawn before applying all placement and acceptance decisions in C\n    '+typ+' *'+var+'=SetPieces_BackendConstruct'+kind+'();\n'+text[end:]
            text=text.replace('CVector pos(vSpawn.x, vSpawn.y, 1000.0f)','SetPieceVector pos={vSpawn.x,vSpawn.y,1000.0f}')
            text=text.replace('CColPoint point;\n\tCEntity* pEntity;','float hitHeight;')
            text=text.replace('CWorld::ProcessVerticalLine(pos, -1000.0f, point, pEntity, true, false, false, false, true, false, nil)','SetPieces_BackendVerticalLine(pos,&hitHeight)').replace('point.point.z','hitHeight').replace('pVehicle->GetHeightAboveRoad()','SetPieces_BackendHeightAboveRoad(pVehicle)')
            text=text.replace('CVector vDirection(vTarget.x - vSpawn.x, vTarget.y - vSpawn.y, 0.0f)','SetPieceVector vDirection={vTarget.x-vSpawn.x,vTarget.y-vSpawn.y,0.0f}').replace('vDirection.Normalise()','SetPiece_Normalise(&vDirection)')
            for vec,reader in [('Forward','SetForward'),('Right','SetRight'),('Up','SetUp')]:
                text=re.sub(r'pVehicle->Get'+vec+r'\(\) = CVector\(([^)]+)\)',r'SetPieces_Backend'+reader+r'(pVehicle,(SetPieceVector){\1})',text)
            text=text.replace(var+'->SetPosition(pos)','SetPieces_Backend'+kind+'Position('+var+',pos)')
            text=text.replace('CWorld::FindObjectsKindaColliding(pos, '+var+'->GetColModel()->spheres->radius, false, &total, 16, nil, false, true, true, false, false)','SetPieces_BackendColliding(pos,SetPieces_Backend'+kind+'Radius('+var+'),&total)')
            text=text.replace('pVehicle->ChangeLawEnforcerState(true)','SetPieces_BackendLawEnforcer(pVehicle,true)').replace('CWorld::Add('+var+')','SetPieces_BackendAdd'+kind+'('+var+')')
        chunks.append(text)
    text='\n\n'.join(chunks)
    for name in CONSTANTS:text=re.sub(r'\b'+name+r'\b','SETPIECE_'+name,text)
    for old,new in [('uint8','uint8_t'),('uint32','uint32_t'),('int16','int16_t'),('nil','NULL')]:text=re.sub(r'\b'+old+r'\b',new,text)
    text=re.sub(r'(?m)^\{\n', '{\n    // Preserve every original set-piece decision, live access and transfer order\n',text)
    code=re.sub(r'//[^\n]*|"(?:\\.|[^"\\])*"','',text)
    assert not re.search(r'::|\b(?:class|CSetPiece|CSetPieces|CVector|std|FindPlayerCoors|FindPlayerSpeed)\b',code),code
    return '''//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "SetPieces_backend.h"
#include "Timer.h"
#include "EntityStatus.h"
#include <stddef.h>
#include <math.h>
#undef assert
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "SaveBuf.h"
#define TIME_BETWEEN_SETPIECE_SPAWNS 20000
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
bool SetPieces_bDebug;
uint32_t SetPieces_NumSetPieces;
SetPiece SetPieces_aSetPieces[NUM_SETPIECES];
static SetPiecePoint SetPiece_Offset(SetPieceVector position,SetPiecePoint origin)
{
    // Copy the original two-dimensional component differences
    SetPiecePoint result={position.x-origin.x,position.y-origin.y};return result;
}
static float SetPiece_Dot2(SetPiecePoint a,SetPiecePoint b)
{
    // Preserve the original ordered two-dimensional products and addition
    return a.x*b.x+a.y*b.y;
}
static SetPieceVector SetPiece_ScaledForward(SetPieceVector value)
{
    // Keep the original multiply before divide for each native forward component
    SetPieceVector result={(2.0f*value.x)/3.0f,(2.0f*value.y)/3.0f,(2.0f*value.z)/3.0f};return result;
}
static void SetPiece_Normalise(SetPieceVector *value)
{
    // Preserve the complete original three-component normalization and zero-direction fallback
    float sq=value->x*value->x+value->y*value->y+value->z*value->z;
    if(sq>0.0f){
        // Multiply all original components by the reciprocal square root
        float invsqrt=1.0f/sqrtf(sq);value->x*=invsqrt;value->y*=invsqrt;value->z*=invsqrt;
    }else{
        // Retain the original fallback without resetting the other components
        value->x=1.0f;
    }
}
'''+text+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Remove both classes and replace all complete controller call sites
    if 'bool CSetPieces::bDebug;' in source:
        if OBSERVER:OBSERVER(source)
        return HOST
    if 'class CSetPiece\n' in source:return HEADER
    original=source
    source=source.replace('CSetPieces::','SetPieces_')
    source=re.sub(r'SetPieces_AddOne\([\s\S]*?\);',lambda match:re.sub(r'\*\(CVector2D\*\)&ScriptParams\[(\d+)\]',r'SetPiecePoint{*(float*)&ScriptParams[\1],*(float*)&ScriptParams[\1+1]}',match[0]),source)
    before=original.splitlines(keepends=True);after=source.splitlines(keepends=True);assert len(before)==len(after)
    return weather.calls(''.join(new.rstrip('\n')+' // rouz edit (ChatGPT)\n' if old!=new else new for old,new in zip(before,after)))

def project(source):
    # Register a uniquely named C11 record controller and its typed native backend header
    entry='<ClCompile Include="..\\src\\control\\SetPieces.cpp" />'
    if '..\\src\\control\\SetPieceControl.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\control\\SetPieceControl.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\control\\SetPieces.h" />'
    if '..\\src\\control\\SetPieces_backend.h"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\control\\SetPieces_backend.h" />')
    return weather.project(source)

def verify_legacy(source,transform,root):
    # Preserve complete earlier snapshots of every relocated controller and record operation
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/control/SetPieces.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/control/SetPieceControl.c').read_text())
