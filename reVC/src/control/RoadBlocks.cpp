//+ rouz edit (ChatGPT)
#include "common.h"
#include "RoadBlocks_backend.h"
#include "PathFind.h"
#include "Streaming.h"
#include "World.h"
#include "PedPlacement.h"
#include "Automobile.h"
#include "CopPed.h"
#include "VisibilityPlugins.h"
#include "PlayerPed.h"
#include "CarCtrl.h"
#include "Object.h"
#include "Pools.h"
static_assert(sizeof(CMatrix)<=sizeof(RoadBlockMatrixStorage),"Native matrix storage extent");
static_assert(alignof(CMatrix)<=alignof(RoadBlockMatrixStorage),"Native matrix storage alignment");
static_assert(COP_STREET==0,"Original COP_STREET constant");
static_assert(COP_FBI==1,"Original COP_FBI constant");
static_assert(COP_SWAT==2,"Original COP_SWAT constant");
static_assert(COP_ARMY==4,"Original COP_ARMY constant");
static_assert(RANDOM_VEHICLE==1,"Original RANDOM_VEHICLE constant");
static_assert(CARLOCK_UNLOCKED==1,"Original CARLOCK_UNLOCKED constant");
static_assert(MISSION_NONE==0,"Original MISSION_NONE constant");
static_assert(TEMPACT_NONE==0,"Original TEMPACT_NONE constant");
static_assert(TEMP_OBJECT==3,"Original TEMP_OBJECT constant");
static_assert(STREAMFLAGS_DONT_REMOVE==1,"Original STREAMFLAGS_DONT_REMOVE constant");
RoadBlockVector RoadBlocks_FromVector(CVector value)
{
    // Copy the original three float components at the native argument boundary
    RoadBlockVector result={value.x,value.y,value.z};return result;
}
int32_t RoadBlocks_BackendNumCarNodes(void)
{
    // Access the original typed primitive at its original C policy access point
    return ThePaths.m_numCarPathNodes;
}

CPathNode * RoadBlocks_BackendNode(int32_t node)
{
    // Access the original typed primitive at its original C policy access point
    return &ThePaths.m_pathNodes[node];
}

bool RoadBlocks_BackendNodeUse(CPathNode *node)
{
    // Access the original typed primitive at its original C policy access point
    return node->bUseInRoadBlock;
}

uint8_t RoadBlocks_BackendNodeLinks(CPathNode *node)
{
    // Access the original typed primitive at its original C policy access point
    return node->numLinks;
}

int16_t RoadBlocks_BackendNodeFirstLink(CPathNode *node)
{
    // Access the original typed primitive at its original C policy access point
    return node->firstLink;
}

bool RoadBlocks_BackendNodeDivider(CPathNode *node)
{
    // Access the original typed primitive at its original C policy access point
    return node->HasDivider();
}

float RoadBlocks_BackendNodeDividerWidth(CPathNode *node)
{
    // Access the original typed primitive at its original C policy access point
    return node->GetDividerWidth();
}

RoadBlockVector RoadBlocks_BackendNodePosition(CPathNode *node)
{
    // Access the original typed primitive at its original C policy access point
    return RoadBlocks_FromVector(node->GetPosition());
}

int32_t RoadBlocks_BackendConnection(int32_t link)
{
    // Access the original typed primitive at its original C policy access point
    return ThePaths.m_carPathConnections[link];
}

CCarPathLink * RoadBlocks_BackendLink(int32_t link)
{
    // Access the original typed primitive at its original C policy access point
    return &ThePaths.m_carPathLinks[link];
}

int8_t RoadBlocks_BackendLinkLeft(CCarPathLink *link)
{
    // Access the original typed primitive at its original C policy access point
    return link->numLeftLanes;
}

int8_t RoadBlocks_BackendLinkRight(CCarPathLink *link)
{
    // Access the original typed primitive at its original C policy access point
    return link->numRightLanes;
}

float RoadBlocks_BackendLinkX(CCarPathLink *link)
{
    // Access the original typed primitive at its original C policy access point
    return link->GetX();
}

float RoadBlocks_BackendLinkY(CCarPathLink *link)
{
    // Access the original typed primitive at its original C policy access point
    return link->GetY();
}

RoadBlockVector RoadBlocks_BackendPlayerCoors(void)
{
    // Access the original typed primitive at its original C policy access point
    return RoadBlocks_FromVector(FindPlayerCoors());
}

CVehicle * RoadBlocks_BackendPlayerVehicle(void)
{
    // Access the original typed primitive at its original C policy access point
    return FindPlayerVehicle();
}

CEntity * RoadBlocks_BackendVehicleEntity(CVehicle *vehicle)
{
    // Access the original typed primitive at its original C policy access point
    return (CEntity*)vehicle;
}

CEntity * RoadBlocks_BackendPlayerEntity(void)
{
    // Access the original typed primitive at its original C policy access point
    return (CEntity*)FindPlayerPed();
}

CWanted * RoadBlocks_BackendWanted(void)
{
    // Access the original typed primitive at its original C policy access point
    return FindPlayerPed()->m_pWanted;
}

uint8_t RoadBlocks_BackendDensity(void)
{
    // Access the original typed primitive at its original C policy access point
    return FindPlayerPed()->m_pWanted->m_RoadblockDensity;
}

bool RoadBlocks_BackendModelLoaded(int32_t model)
{
    // Access the original typed primitive at its original C policy access point
    return Streaming_HasModelLoaded(model); // rouz edit (ChatGPT)
}

void RoadBlocks_BackendRequestModel(int32_t model,int32_t flags)
{
    // Access the original typed primitive at its original C policy access point
    Streaming_RequestModel(model,flags); // rouz edit (ChatGPT)
}

float RoadBlocks_BackendColRadius(CColModel *model)
{
    // Access the original typed primitive at its original C policy access point
    return model->boundingSphere.radius;
}

float RoadBlocks_BackendColMinZ(CColModel *model)
{
    // Access the original typed primitive at its original C policy access point
    return model->boundingBox.min.z;
}

float RoadBlocks_BackendVehicleRadius(CVehicle *vehicle)
{
    // Access the original typed primitive at its original C policy access point
    return vehicle->GetBoundRadius();
}

int32_t RoadBlocks_BackendVehicleModel(CVehicle *vehicle)
{
    // Access the original typed primitive at its original C policy access point
    return vehicle->GetModelIndex();
}

CPool * RoadBlocks_BackendPedPool(void)
{
    // Access the original typed primitive at its original C policy access point
    return CPools::GetPedPool();
}

CPool * RoadBlocks_BackendVehiclePool(void)
{
    // Access the original typed primitive at its original C policy access point
    return CPools::GetVehiclePool();
}

CPool * RoadBlocks_BackendObjectPool(void)
{
    // Access the original typed primitive at its original C policy access point
    return CPools::GetObjectPool();
}

CCopPed * RoadBlocks_BackendConstructCop(CCopPed *cop,int32_t type)
{
    // Access the original typed primitive at its original C policy access point
    std::allocator<CCopPed>().construct(cop,(eCopType)type);return cop;
}

CAutomobile * RoadBlocks_BackendConstructCar(CAutomobile *car,int32_t model,int32_t createdBy)
{
    // Access the original typed primitive at its original C policy access point
    std::allocator<CAutomobile>().construct(car,model,createdBy);return car;
}

void RoadBlocks_BackendConstructObject(CObject *object,int32_t model,bool create)
{
    // Access the original typed primitive at its original C policy access point
    std::allocator<CObject>().construct(object,model,create);
}

void RoadBlocks_BackendDestroyCar(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    car->~CAutomobile();
}

void RoadBlocks_BackendDestroyObject(CObject *object)
{
    // Access the original typed primitive at its original C policy access point
    object->~CObject();
}

void RoadBlocks_BackendCopWeapon(CCopPed *cop,int32_t weapon)
{
    // Access the original typed primitive at its original C policy access point
    cop->SetCurrentWeapon((eWeaponType)weapon);
}

void RoadBlocks_BackendCopPosition(CCopPed *cop,RoadBlockVector point)
{
    // Access the original typed primitive at its original C policy access point
    cop->SetPosition(CVector(point.x,point.y,point.z));
}

void RoadBlocks_BackendCopOrientation(CCopPed *cop,float x,float y,float z)
{
    // Access the original typed primitive at its original C policy access point
    cop->SetOrientation(x,y,z);
}

void RoadBlocks_BackendCopIdle(CCopPed *cop)
{
    // Access the original typed primitive at its original C policy access point
    cop->SetIdle();
}

void RoadBlocks_BackendCopDisabled(CCopPed *cop,bool value)
{
    // Access the original typed primitive at its original C policy access point
    cop->m_bIsDisabledCop=value;
}

void RoadBlocks_BackendCopStay(CCopPed *cop,bool value)
{
    // Access the original typed primitive at its original C policy access point
    cop->bKindaStayInSamePlace=value;
}

void RoadBlocks_BackendCopNoDuck(CCopPed *cop,bool value)
{
    // Access the original typed primitive at its original C policy access point
    cop->bNotAllowedToDuck=value;
}

void RoadBlocks_BackendCopCrouch(CCopPed *cop,bool value)
{
    // Access the original typed primitive at its original C policy access point
    cop->bCrouchWhenShooting=value;
}

void RoadBlocks_BackendCopCull(CCopPed *cop,bool value)
{
    // Access the original typed primitive at its original C policy access point
    cop->bCullExtraFarAway=value;
}

uint32_t * RoadBlocks_BackendCopRange(CCopPed *cop)
{
    // Access the original typed primitive at its original C policy access point
    return &cop->m_nExtendedRangeTimer;
}

CVehicle * * RoadBlocks_BackendCopRoadVehicle(CCopPed *cop)
{
    // Access the original typed primitive at its original C policy access point
    return &cop->m_nRoadblockVeh;
}

CVehicle * * RoadBlocks_BackendCopVehicle(CCopPed *cop)
{
    // Access the original typed primitive at its original C policy access point
    return &cop->m_pMyVehicle;
}

void RoadBlocks_BackendCopRoadReference(CCopPed *cop)
{
    // Access the original typed primitive at its original C policy access point
    cop->m_nRoadblockVeh->RegisterReference((CEntity**)&cop->m_nRoadblockVeh);
}

void RoadBlocks_BackendCopVehicleReference(CVehicle *vehicle,CCopPed *cop)
{
    // Access the original typed primitive at its original C policy access point
    vehicle->RegisterReference((CEntity**)&cop->m_pMyVehicle);
}

void RoadBlocks_BackendCopLockOn(CCopPed *cop,CEntity *target)
{
    // Access the original typed primitive at its original C policy access point
    cop->SetWeaponLockOnTarget(target);
}

void RoadBlocks_BackendCopAttack(CCopPed *cop,CEntity *target)
{
    // Access the original typed primitive at its original C policy access point
    cop->SetAttack(target);
}

void RoadBlocks_BackendCopAlpha(CCopPed *entity,int32_t alpha)
{
    // Access the original typed primitive at its original C policy access point
    CVisibilityPlugins::SetClumpAlpha(entity->GetClump(),alpha);
}

void RoadBlocks_BackendAddCop(CCopPed *entity)
{
    // Access the original typed primitive at its original C policy access point
    CWorld::Add(entity);
}

void RoadBlocks_BackendVehicleAlpha(CAutomobile *entity,int32_t alpha)
{
    // Access the original typed primitive at its original C policy access point
    CVisibilityPlugins::SetClumpAlpha(entity->GetClump(),alpha);
}

void RoadBlocks_BackendAddVehicle(CAutomobile *entity)
{
    // Access the original typed primitive at its original C policy access point
    CWorld::Add(entity);
}

void RoadBlocks_BackendColliding(RoadBlockVector point,float radius,int16_t *count)
{
    // Access the original typed primitive at its original C policy access point
    CWorld::FindObjectsKindaColliding(CVector(point.x,point.y,point.z),radius,0,count,2,nil,false,true,true,false,false);
}

float RoadBlocks_BackendGroundZ(float x,float y,float z,bool *found)
{
    // Access the original typed primitive at its original C policy access point
    return CWorld::FindGroundZFor3DCoord(x,y,z,found);
}

void RoadBlocks_BackendCarStatus(CAutomobile *car,int32_t status)
{
    // Access the original typed primitive at its original C policy access point
    car->SetStatus((eEntityStatus)status);
}

void RoadBlocks_BackendCarMatrix(CAutomobile *car,CMatrix *matrix)
{
    // Access the original typed primitive at its original C policy access point
    car->SetMatrix(*matrix);
}

void RoadBlocks_BackendCarOnRoad(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    car->PlaceOnRoadProperly();
}

void RoadBlocks_BackendCarStatic(CAutomobile *car,bool value)
{
    // Access the original typed primitive at its original C policy access point
    car->SetIsStatic(value);
}

void RoadBlocks_BackendCarUpdateRW(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    car->GetMatrix().UpdateRW();
}

void RoadBlocks_BackendCarLock(CAutomobile *car,int32_t value)
{
    // Access the original typed primitive at its original C policy access point
    car->m_nDoorLock=(eCarLock)value;
}

void RoadBlocks_BackendCarJoinRoad(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    CCarCtrl::JoinCarWithRoadSystem(car);
}

void RoadBlocks_BackendCarLocked(CAutomobile *car,bool value)
{
    // Access the original typed primitive at its original C policy access point
    car->bIsLocked=value;
}

void RoadBlocks_BackendCarExtended(CAutomobile *car,bool value)
{
    // Access the original typed primitive at its original C policy access point
    car->bExtendedRange=value;
}

void RoadBlocks_BackendCarSiren(CAutomobile *car,bool value)
{
    // Access the original typed primitive at its original C policy access point
    car->m_bSirenOrAlarm=value;
}

void RoadBlocks_BackendCarCreatePeds(CAutomobile *car,bool value)
{
    // Access the original typed primitive at its original C policy access point
    car->bCreateRoadBlockPeds=value;
}

uint8_t * RoadBlocks_BackendCarMission(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return &car->AutoPilot.m_nCarMission;
}

uint8_t * RoadBlocks_BackendCarTempAction(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return &car->AutoPilot.m_nTempAction;
}

int8_t * RoadBlocks_BackendCarNextLane(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return &car->AutoPilot.m_nNextLane;
}

int8_t * RoadBlocks_BackendCarCurrentLane(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return &car->AutoPilot.m_nCurrentLane;
}

uint8_t * RoadBlocks_BackendCarCruiseSpeed(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return &car->AutoPilot.m_nCruiseSpeed;
}

float * RoadBlocks_BackendCarMaxSpeed(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return &car->AutoPilot.m_fMaxTrafficSpeed;
}

uint8_t * RoadBlocks_BackendCarRoadblockType(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return &car->m_nRoadblockType;
}

uint32_t * RoadBlocks_BackendCarRange(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return &car->m_nSetPieceExtendedRangeTime;
}

bool RoadBlocks_BackendCarUsesSiren(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return car->UsesSiren();
}

float RoadBlocks_BackendCarUpZ(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return car->GetUp().z; // rouz edit (ChatGPT)
}

int8_t RoadBlocks_BackendObjectCreated(CObject *object)
{
    // Access the original typed primitive at its original C policy access point
    return object->ObjectCreatedBy;
}

RoadBlockVector RoadBlocks_BackendCarRight(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return RoadBlocks_FromVector(car->GetRight()); // rouz edit (ChatGPT)
}

RoadBlockVector RoadBlocks_BackendCarPosition(CAutomobile *car)
{
    // Access the original typed primitive at its original C policy access point
    return RoadBlocks_FromVector(car->GetPosition());
}

void RoadBlocks_BackendRemoveObject(CObject *object)
{
    // Access the original typed primitive at its original C policy access point
    CWorld::Remove(object);
}

void RoadBlocks_BackendObjectMatrix(CObject *object,CMatrix *matrix)
{
    // Access the original typed primitive at its original C policy access point
    object->GetMatrix()=*matrix;
}

void RoadBlocks_BackendObjectCreatedSet(CObject *object,int8_t value)
{
    // Access the original typed primitive at its original C policy access point
    object->ObjectCreatedBy=value;
}

uint32_t * RoadBlocks_BackendObjectLife(CObject *object)
{
    // Access the original typed primitive at its original C policy access point
    return &object->m_nEndOfLifeTime;
}

void RoadBlocks_BackendAddObject(CObject *object)
{
    // Access the original typed primitive at its original C policy access point
    CWorld::Add(object);
}

CMatrix * RoadBlocks_BackendMatrixConstruct(RoadBlockMatrixStorage *storage)
{
    // Access the original typed primitive at its original C policy access point
    CMatrix *matrix=(CMatrix*)storage;std::allocator<CMatrix>().construct(matrix);return matrix;
}

void RoadBlocks_BackendMatrixDestroy(CMatrix *matrix)
{
    // Access the original typed primitive at its original C policy access point
    matrix->~CMatrix();
}

void RoadBlocks_BackendMatrixTranslate(CMatrix *matrix,float x,float y,float z)
{
    // Access the original typed primitive at its original C policy access point
    matrix->SetTranslate(x,y,z);
}

void RoadBlocks_BackendMatrixTranslateOnly(CMatrix *matrix,RoadBlockVector point)
{
    // Access the original typed primitive at its original C policy access point
    matrix->SetTranslateOnly(CVector(point.x,point.y,point.z));
}

void RoadBlocks_BackendMatrixRotateZ(CMatrix *matrix,float angle)
{
    // Access the original typed primitive at its original C policy access point
    matrix->RotateZ(angle);
}

void RoadBlocks_BackendMatrixRight(CMatrix *matrix,RoadBlockVector point)
{
    // Access the original typed primitive at its original C policy access point
    matrix->GetRight()=CVector(point.x,point.y,point.z); // rouz edit (ChatGPT)
}

void RoadBlocks_BackendMatrixForward(CMatrix *matrix,RoadBlockVector point)
{
    // Access the original typed primitive at its original C policy access point
    matrix->GetForward()=CVector(point.x,point.y,point.z);
}

void RoadBlocks_BackendMatrixUp(CMatrix *matrix,RoadBlockVector point)
{
    // Access the original typed primitive at its original C policy access point
    matrix->GetUp()=CVector(point.x,point.y,point.z); // rouz edit (ChatGPT)
}

RoadBlockVector RoadBlocks_BackendMatrixPosition(CMatrix *matrix)
{
    // Access the original typed primitive at its original C policy access point
    return RoadBlocks_FromVector(matrix->GetPosition());
}

float * RoadBlocks_BackendMatrixX(CMatrix *matrix)
{
    // Access the original typed primitive at its original C policy access point
    return &matrix->GetPosition().x;
}

float * RoadBlocks_BackendMatrixY(CMatrix *matrix)
{
    // Access the original typed primitive at its original C policy access point
    return &matrix->GetPosition().y;
}

float * RoadBlocks_BackendMatrixZ(CMatrix *matrix)
{
    // Access the original typed primitive at its original C policy access point
    return &matrix->GetPosition().z;
}

RoadBlockVector RoadBlocks_BackendVehicleTransform(CVehicle *vehicle,RoadBlockVector point)
{
    // Access the original typed primitive at its original C policy access point
    return RoadBlocks_FromVector(vehicle->GetMatrix()*CVector(point.x,point.y,point.z));
}

void RoadBlocks_BackendAssertFailed(const char *expression,const char *file,unsigned line,const char *function)
{
    // Access the original typed primitive at its original C policy access point
    re3_assert(expression,file,line,function);
}
//- rouz edit (ChatGPT)
