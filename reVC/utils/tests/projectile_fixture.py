"""Audit the whole controller relocation and emit original/C console comparison programs."""
from pathlib import Path
import json,re
import projectile_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/projectile-c-tests'
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/weapons/ProjectileInfo.cpp').read_text();header=(out/'before/src/weapons/ProjectileInfo.h').read_text()
assert (root/'src/weapons/ProjectileControl.c').read_text()==m.kernel(old)
assert (root/'src/weapons/Projectile_backend.h').read_text()==m.BACKEND
normalize=lambda text:re.sub(r'\s+','',re.sub(r'//[^\n]*','',text))
for helper in ['Timer_GetDefaultTimeStep','Timer_GetTimeStepFix']:
    assert normalize(function((out/'before/src/core/Timer.h').read_text(),'',helper))==normalize(function(m.kernel(old),'',helper)),helper
for path in json.loads((out/'dependencies.json').read_text()):
    if path not in json.loads((out/'changes.json').read_text()):
        import fire_migration
        fire_migration.verify_dependency(out/'before'/path,root/path)
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
#include <memory>
#include "config.h"
typedef uint8_t uint8;typedef uint16_t uint16;typedef uint32_t uint32;
typedef int8_t int8;typedef int16_t int16;typedef int32_t int32;
struct RwV3d {float x,y,z;};
struct RwMatrix {RwV3d right,up,at,pos;};
class CMatrix;
#define nil NULL
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
template<typename T> T Max(T a,T b){return a>b?a:b;}
#define GRAVITY (0.008f)
#include "maths.h"
#include "Vector.h"
#include "Vector2D.h"
#include "Matrix.h"
#endif
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Timer.h"
#undef malloc
#undef free
#undef debug
#undef Error
void TestProjectile_Debug(const char *,...);
void TestProjectile_Error(const char *,...);
void TestProjectile_MatrixUpdate(RwMatrix *);
void TestProjectile_MatrixDestroy(RwMatrix *);
#define debug TestProjectile_Debug
#define Error TestProjectile_Error
#undef RwMatrixUpdate
#undef RwMatrixDestroy
#define RwMatrixUpdate TestProjectile_MatrixUpdate
#define RwMatrixDestroy TestProjectile_MatrixDestroy
'''
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
owners='''#pragma once
#include "common.h"
#include "ProjectileInfo.h"
#include "Pool.h"
#include "ExplosionTypes.h"
#include "ModelIndexData.h"
#ifndef GRAVITY
#define GRAVITY (0.008f)
#endif
enum {COLLRESPONSE_NONE=0,COLLRESPONSE_UNKNOWN5=5,UNKNOWN_OBJECT=0,MISSION_OBJECT=2,TEMP_OBJECT=3};
class CEntity;class CObject;
void TestProjectile_Destroy(CObject *);
void TestProjectile_Construct(CObject *);
class CEntity {
public:
    CMatrix matrix;int32 model,id,kind;
    CEntity():model(0),id(-1),kind(0){matrix.SetTranslate(0,0,0);}
    int32 GetModelIndex(){return model;}
    void SetModelIndex(int32 value){model=value;}
    CMatrix &GetMatrix(){return matrix;}
    CVector &GetPosition(){return matrix.GetPosition();}
    CVector &GetForward(){return matrix.GetForward();}
    CVector &GetUp(){return matrix.GetUp();}
    bool IsPed(){return kind==1;}
    bool IsVehicle(){return kind==2;}
};
class CPhysical:public CEntity {
public:
    CVector m_vecMoveSpeed;bool bAffectedByGravity,bHasCollided,bExplosionProof;
    float m_fMass,m_fTurnMass,m_fAirResistance,m_fElasticity,m_fBuoyancy;
    CPhysical():m_vecMoveSpeed(0,0,0),bAffectedByGravity(true),bHasCollided(false),bExplosionProof(false),m_fMass(0),m_fTurnMass(0),m_fAirResistance(0),m_fElasticity(0),m_fBuoyancy(0){}
    void SetMoveSpeed(const CVector &value){m_vecMoveSpeed=value;}
};
class CObject:public CPhysical {
public:
    int8 ObjectCreatedBy;uint8 m_nSpecialCollisionResponseCases;
    CObject():ObjectCreatedBy(UNKNOWN_OBJECT),m_nSpecialCollisionResponseCases(0){kind=3;TestProjectile_Construct(this);}
    ~CObject(){TestProjectile_Destroy(this);}
};
class CProjectile:public CObject {public:CProjectile(int32);};
struct TestProjectileWeapon {eWeaponType m_eWeaponType;uint32 m_nAmmoTotal;};
class CPed:public CPhysical {
public:
    bool player,valid;CEntity *m_pSeekTarget,*m_pCollidingEntity;TestProjectileWeapon weapon;
    CPed():player(false),valid(true),m_pSeekTarget(nil),m_pCollidingEntity(nil){kind=1;weapon={WEAPONTYPE_DETONATOR,3};}
    bool IsPlayer(){return player;}
    bool IsPointerValid(){return valid;}
    int GetWeaponSlot(eWeaponType);
    TestProjectileWeapon &GetWeapon(int);
};
class CVehicle:public CPhysical {public:CEntity *pDriver;CVehicle():pDriver(nil){kind=2;}};
struct TestProjectileCam {CVector Front,Up;};
struct TestProjectileCamera {TestProjectileCam Cams[3];uint8 ActiveCam;};
extern TestProjectileCamera TheCamera;
typedef CPool CObjectPool;
class CPools {public:static CObjectPool *GetObjectPool();};
class CWorld {
public:
    static CEntity *pIgnoreEntity;
    static void Add(CEntity *);static void Remove(CEntity *);
    static bool GetIsLineOfSightClear(const CVector &,const CVector &,bool,bool,bool,bool,bool,bool,bool=false);
    static void SetPedsChoking(float,float,float,float,CEntity *);
};
class CExplosion {public:static bool AddExplosion(CEntity *,CEntity *,eExplosionType,const CVector &,uint32,bool=true);};
enum {PARTICLE_TEARGAS=26,PARTICLE_ROCKET_SMOKE=17};
class CParticle {
public:
    static CParticle *AddParticle(int,const CVector &,const CVector &,CEntity *=nil,float=0,int32=0,int32=0,int32=0,int32=0);
    static void AddParticlesAlongLine(int,const CVector &,const CVector &,const CVector &,float,CEntity *=nil,float=0,int32=0,int32=0,int32=0,int32=0);
};
class CHeli {public:static bool TestRocketCollision(CVector *);};
class CPlane {public:static bool TestRocketCollision(CVector *);};
'''.replace('CEntity *=','CEntity * =')
(out/'owners.h').write_text(owners,newline='\n')
for name in ['Camera.h','Ped.h','Projectile.h','Explosion.h','World.h','Particle.h','Heli.h','Plane.h','Weapon.h','Pools.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
(out/'ModelIndices.h').write_text('#pragma once\n#include "ModelIndexData.h"\n',newline='\n')
prototypes='''void TestProjectile_Init(void);void TestProjectile_Shutdown(void);void TestProjectile_Update(void);
bool TestProjectile_Add(CEntity *,eWeaponType,const CVector *,float);
void TestProjectile_Remove(int);void TestProjectile_NotAdd(CEntity *,eWeaponType,const CVector *);
bool TestProjectile_Range(float,float,float,float,float,float,bool);
void TestProjectile_All(void);void TestProjectile_Detonators(void);bool TestProjectile_If(CObject *);
void TestProjectile_Read(int,ProjectileInfo *);void TestProjectile_Write(int,const ProjectileInfo *);
CProjectile *TestProjectile_Get(int);void TestProjectile_Set(int,CProjectile *);uint32_t TestProjectile_Count(void);void TestProjectile_SetCount(uint32_t);
'''
(out/'caller.h').write_text('#pragma once\n#include "owners.h"\nextern "C" {\n'+prototypes+'}\n',newline='\n')
for before in [True,False]:
    text='#include "caller.h"\n#include "General.h"\n'
    if before:
        h=re.sub(r'#pragma once|#include[^\n]*\n|class CEntity;|class CObject;|class CProjectile;','',header)
        timer=(out/'before/src/core/Timer.h').read_text()
        helper='\n#ifndef FIX_BUGS\n'+function(timer,'','Timer_GetDefaultTimeStep')+'\n'+function(timer,'','Timer_GetTimeStepFix')+'\n#endif\n'
        text+='namespace Original {\n'+helper+h+'\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n}\n'
    prefix='Original::CProjectileInfo::' if before else 'ProjectileInfo_'
    methods=[('void','Init','void','Initialise',''),('void','Shutdown','void','Shutdown',''),('void','Update','void','Update',''),('bool','Add','CEntity *entity,eWeaponType type,const CVector *pos,float speed','AddProjectile','entity,type,'+('*pos' if before else 'pos')+',speed'),('void','NotAdd','CEntity *entity,eWeaponType type,const CVector *pos','RemoveNotAdd','entity,type,'+('*pos' if before else 'pos')),('bool','Range','float x1,float x2,float y1,float y2,float z1,float z2,bool remove','IsProjectileInRange','x1,x2,y1,y2,z1,z2,remove'),('void','All','void','RemoveAllProjectiles',''),('void','Detonators','void','RemoveDetonatorProjectiles',''),('bool','If','CObject *object','RemoveIfThisIsAProjectile','object')]
    for kind,alias,args,method,values in methods:text+='extern "C" '+kind+' TestProjectile_'+alias+'('+args+')\n{\n    // Enter the complete original or converted operation with identical captured arguments\n    '+('return ' if kind!='void' else '')+prefix+method+'('+values+');\n}\n'
    records='Original::gaProjectileInfo' if before else 'ProjectileInfo_records';array='Original::CProjectileInfo::ms_apProjectile' if before else 'ProjectileInfo_projectiles'
    text+='extern "C" void TestProjectile_Remove(int i)\n{\n    // Remove the actual indexed record and live native projectile\n    '+prefix+'RemoveProjectile('+prefix+'GetProjectileInfo(i),'+array+'[i]);\n}\n'
    for alias,kind,args,body in [('Read','void','int i,ProjectileInfo *out','memcpy(out,'+prefix+'GetProjectileInfo(i),sizeof(*out));'),('Write','void','int i,const ProjectileInfo *value','memcpy('+prefix+'GetProjectileInfo(i),value,sizeof(*value));'),('Get','CProjectile *','int i','return '+array+'[i];'),('Set','void','int i,CProjectile *value',array+'[i]=value;')]:text+='extern "C" '+kind+' TestProjectile_'+alias+'('+args+')\n{\n    // Access the actual canonical records and pointer slots without substitute storage\n    '+body+'\n}\n'
    count='Original::projectileInUse' if before else 'projectileInUse'
    text+='extern "C" uint32_t TestProjectile_Count(void)\n{\n    // Observe the real optional active counter without synthesizing a count\n#ifdef SQUEEZE_PERFORMANCE\n    return '+count+';\n#else\n    return 0;\n#endif\n}\n'
    text+='extern "C" void TestProjectile_SetCount(uint32_t value)\n{\n    // Seed the actual optional counter alongside explicitly supplied fixture records\n#ifdef SQUEEZE_PERFORMANCE\n    '+count+'=value;\n#else\n    (void)value;\n#endif\n}\n'
    (out/('original.cpp' if before else 'adapter.cpp')).write_text(text,newline='\n')
matrix=(root/'src/math/Matrix.cpp').read_text()
parts=[]
for name in ['CMatrix','~CMatrix','UpdateRW','operator=','SetTranslate','Rotate','RotateZ']:
    if name=='CMatrix':
        match=re.search(r'CMatrix::CMatrix\(void\)\n\{[\s\S]*?\n\}',matrix);assert match;parts.append(match[0])
    else:parts.append(function(matrix,'CMatrix::',re.escape(name)))
vector=(root/'src/math/Vector.cpp').read_text()
for signature in [r'CVector\nCrossProduct\(',r'CVector\nMultiply3x3\(const CMatrix &mat, const CVector &vec\)']:
    match=re.search(signature,vector);assert match,signature
    start=match.start();opening=vector.index('{',start);end=vector.index('\n}',opening)+2;parts.append(vector[start:end])
(out/'math.cpp').write_text('#include "owners.h"\n'+'\n\n'.join(parts)+'\n',newline='\n')
(out/'projectile.cpp').write_text((root/'src/objects/Projectile.cpp').read_text(),newline='\n')
(out/'host.cpp').write_text((root/'src/weapons/ProjectileInfo.cpp').read_text(),newline='\n')
(out/'fastabs.c').write_text((root/'build/weather-c-tests/fastabs.c').read_text(),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/projectile.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all eleven projectile operations, canonical records and pointers, real pool/vector/matrix/constructor sources and nine complete callers')
