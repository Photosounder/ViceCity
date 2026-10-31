//+ rouz edit (ChatGPT)
#include "common.h"
#include "Fire_backend.h"
#include "PlayerPed.h"
#include "Vehicle.h"
#include "Automobile.h"
#include "World.h"
#include "Particle.h"
#include "Shadows.h"
#include "EventList.h"
#include "GameLogic.h"
#include "CarAI.h"
FireVector Fire_VectorFromNative(const CVector &position)
{
    // Copy one native value into the plain C position packet at its original value boundary
    FireVector result={position.x,position.y,position.z};return result;
}
CVector Fire_VectorToNative(FireVector position)
{
    // Supply a native vector value to the still-native caller or external engine operation
    return CVector(position.x,position.y,position.z);
}
FireVector Fire_BackendReadVector(const void *position)
{
    // Enter the original typed native operation or canonical field at the C decision point
    const CVector &v=*(const CVector *)position;return FireVector{v.x,v.y,v.z};
}
FireVector Fire_BackendPosition(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return Fire_VectorFromNative(entity->GetPosition());
}
bool Fire_BackendIsPed(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return entity->IsPed();
}
bool Fire_BackendIsVehicle(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return entity->IsVehicle();
}
bool Fire_BackendIsCar(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CVehicle *)entity)->IsCar();
}
CFire * Fire_BackendPedFire(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CPed *)entity)->m_pFire;
}
CFire * Fire_BackendVehicleFire(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CVehicle *)entity)->m_pCarFire;
}
void Fire_BackendSetPedFire(CEntity *entity,CFire *fire)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->m_pFire=fire;
}
void Fire_BackendSetVehicleFire(CEntity *entity,CFire *fire)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CVehicle *)entity)->m_pCarFire=fire;
}
int32_t Fire_BackendMoveState(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CPed *)entity)->m_nMoveState;
}
bool Fire_BackendInVehicle(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CPed *)entity)->bInVehicle;
}
CEntity * Fire_BackendMyVehicle(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CPed *)entity)->m_pMyVehicle;
}
void Fire_BackendVehicleHealth(CEntity *entity,float health)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CVehicle *)entity)->m_fHealth=health;
}
CEntity * Fire_BackendPlayerPed(void)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return FindPlayerPed();
}
CEntity * Fire_BackendPlayerVehicle(void)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return FindPlayerVehicle();
}
bool Fire_BackendPedDamage(CEntity *entity,CEntity *source,float damage)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CPlayerPed *)entity)->InflictDamage((CPlayerPed *)source,WEAPONTYPE_FLAMETHROWER,damage,PEDPIECE_TORSO,0);
}
void Fire_BackendVehicleDamage(CEntity *entity,CEntity *source,float damage)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CVehicle *)entity)->InflictDamage((CVehicle *)source,WEAPONTYPE_FLAMETHROWER,damage);
}
void Fire_BackendScorched(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    entity->bRenderScorched=true;
}
CEntity * Fire_BackendShortcutTaxi(void)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return CGameLogic::pShortCutTaxi;
}
bool Fire_BackendShortcutTransition(void)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return CGameLogic::ShortCutState==CGameLogic::SHORTCUT_TRANSITION;
}
bool Fire_BackendFireProof(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CPed *)entity)->bFireProof;
}
void Fire_BackendPrepareOnFire(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPlayerPed *)entity)->DoStuffToGoOnFire();
}
int32_t Fire_BackendModel(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return entity->GetModelIndex();
}
FireVector Fire_BackendHeadlights(int32_t model)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return Fire_VectorFromNative(((CVehicleModelInfo *)CModelInfo::GetModelInfo(model))->m_positions[CAR_POS_HEADLIGHTS]);
}
FireVector Fire_BackendTransform(CEntity *entity,FireVector position)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return Fire_VectorFromNative(entity->GetMatrix()*Fire_VectorToNative(position));
}
void Fire_BackendFlame(FireVector position,FireVector velocity,float size)
{
    // Enter the original typed native operation or canonical field at the C decision point
    CParticle::AddParticle(PARTICLE_CARFLAME,Fire_VectorToNative(position),Fire_VectorToNative(velocity),0,size,0,0,0,0);
}
void Fire_BackendSmoke(FireVector position)
{
    // Enter the original typed native operation or canonical field at the C decision point
    CParticle::AddParticle(PARTICLE_CARFLAME_SMOKE,Fire_VectorToNative(position),CVector(0.0f,0.0f,0.0f),0,0.0f,0,0,0,0);
}
void Fire_BackendSteam(FireVector position,float upward,float size)
{
    // Enter the original typed native operation or canonical field at the C decision point
    CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION,Fire_VectorToNative(position),CVector(0.0f,0.0f,upward),nil,size);
}
void Fire_BackendShadow(uintptr_t id,FireVector position,int16_t random)
{
    // Enter the original typed native operation or canonical field at the C decision point
    CVector native=Fire_VectorToNative(position);CShadows::StoreStaticShadow(id,SHADOWTYPE_ADDITIVE,gpShadowExplosionTex,&native,7.0f,0.0f,0.0f,-7.0f,0,random/2,random/2,0,10.0f,1.0f,40.0f,0,0.0f);
}
void Fire_BackendReport(FireVector position)
{
    // Enter the original typed native operation or canonical field at the C decision point
    Events_RegisterPosition(EVENT_FIRE,Events_VectorFromNative(Fire_VectorToNative(position)),1000);
}
bool Fire_BackendCanSetState(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CPed *)entity)->CanSetPedState();
}
int32_t Fire_BackendPedState(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CPed *)entity)->m_nPedState;
}
bool Fire_BackendIsPlayer(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CPed *)entity)->IsPlayer();
}
void Fire_BackendIdle(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->SetIdle();
}
void Fire_BackendLastStateNone(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->m_nLastPedState=PED_NONE;
}
void Fire_BackendWander(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->SetWanderPath(0);
}
void Fire_BackendWaitFinish(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->SetWaitState(WAITSTATE_FINISH_FLEE,0);
}
bool Fire_BackendInControl(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CPed *)entity)->IsPedInControl();
}
uint8_t Fire_BackendEngineStatus(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return Damage_GetEngineStatus(&((CAutomobile *)entity)->Damage);
}
void Fire_BackendSetEngineStatus(CEntity *entity,uint8_t status)
{
    // Enter the original typed native operation or canonical field at the C decision point
    Damage_SetEngineStatus(&((CAutomobile *)entity)->Damage, status);
}
void Fire_BackendFleeEntity(CEntity *entity,CEntity *source)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->SetFlee(source,10000);
}
void Fire_BackendFleePosition(CEntity *entity,FireVector position)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->SetFlee(CVector2D(position.x,position.y),10000);
}
void Fire_BackendClearFleeFrom(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->m_fleeFrom=nil;
}
uint32_t * Fire_BackendFleeTimer(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return &((CPed *)entity)->m_fleeTimer;
}
void Fire_BackendDrawLastOff(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->bDrawLast=false;
}
void Fire_BackendSprint(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->SetMoveState(PEDMOVE_SPRINT);
}
void Fire_BackendMoveAnim(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->SetMoveAnim();
}
void Fire_BackendOnFireState(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    ((CPed *)entity)->SetPedState(PED_ON_FIRE);
}
int32_t Fire_BackendPedType(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return ((CPed *)entity)->m_nPedType;
}
void Fire_BackendCopEvent(CEntity *entity,CEntity *source)
{
    // Enter the original typed native operation or canonical field at the C decision point
    Events_RegisterEntity(EVENT_COP_SET_ON_FIRE,EVENT_ENTITY_PED,entity,(CPed *)source,10000);
}
void Fire_BackendPedEvent(CEntity *entity,CEntity *source)
{
    // Enter the original typed native operation or canonical field at the C decision point
    Events_RegisterEntity(EVENT_PED_SET_ON_FIRE,EVENT_ENTITY_PED,entity,(CPed *)source,10000);
}
void Fire_BackendVehicleEvent(CEntity *entity,CEntity *source)
{
    // Enter the original typed native operation or canonical field at the C decision point
    Events_RegisterEntity(EVENT_CAR_SET_ON_FIRE,EVENT_ENTITY_VEHICLE,entity,(CPed *)source,10000);
}
bool Fire_BackendBikeModel(int32_t model)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return CModelInfo::IsBikeModel(model);
}
bool Fire_BackendCarModel(int32_t model)
{
    // Enter the original typed native operation or canonical field at the C decision point
    return CModelInfo::IsCarModel(model);
}
void Fire_BackendFleeOccupants(CEntity *entity)
{
    // Enter the original typed native operation or canonical field at the C decision point
    CCarAI::TellOccupantsToFleeCar((CVehicle *)entity);
}
//- rouz edit (ChatGPT)
