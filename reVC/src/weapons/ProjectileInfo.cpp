//+ rouz edit (ChatGPT)
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
ProjectileVector Projectile_BackendReadVector(const void *position)
{
    // Access the original native operation or canonical field without moving controller decisions
    const CVector &v=*(const CVector *)position;return ProjectileVector{v.x,v.y,v.z};
}
void Projectile_BackendWithMatrix(void (*operation)(void *,void *),void *context)
{
    // Access the original native operation or canonical field without moving controller decisions
    CMatrix matrix;operation(&matrix,context);
}
int32_t Projectile_BackendModel(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    return entity->GetModelIndex();
}
void Projectile_BackendEntityMatrix(void *matrix,CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    *(CMatrix *)matrix=entity->GetMatrix();
}
void Projectile_BackendMatrixPosition(void *matrix,ProjectileVector position)
{
    // Access the original native operation or canonical field without moving controller decisions
    ((CMatrix *)matrix)->GetPosition()=CVector(position.x,position.y,position.z);
}
void Projectile_BackendMatrixTranslate(void *matrix,float x,float y,float z)
{
    // Access the original native operation or canonical field without moving controller decisions
    ((CMatrix *)matrix)->SetTranslate(x,y,z);
}
void Projectile_BackendMatrixRotate(void *matrix,float x,float y,float z)
{
    // Access the original native operation or canonical field without moving controller decisions
    ((CMatrix *)matrix)->Rotate(x,y,z);
}
void Projectile_BackendMatrixRotateZ(void *matrix,float z)
{
    // Access the original native operation or canonical field without moving controller decisions
    ((CMatrix *)matrix)->RotateZ(z);
}
void Projectile_BackendMatrixTranslateBy(void *matrix,ProjectileVector position)
{
    // Access the original native operation or canonical field without moving controller decisions
    ((CMatrix *)matrix)->GetPosition()+=CVector(position.x,position.y,position.z);
}
void Projectile_BackendCameraForward(void *matrix)
{
    // Access the original native operation or canonical field without moving controller decisions
    ((CMatrix *)matrix)->GetForward()=TheCamera.Cams[TheCamera.ActiveCam].Front;
}
void Projectile_BackendCameraUp(void *matrix)
{
    // Access the original native operation or canonical field without moving controller decisions
    ((CMatrix *)matrix)->GetUp()=TheCamera.Cams[TheCamera.ActiveCam].Up;
}
void Projectile_BackendCameraRight(void *matrix)
{
    // Access the original native operation or canonical field without moving controller decisions
    ((CMatrix *)matrix)->GetRight()=CrossProduct(TheCamera.Cams[TheCamera.ActiveCam].Up,TheCamera.Cams[TheCamera.ActiveCam].Front);
}
ProjectileVector Projectile_BackendMatrixVelocity(void *matrix,float velocity)
{
    // Access the original native operation or canonical field without moving controller decisions
    CVector v=Multiply3x3(*(CMatrix *)matrix,CVector(0.0f,velocity,0.0f));return ProjectileVector{v.x,v.y,v.z};
}
ProjectileVector Projectile_BackendMoveSpeed(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    const CVector &v=((CPhysical *)entity)->m_vecMoveSpeed;return ProjectileVector{v.x,v.y,v.z};
}
ProjectileVector Projectile_BackendForward(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    const CVector &v=entity->GetForward();return ProjectileVector{v.x,v.y,v.z};
}
ProjectileVector Projectile_BackendUp(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    const CVector &v=entity->GetUp();return ProjectileVector{v.x,v.y,v.z};
}
bool Projectile_BackendIsPlayer(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    return ((CPed *)entity)->IsPlayer();
}
bool Projectile_BackendIsPed(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    return entity->IsPed();
}
bool Projectile_BackendIsVehicle(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    return entity->IsVehicle();
}
bool Projectile_BackendPointerValid(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    return ((CPed *)entity)->IsPointerValid();
}
CEntity * Projectile_BackendSeekTarget(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    return ((CPed *)entity)->m_pSeekTarget;
}
const void * Projectile_BackendEntityPosition(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    return &entity->GetPosition();
}
const void * Projectile_BackendPosition(CProjectile *projectile)
{
    // Access the original native operation or canonical field without moving controller decisions
    return &projectile->GetPosition();
}
CPool * Projectile_BackendObjectPool(void)
{
    // Access the original native operation or canonical field without moving controller decisions
    return CPools::GetObjectPool();
}
int8_t Projectile_BackendCreatedBy(CObject *object)
{
    // Access the original native operation or canonical field without moving controller decisions
    return object->ObjectCreatedBy;
}
void Projectile_BackendDestroyObject(CObject *object)
{
    // Access the original native operation or canonical field without moving controller decisions
    object->~CObject();
}
void Projectile_BackendConstruct(CProjectile *projectile,int32_t model)
{
    // Access the original native operation or canonical field without moving controller decisions
    std::allocator<CProjectile>().construct(projectile,model);
}
void Projectile_BackendDestroy(CProjectile *projectile)
{
    // Access the original native operation or canonical field without moving controller decisions
    projectile->~CProjectile();
}
void Projectile_BackendSetMatrix(CProjectile *projectile,void *matrix)
{
    // Access the original native operation or canonical field without moving controller decisions
    projectile->GetMatrix()=*(CMatrix *)matrix;
}
void Projectile_BackendSetSpeed(CProjectile *projectile,ProjectileVector speed)
{
    // Access the original native operation or canonical field without moving controller decisions
    projectile->SetMoveSpeed(CVector(speed.x,speed.y,speed.z));
}
void Projectile_BackendGravity(CProjectile *projectile,bool gravity)
{
    // Access the original native operation or canonical field without moving controller decisions
    projectile->bAffectedByGravity=gravity;
}
float * Projectile_BackendElasticity(CProjectile *projectile)
{
    // Access the original native operation or canonical field without moving controller decisions
    return &projectile->m_fElasticity;
}
void Projectile_BackendCollisionResponse(CProjectile *projectile,int8_t response)
{
    // Access the original native operation or canonical field without moving controller decisions
    projectile->m_nSpecialCollisionResponseCases=response;
}
void Projectile_BackendAdd(CProjectile *projectile)
{
    // Access the original native operation or canonical field without moving controller decisions
    CWorld::Add(projectile);
}
void Projectile_BackendRemove(CProjectile *projectile)
{
    // Access the original native operation or canonical field without moving controller decisions
    CWorld::Remove(projectile);
}
void Projectile_BackendRemoveObject(CObject *object)
{
    // Access the original native operation or canonical field without moving controller decisions
    CWorld::Remove(object);
}
CEntity * Projectile_BackendColliding(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    return ((CPed *)entity)->m_pCollidingEntity;
}
void Projectile_BackendSetColliding(CEntity *entity,CProjectile *projectile)
{
    // Access the original native operation or canonical field without moving controller decisions
    ((CPed *)entity)->m_pCollidingEntity=projectile;
}
CEntity * Projectile_BackendDriver(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    return ((CVehicle *)entity)->pDriver;
}
void Projectile_BackendExplosion(CEntity *source,enum eExplosionType type,ProjectileVector position)
{
    // Access the original native operation or canonical field without moving controller decisions
    CExplosion::AddExplosion(nil,source,type,CVector(position.x,position.y,position.z),0);
}
void Projectile_BackendProjectileExplosion(CEntity *source,enum eExplosionType type,CProjectile *projectile)
{
    // Access the original native operation or canonical field without moving controller decisions
    CExplosion::AddExplosion(nil,source,type,projectile->GetPosition(),0);
}
ProjectileVector Projectile_BackendProjectileSpeed(CProjectile *projectile)
{
    // Access the original native operation or canonical field without moving controller decisions
    const CVector &v=projectile->m_vecMoveSpeed;return ProjectileVector{v.x,v.y,v.z};
}
void Projectile_BackendScaleSpeed(CProjectile *projectile,float scale)
{
    // Access the original native operation or canonical field without moving controller decisions
    projectile->m_vecMoveSpeed*=scale;
}
bool Projectile_BackendCollided(CProjectile *projectile)
{
    // Access the original native operation or canonical field without moving controller decisions
    return projectile->bHasCollided;
}
void Projectile_BackendIgnore(CProjectile *projectile)
{
    // Access the original native operation or canonical field without moving controller decisions
    CWorld::pIgnoreEntity=projectile;
}
bool Projectile_BackendLineClear(ProjectileVector from,ProjectileVector to)
{
    // Access the original native operation or canonical field without moving controller decisions
    return CWorld::GetIsLineOfSightClear(CVector(from.x,from.y,from.z),CVector(to.x,to.y,to.z),true,true,true,true,false,false);
}
bool Projectile_BackendHeliCollision(ProjectileVector *position)
{
    // Access the original native operation or canonical field without moving controller decisions
    CVector v(position->x,position->y,position->z);bool result=CHeli::TestRocketCollision(&v);*position=ProjectileVector{v.x,v.y,v.z};return result;
}
bool Projectile_BackendPlaneCollision(ProjectileVector *position)
{
    // Access the original native operation or canonical field without moving controller decisions
    CVector v(position->x,position->y,position->z);bool result=CPlane::TestRocketCollision(&v);*position=ProjectileVector{v.x,v.y,v.z};return result;
}
void Projectile_BackendTearGas(const void *position,ProjectileVector velocity)
{
    // Access the original native operation or canonical field without moving controller decisions
    CParticle::AddParticle(PARTICLE_TEARGAS,*(const CVector *)position,CVector(velocity.x,velocity.y,velocity.z),0,0.0f,0,0,0,0);
}
void Projectile_BackendSmoke(ProjectileVector from,ProjectileVector to)
{
    // Access the original native operation or canonical field without moving controller decisions
    CParticle::AddParticlesAlongLine(PARTICLE_ROCKET_SMOKE,CVector(from.x,from.y,from.z),CVector(to.x,to.y,to.z),CVector(0.0f,0.0f,0.0f),0.7f,0,0,0,3000);
}
void Projectile_BackendChoking(float x,float y,float z,CEntity *source)
{
    // Access the original native operation or canonical field without moving controller decisions
    CWorld::SetPedsChoking(x,y,z,6.0f,source);
}
enum eWeaponType Projectile_BackendDetonatorType(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    CPed *ped=(CPed *)entity;return ped->GetWeapon(ped->GetWeaponSlot(WEAPONTYPE_DETONATOR)).m_eWeaponType;
}
uint32_t Projectile_BackendDetonatorAmmo(CEntity *entity)
{
    // Access the original native operation or canonical field without moving controller decisions
    CPed *ped=(CPed *)entity;return ped->GetWeapon(ped->GetWeaponSlot(WEAPONTYPE_DETONATOR)).m_nAmmoTotal;
}
bool Projectile_BackendSameObject(CProjectile *projectile,CObject *object)
{
    // Access the original native operation or canonical field without moving controller decisions
    return projectile==object;
}
void Projectile_BackendDebug(const char *message)
{
    // Access the original native operation or canonical field without moving controller decisions
    debug("%s",message);
}
void Projectile_BackendUndefined(void)
{
    // Access the original native operation or canonical field without moving controller decisions
    Error("Undefined projectile type, AddProjectile, ProjectileInfo.cpp");
}
//- rouz edit (ChatGPT)
