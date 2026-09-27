//+ rouz edit (ChatGPT)
#pragma once
#include <atomic> // rouz edit (ChatGPT)

namespace rw { struct Atomic; struct Camera; struct Clump; struct RGBA; struct Raster; struct Texture; } // rouz edit (ChatGPT)

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
	unsigned int triviallyUnclippedTriangles, tileDepthRejectedBlocks; // rouz edit (ChatGPT)
	unsigned int offscreenTriangles; // rouz edit (ChatGPT)
	unsigned int nonFiniteTriangles; // rouz edit (ChatGPT)
	float minScreenX, maxScreenX, minScreenY, maxScreenY; // rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	float firstLocal[3], firstWorld[3], firstProjected[3], firstScreen[2];
	float cameraViewWindow[2], cameraViewOffset[2], viewMatrixX[4], viewMatrixY[4];
	//- rouz edit (ChatGPT)
	unsigned int degenerateTriangles; // rouz edit (ChatGPT)
	unsigned int culledTriangles; // rouz edit (ChatGPT)
	unsigned int coveredPixels; // rouz edit (ChatGPT)
	unsigned int simpleOpaqueTriangles, simpleOpaquePixels; // rouz edit (ChatGPT)
	unsigned int texturedTriangles, texturedPixels, cachedTextures; // rouz edit (ChatGPT)
	bool textureUploaded; // rouz edit (ChatGPT)
	unsigned long long totalPipelineCalls; // rouz edit (ChatGPT)
	unsigned long long totalCoveredPixels; // rouz edit (ChatGPT)
	unsigned long long totalUploadedFrames; // rouz edit (ChatGPT)
};
extern Framebuffer framebuffer;
extern int maxFramebufferWidthPixels; // rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
enum TextureSamplingMode {
	TEXTURE_SAMPLING_NEAREST = 0,
	TEXTURE_SAMPLING_BILINEAR = 1,
	TEXTURE_SAMPLING_RENDERWARE = 2,
	TEXTURE_SAMPLING_NEAREST_MIP = 3,
	TEXTURE_SAMPLING_SOLID_COLOR = 4
};
//+ rouz edit (ChatGPT)
extern std::atomic<int> materialTextureSamplingMode;
extern std::atomic<int> worldEffectTextureSamplingMode;
extern std::atomic<int> renderSkyStage;
extern std::atomic<int> renderGeometryStage;
extern std::atomic<int> deferTransparentStage;
extern std::atomic<int> renderWorldEffectsStage;
extern std::atomic<int> renderFogStage;
extern std::atomic<int> renderColourFilterStage;
extern std::atomic<int> renderHudStage;
extern std::atomic<int> renderPedsStage;
extern std::atomic<int> renderVehiclesStage;
extern std::atomic<int> renderWaterStage;
extern std::atomic<int> renderPostEffectsStage;
extern std::atomic<int> renderDynamicLightsStage;
extern std::atomic<int> renderAdvancedMaterialsStage;
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
void Install();
void Shutdown(); // rouz edit (ChatGPT)
void Attach(rw::Atomic *atomic, bool gloss = false, bool rimlight = false, bool vehicle = false); // rouz edit (ChatGPT)
void AttachRimlightClump(rw::Clump *clump); // rouz edit (ChatGPT)
void AttachVehicleClump(rw::Clump *clump); // rouz edit (ChatGPT)
void RenderAtomic(rw::Atomic *atomic, int alpha = 255); // rouz edit (ChatGPT)
void FlushTransparentTriangles(); // rouz edit (ChatGPT)
void RenderClump(rw::Clump *clump); // rouz edit (ChatGPT)
void RenderVehicleClump(rw::Clump *clump); // rouz edit (ChatGPT)
void BeginFrame(rw::Camera *camera, int width, int height, const rw::RGBA &top, const rw::RGBA &bottom); // rouz edit (ChatGPT)
bool FramePending(); // rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
bool BeginOffscreenFrame(rw::Camera *camera, const rw::RGBA &clearColor);
bool BeginOffscreenFrame(rw::Camera *camera, const rw::RGBA &topColor, const rw::RGBA &bottomColor); // rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
bool BeginOffscreenFrame(rw::Camera *camera, rw::Raster *initialRaster);
//- rouz edit (ChatGPT)
void PrimeTexture(rw::Texture *texture);
void ApplyTextureMask(rw::Texture *texture);
bool EndOffscreenFrame(rw::Raster *raster);
//- rouz edit (ChatGPT)
void ApplyColourFilter(int mode, int red, int green, int blue, float intensity); // rouz edit (ChatGPT)
void ApplyPreviousFrameOverlay(int red, int green, int blue, int alpha); // rouz edit (ChatGPT)
void ApplyPreviousFrameBlur(int red, int green, int blue, bool offset); // rouz edit (ChatGPT)
void SavePreviousFrame(bool enabled); // rouz edit (ChatGPT)
void Present();
//+ rouz edit (ChatGPT)
void BeginWorldEffects();
void EndWorldEffects();
bool CapturingWorldEffects();
bool BeginScreenTextureSampling(); // rouz edit (ChatGPT)
void BeginImmediate(const void *vertices, int count, const void *matrix, bool hasUV); // rouz edit (ChatGPT)
void RenderImmediatePrimitive(int primitiveType); // rouz edit (ChatGPT)
void RenderImmediateIndexed(int primitiveType, const unsigned short *indices, int count);
void RenderImmediate2D(int primitiveType, const void *vertices, int vertexCount,
	const unsigned short *indices, int indexCount); // rouz edit (ChatGPT)
void RenderImmediate2DUV2(int primitiveType, const void *vertices, int vertexCount,
	const unsigned short *indices, int indexCount, const float *secondaryUVs,
	int sourceTextureWidth, int sourceTextureHeight); // rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
void RenderScreenRefraction(float left, float top, float right, float bottom,
	int screenWidth, int screenHeight, rw::Raster *maskRaster,
	float leftUOffset, float topVOffset, float rightUOffset, float bottomVOffset, int strength);
void RenderScreenTexture(float left, float top, float right, float bottom,
	int screenWidth, int screenHeight, rw::Raster *raster,
	int red, int green, int blue, int alpha, bool additive);
//- rouz edit (ChatGPT)
void EndImmediate();
//- rouz edit (ChatGPT)
}
//- rouz edit (ChatGPT)
