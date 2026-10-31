"""Move every projectile-controller record and lifecycle decision into C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import gangs_migration as gangs
METHODS=['Initialise','Shutdown','GetProjectileInfo','AddProjectile','RemoveProjectile','RemoveNotAdd','Update','IsProjectileInRange','RemoveDetonatorProjectiles','RemoveAllProjectiles','RemoveIfThisIsAProjectile']
OBSERVER=None
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "WeaponType.h"
#ifdef __cplusplus
class CEntity;class CObject;class CProjectile;class CVector;
typedef CVector ProjectilePosition;
#else
typedef struct CEntity CEntity;typedef struct CObject CObject;typedef struct CProjectile CProjectile;
typedef struct CVector ProjectilePosition;
#endif
typedef struct ProjectileVector {float x,y,z;} ProjectileVector;
typedef struct ProjectileInfo {
    enum eWeaponType m_eWeaponType;
    CEntity *m_pSource;
    uint32_t m_nExplosionTime;
    bool m_bInUse;
    ProjectileVector m_vecPos;
} ProjectileInfo;
#ifdef __cplusplus
extern "C" {
#endif
extern ProjectileInfo ProjectileInfo_records[NUM_PROJECTILES];
extern CProjectile *ProjectileInfo_projectiles[NUM_PROJECTILES];
#ifdef SQUEEZE_PERFORMANCE
extern uint32_t projectileInUse;
#endif
ProjectileInfo *ProjectileInfo_GetProjectileInfo(int32_t id);
void ProjectileInfo_Initialise(void);
void ProjectileInfo_Shutdown(void);
bool ProjectileInfo_AddProjectile(CEntity *entity,enum eWeaponType weapon,const ProjectilePosition *position,float speed);
void ProjectileInfo_RemoveProjectile(ProjectileInfo *info,CProjectile *projectile);
void ProjectileInfo_RemoveNotAdd(CEntity *entity,enum eWeaponType weapon,const ProjectilePosition *position);
bool ProjectileInfo_RemoveIfThisIsAProjectile(CObject *object);
void ProjectileInfo_RemoveAllProjectiles(void);
void ProjectileInfo_RemoveDetonatorProjectiles(void);
void ProjectileInfo_Update(void);
bool ProjectileInfo_IsProjectileInRange(float x1,float x2,float y1,float y2,float z1,float z2,bool remove);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BOUNDARIES=[
 ('ProjectileVector','ReadVector','const void *position','const CVector &v=*(const CVector *)position;return ProjectileVector{v.x,v.y,v.z};'),
 ('void','WithMatrix','void (*operation)(void *,void *),void *context','CMatrix matrix;operation(&matrix,context);'),
 ('int32_t','Model','CEntity *entity','return entity->GetModelIndex();'),
 ('void','EntityMatrix','void *matrix,CEntity *entity','*(CMatrix *)matrix=entity->GetMatrix();'),
 ('void','MatrixPosition','void *matrix,ProjectileVector position','((CMatrix *)matrix)->GetPosition()=CVector(position.x,position.y,position.z);'),
 ('void','MatrixTranslate','void *matrix,float x,float y,float z','((CMatrix *)matrix)->SetTranslate(x,y,z);'),
 ('void','MatrixRotate','void *matrix,float x,float y,float z','((CMatrix *)matrix)->Rotate(x,y,z);'),
 ('void','MatrixRotateZ','void *matrix,float z','((CMatrix *)matrix)->RotateZ(z);'),
 ('void','MatrixTranslateBy','void *matrix,ProjectileVector position','((CMatrix *)matrix)->GetPosition()+=CVector(position.x,position.y,position.z);'),
 ('void','CameraForward','void *matrix','((CMatrix *)matrix)->GetForward()=TheCamera.Cams[TheCamera.ActiveCam].Front;'),
 ('void','CameraUp','void *matrix','((CMatrix *)matrix)->GetUp()=TheCamera.Cams[TheCamera.ActiveCam].Up;'),
 ('void','CameraRight','void *matrix','((CMatrix *)matrix)->GetRight()=CrossProduct(TheCamera.Cams[TheCamera.ActiveCam].Up,TheCamera.Cams[TheCamera.ActiveCam].Front);'),
 ('ProjectileVector','MatrixVelocity','void *matrix,float velocity','CVector v=Multiply3x3(*(CMatrix *)matrix,CVector(0.0f,velocity,0.0f));return ProjectileVector{v.x,v.y,v.z};'),
 ('ProjectileVector','MoveSpeed','CEntity *entity','const CVector &v=((CPhysical *)entity)->m_vecMoveSpeed;return ProjectileVector{v.x,v.y,v.z};'),
 ('ProjectileVector','Forward','CEntity *entity','const CVector &v=entity->GetForward();return ProjectileVector{v.x,v.y,v.z};'),
 ('ProjectileVector','Up','CEntity *entity','const CVector &v=entity->GetUp();return ProjectileVector{v.x,v.y,v.z};'),
 ('bool','IsPlayer','CEntity *entity','return ((CPed *)entity)->IsPlayer();'),
 ('bool','IsPed','CEntity *entity','return entity->IsPed();'),
 ('bool','IsVehicle','CEntity *entity','return entity->IsVehicle();'),
 ('bool','PointerValid','CEntity *entity','return ((CPed *)entity)->IsPointerValid();'),
 ('CEntity *','SeekTarget','CEntity *entity','return ((CPed *)entity)->m_pSeekTarget;'),
 ('const void *','EntityPosition','CEntity *entity','return &entity->GetPosition();'),
 ('const void *','Position','CProjectile *projectile','return &projectile->GetPosition();'),
 ('CPool *','ObjectPool','void','return CPools::GetObjectPool();'),
 ('int8_t','CreatedBy','CObject *object','return object->ObjectCreatedBy;'),
 ('void','DestroyObject','CObject *object','object->~CObject();'),
 ('void','Construct','CProjectile *projectile,int32_t model','std::allocator<CProjectile>().construct(projectile,model);'),
 ('void','Destroy','CProjectile *projectile','projectile->~CProjectile();'),
 ('void','SetMatrix','CProjectile *projectile,void *matrix','projectile->GetMatrix()=*(CMatrix *)matrix;'),
 ('void','SetSpeed','CProjectile *projectile,ProjectileVector speed','projectile->SetMoveSpeed(CVector(speed.x,speed.y,speed.z));'),
 ('void','Gravity','CProjectile *projectile,bool gravity','projectile->bAffectedByGravity=gravity;'),
 ('float *','Elasticity','CProjectile *projectile','return &projectile->m_fElasticity;'),
 ('void','CollisionResponse','CProjectile *projectile,int8_t response','projectile->m_nSpecialCollisionResponseCases=response;'),
 ('void','Add','CProjectile *projectile','CWorld::Add(projectile);'),
 ('void','Remove','CProjectile *projectile','CWorld::Remove(projectile);'),
 ('void','RemoveObject','CObject *object','CWorld::Remove(object);'),
 ('CEntity *','Colliding','CEntity *entity','return ((CPed *)entity)->m_pCollidingEntity;'),
 ('void','SetColliding','CEntity *entity,CProjectile *projectile','((CPed *)entity)->m_pCollidingEntity=projectile;'),
 ('CEntity *','Driver','CEntity *entity','return ((CVehicle *)entity)->pDriver;'),
 ('void','Explosion','CEntity *source,enum eExplosionType type,ProjectileVector position','CExplosion::AddExplosion(nil,source,type,CVector(position.x,position.y,position.z),0);'),
 ('void','ProjectileExplosion','CEntity *source,enum eExplosionType type,CProjectile *projectile','CExplosion::AddExplosion(nil,source,type,projectile->GetPosition(),0);'),
 ('ProjectileVector','ProjectileSpeed','CProjectile *projectile','const CVector &v=projectile->m_vecMoveSpeed;return ProjectileVector{v.x,v.y,v.z};'),
 ('void','ScaleSpeed','CProjectile *projectile,float scale','projectile->m_vecMoveSpeed*=scale;'),
 ('bool','Collided','CProjectile *projectile','return projectile->bHasCollided;'),
 ('void','Ignore','CProjectile *projectile','CWorld::pIgnoreEntity=projectile;'),
 ('bool','LineClear','ProjectileVector from,ProjectileVector to','return CWorld::GetIsLineOfSightClear(CVector(from.x,from.y,from.z),CVector(to.x,to.y,to.z),true,true,true,true,false,false);'),
 ('bool','HeliCollision','ProjectileVector *position','CVector v(position->x,position->y,position->z);bool result=CHeli::TestRocketCollision(&v);*position=ProjectileVector{v.x,v.y,v.z};return result;'),
 ('bool','PlaneCollision','ProjectileVector *position','CVector v(position->x,position->y,position->z);bool result=CPlane::TestRocketCollision(&v);*position=ProjectileVector{v.x,v.y,v.z};return result;'),
 ('void','TearGas','const void *position,ProjectileVector velocity','CParticle::AddParticle(PARTICLE_TEARGAS,*(const CVector *)position,CVector(velocity.x,velocity.y,velocity.z),0,0.0f,0,0,0,0);'),
 ('void','Smoke','ProjectileVector from,ProjectileVector to','CParticle::AddParticlesAlongLine(PARTICLE_ROCKET_SMOKE,CVector(from.x,from.y,from.z),CVector(to.x,to.y,to.z),CVector(0.0f,0.0f,0.0f),0.7f,0,0,0,3000);'),
 ('void','Choking','float x,float y,float z,CEntity *source','CWorld::SetPedsChoking(x,y,z,6.0f,source);'),
 ('enum eWeaponType','DetonatorType','CEntity *entity','CPed *ped=(CPed *)entity;return ped->GetWeapon(ped->GetWeaponSlot(WEAPONTYPE_DETONATOR)).m_eWeaponType;'),
 ('uint32_t','DetonatorAmmo','CEntity *entity','CPed *ped=(CPed *)entity;return ped->GetWeapon(ped->GetWeaponSlot(WEAPONTYPE_DETONATOR)).m_nAmmoTotal;'),
 ('bool','SameObject','CProjectile *projectile,CObject *object','return projectile==object;'),
 ('void','Debug','const char *message','debug("%s",message);'),
 ('void','Undefined','void','Error("Undefined projectile type, AddProjectile, ProjectileInfo.cpp");')]
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "ProjectileInfo.h"
#include "ExplosionTypes.h"
#include "Pool.h"
enum {PROJECTILE_COLLRESPONSE_NONE=0,PROJECTILE_COLLRESPONSE_UNKNOWN5=5,PROJECTILE_TEMP_OBJECT=3};
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(ret+' Projectile_Backend'+name+'('+args+');\n' for ret,name,args,body in BOUNDARIES)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Projectile_backend.h"
#include "Camera.h"
#include "Ped.h"
#include "Projectile.h"
#include "Explosion.h"
#include "World.h"
#include "Particle.h"
#include "Heli.h"
#include "Plane.h"
#include "Weapon.h"
#include "Pools.h"
static_assert(PROJECTILE_COLLRESPONSE_NONE==COLLRESPONSE_NONE && PROJECTILE_COLLRESPONSE_UNKNOWN5==COLLRESPONSE_UNKNOWN5 && PROJECTILE_TEMP_OBJECT==TEMP_OBJECT,"Original projectile collision and pool constants");
'''+''.join(ret+' Projectile_Backend'+name+'('+args+')\n{\n    // Access the original native operation or canonical field without moving controller decisions\n    '+body+'\n}\n' for ret,name,args,body in BOUNDARIES)+'//- rouz edit (ChatGPT)\n'

def kernel(source):
    # Translate every complete original method, retaining branch order and native object lifetimes
    chunks=[]
    for name in METHODS:
        text=function(source,'CProjectileInfo::',name)
        text=re.sub(r'(?m)^\s*//[+-] rouz edit \(ChatGPT\)\n','\n',text).replace(' // rouz edit (ChatGPT)','')
        text=text.replace('CProjectileInfo::','ProjectileInfo_').replace('CProjectileInfo*','ProjectileInfo*').replace('CProjectileInfo *','ProjectileInfo *')
        text=text.replace('gaProjectileInfo','ProjectileInfo_records').replace('ms_apProjectile','ProjectileInfo_projectiles')
        text=text.replace('debug(', 'Projectile_BackendDebug(').replace('Error("Undefined projectile type, AddProjectile, ProjectileInfo.cpp")','Projectile_BackendUndefined()')
        for method in METHODS:text=re.sub(r'(?<![\w])'+method+r'\(', 'ProjectileInfo_'+method+'(',text)
        text=text.replace('CPed* ped = (CPed*)entity','CEntity* ped = entity').replace('CPed *ped = (CPed*)ProjectileInfo_records[i].m_pSource','CEntity *ped = ProjectileInfo_records[i].m_pSource').replace('CPed *ped = (CPed*)ent','CEntity *ped = ent')
        text=re.sub(r'\beWeaponType\b','enum eWeaponType',text).replace('ARRAY_SIZE(','PROJECTILE_ARRAY_SIZE(').replace('nil','NULL').replace('CObjectPool *objectPool','CPool *objectPool')
        text=text.replace('CPools::GetObjectPool()','Projectile_BackendObjectPool()')
        text=text.replace('entity->GetModelIndex()','Projectile_BackendModel(entity)').replace('ped->IsPlayer()','Projectile_BackendIsPlayer(ped)').replace('((CPed*)ped)->IsPlayer()','Projectile_BackendIsPlayer(ped)')
        text=text.replace('ped->m_pSeekTarget','Projectile_BackendSeekTarget(ped)')
        text=text.replace('matrix = ped->GetMatrix()','Projectile_BackendEntityMatrix(matrix,ped)').replace('matrix.GetPosition() = pos','Projectile_BackendMatrixPosition(matrix,pos)')
        for axis in ['Forward','Up','Right']:text=text.replace('matrix.Get'+axis+'() = '+('CrossProduct(TheCamera.Cams[TheCamera.ActiveCam].Up, TheCamera.Cams[TheCamera.ActiveCam].Front)' if axis=='Right' else 'TheCamera.Cams[TheCamera.ActiveCam].'+('Front' if axis=='Forward' else 'Up')),'Projectile_BackendCamera'+axis+'(matrix)')
        text=text.replace('((CPhysical*)entity)->m_vecMoveSpeed','Projectile_BackendMoveSpeed(entity)')
        text=text.replace('Projectile_BackendSeekTarget(ped)->GetPosition().z','PV(Projectile_BackendEntityPosition(Projectile_BackendSeekTarget(ped))).z')
        for entity in ['entity','ped']:
            for axis in ['Forward','Up']:text=text.replace(entity+'->Get'+axis+'()','Projectile_Backend'+axis+'('+entity+')')
        text=text.replace('DotProduct(','Projectile_Dot(').replace('Multiply3x3(matrix, CVector(0.0f, vy, 0.0f))','Projectile_BackendMatrixVelocity(matrix,vy)')
        text=text.replace('matrix.SetTranslate(','Projectile_BackendMatrixTranslate(matrix,').replace('matrix.RotateZ(','Projectile_BackendMatrixRotateZ(matrix,').replace('matrix.Rotate(','Projectile_BackendMatrixRotate(matrix,').replace('matrix.GetPosition() += pos','Projectile_BackendMatrixTranslateBy(matrix,pos)')
        text=text.replace('existing->ObjectCreatedBy','Projectile_BackendCreatedBy(existing)').replace('TEMP_OBJECT','PROJECTILE_TEMP_OBJECT').replace('CWorld::Remove(existing)','Projectile_BackendRemoveObject(existing)').replace('existing->~CObject()','Projectile_BackendDestroyObject(existing)')
        text=text.replace('std::allocator<CProjectile>().construct(ProjectileInfo_projectiles[i], projectileModel)','Projectile_BackendConstruct(ProjectileInfo_projectiles[i],projectileModel)')
        text=text.replace('ProjectileInfo_projectiles[i]->GetMatrix() = matrix','Projectile_BackendSetMatrix(ProjectileInfo_projectiles[i],matrix)').replace('ProjectileInfo_projectiles[i]->SetMoveSpeed(velocity)','Projectile_BackendSetSpeed(ProjectileInfo_projectiles[i],velocity)').replace('ProjectileInfo_projectiles[i]->bAffectedByGravity = gravity','Projectile_BackendGravity(ProjectileInfo_projectiles[i],gravity)')
        text=text.replace('ProjectileInfo_projectiles[i]->m_fElasticity','(*Projectile_BackendElasticity(ProjectileInfo_projectiles[i]))').replace('ProjectileInfo_projectiles[i]->m_nSpecialCollisionResponseCases = SpecialCollisionResponseCase','Projectile_BackendCollisionResponse(ProjectileInfo_projectiles[i],SpecialCollisionResponseCase)')
        text=text.replace('CWorld::Add(ProjectileInfo_projectiles[i])','Projectile_BackendAdd(ProjectileInfo_projectiles[i])')
        text=text.replace('entity->IsPed()','Projectile_BackendIsPed(entity)').replace('ped->IsPed()','Projectile_BackendIsPed(ped)').replace('ent->IsPed()','Projectile_BackendIsPed(ent)').replace('ped->IsPointerValid()','Projectile_BackendPointerValid(ped)')
        text=text.replace('ped->m_pCollidingEntity = ProjectileInfo_projectiles[i]','Projectile_BackendSetColliding(ped,ProjectileInfo_projectiles[i])').replace('ped->m_pCollidingEntity','Projectile_BackendColliding(ped)')
        text=text.replace('info->m_pSource->IsVehicle()','Projectile_BackendIsVehicle(info->m_pSource)').replace('((CVehicle*)info->m_pSource)->pDriver','Projectile_BackendDriver(info->m_pSource)')
        text=re.sub(r'CExplosion::AddExplosion\(NULL, (.*?), (EXPLOSION_\w+), projectile->GetPosition\(\), 0\)',r'Projectile_BackendProjectileExplosion(\1,\2,projectile)',text)
        text=re.sub(r'CExplosion::AddExplosion\(NULL, (.*?), (EXPLOSION_\w+), (pos|ProjectileInfo_records\[i\].m_vecPos), 0\)',r'Projectile_BackendExplosion(\1,\2,\3)',text)
        for p in ['projectile','ProjectileInfo_projectiles[i]']:
            text=text.replace('CWorld::Remove('+p+')','Projectile_BackendRemove('+p+')').replace(p+'->~CProjectile()','Projectile_BackendDestroy('+p+')')
        text=text.replace('const CVector &projectilePos = ProjectileInfo_projectiles[i]->GetPosition()','const void *projectilePos = Projectile_BackendPosition(ProjectileInfo_projectiles[i])')
        text=text.replace('const CVector &pos = ProjectileInfo_projectiles[i]->GetPosition()','ProjectileVector pos = PV(Projectile_BackendPosition(ProjectileInfo_projectiles[i]))')
        text=text.replace('ProjectileInfo_projectiles[i]->GetPosition()','PV(Projectile_BackendPosition(ProjectileInfo_projectiles[i]))')
        text=text.replace('ProjectileInfo_projectiles[i]->m_vecMoveSpeed *= Pow(1.07f, Timer_GetTimeStepFix())','Projectile_BackendScaleSpeed(ProjectileInfo_projectiles[i],powf(1.07f,Timer_GetTimeStepFix()))')
        text=text.replace('ProjectileInfo_projectiles[i]->m_vecMoveSpeed','Projectile_BackendProjectileSpeed(ProjectileInfo_projectiles[i])')
        text=text.replace('(*Timer_GetTimeStep()) * Projectile_BackendProjectileSpeed(ProjectileInfo_projectiles[i]) + projectilePos','Projectile_Add(Projectile_ScaleLeft((*Timer_GetTimeStep()),Projectile_BackendProjectileSpeed(ProjectileInfo_projectiles[i])),PV(projectilePos))')
        for axis in 'xyz':text=text.replace('projectilePos.'+axis,'PV(projectilePos).'+axis)
        text=re.sub(r'CParticle::AddParticle\(PARTICLE_TEARGAS, projectilePos, CVector\((.*?)\), 0, 0.0f, 0, 0, 0, 0\)',r'Projectile_BackendTearGas(projectilePos,Projectile_Make(\1))',text)
        text=text.replace('CParticle::AddParticlesAlongLine(PARTICLE_ROCKET_SMOKE, smokeStart, smokeEnd, CVector(0.0f, 0.0f, 0.0f), 0.7f, 0, 0, 0, 3000)','Projectile_BackendSmoke(smokeStart,smokeEnd)')
        text=text.replace('CWorld::SetPedsChoking(PV(projectilePos).x, PV(projectilePos).y, PV(projectilePos).z, 6.0f, ProjectileInfo_records[i].m_pSource)','Projectile_BackendChoking(PV(projectilePos).x,PV(projectilePos).y,PV(projectilePos).z,ProjectileInfo_records[i].m_pSource)')
        text=text.replace('(projectilePos - rocketSmokePreviousPos[i]) * (1.0f / smokeFrames)','Projectile_Scale(Projectile_Sub(PV(projectilePos),rocketSmokePreviousPos[i]),(1.0f / smokeFrames))').replace('smokeStart + smokeStep','Projectile_Add(smokeStart,smokeStep)').replace('rocketSmokePreviousPos[i] = projectilePos','rocketSmokePreviousPos[i] = PV(projectilePos)')
        text=text.replace('CWorld::pIgnoreEntity = ProjectileInfo_projectiles[i]','Projectile_BackendIgnore(ProjectileInfo_projectiles[i])').replace('CWorld::pIgnoreEntity = NULL','Projectile_BackendIgnore(NULL)')
        text=text.replace('ProjectileInfo_projectiles[i]->bHasCollided','Projectile_BackendCollided(ProjectileInfo_projectiles[i])')
        text=text.replace('CWorld::GetIsLineOfSightClear(ProjectileInfo_records[i].m_vecPos, pos, true, true, true, true, false, false)','Projectile_BackendLineClear(ProjectileInfo_records[i].m_vecPos,pos)').replace('CHeli::TestRocketCollision(&pos)','Projectile_BackendHeliCollision(&pos)').replace('CPlane::TestRocketCollision(&pos)','Projectile_BackendPlaneCollision(&pos)')
        text=text.replace('((ProjectileInfo_records[i].m_vecPos - ProjectileInfo_records[i].m_pSource->GetPosition()).MagnitudeSqr() >= 2.0f)','(Projectile_MagnitudeSqr(Projectile_Sub(ProjectileInfo_records[i].m_vecPos,PV(Projectile_BackendEntityPosition(ProjectileInfo_records[i].m_pSource)))) >= 2.0f)')
        text=text.replace('ped->GetWeapon(ped->GetWeaponSlot(WEAPONTYPE_DETONATOR)).m_eWeaponType','Projectile_BackendDetonatorType(ped)').replace('ped->GetWeapon(ped->GetWeaponSlot(WEAPONTYPE_DETONATOR)).m_nAmmoTotal','Projectile_BackendDetonatorAmmo(ped)')
        text=text.replace('ProjectileInfo_projectiles[i++] != object','!Projectile_BackendSameObject(ProjectileInfo_projectiles[i++],object)')
        text=text.replace('COLLRESPONSE_NONE','PROJECTILE_COLLRESPONSE_NONE').replace('COLLRESPONSE_UNKNOWN5','PROJECTILE_COLLRESPONSE_UNKNOWN5')
        text=text.replace('CVector','ProjectileVector').replace('Atan2(','atan2f(').replace('Sin(','sinf(').replace('Cos(','cosf(').replace('Abs(','fabsf(')
        for old,new in [('uint32','uint32_t'),('int32','int32_t'),('int8','int8_t')]:text=re.sub(r'\b'+old+r'\b',new,text)
        text=text.replace('()\n','(void)\n')
        if name=='AddProjectile':
            signature='bool\nProjectileInfo_AddProjectile(CEntity *entity, enum eWeaponType weapon, ProjectileVector pos, float speed)'
            assert signature in text
            text=text.replace(signature,'static void Projectile_AddWithMatrix(void *matrix,void *context)')
            text=text.replace('\n\tCMatrix matrix;','')
            text=text.replace('\n{','\n{\n    // Keep the native matrix alive across every original creation branch and early return\n    ProjectileAddContext *args=(ProjectileAddContext *)context;\n    CEntity *entity=args->entity;enum eWeaponType weapon=args->weapon;ProjectileVector pos=args->position;float speed=args->speed;',1)
            text=text.replace('return false;','{args->result=false;return;}').replace('return true;','args->result=true;')
            chunks.append(text)
            chunks.append('''bool ProjectileInfo_AddProjectile(CEntity *entity,enum eWeaponType weapon,const ProjectilePosition *position,float speed)
{
    // Capture the original by-value position before constructing the scoped native matrix
    ProjectileAddContext args={entity,weapon,Projectile_BackendReadVector(position),speed,false};
    Projectile_BackendWithMatrix(Projectile_AddWithMatrix,&args);
    return args.result;
}''')
            continue
        if name=='RemoveNotAdd':
            text=text.replace('enum eWeaponType weaponType, ProjectileVector pos','enum eWeaponType weaponType, const ProjectilePosition *position')
            text=text.replace('\n{','\n{\n    // Capture the original position value before dispatching the unchanged explosion choice\n    ProjectileVector pos=Projectile_BackendReadVector(position);',1)
        else:text=text.replace('\n{','\n{\n    // Execute the complete original projectile '+name+' operation',1)
        chunks.append(text)
    return '''//+ rouz edit (ChatGPT)
#include "config.h"
#undef assert
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "Projectile_backend.h"
#include "Timer.h"
#include "General.h"
#include "ModelIndexData.h"
#include <math.h>
#ifndef FIX_BUGS
static inline float Timer_GetDefaultTimeStep(void)
{
    // Retain the existing timer helper when its public declaration is disabled
    return 50.0f / 30.0f;
}
static inline float Timer_GetTimeStepFix(void)
{
    // Preserve the same normalization used by the unconditional projectile smoke code
    return (*Timer_GetTimeStep()) / Timer_GetDefaultTimeStep();
}
#endif
#define PROJECTILE_ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
#define PV(value) Projectile_BackendReadVector(value)
#define Max(a,b) ((a)>(b)?(a):(b))
#define PROJECTILE_BOUNDARY_MIN_X -2390.0f
#define PROJECTILE_BOUNDARY_MAX_X 1590.0f
#define PROJECTILE_BOUNDARY_MIN_Y -1990.0f
#define PROJECTILE_BOUNDARY_MAX_Y 1990.0f
#ifdef SQUEEZE_PERFORMANCE
uint32_t projectileInUse;
#endif
ProjectileInfo ProjectileInfo_records[NUM_PROJECTILES];
CProjectile *ProjectileInfo_projectiles[NUM_PROJECTILES];
typedef struct ProjectileAddContext {CEntity *entity;enum eWeaponType weapon;ProjectileVector position;float speed;bool result;} ProjectileAddContext;
static ProjectileVector Projectile_Make(float x,float y,float z)
{
    // Materialize an original three-component vector value
    ProjectileVector result={x,y,z};return result;
}
static ProjectileVector Projectile_Add(ProjectileVector a,ProjectileVector b)
{
    // Preserve the ordered component additions
    return Projectile_Make(a.x+b.x,a.y+b.y,a.z+b.z);
}
static ProjectileVector Projectile_Sub(ProjectileVector a,ProjectileVector b)
{
    // Preserve the ordered component subtractions
    return Projectile_Make(a.x-b.x,a.y-b.y,a.z-b.z);
}
static ProjectileVector Projectile_Scale(ProjectileVector v,float scale)
{
    // Preserve vector-left multiplication for smoke interpolation
    return Projectile_Make(v.x*scale,v.y*scale,v.z*scale);
}
static ProjectileVector Projectile_ScaleLeft(float scale,ProjectileVector v)
{
    // Preserve scalar-left multiplication for predicted positions
    return Projectile_Make(scale*v.x,scale*v.y,scale*v.z);
}
static float Projectile_Dot(ProjectileVector a,ProjectileVector b)
{
    // Keep the original three-term scalar-product evaluation order
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static float Projectile_MagnitudeSqr(ProjectileVector v)
{
    // Preserve the original distance comparison without a square root
    return v.x*v.x+v.y*v.y+v.z*v.z;
}
'''+ '\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def arguments(source,opening):
    # Locate complete caller arguments without splitting nested vector expressions
    parts=[];start=opening+1;depth=1;i=start
    while depth:
        if source[i] in '([{':depth+=1
        elif source[i] in ')]}':depth-=1
        if source[i]==',' and depth==1:parts.append(source[start:i]);start=i+1
        i+=1
    parts.append(source[start:i-1]);return i,parts

def projectile_calls(source):
    # Replace the complete class and every caller while preserving replay arrays and position captures
    if 'class CProjectileInfo\n' in source:return HEADER
    if re.search(r'(?m)^CProjectileInfo gaProjectileInfo\[NUM_PROJECTILES\];',source):
        if OBSERVER:OBSERVER(source)
        return HOST
    original=source;changes=[]
    for match in re.finditer(r'CProjectileInfo::(?:AddProjectile|RemoveNotAdd)\(',source):
        end,args=arguments(source,source.index('(',match.start()));assert len(args) in [3,4]
        args[2]=' &('+args[2].strip()+')';changes.append((match.start(),end,source[match.start():source.index('(',match.start())+1]+','.join(args)+')'))
    for start,end,text in reversed(changes):source=source[:start]+text+source[end:]
    source=source.replace('CProjectileInfo::ms_apProjectile','ProjectileInfo_projectiles').replace('CProjectileInfo::','ProjectileInfo_').replace('gaProjectileInfo','ProjectileInfo_records')
    source=re.sub(r'\bCProjectileInfo\b','ProjectileInfo',source)
    if source==original:return source
    if original.lstrip().startswith('//+ rouz edit (ChatGPT)') and original.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Forward complete historical callers through the separately verified gang class and constructor removal
    return gangs.calls(projectile_calls(source))
def project(source):
    # Register a uniquely named controller unit and its typed native boundary header
    for tag,old,new in [('ClCompile','ProjectileInfo.cpp','ProjectileControl.c'),('ClInclude','ProjectileInfo.h','Projectile_backend.h')]:
        entry='<'+tag+' Include="..\\src\\weapons\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\weapons\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\weapons\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\weapons\\'+re.escape(old)+r'">\n\s*<Filter>weapons</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return gangs.project(source)

def verify_legacy(source,transform,root):
    # Prove the complete relocated controller and native bridge from each earlier immutable source
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/weapons/ProjectileInfo.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/weapons/ProjectileControl.c').read_text())

def verify_dependency(saved,current):
    # Keep byte guards unless the complete dependency matches the exact projectile migration
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original)
    assert expected!=original and expected==current.read_text(),str(current)

def legacy_member_spelling(source):
    # Restore known getter and pointer-array spellings while older audio body translators bind their hosts
    return source.replace('ProjectileInfo_GetProjectileInfo(','CProjectileInfo::GetProjectileInfo(').replace('ProjectileInfo_projectiles','CProjectileInfo::ms_apProjectile')
