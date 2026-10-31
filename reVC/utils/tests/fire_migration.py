"""Move all persistent fire records and manager/record lifecycle decisions into C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import bridge_migration as bridge
RECORD=['ProcessFire','ReportThisFire','Extinguish']
MANAGER=['StartFireAt','StartFireOn','Update','FindNearestFire','FindFurthestFire_NeverMindFireMen','GetNextFreeFire','GetTotalActiveFires','ExtinguishPoint','ExtinguishPointWithWater','StartScriptFire','IsScriptFireExtinguish','RemoveAllScriptFires','RemoveScriptFire','SetScriptFireAudio']
FIELDS=['m_bIsOngoing','m_bIsScriptFire','m_bPropagationFlag','m_bAudioSet','m_vecPos','m_pEntity','m_pSource','m_nExtinguishTime','m_nStartTime','m_nNextTimeToAddFlames','m_fStrength','m_fWaterExtinguishCountdown','m_bExtinguishedWithWater']
OBSERVER=None
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CEntity;class CVector;
typedef CVector FirePosition;
#else
typedef struct CEntity CEntity;
typedef struct CVector FirePosition;
#endif
typedef struct FireVector {float x,y,z;} FireVector;
typedef struct CFire {
    bool m_bIsOngoing,m_bIsScriptFire,m_bPropagationFlag,m_bAudioSet;
    FireVector m_vecPos;
    CEntity *m_pEntity,*m_pSource;
    uint32_t m_nExtinguishTime,m_nStartTime,m_nNextTimeToAddFlames;
    float m_fStrength,m_fWaterExtinguishCountdown;
    bool m_bExtinguishedWithWater;
} CFire;
typedef struct CFireManager {uint32_t m_nTotalFires;CFire m_aFires[NUM_FIRES];} CFireManager;
enum {FIRE_MAX_FIREMEN_ATTENDING=2};
#ifdef __cplusplus
extern "C" {
#endif
extern CFireManager gFireManager;
void Fire_ProcessFire(CFire *fire);
void Fire_ReportThisFire(CFire *fire);
void Fire_Extinguish(CFire *fire);
void FireManager_StartFireAt(CFireManager *manager,FireVector position,float size,uint8_t propagation);
CFire *FireManager_StartFireOn(CFireManager *manager,CEntity *entity,CEntity *source,float strength,uint8_t propagation);
void FireManager_Update(CFireManager *manager);
CFire *FireManager_FindNearestFire(CFireManager *manager,FireVector position,float *distance);
CFire *FireManager_FindFurthestFire_NeverMindFireMen(CFireManager *manager,FireVector position,float minimum,float maximum);
CFire *FireManager_GetNextFreeFire(CFireManager *manager);
uint32_t FireManager_GetTotalActiveFires(const CFireManager *manager);
void FireManager_ExtinguishPoint(CFireManager *manager,FireVector point,float range);
bool FireManager_ExtinguishPointWithWater(CFireManager *manager,FireVector point,float range);
int32_t FireManager_StartScriptFire(CFireManager *manager,const FirePosition *position,CEntity *target,float strength,uint8_t propagation);
bool FireManager_IsScriptFireExtinguish(CFireManager *manager,int16_t index);
void FireManager_RemoveAllScriptFires(CFireManager *manager);
void FireManager_RemoveScriptFire(CFireManager *manager,int16_t index);
void FireManager_SetScriptFireAudio(CFireManager *manager,int16_t index,bool state);
#ifdef __cplusplus
}
FireVector Fire_VectorFromNative(const CVector &position);
CVector Fire_VectorToNative(FireVector position);
#endif
//- rouz edit (ChatGPT)
'''
BOUNDARIES=[
 ('FireVector','ReadVector','const void *position','const CVector &v=*(const CVector *)position;return FireVector{v.x,v.y,v.z};'),
 ('FireVector','Position','CEntity *entity','return Fire_VectorFromNative(entity->GetPosition());'),
 ('bool','IsPed','CEntity *entity','return entity->IsPed();'),
 ('bool','IsVehicle','CEntity *entity','return entity->IsVehicle();'),
 ('bool','IsCar','CEntity *entity','return ((CVehicle *)entity)->IsCar();'),
 ('CFire *','PedFire','CEntity *entity','return ((CPed *)entity)->m_pFire;'),
 ('CFire *','VehicleFire','CEntity *entity','return ((CVehicle *)entity)->m_pCarFire;'),
 ('void','SetPedFire','CEntity *entity,CFire *fire','((CPed *)entity)->m_pFire=fire;'),
 ('void','SetVehicleFire','CEntity *entity,CFire *fire','((CVehicle *)entity)->m_pCarFire=fire;'),
 ('int32_t','MoveState','CEntity *entity','return ((CPed *)entity)->m_nMoveState;'),
 ('bool','InVehicle','CEntity *entity','return ((CPed *)entity)->bInVehicle;'),
 ('CEntity *','MyVehicle','CEntity *entity','return ((CPed *)entity)->m_pMyVehicle;'),
 ('void','VehicleHealth','CEntity *entity,float health','((CVehicle *)entity)->m_fHealth=health;'),
 ('CEntity *','PlayerPed','void','return FindPlayerPed();'),
 ('CEntity *','PlayerVehicle','void','return FindPlayerVehicle();'),
 ('bool','PedDamage','CEntity *entity,CEntity *source,float damage','return ((CPlayerPed *)entity)->InflictDamage((CPlayerPed *)source,WEAPONTYPE_FLAMETHROWER,damage,PEDPIECE_TORSO,0);'),
 ('void','VehicleDamage','CEntity *entity,CEntity *source,float damage','((CVehicle *)entity)->InflictDamage((CVehicle *)source,WEAPONTYPE_FLAMETHROWER,damage);'),
 ('void','Scorched','CEntity *entity','entity->bRenderScorched=true;'),
 ('CEntity *','ShortcutTaxi','void','return CGameLogic::pShortCutTaxi;'),
 ('bool','ShortcutTransition','void','return CGameLogic::ShortCutState==CGameLogic::SHORTCUT_TRANSITION;'),
 ('bool','FireProof','CEntity *entity','return ((CPed *)entity)->bFireProof;'),
 ('void','PrepareOnFire','CEntity *entity','((CPlayerPed *)entity)->DoStuffToGoOnFire();'),
 ('int32_t','Model','CEntity *entity','return entity->GetModelIndex();'),
 ('FireVector','Headlights','int32_t model','return Fire_VectorFromNative(((CVehicleModelInfo *)CModelInfo::GetModelInfo(model))->m_positions[CAR_POS_HEADLIGHTS]);'),
 ('FireVector','Transform','CEntity *entity,FireVector position','return Fire_VectorFromNative(entity->GetMatrix()*Fire_VectorToNative(position));'),
 ('void','Flame','FireVector position,FireVector velocity,float size','CParticle::AddParticle(PARTICLE_CARFLAME,Fire_VectorToNative(position),Fire_VectorToNative(velocity),0,size,0,0,0,0);'),
 ('void','Smoke','FireVector position','CParticle::AddParticle(PARTICLE_CARFLAME_SMOKE,Fire_VectorToNative(position),CVector(0.0f,0.0f,0.0f),0,0.0f,0,0,0,0);'),
 ('void','Steam','FireVector position,float upward,float size','CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION,Fire_VectorToNative(position),CVector(0.0f,0.0f,upward),nil,size);'),
 ('void','Shadow','uintptr_t id,FireVector position,int16_t random','CVector native=Fire_VectorToNative(position);CShadows::StoreStaticShadow(id,SHADOWTYPE_ADDITIVE,gpShadowExplosionTex,&native,7.0f,0.0f,0.0f,-7.0f,0,random/2,random/2,0,10.0f,1.0f,40.0f,0,0.0f);'),
 ('void','Report','FireVector position','CEventList::RegisterEvent(EVENT_FIRE,Fire_VectorToNative(position),1000);'),
 ('bool','CanSetState','CEntity *entity','return ((CPed *)entity)->CanSetPedState();'),
 ('int32_t','PedState','CEntity *entity','return ((CPed *)entity)->m_nPedState;'),
 ('bool','IsPlayer','CEntity *entity','return ((CPed *)entity)->IsPlayer();'),
 ('void','Idle','CEntity *entity','((CPed *)entity)->SetIdle();'),
 ('void','LastStateNone','CEntity *entity','((CPed *)entity)->m_nLastPedState=PED_NONE;'),
 ('void','Wander','CEntity *entity','((CPed *)entity)->SetWanderPath(0);'),
 ('void','WaitFinish','CEntity *entity','((CPed *)entity)->SetWaitState(WAITSTATE_FINISH_FLEE,0);'),
 ('bool','InControl','CEntity *entity','return ((CPed *)entity)->IsPedInControl();'),
 ('uint8_t','EngineStatus','CEntity *entity','return ((CAutomobile *)entity)->Damage.GetEngineStatus();'),
 ('void','SetEngineStatus','CEntity *entity,uint8_t status','((CAutomobile *)entity)->Damage.SetEngineStatus(status);'),
 ('void','FleeEntity','CEntity *entity,CEntity *source','((CPed *)entity)->SetFlee(source,10000);'),
 ('void','FleePosition','CEntity *entity,FireVector position','((CPed *)entity)->SetFlee(CVector2D(position.x,position.y),10000);'),
 ('void','ClearFleeFrom','CEntity *entity','((CPed *)entity)->m_fleeFrom=nil;'),
 ('uint32_t *','FleeTimer','CEntity *entity','return &((CPed *)entity)->m_fleeTimer;'),
 ('void','DrawLastOff','CEntity *entity','((CPed *)entity)->bDrawLast=false;'),
 ('void','Sprint','CEntity *entity','((CPed *)entity)->SetMoveState(PEDMOVE_SPRINT);'),
 ('void','MoveAnim','CEntity *entity','((CPed *)entity)->SetMoveAnim();'),
 ('void','OnFireState','CEntity *entity','((CPed *)entity)->SetPedState(PED_ON_FIRE);'),
 ('int32_t','PedType','CEntity *entity','return ((CPed *)entity)->m_nPedType;'),
 ('void','CopEvent','CEntity *entity,CEntity *source','CEventList::RegisterEvent(EVENT_COP_SET_ON_FIRE,EVENT_ENTITY_PED,entity,(CPed *)source,10000);'),
 ('void','PedEvent','CEntity *entity,CEntity *source','CEventList::RegisterEvent(EVENT_PED_SET_ON_FIRE,EVENT_ENTITY_PED,entity,(CPed *)source,10000);'),
 ('void','VehicleEvent','CEntity *entity,CEntity *source','CEventList::RegisterEvent(EVENT_CAR_SET_ON_FIRE,EVENT_ENTITY_VEHICLE,entity,(CPed *)source,10000);'),
 ('bool','BikeModel','int32_t model','return CModelInfo::IsBikeModel(model);'),
 ('bool','CarModel','int32_t model','return CModelInfo::IsCarModel(model);'),
 ('void','FleeOccupants','CEntity *entity','CCarAI::TellOccupantsToFleeCar((CVehicle *)entity);')]
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "Fire.h"
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(ret+' Fire_Backend'+name+'('+args+');\n' for ret,name,args,body in BOUNDARIES)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
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
'''+''.join(ret+' Fire_Backend'+name+'('+args+')\n{\n    // Enter the original typed native operation or canonical field at the C decision point\n    '+body+'\n}\n' for ret,name,args,body in BOUNDARIES)+'//- rouz edit (ChatGPT)\n'

def kernel(source):
    # Relocate every complete record and manager body without introducing lifecycle checks or resets
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*','',value))
    expected='''CFire::CFire(){m_bIsOngoing=false;m_bIsScriptFire=false;m_bPropagationFlag=true;m_bAudioSet=true;m_vecPos=CVector(0.0f,0.0f,0.0f);m_nExtinguishTime=0;m_nStartTime=0;m_pEntity=nil;m_pSource=nil;m_fStrength=0.8f;m_fWaterExtinguishCountdown=1.0f;m_bExtinguishedWithWater=false;}'''
    assert normalize(function(source,'CFire::','CFire'))==normalize(expected)
    assert re.search(r'CFire::~CFire\(\)\s*\{\s*\}',source)
    source=source.replace('CFireManager::StartFire(CVector pos','CFireManager::StartFireAt(CVector pos').replace('CFireManager::StartFire(CEntity *entityOnFire','CFireManager::StartFireOn(CEntity *entityOnFire')
    source=source.replace('CFireManager::GetTotalActiveFires(void) const','CFireManager::GetTotalActiveFires(void)')
    chunks=[]
    for record,name in [(True,name) for name in RECORD]+[(False,name) for name in MANAGER]:
        text=function(source,'CFire::' if record else 'CFireManager::',name)
        text=re.sub(r'(?m)^\s*//[+-] rouz edit \(ChatGPT\)\n','\n',text).replace(' // rouz edit (ChatGPT)','')
        text=text.replace('CFire::','Fire_').replace('CFireManager::','FireManager_')
        if record:
            text=text.replace('Fire_'+name+'(void)','Fire_'+name+'(CFire *fire)',1)
            for field in FIELDS:text=re.sub(r'(?<![.>\w])\b'+field+r'\b','fire->'+field,text)
        else:
            text=text.replace('FireManager_'+name+'(', 'FireManager_'+name+'('+('const ' if name=='GetTotalActiveFires' else '')+'CFireManager *manager,',1).replace('*manager,void)','*manager)').replace(') const\n',')\n')
            text=re.sub(r'(?<![.>\w])\b(m_nTotalFires|m_aFires)\b',r'manager->\1',text)
        text=text.replace('this','fire').replace('nil','NULL')
        text=text.replace('CPed *ped = (CPed *)fire->m_pEntity','CEntity *ped = fire->m_pEntity').replace('CVehicle *veh = (CVehicle*)fire->m_pEntity','CEntity *veh = fire->m_pEntity')
        text=text.replace('CPed *ped = (CPed*)fire->m_pEntity','CEntity *ped = fire->m_pEntity')
        text=text.replace('CPed *ped = (CPed *)entityOnFire','CEntity *ped = entityOnFire').replace('CVehicle *veh = (CVehicle *)entityOnFire','CEntity *veh = entityOnFire')
        text=text.replace('CPed *ped = (CPed *)target','CEntity *ped = target').replace('CVehicle *veh = (CVehicle *)target','CEntity *veh = target')
        text=text.replace('((CPed *)fire->m_pEntity)->IsPed()','Fire_BackendIsPed(fire->m_pEntity)')
        text=text.replace('((CVehicle *)fire->m_pEntity)->m_pCarFire = NULL','Fire_BackendSetVehicleFire(fire->m_pEntity,NULL)')
        for entity in ['fire->m_pEntity','entityOnFire','target','veh']:
            for method in ['IsPed','IsVehicle','IsCar']:
                text=text.replace(entity+'->'+method+'()', 'Fire_Backend'+method+'('+entity+')')
            text=text.replace(entity+'->GetPosition()', 'Fire_BackendPosition('+entity+')')
        text=text.replace('ped->m_pFire = fire','Fire_BackendSetPedFire(ped,fire)').replace('ped->m_pFire = NULL','Fire_BackendSetPedFire(ped,NULL)')
        text=text.replace('veh->m_pCarFire = fire','Fire_BackendSetVehicleFire(veh,fire)')
        text=text.replace('ped->m_pFire','Fire_BackendPedFire(ped)').replace('veh->m_pCarFire','Fire_BackendVehicleFire(veh)')
        text=text.replace('ped->m_nMoveState','Fire_BackendMoveState(ped)').replace('ped->bInVehicle','Fire_BackendInVehicle(ped)').replace('ped->m_pMyVehicle->IsCar()','Fire_BackendIsCar(Fire_BackendMyVehicle(ped))').replace('ped->m_pMyVehicle->m_fHealth = 75.0f','Fire_BackendVehicleHealth(Fire_BackendMyVehicle(ped),75.0f)').replace('ped->m_pMyVehicle','Fire_BackendMyVehicle(ped)')
        text=text.replace('CGameLogic::pShortCutTaxi','Fire_BackendShortcutTaxi()').replace('CGameLogic::ShortCutState == CGameLogic::SHORTCUT_TRANSITION','Fire_BackendShortcutTransition()')
        text=text.replace('(CPed *)FindPlayerPed()','Fire_BackendPlayerPed()').replace('FindPlayerPed()','Fire_BackendPlayerPed()').replace('FindPlayerVehicle()','Fire_BackendPlayerVehicle()')
        text=text.replace('Fire_BackendPlayerPed()->m_pFire','Fire_BackendPedFire(Fire_BackendPlayerPed())').replace('Fire_BackendPlayerPed()->bFireProof','Fire_BackendFireProof(Fire_BackendPlayerPed())').replace('Fire_BackendPlayerPed()->GetPosition()','Fire_BackendPosition(Fire_BackendPlayerPed())').replace('Fire_BackendPlayerPed()->DoStuffToGoOnFire()','Fire_BackendPrepareOnFire(Fire_BackendPlayerPed())')
        text=text.replace('gFireManager.StartFire(Fire_BackendPlayerPed(), fire->m_pSource, 0.8f, 1)','FireManager_StartFireOn(&gFireManager,Fire_BackendPlayerPed(),fire->m_pSource,0.8f,1)')
        text=re.sub(r'\(\(CPlayerPed \*\)fire->m_pEntity\)->InflictDamage\(\s*\(CPlayerPed \*\)fire->m_pSource, WEAPONTYPE_FLAMETHROWER,\s*(fDamagePlayer|fDamagePeds), PEDPIECE_TORSO, 0\)',r'Fire_BackendPedDamage(fire->m_pEntity,fire->m_pSource,\1)',text)
        text=text.replace('veh->InflictDamage((CVehicle *)fire->m_pSource, WEAPONTYPE_FLAMETHROWER, fDamageVehicle)','Fire_BackendVehicleDamage(veh,fire->m_pSource,fDamageVehicle)').replace('fire->m_pEntity->bRenderScorched = true','Fire_BackendScorched(fire->m_pEntity)')
        text=text.replace('CVehicleModelInfo *mi = ((CVehicleModelInfo*)CModelInfo::GetModelInfo(veh->GetModelIndex()));\n\t\t\tCVector ModelInfo = mi->m_positions[CAR_POS_HEADLIGHTS];','FireVector ModelInfo = Fire_BackendHeadlights(Fire_BackendModel(veh));').replace('ModelInfo = fire->m_pEntity->GetMatrix() * ModelInfo','ModelInfo = Fire_BackendTransform(fire->m_pEntity,ModelInfo)')
        text=re.sub(r'CParticle::AddParticle\(PARTICLE_CARFLAME, firePos,\s*CVector\((.*?)\),\s*0, fire->m_fStrength, 0, 0, 0, 0\)',r'Fire_BackendFlame(firePos,Fire_Make(\1),fire->m_fStrength)',text)
        text=re.sub(r'CParticle::AddParticle\(PARTICLE_CARFLAME_SMOKE, firePos,\s*CVector\(0.0f, 0.0f, 0.0f\), 0, 0.0f, 0, 0, 0, 0\)',r'Fire_BackendSmoke(firePos)',text)
        text=re.sub(r'CShadows::StoreStaticShadow\(\(uintptr\)fire, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &lightpos,[\s\S]*?0, 0.0f\)',r'Fire_BackendShadow((uintptr_t)fire,lightpos,nRandNumber)',text)
        text=text.replace('PointLights_AddLightLegacy(POINTLIGHTS_LIGHT_POINT, fire->m_vecPos, CVector(0.0f, 0.0f, 0.0f), 12.0f, fRed, fGreen, 0.0f, 0, 0)','PointLights_AddLight(POINTLIGHTS_LIGHT_POINT,Fire_LightVector(fire->m_vecPos),(PointLightsVector){0.0f,0.0f,0.0f},12.0f,fRed,fGreen,0.0f,0,0)')
        text=text.replace('CEventList::RegisterEvent(EVENT_FIRE, fire->m_vecPos, 1000)','Fire_BackendReport(fire->m_vecPos)')
        text=text.replace('ped->CanSetPedState()','Fire_BackendCanSetState(ped)').replace('ped->m_nPedState','Fire_BackendPedState(ped)').replace('ped->IsPlayer()','Fire_BackendIsPlayer(ped)').replace('ped->SetIdle()','Fire_BackendIdle(ped)').replace('ped->m_nLastPedState = PED_NONE','Fire_BackendLastStateNone(ped)').replace('ped->SetWanderPath(0)','Fire_BackendWander(ped)').replace('ped->SetWaitState(WAITSTATE_FINISH_FLEE, 0)','Fire_BackendWaitFinish(ped)')
        text=text.replace('ped->IsPedInControl()','Fire_BackendInControl(ped)').replace('((CAutomobile *)veh)->Damage.SetEngineStatus(215)','Fire_BackendSetEngineStatus(veh,215)').replace('((CAutomobile *)veh)->Damage.GetEngineStatus()','Fire_BackendEngineStatus(veh)')
        text=text.replace('ped->SetFlee(fleeFrom, 10000)','Fire_BackendFleeEntity(ped,fleeFrom)').replace('CVector2D pos = Fire_BackendPosition(entityOnFire);','FireVector fleePosition = Fire_BackendPosition(entityOnFire);').replace('CVector2D pos = Fire_BackendPosition(target);','FireVector fleePosition = Fire_BackendPosition(target);').replace('ped->SetFlee(pos, 10000)','Fire_BackendFleePosition(ped,fleePosition)').replace('ped->m_fleeFrom = NULL','Fire_BackendClearFleeFrom(ped)').replace('ped->m_fleeTimer','(*Fire_BackendFleeTimer(ped))').replace('ped->bDrawLast = false','Fire_BackendDrawLastOff(ped)').replace('ped->SetMoveState(PEDMOVE_SPRINT)','Fire_BackendSprint(ped)').replace('ped->SetMoveAnim()','Fire_BackendMoveAnim(ped)').replace('ped->SetPedState(PED_ON_FIRE)','Fire_BackendOnFireState(ped)').replace('ped->m_nPedType','Fire_BackendPedType(ped)')
        for event,kind in [('COP','PED'),('PED','PED'),('CAR','VEHICLE')]:
            text=re.sub(r'CEventList::RegisterEvent\(EVENT_'+event+r'_SET_ON_FIRE, EVENT_ENTITY_'+kind+r',\s*entityOnFire, \(CPed \*\)fleeFrom, 10000\)', 'Fire_Backend'+('Vehicle' if event=='CAR' else event.title())+'Event(entityOnFire,fleeFrom)',text)
        text=text.replace('veh->GetModelIndex()','Fire_BackendModel(veh)').replace('CModelInfo::IsBikeModel(','Fire_BackendBikeModel(').replace('CModelInfo::IsCarModel(','Fire_BackendCarModel(').replace('CCarAI::TellOccupantsToFleeCar(veh)','Fire_BackendFleeOccupants(veh)')
        text=text.replace('entityOnFire->RegisterReference(&fire->m_pEntity)','References_RegisterReference(entityOnFire,&fire->m_pEntity)').replace('fleeFrom->RegisterReference(&fire->m_pSource)','References_RegisterReference(fleeFrom,&fire->m_pSource)').replace('target->RegisterReference(&fire->m_pEntity)','References_RegisterReference(target,&fire->m_pEntity)')
        text=text.replace('(Fire_BackendPosition(Fire_BackendPlayerPed()) - fire->m_vecPos).MagnitudeSqr()','Fire_MagnitudeSqr(Fire_Sub(Fire_BackendPosition(Fire_BackendPlayerPed()),fire->m_vecPos))')
        text=text.replace('(manager->m_aFires[j].m_vecPos - vecPos).Magnitude2D()','Fire_Magnitude2D(Fire_Sub(manager->m_aFires[j].m_vecPos,vecPos))').replace('(manager->m_aFires[i].m_vecPos - coords).Magnitude2D()','Fire_Magnitude2D(Fire_Sub(manager->m_aFires[i].m_vecPos,coords))').replace('(point - manager->m_aFires[i].m_vecPos).MagnitudeSqr()','Fire_MagnitudeSqr(Fire_Sub(point,manager->m_aFires[i].m_vecPos))')
        text=text.replace('fireToExtinguish->m_vecPos +\n\t\tCVector(', 'Fire_Add(fireToExtinguish->m_vecPos,Fire_Make(').replace('General_GetRandomNumber() / 200.f);','General_GetRandomNumber() / 200.f));')
        text=re.sub(r'CParticle::AddParticle\(PARTICLE_STEAM_NY_SLOWMOTION, steamPos, CVector\(0.f, 0.f, (0.[12]f)\), NULL, (0.[58]f)\)',r'Fire_BackendSteam(steamPos,\1,\2)',text)
        for expr in ['manager->m_aFires[i]','manager->m_aFires[index]']:
            for method in RECORD:text=text.replace(expr+'.'+method+'()','Fire_'+method+'(&'+expr+')')
        for expr in ['fire','fireToExtinguish','Fire_BackendPedFire(ped)','Fire_BackendVehicleFire(veh)']:
            for method in RECORD:text=text.replace(expr+'->'+method+'()','Fire_'+method+'('+expr+')')
        if record:text=re.sub(r'(?<![\w])Extinguish\(\)','Fire_Extinguish(fire)',text)
        if not record:
            for method in MANAGER:text=re.sub(r'(?<![\w])'+method+r'\(', 'FireManager_'+method+'(manager,',text)
            text=text.replace('FireManager_GetNextFreeFire(manager,)','FireManager_GetNextFreeFire(manager)')
        text=text.replace('CVector','FireVector').replace('Min(','Fire_Min(').replace('sq(range)','Fire_Square(range)')
        if name=='StartScriptFire':
            text=text.replace('const FireVector &pos','const FirePosition *position').replace('fire->m_vecPos = pos','fire->m_vecPos = Fire_BackendReadVector(position)')
        for old,new in [('uint32','uint32_t'),('int32','int32_t'),('uint8','uint8_t'),('int16','int16_t')]:text=re.sub(r'\b'+old+r'\b',new,text)
        text=text.replace('\n{','\n{\n    // Preserve the complete original '+('record' if record else 'manager')+' '+name+' operation and ordered live accesses',1)
        chunks.append(text)
    initializer=',\n'.join('    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false}' for i in range(40))
    return '''//+ rouz edit (ChatGPT)
#include "Fire_backend.h"
#include "Timer.h"
#include "General.h"
#include "PedType.h"
#include "PedStates.h"
#include "References.h"
#include "PointLights.h"
#include <stddef.h>
#include <math.h>
_Static_assert(NUM_FIRES==40,"Original fire count for constructor-equivalent static initialization");
CFireManager gFireManager={0,{
'''+initializer+'''
}};
static FireVector Fire_Make(float x,float y,float z)
{
    // Materialize the original three-component vector value
    FireVector v={x,y,z};return v;
}
static FireVector Fire_Add(FireVector a,FireVector b)
{
    // Preserve the original component-wise additions
    return Fire_Make(a.x+b.x,a.y+b.y,a.z+b.z);
}
static FireVector Fire_Sub(FireVector a,FireVector b)
{
    // Preserve the original component-wise subtractions
    return Fire_Make(a.x-b.x,a.y-b.y,a.z-b.z);
}
static float Fire_MagnitudeSqr(FireVector v)
{
    // Keep the original three-term squared distance evaluation
    return v.x*v.x+v.y*v.y+v.z*v.z;
}
static float Fire_Magnitude2D(FireVector v)
{
    // Keep the original two-term squared distance and float square root
    return sqrtf(v.x*v.x+v.y*v.y);
}
static float Fire_Square(float value)
{
    // Evaluate the original range argument once before multiplying it by itself
    return value*value;
}
static float Fire_Min(float a,float b)
{
    // Preserve the original strict comparison including ties and unordered values
    return a<b?a:b;
}
static PointLightsVector Fire_LightVector(FireVector v)
{
    // Transfer the three original coordinates into the existing C point-light API
    PointLightsVector result={v.x,v.y,v.z};return result;
}
'''+ '\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def arguments(source,opening):
    # Keep nested native vector expressions together while rewriting each actual call
    parts=[];start=opening+1;depth=1;i=start
    while depth:
        if source[i] in '([{':depth+=1
        elif source[i] in ')]}':depth-=1
        if source[i]==',' and depth==1:parts.append(source[start:i]);start=i+1
        i+=1
    parts.append(source[start:i-1]);return i,parts

def fire_calls(source):
    # Replace both class definitions and every actual owning call without changing caller evaluation counts
    if 'class CFireManager\n' in source:return HEADER
    if re.search(r'(?m)^CFireManager gFireManager;',source):
        if OBSERVER:OBSERVER(source)
        return HOST
    original=source;changes=[]
    vectorMethods=['StartFire','FindNearestFire','FindFurthestFire_NeverMindFireMen','ExtinguishPoint','ExtinguishPointWithWater']
    for match in re.finditer(r'gFireManager\.(\w+)\(',source):
        end,args=arguments(source,source.index('(',match.start()));name=match[1]
        if args==['']:args=[]
        if name=='StartFire':name='StartFireAt' if len(args)==3 else 'StartFireOn'
        if match[1] in vectorMethods and name!='StartFireOn':args[0]='Fire_VectorFromNative('+args[0].strip()+')'
        if name=='StartScriptFire':args[0]='&('+args[0].strip()+')'
        changes.append((match.start(),end,'FireManager_'+name+'(&gFireManager'+(','+','.join(args) if args else '')+')'))
    for start,end,text in reversed(changes):source=source[:start]+text+source[end:]
    source=re.sub(r'((?:\b\w+->)*\b\w+)->Extinguish\(\)',r'Fire_Extinguish(\1)',source)
    source=source.replace('class CFire;','struct CFire;')
    for value in ['nearestFire->m_vecPos','pNearestFire->m_vecPos']:
        source=re.sub(r'(?<!Fire_VectorToNative\()'+re.escape(value),lambda match:'Fire_VectorToNative('+match[0]+')',source)
    source=source.replace('AudioSoundPosition_FromVector(gFireManager.m_aFires[id].m_vecPos)','AudioSoundPosition_FromVector(Fire_VectorToNative(gFireManager.m_aFires[id].m_vecPos))')
    if source==original:return source
    if original.lstrip().startswith('//+ rouz edit (ChatGPT)') and original.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Forward complete historical owners through the separately verified bridge class removal
    return bridge.calls(fire_calls(source))

def project(source):
    # Keep controller storage and lifecycle in a uniquely named C11 unit beside typed native adapters
    for tag,old,new in [('ClCompile','Fire.cpp','FireSystem.c'),('ClInclude','Fire.h','Fire_backend.h')]:
        entry='<'+tag+' Include="..\\src\\core\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\core\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\core\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\core\\'+re.escape(old)+r'">\n\s*<Filter>core</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return bridge.project(source)

def verify_legacy(source,transform,root):
    # Prove complete relocated fire lifecycles and their native field bindings from earlier immutable sources
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/core/Fire.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/core/FireSystem.c').read_text())

def verify_dependency(saved,current):
    # Keep byte guards unless the complete file follows the exact fire conversion
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original)
    assert expected!=original and expected==current.read_text(),str(current)
