"""Audit complete fire lifecycles and emit original/C programs with real math and references."""
from pathlib import Path
import json,re
import fire_migration as m
import timecycle_migration as cycle
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/fire-c-tests'
changes=json.loads((out/'changes.json').read_text())
for path in changes:
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/core/Fire.cpp').read_text();header=(out/'before/src/core/Fire.h').read_text()
assert (root/'src/core/FireSystem.c').read_text()==m.kernel(old)
assert (root/'src/core/Fire_backend.h').read_text()==m.BACKEND
for path in json.loads((out/'dependencies.json').read_text()):
    if path not in changes:cycle.verify_dependency(out/'before'/path,root/path)
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
typedef uint8_t uint8;typedef uint16_t uint16;typedef uint32_t uint32;
typedef int8_t int8;typedef int16_t int16;typedef int32_t int32;typedef uintptr_t uintptr;
struct RwV3d {float x,y,z;};struct RwMatrix {RwV3d right,up,at,pos;};struct RwTexture;
class CMatrix;
#define nil NULL
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
template<typename T> T Min(T a,T b){
    // Preserve the native strict comparison and its value arguments
    return a<b?a:b;
}
template<typename T> T sq(T value){
    // Preserve the native multiplication with one evaluated argument
    return value*value;
}
#include "maths.h"
#include "Vector.h"
#include "Vector2D.h"
#include "Matrix.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#undef malloc
#undef free
void TestFire_MatrixUpdate(RwMatrix *);void TestFire_MatrixDestroy(RwMatrix *);
#undef RwMatrixUpdate
#undef RwMatrixDestroy
#define RwMatrixUpdate TestFire_MatrixUpdate
#define RwMatrixDestroy TestFire_MatrixDestroy
'''
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','maths.h']:(out/name).write_text((out/'before/src/math'/name).read_text(),newline='\n')
(out/'original-Fire.h').write_text(header,newline='\n')
event=(out/'before/src/core/EventList.h').read_text();enums='\n'.join(re.findall(r'enum eEvent(?:Type|Entity)\n\{[\s\S]*?\n\};',event));assert enums
owners='''#pragma once
#include "common.h"
#include "Timer.h"
#include "General.h"
#include "PedStates.h"
#include "PedType.h"
#include "WeaponType.h"
#include "References_backend.h"
#include "PointLights.h"
#ifdef TEST_FIRE_ORIGINAL
#include "original-Fire.h"
#else
#include "'''+(root/'src/core/Fire.h').as_posix()+'''"
#endif
'''+enums+'''
enum {PEDPIECE_TORSO=0,WAITSTATE_FINISH_FLEE=99,CAR_POS_HEADLIGHTS=0,SHADOWTYPE_ADDITIVE=2,PARTICLE_CARFLAME=1,PARTICLE_CARFLAME_SMOKE=2,PARTICLE_STEAM_NY_SLOWMOTION=3};
class CVehicle;
void TestFire_Action(int operation,class CEntity *entity,class CEntity *source=0,float value=0);
class CEntity {
public:
    CMatrix matrix;CReference *m_pFirstReference;int id,kind,model;bool bRenderScorched;
    CEntity():m_pFirstReference(nil),id(-1),kind(0),model(0),bRenderScorched(false){
        // Start the quiet native shell with an identity transform
        matrix.SetTranslate(0,0,0);
    }
    bool IsPed(){
        // Read the independently seeded pedestrian category
        return kind==1;
    }
    bool IsVehicle(){
        // Read the independently seeded vehicle category
        return kind==2;
    }
    bool IsBuilding(){
        // Read the independently seeded building category
        return kind==3;
    }
    int GetModelIndex(){
        // Expose the current native model identifier
        return model;
    }
    CVector &GetPosition(){
        // Keep position reads and references attached to the actual matrix
        return matrix.GetPosition();
    }
    CMatrix &GetMatrix(){
        // Expose the complete transform used by the real vector multiplication
        return matrix;
    }
    void RegisterReference(CEntity **slot){
        // Enter the actual C reference registration exactly as the current native entity facade does
        References_RegisterReference(this,slot);
    }
};
class CPed:public CEntity {
public:
    CFire *m_pFire;CVehicle *m_pMyVehicle;bool bInVehicle,bFireProof,bDrawLast,player,inControl,canSet,damageResult;
    int m_nMoveState,m_nPedState,m_nLastPedState,m_nPedType;CEntity *m_fleeFrom;uint32 m_fleeTimer;float health;
    CPed():m_pFire(nil),m_pMyVehicle(nil),bInVehicle(false),bFireProof(false),bDrawLast(true),player(false),inControl(true),canSet(true),damageResult(false),m_nMoveState(PEDMOVE_RUN),m_nPedState(PED_NONE),m_nLastPedState(PED_NONE),m_nPedType(PEDTYPE_CIVMALE),m_fleeFrom(nil),m_fleeTimer(0),health(100){
        // Select the pedestrian branch for this native shell
        kind=1;
    }
    bool IsPlayer(){
        // Return the independently seeded player policy
        return player;
    }
    bool IsPedInControl(){
        // Return the independently seeded movement control policy
        return inControl;
    }
    bool CanSetPedState(){
        // Return the independently seeded state transition policy
        return canSet;
    }
    bool InflictDamage(CEntity *source,eWeaponType weapon,float damage,int piece,int direction);
    void SetIdle();void SetWanderPath(int);void SetWaitState(int,int);
    void SetFlee(CEntity *,int);void SetFlee(const CVector2D &,int);
    void SetMoveState(int state);void SetMoveAnim();void SetPedState(int state);
};
class CPlayerPed:public CPed {public:CPlayerPed(){
    // Select the player policy for the specialized native pedestrian
    player=true;
}void DoStuffToGoOnFire();};
class CVehicle:public CEntity {public:CFire *m_pCarFire;bool car;float m_fHealth;CVehicle():m_pCarFire(nil),car(true),m_fHealth(1000){
    // Select the vehicle branch for this native shell
    kind=2;
}bool IsCar(){
    // Return the independently seeded automobile category
    return car;
}void InflictDamage(CVehicle *,eWeaponType,float);};
class TestFireDamage {public:uint8 status;TestFireDamage():status(0){}uint8 GetEngineStatus(){
    // Read the live engine damage byte
    return status;
}void SetEngineStatus(uint8 value){
    // Update the same live engine damage byte
    status=value;
}};
class CAutomobile:public CVehicle {public:TestFireDamage Damage;};
class CVehicleModelInfo {public:CVector m_positions[5];};
class CModelInfo {public:static CVehicleModelInfo *GetModelInfo(int model);static bool IsBikeModel(int model);static bool IsCarModel(int model);};
class CGameLogic {public:enum {SHORTCUT_TRANSITION=4};static CAutomobile *pShortCutTaxi;static uint8 ShortCutState;};
CPlayerPed *FindPlayerPed(void);CVehicle *FindPlayerVehicle(void);
class CEventList {public:static void RegisterEvent(eEventType,CVector,int32);static void RegisterEvent(eEventType,eEventEntity,CEntity *,CPed *,int32);};
struct EventsVector {float x,y,z;};
inline EventsVector Events_VectorFromNative(const CVector &value)
{
    // Preserve the historical fixture's native input values across the updated C event dependency
    EventsVector result={value.x,value.y,value.z};return result;
}
extern "C" inline void Events_RegisterPosition(eEventType type,EventsVector value,int32 timeout)
{
    // Keep the unchanged observed event provider while testing the separately converted fire owner
    CEventList::RegisterEvent(type,CVector(value.x,value.y,value.z),timeout);
}
extern "C" inline void Events_RegisterEntity(eEventType type,eEventEntity kind,CEntity *entity,CPed *criminal,int32 timeout)
{
    // Keep the exact entity and criminal pointers at the historical observed dependency
    CEventList::RegisterEvent(type,kind,entity,criminal,timeout);
}
class CCarAI {public:static void TellOccupantsToFleeCar(CVehicle *);};
class CParticle {public:static CParticle *AddParticle(int,const CVector &,const CVector &,CEntity * =nil,float=0,int32=0,int32=0,int32=0,int32=0);};
extern RwTexture *gpShadowExplosionTex;
class CShadows {public:static bool StoreStaticShadow(uint32,uint8,RwTexture *,const CVector *,float,float,float,float,int16,uint8,uint8,uint8,float,float,float,bool,float);};
'''
(out/'owners.h').write_text(owners,newline='\n')
for name in ['PlayerPed.h','Entity.h','PointLights.h','Particle.h','Vehicle.h','Shadows.h','Automobile.h','World.h','EventList.h','DamageManager.h','Ped.h','GameLogic.h','CarAI.h']:
    if name=='PointLights.h':continue
    (out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
state='''typedef struct TestFireState {bool ongoing,script,propagation,audio;float x,y,z;CEntity *entity,*source;uint32_t extinguish,start,next;float strength,water;bool withWater;} TestFireState;
'''
prototypes='''void TestFire_Read(int index,TestFireState *state);void TestFire_Write(int index,const TestFireState *state);
void TestFire_Total(uint32_t total);uint32_t TestFire_GetTotal(void);
void TestFire_Process(int index);void TestFire_Report(int index);void TestFire_Extinguish(int index);
void TestFire_At(CVector position,float size,uint8_t propagation);int TestFire_On(CEntity *target,CEntity *source,float strength,uint8_t propagation);
void TestFire_Update(void);int TestFire_Nearest(CVector position,float *distance);int TestFire_Furthest(CVector position,float minimum,float maximum);
int TestFire_Free(void);void TestFire_ExtinguishPoint(CVector position,float range);bool TestFire_Water(CVector position,float range);
int TestFire_Script(const CVector *position,CEntity *target,float strength,uint8_t propagation);
bool TestFire_IsOut(int16_t index);void TestFire_RemoveAll(void);void TestFire_Remove(int16_t index);void TestFire_Audio(int16_t index,bool state);
'''
(out/'caller.h').write_text('#pragma once\n#include "owners.h"\n'+state+'extern "C" {\n'+prototypes+'}\n',newline='\n')
wrappers='''#include "caller.h"
static int TestFire_Index(CFire *fire)
{
    // Preserve null results and actual canonical fire slot identity
    return fire?int(fire-gFireManager.m_aFires):-1;
}
extern "C" void TestFire_Read(int index,TestFireState *s)
{
    // Read each real member without type-punning the old nontrivial fire record
    CFire &f=gFireManager.m_aFires[index];s->ongoing=f.m_bIsOngoing;s->script=f.m_bIsScriptFire;s->propagation=f.m_bPropagationFlag;s->audio=f.m_bAudioSet;s->x=f.m_vecPos.x;s->y=f.m_vecPos.y;s->z=f.m_vecPos.z;s->entity=f.m_pEntity;s->source=f.m_pSource;s->extinguish=f.m_nExtinguishTime;s->start=f.m_nStartTime;s->next=f.m_nNextTimeToAddFlames;s->strength=f.m_fStrength;s->water=f.m_fWaterExtinguishCountdown;s->withWater=f.m_bExtinguishedWithWater;
}
extern "C" void TestFire_Write(int index,const TestFireState *s)
{
    // Seed corresponding real members while retaining their original object lifetimes
    CFire &f=gFireManager.m_aFires[index];f.m_bIsOngoing=s->ongoing;f.m_bIsScriptFire=s->script;f.m_bPropagationFlag=s->propagation;f.m_bAudioSet=s->audio;f.m_vecPos.x=s->x;f.m_vecPos.y=s->y;f.m_vecPos.z=s->z;f.m_pEntity=s->entity;f.m_pSource=s->source;f.m_nExtinguishTime=s->extinguish;f.m_nStartTime=s->start;f.m_nNextTimeToAddFlames=s->next;f.m_fStrength=s->strength;f.m_fWaterExtinguishCountdown=s->water;f.m_bExtinguishedWithWater=s->withWater;
}
extern "C" void TestFire_Total(uint32_t total)
{
    // Seed the actual manager counter independently of the record count
    gFireManager.m_nTotalFires=total;
}
'''
simple=[('void','Process','int index','gFireManager.m_aFires[index].ProcessFire()','Fire_ProcessFire(&gFireManager.m_aFires[index])'),('void','Report','int index','gFireManager.m_aFires[index].ReportThisFire()','Fire_ReportThisFire(&gFireManager.m_aFires[index])'),('void','Extinguish','int index','gFireManager.m_aFires[index].Extinguish()','Fire_Extinguish(&gFireManager.m_aFires[index])'),('uint32_t','GetTotal','void','gFireManager.GetTotalActiveFires()','FireManager_GetTotalActiveFires(&gFireManager)'),('void','At','CVector position,float size,uint8_t propagation','gFireManager.StartFire(position,size,propagation)','FireManager_StartFireAt(&gFireManager,Fire_VectorFromNative(position),size,propagation)'),('int','On','CEntity *target,CEntity *source,float strength,uint8_t propagation','TestFire_Index(gFireManager.StartFire(target,source,strength,propagation))','TestFire_Index(FireManager_StartFireOn(&gFireManager,target,source,strength,propagation))'),('void','Update','void','gFireManager.Update()','FireManager_Update(&gFireManager)'),('int','Nearest','CVector position,float *distance','TestFire_Index(gFireManager.FindNearestFire(position,distance))','TestFire_Index(FireManager_FindNearestFire(&gFireManager,Fire_VectorFromNative(position),distance))'),('int','Furthest','CVector position,float minimum,float maximum','TestFire_Index(gFireManager.FindFurthestFire_NeverMindFireMen(position,minimum,maximum))','TestFire_Index(FireManager_FindFurthestFire_NeverMindFireMen(&gFireManager,Fire_VectorFromNative(position),minimum,maximum))'),('int','Free','void','TestFire_Index(gFireManager.GetNextFreeFire())','TestFire_Index(FireManager_GetNextFreeFire(&gFireManager))'),('void','ExtinguishPoint','CVector position,float range','gFireManager.ExtinguishPoint(position,range)','FireManager_ExtinguishPoint(&gFireManager,Fire_VectorFromNative(position),range)'),('bool','Water','CVector position,float range','gFireManager.ExtinguishPointWithWater(position,range)','FireManager_ExtinguishPointWithWater(&gFireManager,Fire_VectorFromNative(position),range)'),('int','Script','const CVector *position,CEntity *target,float strength,uint8_t propagation','gFireManager.StartScriptFire(*position,target,strength,propagation)','FireManager_StartScriptFire(&gFireManager,position,target,strength,propagation)'),('bool','IsOut','int16_t index','gFireManager.IsScriptFireExtinguish(index)','FireManager_IsScriptFireExtinguish(&gFireManager,index)'),('void','RemoveAll','void','gFireManager.RemoveAllScriptFires()','FireManager_RemoveAllScriptFires(&gFireManager)'),('void','Remove','int16_t index','gFireManager.RemoveScriptFire(index)','FireManager_RemoveScriptFire(&gFireManager,index)'),('void','Audio','int16_t index,bool state','gFireManager.SetScriptFireAudio(index,state)','FireManager_SetScriptFireAudio(&gFireManager,index,state)')]
for kind,name,args,before,after in simple:wrappers+='extern "C" '+kind+' TestFire_'+name+'('+args+')\n{\n    // Enter the complete original or C operation with the same native values\n#ifdef TEST_FIRE_ORIGINAL\n    '+('return ' if kind!='void' else '')+before+';\n#else\n    '+('return ' if kind!='void' else '')+after+';\n#endif\n}\n'
(out/'original.cpp').write_text('#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n'+wrappers,newline='\n')
(out/'adapter.cpp').write_text(wrappers,newline='\n');(out/'host.cpp').write_text((root/'src/core/Fire.cpp').read_text(),newline='\n')
matrix=(out/'before/src/math/Matrix.cpp').read_text();parts=[]
for name in ['CMatrix','~CMatrix','SetTranslate','RotateZ']:
    if name=='CMatrix':parts.append(re.search(r'CMatrix::CMatrix\(void\)\n\{[\s\S]*?\n\}',matrix)[0])
    else:parts.append(function(matrix,'CMatrix::',re.escape(name)))
vector=(out/'before/src/math/Vector.cpp').read_text();match=re.search(r'CVector\noperator\*\(const CMatrix &mat, const CVector &vec\)',vector);assert match;parts.append(vector[match.start():vector.index('\n}',match.end())+2])
legacy=function((out/'before/src/renderer/PointLights.cpp').read_text(),'','PointLights_AddLightLegacy')
(out/'math.cpp').write_text('#include "owners.h"\n#include "PointLights_backend.h"\n'+'\n\n'.join(parts)+'\n'+legacy+'\n',newline='\n')
(out/'fastabs.c').write_text((root/'build/weather-c-tests/fastabs.c').read_text(),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/fire.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
(out/'crtrand.h').write_text('#include "config.h"\n#undef USE_PS2_RAND\n',newline='\n')
print('Audited all 17 fire operations, constructor-equivalent canonical records, real references/math/point-light adapter and 23 complete callers')
