//+ rouz edit (ChatGPT)
#include "ModelRegistry.h"
#include "RoadBlocks_backend.h"
#include "PedPlacement.h" // rouz edit (ChatGPT)
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
static RoadBlockVector rb_vec(float x,float y,float z)
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

#define ROADBLOCKDIST (90.0f)
#define ROADBLOCK_OBJECT_WIDTH (4.0f)

int16_t RoadBlocks_NumRoadBlocks;
int16_t RoadBlocks_RoadBlockNodes[NUMROADBLOCKS];
bool RoadBlocks_InOrOut[NUMROADBLOCKS];
CScriptRoadblock RoadBlocks_aScriptRoadBlocks[NUM_SCRIPT_ROADBLOCKS];

#ifdef SECUROM
uint8_t roadBlocksPirateCheck = 0;
#endif

void
RoadBlocks_Init(void)
{
    // Preserve the complete original Init policy and ordered native accesses
	int i;
	RoadBlocks_NumRoadBlocks = 0;
	for(i = 0; i < RoadBlocks_BackendNumCarNodes(); i++){
		if(RoadBlocks_BackendNodeUse(RoadBlocks_BackendNode(i)) && RoadBlocks_BackendNodeLinks(RoadBlocks_BackendNode(i)) == 2){
			if (RoadBlocks_NumRoadBlocks < NUMROADBLOCKS) {
				RoadBlocks_InOrOut[RoadBlocks_NumRoadBlocks] = true;
				RoadBlocks_RoadBlockNodes[RoadBlocks_NumRoadBlocks] = i;
				RoadBlocks_NumRoadBlocks++;
			} else {
#ifndef MASTER
				printf("Not enough room for the potential roadblocks\n");
#endif
				// FIX: Don't iterate loop after NUMROADBLOCKS
				return;
			}
		}
	}

	RoadBlocks_ClearScriptRoadBlocks();
}

void
RoadBlocks_GenerateRoadBlockCopsForCar(CVehicle* pVehicle, int32_t roadBlockType)
{
    // Preserve the complete original GenerateRoadBlockCopsForCar policy and ordered native accesses
	static const RoadBlockVector vecRoadBlockOffets[6] = {{-1.5,1.8f,0.0f},{-1.5f,-1.8f,0.0f},{1.5f,1.8f,0.0f},{1.5f,-1.8f,0.0f},{-1.5f,0.0f,0.0f},{1.5,0.0,0.0}};
	CEntity* pEntityToAttack = RoadBlocks_BackendVehicleEntity(RoadBlocks_BackendPlayerVehicle());
	if (!pEntityToAttack)
		pEntityToAttack = RoadBlocks_BackendPlayerEntity();
	CColModel* pPoliceColModel = ModelRegistry_Collision(ModelRegistry_Get(MI_POLICE));
	float fRadius = RoadBlocks_BackendVehicleRadius(pVehicle) / RoadBlocks_BackendColRadius(pPoliceColModel);
	for (int32_t i = 0; i < 2; i++) {
		const int32_t roadBlockIndex = i + 2 * roadBlockType;
		RoadBlockVector posForZ = RoadBlocks_BackendVehicleTransform(pVehicle,rb_left(fRadius,vecRoadBlockOffets[roadBlockIndex]));
		int32_t modelInfoId = MI_COP;
		int32_t copType = ROADBLOCKS_COP_STREET;
		switch (RoadBlocks_BackendVehicleModel(pVehicle))
		{
		case MI_FBIRANCH:
			modelInfoId = MI_FBI;
			copType = ROADBLOCKS_COP_FBI;
			break;
		case MI_ENFORCER:
			modelInfoId = MI_SWAT;
			copType = ROADBLOCKS_COP_SWAT;
			break;
		case MI_BARRACKS:
			modelInfoId = MI_ARMY;
			copType = ROADBLOCKS_COP_ARMY;
			break;
		}
		if (!RoadBlocks_BackendModelLoaded(modelInfoId))
			copType = ROADBLOCKS_COP_STREET;
//+ rouz edit (ChatGPT)
		// Allocate the roadblock cop without invoking C++ new.
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CCopPed* pCopPed = (CCopPed*)CPool_New(RoadBlocks_BackendPedPool());
		//- rouz edit (ChatGPT)
		assert(pCopPed);
		RoadBlocks_BackendConstructCop(pCopPed,copType);
//- rouz edit (ChatGPT)
		if (copType == ROADBLOCKS_COP_STREET)
			RoadBlocks_BackendCopWeapon(pCopPed,WEAPONTYPE_COLT45);
		PedPlacement_CFindZ(posForZ.x,posForZ.y,&posForZ.z); // rouz edit (ChatGPT)
		RoadBlocks_BackendCopPosition(pCopPed,posForZ);
		RoadBlocks_BackendCopOrientation(pCopPed,0.0f,0.0f,-HALFPI);
		RoadBlocks_BackendCopDisabled(pCopPed,true);
		RoadBlocks_BackendCopIdle(pCopPed);
		RoadBlocks_BackendCopStay(pCopPed,true);
		RoadBlocks_BackendCopNoDuck(pCopPed,false);
		(*RoadBlocks_BackendCopRange(pCopPed)) = (*Timer_GetTimeInMilliseconds()) + 10000;
		(*RoadBlocks_BackendCopRoadVehicle(pCopPed)) = pVehicle;
		RoadBlocks_BackendCopRoadReference(pCopPed);
		RoadBlocks_BackendCopCrouch(pCopPed,roadBlockType == 2 ? false : true);
		if (pEntityToAttack) {
			RoadBlocks_BackendCopLockOn(pCopPed,pEntityToAttack);
			RoadBlocks_BackendCopAttack(pCopPed,pEntityToAttack);
		}
		(*RoadBlocks_BackendCopVehicle(pCopPed)) = pVehicle;
		RoadBlocks_BackendCopVehicleReference(pVehicle,pCopPed);
		RoadBlocks_BackendCopCull(pCopPed,true);
		RoadBlocks_BackendCopAlpha(pCopPed,0);
		RoadBlocks_BackendAddCop(pCopPed);
	}
}

void 
RoadBlocks_GenerateRoadBlocks(void) 
{
    // Preserve the complete original GenerateRoadBlocks policy and ordered native accesses 
	RoadBlockMatrixStorage storage1,storage2;
    CMatrix *tmp1=RoadBlocks_BackendMatrixConstruct(&storage1),*tmp2=RoadBlocks_BackendMatrixConstruct(&storage2);
	static int16_t unk;
#ifdef SQUEEZE_PERFORMANCE
	if (RoadBlocks_BackendDensity() == 0)
		goto roadblocks_done;
#endif
	uint32_t frame = (*Timer_GetFrameCounter()) & 0xF;
	int16_t nRoadblockNode = (int16_t)(NUMROADBLOCKS * frame) / 16;
	const int16_t maxRoadBlocks = (int16_t)(NUMROADBLOCKS * (frame + 1)) / 16;
	for (; nRoadblockNode < Min(RoadBlocks_NumRoadBlocks, maxRoadBlocks); nRoadblockNode++) {
		int16_t node = RoadBlocks_RoadBlockNodes[nRoadblockNode];
		RoadBlockVector vecDistance = rb_sub(RoadBlocks_BackendPlayerCoors(),RoadBlocks_BackendNodePosition(RoadBlocks_BackendNode(node)));
		if (vecDistance.x > -ROADBLOCKDIST && vecDistance.x < ROADBLOCKDIST &&
			vecDistance.y > -ROADBLOCKDIST && vecDistance.y < ROADBLOCKDIST &&
			rb_magnitude2(vecDistance) < ROADBLOCKDIST) {
			if (!RoadBlocks_InOrOut[nRoadblockNode]) {
				RoadBlocks_InOrOut[nRoadblockNode] = true;
				if (RoadBlocks_BackendPlayerVehicle() && (General_GetRandomNumber() & 0x7F) < RoadBlocks_BackendDensity()) {
					CCarPathLink* pLink1 = RoadBlocks_BackendLink(RoadBlocks_BackendConnection(RoadBlocks_BackendNodeFirstLink(RoadBlocks_BackendNode(node))));
					CCarPathLink* pLink2 = RoadBlocks_BackendLink(RoadBlocks_BackendConnection(RoadBlocks_BackendNodeFirstLink(RoadBlocks_BackendNode(node))+1));
					int lanes = Min(RoadBlocks_BackendLinkRight(pLink1) + RoadBlocks_BackendLinkLeft(pLink1), RoadBlocks_BackendLinkLeft(pLink2) + RoadBlocks_BackendLinkRight(pLink2));
					float length = LANE_WIDTH * (lanes + 1);
					RoadBlockVector forward=rb_vec(RoadBlocks_BackendLinkY(pLink2)-RoadBlocks_BackendLinkY(pLink1),-(RoadBlocks_BackendLinkX(pLink2)-RoadBlocks_BackendLinkX(pLink1)),0.0f);
					rb_normalise(&forward);
					if (RoadBlocks_BackendNodeDivider(RoadBlocks_BackendNode(node))) {
						RoadBlocks_CreateRoadBlockBetween2Points(
							rb_add(RoadBlocks_BackendNodePosition(RoadBlocks_BackendNode(node)),rb_left((length * 0.5f + RoadBlocks_BackendNodeDividerWidth(RoadBlocks_BackendNode(node))),forward)),
							rb_add(RoadBlocks_BackendNodePosition(RoadBlocks_BackendNode(node)),rb_left(RoadBlocks_BackendNodeDividerWidth(RoadBlocks_BackendNode(node)),forward)));
						RoadBlocks_CreateRoadBlockBetween2Points(
							rb_sub(RoadBlocks_BackendNodePosition(RoadBlocks_BackendNode(node)),rb_left(RoadBlocks_BackendNodeDividerWidth(RoadBlocks_BackendNode(node)),forward)),
							rb_sub(RoadBlocks_BackendNodePosition(RoadBlocks_BackendNode(node)),rb_left((length * 0.5f + RoadBlocks_BackendNodeDividerWidth(RoadBlocks_BackendNode(node))),forward)));
					}
					else {
						RoadBlocks_CreateRoadBlockBetween2Points(
							rb_add(RoadBlocks_BackendNodePosition(RoadBlocks_BackendNode(node)),rb_left((length * 0.5f),forward)),
							rb_sub(RoadBlocks_BackendNodePosition(RoadBlocks_BackendNode(node)),rb_left((length * 0.5f),forward)));
					}
				}
			}
		}
		else {
			RoadBlocks_InOrOut[nRoadblockNode] = false;
		}
	}
	int i = (*Timer_GetFrameCounter()) & 0xF;
	if (!RoadBlocks_aScriptRoadBlocks[i].m_bInUse)
		goto roadblocks_done;
	if (rb_magnitude(rb_sub(ScriptRoadblock_GetPosition(&RoadBlocks_aScriptRoadBlocks[i]),RoadBlocks_BackendPlayerCoors())) < 100.0f) {
		RoadBlocks_CreateRoadBlockBetween2Points(RoadBlocks_aScriptRoadBlocks[i].m_vInf, RoadBlocks_aScriptRoadBlocks[i].m_vSup);
		RoadBlocks_aScriptRoadBlocks[i].m_bInUse = false;
	}
roadblocks_done:
    RoadBlocks_BackendMatrixDestroy(tmp2);
    RoadBlocks_BackendMatrixDestroy(tmp1);
}
void
RoadBlocks_ClearScriptRoadBlocks(void)
{
    // Preserve the complete original ClearScriptRoadBlocks policy and ordered native accesses
	for (int i = 0; i < NUM_SCRIPT_ROADBLOCKS; i++)
		RoadBlocks_aScriptRoadBlocks[i].m_bInUse = false;
}

void
RoadBlocks_RegisterScriptRoadBlock(RoadBlockVector vInf, RoadBlockVector vSup)
{
    // Preserve the complete original RegisterScriptRoadBlock policy and ordered native accesses
	int32_t i;
	for (i = 0; i < NUM_SCRIPT_ROADBLOCKS; i++) {
		if (!RoadBlocks_aScriptRoadBlocks[i].m_bInUse)
			break;
	}
	if (i == NUM_SCRIPT_ROADBLOCKS)
		return;
	RoadBlocks_aScriptRoadBlocks[i].m_bInUse = true;
	RoadBlocks_aScriptRoadBlocks[i].m_vInf = vInf;
	RoadBlocks_aScriptRoadBlocks[i].m_vSup = vSup;
}

void 
RoadBlocks_CreateRoadBlockBetween2Points(RoadBlockVector point1, RoadBlockVector point2)
{
    // Preserve the complete original CreateRoadBlockBetween2Points policy and ordered native accesses
#ifdef SECUROM
	if (roadBlocksPirateCheck == 0)
		// if not pirated game
		// roadBlocksPirateCheck = 1;
		// else
		roadBlocksPirateCheck = 2;
#endif
	RoadBlockMatrixStorage storage;
    CMatrix *tmp=RoadBlocks_BackendMatrixConstruct(&storage);
	RoadBlockVector forward = rb_sub(point2,point1);
	float distBetween = rb_magnitude(forward);
	RoadBlockVector pos = rb_div(rb_add(point1,point2),2);
	RoadBlockVector right=rb_vec(forward.y,-forward.x,0.0f);
	rb_normalise(&forward);
	rb_normalise(&right);
	if (rb_dot(rb_sub(RoadBlocks_BackendPlayerCoors(),pos),right) < 0.0f) {
		right = rb_scale(right,-1.0f);
	}
	int32_t vehicleId = MI_POLICE;
	if (Wanted_AreArmyRequired(RoadBlocks_BackendWanted()))
		vehicleId = MI_BARRACKS;
	else if (Wanted_AreFbiRequired(RoadBlocks_BackendWanted()))
		vehicleId = MI_FBICAR;
	else if (Wanted_AreSwatRequired(RoadBlocks_BackendWanted()))
		vehicleId = MI_ENFORCER;
	if (!RoadBlocks_BackendModelLoaded(vehicleId))
		vehicleId = MI_POLICE;
	CColModel *pVehicleColModel = ModelRegistry_Collision(ModelRegistry_Get(vehicleId));
	float fModelRadius = 2.0f * RoadBlocks_BackendColRadius(pVehicleColModel) + 0.25f;
	int16_t numRoadblockVehicles = Min(6, (int16_t)(distBetween / fModelRadius));
	for (int16_t i = 0; i < numRoadblockVehicles; i++) {
		float offset = fModelRadius * (i - numRoadblockVehicles / 2);
		RoadBlocks_BackendMatrixTranslate(tmp,0.0f,0.0f,0.0f);
		RoadBlocks_BackendMatrixRight(tmp,rb_vec(forward.y,-forward.x,0.0f));
		RoadBlocks_BackendMatrixForward(tmp,forward);
		RoadBlocks_BackendMatrixUp(tmp,rb_vec(0.0f,0.0f,1.0f));
		RoadBlocks_BackendMatrixRotateZ(tmp,((General_GetRandomNumber() & 0xFF) - 128.0f) * 0.003f);
		if (General_GetRandomNumber() & 1)
			RoadBlocks_BackendMatrixRotateZ(tmp,((General_GetRandomNumber() & 0xFF) - 128.0f) * 0.003f + 3.1416f);
		RoadBlocks_BackendMatrixTranslateOnly(tmp,rb_add(rb_left(offset,forward),pos));
		(*RoadBlocks_BackendMatrixZ(tmp)) += 0.6f;
		float fModelRadius = RoadBlocks_BackendColRadius(ModelRegistry_Collision(ModelRegistry_Get(vehicleId))) - 0.25f;
		int16_t colliding = 0;
		RoadBlocks_BackendColliding(RoadBlocks_BackendMatrixPosition(tmp),fModelRadius,&colliding);
		if (!colliding) {
//+ rouz edit (ChatGPT)
			// Allocate the roadblock vehicle without invoking C++ new.
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CAutomobile* pVehicle = (CAutomobile*)CPool_New(RoadBlocks_BackendVehiclePool());
			//- rouz edit (ChatGPT)
			assert(pVehicle);
			RoadBlocks_BackendConstructCar(pVehicle,vehicleId,ROADBLOCKS_RANDOM_VEHICLE);
//- rouz edit (ChatGPT)
			RoadBlocks_BackendCarStatus(pVehicle,STATUS_ABANDONED);
			// pVehicle->GetHeightAboveRoad(); // called but return value is ignored?
			(*RoadBlocks_BackendMatrixZ(tmp)) += fModelRadius - 0.6f;
			RoadBlocks_BackendCarMatrix(pVehicle,tmp);
			RoadBlocks_BackendCarOnRoad(pVehicle);
			RoadBlocks_BackendCarStatic(pVehicle,false);
			RoadBlocks_BackendCarUpdateRW(pVehicle);
			RoadBlocks_BackendCarLock(pVehicle,ROADBLOCKS_CARLOCK_UNLOCKED);
			RoadBlocks_BackendCarJoinRoad(pVehicle);
			RoadBlocks_BackendCarLocked(pVehicle,false);
			(*RoadBlocks_BackendCarMission(pVehicle)) = ROADBLOCKS_MISSION_NONE;
			(*RoadBlocks_BackendCarTempAction(pVehicle)) = ROADBLOCKS_TEMPACT_NONE;
			(*RoadBlocks_BackendCarNextLane(pVehicle)) = (*RoadBlocks_BackendCarCurrentLane(pVehicle)) = 0;
			(*RoadBlocks_BackendCarCruiseSpeed(pVehicle)) = (*RoadBlocks_BackendCarMaxSpeed(pVehicle)) = 0;
			RoadBlocks_BackendCarExtended(pVehicle,true);
			if (RoadBlocks_BackendCarUsesSiren(pVehicle) && General_GetRandomNumber() & 1)
				RoadBlocks_BackendCarSiren(pVehicle,true);
			if (RoadBlocks_BackendCarUpZ(pVehicle) > 0.94f) {
				RoadBlocks_BackendVehicleAlpha(pVehicle,0);
				RoadBlocks_BackendAddVehicle(pVehicle);
				RoadBlocks_BackendCarCreatePeds(pVehicle,true);
				(*RoadBlocks_BackendCarRoadblockType(pVehicle)) = rb_dot(RoadBlocks_BackendCarRight(pVehicle),rb_sub(RoadBlocks_BackendCarPosition(pVehicle),RoadBlocks_BackendPlayerCoors())) >= 0.0f;
				(*RoadBlocks_BackendCarRange(pVehicle)) = (*Timer_GetTimeInMilliseconds()) + 7000;
			}
			else {
//+ rouz edit (ChatGPT)
				// Destroy and release the roadblock vehicle without invoking C++ delete.
				RoadBlocks_BackendDestroyCar(pVehicle);
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				CPool_Delete(RoadBlocks_BackendVehiclePool(), pVehicle);
				//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
			}
		}
	}
	int numBarriers = distBetween / ROADBLOCK_OBJECT_WIDTH;
	RoadBlocks_BackendRequestModel(MI_ROADWORKBARRIER1, ROADBLOCKS_STREAMFLAGS_DONT_REMOVE);
	if (!RoadBlocks_BackendModelLoaded(MI_ROADWORKBARRIER1))
		goto roadblocks_done;
	for (int i = 0; i < numBarriers; i++) {
		float offset = ROADBLOCK_OBJECT_WIDTH * (i - numBarriers / 2);
		RoadBlocks_BackendMatrixTranslate(tmp,0.0f,0.0f,0.0f);
		RoadBlocks_BackendMatrixRight(tmp,rb_vec(forward.y,-forward.x,0.0f));
		RoadBlocks_BackendMatrixForward(tmp,forward);
		RoadBlocks_BackendMatrixUp(tmp,rb_vec(0.0f,0.0f,1.0f));
		RoadBlocks_BackendMatrixRotateZ(tmp,((General_GetRandomNumber() & 0xFF) - 128.0f) * 0.003f);
		RoadBlocks_BackendMatrixTranslateOnly(tmp,rb_add(rb_add(rb_left(5.0f,right),rb_left(offset,forward)),pos));
		(*RoadBlocks_BackendMatrixX(tmp)) += (General_GetRandomNumber() & 0xF) * 0.1f;
		(*RoadBlocks_BackendMatrixY(tmp)) += (General_GetRandomNumber() & 0xF) * 0.1f;
		bool found;
		(*RoadBlocks_BackendMatrixZ(tmp)) = RoadBlocks_BackendGroundZ((*RoadBlocks_BackendMatrixX(tmp)), (*RoadBlocks_BackendMatrixY(tmp)), (*RoadBlocks_BackendMatrixZ(tmp)) + 2.0f, &found);
		if (!found)
			continue;
		int16_t colliding = 0;
		CBaseModelInfo* pMI = ModelRegistry_Get(MI_ROADWORKBARRIER1);
		(*RoadBlocks_BackendMatrixZ(tmp)) -= RoadBlocks_BackendColMinZ(ModelRegistry_Collision(pMI));
		RoadBlocks_BackendColliding(RoadBlocks_BackendMatrixPosition(tmp),RoadBlocks_BackendColRadius(ModelRegistry_Collision(pMI)),&colliding);
		if (colliding == 0) {
//+ rouz edit (ChatGPT)
			// Allocate the roadblock barrier without invoking C++ new.
			CPool *objectPool = RoadBlocks_BackendObjectPool();
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CObject* pObject = ((CObject*)CPool_New(objectPool));
			//- rouz edit (ChatGPT)
#ifdef FIX_BUGS
			if (!pObject) {
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				for (int32_t j = 0; j < CPool_GetSize(objectPool); j++) {
					// Access raw storage through the C store or pool API
					CObject *existing = ((CObject*)CPool_GetSlot(objectPool, j));
				//- rouz edit (ChatGPT)
					if (existing && RoadBlocks_BackendObjectCreated(existing) == ROADBLOCKS_TEMP_OBJECT) {
						//+ rouz edit (ChatGPT)
						// Access raw storage through the C store or pool API
						int32_t handle = CPool_GetIndex(objectPool, existing);
						//- rouz edit (ChatGPT)
						RoadBlocks_BackendRemoveObject(existing);
						RoadBlocks_BackendDestroyObject(existing);
						//+ rouz edit (ChatGPT)
						// Access raw storage through the C store or pool API
						CPool_Delete(objectPool, existing);
						pObject = ((CObject*)CPool_NewAt(objectPool, handle));
						//- rouz edit (ChatGPT)
						break;
					}
				}
			}
			if (pObject)
#endif
				RoadBlocks_BackendConstructObject(pObject,MI_ROADWORKBARRIER1,true);
			if (!pObject)
				continue;
//- rouz edit (ChatGPT)
			RoadBlocks_BackendObjectMatrix(pObject,tmp);
			RoadBlocks_BackendObjectCreatedSet(pObject,ROADBLOCKS_TEMP_OBJECT);
			(*RoadBlocks_BackendObjectLife(pObject)) = (*Timer_GetTimeInMilliseconds()) + 600000;
			RoadBlocks_BackendAddObject(pObject);
		}
	}
roadblocks_done:
    RoadBlocks_BackendMatrixDestroy(tmp);
}

//- rouz edit (ChatGPT)
