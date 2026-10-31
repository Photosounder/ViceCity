"""Prepare whole original/C roadblock comparisons using actual vector arithmetic and C pool primitives."""
from pathlib import Path
import re,json,sys
import roadblocks_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/roadblocks-c-tests';saved=out/'before';preview=out/'preview'
player=(saved/'src/core/PlayerInfo.cpp').read_text();getter=re.search(r'FindPlayerPed\(void\)\s*\{([^}]+)\}',player)[1]
assert re.sub(r'\s+','',getter)=='returnCWorld::Players[CWorld::PlayerInFocus].m_pPed;'
random=(saved/'src/core/re3.cpp').read_text();randomBody=re.search(r'myrand\(void\)\s*\{([^}]+)\}',random)[1];assert 'CWorld' not in randomBody and 'Wanted' not in randomBody
if '--production' in sys.argv:
    for path in json.loads((out/'changes.json').read_text()):assert (root/path).read_text()==m.calls((saved/path).read_text()),path
    for name in ['RoadBlockSystem.c','RoadBlocks_backend.h']:assert (root/'src/control'/name).read_text()==(preview/name).read_text(),name
for name,path in [('Pool.h','src/core/Pool.h'),('maths.h','src/math/maths.h'),('Vector.h','src/math/Vector.h'),('Vector2D.h','src/math/Vector2D.h'),('Matrix.h','src/math/Matrix.h'),('ModelIds.h','src/modelinfo/ModelIds.h'),('ModelIndexData.h','src/modelinfo/ModelIndexData.h'),('WeaponType.h','src/weapons/WeaponType.h'),('EntityStatus.h','src/entities/EntityStatus.h'),('Draw_constants.h','src/renderer/Draw_constants.h')]:
    (out/name).write_text((saved/path).read_text(),newline='\n')
(out/'original-RoadBlocks.h').write_text((saved/'src/control/RoadBlocks.h').read_text(),newline='\n')
config=(saved/'src/core/config.h').read_text();sizes=[(name,re.search(r'\b'+name+r'\s*=\s*(\d+)',config)[1]) for name in ['NUMROADBLOCKS','NUM_SCRIPT_ROADBLOCKS']]
(out/'config.h').write_text('#pragma once\n'+''.join('#define '+name+' '+value+'\n' for name,value in sizes),newline='\n')
(out/'ModelIndices.h').write_text('#pragma once\n#include "ModelIndexData.h"\n',newline='\n')
(out/'Wanted.h').write_text('''#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct CWanted {uint8_t m_RoadblockDensity;bool army,fbi,swat;} CWanted;
#ifdef __cplusplus
extern "C" {
#endif
bool Wanted_AreArmyRequired(const CWanted*);bool Wanted_AreFbiRequired(const CWanted*);bool Wanted_AreSwatRequired(const CWanted*);
#ifdef __cplusplus
}
#endif
''',newline='\n')
(out/'Timer.h').write_text('''#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
const uint32_t *Timer_GetTimeInMilliseconds(void);const uint32_t *Timer_GetFrameCounter(void);
#ifdef __cplusplus
}
#endif
''',newline='\n')
(out/'General.h').write_text('''#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
uint16_t General_GetRandomNumber(void);
#ifdef __cplusplus
}
#endif
''',newline='\n')
owners='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory>
#include "config.h"
#include "Wanted.h"
#include "General.h"
#include "Timer.h"
#include "EntityStatus.h"
#include "WeaponType.h"
#include "ModelIndices.h"
#include "Draw_constants.h"
typedef int8_t int8;typedef int16_t int16;typedef int32_t int32;typedef uint8_t uint8;typedef uint16_t uint16;typedef uint32_t uint32;
#define nil NULL
#define Min(a,b) ((a)<(b)?(a):(b))
#define LANE_WIDTH 5.0f
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
void re3_assert(const char*,const char*,unsigned,const char*);
#ifdef MASTER
#define assert(expression) (expression)
#else
#define assert(expression) (void)((!!(expression))||(re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#endif
#include "Pool.h"
#include "maths.h"
struct RwV3d {float x,y,z;};
struct RwMatrix {RwV3d right,up,at,pos;};void RwMatrixUpdate(RwMatrix*);void RwMatrixDestroy(RwMatrix*);
#include "Vector.h"
#include "Vector2D.h"
#include "Matrix.h"
struct CColSphere {float radius;};struct CColBox {CVector min;};
class CColModel {public:CColSphere boundingSphere;CColBox boundingBox;};
class CBaseModelInfo {public:CColModel col;CColModel *GetColModel();};
class CModelInfo {public:static CColModel *GetColModel(int);static CBaseModelInfo *GetModelInfo(int);};
class CEntity {public:int identity,model,status;CMatrix matrix;CEntity();void RegisterReference(CEntity**);int GetModelIndex();void SetStatus(eEntityStatus);void *GetClump();CMatrix &GetMatrix(){return matrix;}CVector &GetPosition(){return matrix.GetPosition();}CVector &GetRight(){return matrix.GetRight();}CVector &GetUp(){return matrix.GetUp();}void SetPosition(CVector p);};
struct NativePrefix {virtual ~NativePrefix(){};uintptr_t prefix;};
struct TestAutoPilot {int8 m_nCurrentLane,m_nNextLane;uint8 m_nCarMission,m_nTempAction,m_nCruiseSpeed;float m_fMaxTrafficSpeed;};
enum eCopType {COP_STREET=0,COP_FBI=1,COP_SWAT=2,COP_ARMY=4};enum eCarLock {CARLOCK_UNLOCKED=1};
enum {RANDOM_VEHICLE=1,TEMP_OBJECT=3,STREAMFLAGS_DONT_REMOVE=1,MISSION_NONE=0,TEMPACT_NONE=0};
class CVehicle:public NativePrefix,public CEntity {public:float radius;CVehicle();float GetBoundRadius();};
class CAutoMobileUnused;
class CAutomobile:public CVehicle {public:eCarLock m_nDoorLock;TestAutoPilot AutoPilot;bool bIsLocked,bExtendedRange,m_bSirenOrAlarm,bCreateRoadBlockPeds,isStatic,siren;uint8 m_nRoadblockType;uint32 m_nSetPieceExtendedRangeTime;CAutomobile(int model,int createdBy);~CAutomobile();void SetMatrix(const CMatrix&);void PlaceOnRoadProperly();void SetIsStatic(bool);bool UsesSiren();};
class CPed:public NativePrefix,public CEntity {public:CPed();};
class CCopPed:public CPed {public:bool m_bIsDisabledCop,bKindaStayInSamePlace,bNotAllowedToDuck,bCrouchWhenShooting,bCullExtraFarAway;uint32 m_nExtendedRangeTimer;CVehicle *m_nRoadblockVeh,*m_pMyVehicle;int type,weapon;CEntity *target;CCopPed(eCopType);void SetCurrentWeapon(eWeaponType);void SetOrientation(float,float,float);void SetIdle();void SetWeaponLockOnTarget(CEntity*);void SetAttack(CEntity*);};
class CPlayerPed:public CPed {public:CWanted *m_pWanted;};
class CObject:public NativePrefix,public CEntity {public:int8 ObjectCreatedBy;uint32 m_nEndOfLifeTime;CObject(int,bool);~CObject();};
typedef CPool CObjectPool;
class CPools {public:static CPool *GetPedPool();static CPool *GetVehiclePool();static CPool *GetObjectPool();};
class CStreaming {public:static bool HasModelLoaded(int);static void RequestModel(int,int);};
class CPedPlacement {public:static void FindZCoorForPed(CVector*);};
class CVisibilityPlugins {public:static void SetClumpAlpha(void*,int);};
class CWorld {public:static void Add(CEntity*);static void Remove(CEntity*);static void FindObjectsKindaColliding(CVector,float,int,int16*,int,CEntity**,bool,bool,bool,bool,bool);static float FindGroundZFor3DCoord(float,float,float,bool*);};
class CCarCtrl {public:static void JoinCarWithRoadSystem(CAutomobile*);};
struct CPathNode {int16 x,y,z,firstLink;uint8 width,numLinks;bool bUseInRoadBlock;CVector GetPosition(){return CVector(x/8.0f,y/8.0f,z/8.0f);}bool HasDivider(){return width!=0;}float GetDividerWidth(){return width/(2*8.0f);}};
struct CCarPathLink {int16 x,y;int8 numLeftLanes,numRightLanes;float GetX(){return x/8.0f;}float GetY(){return y/8.0f;}};
struct TestPaths {int m_numCarPathNodes;CPathNode m_pathNodes[NUMROADBLOCKS+12];int m_carPathConnections[2*(NUMROADBLOCKS+12)];CCarPathLink m_carPathLinks[2*(NUMROADBLOCKS+12)];};extern TestPaths ThePaths;
CVector FindPlayerCoors();CVehicle *FindPlayerVehicle();CPlayerPed *FindPlayerPed();
#ifdef TEST_ROADBLOCKS_ORIGINAL
#include "original-RoadBlocks.h"
struct RoadBlockVector {float x,y,z;};
#define GLOBAL(name) CRoadBlocks::name
#else
#include "@PREVIEW@/RoadBlocks.h"
#define GLOBAL(name) RoadBlocks_##name
#endif
#include "entry.h"
'''.replace('@PREVIEW@',preview.as_posix())
(out/'owners.h').write_text(owners,newline='\n');(out/'common.h').write_text('#pragma once\n#include "owners.h"\n',newline='\n')
for name in ['PathFind','Streaming','World','PedPlacement','Automobile','CopPed','VisibilityPlugins','PlayerPed','CarCtrl','Object','Pools','Camera']:(out/(name+'.h')).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
api='''#pragma once
#include <stdint.h>
#ifdef __cplusplus
class CVehicle;
extern "C" {
#else
typedef struct CVehicle CVehicle;
#endif
typedef struct RBTestVector {float x,y,z;} RBTestVector;
'''
signatures=[('void','Init','void',[]),('void','GenerateRoadBlockCopsForCar','CVehicle *vehicle,int32_t roadBlockType',['vehicle','roadBlockType']),('void','GenerateRoadBlocks','void',[]),('void','ClearScriptRoadBlocks','void',[]),('void','RegisterScriptRoadBlock','RBTestVector inf,RBTestVector sup',['inf','sup']),('void','CreateRoadBlockBetween2Points','RBTestVector point1,RBTestVector point2',['point1','point2']),('RBTestVector','Midpoint','RBTestVector inf,RBTestVector sup',['inf','sup'])]
original='#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/control/RoadBlocks.cpp').read_text(),flags=re.M)+'\n'
caller='#include "entry.h"\n#ifndef TEST_ROADBLOCKS_ORIGINAL\n#include "'+(preview/'RoadBlocks.h').as_posix()+'"\n#endif\n'
for ret,name,args,values in signatures:
    api+=ret+' Before_RoadBlocks_'+name+'('+args+');\n'+ret+' Run_RoadBlocks_'+name+'('+args+');\n'
    nativeArgs=','.join('CVector('+v+'.x,'+v+'.y,'+v+'.z)' if 'RBTestVector' in args else v for v in values)
    cArgs=','.join('(RoadBlockVector){'+v+'.x,'+v+'.y,'+v+'.z}' if 'RBTestVector' in args else v for v in values)
    native='CRoadBlocks::'+name+'('+nativeArgs+');';plain='RoadBlocks_'+name+'('+cArgs+');'
    if name=='Midpoint':
        native='CScriptRoadblock block;block.m_vInf=CVector(inf.x,inf.y,inf.z);block.m_vSup=CVector(sup.x,sup.y,sup.z);CVector result=block.GetPosition();RBTestVector value={result.x,result.y,result.z};return value;'
        plain='CScriptRoadblock block={(RoadBlockVector){inf.x,inf.y,inf.z},(RoadBlockVector){sup.x,sup.y,sup.z},false};RoadBlockVector result=ScriptRoadblock_GetPosition(&block);RBTestVector value={result.x,result.y,result.z};return value;'
    original+='extern "C" '+ret+' Before_RoadBlocks_'+name+'('+args+')\n{\n    // Invoke the complete original operation with its original native argument conversions\n    '+native+'\n}\n'
    caller+=ret+' Run_RoadBlocks_'+name+'('+args+')\n{\n    // Exercise the complete operation through a standalone strict C11 caller\n#ifdef TEST_ROADBLOCKS_ORIGINAL\n    '+('return ' if ret!='void' else '')+'Before_RoadBlocks_'+name+'('+','.join(values)+');\n#else\n    '+plain+'\n#endif\n}\n'
api+='#ifdef __cplusplus\n}\n#endif\n';(out/'entry.h').write_text(api,newline='\n');(out/'caller.c').write_text(caller,newline='\n');(out/'original.cpp').write_text(original,newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n#include "'+(preview/'RoadBlocks_backend.h').as_posix()+'"\n'+re.sub(r'^#include[^\n]*\n','',m.host(),flags=re.M),newline='\n');(out/'kernel.c').write_text((preview/'RoadBlockSystem.c').read_text(),newline='\n')
math='#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/math/Vector.cpp').read_text(),flags=re.M)+'\n'
matrix=(saved/'src/math/Matrix.cpp').read_text()
for name in ['CMatrix(void)','~CMatrix(void)','SetTranslate(float x, float y, float z)','RotateZ(float z)','operator=(CMatrix const &rhs)','UpdateRW(void)']:
    start=matrix.index('CMatrix::'+name);opening=matrix.index('{',start);end=matrix.index('\n}',opening)+2
    returnType='' if name in ['CMatrix(void)','~CMatrix(void)'] else 'void\n'
    math+=returnType+matrix[start:end]+'\n'
(out/'math.cpp').write_text(math,newline='\n');(out/'fixture.cpp').write_text((root/'utils/tests/roadblocks.cpp.in').read_text(),newline='\n')
print('Prepared all seven original/C roadblock operations, actual vector/matrix math, actual C pools and native placement models')
