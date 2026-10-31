//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "TxdStore.h"
#include "Replay.h"
#include "Skidmarks_backend.h"

RwIm3DVertex SkidmarkVertices[SKIDMARK_LENGTH * 2];

const void *Skidmarks_BackendPosition(const CVector &position)
{
	// Borrow the original object without reading its fields before replay checks
	return &position;
}
SkidmarksVector Skidmarks_BackendReadPosition(const void *position)
{
	// Read the actual typed position only at its original assignment point
	const CVector *value = (const CVector *)position;
	SkidmarksVector result = {value->x, value->y, value->z};
	return result;
}




void *Skidmarks_BackendTextureRead(const char *name, const char *mask)
{
	// Retain the original typed texture load and names
	return RwTextureRead(name, mask);
}
void Skidmarks_BackendTextureDestroy(void *texture)
{
	// Destroy the original typed texture at the C algorithm's release point
	RwTextureDestroy((RwTexture *)texture);
}
void *Skidmarks_BackendRaster(void *texture)
{
	// Read the raster through the original typed texture API
	return RwTextureGetRaster((RwTexture *)texture);
}
void Skidmarks_BackendTextureRaster(void *raster)
{
	// Select the same raster in the original render state
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
}
void Skidmarks_BackendZWrite(bool enabled)
{
	// Preserve the original typed depth-write state
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)(uintptr_t)enabled);
}
void Skidmarks_BackendZTest(void)
{
	// Restore the original enabled depth-test state
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
}
void Skidmarks_BackendVertexAlpha(bool enabled)
{
	// Preserve the original typed vertex-alpha state
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)(uintptr_t)enabled);
}
void Skidmarks_BackendSrcBlend(void)
{
	// Set the original source blend function
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
}
void Skidmarks_BackendDestBlend(void)
{
	// Set the original destination blend function
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
}
void Skidmarks_BackendSetRGBA(int index, uint8_t red, uint8_t green, uint8_t blue, uint32_t alpha)
{
	// Retain the original final alpha narrowing in the typed vertex setter
	RwIm3DVertexSetRGBA(&SkidmarkVertices[index], red, green, blue, alpha);
}
void Skidmarks_BackendSetPos(int index, float x, float y, float z)
{
	// Write the original persistent vertex buffer through its typed position setter
	RwIm3DVertexSetPos(&SkidmarkVertices[index], x, y, z);
}
void Skidmarks_BackendSetU(int index, float value)
{
	// Write the original typed U coordinate
	RwIm3DVertexSetU(&SkidmarkVertices[index], value);
}
void Skidmarks_BackendSetV(int index, float value)
{
	// Write the original typed V coordinate
	RwIm3DVertexSetV(&SkidmarkVertices[index], value);
}
void Skidmarks_BackendLittleTest(void)
{
	// Preserve the original hook after all vertex updates
	LittleTest();
}
bool Skidmarks_BackendTransform(int count)
{
	// Retain the original vertex count, null matrix and UV flag
	return RwIm3DTransform(SkidmarkVertices, count, nil, rwIM3D_VERTEXUV) != nil;
}
void Skidmarks_BackendRenderIndexed(uint16_t *indices, int count)
{
	// Render the same triangle-list index count after a successful transform
	RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indices, count);
}
void Skidmarks_BackendEnd(void)
{
	// Finish the original successful draw
	RwIm3DEnd();
}
void Skidmarks_BackendPush(const char *name)
{
	// Preserve the original debug group boundary
	PUSH_RENDERGROUP(name);
}
void Skidmarks_BackendPop(void)
{
	// Close the original debug group when enabled
	POP_RENDERGROUP();
}
bool Skidmarks_BackendPlayingBack(void)
{
	// Query the original live replay owner before any position read
	return CReplay::IsPlayingBack();
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
