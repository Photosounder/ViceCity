//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Pools.h"
#include "Ped.h"
#include "AnimManager.h"
#include "Fire.h"
#include "WaterLevel.h"
#include "Camera.h"
#include "Particle.h"
#include "WaterCannon_backend.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif
RwIm3DVertex WaterCannonVertices[4];

WaterCannonVector WaterCannon_BackendReadVector(const void *value)
{
	// Read the original typed legacy input only at its copy point
	const CVector *v = (const CVector *)value;
	WaterCannonVector result = {v->x, v->y, v->z};
	return result;
}
WaterCannonVector WaterCannon_BackendCameraForward(void)
{
	// Copy the original typed camera forward value
	const CVector &v = TheCamera.GetForward();
	WaterCannonVector result = {v.x, v.y, v.z};
	return result;
}
bool WaterCannon_BackendEnvMap(void)
{
	// Read the live environment-pass flag at each original access
	#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	return CustomPipes::bRenderingEnvMap;
#else
	return false;
#endif
}
void * WaterCannon_BackendEnvCamera(void)
{
	// Read the original auxiliary camera pointer
	#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	return CustomPipes::EnvMapCam;
#else
	return NULL;
#endif
}
void * WaterCannon_BackendCameraFrame(void *camera)
{
	// Query the actual typed auxiliary camera frame
	return RwCameraGetFrame((RwCamera *)camera);
}
void * WaterCannon_BackendFrameMatrix(void *frame)
{
	// Query the actual typed frame matrix
	return RwFrameGetMatrix((RwFrame *)frame);
}
WaterCannonVector WaterCannon_BackendMatrixForward(void *matrix)
{
	// Copy the actual matrix forward vector before C normalization
	const RwV3d &v = ((RwMatrix *)matrix)->at;
	WaterCannonVector result = {v.x, v.y, v.z};
	return result;
}
bool WaterCannon_BackendExtinguish(WaterCannonVector pos, float range)
{
	// Enter the original fire query at its eligibility gate
	return FireManager_ExtinguishPointWithWater(&gFireManager,Fire_VectorFromNative(CVector(pos.x, pos.y, pos.z)), range);
}
CPool * WaterCannon_BackendPedPool(void)
{
	// Read the original live ped pool at every query
	return CPools::GetPedPool();
}
CPed * WaterCannon_BackendPed(void *slot)
{
	// Convert actual raw pool storage through the original typed owner
	return (CPed *)slot;
}
float WaterCannon_BackendPedX(CPed *ped)
{
	// Read only the original live ped X component
	return ped->GetPosition().x;
}
float WaterCannon_BackendPedY(CPed *ped)
{
	// Read only the original live ped Y component
	return ped->GetPosition().y;
}
float WaterCannon_BackendPedZ(CPed *ped)
{
	// Read only the original live ped Z component
	return ped->GetPosition().z;
}
WaterCannonVector WaterCannon_BackendPedPosition(CPed *ped)
{
	// Copy the original complete ped position at its use point
	const CVector &v = ped->GetPosition();
	WaterCannonVector result = {v.x, v.y, v.z};
	return result;
}
int32_t WaterCannon_BackendLocalDirection(CPed *ped)
{
	// Enter the original typed local-direction query
	return ped->GetLocalDirection(CVector2D(1.0f, 0.0f));
}
void WaterCannon_BackendStanding(CPed *ped)
{
	// Clear the original standing bit through its actual owner
	ped->bIsStanding = false;
}
void WaterCannon_BackendForce(CPed *ped, float z)
{
	// Apply the original typed vertical force after C computes its magnitude
	ped->ApplyMoveForce(0.0f, 0.0f, z);
}
float WaterCannon_BackendSpeedX(CPed *ped)
{
	// Read the current ped X velocity at its original use
	return ped->m_vecMoveSpeed.x;
}
float WaterCannon_BackendSpeedY(CPed *ped)
{
	// Read the current ped Y velocity at its original use
	return ped->m_vecMoveSpeed.y;
}
void WaterCannon_BackendSetSpeedX(CPed *ped, float value)
{
	// Write the original ped X velocity at its original point
	ped->m_vecMoveSpeed.x = value;
}
void WaterCannon_BackendSetSpeedY(CPed *ped, float value)
{
	// Write the original ped Y velocity at its original point
	ped->m_vecMoveSpeed.y = value;
}
WaterCannonVector WaterCannon_BackendVelocity(CPed *ped)
{
	// Copy the original full ped velocity for particle direction math
	const CVector &v = ped->m_vecMoveSpeed;
	WaterCannonVector result = {v.x, v.y, v.z};
	return result;
}
void WaterCannon_BackendFall(CPed *ped, int32_t animation)
{
	// Enter the original typed fall animation after C selects its ID
	ped->SetFall(2000, (AnimationId)animation, 0);
}
void WaterCannon_BackendParticle(int32_t kind, WaterCannonVector pos, WaterCannonVector dir, int32_t rotationSpeed, int32_t rotation, int32_t frame)
{
	// Enter the original typed particle overload with C-selected values
	CParticle::AddParticle((tParticleType)kind, CVector(pos.x, pos.y, pos.z), CVector(dir.x, dir.y, dir.z), NULL, 0.5f, rotationSpeed, rotation, frame);
}
void WaterCannon_BackendSetU(int index, float value)
{
	// Write the original typed shared U coordinate
	RwIm3DVertexSetU(&WaterCannonVertices[index], value);
}
void WaterCannon_BackendSetV(int index, float value)
{
	// Write the original typed shared V coordinate
	RwIm3DVertexSetV(&WaterCannonVertices[index], value);
}
void WaterCannon_BackendSetRGBA(int index, int32_t r, int32_t g, int32_t b, int32_t a)
{
	// Retain final narrowing in the actual typed color setter
	RwIm3DVertexSetRGBA(&WaterCannonVertices[index], r, g, b, a);
}
void WaterCannon_BackendSetPos(int index, float x, float y, float z)
{
	// Write the original typed shared vertex position
	RwIm3DVertexSetPos(&WaterCannonVertices[index], x, y, z);
}
void WaterCannon_BackendZWrite(bool enabled)
{
	// Set the original typed depth-write state
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)(uintptr_t)enabled);
}
void WaterCannon_BackendVertexAlpha(bool enabled)
{
	// Set the original typed vertex-alpha state
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)(uintptr_t)enabled);
}
void WaterCannon_BackendFog(bool enabled)
{
	// Set the original typed fog state
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)(uintptr_t)enabled);
}
void WaterCannon_BackendRaster(void)
{
	// Select the original live water raster
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)gpWaterRaster);
}
void WaterCannon_BackendLittleTest(void)
{
	// Enter the original hook after the four vertices are generated
	LittleTest();
}
bool WaterCannon_BackendTransform(void)
{
	// Retain the original buffer, count, null matrix and UV flag
	return RwIm3DTransform(WaterCannonVertices, 4, NULL, rwIM3D_VERTEXUV) != NULL;
}
void WaterCannon_BackendRenderIndexed(uint16_t *indices)
{
	// Draw the original twelve indices as a triangle list
	RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indices, 12);
}
void WaterCannon_BackendEnd(void)
{
	// Finish the original successful draw
	RwIm3DEnd();
}
void WaterCannon_BackendPush(const char *name)
{
	// Preserve the original debug group boundary
	PUSH_RENDERGROUP(name);
}
void WaterCannon_BackendPop(void)
{
	// Close the original debug group when enabled
	POP_RENDERGROUP();
}
//- rouz edit (ChatGPT)
