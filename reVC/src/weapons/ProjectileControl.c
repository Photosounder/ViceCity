//+ rouz edit (ChatGPT)
#include "config.h"
#undef assert
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "Projectile_backend.h"
#include "Explosion_access.h"
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
void
ProjectileInfo_Initialise(void)
{
    // Execute the complete original projectile Initialise operation
	Projectile_BackendDebug("Initialising CProjectileInfo...\n");

	for (int i = 0; i < PROJECTILE_ARRAY_SIZE(ProjectileInfo_projectiles); i++) {
		ProjectileInfo_projectiles[i] = NULL;
		ProjectileInfo_records[i].m_eWeaponType = WEAPONTYPE_GRENADE;
		ProjectileInfo_records[i].m_pSource = NULL;
		ProjectileInfo_records[i].m_nExplosionTime = 0;
		ProjectileInfo_records[i].m_bInUse = false;
	}

	Projectile_BackendDebug("CProjectileInfo ready\n");

#ifdef SQUEEZE_PERFORMANCE
	projectileInUse = 0;
#endif
}

void
ProjectileInfo_Shutdown(void)
{
    // Execute the complete original projectile Shutdown operation
	Projectile_BackendDebug("Shutting down CProjectileInfo...\n");
	Projectile_BackendDebug("CProjectileInfo shut down\n");
}

ProjectileInfo*
ProjectileInfo_GetProjectileInfo(int32_t id)
{
    // Execute the complete original projectile GetProjectileInfo operation
	return &ProjectileInfo_records[id];
}

static void Projectile_AddWithMatrix(void *matrix,void *context)
{
    // Keep the native matrix alive across every original creation branch and early return
    ProjectileAddContext *args=(ProjectileAddContext *)context;
    CEntity *entity=args->entity;enum eWeaponType weapon=args->weapon;ProjectileVector pos=args->position;float speed=args->speed;
	int8_t SpecialCollisionResponseCase = PROJECTILE_COLLRESPONSE_NONE;
	bool gravity = true;
	float elasticity = 0.75f;
	CEntity* ped = entity;
	int time;
	ProjectileVector velocity;

	switch (weapon)
	{
		case WEAPONTYPE_ROCKET:
		{
			float vy = 0.35f;
			time = (*Timer_GetTimeInMilliseconds()) + 2000;
			if (Projectile_BackendModel(entity) == MI_SPARROW || Projectile_BackendModel(entity) == MI_HUNTER || Projectile_BackendModel(entity) == MI_SENTINEL) {
				Projectile_BackendEntityMatrix(matrix,ped);
				Projectile_BackendMatrixPosition(matrix,pos);
				ProjectileVector vecSpeed = Projectile_BackendMoveSpeed(entity);
				vy += Max(0.0f, Projectile_Dot(vecSpeed, Projectile_BackendForward(entity))) + Max(0.0f, Projectile_Dot(vecSpeed, Projectile_BackendUp(entity)));
			} else {
				if (Projectile_BackendIsPlayer(ped)) {
					Projectile_BackendCameraForward(matrix);
					Projectile_BackendCameraUp(matrix);
					Projectile_BackendCameraRight(matrix);
					Projectile_BackendMatrixPosition(matrix,pos);
					// rouz edit, added velocity so that we can fire rockets while on a moving vehicle
					ProjectileVector vecSpeed = Projectile_BackendMoveSpeed(entity);
					vy += Max(0.0f, Projectile_Dot(vecSpeed, Projectile_BackendForward(entity))) + Max(0.0f, Projectile_Dot(vecSpeed, Projectile_BackendUp(entity)));
				} else if (Projectile_BackendSeekTarget(ped) != NULL) {
					float ry = General_GetRadianAngleBetweenPoints(1.0f, PV(Projectile_BackendEntityPosition(Projectile_BackendSeekTarget(ped))).z, 1.0f, pos.z);
					float rz = atan2f(-Projectile_BackendForward(ped).x, Projectile_BackendForward(ped).y);
					vy = 0.35f * speed + 0.15f;
					Projectile_BackendMatrixTranslate(matrix,0.0f, 1.0f, 1.0f);
					Projectile_BackendMatrixRotate(matrix,0.0f, ry, rz);
					Projectile_BackendMatrixTranslateBy(matrix,pos);
				} else {
					Projectile_BackendEntityMatrix(matrix,ped);
				}
			}
			velocity = Projectile_BackendMatrixVelocity(matrix,vy);
			gravity = false;
			break;
		}
		case WEAPONTYPE_MOLOTOV:
		{
			time = (*Timer_GetTimeInMilliseconds()) + 2000;
			float scale = 0.22f * speed + 0.15f;
			if (scale < 0.2f)
				scale = 0.2f;
			float angle = atan2f(-Projectile_BackendForward(ped).x, Projectile_BackendForward(ped).y);
			Projectile_BackendMatrixTranslate(matrix,0.0f, 0.0f, 0.0f);
			Projectile_BackendMatrixRotateZ(matrix,angle);
			Projectile_BackendMatrixTranslateBy(matrix,pos);
			velocity.x = -1.0f * scale * sinf(angle);
			velocity.y = scale * cosf(angle);
			velocity.z = (0.2f * speed + 0.4f) * scale;
			break;
		}
		case WEAPONTYPE_TEARGAS:
		{
			time = (*Timer_GetTimeInMilliseconds()) + 20000;
			float scale = 0.0f;
			if (speed != 0.0f)
				scale = 0.22f * speed + 0.15f;
			float angle = atan2f(-Projectile_BackendForward(ped).x, Projectile_BackendForward(ped).y);
			Projectile_BackendMatrixTranslate(matrix,0.0f, 0.0f, 0.0f);
			Projectile_BackendMatrixRotateZ(matrix,angle);
			Projectile_BackendMatrixTranslateBy(matrix,pos);
			SpecialCollisionResponseCase = PROJECTILE_COLLRESPONSE_UNKNOWN5;
			velocity.x = -1.0f * scale * sinf(angle);
			velocity.y = scale * cosf(angle);
			velocity.z = (0.4f * speed + 0.4f) * scale;
			elasticity = 0.5f;
			break;
		}
		case WEAPONTYPE_GRENADE:
		case WEAPONTYPE_DETONATOR_GRENADE:
		{
			time = (*Timer_GetTimeInMilliseconds()) + 2000;
			float scale = 0.0f;
			if (speed != 0.0f)
				scale = 0.22f * speed + 0.15f;
			float angle = atan2f(-Projectile_BackendForward(ped).x, Projectile_BackendForward(ped).y);
			Projectile_BackendMatrixTranslate(matrix,0.0f, 0.0f, 0.0f);
			Projectile_BackendMatrixRotateZ(matrix,angle);
			Projectile_BackendMatrixTranslateBy(matrix,pos);
			SpecialCollisionResponseCase = PROJECTILE_COLLRESPONSE_UNKNOWN5;
			velocity.x = -1.0f * scale * sinf(angle);
			velocity.y = scale * cosf(angle);
			velocity.z = (0.4f * speed + 0.4f) * scale;
			elasticity = 0.5f;
			break;
		}
		default:
		Projectile_BackendUndefined();
		break;
	}

	int i = 0;
#ifdef FIX_BUGS
	while (i < PROJECTILE_ARRAY_SIZE(ProjectileInfo_records) && ProjectileInfo_records[i].m_bInUse) i++;
#else
	// array overrun is UB
	while (ProjectileInfo_records[i].m_bInUse && i < PROJECTILE_ARRAY_SIZE(ProjectileInfo_records)) i++;
#endif
	if (i == PROJECTILE_ARRAY_SIZE(ProjectileInfo_records))
		{args->result=false;return;}

	int32_t projectileModel = -1;
	switch (weapon)
	{
		case WEAPONTYPE_ROCKET:
		projectileModel = MI_MISSILE;
		break;
		case WEAPONTYPE_TEARGAS:
		projectileModel = MI_TEARGAS;
		break;
		case WEAPONTYPE_MOLOTOV:
		projectileModel = MI_MOLOTOV;
		break;
		case WEAPONTYPE_GRENADE:
		case WEAPONTYPE_DETONATOR_GRENADE:
		projectileModel = MI_GRENADE;
		break;
		default: break;
	}

	// Allocate the projectile from the object pool without invoking C++ new.
	if (projectileModel != -1) {
		CPool *objectPool = Projectile_BackendObjectPool();

		// Access raw storage through the C store or pool API
		ProjectileInfo_projectiles[i] = (CProjectile*)((CObject*)CPool_New(objectPool));

#ifdef FIX_BUGS
		if (!ProjectileInfo_projectiles[i]) {

			// Access raw storage through the C store or pool API
			for (int32_t j = 0; j < CPool_GetSize(objectPool); j++) {
				// Access raw storage through the C store or pool API
				CObject *existing = ((CObject*)CPool_GetSlot(objectPool, j));

				if (existing && Projectile_BackendCreatedBy(existing) == PROJECTILE_TEMP_OBJECT) {

					// Access raw storage through the C store or pool API
					int32_t handle = CPool_GetIndex(objectPool, existing);

					Projectile_BackendRemoveObject(existing);
					Projectile_BackendDestroyObject(existing);

					// Access raw storage through the C store or pool API
					CPool_Delete(objectPool, existing);
					ProjectileInfo_projectiles[i] = (CProjectile*)((CObject*)CPool_NewAt(objectPool, handle));

					break;
				}
			}
		}
		if (ProjectileInfo_projectiles[i])
#endif
			Projectile_BackendConstruct(ProjectileInfo_projectiles[i],projectileModel);
	}


	if (ProjectileInfo_projectiles[i] == NULL)
		{args->result=false;return;}

	ProjectileInfo_records[i].m_eWeaponType = weapon;
	ProjectileInfo_records[i].m_pSource = ped;
	Projectile_BackendSetMatrix(ProjectileInfo_projectiles[i],matrix);
	Projectile_BackendSetSpeed(ProjectileInfo_projectiles[i],velocity);
	Projectile_BackendGravity(ProjectileInfo_projectiles[i],gravity);

	ProjectileInfo_records[i].m_nExplosionTime = time;
	(*Projectile_BackendElasticity(ProjectileInfo_projectiles[i])) = elasticity;
	Projectile_BackendCollisionResponse(ProjectileInfo_projectiles[i],SpecialCollisionResponseCase);

#ifdef SQUEEZE_PERFORMANCE
	projectileInUse++;
#endif

	ProjectileInfo_records[i].m_bInUse = true;
	Projectile_BackendAdd(ProjectileInfo_projectiles[i]);

	ProjectileInfo_records[i].m_vecPos = PV(Projectile_BackendPosition(ProjectileInfo_projectiles[i]));

	if (entity && Projectile_BackendIsPed(entity) && !Projectile_BackendColliding(ped)) {
		Projectile_BackendSetColliding(ped,ProjectileInfo_projectiles[i]);
	}
	args->result=true;
}

bool ProjectileInfo_AddProjectile(CEntity *entity,enum eWeaponType weapon,const ProjectilePosition *position,float speed)
{
    // Capture the original by-value position before constructing the scoped native matrix
    ProjectileAddContext args={entity,weapon,Projectile_BackendReadVector(position),speed,false};
    Projectile_BackendWithMatrix(Projectile_AddWithMatrix,&args);
    return args.result;
}

void
ProjectileInfo_RemoveProjectile(ProjectileInfo *info, CProjectile *projectile)
{
    // Execute the complete original projectile RemoveProjectile operation
	// TODO(Miami): New parameter: 1
	switch (info->m_eWeaponType) {
		case WEAPONTYPE_GRENADE:
			Projectile_BackendProjectileExplosion(info->m_pSource,EXPLOSION_GRENADE,projectile);
			break;
		case WEAPONTYPE_MOLOTOV:
			Projectile_BackendProjectileExplosion(info->m_pSource,EXPLOSION_MOLOTOV,projectile);
			break;
		case WEAPONTYPE_ROCKET:
			Projectile_BackendProjectileExplosion(Projectile_BackendIsVehicle(info->m_pSource) ? Projectile_BackendDriver(info->m_pSource) : info->m_pSource,EXPLOSION_ROCKET,projectile);
			break;
	}
#ifdef SQUEEZE_PERFORMANCE
	projectileInUse--;
#endif

	info->m_bInUse = false;
	Projectile_BackendRemove(projectile);

	// Destroy and release the projectile without invoking C++ delete.
	Projectile_BackendDestroy(projectile);

	// Access raw storage through the C store or pool API
	CPool_Delete(Projectile_BackendObjectPool(), projectile);


}

static void Projectile_ExplosionAt(CEntity *source,enum eExplosionType type,ProjectileVector position)
{
    // Forward the projectile position by value to the explosion subsystem
    ExplosionPosition point={position.x,position.y,position.z};
    Explosion_AddAtPosition(source,type,point);
}

void
ProjectileInfo_RemoveNotAdd(CEntity *entity, enum eWeaponType weaponType, const ProjectilePosition *position)
{
    // Capture the original position value before dispatching the unchanged explosion choice
    ProjectileVector pos=Projectile_BackendReadVector(position);
	// TODO(Miami): New parameter: 1
	switch (weaponType) {
		case WEAPONTYPE_GRENADE:
			Projectile_ExplosionAt(entity,EXPLOSION_GRENADE,pos);
			break;
		case WEAPONTYPE_MOLOTOV:
			Projectile_ExplosionAt(entity,EXPLOSION_MOLOTOV,pos);
			break;
		case WEAPONTYPE_ROCKET:
			Projectile_ExplosionAt(entity,EXPLOSION_ROCKET,pos);
			break;
	}
}

void
ProjectileInfo_Update(void)
{
    // Execute the complete original projectile Update operation
#ifdef SQUEEZE_PERFORMANCE
	if (projectileInUse == 0)
		return;
#endif

	int tearGasOffset = -0.0f; // unused

	static float rocketSmokeFrameCounter[NUM_PROJECTILES];
	static uint32_t rocketSmokeExpirationTime[NUM_PROJECTILES];
	static CProjectile *rocketSmokeProjectile[NUM_PROJECTILES];
	static ProjectileVector rocketSmokePreviousPos[NUM_PROJECTILES];


	for (int i = 0; i < PROJECTILE_ARRAY_SIZE(ProjectileInfo_records); i++) {
		if (!ProjectileInfo_records[i].m_bInUse) {

			rocketSmokeFrameCounter[i] = 0.0f;
			rocketSmokeExpirationTime[i] = 0;
			rocketSmokeProjectile[i] = NULL;

			continue;
		}

		CEntity *ped = ProjectileInfo_records[i].m_pSource;
		if (ped != NULL && Projectile_BackendIsPed(ped) && !Projectile_BackendPointerValid(ped))
			ProjectileInfo_records[i].m_pSource = NULL;

		if (ProjectileInfo_projectiles[i] == NULL) {
#ifdef SQUEEZE_PERFORMANCE
			projectileInUse--;
#endif

			ProjectileInfo_records[i].m_bInUse = false;
			continue;
		}
		if ( (ProjectileInfo_records[i].m_eWeaponType == WEAPONTYPE_DETONATOR_GRENADE || ProjectileInfo_records[i].m_eWeaponType == WEAPONTYPE_TEARGAS) && (*Projectile_BackendElasticity(ProjectileInfo_projectiles[i])) > 0.1f ) {
			if ( fabsf(Projectile_BackendProjectileSpeed(ProjectileInfo_projectiles[i]).x) < 0.05f && fabsf(Projectile_BackendProjectileSpeed(ProjectileInfo_projectiles[i]).y) < 0.05f && fabsf(Projectile_BackendProjectileSpeed(ProjectileInfo_projectiles[i]).z) < 0.05f ) {
				(*Projectile_BackendElasticity(ProjectileInfo_projectiles[i])) = 0.03f;
			}
		}
		const void *projectilePos = Projectile_BackendPosition(ProjectileInfo_projectiles[i]);
		ProjectileVector nextPos = Projectile_Add(Projectile_ScaleLeft((*Timer_GetTimeStep()),Projectile_BackendProjectileSpeed(ProjectileInfo_projectiles[i])),PV(projectilePos));

		if ( nextPos.x <= PROJECTILE_BOUNDARY_MIN_X || nextPos.x >= PROJECTILE_BOUNDARY_MAX_X || nextPos.y <= PROJECTILE_BOUNDARY_MIN_Y || nextPos.y >= PROJECTILE_BOUNDARY_MAX_Y ) {
			// Not RemoveProjectile, because we don't want no explosion

			rocketSmokeFrameCounter[i] = 0.0f;
			rocketSmokeExpirationTime[i] = 0;
			rocketSmokeProjectile[i] = NULL;

			ProjectileInfo_records[i].m_bInUse = false;
			Projectile_BackendRemove(ProjectileInfo_projectiles[i]);

			// Destroy and release the projectile without invoking C++ delete.
			Projectile_BackendDestroy(ProjectileInfo_projectiles[i]);

			// Access raw storage through the C store or pool API
			CPool_Delete(Projectile_BackendObjectPool(), ProjectileInfo_projectiles[i]);


        	continue;
		}
		if ( ProjectileInfo_records[i].m_eWeaponType == WEAPONTYPE_TEARGAS && (*Timer_GetTimeInMilliseconds()) > ProjectileInfo_records[i].m_nExplosionTime - 19500 ) {
			Projectile_BackendTearGas(projectilePos,Projectile_Make(0.2f, tearGasOffset, 0.0f));
			Projectile_BackendTearGas(projectilePos,Projectile_Make(-0.2f, tearGasOffset, 0.0f));
			Projectile_BackendTearGas(projectilePos,Projectile_Make(tearGasOffset, tearGasOffset, 0.0f));

			if ( (*Timer_GetTimeInMilliseconds()) & 0x200 )
				Projectile_BackendChoking(PV(projectilePos).x,PV(projectilePos).y,PV(projectilePos).z,ProjectileInfo_records[i].m_pSource);
		}

		if (ProjectileInfo_records[i].m_eWeaponType == WEAPONTYPE_ROCKET) {

			if (rocketSmokeProjectile[i] != ProjectileInfo_projectiles[i] || rocketSmokeExpirationTime[i] != ProjectileInfo_records[i].m_nExplosionTime) {
				rocketSmokeFrameCounter[i] = 0.0f;
				rocketSmokeExpirationTime[i] = ProjectileInfo_records[i].m_nExplosionTime;
				rocketSmokeProjectile[i] = ProjectileInfo_projectiles[i];
				rocketSmokePreviousPos[i] = ProjectileInfo_records[i].m_vecPos;
			}

			rocketSmokeFrameCounter[i] += Timer_GetTimeStepFix();
			if (rocketSmokeFrameCounter[i] >= 1.0f) {
				int32_t smokeFrames = (int32_t)rocketSmokeFrameCounter[i];
				ProjectileVector smokeStart = rocketSmokePreviousPos[i];
				ProjectileVector smokeStep = Projectile_Scale(Projectile_Sub(PV(projectilePos),rocketSmokePreviousPos[i]),(1.0f / smokeFrames));

				for (int32_t smokeFrame = 0; smokeFrame < smokeFrames; smokeFrame++) {
					ProjectileVector smokeEnd = Projectile_Add(smokeStart,smokeStep);
					Projectile_BackendSmoke(smokeStart,smokeEnd);
					smokeStart = smokeEnd;
				}

				rocketSmokePreviousPos[i] = PV(projectilePos);
				rocketSmokeFrameCounter[i] -= smokeFrames;
			}

		}

		if ((*Timer_GetTimeInMilliseconds()) <= ProjectileInfo_records[i].m_nExplosionTime || ProjectileInfo_records[i].m_nExplosionTime == 0) {
			if (ProjectileInfo_records[i].m_eWeaponType == WEAPONTYPE_ROCKET) {
				ProjectileVector pos = PV(Projectile_BackendPosition(ProjectileInfo_projectiles[i]));
				Projectile_BackendIgnore(ProjectileInfo_projectiles[i]);

				bool removeProjectile = Projectile_BackendCollided(ProjectileInfo_projectiles[i])
					|| !Projectile_BackendLineClear(ProjectileInfo_records[i].m_vecPos,pos)
					|| Projectile_BackendHeliCollision(&pos) || Projectile_BackendPlaneCollision(&pos);
				if (removeProjectile) {
					ProjectileInfo_RemoveProjectile(&ProjectileInfo_records[i], ProjectileInfo_projectiles[i]);
					rocketSmokeFrameCounter[i] = 0.0f;
					rocketSmokeExpirationTime[i] = 0;
					rocketSmokeProjectile[i] = NULL;
					Projectile_BackendIgnore(NULL);
					continue;
				}
				Projectile_BackendIgnore(NULL);
				Projectile_BackendScaleSpeed(ProjectileInfo_projectiles[i],powf(1.07f,Timer_GetTimeStepFix()));


			} else if (ProjectileInfo_records[i].m_eWeaponType == WEAPONTYPE_MOLOTOV) {
				ProjectileVector pos = PV(Projectile_BackendPosition(ProjectileInfo_projectiles[i]));
				Projectile_BackendIgnore(ProjectileInfo_projectiles[i]);

				if (ProjectileInfo_records[i].m_pSource == NULL 
					|| (Projectile_MagnitudeSqr(Projectile_Sub(ProjectileInfo_records[i].m_vecPos,PV(Projectile_BackendEntityPosition(ProjectileInfo_records[i].m_pSource)))) >= 2.0f))
				{
					if (Projectile_BackendCollided(ProjectileInfo_projectiles[i])
						|| !Projectile_BackendLineClear(ProjectileInfo_records[i].m_vecPos,pos)) {
						ProjectileInfo_RemoveProjectile(&ProjectileInfo_records[i], ProjectileInfo_projectiles[i]);
					}
				}
				Projectile_BackendIgnore(NULL);
			}
		} else {
			if (ProjectileInfo_records[i].m_eWeaponType == WEAPONTYPE_DETONATOR_GRENADE) {
				CEntity *ent = ProjectileInfo_records[i].m_pSource;
				if (Projectile_BackendIsPed(ent) && Projectile_BackendIsPlayer(ped)) {
					CEntity *ped = ent;
					if (Projectile_BackendDetonatorType(ped) != WEAPONTYPE_DETONATOR
						|| Projectile_BackendDetonatorAmmo(ped) == 0) {
						ProjectileInfo_records[i].m_nExplosionTime = 0;
					}
				}
			} else {
				ProjectileInfo_RemoveProjectile(&ProjectileInfo_records[i], ProjectileInfo_projectiles[i]);
			}
		}

		if (!ProjectileInfo_records[i].m_bInUse)
			continue;


		ProjectileInfo_records[i].m_vecPos = PV(Projectile_BackendPosition(ProjectileInfo_projectiles[i]));
	}
}

bool
ProjectileInfo_IsProjectileInRange(float x1, float x2, float y1, float y2, float z1, float z2, bool remove)
{
    // Execute the complete original projectile IsProjectileInRange operation
	bool result = false;
	for (int i = 0; i < PROJECTILE_ARRAY_SIZE(ProjectileInfo_projectiles); i++) {
		if (ProjectileInfo_records[i].m_bInUse) {
			if (ProjectileInfo_records[i].m_eWeaponType == WEAPONTYPE_ROCKET || ProjectileInfo_records[i].m_eWeaponType == WEAPONTYPE_MOLOTOV || ProjectileInfo_records[i].m_eWeaponType == WEAPONTYPE_GRENADE) {
				ProjectileVector pos = PV(Projectile_BackendPosition(ProjectileInfo_projectiles[i]));
				if (pos.x >= x1 && pos.x <= x2 && pos.y >= y1 && pos.y <= y2 && pos.z >= z1 && pos.z <= z2) {
					result = true;
					if (remove) {
#ifdef SQUEEZE_PERFORMANCE
						projectileInUse--;
#endif

						ProjectileInfo_records[i].m_bInUse = false;
						Projectile_BackendRemove(ProjectileInfo_projectiles[i]);

						// Destroy and release the projectile without invoking C++ delete.
						Projectile_BackendDestroy(ProjectileInfo_projectiles[i]);

						// Access raw storage through the C store or pool API
						CPool_Delete(Projectile_BackendObjectPool(), ProjectileInfo_projectiles[i]);


					}
				}
			}
		}
	}
	return result;
}

void
ProjectileInfo_RemoveDetonatorProjectiles(void)
{
    // Execute the complete original projectile RemoveDetonatorProjectiles operation
	for (int i = 0; i < PROJECTILE_ARRAY_SIZE(ProjectileInfo_projectiles); i++) {
		if (ProjectileInfo_records[i].m_bInUse && ProjectileInfo_records[i].m_eWeaponType == WEAPONTYPE_DETONATOR_GRENADE) {
			Projectile_ExplosionAt(ProjectileInfo_records[i].m_pSource,EXPLOSION_GRENADE,ProjectileInfo_records[i].m_vecPos); // TODO(Miami): New parameter: 1
			ProjectileInfo_records[i].m_bInUse = false;
			Projectile_BackendRemove(ProjectileInfo_projectiles[i]);

			// Destroy and release the projectile without invoking C++ delete.
			Projectile_BackendDestroy(ProjectileInfo_projectiles[i]);

			// Access raw storage through the C store or pool API
			CPool_Delete(Projectile_BackendObjectPool(), ProjectileInfo_projectiles[i]);


		}
	}
}

void
ProjectileInfo_RemoveAllProjectiles(void)
{
    // Execute the complete original projectile RemoveAllProjectiles operation
#ifdef SQUEEZE_PERFORMANCE
	if (projectileInUse == 0)
		return;
#endif

	for (int i = 0; i < PROJECTILE_ARRAY_SIZE(ProjectileInfo_projectiles); i++) {
		if (ProjectileInfo_records[i].m_bInUse) {
#ifdef SQUEEZE_PERFORMANCE
			projectileInUse--;
#endif

			ProjectileInfo_records[i].m_bInUse = false;
			Projectile_BackendRemove(ProjectileInfo_projectiles[i]);

			// Destroy and release the projectile without invoking C++ delete.
			Projectile_BackendDestroy(ProjectileInfo_projectiles[i]);

			// Access raw storage through the C store or pool API
			CPool_Delete(Projectile_BackendObjectPool(), ProjectileInfo_projectiles[i]);


		}
	}
}

bool
ProjectileInfo_RemoveIfThisIsAProjectile(CObject *object)
{
    // Execute the complete original projectile RemoveIfThisIsAProjectile operation
#ifdef SQUEEZE_PERFORMANCE
	if (projectileInUse == 0)
		return false;
#endif

	int i = 0;
	while (!Projectile_BackendSameObject(ProjectileInfo_projectiles[i++],object)) {
		if (i >= PROJECTILE_ARRAY_SIZE(ProjectileInfo_projectiles))
			return false;
	}

#ifdef SQUEEZE_PERFORMANCE
	projectileInUse--;
#endif

	ProjectileInfo_records[i].m_bInUse = false;
	Projectile_BackendRemove(ProjectileInfo_projectiles[i]);

	// Destroy and release the projectile without invoking C++ delete.
	Projectile_BackendDestroy(ProjectileInfo_projectiles[i]);

	// Access raw storage through the C store or pool API
	CPool_Delete(Projectile_BackendObjectPool(), ProjectileInfo_projectiles[i]);


	ProjectileInfo_projectiles[i] = NULL;
	return true;
}
//- rouz edit (ChatGPT)
