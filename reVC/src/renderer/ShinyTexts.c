//+ rouz edit (ChatGPT)
#include "ShinyTexts_backend.h"
#include "RenderBuffer_backend.h"
#include <math.h>
#include <stddef.h>

static float ShinyTexts_Distance(ShinyTextVector a,ShinyTextVector b)
{
    // Preserve the original vector subtraction, sum order and single-precision square root
    float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;return sqrtf(x*x+y*y+z*z);
}
static int32_t NumShinyTexts;
static ShinyText aShinyTexts[NUMSHINYTEXTS];
static ShinyText EnvMapShinyTexts[NUMSHINYTEXTS];
static int32_t EnvMapShinyTextCount;

void
ShinyTexts_Init(void)
{
	// Retain the complete original shiny-text algorithm and live operation order
	NumShinyTexts = 0;
	// Discard reflection geometry retained from an earlier scene
		EnvMapShinyTextCount = 0;
	}

void
ShinyTexts_RegisterOne(ShinyTextVector p0, ShinyTextVector p1, ShinyTextVector p2, ShinyTextVector p3,
	float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3,
	uint8_t type, uint8_t red, uint8_t green, uint8_t blue, float maxDist)
{
	// Retain the complete original shiny-text algorithm and live operation order
	if(NumShinyTexts >= NUMSHINYTEXTS)
		return;

	// Measure shiny text from the camera that is collecting this pass
		ShinyTextVector shinyTextCameraPosition = ShinyTexts_BackendCamera();
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(ShinyTexts_BackendEnvMap() && ShinyTexts_BackendEnvCamera() && ShinyTexts_BackendEnvFrame(ShinyTexts_BackendEnvCamera()))
		shinyTextCameraPosition = ShinyTexts_BackendMatrixPosition(ShinyTexts_BackendFrameLTM(ShinyTexts_BackendEnvFrame(ShinyTexts_BackendEnvCamera())));
#endif
	aShinyTexts[NumShinyTexts].m_camDist = ShinyTexts_Distance(p0,shinyTextCameraPosition);
		if(aShinyTexts[NumShinyTexts].m_camDist > maxDist)
		return;
	aShinyTexts[NumShinyTexts].m_verts[0] = p0;
	aShinyTexts[NumShinyTexts].m_verts[1] = p1;
	aShinyTexts[NumShinyTexts].m_verts[2] = p2;
	aShinyTexts[NumShinyTexts].m_verts[3] = p3;
	aShinyTexts[NumShinyTexts].m_texCoords[0].x = u0;
	aShinyTexts[NumShinyTexts].m_texCoords[0].y = v0;
	aShinyTexts[NumShinyTexts].m_texCoords[1].x = u1;
	aShinyTexts[NumShinyTexts].m_texCoords[1].y = v1;
	aShinyTexts[NumShinyTexts].m_texCoords[2].x = u2;
	aShinyTexts[NumShinyTexts].m_texCoords[2].y = v2;
	aShinyTexts[NumShinyTexts].m_texCoords[3].x = u3;
	aShinyTexts[NumShinyTexts].m_texCoords[3].y = v3;
	aShinyTexts[NumShinyTexts].m_type = type;
	aShinyTexts[NumShinyTexts].m_red = red;
	aShinyTexts[NumShinyTexts].m_green = green;
	aShinyTexts[NumShinyTexts].m_blue = blue;
	// Fade out at half the max dist
	float halfDist = maxDist*0.5f;
	if(aShinyTexts[NumShinyTexts].m_camDist > halfDist){
		// Preserve original conditions, shared writes and reflection lifetime
		float f = 1.0f - (aShinyTexts[NumShinyTexts].m_camDist - halfDist)/halfDist;
		aShinyTexts[NumShinyTexts].m_red *= f;
		aShinyTexts[NumShinyTexts].m_green *= f;
		aShinyTexts[NumShinyTexts].m_blue *= f;
	}

	NumShinyTexts++;
}

void
ShinyTexts_Render(void)
{
	// Retain the complete original shiny-text algorithm and live operation order
	int i, ix, v;
	void *lastTex = NULL;

	// Save main-view registrations without replacing them during a reflection replay
		bool saveForEnvMap = true;
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	saveForEnvMap = !ShinyTexts_BackendEnvMap();
#endif
	if(saveForEnvMap){
		// Preserve original conditions, shared writes and reflection lifetime
		EnvMapShinyTextCount = NumShinyTexts;
		for(i = 0; i < EnvMapShinyTextCount; i++)
			EnvMapShinyTexts[i] = aShinyTexts[i];
	}
		if(NumShinyTexts == 0)
		return;

	ShinyTexts_BackendVertexAlpha(true);
	ShinyTexts_BackendZWrite(false);
	ShinyTexts_BackendSrcBlend(true);
	ShinyTexts_BackendDestBlend(true);

	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;

	for(i = 0; i < NumShinyTexts; i++){
		// Preserve original conditions, shared writes and reflection lifetime
		if(TempBufferIndicesStored > TEMPBUFFERINDEXSIZE-64 || TempBufferVerticesStored > TEMPBUFFERVERTSIZE-62)
			ShinyTexts_RenderOutGeometryBuffer();

		uint8_t r = aShinyTexts[i].m_red;
		uint8_t g = aShinyTexts[i].m_green;
		uint8_t b = aShinyTexts[i].m_blue;

		switch(aShinyTexts[i].m_type){
			// Preserve original conditions, shared writes and reflection lifetime
		case SHINYTEXT_WALK:
			if(lastTex != ShinyTexts_BackendWalkTexture()){
				// Preserve original conditions, shared writes and reflection lifetime
				ShinyTexts_RenderOutGeometryBuffer();
				ShinyTexts_BackendSetRaster(ShinyTexts_BackendRaster(ShinyTexts_BackendWalkTexture()));
				lastTex = ShinyTexts_BackendWalkTexture();
			}
	quad:
			v = TempBufferVerticesStored;
			ShinyTexts_BackendSetRGBA(v+0, r, g, b, 255);
			ShinyTexts_BackendSetPos(v+0, aShinyTexts[i].m_verts[0].x, aShinyTexts[i].m_verts[0].y, aShinyTexts[i].m_verts[0].z);
			ShinyTexts_BackendSetU(v+0, aShinyTexts[i].m_texCoords[0].x);
			ShinyTexts_BackendSetV(v+0, aShinyTexts[i].m_texCoords[0].y);
			ShinyTexts_BackendSetRGBA(v+1, r, g, b, 255);
			ShinyTexts_BackendSetPos(v+1, aShinyTexts[i].m_verts[1].x, aShinyTexts[i].m_verts[1].y, aShinyTexts[i].m_verts[1].z);
			ShinyTexts_BackendSetU(v+1, aShinyTexts[i].m_texCoords[1].x);
			ShinyTexts_BackendSetV(v+1, aShinyTexts[i].m_texCoords[1].y);
			ShinyTexts_BackendSetRGBA(v+2, r, g, b, 255);
			ShinyTexts_BackendSetPos(v+2, aShinyTexts[i].m_verts[2].x, aShinyTexts[i].m_verts[2].y, aShinyTexts[i].m_verts[2].z);
			ShinyTexts_BackendSetU(v+2, aShinyTexts[i].m_texCoords[2].x);
			ShinyTexts_BackendSetV(v+2, aShinyTexts[i].m_texCoords[2].y);
			ShinyTexts_BackendSetRGBA(v+3, r, g, b, 255);
			ShinyTexts_BackendSetPos(v+3, aShinyTexts[i].m_verts[3].x, aShinyTexts[i].m_verts[3].y, aShinyTexts[i].m_verts[3].z);
			ShinyTexts_BackendSetU(v+3, aShinyTexts[i].m_texCoords[3].x);
			ShinyTexts_BackendSetV(v+3, aShinyTexts[i].m_texCoords[3].y);
			ix = TempBufferIndicesStored;
			TempBufferRenderIndexList[ix+0] = 0 + TempBufferVerticesStored;
			TempBufferRenderIndexList[ix+1] = 1 + TempBufferVerticesStored;
			TempBufferRenderIndexList[ix+2] = 2 + TempBufferVerticesStored;
			TempBufferRenderIndexList[ix+3] = 2 + TempBufferVerticesStored;
			TempBufferRenderIndexList[ix+4] = 1 + TempBufferVerticesStored;
			TempBufferRenderIndexList[ix+5] = 3 + TempBufferVerticesStored;
			TempBufferVerticesStored += 4;
			TempBufferIndicesStored += 6;
			break;

		case SHINYTEXT_FLAT:
			if(lastTex != NULL){
				// Preserve original conditions, shared writes and reflection lifetime
				ShinyTexts_RenderOutGeometryBuffer();
				ShinyTexts_BackendSetRaster(NULL);
				lastTex = NULL;
			}
			goto quad;
		}
	}

	ShinyTexts_RenderOutGeometryBuffer();
	NumShinyTexts = 0;

	ShinyTexts_BackendVertexAlpha(false);
	ShinyTexts_BackendZWrite(true);
	ShinyTexts_BackendSrcBlend(false);
	ShinyTexts_BackendDestBlend(false);
}

void
ShinyTexts_RenderForEnvMap(void)
{
	// Retain the complete original shiny-text algorithm and live operation order
	// Fall back to the saved main-view list only when this camera registered no text
		if(NumShinyTexts == 0 && EnvMapShinyTextCount > 0){
			// Preserve original conditions, shared writes and reflection lifetime
		for(int i = 0; i < EnvMapShinyTextCount; i++)
			aShinyTexts[i] = EnvMapShinyTexts[i];
		NumShinyTexts = EnvMapShinyTextCount;
	}
		if(NumShinyTexts <= 0)
		return;

	// Draw this camera's registered geometry through the standard renderer
	ShinyTexts_Render();

	// Clear reflection-only registrations and release this frame's saved list
	NumShinyTexts = 0;
	EnvMapShinyTextCount = 0;
}

void
ShinyTexts_RenderOutGeometryBuffer(void)
{
	// Retain the complete original shiny-text algorithm and live operation order
	if(TempBufferIndicesStored != 0){
		// Preserve original conditions, shared writes and reflection lifetime
		ShinyTexts_BackendLittleTest();
		if(RenderBuffer_BackendTransform(TempBufferVerticesStored)){
			// Preserve original conditions, shared writes and reflection lifetime
			RenderBuffer_BackendRenderIndexed(TempBufferIndicesStored);
			RenderBuffer_BackendEnd();
		}
		TempBufferVerticesStored = 0;
		TempBufferIndicesStored = 0;
	}
}
//- rouz edit (ChatGPT)
