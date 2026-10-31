//+ rouz edit (ChatGPT)
#pragma once
#include "ProjectileInfo.h"
#include "ExplosionTypes.h"
#include "Pool.h"
enum {PROJECTILE_COLLRESPONSE_NONE=0,PROJECTILE_COLLRESPONSE_UNKNOWN5=5,PROJECTILE_TEMP_OBJECT=3};
#ifdef __cplusplus
extern "C" {
#endif
ProjectileVector Projectile_BackendReadVector(const void *position);
void Projectile_BackendWithMatrix(void (*operation)(void *,void *),void *context);
int32_t Projectile_BackendModel(CEntity *entity);
void Projectile_BackendEntityMatrix(void *matrix,CEntity *entity);
void Projectile_BackendMatrixPosition(void *matrix,ProjectileVector position);
void Projectile_BackendMatrixTranslate(void *matrix,float x,float y,float z);
void Projectile_BackendMatrixRotate(void *matrix,float x,float y,float z);
void Projectile_BackendMatrixRotateZ(void *matrix,float z);
void Projectile_BackendMatrixTranslateBy(void *matrix,ProjectileVector position);
void Projectile_BackendCameraForward(void *matrix);
void Projectile_BackendCameraUp(void *matrix);
void Projectile_BackendCameraRight(void *matrix);
ProjectileVector Projectile_BackendMatrixVelocity(void *matrix,float velocity);
ProjectileVector Projectile_BackendMoveSpeed(CEntity *entity);
ProjectileVector Projectile_BackendForward(CEntity *entity);
ProjectileVector Projectile_BackendUp(CEntity *entity);
bool Projectile_BackendIsPlayer(CEntity *entity);
bool Projectile_BackendIsPed(CEntity *entity);
bool Projectile_BackendIsVehicle(CEntity *entity);
bool Projectile_BackendPointerValid(CEntity *entity);
CEntity * Projectile_BackendSeekTarget(CEntity *entity);
const void * Projectile_BackendEntityPosition(CEntity *entity);
const void * Projectile_BackendPosition(CProjectile *projectile);
CPool * Projectile_BackendObjectPool(void);
int8_t Projectile_BackendCreatedBy(CObject *object);
void Projectile_BackendDestroyObject(CObject *object);
void Projectile_BackendConstruct(CProjectile *projectile,int32_t model);
void Projectile_BackendDestroy(CProjectile *projectile);
void Projectile_BackendSetMatrix(CProjectile *projectile,void *matrix);
void Projectile_BackendSetSpeed(CProjectile *projectile,ProjectileVector speed);
void Projectile_BackendGravity(CProjectile *projectile,bool gravity);
float * Projectile_BackendElasticity(CProjectile *projectile);
void Projectile_BackendCollisionResponse(CProjectile *projectile,int8_t response);
void Projectile_BackendAdd(CProjectile *projectile);
void Projectile_BackendRemove(CProjectile *projectile);
void Projectile_BackendRemoveObject(CObject *object);
CEntity * Projectile_BackendColliding(CEntity *entity);
void Projectile_BackendSetColliding(CEntity *entity,CProjectile *projectile);
CEntity * Projectile_BackendDriver(CEntity *entity);
void Projectile_BackendExplosion(CEntity *source,enum eExplosionType type,ProjectileVector position);
void Projectile_BackendProjectileExplosion(CEntity *source,enum eExplosionType type,CProjectile *projectile);
ProjectileVector Projectile_BackendProjectileSpeed(CProjectile *projectile);
void Projectile_BackendScaleSpeed(CProjectile *projectile,float scale);
bool Projectile_BackendCollided(CProjectile *projectile);
void Projectile_BackendIgnore(CProjectile *projectile);
bool Projectile_BackendLineClear(ProjectileVector from,ProjectileVector to);
bool Projectile_BackendHeliCollision(ProjectileVector *position);
bool Projectile_BackendPlaneCollision(ProjectileVector *position);
void Projectile_BackendTearGas(const void *position,ProjectileVector velocity);
void Projectile_BackendSmoke(ProjectileVector from,ProjectileVector to);
void Projectile_BackendChoking(float x,float y,float z,CEntity *source);
enum eWeaponType Projectile_BackendDetonatorType(CEntity *entity);
uint32_t Projectile_BackendDetonatorAmmo(CEntity *entity);
bool Projectile_BackendSameObject(CProjectile *projectile,CObject *object);
void Projectile_BackendDebug(const char *message);
void Projectile_BackendUndefined(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
