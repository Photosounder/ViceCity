"""Move the complete remote-car controller into C11 with typed native access."""
import re
from audio_controls_migration import function
import set_pieces_migration as pieces
OBSERVER=None
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void Remote_GivePlayerRemoteControlledCar(float x,float y,float z,float rotation,uint16_t model);
void Remote_TakeRemoteControlledCarFromPlayer(bool blowUp);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BOUNDARIES=[
('RemoteVehicle *','Allocate','uint16_t model','CAutomobile *car=(CAutomobile*)((CVehicle*)CPool_New(CPools::GetVehiclePool()));\n    assert(car);\n    std::allocator<CAutomobile>().construct(car,model,MISSION_VEHICLE);\n    return car;'),
('float','BaseDistance','RemoteVehicle *car','return car->GetDistanceFromCentreOfMassToBaseOfModel();'),
('float','GroundZ','float x,float y,float z,bool *found','return CWorld::FindGroundZFor3DCoord(x,y,z,found);'),
('void','Rotate','RemoteVehicle *car,float rotation','car->GetMatrix().SetRotateZOnly(rotation);'),
('void','Position','RemoteVehicle *car,float x,float y,float z','car->SetPosition(x,y,z);'),
('void','Status','RemoteVehicle *car,uint8_t status','car->SetStatus(status);'),
('void','Locked','RemoteVehicle *car,bool value','car->bIsLocked=value;'),
('void','JoinRoad','RemoteVehicle *car','CCarCtrl::JoinCarWithRoadSystem(car);'),
('uint8_t *','CarMission','RemoteVehicle *car','return &car->AutoPilot.m_nCarMission;'),
('uint8_t *','TempAction','RemoteVehicle *car','return &car->AutoPilot.m_nTempAction;'),
('uint8_t *','DrivingStyle','RemoteVehicle *car','return &car->AutoPilot.m_nDrivingStyle;'),
('uint8_t *','CruiseSpeed','RemoteVehicle *car','return &car->AutoPilot.m_nCruiseSpeed;'),
('float *','MaxTrafficSpeed','RemoteVehicle *car','return &car->AutoPilot.m_fMaxTrafficSpeed;'),
('int8_t *','NextLane','RemoteVehicle *car','return &car->AutoPilot.m_nNextLane;'),
('int8_t *','CurrentLane','RemoteVehicle *car','return &car->AutoPilot.m_nCurrentLane;'),
('void','EngineOn','RemoteVehicle *car,bool value','car->bEngineOn=value;'),
('void','AddWorld','RemoteVehicle *car','CWorld::Add(car);'),
('RemoteVehicle *','PlayerVehicle','void','return FindPlayerVehicle();'),
('RemoteVehicle **','RemoteSlot','void','return &CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle;'),
('void','RegisterReference','RemoteVehicle *car,RemoteVehicle **slot','car->RegisterReference((CEntity**)slot);'),
('int32_t','Appearance','RemoteVehicle *car','return car->GetVehicleAppearance();'),
('void','TakeCamera','RemoteVehicle *car,int16_t mode','TheCamera.TakeControl(car,mode,INTERPOLATION,CAMCONTROL_SCRIPT);'),
('void','Zoom','int16_t value','TheCamera.SetZoomValueCamStringScript(value);'),
('uint8_t *','CreatedBy','RemoteVehicle *car','return &car->VehicleCreatedBy;'),
('int32_t *','MissionCars','void','return &CCarCtrl::NumMissionCars;'),
('int32_t *','RandomCars','void','return &CCarCtrl::NumRandomCars;'),
('uint32_t *','LostTime','void','return &CWorld::Players[CWorld::PlayerInFocus].m_nTimeLostRemoteCar;'),
('bool *','RemoteMode','void','return &CWorld::Players[CWorld::PlayerInFocus].m_bInRemoteMode;'),
('bool *','BlowUp','void','return &CWorld::Players[CWorld::PlayerInFocus].field_D5;'),
('bool *','LostFlag','void','return &CWorld::Players[CWorld::PlayerInFocus].field_D6;')]
CONSTANTS={'REMOTE_RANDOM_VEHICLE':'RANDOM_VEHICLE','REMOTE_MISSION_VEHICLE':'MISSION_VEHICLE','REMOTE_MISSION_NONE':'MISSION_NONE','REMOTE_TEMPACT_NONE':'TEMPACT_NONE','REMOTE_DRIVINGSTYLE_STOP_FOR_CARS':'DRIVINGSTYLE_STOP_FOR_CARS'}
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "Remote.h"
#ifdef __cplusplus
class CVehicle;
typedef CVehicle RemoteVehicle;
extern "C" {
#else
typedef struct CVehicle RemoteVehicle;
#endif
enum {REMOTE_RANDOM_VEHICLE=1,REMOTE_MISSION_VEHICLE=2,REMOTE_MISSION_NONE=0,REMOTE_TEMPACT_NONE=0,REMOTE_DRIVINGSTYLE_STOP_FOR_CARS=0};
'''+''.join(ret+' Remote_Backend'+name+'('+args+');\n' for ret,name,args,body in BOUNDARIES)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Automobile.h"
#include "CarCtrl.h"
#include "Camera.h"
#include "World.h"
#include "PlayerInfo.h"
#include "Pools.h"
#include "Remote_backend.h"
'''+''.join('static_assert(int('+name+')==int('+native+'),"Original remote-car constant");\n' for name,native in CONSTANTS.items())+''.join(ret+' Remote_Backend'+name+'('+args+')\n{\n    // Access the original native operation or named field with its actual type\n    '+body+'\n}\n' for ret,name,args,body in BOUNDARIES)+'//- rouz edit (ChatGPT)\n'

def kernel(source):
    # Transform both complete original operations without replacing their control flow
    text='\n\n'.join(function(source,'CRemote::',name) for name in ['GivePlayerRemoteControlledCar','TakeRemoteControlledCarFromPlayer'])
    text=text.replace('CRemote::','Remote_').replace('uint16 model','uint16_t model')
    first=text.index('{');end=text.index('\tbool found;',first)
    text=text[:first+1]+'\n    // Construct the original native car before applying the complete C control sequence\n    RemoteVehicle *car=Remote_BackendAllocate(model);\n'+text[end:]
    text=text.replace('car->GetDistanceFromCentreOfMassToBaseOfModel()','Remote_BackendBaseDistance(car)').replace('CWorld::FindGroundZFor3DCoord(','Remote_BackendGroundZ(')
    text=text.replace('car->GetMatrix().SetRotateZOnly(rot)','Remote_BackendRotate(car,rot)').replace('car->SetPosition(x, y, z)','Remote_BackendPosition(car,x,y,z)').replace('car->SetStatus(STATUS_PLAYER_REMOTE)','Remote_BackendStatus(car,STATUS_PLAYER_REMOTE)').replace('car->bIsLocked = true','Remote_BackendLocked(car,true)').replace('CCarCtrl::JoinCarWithRoadSystem(car)','Remote_BackendJoinRoad(car)')
    for field,name in [('m_nCarMission','CarMission'),('m_nTempAction','TempAction'),('m_nDrivingStyle','DrivingStyle'),('m_nCruiseSpeed','CruiseSpeed'),('m_fMaxTrafficSpeed','MaxTrafficSpeed'),('m_nNextLane','NextLane'),('m_nCurrentLane','CurrentLane')]:text=text.replace('car->AutoPilot.'+field,'*Remote_Backend'+name+'(car)')
    text=text.replace('car->bEngineOn = true','Remote_BackendEngineOn(car,true)').replace('CWorld::Add(car)','Remote_BackendAddWorld(car)').replace('FindPlayerVehicle() != nil','Remote_BackendPlayerVehicle() != NULL').replace('FindPlayerVehicle()->SetStatus(STATUS_PLAYER_DISABLED)','Remote_BackendStatus(Remote_BackendPlayerVehicle(),STATUS_PLAYER_DISABLED)')
    slot='CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle'
    text=text.replace(slot+'->RegisterReference((CEntity**)&'+slot+')','Remote_BackendRegisterReference(*Remote_BackendRemoteSlot(),Remote_BackendRemoteSlot())')
    text=text.replace(slot+'->VehicleCreatedBy','*Remote_BackendCreatedBy(*Remote_BackendRemoteSlot())').replace(slot+'->bIsLocked = false','Remote_BackendLocked(*Remote_BackendRemoteSlot(),false)').replace(slot,'*Remote_BackendRemoteSlot()')
    text=text.replace('car->GetVehicleAppearance()','Remote_BackendAppearance(car)').replace('TheCamera.TakeControl(car, CCam::MODE_CAM_ON_A_STRING, INTERPOLATION, CAMCONTROL_SCRIPT)','Remote_BackendTakeCamera(car,CAMERA_MODE_CAM_ON_A_STRING)').replace('TheCamera.TakeControl(car, CCam::MODE_BEHINDCAR, INTERPOLATION, CAMCONTROL_SCRIPT)','Remote_BackendTakeCamera(car,CAMERA_MODE_BEHINDCAR)').replace('TheCamera.SetZoomValueCamStringScript(0)','Remote_BackendZoom(0)')
    for name,native in CONSTANTS.items():text=re.sub(r'\b'+native+r'\b',name,text)
    text=text.replace('CCarCtrl::NumMissionCars--','(*Remote_BackendMissionCars())--').replace('CCarCtrl::NumRandomCars++','(*Remote_BackendRandomCars())++')
    for field,name in [('m_nTimeLostRemoteCar','LostTime'),('m_bInRemoteMode','RemoteMode'),('field_D5','BlowUp'),('field_D6','LostFlag')]:text=text.replace('CWorld::Players[CWorld::PlayerInFocus].'+field,'*Remote_Backend'+name+'()')
    take=text.index('Remote_TakeRemoteControlledCarFromPlayer');brace=text.index('{',take);text=text[:brace+1]+'\n    // Retain the original ownership transition and every live player-state write'+text[brace+1:]
    code=re.sub(r'//[^\n]*|"(?:\\.|[^"\\])*"','',text);assert not re.search(r'::|->|\b(?:class|CRemote|CAutomobile|std|nil)\b',code),code
    return '//+ rouz edit (ChatGPT)\n#include "Remote_backend.h"\n#include "Timer.h"\n#include "EntityStatus.h"\n#include "VehicleAppearance.h"\n#include "CameraModes.h"\n#include <stddef.h>\n\n'+text+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Remove the static-only class and make the former default argument explicit at each caller
    if 'CRemote::GivePlayerRemoteControlledCar(float' in source:
        if OBSERVER:OBSERVER(source)
        return HOST
    if 'class CRemote\n' in source:return HEADER
    lines=[]
    for line in source.splitlines(keepends=True):
        changed=line.replace('CRemote::','Remote_').replace('Remote_TakeRemoteControlledCarFromPlayer()','Remote_TakeRemoteControlledCarFromPlayer(true)')
        if changed!=line:changed=changed.rstrip('\n')+' // rouz edit (ChatGPT)\n'
        lines.append(changed)
    return pieces.calls(''.join(lines))

def project(source):
    # Register a uniquely named C11 controller and its typed native boundary declarations
    entry='<ClCompile Include="..\\src\\control\\Remote.cpp" />'
    if '..\\src\\control\\RemoteControl.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\control\\RemoteControl.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\control\\Remote.h" />'
    if '..\\src\\control\\Remote_backend.h"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\control\\Remote_backend.h" />')
    return pieces.project(source)

def verify_legacy(source,transform,root):
    # Verify every relocated statement when an earlier complete-file audit reaches the former class
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/control/Remote.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/control/RemoteControl.c').read_text())
