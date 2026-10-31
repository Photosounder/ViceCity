"""Audit complete remote-control sources and generate original/C console comparisons."""
from pathlib import Path
import json,re
import remote_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/remote-c-tests'
for path in json.loads((out/'changes.json').read_text()):
    source=(out/'before'/path).read_text();expected=m.project(source) if path.startswith('build/') else m.calls(source)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/control/Remote.cpp').read_text()
assert (root/'src/control/RemoteControl.c').read_text()==m.kernel(old)
assert (root/'src/control/Remote_backend.h').read_text()==m.BACKEND
for path in json.loads((out/'untouched.json').read_text()):
    import fire_migration
    fire_migration.verify_dependency(out/'before'/path,root/path)
(out/'common.h').write_text((root/'build/water-creatures-c-tests/common.h').read_text(),newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
owners='''#pragma once
#include "common.h"
#include <cassert>
#include <memory>
#include "Remote_backend.h"
#include "Pool.h"
#ifndef NATIVE_PROJECT_HEADERS
#pragma push_macro("VALIDATE_SIZE")
#undef VALIDATE_SIZE
#define VALIDATE_SIZE(type,size) static_assert(sizeof(type)==(sizeof(void*)==8?160:112),"Actual pointer-size-dependent AutoPilot layout")
#endif
#include "AutoPilot.h"
#ifndef NATIVE_PROJECT_HEADERS
#pragma pop_macro("VALIDATE_SIZE")
#endif
#include "VehicleAppearance.h"
#include "EntityStatus.h"
#include "CameraModes.h"
enum {RANDOM_VEHICLE=1,MISSION_VEHICLE=2};
enum {INTERPOLATION=1,CAMCONTROL_SCRIPT=1};
class CEntity {
public:
    CMatrix matrix;int id;uint8 status;
    CMatrix &GetMatrix(){return matrix;}
    void SetPosition(float x,float y,float z){matrix.GetPosition().x=x;matrix.GetPosition().y=y;matrix.GetPosition().z=z;}
    void SetStatus(uint8 value);
    float GetDistanceFromCentreOfMassToBaseOfModel(void);
    void RegisterReference(CEntity **slot);
};
class CVehicle:public CEntity {
public:
    CAutoPilot AutoPilot;uint8 VehicleCreatedBy;uint8 bIsLocked:1;uint8 bEngineOn:1;float baseDistance;
    eVehicleAppearance GetVehicleAppearance(void);
};
class CAutomobile:public CVehicle {public:int32 model;CAutomobile(int32 model,uint8 createdBy);};
class CPlayerInfo {public:CVehicle *m_pRemoteVehicle;uint32 m_nTimeLostRemoteCar;bool m_bInRemoteMode,field_D5,field_D6;};
class CWorld {public:static CPlayerInfo Players[NUMPLAYERS];static uint8 PlayerInFocus;static float FindGroundZFor3DCoord(float,float,float,bool*);static void Add(CEntity*);};
class CPools {public:static CPool *GetVehiclePool(void);};
class CCarCtrl {public:static int32 NumMissionCars,NumRandomCars;static void JoinCarWithRoadSystem(CVehicle*);};
class CCam {public:enum {MODE_CAM_ON_A_STRING=CAMERA_MODE_CAM_ON_A_STRING,MODE_BEHINDCAR=CAMERA_MODE_BEHINDCAR};};
class CCamera {public:void TakeControl(CEntity*,int16,int16,int32);void SetZoomValueCamStringScript(int16);};extern CCamera TheCamera;
CVehicle *FindPlayerVehicle(void);
extern "C" void TestRemote_Give(float,float,float,float,uint16_t);
extern "C" void TestRemote_Take(bool);
extern "C" void TestRemote_RunGive(float,float,float,float,uint16_t);
extern "C" void TestRemote_RunTake(bool);
'''
(out/'owners.h').write_text(owners,newline='\n')
original='#include "owners.h"\nnamespace Original {\n'+(out/'before/src/control/Remote.h').read_text().replace('#pragma once','')+'\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n}\n'
wrappers='''extern "C" void TestRemote_Give(float x,float y,float z,float rotation,uint16_t model)
{
    // Enter the entire selected remote-car creation operation
    PREFIXGivePlayerRemoteControlledCar(x,y,z,rotation,model);
}
extern "C" void TestRemote_Take(bool blowUp)
{
    // Enter the entire selected remote-car release operation
    PREFIXTakeRemoteControlledCarFromPlayer(blowUp);
}
'''
(out/'original.cpp').write_text(original+wrappers.replace('PREFIX','Original::CRemote::'),newline='\n')
(out/'adapter.cpp').write_text('#include "owners.h"\n'+wrappers.replace('PREFIX','Remote_'),newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',m.HOST,flags=re.M),newline='\n')
(out/'caller.c').write_text('''#include "Remote.h"
void TestRemote_Give(float,float,float,float,uint16_t);void TestRemote_Take(bool);
void TestRemote_RunGive(float x,float y,float z,float rotation,uint16_t model)
{
    // Reach the complete operation through a separate C11 caller
    TestRemote_Give(x,y,z,rotation,model);
}
void TestRemote_RunTake(bool blowUp)
{
    // Preserve both explicit boolean argument values at the C boundary
    TestRemote_Take(blowUp);
}
''',newline='\n')
matrix=(root/'src/math/Matrix.cpp').read_text()
(out/'matrix.cpp').write_text('#include "owners.h"\n'+function(matrix,'CMatrix::','SetRotateZOnly')+'''
CMatrix::CMatrix(void){
    // Retain the original unattached native matrix constructor state
    m_attachment=nil;m_hasRwMatrix=false;
}
CMatrix::~CMatrix(void){
    // Reject unexpected ownership of an external rendering matrix in this console fixture
    if(m_hasRwMatrix || m_attachment)abort();
}
''',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/remote.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited both complete remote-car operations, thirty typed boundaries, four callers and eight full source/project changes')
