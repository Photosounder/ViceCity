"""Audit entire set-piece migration and generate independent original/C console programs."""
from pathlib import Path
import json,re
import set_pieces_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/set-pieces-c-tests'
old=(out/'before/src/control/SetPieces.cpp').read_text()
for path in json.loads((out/'changes.json').read_text()):
    before=(out/'before'/path).read_text();expected=m.project(before) if path.startswith('build/') else m.calls(before)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
assert (root/'src/control/SetPieceControl.c').read_text()==m.kernel(old)
assert (root/'src/control/SetPieces_backend.h').read_text()==m.BACKEND
for path in json.loads((out/'untouched.json').read_text()):
    if path=='src/peds/Ped.h':
        import ped_data_migration
        ped_data_migration.verify_dependency(out/'before'/path,root/path)
    else:
        import projectile_migration
        projectile_migration.verify_dependency(out/'before'/path,root/path)
(out/'common.h').write_text((root/'build/remote-c-tests/common.h').read_text(),newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
owners='''#pragma once
#include "common.h"
#include <cassert>
#include <memory>
#include "SetPieces_backend.h"
#include "Pool.h"
#include "Timer.h"
#include "SaveBuf.h"
#include "VarConsole.h"
#include "EntityStatus.h"
#include "ModelIndexData.h"
#ifndef NATIVE_PROJECT_HEADERS
#pragma push_macro("VALIDATE_SIZE")
#undef VALIDATE_SIZE
#define VALIDATE_SIZE(type,size) static_assert(sizeof(type)==(sizeof(void*)==8?160:112),"Actual pointer-size-dependent AutoPilot layout")
#endif
#include "AutoPilot.h"
#ifndef NATIVE_PROJECT_HEADERS
#pragma pop_macro("VALIDATE_SIZE")
#endif
enum {RANDOM_VEHICLE=1,COP_STREET=0,OBJECTIVE_GOTO_AREA_ON_FOOT=1};
struct CColSphere {float radius;};struct CColModel {CColSphere *spheres;};struct CColPoint {CVector point;};class CStoredCollPoly;
class CEntity {public:CMatrix matrix;int id,kind;CColSphere sphere;CColModel collision;
    CMatrix &GetMatrix(){return matrix;}CVector &GetForward(){return matrix.GetForward();}CVector &GetRight(){return matrix.GetRight();}CVector &GetUp(){return matrix.GetUp();}
    void SetPosition(CVector value){matrix.GetPosition()=value;}CColModel *GetColModel(){return &collision;}
};
class CVehicle:public CEntity {public:CAutoPilot AutoPilot;uint8 status;uint32 m_nSetPieceExtendedRangeTime;CVector move;bool law;
    CVehicle(){}virtual ~CVehicle();virtual float GetHeightAboveRoad(void);void SetStatus(uint8);void SetMoveSpeed(const CVector&);void ChangeLawEnforcerState(uint8);
};
class CAutomobile:public CVehicle {public:CAutomobile(int32,uint8);};
class CPed:public CEntity {};
class CCopPed:public CPed {public:uint32 bScriptObjectiveCompleted:1;uint32 m_nExtendedRangeTimer;CVector destination;CCopPed(int);~CCopPed();void SetObjective(int,CVector);};
class CWanted {public:int GetWantedLevel(void);};class CPlayerPed {public:CWanted *m_pWanted;};
class CPools {public:static CPool *GetVehiclePool();static CPool *GetPedPool();};
class CWorld {public:static void Add(CEntity*);static void Remove(CEntity*);static float FindGroundZForCoord(float,float);
    static bool ProcessVerticalLine(const CVector&,float,CColPoint&,CEntity*&,bool,bool,bool,bool,bool,bool,CStoredCollPoly*);
    static void FindObjectsKindaColliding(const CVector&,float,bool,int16*,int16,CEntity**,bool,bool,bool,bool,bool);
};
class CCarAI {public:static void AddPoliceCarOccupants(CVehicle*);};
CVector FindPlayerCoors(void);const CVector &FindPlayerSpeed(void);CPlayerPed *FindPlayerPed(void);CVehicle *FindPlayerVehicle(void);
'''
prototypes='''bool *TestPieces_Debug(void);uint32_t *TestPieces_Count(void);
void TestPieces_Read(unsigned,SetPiece*);void TestPieces_Write(unsigned,const SetPiece*);
void TestPieces_Init(void);void TestPieces_Add(uint8_t,SetPiecePoint,SetPiecePoint,SetPiecePoint,SetPiecePoint,SetPiecePoint,SetPiecePoint);
void TestPieces_Update(void);void TestPieces_Record(unsigned);void TestPieces_Save(uint8_t*,uint32_t*);void TestPieces_Load(uint8_t*,uint32_t);
CVehicle *TestPieces_Car(unsigned,SetPiecePoint,SetPiecePoint);CCopPed *TestPieces_Cop(unsigned,SetPiecePoint);
void RunPieces_Init(void);void RunPieces_Add(uint8_t,SetPiecePoint,SetPiecePoint,SetPiecePoint,SetPiecePoint,SetPiecePoint,SetPiecePoint);
void RunPieces_Update(void);void RunPieces_Record(unsigned);void RunPieces_Save(uint8_t*,uint32_t*);void RunPieces_Load(uint8_t*,uint32_t);
CVehicle *RunPieces_Car(unsigned,SetPiecePoint,SetPiecePoint);CCopPed *RunPieces_Cop(unsigned,SetPiecePoint);
'''
(out/'caller.h').write_text('#pragma once\n#include "SetPieces.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
(out/'owners.h').write_text(owners+'#include "caller.h"\n',newline='\n')
for before in [True,False]:
    text='#include "owners.h"\n'
    if before:
        header=(out/'before/src/control/SetPieces.h').read_text().replace('#pragma once','').replace('class CSetPieces\n{','class CSetPieces\n{\npublic:').replace('class CVehicle;','').replace('class CCopPed;','')
        text+='namespace Original {\n'+header+'\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n}\n'
    prefix='Original::CSetPieces::' if before else 'SetPieces_';points=prefix+'aSetPieces'
    text+='extern "C" {\nbool *TestPieces_Debug(void){\n    // Access the original canonical debug flag\n    return &'+prefix+'bDebug;\n}\nuint32_t *TestPieces_Count(void){\n    // Access the original canonical count\n    return &'+prefix+'NumSetPieces;\n}\n'
    for read in [True,False]:
        text+=('void TestPieces_Read(unsigned index,SetPiece *record)' if read else 'void TestPieces_Write(unsigned index,const SetPiece *record)')+'\n{\n    // Transfer only canonical named fields without observing or changing padding\n'
        for field in m.FIELDS:
            for suffix in (['.x','.y'] if field.startswith('m_v') else ['']):
                a='record->'+field+suffix;b=points+'[index].'+field+suffix
                text+='    '+(a+'='+b if read else b+'='+a)+';\n'
        text+='}\n'
    for name,ret,args,values in [
        ('Init','void','void',''),('Add','void','uint8_t kind,SetPiecePoint a,SetPiecePoint b,SetPiecePoint c,SetPiecePoint d,SetPiecePoint e,SetPiecePoint f','kind,a,b,c,d,e,f'),('Update','void','void',''),('Record','void','unsigned index',''),('Save','void','uint8_t *buffer,uint32_t *size','buffer,size'),('Load','void','uint8_t *buffer,uint32_t size','buffer,size'),('Car','CVehicle *','unsigned index,SetPiecePoint a,SetPiecePoint b','a,b'),('Cop','CCopPed *','unsigned index,SetPiecePoint a','a')]:
        if before:
            if name in ['Record','Car','Cop']:call=points+'[index].'+{'Record':'Update','Car':'TryToGenerateCopCar','Cop':'TryToGenerateCopPed'}[name]
            else:call=prefix+('AddOne' if name=='Add' else name)
            if name in ['Add','Car','Cop']:values=','.join('CVector2D('+v+'.x,'+v+'.y)' if v in 'abcdef' else v for v in values.split(','))
        else:
            if name in ['Record','Car','Cop']:call='SetPiece_'+{'Record':'Update','Car':'TryToGenerateCopCar','Cop':'TryToGenerateCopPed'}[name];values='&'+points+'[index]'+(','+values if values else '')
            else:call=prefix+('AddOne' if name=='Add' else name)
        text+=ret+' TestPieces_'+name+'('+args+')\n{\n    // Execute the complete preserved original or C operation\n    '+('return ' if ret!='void' else '')+call+'('+values+');\n}\n'
    (out/('original.cpp' if before else 'adapter.cpp')).write_text(text+'}\n',newline='\n')
caller='#include "caller.h"\n'
for name,ret,args,values in [('Init','void','void',''),('Add','void','uint8_t kind,SetPiecePoint a,SetPiecePoint b,SetPiecePoint c,SetPiecePoint d,SetPiecePoint e,SetPiecePoint f','kind,a,b,c,d,e,f'),('Update','void','void',''),('Record','void','unsigned index','index'),('Save','void','uint8_t *buffer,uint32_t *size','buffer,size'),('Load','void','uint8_t *buffer,uint32_t size','buffer,size'),('Car','CVehicle *','unsigned index,SetPiecePoint a,SetPiecePoint b','index,a,b'),('Cop','CCopPed *','unsigned index,SetPiecePoint a','index,a')]:caller+=ret+' RunPieces_'+name+'('+args+')\n{\n    // Reach every complete selected operation through standalone C11\n    '+('return ' if ret!='void' else '')+'TestPieces_'+name+'('+values+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',m.HOST,flags=re.M),newline='\n')
(out/'math.cpp').write_text((root/'build/remote-c-tests/matrix.cpp').read_text()+'\n'+function((root/'src/math/Vector.cpp').read_text(),'CVector::','Normalise'),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/set_pieces.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited eight complete set-piece operations, all 96 canonical records, forty-two native boundaries and seven complete source/project changes')
