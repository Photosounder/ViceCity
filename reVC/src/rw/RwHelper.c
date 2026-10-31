//+ rouz edit (ChatGPT)
#include "RwHelper.h"
#include "RwHelper_backend.h"
#include "Timecycle.h"
#include "Debug.h"
#include "MBlur.h"
#include <stddef.h>
#include <string.h>
#define nil NULL
#define TRUE 1
#define FALSE 0
void re3_assert(const char *expression,const char *file,unsigned int line,const char *function);
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
#define RwHelper_PackColour(r,g,b,a) ((uint32_t)(((a)<<24)|((r)<<16)|((g)<<8)|(b)))
static RwHelperRectRef RwHelper_LocalRect(RwHelperRect *rectangle)
{
    // Borrow direct scalar subobjects of the C-owned fallback rectangle
    RwHelperRectRef value={&rectangle->x,&rectangle->y,&rectangle->w,&rectangle->h};return value;
}
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)


#if !defined(FINAL) || defined(DEBUGMENU)
#endif
#ifndef FINAL
void *debugCharset;
bool bDebugRenderGroups;
#endif

#ifdef PS2_ALPHA_TEST
bool gPS2alphaTest = true;
#else
bool gPS2alphaTest = false;
#endif
bool gBackfaceCulling = true;

#if !defined(FINAL) || defined(DEBUGMENU)
static bool charsetOpen;
void OpenCharsetSafe(void)
{
    // Preserve the original rendering-helper operation and its live application state
	if(!charsetOpen)
		RwHelper_HostCharsetOpen();
	charsetOpen = true;
}
#endif

void CreateDebugFont(void)
{
    // Preserve the original rendering-helper operation and its live application state
#ifndef FINAL
	RwHelperColour color = { 255, 255, 128, 255 };
	RwHelperColour colorbg = { 0, 0, 0, 0 };
	OpenCharsetSafe();
	debugCharset = RwHelper_HostCharsetCreate(&color,&colorbg);
#endif
}

void DestroyDebugFont(void)
{
    // Preserve the original rendering-helper operation and its live application state
#ifndef FINAL
	RwHelper_HostCharsetDestroy(debugCharset);
	RwHelper_HostCharsetClose();
	charsetOpen = false;
#endif
}

void ObrsPrintfString(const char *str, short x, short y)
{
    // Preserve the original rendering-helper operation and its live application state
#ifndef FINAL
	RwHelper_HostCharsetPrint(debugCharset, str, x*8, y*16, true);
#endif
}

void FlushObrsPrintfs(void)
{
    // Preserve the original rendering-helper operation and its live application state
#ifndef FINAL
	RwHelper_HostCharsetFlush();
#endif
}

void
DefinedState(void)
{
    // Preserve the original rendering-helper operation and its live application state
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATETEXTUREADDRESS), (void*)(uintptr_t)RwHelper_HostConstant(RWH_CONST_rwTEXTUREADDRESSWRAP));
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATETEXTUREPERSPECTIVE), (void*)TRUE);
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATEZTESTENABLE), (void*)TRUE);
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATEZWRITEENABLE), (void*)TRUE);
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATESHADEMODE), (void*)(uintptr_t)RwHelper_HostConstant(RWH_CONST_rwSHADEMODEGOURAUD));
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATETEXTUREFILTER), (void*)(uintptr_t)RwHelper_HostConstant(RWH_CONST_rwFILTERLINEAR));
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void*)FALSE);
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATESRCBLEND), (void*)(uintptr_t)RwHelper_HostConstant(RWH_CONST_rwBLENDSRCALPHA));
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATEDESTBLEND), (void*)(uintptr_t)RwHelper_HostConstant(RWH_CONST_rwBLENDINVSRCALPHA));
	//RwHelper_HostRenderState(rwRENDERSTATEALPHAPRIMITIVEBUFFER, (void*)FALSE);
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATEBORDERCOLOR), (void*)(uintptr_t)RwHelper_PackColour(0, 0, 0, 255));
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATEFOGENABLE), (void*)FALSE);
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATEFOGCOLOR),
		(void*)(uintptr_t)RwHelper_PackColour(TimeCycle_GetFogRed(), TimeCycle_GetFogGreen(), TimeCycle_GetFogBlue(), 255));
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATEFOGTYPE), (void*)(uintptr_t)RwHelper_HostConstant(RWH_CONST_rwFOGTYPELINEAR));
	RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATECULLMODE), (void*)(uintptr_t)RwHelper_HostConstant(RWH_CONST_rwCULLMODECULLNONE));

#ifdef LIBRW
	RwHelper_HostLibSetState(RwHelper_HostConstant(RWH_CONST_LIB_ALPHATESTFUNC), RwHelper_HostConstant(RWH_CONST_LIB_ALPHAGREATEREQUAL));
	RwHelper_HostLibSetState(RwHelper_HostConstant(RWH_CONST_LIB_ALPHATESTREF), 3);

	RwHelper_HostLibSetState(RwHelper_HostConstant(RWH_CONST_LIB_GSALPHATEST), gPS2alphaTest);
#else
	// D3D stuff
	RwHelper_HostD3DSetState(RwHelper_HostConstant(RWH_CONST_D3DRS_ALPHAFUNC), RwHelper_HostConstant(RWH_CONST_D3DCMP_GREATER));
	RwHelper_HostD3DSetState(RwHelper_HostConstant(RWH_CONST_D3DRS_ALPHAREF), 2);
#endif
}

void
SetCullMode(uint32_t mode)
{
    // Preserve the original rendering-helper operation and its live application state
	if(gBackfaceCulling)
		RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATECULLMODE), (void*)(uintptr_t)mode);
	else
		RwHelper_HostRenderState(RwHelper_HostConstant(RWH_CONST_rwRENDERSTATECULLMODE), (void*)(uintptr_t)RwHelper_HostConstant(RWH_CONST_rwCULLMODECULLNONE));
}

#ifndef FINAL
void
PushRendergroup(const char *name)
{
    // Preserve the original rendering-helper operation and its live application state
	if(!bDebugRenderGroups)
		return;
#if defined(RW_OPENGL)
	if(RwHelper_HostGLDebugAvailable())
		RwHelper_HostGLDebugPush(name);
#elif defined(RW_D3D9)
	void *tmp=RwHelper_HostWideBuffer();
	RwHelper_HostToWide(name,tmp,256*sizeof(uint16_t));
	RwHelper_HostPerfBegin(0xFFFFFFFF,tmp);
#endif
}

void
PopRendergroup(void)
{
    // Preserve the original rendering-helper operation and its live application state
	if(!bDebugRenderGroups)
		return;
#if defined(RW_OPENGL)
	if(RwHelper_HostGLDebugAvailable())
		RwHelper_HostGLDebugPop();
#elif defined(RW_D3D9)
	RwHelper_HostPerfEnd();
#endif
}
#endif

void*
GetFirstFrameCallback(void *child, void *data)
{
    // Preserve the original rendering-helper operation and its live application state
	*(void**)data = child;
	return nil;
}

void*
RwHelper_GetFirstChild(void *frame)
{
    // Preserve the original rendering-helper operation and its live application state
	void *child;

	child = nil;
	RwHelper_HostChildren(frame, GetFirstFrameCallback, &child);
	return child;
}

void*
GetFirstObjectCallback(void *object, void *data)
{
    // Preserve the original rendering-helper operation and its live application state
	*(void**)data = object;
	return nil;
}

void*
GetFirstObject(void *frame)
{
    // Preserve the original rendering-helper operation and its live application state
	void *obj;

	obj = nil;
	RwHelper_HostObjects(frame, GetFirstObjectCallback, &obj);
	return obj;
}

void*
GetFirstAtomicCallback(void *atm, void *data)
{
    // Preserve the original rendering-helper operation and its live application state
	*(void**)data = atm;
	return nil;
}

void*
GetFirstAtomic(void *clump)
{
    // Preserve the original rendering-helper operation and its live application state
	void *atm;

	atm = nil;
	RwHelper_HostAtomics(clump, GetFirstAtomicCallback, &atm);
	return atm;
}

void*
GetFirstTextureCallback(void *tex, void *data)
{
    // Preserve the original rendering-helper operation and its live application state
	*(void**)data = tex;
	return nil;
}

void*
GetFirstTexture(void *txd)
{
    // Preserve the original rendering-helper operation and its live application state
	void *tex;

	tex = nil;
	RwHelper_HostTextures(txd, GetFirstTextureCallback, &tex);
	return tex;
}

bool
IsClumpSkinned(void *clump)
{
    // Preserve the original rendering-helper operation and its live application state
	void *atomic = GetFirstAtomic(clump);
	return atomic ? RwHelper_HostGeometrySkin(RwHelper_HostAtomicGeometry(atomic)) : nil;
}

static void*
GetAnimHierarchyCallback(void *atomic, void *data)
{
    // Preserve the original rendering-helper operation and its live application state
	*(void**)data = RwHelper_HostAtomicHierarchy(atomic);
	return nil;
}

void*
GetAnimHierarchyFromSkinClump(void *clump)
{
    // Preserve the original rendering-helper operation and its live application state
	void *hier = nil;
	RwHelper_HostAtomics(clump, GetAnimHierarchyCallback, &hier);
	return hier;
}

static void*
GetAnimHierarchyFromClumpCB(void *frame, void *data)
{
    // Preserve the original rendering-helper operation and its live application state
	void *hier = RwHelper_HostFrameHierarchy(frame);
	if(hier){
		*(void**)data = hier;
		return nil;
	}
	RwHelper_HostChildren(frame, GetAnimHierarchyFromClumpCB, data);
	return frame;
}

void*
GetAnimHierarchyFromClump(void *clump)
{
    // Preserve the original rendering-helper operation and its live application state
	void *hier = nil;
	RwHelper_HostChildren(RwHelper_HostClumpFrame(clump), GetAnimHierarchyFromClumpCB, &hier);
	return hier;
}

typedef struct RwHelper_SkinGetBonePositionsToTableScope {void * clump;void * boneTable;} RwHelper_SkinGetBonePositionsToTableScope;
static void RwHelper_SkinGetBonePositionsToTableBody(void *data,void *m,void *invmat)
{
    // Execute the entire original algorithm with per-invocation typed SDK storage
    RwHelper_SkinGetBonePositionsToTableScope *scope=(RwHelper_SkinGetBonePositionsToTableScope*)data;
    void * clump=scope->clump;
    void * boneTable=scope->boneTable;

	int i, parent;
	void *atomic;
	void *skin;
	void *hier;
	int numBones;
	
	int stack[32];
	int sp;

	if(boneTable == nil)
		return;

	atomic = GetFirstAtomic(clump);		// mobile, also VC
	assert(atomic);
	skin = RwHelper_HostGeometrySkin(RwHelper_HostAtomicGeometry(atomic));
	assert(skin);
	hier = GetAnimHierarchyFromSkinClump(clump);
	assert(hier);
	(*RwHelper_HostVectorField(RwHelper_HostVectorAt(boneTable,0),0)) = 0.0f;
	(*RwHelper_HostVectorField(RwHelper_HostVectorAt(boneTable,0),1)) = 0.0f;
	(*RwHelper_HostVectorField(RwHelper_HostVectorAt(boneTable,0),2)) = 0.0f;
	numBones = RwHelper_HostNumBones(skin);
	parent = 0;
	sp = 0;
#ifdef FIX_BUGS
	stack[0] = 0;	// i think this is ok
#endif
	for(i = 1; i < numBones; i++){
		RwHelper_HostCopyMatrix(m, RwHelper_HostMatrixAt(RwHelper_HostSkinMatrices(skin),i));
		RwHelper_HostInvertMatrix(invmat, m);
		const void *x = RwHelper_HostSkinMatrices(skin);
		RwHelper_HostTransformPoints(RwHelper_HostVectorAt(boneTable,i), RwHelper_HostMatrixPosition(invmat), 1, RwHelper_HostMatrixAt(x,parent));
		if(RwHelper_HostNodeFlags(hier,i) & RwHelper_HostConstant(RWH_CONST_rpHANIMPUSHPARENTMATRIX))
			stack[++sp] = parent;
		if(RwHelper_HostNodeFlags(hier,i) & RwHelper_HostConstant(RWH_CONST_rpHANIMPOPPARENTMATRIX))
			parent = stack[sp--];
		else
			parent = i;

		//assert(parent >= 0 && parent < numBones);
	}

}
void SkinGetBonePositionsToTable(void * clump,void * boneTable)
{
    // Keep native stack objects alive for the complete synchronous C operation
    RwHelper_SkinGetBonePositionsToTableScope scope={clump,boneTable};RwHelper_HostBoneMatrices(RwHelper_SkinGetBonePositionsToTableBody,&scope);
}


void*
HAnimAnimationCreateForHierarchy(void *hier)
{
    // Preserve the original rendering-helper operation and its live application state
	int i;
#if defined FIX_BUGS || defined LIBRW
	int numNodes = RwHelper_HostNumNodes(hier)*2;	// you're supposed to have at least two KFs per node
#else
	int numNodes = RwHelper_HostNumNodes(hier);
#endif
	void *anim = RwHelper_HostCreateAnimation(RwHelper_HostConstant(RWH_CONST_rpHANIMSTDKEYFRAMETYPEID), numNodes, 0, 0.0f);
	if(anim == nil)
		return nil;
	void *frame;
	for(i = 0; i < numNodes; i++){
		frame = (void*)RwHelper_HostAnimationFrame(anim,i);	// games uses struct size here, not safe
		(*RwHelper_HostKeyFrameField(frame,0)) = 1.0f;
		(*RwHelper_HostKeyFrameField(frame,1)) = (*RwHelper_HostKeyFrameField(frame,2)) = (*RwHelper_HostKeyFrameField(frame,3)) = 0.0f;
		(*RwHelper_HostKeyFrameField(frame,4)) = (*RwHelper_HostKeyFrameField(frame,5)) = (*RwHelper_HostKeyFrameField(frame,6)) = 0.0f;
#if defined FIX_BUGS || defined LIBRW
		// times are subtracted and divided giving NaNs
		// so they can't both be 0
		(*RwHelper_HostKeyFrameField(frame,7)) = i/RwHelper_HostNumNodes(hier);
#else
		(*RwHelper_HostKeyFrameField(frame,7)) = 0.0f;
#endif
		RwHelper_HostPreviousFrame(frame,nil);
	}
	return anim;
}

void
RenderSkeleton(void *hier)
{
    // Preserve the original rendering-helper operation and its live application state
	int i;
	int sp;
	int stack[32];
	int par;
	Debug_Vector p1, p2;
	int numNodes = RwHelper_HostNumNodes(hier);
	void *mats = RwHelper_HostHierarchyMatrices(hier);
	p1 = RwHelper_HostVectorValue(RwHelper_HostMatrixPosition(RwHelper_HostMatrixAt(mats,0)));

	par = 0;
	sp = 0;
	stack[sp++] = par;
	for(i = 1; i < numNodes; i++){
		p1 = RwHelper_HostVectorValue(RwHelper_HostMatrixPosition(RwHelper_HostMatrixAt(mats,par)));
		p2 = RwHelper_HostVectorValue(RwHelper_HostMatrixPosition(RwHelper_HostMatrixAt(mats,i)));
		Debug_AddLine(p1,p2,0xFFFFFFFF,0xFFFFFFFF);
		if(RwHelper_HostNodeFlags(hier,i) & RwHelper_HostConstant(RWH_CONST_rpHANIMPUSHPARENTMATRIX))
			stack[sp++] = par;
		par = i;
		if(RwHelper_HostNodeFlags(hier,i) & RwHelper_HostConstant(RWH_CONST_rpHANIMPOPPARENTMATRIX))
			par = stack[--sp];
	}
}


typedef struct RwHelper_Im2DRenderQuadScope {float x1;float y1;float x2;float y2;float z;float recipCamZ;float uvOffset;int32_t result;} RwHelper_Im2DRenderQuadScope;
static void RwHelper_Im2DRenderQuadBody(void *data,void *vx)
{
    // Execute the entire original algorithm with per-invocation typed SDK storage
    RwHelper_Im2DRenderQuadScope *scope=(RwHelper_Im2DRenderQuadScope*)data;
    float x1=scope->x1;
    float y1=scope->y1;
    float x2=scope->x2;
    float y2=scope->y2;
    float z=scope->z;
    float recipCamZ=scope->recipCamZ;
    float uvOffset=scope->uvOffset;

    

    /*
     *  Render an opaque white 2D quad at the given coordinates and
     *  spanning a whole texture.
     */

    RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(vx,0), x1);
    RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(vx,0), y1);
    RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(vx,0), z);
    RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(vx,0), 255, 255, 255, 255);
    RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(vx,0), recipCamZ);
    RwHelper_HostVertexU(RwHelper_HostVertexAt(vx,0), uvOffset, recipCamZ);
    RwHelper_HostVertexV(RwHelper_HostVertexAt(vx,0), uvOffset, recipCamZ);

    RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(vx,1), x1);
    RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(vx,1), y2);
    RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(vx,1), z);
    RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(vx,1), 255, 255, 255, 255);
    RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(vx,1), recipCamZ);
    RwHelper_HostVertexU(RwHelper_HostVertexAt(vx,1), uvOffset, recipCamZ);
    RwHelper_HostVertexV(RwHelper_HostVertexAt(vx,1), 1.0f + uvOffset, recipCamZ);

    RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(vx,2), x2);
    RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(vx,2), y1);
    RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(vx,2), z);
    RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(vx,2), 255, 255, 255, 255);
    RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(vx,2), recipCamZ);
    RwHelper_HostVertexU(RwHelper_HostVertexAt(vx,2), 1.0f + uvOffset, recipCamZ);
    RwHelper_HostVertexV(RwHelper_HostVertexAt(vx,2), uvOffset, recipCamZ);

    RwHelper_HostVertexScreenX(RwHelper_HostVertexAt(vx,3), x2);
    RwHelper_HostVertexScreenY(RwHelper_HostVertexAt(vx,3), y2);
    RwHelper_HostVertexScreenZ(RwHelper_HostVertexAt(vx,3), z);
    RwHelper_HostVertexIntRGBA(RwHelper_HostVertexAt(vx,3), 255, 255, 255, 255);
    RwHelper_HostVertexRecipCameraZ(RwHelper_HostVertexAt(vx,3), recipCamZ);
    RwHelper_HostVertexU(RwHelper_HostVertexAt(vx,3), 1.0f + uvOffset, recipCamZ);
    RwHelper_HostVertexV(RwHelper_HostVertexAt(vx,3), 1.0f + uvOffset, recipCamZ);

    RwHelper_HostDrawStrip(vx,4);

    {scope->result=TRUE;return;}

}
int32_t Im2DRenderQuad(float x1,float y1,float x2,float y2,float z,float recipCamZ,float uvOffset)
{
    // Keep native stack objects alive for the complete synchronous C operation
    RwHelper_Im2DRenderQuadScope scope={x1,y1,x2,y2,z,recipCamZ,uvOffset,0};RwHelper_HostQuadVertices(RwHelper_Im2DRenderQuadBody,&scope);return scope.result;
}


bool b_cbsUseLTM = true;

typedef struct RwHelper_cbsCalcMeanBSphereRadiusCBScope {void * atomic;void * originalData;void * result;} RwHelper_cbsCalcMeanBSphereRadiusCBScope;
static void RwHelper_cbsCalcMeanBSphereRadiusCBBody(void *storage,void *atomicPos,void *temp)
{
    // Execute the entire original algorithm with per-invocation typed SDK storage
    RwHelper_cbsCalcMeanBSphereRadiusCBScope *scope=(RwHelper_cbsCalcMeanBSphereRadiusCBScope*)storage;
    void * atomic=scope->atomic;
    void * data=scope->originalData;

	

	if ( b_cbsUseLTM )
		RwHelper_HostTransformPoints(atomicPos, RwHelper_HostSphereCenter(RwHelper_HostAtomicSphere(atomic)), 1, RwHelper_HostFrameLTM(RwHelper_HostClumpFrame(RwHelper_HostAtomicClump(atomic))));
	else
		RwHelper_HostTransformPoints(atomicPos, RwHelper_HostSphereCenter(RwHelper_HostAtomicSphere(atomic)), 1, RwHelper_HostFrameMatrix(RwHelper_HostClumpFrame(RwHelper_HostAtomicClump(atomic))));

	
	RwHelper_HostVectorSub(temp, atomicPos, RwHelper_HostSphereCenter(data));	
	float radius = RwHelper_HostVectorLength(temp) + (*RwHelper_HostSphereRadius(RwHelper_HostAtomicSphere(atomic)));
	
	if ( (*RwHelper_HostSphereRadius(data)) < radius )
		(*RwHelper_HostSphereRadius(data)) = radius;

	{scope->result=atomic;return;}

}
void * cbsCalcMeanBSphereRadiusCB(void * atomic,void * originalData)
{
    // Keep native stack objects alive for the complete synchronous C operation
    RwHelper_cbsCalcMeanBSphereRadiusCBScope scope={atomic,originalData,0};RwHelper_HostAtomicVectors(RwHelper_cbsCalcMeanBSphereRadiusCBBody,&scope);return scope.result;
}


typedef struct RwHelper_cbsCalcMeanBSphereCenterCBScope {void * atomic;void * originalData;void * result;} RwHelper_cbsCalcMeanBSphereCenterCBScope;
static void RwHelper_cbsCalcMeanBSphereCenterCBBody(void *storage,void *atomicPos,void *temp)
{
    // Execute the entire original algorithm with per-invocation typed SDK storage
    RwHelper_cbsCalcMeanBSphereCenterCBScope *scope=(RwHelper_cbsCalcMeanBSphereCenterCBScope*)storage;
    void * atomic=scope->atomic;
    void * data=scope->originalData;

	

	if ( b_cbsUseLTM )
		RwHelper_HostTransformPoints(atomicPos, RwHelper_HostSphereCenter(RwHelper_HostAtomicSphere(atomic)), 1, RwHelper_HostFrameLTM(RwHelper_HostClumpFrame(RwHelper_HostAtomicClump(atomic))));
	else
		RwHelper_HostTransformPoints(atomicPos, RwHelper_HostSphereCenter(RwHelper_HostAtomicSphere(atomic)), 1, RwHelper_HostFrameMatrix(RwHelper_HostClumpFrame(RwHelper_HostAtomicClump(atomic))));
	
	RwHelper_HostVectorAdd(RwHelper_HostSphereCenter(data), RwHelper_HostSphereCenter(data), atomicPos);
	
	{scope->result=atomic;return;}

}
void * cbsCalcMeanBSphereCenterCB(void * atomic,void * originalData)
{
    // Keep native stack objects alive for the complete synchronous C operation
    RwHelper_cbsCalcMeanBSphereCenterCBScope scope={atomic,originalData,0};RwHelper_HostAtomicVectors(RwHelper_cbsCalcMeanBSphereCenterCBBody,&scope);return scope.result;
}


typedef struct RwHelper_ClumpSphereViewScope {void * clump;RwHelperSphereRef sphere;bool useLTM;void * result;} RwHelper_ClumpSphereViewScope;
static void RwHelper_ClumpSphereViewBody(void *data,void *matrix,void *result)
{
    // Execute the entire original algorithm with per-invocation typed SDK storage
    RwHelper_ClumpSphereViewScope *scope=(RwHelper_ClumpSphereViewScope*)data;
    void * clump=scope->clump;
    RwHelperSphereRef sphere=scope->sphere;
    bool useLTM=scope->useLTM;

	
	RwHelper_HostStoreSphere(result,0.0f,0.0f,0.0f,0.0f);
	
	b_cbsUseLTM = useLTM;
 
	if ( clump == nil || sphere.radius == nil )
		{scope->result=nil;return;}
  
	(*sphere.radius) = 0.0f;
	(*sphere.x) = 0.0f;
	(*sphere.y) = 0.0f;
	(*sphere.z) = 0.0f;
    
	int32_t numAtomics = RwHelper_HostNumAtomics(clump);
	if ( numAtomics < 1.0f )
		{scope->result=nil;return;}
	
	RwHelper_HostAtomics(clump, cbsCalcMeanBSphereCenterCB, result);
	
	RwHelper_HostVectorScale(RwHelper_HostSphereCenter(result), RwHelper_HostSphereCenter(result), 1.0f/numAtomics);
	
	RwHelper_HostAtomics(clump, cbsCalcMeanBSphereRadiusCB, result);
	
	if ( b_cbsUseLTM )
		RwHelper_HostInvertMatrix(matrix, RwHelper_HostFrameLTM(RwHelper_HostClumpFrame(clump)));
	else
		RwHelper_HostInvertMatrix(matrix, RwHelper_HostFrameMatrix(RwHelper_HostClumpFrame(clump)));
	
	RwHelper_HostTransformPoints(RwHelper_HostSphereCenter(result), RwHelper_HostSphereCenter(result), 1, matrix);

	// Copy defined SDK result fields into the actual caller receiver without casting its containing C record
    void *resultCenter=RwHelper_HostSphereCenter(result);
    memcpy(sphere.x,RwHelper_HostVectorField(resultCenter,0),sizeof(float));
    memcpy(sphere.y,RwHelper_HostVectorField(resultCenter,1),sizeof(float));
    memcpy(sphere.z,RwHelper_HostVectorField(resultCenter,2),sizeof(float));
    memcpy(sphere.radius,RwHelper_HostSphereRadius(result),sizeof(float));
	
	{scope->result=clump;return;}

}
void * RwHelper_ClumpSphereView(void * clump,RwHelperSphereRef sphere,bool useLTM)
{
    // Keep native stack objects alive for the complete synchronous C operation
    RwHelper_ClumpSphereViewScope scope={clump,sphere,useLTM,0};RwHelper_HostBoundingSphere(RwHelper_ClumpSphereViewBody,&scope);return scope.result;
}


void *RpClumpGetBoundingSphere(void *clump,void *sphere,bool useLTM)
{
    // Retain the original SDK-receiver ABI and ignored-pointer behavior for null clumps
    RwHelperSphereRef receiver={NULL,NULL,NULL,NULL};
    if(clump&&sphere){
        // Borrow actual SDK scalar subobjects while the complete policy remains in the C view operation
        void *center=RwHelper_HostSphereCenter(sphere);receiver.x=RwHelper_HostVectorField(center,0);receiver.y=RwHelper_HostVectorField(center,1);receiver.z=RwHelper_HostVectorField(center,2);receiver.radius=RwHelper_HostSphereRadius(sphere);
    }
    return RwHelper_ClumpSphereView(clump,receiver,useLTM);
}

void CameraSize(void *camera,void *rectangle,float viewWindow,float aspectRatio)
{
    // Preserve the original rendering-helper operation and its live application state
	if (camera)
	{
        RwHelperRectRef rect=RwHelper_HostRect(rectangle);
		RwHelperVideoMode         videoMode;
		RwHelperRect              r;
		RwHelperRect              origSize = { 0, 0, 0, 0 };	// FIX just to make the compier happy
		RwHelperVector2               vw;

		RwHelper_HostVideoMode(&videoMode,
								 RwHelper_HostCurrentVideoMode());

		origSize.w  = RwHelper_HostRasterWidth(RwHelper_HostCameraRaster(camera));
		origSize.h = RwHelper_HostRasterHeight(RwHelper_HostCameraRaster(camera));

		if (!rect.w)
		{
			if (videoMode.flags & RwHelper_HostConstant(RWH_CONST_rwVIDEOMODEEXCLUSIVE))
			{
				/* For full screen applications, resizing the camera just doesn't
				 * make sense, use the video mode size.
				 */

				r.x = r.y = 0;
				r.w = videoMode.width;
				r.h = videoMode.height;
				rect = RwHelper_LocalRect(&r);
			}
			else
			{
				/*
				rect not specified - reuse current values
				*/
				r.w = RwHelper_HostRasterWidth(RwHelper_HostCameraRaster(camera));
				r.h = RwHelper_HostRasterHeight(RwHelper_HostCameraRaster(camera));
				r.x = r.y = 0;
				rect = RwHelper_LocalRect(&r);
			}
		}

		if (( origSize.w != (*rect.w) ) || ( origSize.h != (*rect.h) ))
		{
			void           *raster;
			void           *zRaster;

			// BUG: game just changes camera raster's sizes, but this is a hack
#if defined FIX_BUGS || defined LIBRW
			/*
			 * Destroy rasters...
			 */

			raster = RwHelper_HostCameraRaster(camera);
			if( raster )
			{
				RwHelper_HostDestroyRaster(raster);
				RwHelper_HostStoreCameraRaster(camera,nil);
			}

			zRaster = RwHelper_HostCameraZRaster(camera);
			if( zRaster )
			{
				RwHelper_HostDestroyRaster(zRaster);
				RwHelper_HostStoreCameraZRaster(camera,nil);
			}

			/*
			 * Create new rasters...
			 */

			raster = RwHelper_HostCreateRaster((*rect.w), (*rect.h), 0, RwHelper_HostConstant(RWH_CONST_rwRASTERTYPECAMERA));
			zRaster = RwHelper_HostCreateRaster((*rect.w), (*rect.h), 0, RwHelper_HostConstant(RWH_CONST_rwRASTERTYPEZBUFFER));

			if( raster && zRaster )
			{
				RwHelper_HostSetCameraRaster(camera, raster);
				RwHelper_HostSetCameraZRaster(camera, zRaster);
			}
			else
			{
				if( raster )
				{
					RwHelper_HostDestroyRaster(raster);
				}

				if( zRaster )
				{
					RwHelper_HostDestroyRaster(zRaster);
				}

				(*rect.x) = origSize.x;
				(*rect.y) = origSize.y;
				(*rect.w) = origSize.w;
				(*rect.h) = origSize.h;

				/*
				 * Use default values...
				 */
				raster =
					RwHelper_HostCreateRaster((*rect.w), (*rect.h), 0, RwHelper_HostConstant(RWH_CONST_rwRASTERTYPECAMERA));

				zRaster =
					RwHelper_HostCreateRaster((*rect.w), (*rect.h), 0, RwHelper_HostConstant(RWH_CONST_rwRASTERTYPEZBUFFER));

				RwHelper_HostSetCameraRaster(camera, raster);
				RwHelper_HostSetCameraZRaster(camera, zRaster);
			}
#else
			raster = RwHelper_HostCameraRaster(camera);
			zRaster = RwHelper_HostCameraZRaster(camera);
			
			(*RwHelper_HostRasterWidthSlot(raster)) = (*RwHelper_HostRasterWidthSlot(zRaster)) = (*rect.w);
			(*RwHelper_HostRasterHeightSlot(raster)) = (*RwHelper_HostRasterHeightSlot(zRaster)) = (*rect.h);
#endif
#ifdef FIX_BUGS
			// Use the complete C motion-blur owner and its live shared state
			if(MBlur_BlurOn){
				// Use the complete C motion-blur owner and its live shared state
				MBlur_MotionBlurClose();
				// Use the complete C motion-blur owner and its live shared state
				MBlur_MotionBlurOpen(camera);
			}
#endif
		}

		/* Figure out the view window */
		if (videoMode.flags & RwHelper_HostConstant(RWH_CONST_rwVIDEOMODEEXCLUSIVE))
		{
			/* derive ratio from aspect ratio */
			vw.x = viewWindow;
			vw.y = viewWindow / aspectRatio;
		}
		else
		{
			/* derive from pixel ratios */
			if ((*rect.w) > (*rect.h))
			{
				vw.x = viewWindow;
				vw.y = ((*rect.h) * viewWindow) / (*rect.w);
			}
			else
			{
				vw.x = ((*rect.w) * viewWindow) / (*rect.h);
				vw.y = viewWindow;
			}
		}

		RwHelper_HostViewWindow(camera, &vw);

		RwHelper_HostScreenWidth((*rect.w));
		RwHelper_HostScreenHeight((*rect.h));
	}

	return;
}


void
CameraDestroy(void *camera)
{
    // Preserve the original rendering-helper operation and its live application state
	void    *raster, *tmpRaster;
	void     *frame;

	if (camera)
	{
		frame = RwHelper_HostCameraFrame(camera);
		if (frame)
		{
			RwHelper_HostDestroyFrame(frame);
		}

		raster = RwHelper_HostCameraRaster(camera);
		if (raster)
		{
			tmpRaster = RwHelper_HostRasterParent(raster);

			RwHelper_HostDestroyRaster(raster);

			if ((tmpRaster != nil) && (tmpRaster != raster))
			{
				RwHelper_HostDestroyRaster(tmpRaster);
			}
		}

		raster = RwHelper_HostCameraZRaster(camera);
		if (raster)
		{
			tmpRaster = RwHelper_HostRasterParent(raster);

			RwHelper_HostDestroyRaster(raster);

			if ((tmpRaster != nil) && (tmpRaster != raster))
			{
				RwHelper_HostDestroyRaster(tmpRaster);
			}
		}

		RwHelper_HostDestroyCamera(camera);
	}

	return;
}

void           *
CameraCreate(int32_t width, int32_t height, int32_t zBuffer)
{
    // Preserve the original rendering-helper operation and its live application state
	void           *camera;

	camera = RwHelper_HostCreateCamera();

	if (camera)
	{
		RwHelper_HostSetCameraFrame(camera, RwHelper_HostCreateFrame());
		RwHelper_HostSetCameraRaster(camera,
						  RwHelper_HostCreateRaster(0, 0, 0, RwHelper_HostConstant(RWH_CONST_rwRASTERTYPECAMERA)));

		if (zBuffer)
		{
			RwHelper_HostSetCameraZRaster(camera,
							   RwHelper_HostCreateRaster(0, 0, 0,
											  RwHelper_HostConstant(RWH_CONST_rwRASTERTYPEZBUFFER)));
		}

		/* now check that everything is valid */
		if (RwHelper_HostCameraFrame(camera) &&
			RwHelper_HostCameraRaster(camera) &&
			RwHelper_HostRasterParent(RwHelper_HostCameraRaster(camera)) &&
			(!zBuffer || (RwHelper_HostCameraZRaster(camera) &&
						  RwHelper_HostRasterParent(RwHelper_HostCameraZRaster
											(camera)))))
		{
			/* everything OK */
			return (camera);
		}
	}

	/* if we're here then an error must have occurred so clean up */

	CameraDestroy(camera);
	return (nil);
}

#ifdef LIBRW

int32_t
findPlatform(void *a)
{
    // Preserve the original rendering-helper operation and its live application state
	void *g=RwHelper_HostAtomicGeometryField(a);
	if(RwHelper_HostInstanceData(g))
		return RwHelper_HostInstancePlatform(RwHelper_HostInstanceData(g));
	return 0;
}

// Game doesn't read atomic extensions so we never get any other than the default pipe,
// but we need it for uninstancing
void
attachPipe(void *atomic)
{
    // Preserve the original rendering-helper operation and its live application state
	if(RwHelper_HostGeometrySkin(RwHelper_HostAtomicGeometry(atomic)))
		RwHelper_HostSetPipeline(atomic,RwHelper_HostSkinPipeline(RwHelper_HostPlatform()));
	else{
		int fx = RwHelper_HostConstant(RWH_CONST_rpMATFXEFFECTNULL);
		RwHelper_HostForMaterials(RwHelper_HostAtomicGeometry(atomic),&fx);
		if(fx != RwHelper_HostConstant(RWH_CONST_rpMATFXEFFECTNULL))
			RwHelper_HostMaterialEffects(atomic);
	}
}

// Attach pipes for the platform we have native data for so we can uninstance
void
switchPipes(void *a, int32_t platform)
{
    // Preserve the original rendering-helper operation and its live application state
	if(RwHelper_HostPipeline(a) && RwHelper_HostPipelinePlatform(RwHelper_HostPipeline(a)) != platform){
		uint32_t plgid = RwHelper_HostPipelinePlugin(RwHelper_HostPipeline(a));
		if(plgid==RwHelper_HostConstant(RWH_CONST_LIB_ID_SKIN)){
			RwHelper_HostSetPipeline(a,RwHelper_HostSkinPipeline(platform));
			}else if(plgid==RwHelper_HostConstant(RWH_CONST_LIB_ID_MATFX)){
			RwHelper_HostSetPipeline(a,RwHelper_HostMatFXPipeline(platform));
			}
	}
}

void*
RwHelper_ConvertPlatformAtomic(void *atomic, void *data)
{
    // Preserve the original rendering-helper operation and its live application state
	int32_t driver = RwHelper_HostPlatform();
	int32_t platform = findPlatform(atomic);
	if(platform != 0 && platform != driver){
		attachPipe(atomic);	// kludge
		void *origPipe=RwHelper_HostPipeline(atomic);
		RwHelper_HostStorePlatform(platform);
		switchPipes(atomic, RwHelper_HostPlatform());
		if(RwHelper_HostGeometryFlags(RwHelper_HostAtomicGeometryField(atomic)) & RwHelper_HostConstant(RWH_CONST_LIB_Geometry_NATIVE))
			RwHelper_HostUninstance(atomic);
		// no ADC in this game
		//rw::ps2::unconvertADC(atomic->geometry);
		RwHelper_HostStorePlatform(driver);
		RwHelper_HostSetPipeline(atomic,origPipe);
	}
	return atomic;
}
#endif

#if defined(FIX_BUGS) && defined(GTA_PC)
uint32_t saved_alphafunc, saved_alpharef;

void
SetAlphaTest(uint32_t alpharef)
{
    // Preserve the original rendering-helper operation and its live application state
#ifdef LIBRW
	saved_alphafunc = RwHelper_HostLibGetState(RwHelper_HostConstant(RWH_CONST_LIB_ALPHATESTFUNC));
	saved_alpharef = RwHelper_HostLibGetState(RwHelper_HostConstant(RWH_CONST_LIB_ALPHATESTREF));

	RwHelper_HostLibSetState(RwHelper_HostConstant(RWH_CONST_LIB_ALPHATESTFUNC), RwHelper_HostConstant(RWH_CONST_LIB_ALPHAGREATEREQUAL));
	RwHelper_HostLibSetState(RwHelper_HostConstant(RWH_CONST_LIB_ALPHATESTREF), 0);
#else
	RwHelper_HostD3DGetState(RwHelper_HostConstant(RWH_CONST_D3DRS_ALPHAFUNC), &saved_alphafunc);
	RwHelper_HostD3DGetState(RwHelper_HostConstant(RWH_CONST_D3DRS_ALPHAREF), &saved_alpharef);
	
	RwHelper_HostD3DSetState(RwHelper_HostConstant(RWH_CONST_D3DRS_ALPHAFUNC), RwHelper_HostConstant(RWH_CONST_D3DCMP_GREATEREQUAL));
	RwHelper_HostD3DSetState(RwHelper_HostConstant(RWH_CONST_D3DRS_ALPHAREF), alpharef);
#endif
}

void
RestoreAlphaTest(void)
{
    // Preserve the original rendering-helper operation and its live application state
#ifdef LIBRW
	RwHelper_HostLibSetState(RwHelper_HostConstant(RWH_CONST_LIB_ALPHATESTFUNC), saved_alphafunc);
	RwHelper_HostLibSetState(RwHelper_HostConstant(RWH_CONST_LIB_ALPHATESTREF), saved_alpharef);
#else
	RwHelper_HostD3DSetState(RwHelper_HostConstant(RWH_CONST_D3DRS_ALPHAFUNC), saved_alphafunc);
	RwHelper_HostD3DSetState(RwHelper_HostConstant(RWH_CONST_D3DRS_ALPHAREF), saved_alpharef);
#endif
}
#endif
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
