//+ rouz edit (ChatGPT)
#include "WaterCreatures_backend.h"
#include "ModelIndexData.h"
#include "General.h"
#include "Timer.h"
#include <math.h>
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define WATER_CREATURE_PI 3.14159265358979323846f
#define WATER_CREATURE_HALFPI (WATER_CREATURE_PI/2)
#define WATER_CREATURE_DEGTORAD(value) ((value)*WATER_CREATURE_PI/180.0f)
#define WATER_CREATURE_RADTODEG(value) ((value)*180.0f/WATER_CREATURE_PI)
_Static_assert(NUM_WATER_CREATURES==8,"Original complete statically constructed slot pool");
int32_t WaterCreatures_nNumActiveSeaLifeForms;
WaterCreature WaterCreatures_aWaterCreatures[NUM_WATER_CREATURES]={
    {NULL,0.0f,0.0f,0,0.0f,WATER_CREATURE_DISABLED},
    {NULL,0.0f,0.0f,0,0.0f,WATER_CREATURE_DISABLED},
    {NULL,0.0f,0.0f,0,0.0f,WATER_CREATURE_DISABLED},
    {NULL,0.0f,0.0f,0,0.0f,WATER_CREATURE_DISABLED},
    {NULL,0.0f,0.0f,0,0.0f,WATER_CREATURE_DISABLED},
    {NULL,0.0f,0.0f,0,0.0f,WATER_CREATURE_DISABLED},
    {NULL,0.0f,0.0f,0,0.0f,WATER_CREATURE_DISABLED},
    {NULL,0.0f,0.0f,0,0.0f,WATER_CREATURE_DISABLED}
};
WaterCreatureProperties WaterCreatures_Properties[65] = {
	{ &MI_FISH1SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH2SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH3SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH3S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH1SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH2SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH3SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH3S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_JELLYFISH,	0.01f, 2.2f, 0.0005f, 3.5f },
	{ &MI_JELLYFISH01,	0.01f, 2.2f, 0.0005f, 3.5f },
	{ &MI_FISH1SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH2SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH3SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH3S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH1SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH2SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH3SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH3S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_TURTLE,		0.01f, 2.0f, 0.0005f, 4.0f },
	{ &MI_FISH1SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH2SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH3SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH3S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH1SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH2SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH3SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH3S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_DOLPHIN,		0.03f, 1.5f, 0.0005f, 4.0f },
	{ &MI_FISH1SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH2SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH3SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH3S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH1SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH2SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH3SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH3S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_SHARK,		0.03f, 0.4f, 0.0005f, 4.0f },
	{ &MI_FISH1SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH2SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH3SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH3S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH1SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH2SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH2S,		0.04f, 1.5f, 0.0008f, 3.0f },
	{ &MI_FISH3SINGLE,	0.04f, 1.0f, 0.0008f, 3.0f },
	{ &MI_FISH3S,		0.04f, 1.5f, 0.0008f, 3.0f },
};

static WaterCreatureVector WaterCreatures_Subtract(WaterCreatureVector a,WaterCreatureVector b)
{
    // Preserve every original component subtraction
    WaterCreatureVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static float WaterCreatures_MagnitudeSqr(WaterCreatureVector value)
{
    // Preserve the original ordered sum of squared components
    return value.x*value.x+value.y*value.y+value.z*value.z;
}
static void WaterCreatures_Normalise(WaterCreatureVector *value)
{
    // Preserve the original reciprocal-square-root normalization and x-only zero fallback
    float sq=WaterCreatures_MagnitudeSqr(*value);if(sq>0.0f){
        // Apply the same captured reciprocal to each original component
        float inverse=1.0f/sqrtf(sq);value->x*=inverse;value->y*=inverse;value->z*=inverse;
    }else value->x=1.0f;
}
static WaterCreatureVector WaterCreatures_Multiply(WaterCreatureVector value,float scalar)
{
    // Preserve the original vector-left multiplication operand order
    WaterCreatureVector result={value.x*scalar,value.y*scalar,value.z*scalar};return result;
}
static float WaterCreatures_HeadingValues(WaterCreatureVector position1,WaterCreatureVector position2)
{
    // Normalize the same captured difference before the original integer random heading offset
    WaterCreatureVector delta=WaterCreatures_Subtract(position1,position2);WaterCreatures_Normalise(&delta);
    return General_GetRandomNumberInRangeInt(-90,90)+WATER_CREATURE_RADTODEG(atan2f(-delta.x,delta.y)+WATER_CREATURE_HALFPI+WATER_CREATURE_PI);
}
static void WaterCreatures_CreateOneWithPosition(WaterCreatureVector playerPos,WaterCreaturePosition *fishPos,int32_t modelID);
void WaterCreature_Construct(WaterCreature *record)
{
    // Execute the same complete clearing operation as the original constructor
    WaterCreature_Free(record);
}
void WaterCreature_Initialise(WaterCreature *record,CObject *pObj, float fFwdSpeed, float fZTurnSpeed, float fWaterDepth, uint32_t alpha, enum eFishSlotState state) {
    // Preserve every original state write, native call and traversal decision
	record->m_pObj = pObj;
	record->m_fFwdSpeed = fFwdSpeed;
	record->m_fZTurnSpeed = fZTurnSpeed;
	record->m_fWaterDepth = fWaterDepth;
	record->m_alpha = alpha;
	record->m_state = state;
}

void WaterCreature_Allocate(WaterCreature *record,CObject *pObj, float fFwdSpeed, float fZTurnSpeed, float fWaterDepth, uint32_t alpha, enum eFishSlotState state) {
    // Preserve every original state write, native call and traversal decision
	WaterCreature_Initialise(record,pObj, fFwdSpeed, fZTurnSpeed, fWaterDepth, alpha, state);
}

void WaterCreature_Free(WaterCreature *record) {
    // Preserve every original state write, native call and traversal decision
	WaterCreature_Initialise(record,NULL, 0.0f, 0.0f, 0.0f, 0, WATER_CREATURE_DISABLED);
}

WaterCreature *WaterCreatures_GetFishStructSlot(void) {
    // Preserve every original state write, native call and traversal decision
	for (int i = 0; i < NUM_WATER_CREATURES; i++)
		if (WaterCreatures_aWaterCreatures[i].m_state == WATER_CREATURE_DISABLED)
			return &WaterCreatures_aWaterCreatures[i];
	
	return NULL;
}

CObject *WaterCreatures_CreateSeaLifeForm(const WaterCreaturePosition *pos, int16_t modelID, int32_t zRotAngle) {
    // Preserve every original state write, native call and traversal decision
	if ((*WaterCreatures_BackendTempCount()) >= NUMTEMPOBJECTS)
		return NULL;

//+ rouz edit (ChatGPT)
	// Allocate the water creature object without invoking C++ new.
	CPool *objectPool = WaterCreatures_BackendObjectPool();
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CObject *pObj = ((CObject*)CPool_New(objectPool));
	//- rouz edit (ChatGPT)
#ifdef FIX_BUGS
	if (!pObj) {
		// Preserve the original branch and side-effect ordering
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		for (int32_t i = 0; i < CPool_GetSize(objectPool); i++) {
			// Preserve the original branch and side-effect ordering
			// Access raw storage through the C store or pool API
			CObject *existing = ((CObject*)CPool_GetSlot(objectPool, i));
		//- rouz edit (ChatGPT)
			if (existing && WaterCreatures_BackendCreatedBy(existing) == WATER_CREATURE_TEMP_OBJECT) {
				// Preserve the original branch and side-effect ordering
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				int32_t handle = CPool_GetIndex(objectPool, existing);
				//- rouz edit (ChatGPT)
				WaterCreatures_BackendWorldRemove(existing);
				WaterCreatures_BackendDestroy(existing);
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				CPool_Delete(objectPool, existing);
				pObj = ((CObject*)CPool_NewAt(objectPool, handle));
				//- rouz edit (ChatGPT)
				break;
			}
		}
	}
	if (pObj)
#endif
		WaterCreatures_BackendConstruct(pObj,modelID);
//- rouz edit (ChatGPT)

	if (!pObj) return NULL;
	
	WaterCreatures_BackendSetPosition(pObj,pos);
	WaterCreatures_BackendUpdateRW(pObj);
	WaterCreatures_BackendSetMoveSpeed(pObj,(WaterCreatureVector){0.0f,0.0f,0.0f});
	WaterCreatures_BackendSetTurnSpeed(pObj,(WaterCreatureVector){0.0f,0.0f,0.0f});
	WaterCreatures_BackendRotateZ(pObj,WATER_CREATURE_DEGTORAD(zRotAngle));
	WaterCreatures_BackendUpdateRW(pObj);
	WaterCreatures_BackendSetCreatedBy(pObj,WATER_CREATURE_CONTROLLED_SUB_OBJECT);
	WaterCreatures_BackendSetStatic(pObj,false);

	if (WaterCreatures_BackendCreatedBy(pObj) == WATER_CREATURE_TEMP_OBJECT) {
		// Preserve the original branch and side-effect ordering
		(*WaterCreatures_BackendTempCount())++;
		(*WaterCreatures_BackendEndTime(pObj)) = (*Timer_GetTimeInMilliseconds()) + 60000;
	}

	WaterCreatures_BackendSetTouchingWater(pObj,true);
	WaterCreatures_BackendSetUnderwater(pObj,true);
	WaterCreatures_BackendWorldAdd(pObj);

	return pObj;
}

bool WaterCreatures_IsSpaceForMoreWaterCreatures(void) {
    // Preserve every original state write, native call and traversal decision
	return WaterCreatures_nNumActiveSeaLifeForms < NUM_WATER_CREATURES;
}

float WaterCreatures_CalculateFishHeading(const WaterCreaturePosition *pos1,const WaterCreaturePosition *pos2)
{
    // Preserve every original state write, native call and traversal decision
    // Read the same two borrowed native positions before the complete C heading calculation
    return WaterCreatures_HeadingValues(WaterCreatures_BackendVector(pos1),WaterCreatures_BackendVector(pos2));
}

void WaterCreatures_CreateOne(const WaterCreaturePosition *pos,int32_t modelID)
{
    // Preserve every original state write, native call and traversal decision
    // Capture the original player value before copying the caller's still-live native position
    if(!WaterCreatures_IsSpaceForMoreWaterCreatures())return;
    WaterCreatureVector player=WaterCreatures_BackendPlayerPosition();
    WaterCreatures_BackendWithFishPosition(pos,player,modelID,WaterCreatures_CreateOneWithPosition);
}
static void WaterCreatures_CreateOneWithPosition(WaterCreatureVector playerPos,WaterCreaturePosition *fishPos,int32_t modelID)
{
    // Preserve every original state write, native call and traversal decision
    // Retain the native fish-position reference throughout the complete original creation sequence
	float fDepth, fLevelNoWaves;
	if (!WaterCreatures_BackendVisible(fishPos,3.0f)
		&& WaterCreatures_BackendDepth(fishPos, &fDepth, &fLevelNoWaves, NULL) && fDepth > 4.5f) {
			// Preserve the original branch and side-effect ordering
		
		if (modelID == -1 || modelID < 0 || modelID > 64)
			modelID = General_GetRandomNumberInRangeInt(0, 64);

		WaterCreatureProperties *creature = &WaterCreatures_Properties[modelID];
		WaterCreatures_BackendSetZ(fishPos,fLevelNoWaves-creature->fLevel);
		float fFwdSpeed = General_GetRandomNumberInRangeFloat(0.0f, creature->fFwdSpeed) + 0.01f;
		float angle = WaterCreatures_HeadingValues(playerPos,WaterCreatures_BackendVector(fishPos));
		
		CObject *fish = WaterCreatures_CreateSeaLifeForm(fishPos, *creature->modelID, angle);
		if (!fish) return;

		WaterCreatures_BackendSetAlpha(fish,255); 
		WaterCreature *wc = WaterCreatures_GetFishStructSlot();
		WaterCreature_Allocate(wc,fish, fFwdSpeed, 0.0f, creature->fWaterDepth, 255, WATER_CREATURE_INIT);
		WaterCreatures_nNumActiveSeaLifeForms++;
	}
}

void WaterCreatures_FreeFishStructSlot(WaterCreature *wc) {
    // Preserve every original state write, native call and traversal decision
	WaterCreature_Free(wc);
}

void WaterCreatures_UpdateAll(void) {
    // Preserve every original state write, native call and traversal decision
	if (WaterCreatures_nNumActiveSeaLifeForms == 0)
		return;

	WaterCreatureVector playerPos=WaterCreatures_BackendPlayerPosition();
	for (int i = 0; i < NUM_WATER_CREATURES; i++) {
		// Preserve the original branch and side-effect ordering
		switch (WaterCreatures_aWaterCreatures[i].m_state) {
			// Preserve the original branch and side-effect ordering
		case WATER_CREATURE_ACTIVE:
			// is this even reachable?
			(*WaterCreatures_BackendEndTime(WaterCreatures_aWaterCreatures[i].m_pObj)) = (*Timer_GetTimeInMilliseconds()) + 40000;
			if (!WaterCreatures_BackendOnScreen(WaterCreatures_aWaterCreatures[i].m_pObj)) {
				// Preserve the original branch and side-effect ordering
				WaterCreatures_BackendSetAlpha(WaterCreatures_aWaterCreatures[i].m_pObj,0);
				WaterCreatures_aWaterCreatures[i].m_state = WATER_CREATURE_REMOVE;
				break;
			}
			// fall through
		case WATER_CREATURE_INIT: {
			// Preserve the original branch and side-effect ordering
			if (WaterCreatures_MagnitudeSqr(WaterCreatures_Subtract(playerPos,WaterCreatures_BackendPosition(WaterCreatures_aWaterCreatures[i].m_pObj))) < (75.0f*75.0f)) {
				// Preserve the original branch and side-effect ordering
				if (WaterCreatures_aWaterCreatures[i].m_alpha < 255)
					WaterCreatures_aWaterCreatures[i].m_alpha = Min(WaterCreatures_aWaterCreatures[i].m_alpha + 4, 255);
				WaterCreatures_BackendSetAlpha(WaterCreatures_aWaterCreatures[i].m_pObj,WaterCreatures_aWaterCreatures[i].m_alpha);
				WaterCreatureVector fwd=WaterCreatures_BackendRight(WaterCreatures_aWaterCreatures[i].m_pObj);	// for some reason they used x for forward
				WaterCreatures_Normalise(&fwd);
				WaterCreatures_BackendSetMoveSpeed(WaterCreatures_aWaterCreatures[i].m_pObj,WaterCreatures_Multiply(fwd,WaterCreatures_aWaterCreatures[i].m_fFwdSpeed));
				WaterCreatures_BackendSetTurnSpeed(WaterCreatures_aWaterCreatures[i].m_pObj,(WaterCreatureVector){0.0f,0.0f,WaterCreatures_aWaterCreatures[i].m_fZTurnSpeed});
				WaterCreatures_BackendSetStatic(WaterCreatures_aWaterCreatures[i].m_pObj,false);
				float fDepth = 0.0;
				WaterCreatures_BackendDepth(WaterCreatures_BackendObjectPosition(WaterCreatures_aWaterCreatures[i].m_pObj), &fDepth, NULL, NULL);
				if (WaterCreatures_aWaterCreatures[i].m_fWaterDepth < fDepth) {
					// Preserve the original branch and side-effect ordering
					// it looks like this can never be true initially, looks like a BUG
					if ((*WaterCreatures_BackendEndTime(WaterCreatures_aWaterCreatures[i].m_pObj)) - 40000 <= (*Timer_GetTimeInMilliseconds()))
						WaterCreatures_aWaterCreatures[i].m_state = WATER_CREATURE_ACTIVE;
				}
				else {
					// Preserve the original branch and side-effect ordering
					// creature is deeper than water
					WaterCreatures_aWaterCreatures[i].m_state = WATER_CREATURE_FADE_OUT;
				}

			}
			else {
				// Preserve the original branch and side-effect ordering
				WaterCreatures_aWaterCreatures[i].m_state = WATER_CREATURE_REMOVE;
			}
			break;
		}
		case WATER_CREATURE_FADE_OUT: {
			// Preserve the original branch and side-effect ordering
			(*WaterCreatures_BackendEndTime(WaterCreatures_aWaterCreatures[i].m_pObj)) = (*Timer_GetTimeInMilliseconds()) + 40000;
			if (WaterCreatures_aWaterCreatures[i].m_alpha <= 0) {
				// Preserve the original branch and side-effect ordering
				WaterCreatures_aWaterCreatures[i].m_state = WATER_CREATURE_REMOVE;
			}
			else {
				// Preserve the original branch and side-effect ordering
				WaterCreatures_aWaterCreatures[i].m_alpha = Max(WaterCreatures_aWaterCreatures[i].m_alpha - 6, 0);
				WaterCreatures_BackendSetAlpha(WaterCreatures_aWaterCreatures[i].m_pObj,WaterCreatures_aWaterCreatures[i].m_alpha);
				WaterCreatureVector speed=WaterCreatures_BackendRight(WaterCreatures_aWaterCreatures[i].m_pObj);
				WaterCreatures_Normalise(&speed);
				speed.x *= WaterCreatures_aWaterCreatures[i].m_fFwdSpeed;
				speed.y *= WaterCreatures_aWaterCreatures[i].m_fFwdSpeed;
				speed.z = -0.015f;
				WaterCreatures_BackendSetMoveSpeed(WaterCreatures_aWaterCreatures[i].m_pObj,speed);

				if (!WaterCreatures_BackendOnScreen(WaterCreatures_aWaterCreatures[i].m_pObj))
					WaterCreatures_aWaterCreatures[i].m_state = WATER_CREATURE_REMOVE;
			}
			break;
		}
		case WATER_CREATURE_REMOVE: 
			if (WaterCreatures_aWaterCreatures[i].m_pObj){
				// Preserve the original branch and side-effect ordering
				WaterCreatures_BackendWorldRemove(WaterCreatures_aWaterCreatures[i].m_pObj);
//+ rouz edit (ChatGPT)
				// Destroy and release the water creature object without invoking C++ delete.
				WaterCreatures_BackendDestroy(WaterCreatures_aWaterCreatures[i].m_pObj);
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				CPool_Delete(WaterCreatures_BackendObjectPool(), WaterCreatures_aWaterCreatures[i].m_pObj);
				//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
			}
			WaterCreatures_FreeFishStructSlot(&WaterCreatures_aWaterCreatures[i]);
			WaterCreatures_nNumActiveSeaLifeForms--;
			WaterCreatures_aWaterCreatures[i].m_state = WATER_CREATURE_DISABLED;
			break;
		default:
			break;
		}
	}
}

void WaterCreatures_RemoveAll(void) {
    // Preserve every original state write, native call and traversal decision
	for (int i = 0; i < NUM_WATER_CREATURES; i++) {
		// Preserve the original branch and side-effect ordering
		if (WaterCreatures_aWaterCreatures[i].m_state != WATER_CREATURE_DISABLED) {
			// Preserve the original branch and side-effect ordering
			if (WaterCreatures_aWaterCreatures[i].m_pObj){
				// Preserve the original branch and side-effect ordering
				WaterCreatures_BackendWorldRemove(WaterCreatures_aWaterCreatures[i].m_pObj);
//+ rouz edit (ChatGPT)
				// Destroy and release the water creature object without invoking C++ delete.
				WaterCreatures_BackendDestroy(WaterCreatures_aWaterCreatures[i].m_pObj);
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				CPool_Delete(WaterCreatures_BackendObjectPool(), WaterCreatures_aWaterCreatures[i].m_pObj);
				//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
			}
			WaterCreatures_FreeFishStructSlot(&WaterCreatures_aWaterCreatures[i]);
			WaterCreatures_aWaterCreatures[i].m_state = WATER_CREATURE_DISABLED;
			WaterCreatures_nNumActiveSeaLifeForms--;
		}
	}
}
//- rouz edit (ChatGPT)
