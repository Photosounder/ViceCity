//+ rouz edit (ChatGPT)
#include "ScriptPaths_backend.h"
#include "Timer.h"
#include <math.h>
#include <stdio.h>
#ifdef VALIDATE_SAVE_SIZE
void re3_assert(const char *,const char *,unsigned int,const char *);
#ifdef assert
#undef assert
#endif
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#endif
#include "SaveBuf.h"
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))
#define SCRIPT_PATHS_PI 3.14159265358979323846f
#define SCRIPT_PATHS_TWOPI 6.28318530717958647692f
#ifdef COMPATIBLE_SAVES
#define SCRIPTPATHS_SAVE_SIZE 0x9C
#else
#define SCRIPTPATHS_SAVE_SIZE sizeof(ScriptPaths_aArray)
#endif
ScriptPath ScriptPaths_aArray[3];
CObject *g_pScriptPathObjects[18];
static ScriptPathVector ScriptPaths_Add(ScriptPathVector a,ScriptPathVector b)
{
    // Preserve each original component addition before midpoint division
    ScriptPathVector result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static ScriptPathVector ScriptPaths_Subtract(ScriptPathVector a,ScriptPathVector b)
{
    // Preserve each original component subtraction before direction or speed calculation
    ScriptPathVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static ScriptPathVector ScriptPaths_Divide(ScriptPathVector a,float value)
{
    // Divide the original components directly without replacing division by reciprocal multiplication
    ScriptPathVector result={a.x/value,a.y/value,a.z/value};return result;
}
static ScriptPathVector ScriptPaths_Cross(ScriptPathVector a,ScriptPathVector b)
{
    // Preserve the native cross-product component order and operand order
    ScriptPathVector result={a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};return result;
}
static void ScriptPaths_Normalise(ScriptPathVector *a)
{
    // Preserve the native reciprocal-square-root normalizer and its x-only zero fallback
    float sq=a->x*a->x+a->y*a->y+a->z*a->z;if(sq>0.0f){
        // Multiply each original component by the same reciprocal square root
        float inv=1.0f/sqrtf(sq);a->x*=inv;a->y*=inv;a->z*=inv;
    }else a->x=1.0f;
}
static float ScriptPaths_Heading(ScriptPathVector a)
{
    // Preserve the original negated-x heading arguments
    return atan2f(-a.x,a.y);
}
static bool ScriptPaths_Different(ScriptPathVector a,ScriptPathVector b)
{
    // Preserve the original ordered component inequality checks
    return a.x!=b.x || a.y!=b.y || a.z!=b.z;
}
static void ScriptPaths_UpdateObject(ScriptPath *path,int32_t i,ScriptPathGeometry geometry,void *previous);
void ScriptPath_FindCoorsFromDistanceOnPath(ScriptPath *path,float t, float *pX, float *pY, float *pZ)
{
    // Preserve every original live read, save cursor and state-write ordering
	int32_t i;
	for (i = 0; ScriptPaths_BackendNodeT(path,i+1) < t; i++)
		if (i == path->m_numNodes - 1) {
			// Preserve the original callback and traversal order
			// don't go beyond last node
			*pX = ScriptPaths_BackendNodeAxis(path,path->m_numNodes-1,0);
			*pY = ScriptPaths_BackendNodeAxis(path,path->m_numNodes-1,1);
			*pZ = ScriptPaths_BackendNodeAxis(path,path->m_numNodes-1,2);
			return;
		}
	float f = (t - ScriptPaths_BackendNodeT(path,i)) / (ScriptPaths_BackendNodeT(path,i+1) - ScriptPaths_BackendNodeT(path,i));
	*pX = (1.0f - f)*ScriptPaths_BackendNodeAxis(path,i,0) + f*ScriptPaths_BackendNodeAxis(path,i+1,0);
	*pY = (1.0f - f)*ScriptPaths_BackendNodeAxis(path,i,1) + f*ScriptPaths_BackendNodeAxis(path,i+1,1);
	*pZ = (1.0f - f)*ScriptPaths_BackendNodeAxis(path,i,2) + f*ScriptPaths_BackendNodeAxis(path,i+1,2);
}

void ScriptPath_Update(ScriptPath *path) {
    // Preserve every original live read, save cursor and state-write ordering
	if (path->m_state != SCRIPT_PATH_ACTIVE)
		return;

	path->m_fPosition += path->m_fSpeed * Timer_GetTimeStepInSeconds();
	path->m_fPosition = Clamp(path->m_fPosition, 0.0f, path->m_fTotalLength);

	if (path->m_pObjects[0] || path->m_pObjects[1] || path->m_pObjects[2] || path->m_pObjects[3]
		|| path->m_pObjects[4] || path->m_pObjects[5]) {
			// Preserve the original callback and traversal order

		float t1, t2;
		ScriptPathVector pos1,pos2;

		t1 = Max(path->m_fPosition - path->m_fObjectLength / 2.0f, 0.0f);
		ScriptPath_FindCoorsFromDistanceOnPath(path,t1, &pos1.x, &pos1.y, &pos1.z);
		t2 = Min(path->m_fPosition + path->m_fObjectLength / 2.0f, path->m_fTotalLength);
		ScriptPath_FindCoorsFromDistanceOnPath(path,t2, &pos2.x, &pos2.y, &pos2.z);

		ScriptPathVector newForward,newUp={0.0f,0.0f,1.0f},newRight;

		newForward=ScriptPaths_Subtract(pos2,pos1);
		ScriptPaths_Normalise(&newForward);
		newRight = ScriptPaths_Cross(newForward, newUp);
		ScriptPaths_Normalise(&newRight);
		newUp = ScriptPaths_Cross(newRight, newForward);

		for (int i = 0; i < 6; i++) {
			// Preserve the original callback and traversal order
			if (path->m_pObjects[i]) {
				// Preserve the original callback and traversal order
                // Keep the actual native matrix copy alive until all original object writes finish
                ScriptPathGeometry geometry={pos1,pos2,newRight,newUp,newForward};
                ScriptPaths_BackendWithPrevious(path,i,geometry,ScriptPaths_UpdateObject);
			}
		}
	}
}

void ScriptPath_Clear(ScriptPath *path) {
    // Preserve every original live read, save cursor and state-write ordering
	if (path->m_pNode)
		ScriptPaths_BackendFreeNodes(path->m_pNode); // rouz edit (ChatGPT)
	path->m_pNode = NULL;
	path->m_numNodes = 0;
	for (int i = 0; i < 6; i++)
		path->m_pObjects[i] = NULL;
	path->m_state = SCRIPT_PATH_DISABLED;
}

void ScriptPath_InitialiseOne(ScriptPath *path,int32_t numNodes, float length) {
    // Preserve every original live read, save cursor and state-write ordering
	char Dest[32];
	sprintf(Dest, "data\\paths\\spath%d.dat", numNodes);
	path->m_pNode = ScriptPaths_BackendLoadNodes(Dest,&path->m_numNodes,&path->m_fTotalLength);
	path->m_fSpeed = 1.0f;
	path->m_fPosition = 0.0f;
	path->m_fObjectLength = length;
	path->m_state = SCRIPT_PATH_INITIALIZED;
}

void ScriptPath_SetObjectToControl(ScriptPath *path,CObject *pObj) {
    // Preserve every original live read, save cursor and state-write ordering
	int32_t i = 0;
	while (i < 6 && path->m_pObjects[i])
		i++;
	path->m_pObjects[i] = pObj;
	ScriptPaths_BackendRegister(pObj,&path->m_pObjects[i]);
	ScriptPaths_BackendClearPhysics(pObj);
	path->m_state = SCRIPT_PATH_ACTIVE;
}

void ScriptPaths_Init(void) {
    // Preserve every original live read, save cursor and state-write ordering
	for (int i = 0; i < 3; i++)
		ScriptPath_Clear(&ScriptPaths_aArray[i]);
}

void ScriptPaths_Shutdown(void) {
    // Preserve every original live read, save cursor and state-write ordering
	for (int i = 0; i < 3; i++)
		ScriptPath_Clear(&ScriptPaths_aArray[i]);
}

void ScriptPaths_Update(void) {
    // Preserve every original live read, save cursor and state-write ordering
	for (int i = 0; i < 3; i++)
		ScriptPath_Update(&ScriptPaths_aArray[i]);
}

bool ScriptPaths_IsOneActive(void) {
    // Preserve every original live read, save cursor and state-write ordering
	for (int i = 0; i < 3; i++)
		if (ScriptPaths_aArray[i].m_state == SCRIPT_PATH_ACTIVE && ScriptPaths_aArray[i].m_fSpeed != 0.0f)
			return true;

	return false;
}

void ScriptPaths_Load(uint8_t *buf, uint32_t size) {
    // Preserve every original live read, save cursor and state-write ordering
INITSAVEBUF
	for (int32_t i = 0; i < 3; i++)
		ScriptPath_Clear(&ScriptPaths_aArray[i]);

	for (int32_t i = 0; i < 3; i++) {
		// Preserve the original callback and traversal order
#ifdef COMPATIBLE_SAVES
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&ScriptPaths_aArray[i].m_numNodes, &buf, sizeof(ScriptPaths_aArray[i].m_numNodes));
		SkipSaveBuf(&buf, 4);
		ReadSaveBuf(&ScriptPaths_aArray[i].m_fTotalLength, &buf, sizeof(ScriptPaths_aArray[i].m_fTotalLength));
		ReadSaveBuf(&ScriptPaths_aArray[i].m_fSpeed, &buf, sizeof(ScriptPaths_aArray[i].m_fSpeed));
		ReadSaveBuf(&ScriptPaths_aArray[i].m_fPosition, &buf, sizeof(ScriptPaths_aArray[i].m_fPosition));
		ReadSaveBuf(&ScriptPaths_aArray[i].m_fObjectLength, &buf, sizeof(ScriptPaths_aArray[i].m_fObjectLength));
		ReadSaveBuf(&ScriptPaths_aArray[i].m_state, &buf, sizeof(ScriptPaths_aArray[i].m_state));
		//- rouz edit (ChatGPT)
#else
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		// Preserve record assignment semantics and compiler padding behavior
		ScriptPaths_aArray[i] = *(ScriptPath*)buf;
		SkipSaveBuf(&buf, sizeof(ScriptPaths_aArray[i]));
		//- rouz edit (ChatGPT)
#endif

		for (int32_t j = 0; j < 6; j++) {
			// Preserve the original callback and traversal order
#ifdef COMPATIBLE_SAVES
			ScriptPaths_aArray[i].m_pObjects[j] = NULL;
			int32_t tmp;
			//+ rouz edit (ChatGPT)
			// Transfer save data through the C buffer API with explicit sizes
			ReadSaveBuf(&tmp, &buf, sizeof(tmp));
			//- rouz edit (ChatGPT)
			if (tmp != 0) {
				// Preserve the original callback and traversal order
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				ScriptPaths_aArray[i].m_pObjects[j] = ((CObject*)CPool_GetSlot(ScriptPaths_BackendObjectPool(), tmp - 1));
				//- rouz edit (ChatGPT)
				ScriptPaths_BackendClearPhysics(ScriptPaths_aArray[i].m_pObjects[j]);
			}
#else
			ScriptPath *pPath = &ScriptPaths_aArray[i];
			if (pPath->m_pObjects[j] != NULL) {
				// Preserve the original callback and traversal order
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				pPath->m_pObjects[j] = ((CObject*)CPool_GetSlot(ScriptPaths_BackendObjectPool(), (uintptr_t)pPath->m_pObjects[j] - 1));
				//- rouz edit (ChatGPT)
				ScriptPaths_BackendClearPhysics(pPath->m_pObjects[j]);
			}
#endif
		}

		ScriptPaths_aArray[i].m_pNode=ScriptPaths_BackendAllocateNodes(ScriptPaths_aArray[i].m_numNodes); // rouz edit (ChatGPT)
		for (int32_t j = 0; j < ScriptPaths_aArray[i].m_numNodes; j++) {
			// Preserve the original callback and traversal order
			//+ rouz edit (ChatGPT)
			// Transfer save data through the C buffer API with explicit sizes
			// Preserve record assignment semantics and compiler padding behavior
			ScriptPaths_BackendReadNode(ScriptPaths_aArray[i].m_pNode,j,buf);
			SkipSaveBuf(&buf, SCRIPT_PATH_NODE_SIZE);
			//- rouz edit (ChatGPT)
		}
	}
VALIDATESAVEBUF(size)
}

void ScriptPaths_Save(uint8_t *buf, uint32_t *size) {
    // Preserve every original live read, save cursor and state-write ordering
	*size = SCRIPTPATHS_SAVE_SIZE;
INITSAVEBUF
	for (int32_t i = 0; i < 3; i++) {
		// Preserve the original callback and traversal order
#ifdef COMPATIBLE_SAVES
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &ScriptPaths_aArray[i].m_numNodes, sizeof(ScriptPaths_aArray[i].m_numNodes));
		ZeroSaveBuf(&buf, 4);
		WriteSaveBuf(&buf, &ScriptPaths_aArray[i].m_fTotalLength, sizeof(ScriptPaths_aArray[i].m_fTotalLength));
		WriteSaveBuf(&buf, &ScriptPaths_aArray[i].m_fSpeed, sizeof(ScriptPaths_aArray[i].m_fSpeed));
		WriteSaveBuf(&buf, &ScriptPaths_aArray[i].m_fPosition, sizeof(ScriptPaths_aArray[i].m_fPosition));
		WriteSaveBuf(&buf, &ScriptPaths_aArray[i].m_fObjectLength, sizeof(ScriptPaths_aArray[i].m_fObjectLength));
		WriteSaveBuf(&buf, &ScriptPaths_aArray[i].m_state, sizeof(ScriptPaths_aArray[i].m_state));
		//- rouz edit (ChatGPT)
#else
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		// Preserve record assignment semantics and compiler padding behavior
		ScriptPath *pPath = (ScriptPath*)buf;
		*pPath = ScriptPaths_aArray[i];
		SkipSaveBuf(&buf, sizeof(ScriptPaths_aArray[i]));
		//- rouz edit (ChatGPT)
#endif

		for (int32_t j = 0; j < 6; j++) {
			// Preserve the original callback and traversal order
#ifdef COMPATIBLE_SAVES
			//+ rouz edit (ChatGPT)
			// Transfer save data through the C buffer API with explicit sizes
			{
				// Preserve the original callback and traversal order
				// Materialize the saved value with its original serialized type
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				int32_t saveValue = ScriptPaths_aArray[i].m_pObjects[j] != NULL ? CPool_GetJustIndex_NoFreeAssert(ScriptPaths_BackendObjectPool(), ScriptPaths_aArray[i].m_pObjects[j]) + 1 : 0;
				//- rouz edit (ChatGPT)
				WriteSaveBuf(&buf, &saveValue, sizeof(saveValue));
			}
			//- rouz edit (ChatGPT)
#else
			if (pPath->m_pObjects[j] != NULL)
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				pPath->m_pObjects[j] = (CObject*)(CPool_GetJustIndex_NoFreeAssert(ScriptPaths_BackendObjectPool(), pPath->m_pObjects[j]) + 1);
				//- rouz edit (ChatGPT)
#endif
		}

		for (int32_t j = 0; j < ScriptPaths_aArray[i].m_numNodes; j++) {
			// Preserve the original callback and traversal order
			//+ rouz edit (ChatGPT)
			// Transfer save data through the C buffer API with explicit sizes
			// Preserve record assignment semantics and compiler padding behavior
			ScriptPaths_BackendWriteNode(buf,ScriptPaths_aArray[i].m_pNode,j);
			SkipSaveBuf(&buf, SCRIPT_PATH_NODE_SIZE);
			//- rouz edit (ChatGPT)
			*size += SCRIPT_PATH_NODE_SIZE;
		}
	}
VALIDATESAVEBUF(*size);
}

void ScriptPaths_Load_ForReplay(void) {
    // Preserve every original live read, save cursor and state-write ordering
	for (int i = 0; i < 3; i++) {
		// Preserve the original callback and traversal order
		for (int32_t j = 0; j < 6; j++) {
			// Preserve the original callback and traversal order
			ScriptPaths_aArray[i].m_pObjects[j] = g_pScriptPathObjects[6 * i + j];
		}
	}
}

void ScriptPaths_Save_ForReplay(void) {
    // Preserve every original live read, save cursor and state-write ordering
	for (int i = 0; i < 3; i++) {
		// Preserve the original callback and traversal order
		for (int32_t j = 0; j < 6; j++) {
			// Preserve the original callback and traversal order
			g_pScriptPathObjects[6 * i + j] = ScriptPaths_aArray[i].m_pObjects[j];
		}
	}
}

static void ScriptPaths_UpdateObject(ScriptPath *path,int32_t i,ScriptPathGeometry geometry,void *previous)
{
    // Reuse the same captured geometry throughout the original per-object lifetime
    ScriptPathVector pos1=geometry.pos1,pos2=geometry.pos2,newRight=geometry.right,newUp=geometry.up,newForward=geometry.forward;
				ScriptPathVector prevPosition=ScriptPaths_BackendPosition(path->m_pObjects[i]);

				ScriptPaths_BackendSetPosition(path->m_pObjects[i],ScriptPaths_Divide(ScriptPaths_Add(pos1,pos2),2.0f));
				ScriptPaths_BackendSetRight(path->m_pObjects[i],newRight);
				ScriptPaths_BackendSetUp(path->m_pObjects[i],newUp);
				ScriptPaths_BackendSetForward(path->m_pObjects[i],newForward);
				ScriptPaths_BackendUpdateRW(path->m_pObjects[i]);
				ScriptPaths_BackendUpdateFrame(path->m_pObjects[i]);

				if (!ScriptPaths_BackendBigBuilding(path->m_pObjects[i]) && ScriptPaths_Different(prevPosition,ScriptPaths_BackendPosition(path->m_pObjects[i])))
					ScriptPaths_BackendRemoveAndAdd(path->m_pObjects[i]);

				ScriptPaths_BackendUpdateRW(path->m_pObjects[i]);
				ScriptPaths_BackendUpdateFrame(path->m_pObjects[i]);

				ScriptPaths_BackendSetMoveSpeed(path->m_pObjects[i],ScriptPaths_Divide(ScriptPaths_Subtract(ScriptPaths_BackendPosition(path->m_pObjects[i]),ScriptPaths_BackendPreviousPosition(previous)),(*Timer_GetTimeStep())));

				float deltaAngle = ScriptPaths_Heading(ScriptPaths_BackendForward(path->m_pObjects[i])) - ScriptPaths_Heading(ScriptPaths_BackendPreviousForward(previous));
				while (deltaAngle < SCRIPT_PATHS_PI) deltaAngle += SCRIPT_PATHS_TWOPI;
				while (deltaAngle > SCRIPT_PATHS_PI) deltaAngle -= SCRIPT_PATHS_TWOPI;
				float zTurnSpeed = deltaAngle / (*Timer_GetTimeStep());

				ScriptPaths_BackendSetTurnSpeed(path->m_pObjects[i],(ScriptPathVector){0.0f, 0.0f, zTurnSpeed});
				ScriptPaths_BackendSetMoveFriction(path->m_pObjects[i],(ScriptPathVector){0.0f, 0.0f, 0.0f});
				ScriptPaths_BackendSetTurnFriction(path->m_pObjects[i],(ScriptPathVector){0.0f, 0.0f, 0.0f});
}

//- rouz edit (ChatGPT)
