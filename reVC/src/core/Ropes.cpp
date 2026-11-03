//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "ModelIndices.h"
#include "Streaming.h"
#include "CopPed.h"
#include "Population.h"
#include "RenderBuffer.h"
#include "Camera.h"
#include "Ropes_backend.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif

RopesVector Ropes_BackendCopyVector(const CVector &vector)
{
	// Copy the original typed value including temporary caller expressions
	RopesVector result = {vector.x, vector.y, vector.z};
	return result;
}
void Ropes_BackendStoreVector(void *destination, RopesVector vector)
{
	// Write the original typed output object only after a successful lookup
	*(CVector *)destination = CVector(vector.x, vector.y, vector.z);
}
bool Ropes_BackendEnvMap(void)
{
	// Read the live environment-pass state through its original owner
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	return CustomPipes::bRenderingEnvMap;
#else
	return false;
#endif
}
bool Ropes_BackendSphereVisible(RopesVector center, float radius)
{
	// Query the actual camera with the original typed sphere value
	return TheCamera.IsSphereVisible(CVector(center.x, center.y, center.z), radius);
}
void Ropes_BackendPush(const char *name)
{
	// Preserve the original debug render-group boundary
	PUSH_RENDERGROUP(name);
}
void Ropes_BackendPop(void)
{
	// Close the original render group when enabled
	POP_RENDERGROUP();
}
void Ropes_BackendSetRGBA(int index, int red, int green, int blue, int alpha)
{
	// Write the existing shared render buffer through the typed color setter
	RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index], red, green, blue, alpha);
}
void Ropes_BackendSetPos(int index, float x, float y, float z)
{
	// Write the same shared render buffer through the typed position setter
	RwIm3DVertexSetPos(&TempBufferRenderVertices[index], x, y, z);
}
void Ropes_BackendVertexAlpha(void)
{
	// Enable the original vertex-alpha state
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
}
void Ropes_BackendSrcBlend(void)
{
	// Select the original source blend mode
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
}
void Ropes_BackendDestBlend(void)
{
	// Select the original destination blend mode
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}
void Ropes_BackendTextureRaster(void)
{
	// Clear the original texture-raster state
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
}
bool Ropes_BackendTransform(int count)
{
	// Retain the original shared vertices, null matrix and XYZ/RGBA flags
	return RwIm3DTransform(TempBufferRenderVertices, count, nil, rwIM3D_VERTEXXYZ|rwIM3D_VERTEXRGBA) != nil;
}
void Ropes_BackendRenderIndexed(bool lineList, uint16_t *indices, int count)
{
	// Map the C algorithm's original primitive choice to the actual typed enum
	RwIm3DRenderIndexedPrimitive(lineList ? rwPRIMTYPELINELIST : rwPRIMTYPEPOLYLINE, indices, count);
}
void Ropes_BackendEnd(void)
{
	// End the original successful immediate-mode draw
	RwIm3DEnd();
}
bool Ropes_BackendSwatLoaded(void)
{
	// Query the original live SWAT model state
	return Streaming_HasModelLoaded(MI_SWAT); // rouz edit (ChatGPT)
}
CCopPed *Ropes_BackendAddSwat(RopesVector pos)
{
	// Retain the original population call and actual derived-pointer conversion
	return (CCopPed *)CPopulation::AddPed(PEDTYPE_COP, COP_HELI_SWAT, CVector(pos.x, pos.y, pos.z));
}
void Ropes_BackendDisableCollision(CCopPed *ped)
{
	// Write the original collision bit through its typed owner
	ped->bUsesCollision = false;
}
void Ropes_BackendRopeSentinel(CCopPed *ped)
{
	// Preserve the original entity-pointer sentinel exactly
	ped->m_pRopeEntity = (CEntity *)1;
}
void Ropes_BackendRopeID(CCopPed *ped, uint32_t id)
{
	// Assign the original wrapped 32-bit counter result to its pointer-width field
	ped->m_nRopeID = id;
}
void Ropes_BackendBlendSwat(CCopPed *ped)
{
	// Enter the original typed clump access and animation blend operation
	// Use the complete C animation manager at the original call boundary
	AnimManager_BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_ABSEIL, 4.0f);
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
