//+ rouz edit (ChatGPT)
#pragma once

namespace rw { struct Atomic; struct Clump; struct RGBA; } // rouz edit (ChatGPT)

namespace SoftwarePolygons {
//+ rouz edit (ChatGPT)
struct Framebuffer {
	int width, height;
	const rw::RGBA *pixels;
	const float *depths;
	unsigned int pipelineCalls; // rouz edit (ChatGPT)
	unsigned int missingCpuGeometry; // rouz edit (ChatGPT)
	unsigned int submittedAtomics;
	unsigned int submittedTriangles;
	unsigned int depthRejectedTriangles; // rouz edit (ChatGPT)
	unsigned int offscreenTriangles; // rouz edit (ChatGPT)
	unsigned int nonFiniteTriangles; // rouz edit (ChatGPT)
	float minScreenX, maxScreenX, minScreenY, maxScreenY; // rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	float firstLocal[3], firstWorld[3], firstProjected[3], firstScreen[2];
	float cameraViewWindow[2], cameraViewOffset[2], viewMatrixX[4], viewMatrixY[4];
	//- rouz edit (ChatGPT)
	unsigned int degenerateTriangles; // rouz edit (ChatGPT)
	unsigned int coveredPixels; // rouz edit (ChatGPT)
	unsigned int texturedTriangles, texturedPixels, cachedTextures; // rouz edit (ChatGPT)
	bool textureUploaded; // rouz edit (ChatGPT)
	unsigned long long totalPipelineCalls; // rouz edit (ChatGPT)
	unsigned long long totalCoveredPixels; // rouz edit (ChatGPT)
	unsigned long long totalUploadedFrames; // rouz edit (ChatGPT)
};
extern Framebuffer framebuffer;
//- rouz edit (ChatGPT)
void Install();
void Shutdown(); // rouz edit (ChatGPT)
void Attach(rw::Atomic *atomic);
void RenderAtomic(rw::Atomic *atomic); // rouz edit (ChatGPT)
void RenderClump(rw::Clump *clump); // rouz edit (ChatGPT)
void BeginFrame(int width, int height, const rw::RGBA &top, const rw::RGBA &bottom); // rouz edit (ChatGPT)
void Present();
}
//- rouz edit (ChatGPT)
