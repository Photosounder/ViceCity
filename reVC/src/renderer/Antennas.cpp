//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Antennas_backend.h"

static RwIm3DVertex vertexbufferA[2];

AntennasVector Antennas_BackendVector(const void *vector)
{
	// Read the actual vector components without assuming its C++ layout
	const CVector *value = (const CVector *)vector;
	AntennasVector result = {value->x, value->y, value->z};
	return result;
}
void Antennas_BackendPush(const char *name)
{
	// Preserve the original debug group boundary and FINAL configuration
	PUSH_RENDERGROUP(name);
}
void Antennas_BackendPop(void)
{
	// Close the original debug group when enabled
	POP_RENDERGROUP();
}
void Antennas_BackendZWrite(void)
{
	// Set the original typed depth-write state
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
}
void Antennas_BackendVertexAlpha(bool enabled)
{
	// Set the original typed vertex-alpha state
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)(uintptr_t)enabled);
}
void Antennas_BackendSrcBlend(void)
{
	// Set the original typed source blend mode
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
}
void Antennas_BackendDestBlend(void)
{
	// Set the original typed destination blend mode
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}
void Antennas_BackendTextureRaster(void)
{
	// Clear the original typed texture-raster state
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
}
void Antennas_BackendSetRGBA(int index, int red, int green, int blue, int alpha)
{
	// Write the original private vertex buffer through its typed color setter
	RwIm3DVertexSetRGBA(&vertexbufferA[index], red, green, blue, alpha);
}
void Antennas_BackendSetPos(int index, float x, float y, float z)
{
	// Write the same vertex storage through its typed position setter
	RwIm3DVertexSetPos(&vertexbufferA[index], x, y, z);
}
bool Antennas_BackendTransform(void)
{
	// Retain the original vertex count, null matrix and zero transform flags
	return RwIm3DTransform(vertexbufferA, 2, nil, 0) != nil;
}
void Antennas_BackendRenderLine(void)
{
	// Render the same two indices after a successful transform
	RwIm3DRenderLine(0, 1);
}
void Antennas_BackendEnd(void)
{
	// Finish the original successful immediate-mode draw
	RwIm3DEnd();
}
//- rouz edit (ChatGPT)
