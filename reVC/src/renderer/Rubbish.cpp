//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Weather.h"
#include "Camera.h"
#include "World.h"
#include "Vehicle.h"
#include "Stats.h"
#include "TxdStore.h"
#include "RenderBuffer.h"
#include "Rubbish_backend.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif

RubbishVector Rubbish_BackendCameraPosition(void)
{
	// Read the original complete typed camera position
	const CVector &v = TheCamera.GetPosition();
	RubbishVector result = {v.x, v.y, v.z};
	return result;
}
float Rubbish_BackendCameraX(void)
{
	// Read only the original live camera X component
	return TheCamera.GetPosition().x;
}
float Rubbish_BackendCameraY(void)
{
	// Read only the original live camera Y component
	return TheCamera.GetPosition().y;
}
float Rubbish_BackendCameraZ(void)
{
	// Read only the original live camera Z component
	return TheCamera.GetPosition().z;
}
float Rubbish_BackendOrientation(void)
{
	// Read the original live camera orientation
	return TheCamera.Orientation;
}
bool Rubbish_BackendEnvMap(void)
{
	// Read the original live environment-pass flag
	#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	return CustomPipes::bRenderingEnvMap;
#else
	return false;
#endif
}
void * Rubbish_BackendEnvCamera(void)
{
	// Read the original auxiliary camera pointer at each access point
	#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	return CustomPipes::EnvMapCam;
#else
	return NULL;
#endif
}
void * Rubbish_BackendCameraFrame(void *camera)
{
	// Read the actual typed auxiliary camera frame
	return RwCameraGetFrame((RwCamera *)camera);
}
void * Rubbish_BackendFrameMatrix(void *frame)
{
	// Read the actual typed frame matrix at each original query
	return RwFrameGetMatrix((RwFrame *)frame);
}
RubbishVector Rubbish_BackendMatrixPosition(void *matrix)
{
	// Copy the actual typed matrix position components
	const RwV3d &v = ((RwMatrix *)matrix)->pos;
	RubbishVector result = {v.x, v.y, v.z};
	return result;
}
uint16_t Rubbish_BackendVehicleSeed(CVehicle *veh)
{
	// Read the original entity seed through its actual owner
	return veh->m_randomSeed;
}
float Rubbish_BackendVehicleX(CVehicle *veh)
{
	// Read only the original live vehicle X component
	return veh->GetPosition().x;
}
float Rubbish_BackendVehicleY(CVehicle *veh)
{
	// Read only the original live vehicle Y component
	return veh->GetPosition().y;
}
RubbishVector Rubbish_BackendVehiclePosition(CVehicle *veh)
{
	// Copy the original complete vehicle position at the subtraction
	const CVector &v = veh->GetPosition();
	RubbishVector result = {v.x, v.y, v.z};
	return result;
}
float Rubbish_BackendSpeedX(CVehicle *veh)
{
	// Read only the original live vehicle speed X component
	return veh->GetMoveSpeed().x;
}
float Rubbish_BackendSpeedY(CVehicle *veh)
{
	// Read only the original live vehicle speed Y component
	return veh->GetMoveSpeed().y;
}
RubbishVector2 Rubbish_BackendSpeed(CVehicle *veh)
{
	// Copy only the original speed components used by two-dimensional math
	const CVector &v = veh->GetMoveSpeed();
	RubbishVector2 result = {v.x, v.y};
	return result;
}
RubbishVector2 Rubbish_BackendForward(CVehicle *veh)
{
	// Copy the original forward components at the original query
	const CVector &v = veh->GetForward();
	RubbishVector2 result = {v.x, v.y};
	return result;
}
RubbishVector2 Rubbish_BackendRight(CVehicle *veh)
{
	// Copy the original right components at the original query
	const CVector &v = veh->GetRight();
	RubbishVector2 result = {v.x, v.y};
	return result;
}
RubbishVector2 Rubbish_BackendBounds(CVehicle *veh)
{
	// Read the actual collision model and its original planar maximum
	const CollisionVector3 &v = veh->GetColModel()->boundingBox.max;
	RubbishVector2 result = {v.x, v.y};
	return result;
}
float Rubbish_BackendGround(float x, float y, float z, bool *found)
{
	// Enter the original ground query with its original output pointer
	return CWorld::FindGroundZFor3DCoord(x, y, z, found);
}
float Rubbish_BackendWind(void)
{
	// Read the original live wind scalar at each access point
	return Weather_Wind;
}
int32_t Rubbish_BackendPamphlet(void)
{
	// Read the original mission flag only when selecting the fourth raster
	return Stats_PamphletMissionPassed;
}
int32_t * Rubbish_BackendVertexCount(void)
{
	// Expose the actual shared vertex counter without copying it
	return &TempBufferVerticesStored;
}
int32_t * Rubbish_BackendIndexCount(void)
{
	// Expose the actual shared index counter without copying it
	return &TempBufferIndicesStored;
}
void Rubbish_BackendSetIndex(int index, uint32_t value)
{
	// Retain the original final narrowing into the typed shared index buffer
	TempBufferRenderIndexList[index] = value;
}




void * Rubbish_BackendTextureRead(const char *name, const char *mask)
{
	// Read the original typed texture resource
	return RwTextureRead(name, mask);
}
void Rubbish_BackendTextureDestroy(void *texture)
{
	// Destroy the original typed texture before C clears the handle
	RwTextureDestroy((RwTexture *)texture);
}
void * Rubbish_BackendRaster(void *texture)
{
	// Read the actual raster for the original texture handle
	return RwTextureGetRaster((RwTexture *)texture);
}
void Rubbish_BackendTextureRaster(void *raster)
{
	// Select the same typed texture raster
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
}
void Rubbish_BackendZWrite(bool enabled)
{
	// Set the original typed depth-write state
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)(uintptr_t)enabled);
}
void Rubbish_BackendVertexAlpha(bool enabled)
{
	// Set the original typed vertex-alpha state
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)(uintptr_t)enabled);
}
void Rubbish_BackendFog(bool enabled)
{
	// Set the original typed fog state
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)(uintptr_t)enabled);
}
void Rubbish_BackendSetPos(int index, float x, float y, float z)
{
	// Write the original typed shared vertex position
	RwIm3DVertexSetPos(&TempBufferRenderVertices[index], x, y, z);
}
void Rubbish_BackendSetRGBA(int index, uint8_t r, uint8_t g, uint8_t b, uint32_t alpha)
{
	// Retain the original final alpha narrowing in the typed color setter
	RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index], r, g, b, alpha);
}
void Rubbish_BackendSetU(int index, float value)
{
	// Write the original typed U coordinate
	RwIm3DVertexSetU(&TempBufferRenderVertices[index], value);
}
void Rubbish_BackendSetV(int index, float value)
{
	// Write the original typed V coordinate
	RwIm3DVertexSetV(&TempBufferRenderVertices[index], value);
}
void Rubbish_BackendLittleTest(void)
{
	// Enter the original hook after shared-buffer generation
	LittleTest();
}
bool Rubbish_BackendTransform(int count)
{
	// Retain the original shared vertices, null matrix and UV flag
	return RwIm3DTransform(TempBufferRenderVertices, count, nil, rwIM3D_VERTEXUV) != nil;
}
void Rubbish_BackendRenderIndexed(int count)
{
	// Render the original shared triangle-list index count
	RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, count);
}
void Rubbish_BackendEnd(void)
{
	// Finish the original successful draw
	RwIm3DEnd();
}
void Rubbish_BackendPush(const char *name)
{
	// Preserve the original debug group boundary
	PUSH_RENDERGROUP(name);
}
void Rubbish_BackendPop(void)
{
	// Close the original debug group when enabled
	POP_RENDERGROUP();
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
