"""Relocate the complete roadblock owner and script record to C11 with typed native primitives."""
import re
import object_names_migration as objects
from events_migration import arguments
OBSERVER=None
OPERATIONS=['Init','GenerateRoadBlockCopsForCar','GenerateRoadBlocks','ClearScriptRoadBlocks','RegisterScriptRoadBlock','CreateRoadBlockBetween2Points']
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CVehicle;
#else
typedef struct CVehicle CVehicle;
#endif
typedef struct RoadBlockVector {float x,y,z;} RoadBlockVector;
typedef struct CScriptRoadblock {RoadBlockVector m_vInf,m_vSup;bool m_bInUse;} CScriptRoadblock;
#ifdef __cplusplus
extern "C" {
#endif
extern int16_t RoadBlocks_NumRoadBlocks;
extern int16_t RoadBlocks_RoadBlockNodes[NUMROADBLOCKS];
extern bool RoadBlocks_InOrOut[NUMROADBLOCKS];
extern CScriptRoadblock RoadBlocks_aScriptRoadBlocks[NUM_SCRIPT_ROADBLOCKS];
#ifdef SECUROM
extern uint8_t roadBlocksPirateCheck;
#endif
RoadBlockVector ScriptRoadblock_GetPosition(const CScriptRoadblock *block);
void RoadBlocks_Init(void);
void RoadBlocks_GenerateRoadBlockCopsForCar(CVehicle *vehicle,int32_t roadBlockType);
void RoadBlocks_GenerateRoadBlocks(void);
void RoadBlocks_ClearScriptRoadBlocks(void);
void RoadBlocks_RegisterScriptRoadBlock(RoadBlockVector inf,RoadBlockVector sup);
void RoadBlocks_CreateRoadBlockBetween2Points(RoadBlockVector point1,RoadBlockVector point2);
#ifdef __cplusplus
}
class CVector;
RoadBlockVector RoadBlocks_FromVector(CVector value);
#endif
//- rouz edit (ChatGPT)
'''
LEAVES=[]

def leaf(kind,name,args,code):
    # Register an exact external primitive without any roadblock branch or loop
    LEAVES.append((kind,name,args,code))

leaf('int32_t','NumCarNodes','void','return ThePaths.m_numCarPathNodes;')
leaf('CPathNode *','Node','int32_t node','return &ThePaths.m_pathNodes[node];')
for kind,name,expr in [('bool','NodeUse','node->bUseInRoadBlock'),('uint8_t','NodeLinks','node->numLinks'),('int16_t','NodeFirstLink','node->firstLink'),('bool','NodeDivider','node->HasDivider()'),('float','NodeDividerWidth','node->GetDividerWidth()'),('RoadBlockVector','NodePosition','RoadBlocks_FromVector(node->GetPosition())')]:leaf(kind,name,'CPathNode *node','return '+expr+';')
leaf('int32_t','Connection','int32_t link','return ThePaths.m_carPathConnections[link];')
leaf('CCarPathLink *','Link','int32_t link','return &ThePaths.m_carPathLinks[link];')
for kind,name,expr in [('int8_t','LinkLeft','link->numLeftLanes'),('int8_t','LinkRight','link->numRightLanes'),('float','LinkX','link->GetX()'),('float','LinkY','link->GetY()')]:leaf(kind,name,'CCarPathLink *link','return '+expr+';')
leaf('RoadBlockVector','PlayerCoors','void','return RoadBlocks_FromVector(FindPlayerCoors());')
leaf('CVehicle *','PlayerVehicle','void','return FindPlayerVehicle();')
leaf('CEntity *','VehicleEntity','CVehicle *vehicle','return (CEntity*)vehicle;')
leaf('CEntity *','PlayerEntity','void','return (CEntity*)FindPlayerPed();')
leaf('CWanted *','Wanted','void','return FindPlayerPed()->m_pWanted;')
leaf('uint8_t','Density','void','return FindPlayerPed()->m_pWanted->m_RoadblockDensity;')
leaf('bool','ModelLoaded','int32_t model','return CStreaming::HasModelLoaded(model);')
leaf('void','RequestModel','int32_t model,int32_t flags','CStreaming::RequestModel(model,flags);')
leaf('CColModel *','ColModel','int32_t model','return CModelInfo::GetColModel(model);')
leaf('CBaseModelInfo *','ModelInfo','int32_t model','return CModelInfo::GetModelInfo(model);')
leaf('CColModel *','InfoColModel','CBaseModelInfo *info','return info->GetColModel();')
leaf('float','ColRadius','CColModel *model','return model->boundingSphere.radius;')
leaf('float','ColMinZ','CColModel *model','return model->boundingBox.min.z;')
leaf('float','VehicleRadius','CVehicle *vehicle','return vehicle->GetBoundRadius();')
leaf('int32_t','VehicleModel','CVehicle *vehicle','return vehicle->GetModelIndex();')
for name in ['Ped','Vehicle','Object']:leaf('CPool *',name+'Pool','void','return CPools::Get'+name+'Pool();')
leaf('CCopPed *','ConstructCop','CCopPed *cop,int32_t type','std::allocator<CCopPed>().construct(cop,(eCopType)type);return cop;')
leaf('CAutomobile *','ConstructCar','CAutomobile *car,int32_t model,int32_t createdBy','std::allocator<CAutomobile>().construct(car,model,createdBy);return car;')
leaf('void','ConstructObject','CObject *object,int32_t model,bool create','std::allocator<CObject>().construct(object,model,create);')
leaf('void','DestroyCar','CAutomobile *car','car->~CAutomobile();')
leaf('void','DestroyObject','CObject *object','object->~CObject();')
leaf('void','CopWeapon','CCopPed *cop,int32_t weapon','cop->SetCurrentWeapon((eWeaponType)weapon);')
leaf('void','PedZ','RoadBlockVector *point','CVector native(point->x,point->y,point->z);CPedPlacement::FindZCoorForPed(&native);*point=RoadBlocks_FromVector(native);')
leaf('void','CopPosition','CCopPed *cop,RoadBlockVector point','cop->SetPosition(CVector(point.x,point.y,point.z));')
leaf('void','CopOrientation','CCopPed *cop,float x,float y,float z','cop->SetOrientation(x,y,z);')
leaf('void','CopIdle','CCopPed *cop','cop->SetIdle();')
for name,field in [('CopDisabled','m_bIsDisabledCop'),('CopStay','bKindaStayInSamePlace'),('CopNoDuck','bNotAllowedToDuck'),('CopCrouch','bCrouchWhenShooting'),('CopCull','bCullExtraFarAway')]:leaf('void',name,'CCopPed *cop,bool value','cop->'+field+'=value;')
for kind,name,field in [('uint32_t','CopRange','m_nExtendedRangeTimer'),('CVehicle *','CopRoadVehicle','m_nRoadblockVeh'),('CVehicle *','CopVehicle','m_pMyVehicle')]:leaf(kind+' *',name,'CCopPed *cop','return &cop->'+field+';')
leaf('void','CopRoadReference','CCopPed *cop','cop->m_nRoadblockVeh->RegisterReference((CEntity**)&cop->m_nRoadblockVeh);')
leaf('void','CopVehicleReference','CVehicle *vehicle,CCopPed *cop','vehicle->RegisterReference((CEntity**)&cop->m_pMyVehicle);')
leaf('void','CopLockOn','CCopPed *cop,CEntity *target','cop->SetWeaponLockOnTarget(target);')
leaf('void','CopAttack','CCopPed *cop,CEntity *target','cop->SetAttack(target);')
for what,kind in [('Cop','CCopPed'),('Vehicle','CAutomobile')]:
    leaf('void',what+'Alpha',kind+' *entity,int32_t alpha','CVisibilityPlugins::SetClumpAlpha(entity->GetClump(),alpha);')
    leaf('void','Add'+what,kind+' *entity','CWorld::Add(entity);')
leaf('void','Colliding','RoadBlockVector point,float radius,int16_t *count','CWorld::FindObjectsKindaColliding(CVector(point.x,point.y,point.z),radius,0,count,2,nil,false,true,true,false,false);')
leaf('float','GroundZ','float x,float y,float z,bool *found','return CWorld::FindGroundZFor3DCoord(x,y,z,found);')
leaf('void','CarStatus','CAutomobile *car,int32_t status','car->SetStatus((eEntityStatus)status);')
leaf('void','CarMatrix','CAutomobile *car,CMatrix *matrix','car->SetMatrix(*matrix);')
leaf('void','CarOnRoad','CAutomobile *car','car->PlaceOnRoadProperly();')
leaf('void','CarStatic','CAutomobile *car,bool value','car->SetIsStatic(value);')
leaf('void','CarUpdateRW','CAutomobile *car','car->GetMatrix().UpdateRW();')
leaf('void','CarLock','CAutomobile *car,int32_t value','car->m_nDoorLock=(eCarLock)value;')
leaf('void','CarJoinRoad','CAutomobile *car','CCarCtrl::JoinCarWithRoadSystem(car);')
for name,field in [('CarLocked','bIsLocked'),('CarExtended','bExtendedRange'),('CarSiren','m_bSirenOrAlarm'),('CarCreatePeds','bCreateRoadBlockPeds')]:leaf('void',name,'CAutomobile *car,bool value','car->'+field+'=value;')
for kind,name,field in [('uint8_t','CarMission','AutoPilot.m_nCarMission'),('uint8_t','CarTempAction','AutoPilot.m_nTempAction'),('int8_t','CarNextLane','AutoPilot.m_nNextLane'),('int8_t','CarCurrentLane','AutoPilot.m_nCurrentLane'),('uint8_t','CarCruiseSpeed','AutoPilot.m_nCruiseSpeed'),('float','CarMaxSpeed','AutoPilot.m_fMaxTrafficSpeed'),('uint8_t','CarRoadblockType','m_nRoadblockType'),('uint32_t','CarRange','m_nSetPieceExtendedRangeTime')]:leaf(kind+' *',name,'CAutomobile *car','return &car->'+field+';')
leaf('bool','CarUsesSiren','CAutomobile *car','return car->UsesSiren();')
for name,expr in [('CarUpZ','car->GetUp().z'),('ObjectCreated','object->ObjectCreatedBy')]:leaf('float' if name=='CarUpZ' else 'int8_t',name,('CAutomobile *car' if name=='CarUpZ' else 'CObject *object'),'return '+expr+';')
for name,method in [('CarRight','GetRight'),('CarPosition','GetPosition')]:leaf('RoadBlockVector',name,'CAutomobile *car','return RoadBlocks_FromVector(car->'+method+'());')
leaf('void','RemoveObject','CObject *object','CWorld::Remove(object);')
leaf('void','ObjectMatrix','CObject *object,CMatrix *matrix','object->GetMatrix()=*matrix;')
leaf('void','ObjectCreatedSet','CObject *object,int8_t value','object->ObjectCreatedBy=value;')
leaf('uint32_t *','ObjectLife','CObject *object','return &object->m_nEndOfLifeTime;')
leaf('void','AddObject','CObject *object','CWorld::Add(object);')
leaf('CMatrix *','MatrixConstruct','RoadBlockMatrixStorage *storage','CMatrix *matrix=(CMatrix*)storage;std::allocator<CMatrix>().construct(matrix);return matrix;')
leaf('void','MatrixDestroy','CMatrix *matrix','matrix->~CMatrix();')
leaf('void','MatrixTranslate','CMatrix *matrix,float x,float y,float z','matrix->SetTranslate(x,y,z);')
leaf('void','MatrixTranslateOnly','CMatrix *matrix,RoadBlockVector point','matrix->SetTranslateOnly(CVector(point.x,point.y,point.z));')
leaf('void','MatrixRotateZ','CMatrix *matrix,float angle','matrix->RotateZ(angle);')
for name in ['Right','Forward','Up']:leaf('void','Matrix'+name,'CMatrix *matrix,RoadBlockVector point','matrix->Get'+name+'()=CVector(point.x,point.y,point.z);')
leaf('RoadBlockVector','MatrixPosition','CMatrix *matrix','return RoadBlocks_FromVector(matrix->GetPosition());')
for axis in ['X','Y','Z']:leaf('float *','Matrix'+axis,'CMatrix *matrix','return &matrix->GetPosition().'+axis.lower()+';')
leaf('RoadBlockVector','VehicleTransform','CVehicle *vehicle,RoadBlockVector point','return RoadBlocks_FromVector(vehicle->GetMatrix()*CVector(point.x,point.y,point.z));')
leaf('void','AssertFailed','const char *expression,const char *file,unsigned line,const char *function','re3_assert(expression,file,line,function);')
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "RoadBlocks.h"
#include "Pool.h"
#include "Wanted.h"
#include "EntityStatus.h"
#include "WeaponType.h"
#include "ModelIndices.h"
#include "Draw_constants.h"
enum {ROADBLOCKS_COP_STREET=0,ROADBLOCKS_COP_FBI=1,ROADBLOCKS_COP_SWAT=2,ROADBLOCKS_COP_ARMY=4,ROADBLOCKS_RANDOM_VEHICLE=1,ROADBLOCKS_CARLOCK_UNLOCKED=1,ROADBLOCKS_MISSION_NONE=0,ROADBLOCKS_TEMPACT_NONE=0,ROADBLOCKS_TEMP_OBJECT=3,ROADBLOCKS_STREAMFLAGS_DONT_REMOVE=1};
typedef union RoadBlockMatrixStorage {uint64_t alignment;uint8_t bytes[80];} RoadBlockMatrixStorage;
#ifdef __cplusplus
class CMatrix;class CEntity;class CCopPed;class CAutomobile;class CObject;class CColModel;class CBaseModelInfo;struct CPathNode;struct CCarPathLink;
extern "C" {
#else
typedef struct CMatrix CMatrix;typedef struct CEntity CEntity;typedef struct CCopPed CCopPed;typedef struct CAutomobile CAutomobile;typedef struct CObject CObject;typedef struct CColModel CColModel;typedef struct CBaseModelInfo CBaseModelInfo;typedef struct CPathNode CPathNode;typedef struct CCarPathLink CCarPathLink;
#endif
'''+''.join(kind+' RoadBlocks_Backend'+name+'('+args+');\n' for kind,name,args,code in LEAVES)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def host():
    # Retain only typed native storage, matrix operations and external entity primitives
    includes=['common','RoadBlocks_backend','PathFind','Streaming','World','PedPlacement','Automobile','CopPed','VisibilityPlugins','PlayerPed','CarCtrl','Object','Pools']
    source='//+ rouz edit (ChatGPT)\n'+''.join('#include "'+v+'.h"\n' for v in includes)
    source+='static_assert(sizeof(CMatrix)<=sizeof(RoadBlockMatrixStorage),"Native matrix storage extent");\nstatic_assert(alignof(CMatrix)<=alignof(RoadBlockMatrixStorage),"Native matrix storage alignment");\n'
    for name,value in [('COP_STREET',0),('COP_FBI',1),('COP_SWAT',2),('COP_ARMY',4),('RANDOM_VEHICLE',1),('CARLOCK_UNLOCKED',1),('MISSION_NONE',0),('TEMPACT_NONE',0),('TEMP_OBJECT',3),('STREAMFLAGS_DONT_REMOVE',1)]:source+='static_assert('+name+'=='+str(value)+',"Original '+name+' constant");\n'
    source+='RoadBlockVector RoadBlocks_FromVector(CVector value)\n{\n    // Copy the original three float components at the native argument boundary\n    RoadBlockVector result={value.x,value.y,value.z};return result;\n}\n'
    return source+'\n'.join(kind+' RoadBlocks_Backend'+name+'('+args+')\n{\n    // Access the original typed primitive at its original C policy access point\n    '+code+'\n}\n' for kind,name,args,code in LEAVES)+'//- rouz edit (ChatGPT)\n'

HELPERS='''static RoadBlockVector rb_vec(float x,float y,float z)
{
    // Construct the original three float vector value
    RoadBlockVector value={x,y,z};return value;
}
static RoadBlockVector rb_add(RoadBlockVector a,RoadBlockVector b)
{
    // Preserve the original component addition order
    return rb_vec(a.x+b.x,a.y+b.y,a.z+b.z);
}
static RoadBlockVector rb_sub(RoadBlockVector a,RoadBlockVector b)
{
    // Preserve the original component subtraction order
    return rb_vec(a.x-b.x,a.y-b.y,a.z-b.z);
}
static RoadBlockVector rb_scale(RoadBlockVector a,float b)
{
    // Preserve the original vector-times-scalar operand order
    return rb_vec(a.x*b,a.y*b,a.z*b);
}
static RoadBlockVector rb_left(float a,RoadBlockVector b)
{
    // Preserve the original scalar-times-vector operand order
    return rb_vec(a*b.x,a*b.y,a*b.z);
}
static RoadBlockVector rb_div(RoadBlockVector a,float b)
{
    // Preserve division instead of replacing it with reciprocal multiplication
    return rb_vec(a.x/b,a.y/b,a.z/b);
}
static float rb_dot(RoadBlockVector a,RoadBlockVector b)
{
    // Preserve the original dot product association
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static float rb_magnitude(RoadBlockVector value)
{
    // Preserve the original three-component magnitude and square-root precision
    return sqrtf(value.x*value.x+value.y*value.y+value.z*value.z);
}
static float rb_magnitude2(RoadBlockVector value)
{
    // Preserve the original two-component magnitude used for path distance
    return sqrtf(value.x*value.x+value.y*value.y);
}
static void rb_normalise(RoadBlockVector *value)
{
    // Preserve CVector's zero-vector fallback and reciprocal square-root operations
    float sq=value->x*value->x+value->y*value->y+value->z*value->z;
    if(sq>0.0f){float invsqrt=1.0f/sqrtf(sq);value->x*=invsqrt;value->y*=invsqrt;value->z*=invsqrt;}
    else value->x=1.0f;
}
RoadBlockVector ScriptRoadblock_GetPosition(const CScriptRoadblock *block)
{
    // Preserve the complete original inline midpoint operation
    return rb_div(rb_add(block->m_vInf,block->m_vSup),2);
}
'''

def kernel(source):
    # Relocate all six complete original operations and keep every pool-reuse and placement branch
    text=source[source.index('#define ROADBLOCKDIST'):source.rindex('//- rouz edit (ChatGPT)')]
    text=text.replace('CRoadBlocks::','RoadBlocks_')
    for name in ['NumRoadBlocks','RoadBlockNodes','InOrOut','aScriptRoadBlocks']+OPERATIONS:text=re.sub(r'(?<![\w])'+name+r'\b','RoadBlocks_'+name,text)
    text=text.replace('CVector2D vecDistance = FindPlayerCoors() - ThePaths.m_pathNodes[node].GetPosition();','RoadBlockVector vecDistance = rb_sub(RoadBlocks_BackendPlayerCoors(),RoadBlocks_BackendNodePosition(RoadBlocks_BackendNode(node)));')
    text=text.replace('CMatrix tmp1, tmp2;','RoadBlockMatrixStorage storage1,storage2;\n    CMatrix *tmp1=RoadBlocks_BackendMatrixConstruct(&storage1),*tmp2=RoadBlocks_BackendMatrixConstruct(&storage2);')
    text=text.replace('CMatrix tmp;','RoadBlockMatrixStorage storage;\n    CMatrix *tmp=RoadBlocks_BackendMatrixConstruct(&storage);')
    text=text.replace('CVector','RoadBlockVector').replace('eCopType copType','int32_t copType').replace('CObjectPool *','CPool *').replace('nil','NULL')
    for old,new in [('int16','int16_t'),('int32','int32_t'),('uint32','uint32_t'),('uint8','uint8_t')]:text=re.sub(r'\b'+old+r'\b',new,text)
    text=re.sub(r'RoadBlockVector\(([^()]*)\)',r'rb_vec(\1)',text)
    text=text.replace('static const RoadBlockVector vecRoadBlockOffets[6] = { rb_vec(-1.5, 1.8f, 0.0f), rb_vec(-1.5f, -1.8f, 0.0f), rb_vec(1.5f, 1.8f, 0.0f),\n\trb_vec(1.5f, -1.8f, 0.0f), rb_vec(-1.5f, 0.0f, 0.0f), rb_vec(1.5, 0.0, 0.0) };','static const RoadBlockVector vecRoadBlockOffets[6] = {{-1.5,1.8f,0.0f},{-1.5f,-1.8f,0.0f},{1.5f,1.8f,0.0f},{1.5f,-1.8f,0.0f},{-1.5f,0.0f,0.0f},{1.5,0.0,0.0}};')
    replacements={
        '(CEntity*)FindPlayerVehicle()':'RoadBlocks_BackendVehicleEntity(RoadBlocks_BackendPlayerVehicle())','(CEntity*)FindPlayerPed()':'RoadBlocks_BackendPlayerEntity()',
        'FindPlayerPed()->m_pWanted->m_RoadblockDensity':'RoadBlocks_BackendDensity()', 'FindPlayerPed()->m_pWanted':'RoadBlocks_BackendWanted()',
        'ThePaths.m_numCarPathNodes':'RoadBlocks_BackendNumCarNodes()',
        'ThePaths.m_pathNodes[i].bUseInRoadBlock':'RoadBlocks_BackendNodeUse(RoadBlocks_BackendNode(i))','ThePaths.m_pathNodes[i].numLinks':'RoadBlocks_BackendNodeLinks(RoadBlocks_BackendNode(i))',
        'ThePaths.m_pathNodes[node].firstLink':'RoadBlocks_BackendNodeFirstLink(RoadBlocks_BackendNode(node))',
        'ThePaths.m_pathNodes[node].HasDivider()':'RoadBlocks_BackendNodeDivider(RoadBlocks_BackendNode(node))','ThePaths.m_pathNodes[node].GetDividerWidth()':'RoadBlocks_BackendNodeDividerWidth(RoadBlocks_BackendNode(node))',
        'ThePaths.m_pathNodes[node].GetPosition()':'RoadBlocks_BackendNodePosition(RoadBlocks_BackendNode(node))',
        'CStreaming::HasModelLoaded':'RoadBlocks_BackendModelLoaded','CStreaming::RequestModel':'RoadBlocks_BackendRequestModel','CModelInfo::GetColModel':'RoadBlocks_BackendColModel','CModelInfo::GetModelInfo':'RoadBlocks_BackendModelInfo',
        'pPoliceColModel->boundingSphere.radius':'RoadBlocks_BackendColRadius(pPoliceColModel)', 'pVehicleColModel->boundingSphere.radius':'RoadBlocks_BackendColRadius(pVehicleColModel)',
        'pVehicle->GetBoundRadius()':'RoadBlocks_BackendVehicleRadius(pVehicle)','pVehicle->GetModelIndex()':'RoadBlocks_BackendVehicleModel(pVehicle)',
        'pVehicle->GetMatrix() * (fRadius * vecRoadBlockOffets[roadBlockIndex])':'RoadBlocks_BackendVehicleTransform(pVehicle,rb_left(fRadius,vecRoadBlockOffets[roadBlockIndex]))',
        'CPools::GetPedPool()':'RoadBlocks_BackendPedPool()','CPools::GetVehiclePool()':'RoadBlocks_BackendVehiclePool()','CPools::GetObjectPool()':'RoadBlocks_BackendObjectPool()',
        'std::allocator<CCopPed>().construct(pCopPed, copType)':'RoadBlocks_BackendConstructCop(pCopPed,copType)',
        'std::allocator<CAutomobile>().construct(pVehicle, vehicleId, RANDOM_VEHICLE)':'RoadBlocks_BackendConstructCar(pVehicle,vehicleId,ROADBLOCKS_RANDOM_VEHICLE)',
        'std::allocator<CObject>().construct(pObject, MI_ROADWORKBARRIER1, true)':'RoadBlocks_BackendConstructObject(pObject,MI_ROADWORKBARRIER1,true)',
        'pVehicle->~CAutomobile()':'RoadBlocks_BackendDestroyCar(pVehicle)','existing->~CObject()':'RoadBlocks_BackendDestroyObject(existing)',
        'pCopPed->SetCurrentWeapon(WEAPONTYPE_COLT45)':'RoadBlocks_BackendCopWeapon(pCopPed,WEAPONTYPE_COLT45)',
        'CPedPlacement::FindZCoorForPed(&posForZ)':'RoadBlocks_BackendPedZ(&posForZ)', 'pCopPed->SetPosition(posForZ)':'RoadBlocks_BackendCopPosition(pCopPed,posForZ)',
        'pCopPed->SetOrientation(0.0f, 0.0f, -HALFPI)':'RoadBlocks_BackendCopOrientation(pCopPed,0.0f,0.0f,-HALFPI)',
        'pCopPed->SetIdle()':'RoadBlocks_BackendCopIdle(pCopPed)',
        'pCopPed->m_nRoadblockVeh->RegisterReference((CEntity**)&pCopPed->m_nRoadblockVeh)':'RoadBlocks_BackendCopRoadReference(pCopPed)',
        'pVehicle->RegisterReference((CEntity**)&pCopPed->m_pMyVehicle)':'RoadBlocks_BackendCopVehicleReference(pVehicle,pCopPed)',
        'pCopPed->SetWeaponLockOnTarget(pEntityToAttack)':'RoadBlocks_BackendCopLockOn(pCopPed,pEntityToAttack)','pCopPed->SetAttack(pEntityToAttack)':'RoadBlocks_BackendCopAttack(pCopPed,pEntityToAttack)',
        'CVisibilityPlugins::SetClumpAlpha(pCopPed->GetClump(), 0)':'RoadBlocks_BackendCopAlpha(pCopPed,0)','CWorld::Add(pCopPed)':'RoadBlocks_BackendAddCop(pCopPed)',
        'CVisibilityPlugins::SetClumpAlpha(pVehicle->GetClump(), 0)':'RoadBlocks_BackendVehicleAlpha(pVehicle,0)','CWorld::Add(pVehicle)':'RoadBlocks_BackendAddVehicle(pVehicle)',
        'pVehicle->SetStatus(STATUS_ABANDONED)':'RoadBlocks_BackendCarStatus(pVehicle,STATUS_ABANDONED)', 'pVehicle->SetMatrix(tmp)':'RoadBlocks_BackendCarMatrix(pVehicle,tmp)',
        'pVehicle->PlaceOnRoadProperly()':'RoadBlocks_BackendCarOnRoad(pVehicle)','pVehicle->SetIsStatic(false)':'RoadBlocks_BackendCarStatic(pVehicle,false)',
        'pVehicle->GetMatrix().UpdateRW()':'RoadBlocks_BackendCarUpdateRW(pVehicle)','pVehicle->m_nDoorLock = CARLOCK_UNLOCKED':'RoadBlocks_BackendCarLock(pVehicle,ROADBLOCKS_CARLOCK_UNLOCKED)',
        'CCarCtrl::JoinCarWithRoadSystem(pVehicle)':'RoadBlocks_BackendCarJoinRoad(pVehicle)','pVehicle->UsesSiren()':'RoadBlocks_BackendCarUsesSiren(pVehicle)', 'pVehicle->GetUp().z':'RoadBlocks_BackendCarUpZ(pVehicle)',
        'pVehicle->GetRight()':'RoadBlocks_BackendCarRight(pVehicle)','pVehicle->GetPosition()':'RoadBlocks_BackendCarPosition(pVehicle)',
        'CWorld::Remove(existing)':'RoadBlocks_BackendRemoveObject(existing)', 'existing->ObjectCreatedBy':'RoadBlocks_BackendObjectCreated(existing)',
        'pObject->GetMatrix() = tmp':'RoadBlocks_BackendObjectMatrix(pObject,tmp)','pObject->ObjectCreatedBy = TEMP_OBJECT':'RoadBlocks_BackendObjectCreatedSet(pObject,ROADBLOCKS_TEMP_OBJECT)',
        'pObject->m_nEndOfLifeTime':'(*RoadBlocks_BackendObjectLife(pObject))','CWorld::Add(pObject)':'RoadBlocks_BackendAddObject(pObject)',
        'pMI->GetColModel()->boundingBox.min.z':'RoadBlocks_BackendColMinZ(RoadBlocks_BackendInfoColModel(pMI))', 'pMI->GetColModel()->boundingSphere.radius':'RoadBlocks_BackendColRadius(RoadBlocks_BackendInfoColModel(pMI))',
        'RoadBlocks_BackendColModel(vehicleId)->boundingSphere.radius':'RoadBlocks_BackendColRadius(RoadBlocks_BackendColModel(vehicleId))',
        'FindPlayerVehicle()':'RoadBlocks_BackendPlayerVehicle()', 'FindPlayerCoors()':'RoadBlocks_BackendPlayerCoors()',
        'vecDistance.Magnitude()':'rb_magnitude2(vecDistance)', 'forward.Normalise()':'rb_normalise(&forward)', 'right.Normalise()':'rb_normalise(&right)',
        'right *= -1.0f':'right = rb_scale(right,-1.0f)',
        '(point2 - point1)':'rb_sub(point2,point1)','forward.Magnitude()':'rb_magnitude(forward)','(point1 + point2) / 2':'rb_div(rb_add(point1,point2),2)',
        'RoadBlockVector right(forward.y, -forward.x, 0.0f)':'RoadBlockVector right=rb_vec(forward.y,-forward.x,0.0f)',
        'RoadBlockVector forward(pLink2->GetY() - pLink1->GetY(), -(pLink2->GetX() - pLink1->GetX()), 0.0f)':'RoadBlockVector forward=rb_vec(RoadBlocks_BackendLinkY(pLink2)-RoadBlocks_BackendLinkY(pLink1),-(RoadBlocks_BackendLinkX(pLink2)-RoadBlocks_BackendLinkX(pLink1)),0.0f)',
        'DotProduct(RoadBlocks_BackendPlayerCoors() - pos, right)':'rb_dot(rb_sub(RoadBlocks_BackendPlayerCoors(),pos),right)',
        'DotProduct(RoadBlocks_BackendCarRight(pVehicle), RoadBlocks_BackendCarPosition(pVehicle) - RoadBlocks_BackendPlayerCoors())':'rb_dot(RoadBlocks_BackendCarRight(pVehicle),rb_sub(RoadBlocks_BackendCarPosition(pVehicle),RoadBlocks_BackendPlayerCoors()))',
        'tmp.SetTranslate(0.0f, 0.0f, 0.0f)':'RoadBlocks_BackendMatrixTranslate(tmp,0.0f,0.0f,0.0f)',
        'tmp.GetRight() = rb_vec(forward.y, -forward.x, 0.0f)':'RoadBlocks_BackendMatrixRight(tmp,rb_vec(forward.y,-forward.x,0.0f))',
        'tmp.GetForward() = forward':'RoadBlocks_BackendMatrixForward(tmp,forward)','tmp.GetUp() = rb_vec(0.0f, 0.0f, 1.0f)':'RoadBlocks_BackendMatrixUp(tmp,rb_vec(0.0f,0.0f,1.0f))',
        'tmp.RotateZ(':'RoadBlocks_BackendMatrixRotateZ(tmp,',
        'tmp.SetTranslateOnly(offset * forward + pos)':'RoadBlocks_BackendMatrixTranslateOnly(tmp,rb_add(rb_left(offset,forward),pos))',
        'tmp.SetTranslateOnly(5.0f * right + offset * forward + pos)':'RoadBlocks_BackendMatrixTranslateOnly(tmp,rb_add(rb_add(rb_left(5.0f,right),rb_left(offset,forward)),pos))',
        'tmp.GetPosition().x':'(*RoadBlocks_BackendMatrixX(tmp))','tmp.GetPosition().y':'(*RoadBlocks_BackendMatrixY(tmp))','tmp.GetPosition().z':'(*RoadBlocks_BackendMatrixZ(tmp))','tmp.GetPosition()':'RoadBlocks_BackendMatrixPosition(tmp)',
        'CWorld::FindGroundZFor3DCoord':'RoadBlocks_BackendGroundZ',
        'CWorld::FindObjectsKindaColliding(RoadBlocks_BackendMatrixPosition(tmp), fModelRadius, 0, &colliding, 2, NULL, false, true, true, false, false)':'RoadBlocks_BackendColliding(RoadBlocks_BackendMatrixPosition(tmp),fModelRadius,&colliding)',
        'CWorld::FindObjectsKindaColliding(RoadBlocks_BackendMatrixPosition(tmp), RoadBlocks_BackendColRadius(RoadBlocks_BackendInfoColModel(pMI)), 0, &colliding, 2, NULL, false, true, true, false, false)':'RoadBlocks_BackendColliding(RoadBlocks_BackendMatrixPosition(tmp),RoadBlocks_BackendColRadius(RoadBlocks_BackendInfoColModel(pMI)),&colliding)',
    }
    for old,new in replacements.items():text=text.replace(old,new)
    for name,field in [('CopDisabled','m_bIsDisabledCop'),('CopStay','bKindaStayInSamePlace'),('CopNoDuck','bNotAllowedToDuck'),('CopCrouch','bCrouchWhenShooting'),('CopCull','bCullExtraFarAway')]:text=re.sub(r'pCopPed->'+field+r' = ([^;]+);',r'RoadBlocks_Backend'+name+r'(pCopPed,\1);',text)
    for name,field in [('CopRange','m_nExtendedRangeTimer'),('CopRoadVehicle','m_nRoadblockVeh'),('CopVehicle','m_pMyVehicle')]:text=text.replace('pCopPed->'+field,'(*RoadBlocks_Backend'+name+'(pCopPed))')
    for name,field in [('CarLocked','bIsLocked'),('CarExtended','bExtendedRange'),('CarSiren','m_bSirenOrAlarm'),('CarCreatePeds','bCreateRoadBlockPeds')]:text=re.sub(r'pVehicle->'+field+r' = ([^;]+);',r'RoadBlocks_Backend'+name+r'(pVehicle,\1);',text)
    for name,field in [('CarMission','AutoPilot.m_nCarMission'),('CarTempAction','AutoPilot.m_nTempAction'),('CarNextLane','AutoPilot.m_nNextLane'),('CarCurrentLane','AutoPilot.m_nCurrentLane'),('CarCruiseSpeed','AutoPilot.m_nCruiseSpeed'),('CarMaxSpeed','AutoPilot.m_fMaxTrafficSpeed'),('CarRoadblockType','m_nRoadblockType'),('CarRange','m_nSetPieceExtendedRangeTime')]:text=text.replace('pVehicle->'+field,'(*RoadBlocks_Backend'+name+'(pVehicle))')
    for link in ['pLink1','pLink2']:
        for field,name in [('numRightLanes','LinkRight'),('numLeftLanes','LinkLeft')]:text=text.replace(link+'->'+field,'RoadBlocks_Backend'+name+'('+link+')')
    text=text.replace('&ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[RoadBlocks_BackendNodeFirstLink(RoadBlocks_BackendNode(node))]]','RoadBlocks_BackendLink(RoadBlocks_BackendConnection(RoadBlocks_BackendNodeFirstLink(RoadBlocks_BackendNode(node))))')
    text=text.replace('&ThePaths.m_carPathLinks[ThePaths.m_carPathConnections[RoadBlocks_BackendNodeFirstLink(RoadBlocks_BackendNode(node)) + 1]]','RoadBlocks_BackendLink(RoadBlocks_BackendConnection(RoadBlocks_BackendNodeFirstLink(RoadBlocks_BackendNode(node))+1))')
    node='RoadBlocks_BackendNodePosition(RoadBlocks_BackendNode(node))';width='RoadBlocks_BackendNodeDividerWidth(RoadBlocks_BackendNode(node))'
    for op,amount in [('+','(length * 0.5f + '+width+')'),('+',width),('-',width),('-','(length * 0.5f + '+width+')'),('+','(length * 0.5f)'),('-','(length * 0.5f)')]:text=text.replace(node+' '+op+' '+amount+' * forward',('rb_add' if op=='+' else 'rb_sub')+'('+node+',rb_left('+amount+',forward))')
    text=text.replace('(RoadBlocks_aScriptRoadBlocks[i].GetPosition() - RoadBlocks_BackendPlayerCoors()).Magnitude()','rb_magnitude(rb_sub(ScriptRoadblock_GetPosition(&RoadBlocks_aScriptRoadBlocks[i]),RoadBlocks_BackendPlayerCoors()))')
    for name in ['COP_STREET','COP_FBI','COP_SWAT','COP_ARMY','TEMP_OBJECT','STREAMFLAGS_DONT_REMOVE','MISSION_NONE','TEMPACT_NONE']:text=re.sub(r'\b'+name+r'\b','ROADBLOCKS_'+name,text)
    text=text.replace('((CPed*)CPool_New(RoadBlocks_BackendPedPool()))','CPool_New(RoadBlocks_BackendPedPool())').replace('((CVehicle*)CPool_New(RoadBlocks_BackendVehiclePool()))','CPool_New(RoadBlocks_BackendVehiclePool())')
    # Retain native temporary lifetimes across every original early return
    start=text.index('RoadBlocks_GenerateRoadBlocks(void)');end=text.index('\nvoid',start);part=text[start:end].replace('return;','goto roadblocks_done;');part=part[:part.rindex('}')]+'''roadblocks_done:
    RoadBlocks_BackendMatrixDestroy(tmp2);
    RoadBlocks_BackendMatrixDestroy(tmp1);
}''';text=text[:start]+part+text[end:]
    start=text.index('RoadBlocks_CreateRoadBlockBetween2Points(');start=text.index('\n{',start) if text[start:].startswith('RoadBlocks_CreateRoadBlockBetween2Points(RoadBlockVector') else text.index('\n{',text.index('RoadBlocks_CreateRoadBlockBetween2Points(RoadBlockVector'))
    part=text[start:].replace('return;','goto roadblocks_done;');part=part[:part.rindex('}')]+'''roadblocks_done:
    RoadBlocks_BackendMatrixDestroy(tmp);
}
''';text=text[:start]+part
    for name in OPERATIONS:text=re.sub(r'(RoadBlocks_'+name+r'\([^\n]*\)\s*\n\{)',r'\1\n    // Preserve the complete original '+name+' policy and ordered native accesses',text,count=1)
    return '''//+ rouz edit (ChatGPT)
#include "RoadBlocks_backend.h"
#include "General.h"
#include "Timer.h"
#include <math.h>
#include <stdio.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define LANE_WIDTH 5.0f
#define Min(a,b) ((a)<(b)?(a):(b))
#undef assert
#ifdef MASTER
#define assert(expression) (expression)
#else
#define assert(expression) (void)((!!(expression))||(RoadBlocks_BackendAssertFailed(#expression,__FILE__,__LINE__,__func__),0))
#endif
'''+HELPERS+'\n'+text+'\n//- rouz edit (ChatGPT)\n'

def roadblocks_calls(source):
    # Remove both complete classes and project all roadblock callers without changing argument evaluation
    if 'class CRoadBlocks\n' in source:return HEADER
    if 'int16 CRoadBlocks::NumRoadBlocks;' in source:
        if OBSERVER:OBSERVER(source)
        return host()
    original=source
    for name in ['CreateRoadBlockBetween2Points','RegisterScriptRoadBlock']:
        token='CRoadBlocks::'+name+'(';pos=0
        while (index:=source.find(token,pos))>=0:
            end,args=arguments(source,index+len(token)-1);assert len(args)==2
            result='RoadBlocks_'+name+'('+','.join('RoadBlocks_FromVector('+v+')' for v in args)+')';source=source[:index]+result+source[end:];pos=index+len(result)
    source=source.replace('CRoadBlocks::','RoadBlocks_')
    if source==original:return source
    if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def roadblocks_project(source):
    # Register a unique C roadblock object and typed native boundary header
    for tag,old,new in [('ClCompile','RoadBlocks.cpp','RoadBlockSystem.c'),('ClInclude','RoadBlocks.h','RoadBlocks_backend.h')]:
        entry='<'+tag+' Include="..\\src\\control\\'+old+'" />';addition='<'+tag+' Include="..\\src\\control\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\control\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\control\\'+re.escape(old)+r'">\n\s*<Filter>control</Filter>\n\s*</'+tag+'>)';source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove complete relocation of the historical roadblock producer and canonical storage
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/control/RoadBlocks.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/control/RoadBlockSystem.c').read_text())

def verify_dependency(saved,current):
    # Permit only the explicit complete roadblock projection through historical dependency guards
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original);assert expected!=original and expected==current.read_text(),str(current)


def calls(source):
    # Preserve the complete roadblock projection before removing the complete debug display class
    import debug_display_migration as debug_display
    return debug_display.calls(roadblocks_calls(source))

def project(source):
    # Preserve previous registrations before adding the uniquely named debug display C object
    import debug_display_migration as debug_display
    return debug_display.project(roadblocks_project(source))
