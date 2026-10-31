//+ rouz edit (ChatGPT)
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
