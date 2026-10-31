//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Lines_backend.h"

void *
Lines_BackendVertices(void)
{
	// Retain the original function-local typed vertex buffer and its lifetime
	static RwIm3DVertex v[2];
	return v;
}

void
Lines_BackendSetRGBA(void *vertices, int index, int red, int green, int blue, int alpha)
{
	// Write the original vertex color through RenderWare's typed setter
	RwIm3DVertexSetRGBA(&((RwIm3DVertex *)vertices)[index], red, green, blue, alpha);
}

void
Lines_BackendSetPos(void *vertices, int index, float x, float y, float z)
{
	// Write the original vertex position without assuming a backend layout
	RwIm3DVertexSetPos(&((RwIm3DVertex *)vertices)[index], x, y, z);
}

void
Lines_BackendLittleTest(void)
{
	// Preserve the original hook before each transform attempt
	LittleTest();
}

bool
Lines_BackendTransform(void *vertices)
{
	// Preserve both vertices, the null matrix and the original transform flags
	return RwIm3DTransform((RwIm3DVertex *)vertices, 2, nil, 0) != nil;
}

void
Lines_BackendRenderLine(void)
{
	// Render the same two vertex indices after a successful transform
	RwIm3DRenderLine(0, 1);
}

void
Lines_BackendEnd(void)
{
	// Finish the successful immediate-mode draw
	RwIm3DEnd();
}
//- rouz edit (ChatGPT)
