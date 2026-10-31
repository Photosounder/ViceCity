//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
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
class CMatrix;class CEntity;class CCopPed;class CAutomobile;class CObject;struct CColModel;class CBaseModelInfo;struct CPathNode;struct CCarPathLink;
extern "C" {
#else
typedef struct CMatrix CMatrix;typedef struct CEntity CEntity;typedef struct CCopPed CCopPed;typedef struct CAutomobile CAutomobile;typedef struct CObject CObject;typedef struct CColModel CColModel;typedef struct CBaseModelInfo CBaseModelInfo;typedef struct CPathNode CPathNode;typedef struct CCarPathLink CCarPathLink;
#endif
int32_t RoadBlocks_BackendNumCarNodes(void);
CPathNode * RoadBlocks_BackendNode(int32_t node);
bool RoadBlocks_BackendNodeUse(CPathNode *node);
uint8_t RoadBlocks_BackendNodeLinks(CPathNode *node);
int16_t RoadBlocks_BackendNodeFirstLink(CPathNode *node);
bool RoadBlocks_BackendNodeDivider(CPathNode *node);
float RoadBlocks_BackendNodeDividerWidth(CPathNode *node);
RoadBlockVector RoadBlocks_BackendNodePosition(CPathNode *node);
int32_t RoadBlocks_BackendConnection(int32_t link);
CCarPathLink * RoadBlocks_BackendLink(int32_t link);
int8_t RoadBlocks_BackendLinkLeft(CCarPathLink *link);
int8_t RoadBlocks_BackendLinkRight(CCarPathLink *link);
float RoadBlocks_BackendLinkX(CCarPathLink *link);
float RoadBlocks_BackendLinkY(CCarPathLink *link);
RoadBlockVector RoadBlocks_BackendPlayerCoors(void);
CVehicle * RoadBlocks_BackendPlayerVehicle(void);
CEntity * RoadBlocks_BackendVehicleEntity(CVehicle *vehicle);
CEntity * RoadBlocks_BackendPlayerEntity(void);
CWanted * RoadBlocks_BackendWanted(void);
uint8_t RoadBlocks_BackendDensity(void);
bool RoadBlocks_BackendModelLoaded(int32_t model);
void RoadBlocks_BackendRequestModel(int32_t model,int32_t flags);
CColModel * RoadBlocks_BackendColModel(int32_t model);
CBaseModelInfo * RoadBlocks_BackendModelInfo(int32_t model);
CColModel * RoadBlocks_BackendInfoColModel(CBaseModelInfo *info);
float RoadBlocks_BackendColRadius(CColModel *model);
float RoadBlocks_BackendColMinZ(CColModel *model);
float RoadBlocks_BackendVehicleRadius(CVehicle *vehicle);
int32_t RoadBlocks_BackendVehicleModel(CVehicle *vehicle);
CPool * RoadBlocks_BackendPedPool(void);
CPool * RoadBlocks_BackendVehiclePool(void);
CPool * RoadBlocks_BackendObjectPool(void);
CCopPed * RoadBlocks_BackendConstructCop(CCopPed *cop,int32_t type);
CAutomobile * RoadBlocks_BackendConstructCar(CAutomobile *car,int32_t model,int32_t createdBy);
void RoadBlocks_BackendConstructObject(CObject *object,int32_t model,bool create);
void RoadBlocks_BackendDestroyCar(CAutomobile *car);
void RoadBlocks_BackendDestroyObject(CObject *object);
void RoadBlocks_BackendCopWeapon(CCopPed *cop,int32_t weapon);
void RoadBlocks_BackendPedZ(RoadBlockVector *point);
void RoadBlocks_BackendCopPosition(CCopPed *cop,RoadBlockVector point);
void RoadBlocks_BackendCopOrientation(CCopPed *cop,float x,float y,float z);
void RoadBlocks_BackendCopIdle(CCopPed *cop);
void RoadBlocks_BackendCopDisabled(CCopPed *cop,bool value);
void RoadBlocks_BackendCopStay(CCopPed *cop,bool value);
void RoadBlocks_BackendCopNoDuck(CCopPed *cop,bool value);
void RoadBlocks_BackendCopCrouch(CCopPed *cop,bool value);
void RoadBlocks_BackendCopCull(CCopPed *cop,bool value);
uint32_t * RoadBlocks_BackendCopRange(CCopPed *cop);
CVehicle * * RoadBlocks_BackendCopRoadVehicle(CCopPed *cop);
CVehicle * * RoadBlocks_BackendCopVehicle(CCopPed *cop);
void RoadBlocks_BackendCopRoadReference(CCopPed *cop);
void RoadBlocks_BackendCopVehicleReference(CVehicle *vehicle,CCopPed *cop);
void RoadBlocks_BackendCopLockOn(CCopPed *cop,CEntity *target);
void RoadBlocks_BackendCopAttack(CCopPed *cop,CEntity *target);
void RoadBlocks_BackendCopAlpha(CCopPed *entity,int32_t alpha);
void RoadBlocks_BackendAddCop(CCopPed *entity);
void RoadBlocks_BackendVehicleAlpha(CAutomobile *entity,int32_t alpha);
void RoadBlocks_BackendAddVehicle(CAutomobile *entity);
void RoadBlocks_BackendColliding(RoadBlockVector point,float radius,int16_t *count);
float RoadBlocks_BackendGroundZ(float x,float y,float z,bool *found);
void RoadBlocks_BackendCarStatus(CAutomobile *car,int32_t status);
void RoadBlocks_BackendCarMatrix(CAutomobile *car,CMatrix *matrix);
void RoadBlocks_BackendCarOnRoad(CAutomobile *car);
void RoadBlocks_BackendCarStatic(CAutomobile *car,bool value);
void RoadBlocks_BackendCarUpdateRW(CAutomobile *car);
void RoadBlocks_BackendCarLock(CAutomobile *car,int32_t value);
void RoadBlocks_BackendCarJoinRoad(CAutomobile *car);
void RoadBlocks_BackendCarLocked(CAutomobile *car,bool value);
void RoadBlocks_BackendCarExtended(CAutomobile *car,bool value);
void RoadBlocks_BackendCarSiren(CAutomobile *car,bool value);
void RoadBlocks_BackendCarCreatePeds(CAutomobile *car,bool value);
uint8_t * RoadBlocks_BackendCarMission(CAutomobile *car);
uint8_t * RoadBlocks_BackendCarTempAction(CAutomobile *car);
int8_t * RoadBlocks_BackendCarNextLane(CAutomobile *car);
int8_t * RoadBlocks_BackendCarCurrentLane(CAutomobile *car);
uint8_t * RoadBlocks_BackendCarCruiseSpeed(CAutomobile *car);
float * RoadBlocks_BackendCarMaxSpeed(CAutomobile *car);
uint8_t * RoadBlocks_BackendCarRoadblockType(CAutomobile *car);
uint32_t * RoadBlocks_BackendCarRange(CAutomobile *car);
bool RoadBlocks_BackendCarUsesSiren(CAutomobile *car);
float RoadBlocks_BackendCarUpZ(CAutomobile *car);
int8_t RoadBlocks_BackendObjectCreated(CObject *object);
RoadBlockVector RoadBlocks_BackendCarRight(CAutomobile *car);
RoadBlockVector RoadBlocks_BackendCarPosition(CAutomobile *car);
void RoadBlocks_BackendRemoveObject(CObject *object);
void RoadBlocks_BackendObjectMatrix(CObject *object,CMatrix *matrix);
void RoadBlocks_BackendObjectCreatedSet(CObject *object,int8_t value);
uint32_t * RoadBlocks_BackendObjectLife(CObject *object);
void RoadBlocks_BackendAddObject(CObject *object);
CMatrix * RoadBlocks_BackendMatrixConstruct(RoadBlockMatrixStorage *storage);
void RoadBlocks_BackendMatrixDestroy(CMatrix *matrix);
void RoadBlocks_BackendMatrixTranslate(CMatrix *matrix,float x,float y,float z);
void RoadBlocks_BackendMatrixTranslateOnly(CMatrix *matrix,RoadBlockVector point);
void RoadBlocks_BackendMatrixRotateZ(CMatrix *matrix,float angle);
void RoadBlocks_BackendMatrixRight(CMatrix *matrix,RoadBlockVector point);
void RoadBlocks_BackendMatrixForward(CMatrix *matrix,RoadBlockVector point);
void RoadBlocks_BackendMatrixUp(CMatrix *matrix,RoadBlockVector point);
RoadBlockVector RoadBlocks_BackendMatrixPosition(CMatrix *matrix);
float * RoadBlocks_BackendMatrixX(CMatrix *matrix);
float * RoadBlocks_BackendMatrixY(CMatrix *matrix);
float * RoadBlocks_BackendMatrixZ(CMatrix *matrix);
RoadBlockVector RoadBlocks_BackendVehicleTransform(CVehicle *vehicle,RoadBlockVector point);
void RoadBlocks_BackendAssertFailed(const char *expression,const char *file,unsigned line,const char *function);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
