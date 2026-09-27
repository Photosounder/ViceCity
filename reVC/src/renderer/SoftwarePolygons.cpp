//+ rouz edit (ChatGPT)
#if defined(REVC_SOFTWARE_POLYGONS) && defined(RW_GL3)
#include "rwcore.h"
#include "rpworld.h"
#include "SoftwarePolygons.h"
#include "Lights.h"
#ifdef DEBUGMENU
#include "debugmenu.h"
#endif
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
#include "custompipes.h"
#endif

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

extern "C" void *cita_win_malloc(size_t, const char *, const char *, int);
extern "C" void cita_win_free(void *, const char *, const char *, int);
#define SOFTWARE_CIT_MALLOC(size) cita_win_malloc((size), __FILE__, __func__, __LINE__)
#define SOFTWARE_CIT_FREE(ptr) cita_win_free((ptr), __FILE__, __func__, __LINE__)

namespace SoftwarePolygons {
Framebuffer framebuffer = {};
int maxFramebufferWidthPixels = 860;
std::atomic<int> materialTextureSamplingMode(TEXTURE_SAMPLING_NEAREST_MIP);
std::atomic<int> worldEffectTextureSamplingMode(TEXTURE_SAMPLING_NEAREST_MIP);
std::atomic<int> renderSkyStage(1);
std::atomic<int> renderGeometryStage(1);
std::atomic<int> deferTransparentStage(1);
std::atomic<int> renderWorldEffectsStage(1);
std::atomic<int> renderFogStage(1);
std::atomic<int> renderColourFilterStage(1);
std::atomic<int> renderHudStage(1);
std::atomic<int> renderPedsStage(1);
std::atomic<int> renderVehiclesStage(1);
std::atomic<int> renderWaterStage(1);
std::atomic<int> renderPostEffectsStage(1);
std::atomic<int> renderDynamicLightsStage(1);
std::atomic<int> renderAdvancedMaterialsStage(1);
#ifdef DEBUGMENU
SETTWEAKPATH("Software Renderer");
TWEAKINT32N(maxFramebufferWidthPixels, 320, 1920, 40, "Max framebuffer width (pixels)");
#endif
namespace {

struct ScreenVertex {
	float x, y, z;
	float cameraX, cameraY;
	float u, v;
	//+ rouz edit (ChatGPT)
	float u2, v2;
	float dualU, dualV;
	float normalX, normalY, normalZ;
	float glossLightZ;
	float rimRed, rimGreen, rimBlue;
	float vehicleEnvU, vehicleEnvV, vehicleReflectBase;
	float vehicleSpecRed, vehicleSpecGreen, vehicleSpecBlue;
	//- rouz edit (ChatGPT)
	float lightRed, lightGreen, lightBlue;
	rw::RGBA color;
};

struct CachedMipLevel {
	rw::uint8 *pixels;
	int width, height, stride;
};

struct CachedTexture {
	rw::Raster *raster;
	rw::Image *image;
	CachedMipLevel *mipLevels;
	int mipLevelCount;
	size_t cpuBytes;
	int width, height, format;
	rw::uint32 filter;
	char name[32], mask[32];
	bool topDown;
	bool opaque;
	unsigned long long lastUsed;
};

struct TextureSampler {
	const rw::uint8 *pixels;
	int width, height, stride;
	const CachedMipLevel *mipLevels;
	int mipLevelCount;
	float lod;
	bool topDown, linear;
	bool mipmapped, mipLinear;
	bool solidColor;
	int wrap;
	rw::Texture::Addressing addressU, addressV;
};

struct EnvironmentState {
	rw::Texture *source;
	const CachedTexture *cached;
	rw::Matrix normalMatrix;
	rw::RGBA color;
	float coefficient;
	rw::Texture *bumpSource;
	const CachedTexture *bumpCached;
	float bumpCoefficient;
	bool framebufferAlpha, applyLight;
};

struct DualTextureState {
	rw::Texture *source;
	const CachedTexture *cached;
	float coefficient;
	rw::uint32 sourceBlend, destinationBlend;
	bool lightmap, usesSecondaryUV;
};

struct GlossState {
	rw::Texture *source;
	const CachedTexture *cached;
	float multiplier;
};

struct VehicleState {
	rw::Texture *environmentSource;
	const CachedTexture *environmentCached;
	float fresnel, shininess, specularity, lightStrength;
	bool enabled;
};

struct MaterialState {
	rw::RGBA color;
	rw::RGBA blendColor;
	float surfaceAmbient, surfaceDiffuse;
	rw::Texture *source;
	const CachedTexture *cached;
	EnvironmentState environment;
	DualTextureState dualTexture;
	GlossState gloss;
	VehicleState vehicleState;
	rw::Matrix *baseUVTransform;
	rw::Matrix *dualUVTransform;
	bool resolved;
};

int width, height;
rw::RGBA *pixels;
float *depths;
rw::uint8 *stencils;
float *tileDepthBounds;
int tilesAcross;
rw::RGBA *previousFramePixels;
int previousFrameWidth, previousFrameHeight;
bool previousFrameValid;
rw::RGBA *screenTexturePixels;
int screenTextureWidth, screenTextureHeight;
bool screenTextureValid;
float atomicAlphaMultiplier = 1.0f;
bool deferTransparentTriangles;
rw::Raster *texture;
rw::ObjPipeline *pipeline;
rw::ObjPipeline *originalPipeline;

std::unordered_map<rw::Texture*, CachedTexture> textureCache;
std::unordered_set<rw::Atomic*> glossAtomics;
std::unordered_set<rw::Atomic*> rimlightAtomics;
std::unordered_set<rw::Atomic*> vehicleAtomics;
std::unordered_map<rw::Raster*, CachedTexture> effectTextureCache;
bool capturingWorldEffects;
bool framePending;
bool offscreenFrameActive;
rw::Camera *mainFrameCamera;
const RwIm3DVertex *immediateVertices;
int immediateVertexCount;
rw::Matrix immediateMatrix;
bool immediateHasMatrix;
bool immediateHasUV;
unsigned long long cacheFrame;
size_t textureCacheBytes;
const size_t textureCacheLimit = 128u*1024u*1024u;

rw::Image *readTextureImage(rw::Raster *raster, bool &topDown)
{
	// Decode DXT textures from their native level before sampling on the CPU
	rw::gl3::Gl3Raster *native = (rw::gl3::Gl3Raster*)((rw::uint8*)raster + rw::gl3::nativeRasterOffset);
	if(native->isCompressed){
		int dxt = 0;
		switch(native->internalFormat){
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: dxt = 1; break;
		// Decode premultiplied S3TC variants with their identical block layouts
		//+ rouz edit (ChatGPT)
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: dxt = 3; break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: dxt = 5; break;
		//- rouz edit (ChatGPT)
		default: return nullptr;
		}
		rw::uint8 *compressed = raster->lock(0, rw::Raster::LOCKREAD);
		if(!compressed)
			return nullptr;
		// Decode complete 4 by 4 blocks, then crop any padded edge texels
		//+ rouz edit (ChatGPT)
		const int blockWidth = (raster->width+3)&~3;
		const int blockHeight = (raster->height+3)&~3;
		rw::Image *decoded = rw::Image::create(blockWidth, blockHeight, 32);
		rw::Image *image = nullptr;
		if(decoded){
			decoded->allocate();
			if(decoded->pixels){
				decoded->setPixelsDXT(dxt, compressed);
				if(blockWidth == raster->width && blockHeight == raster->height)
					image = decoded;
				else{
					image = rw::Image::create(raster->width, raster->height, 32);
					if(image){
						image->allocate();
						if(image->pixels)
							for(int y = 0; y < raster->height; y++)
								std::memcpy(image->pixels+(size_t)y*image->stride,
									decoded->pixels+(size_t)y*decoded->stride, (size_t)raster->width*4);
						else{
							image->destroy();
							image = nullptr;
						}
					}
					decoded->destroy();
				}
				// Preserve opaque RGB DXT1 texels after decoding
				if(image && native->internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
					for(int y = 0; y < image->height; y++)
						for(int x = 0; x < image->width; x++)
							image->pixels[(size_t)y*image->stride + (size_t)x*4 + 3] = 255;
			}else{
				decoded->destroy();
			}
		}
		//- rouz edit (ChatGPT)
		raster->unlock(0);
		topDown = false;
		return image;
	}
	// Convert any readable ordinary GL texture format to RGBA rows with a top image origin
	//+ rouz edit (ChatGPT)
	rw::Image *image = raster->toImage();
	if(image && image->depth != 32)
		image->convertTo32();
	//- rouz edit (ChatGPT)
	topDown = true;
	return image;
}

static void buildCachedMipmaps(CachedTexture *entry, bool force = false)
{
	// Build a compact alpha-aware mip chain for one decoded material texture
	if(!entry || !entry->image || (entry->width <= 1 && entry->height <= 1) ||
	   (!force && entry->filter != rw::Texture::MIPNEAREST && entry->filter != rw::Texture::MIPLINEAR &&
	    entry->filter != rw::Texture::LINEARMIPNEAREST && entry->filter != rw::Texture::LINEARMIPLINEAR))
		return;
	// Count the levels down to a one-pixel image
	int levelCount = 0;
	int levelWidth = entry->width, levelHeight = entry->height;
	while(levelWidth > 1 || levelHeight > 1){
		levelWidth = std::max(1, levelWidth/2);
		levelHeight = std::max(1, levelHeight/2);
		levelCount++;
	}
	// Allocate a level table that remains stable for the lifetime of the cache entry
	CachedMipLevel *levels = (CachedMipLevel*)SOFTWARE_CIT_MALLOC((size_t)levelCount*sizeof(CachedMipLevel));
	if(!levels)
		return;
	std::memset(levels, 0, (size_t)levelCount*sizeof(CachedMipLevel));
	const rw::uint8 *sourcePixels = entry->image->pixels;
	int sourceWidth = entry->image->width, sourceHeight = entry->image->height;
	int sourceStride = entry->image->stride;
	int builtLevels = 0;
	// Downsample every source footprint with premultiplied-alpha color averaging
	for(int level = 0; level < levelCount; level++){
		const int width = std::max(1, sourceWidth/2);
		const int height = std::max(1, sourceHeight/2);
		const int stride = width*4;
		rw::uint8 *pixels = (rw::uint8*)SOFTWARE_CIT_MALLOC((size_t)stride*height);
		if(!pixels)
			break;
		// Average all source texels covered by each destination texel
		for(int y = 0; y < height; y++){
			const int sourceY0 = y*sourceHeight/height;
			const int sourceY1 = std::max(sourceY0+1, (y+1)*sourceHeight/height);
			for(int x = 0; x < width; x++){
				const int sourceX0 = x*sourceWidth/width;
				const int sourceX1 = std::max(sourceX0+1, (x+1)*sourceWidth/width);
				unsigned int alphaSum = 0, redSum = 0, greenSum = 0, blueSum = 0, sampleCount = 0;
				for(int sourceY = sourceY0; sourceY < sourceY1; sourceY++)
					for(int sourceX = sourceX0; sourceX < sourceX1; sourceX++){
						const rw::uint8 *sample = sourcePixels+(size_t)sourceY*sourceStride+(size_t)sourceX*4;
						alphaSum += sample[3];
						redSum += sample[0]*sample[3];
						greenSum += sample[1]*sample[3];
						blueSum += sample[2]*sample[3];
						sampleCount++;
					}
				rw::uint8 *destination = pixels+(size_t)y*stride+(size_t)x*4;
				destination[3] = (rw::uint8)((alphaSum+sampleCount/2)/sampleCount);
				destination[0] = alphaSum ? (rw::uint8)((redSum+alphaSum/2)/alphaSum) : 0;
				destination[1] = alphaSum ? (rw::uint8)((greenSum+alphaSum/2)/alphaSum) : 0;
				destination[2] = alphaSum ? (rw::uint8)((blueSum+alphaSum/2)/alphaSum) : 0;
			}
		}
		levels[builtLevels++] = { pixels, width, height, stride };
		entry->cpuBytes += (size_t)stride*height;
		sourcePixels = pixels;
		sourceWidth = width;
		sourceHeight = height;
		sourceStride = stride;
	}
	// Retain completed levels and discard an unused table after an allocation failure
	if(builtLevels == 0)
		SOFTWARE_CIT_FREE(levels);
	else{
		entry->mipLevels = levels;
		entry->mipLevelCount = builtLevels;
		entry->cpuBytes += (size_t)levelCount*sizeof(CachedMipLevel);
	}
}

static void destroyCachedTexture(CachedTexture *entry)
{
	// Release the decoded base image and every generated mip allocation
	if(!entry)
		return;
	// Destroy the RenderWare-owned decoded image
	if(entry->image){
		entry->image->destroy();
		entry->image = nullptr;
	}
	// Free each raw CPU mip allocation
	for(int level = 0; level < entry->mipLevelCount; level++)
		if(entry->mipLevels[level].pixels)
			SOFTWARE_CIT_FREE(entry->mipLevels[level].pixels);
	// Release the level table and clear the cached byte count
	if(entry->mipLevels)
		SOFTWARE_CIT_FREE(entry->mipLevels);
	entry->mipLevels = nullptr;
	entry->mipLevelCount = 0;
	entry->cpuBytes = 0;
}

static bool refreshCachedTexturePixels(CachedTexture *entry, bool accountForTextureCache)
{
	// Read the latest GPU contents for a RenderWare camera texture
	if(!entry || !entry->raster)
		return false;
	bool topDown = true;
	rw::Image *image = readTextureImage(entry->raster, topDown);
	if(!image)
		return false;
	CachedTexture decoded = {};
	decoded.width = entry->width;
	decoded.height = entry->height;
	decoded.filter = entry->filter;
	decoded.image = image;
	decoded.topDown = topDown;
	decoded.opaque = true;
	// Record alpha coverage before replacing the previous frame's texture copy
	for(int y = 0; y < image->height && decoded.opaque; y++)
		for(int x = 0; x < image->width; x++)
			if(image->pixels[(size_t)y*image->stride+(size_t)x*4+3] != 255){
				decoded.opaque = false;
				break;
			}
	decoded.cpuBytes = (size_t)image->stride*image->height;
	buildCachedMipmaps(&decoded, materialTextureSamplingMode == TEXTURE_SAMPLING_NEAREST_MIP ||
		worldEffectTextureSamplingMode == TEXTURE_SAMPLING_NEAREST_MIP);
	const size_t previousBytes = entry->cpuBytes;
	if(accountForTextureCache)
		textureCacheBytes -= previousBytes;
	destroyCachedTexture(entry);
	entry->image = decoded.image;
	entry->mipLevels = decoded.mipLevels;
	entry->mipLevelCount = decoded.mipLevelCount;
	entry->cpuBytes = decoded.cpuBytes;
	entry->topDown = decoded.topDown;
	entry->opaque = decoded.opaque;
	if(accountForTextureCache)
		textureCacheBytes += entry->cpuBytes;
	return true;
}

void trimTextureCache()
{
	// Release streamed textures that have not been used recently
	for(auto it = textureCache.begin(); it != textureCache.end(); ){
		if(cacheFrame - it->second.lastUsed > 120){
			//+ rouz edit (ChatGPT)
			// Account for and release the complete decoded texture chain
			textureCacheBytes -= it->second.cpuBytes;
			destroyCachedTexture(&it->second);
			//- rouz edit (ChatGPT)
			it = textureCache.erase(it);
		}else
			++it;
	}
	// Bound retained texture copies while preserving the most recently used images
	while(textureCacheBytes > textureCacheLimit && textureCache.size() > 1){
		auto oldest = std::min_element(textureCache.begin(), textureCache.end(),
			[](const auto &a, const auto &b){ return a.second.lastUsed < b.second.lastUsed; });
		//+ rouz edit (ChatGPT)
		// Account for and release the complete decoded texture chain
		textureCacheBytes -= oldest->second.cpuBytes;
		destroyCachedTexture(&oldest->second);
		//- rouz edit (ChatGPT)
		textureCache.erase(oldest);
	}
}

const CachedTexture *getCachedTexture(rw::Texture *source)
{
	// Reuse decoded pixels only while the material still refers to the same raster
	if(!source || !source->raster || source->raster->platform != rw::PLATFORM_GL3)
		return nullptr;
	rw::Raster *raster = source->raster;
	//+ rouz edit (ChatGPT)
	// Accept ordinary textures and dynamically rendered camera textures
	if((raster->type != rw::Raster::TEXTURE && raster->type != rw::Raster::CAMERATEXTURE) ||
	   raster->width <= 0 || raster->height <= 0)
		return nullptr;
	//- rouz edit (ChatGPT)
	auto found = textureCache.find(source);
	if(found != textureCache.end()){
		CachedTexture &entry = found->second;
		//+ rouz edit (ChatGPT)
		// Reuse the decoded image only while its sampling filter and source data match
		if(entry.raster == raster && entry.width == raster->width && entry.height == raster->height &&
		   entry.format == raster->format && entry.filter == source->getFilter() &&
		   std::memcmp(entry.name, source->name, 32) == 0 &&
		   std::memcmp(entry.mask, source->mask, 32) == 0){
			//+ rouz edit (ChatGPT)
			// Generate missing levels when nearest mip sampling is selected after the texture was cached
			if(materialTextureSamplingMode == TEXTURE_SAMPLING_NEAREST_MIP && entry.image && !entry.mipLevelCount){
				const size_t oldBytes = entry.cpuBytes;
				buildCachedMipmaps(&entry, true);
				textureCacheBytes += entry.cpuBytes-oldBytes;
			}
			//- rouz edit (ChatGPT)
			//+ rouz edit (ChatGPT)
			// Refresh live camera textures after a frame or auxiliary camera pass
			const bool isRenderTarget = rw::engine && rw::engine->currentCamera &&
				rw::engine->currentCamera->frameBuffer == raster;
			if(raster->type == rw::Raster::CAMERATEXTURE && !isRenderTarget &&
			   (entry.lastUsed != cacheFrame || offscreenFrameActive))
				refreshCachedTexturePixels(&entry, true);
			//- rouz edit (ChatGPT)
			entry.lastUsed = cacheFrame;
			return entry.image ? &entry : nullptr;
		}
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Remove stale decoded pixels before rebuilding the cache entry
		textureCacheBytes -= entry.cpuBytes;
		destroyCachedTexture(&entry);
		textureCache.erase(found);
		//- rouz edit (ChatGPT)
	}
	// Read and cache the first CPU copy of this static material texture
	CachedTexture entry = {};
	entry.raster = raster;
	entry.width = raster->width;
	entry.height = raster->height;
	entry.format = raster->format;
	entry.filter = source->getFilter();
	std::memcpy(entry.name, source->name, 32);
	std::memcpy(entry.mask, source->mask, 32);
	entry.lastUsed = cacheFrame;
	entry.image = readTextureImage(raster, entry.topDown);
	// Record whether sampling this texture can ever discard a pixel
	//+ rouz edit (ChatGPT)
	entry.opaque = entry.image != nullptr;
	if(entry.image){
		for(int y = 0; y < entry.image->height && entry.opaque; y++)
			for(int x = 0; x < entry.image->width; x++)
				if(entry.image->pixels[(size_t)y*entry.image->stride+(size_t)x*4+3] != 255){
					entry.opaque = false;
					break;
				}
		entry.cpuBytes = (size_t)entry.image->stride*entry.image->height;
		buildCachedMipmaps(&entry, materialTextureSamplingMode == TEXTURE_SAMPLING_NEAREST_MIP);
		textureCacheBytes += entry.cpuBytes;
	}
	//- rouz edit (ChatGPT)
	auto inserted = textureCache.emplace(source, entry);
	// Keep material pointers valid until the next frame's cache trim
	//+ rouz edit (ChatGPT)
	(void)inserted;
	found = textureCache.find(source);
	return found != textureCache.end() && found->second.image ? &found->second : nullptr;
	//- rouz edit (ChatGPT)
}

const CachedTexture *getCachedEffectTexture(rw::Raster *raster)
{
	// Cache the raster selected by immediate mode effects without requiring a Texture object
	//+ rouz edit (ChatGPT)
	// Accept ordinary effect rasters and dynamically rendered camera textures
	if(!raster || raster->platform != rw::PLATFORM_GL3 ||
	   (raster->type != rw::Raster::TEXTURE && raster->type != rw::Raster::CAMERATEXTURE))
		return nullptr;
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Read the active sampler filter before lazily building effect mip levels
	const rw::uint32 filter = rw::GetRenderState(rw::TEXTUREFILTER);
	//- rouz edit (ChatGPT)
	auto found = effectTextureCache.find(raster);
	if(found != effectTextureCache.end()){
		if(found->second.width == raster->width && found->second.height == raster->height &&
		   found->second.format == raster->format){
			//+ rouz edit (ChatGPT)
			// Refresh effect camera textures after a frame or auxiliary camera pass
			const bool isRenderTarget = rw::engine && rw::engine->currentCamera &&
				rw::engine->currentCamera->frameBuffer == raster;
			if(raster->type == rw::Raster::CAMERATEXTURE && !isRenderTarget &&
			   (found->second.lastUsed != cacheFrame || offscreenFrameActive))
				refreshCachedTexturePixels(&found->second, false);
			//- rouz edit (ChatGPT)
			//+ rouz edit (ChatGPT)
			// Build mip levels once when an effect first requests a mipmapped filter
			if(found->second.image && found->second.mipLevelCount == 0 &&
			   (worldEffectTextureSamplingMode == TEXTURE_SAMPLING_NEAREST_MIP ||
			    filter == rw::Texture::MIPNEAREST || filter == rw::Texture::MIPLINEAR ||
			    filter == rw::Texture::LINEARMIPNEAREST || filter == rw::Texture::LINEARMIPLINEAR)){
				found->second.filter = filter;
				buildCachedMipmaps(&found->second, worldEffectTextureSamplingMode == TEXTURE_SAMPLING_NEAREST_MIP);
			}
			//- rouz edit (ChatGPT)
			found->second.lastUsed = cacheFrame;
			return found->second.image ? &found->second : nullptr;
		}
		destroyCachedTexture(&found->second);
		effectTextureCache.erase(found);
	}
	CachedTexture entry = {};
	entry.raster = raster;
	entry.width = raster->width;
	entry.height = raster->height;
	entry.format = raster->format;
	//+ rouz edit (ChatGPT)
	// Rebuild effect mip levels whenever the active RenderWare filter changes
	entry.filter = filter;
	//- rouz edit (ChatGPT)
	entry.lastUsed = cacheFrame;
	entry.image = readTextureImage(raster, entry.topDown);
	//+ rouz edit (ChatGPT)
	// Generate the mip levels requested by immediate effect texture filtering
	if(entry.image){
		entry.cpuBytes = (size_t)entry.image->stride*entry.image->height;
		buildCachedMipmaps(&entry, worldEffectTextureSamplingMode == TEXTURE_SAMPLING_NEAREST_MIP);
	}
	//- rouz edit (ChatGPT)
	auto inserted = effectTextureCache.emplace(raster, entry);
	return inserted.first->second.image ? &inserted.first->second : nullptr;
}

float addressCoordinate(float value, rw::Texture::Addressing mode)
{
	// Map repeating and mirrored UVs into one texture period
	if(mode == rw::Texture::CLAMP)
		return std::max(0.0f, std::min(1.0f, value));
	if(mode == rw::Texture::BORDER && (value < 0.0f || value > 1.0f))
		return -1.0f;
	if(mode == rw::Texture::MIRROR){
		float period = std::fmod(value, 2.0f);
		if(period < 0.0f) period += 2.0f;
		return period <= 1.0f ? period : 2.0f-period;
	}
	return value - std::floor(value);
}

// Match the magnification filter selected by each RenderWare texture
bool filterUsesLinear(rw::uint32 filter)
{
	// Select linear filtering for modes whose magnification filter is linear
	return filter == rw::Texture::LINEAR || filter == rw::Texture::MIPLINEAR ||
		filter == rw::Texture::LINEARMIPLINEAR;
}

bool filterUsesMipmaps(rw::uint32 filter)
{
	// Enable minification levels only for RenderWare filters that request them
	return filter == rw::Texture::MIPNEAREST || filter == rw::Texture::MIPLINEAR ||
		filter == rw::Texture::LINEARMIPNEAREST || filter == rw::Texture::LINEARMIPLINEAR;
}

bool filterBlendsMipLevels(rw::uint32 filter)
{
	// Interpolate between mip levels for the two linear-mipmap modes
	return filter == rw::Texture::LINEARMIPNEAREST || filter == rw::Texture::LINEARMIPLINEAR;
}

static void applyTextureSamplingMode(TextureSampler *sampler, int mode)
{
	// Override the source filter once per triangle while preserving its address modes
	if(mode == TEXTURE_SAMPLING_NEAREST || mode == TEXTURE_SAMPLING_BILINEAR){
		sampler->linear = mode == TEXTURE_SAMPLING_BILINEAR;
		sampler->mipmapped = false;
		sampler->mipLinear = false;
	}
	//+ rouz edit (ChatGPT)
	// Use one point-sampled texel from the nearest mip level in the new middle mode
	if(mode == TEXTURE_SAMPLING_NEAREST_MIP){
		sampler->linear = false;
		sampler->mipmapped = true;
		sampler->mipLinear = false;
	}
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Skip texture lookup and mip selection for diagnostic solid-color shading
	if(mode == TEXTURE_SAMPLING_SOLID_COLOR){
		sampler->solidColor = true;
		sampler->mipmapped = false;
	}
	//- rouz edit (ChatGPT)
}

static float estimateImmediateTextureLod(const ScreenVertex &a, const ScreenVertex &b,
	const ScreenVertex &c, const TextureSampler *sampler)
{
	// Estimate perspective-correct texture minification at an immediate triangle center
	if(!sampler || !sampler->mipmapped || sampler->mipLevelCount <= 0 ||
	   sampler->width <= 0 || sampler->height <= 0 || a.z <= 0.0f || b.z <= 0.0f || c.z <= 0.0f)
		return 0.0f;
	const float area = (c.x-a.x)*(b.y-a.y)-(c.y-a.y)*(b.x-a.x);
	if(!std::isfinite(area) || std::fabs(area) < 0.001f)
		return 0.0f;
	const float inverseArea = 1.0f/area;
	const float w0dx = (c.y-b.y)*inverseArea;
	const float w0dy = -(c.x-b.x)*inverseArea;
	const float w1dx = (a.y-c.y)*inverseArea;
	const float w1dy = -(a.x-c.x)*inverseArea;
	const float q0 = 1.0f/a.z, q1 = 1.0f/b.z, q2 = 1.0f/c.z;
	const float u0 = a.u*q0, u1 = b.u*q1, u2 = c.u*q2;
	const float v0 = a.v*q0, v1 = b.v*q1, v2 = c.v*q2;
	const float inverseDepth = (q0+q1+q2)/3.0f;
	const float uOverDepth = (u0+u1+u2)/3.0f;
	const float vOverDepth = (v0+v1+v2)/3.0f;
	const float inverseDepthX = (q0-q2)*w0dx+(q1-q2)*w1dx;
	const float inverseDepthY = (q0-q2)*w0dy+(q1-q2)*w1dy;
	const float uOverDepthX = (u0-u2)*w0dx+(u1-u2)*w1dx;
	const float uOverDepthY = (u0-u2)*w0dy+(u1-u2)*w1dy;
	const float vOverDepthX = (v0-v2)*w0dx+(v1-v2)*w1dx;
	const float vOverDepthY = (v0-v2)*w0dy+(v1-v2)*w1dy;
	const float depthSquared = inverseDepth*inverseDepth;
	if(!(depthSquared > 0.0f) || !std::isfinite(depthSquared))
		return 0.0f;
	const float duDx = (uOverDepthX*inverseDepth-uOverDepth*inverseDepthX)/depthSquared;
	const float dvDx = (vOverDepthX*inverseDepth-vOverDepth*inverseDepthX)/depthSquared;
	const float duDy = (uOverDepthY*inverseDepth-uOverDepth*inverseDepthY)/depthSquared;
	const float dvDy = (vOverDepthY*inverseDepth-vOverDepth*inverseDepthY)/depthSquared;
	const float footprintX = std::sqrt(duDx*duDx*sampler->width*sampler->width+
		dvDx*dvDx*sampler->height*sampler->height);
	const float footprintY = std::sqrt(duDy*duDy*sampler->width*sampler->width+
		dvDy*dvDy*sampler->height*sampler->height);
	// Keep nearest mipmaps sharper on steeply angled surfaces such as roads
	//+ rouz edit (ChatGPT)
	const float footprint = !sampler->linear && !sampler->mipLinear
		? std::sqrt(std::max(1.0f, footprintX)*std::max(1.0f, footprintY))
		: std::max(footprintX, footprintY);
	//- rouz edit (ChatGPT)
	if(!(footprint > 1.0f) || !std::isfinite(footprint))
		return 0.0f;
	return std::max(0.0f, std::min((float)sampler->mipLevelCount, std::log2(footprint)));
}

static float estimateImmediateLineTextureLod(const ScreenVertex &a, const ScreenVertex &b,
	const TextureSampler *sampler, float screenLength)
{
	// Estimate a line texture's perspective-correct footprint at its midpoint
	if(!sampler || !sampler->mipmapped || sampler->mipLevelCount <= 0 ||
	   sampler->width <= 0 || sampler->height <= 0 || !(screenLength > 0.0f) ||
	   a.z <= 0.0f || b.z <= 0.0f)
		return 0.0f;
	const float inverseA = 1.0f/a.z, inverseB = 1.0f/b.z;
	const float inverseMid = 0.5f*(inverseA+inverseB);
	const float inverseMidSquared = inverseMid*inverseMid;
	if(!(inverseMidSquared > 0.0f) || !std::isfinite(inverseMidSquared))
		return 0.0f;
	const float uOverDepthA = a.u*inverseA, uOverDepthB = b.u*inverseB;
	const float vOverDepthA = a.v*inverseA, vOverDepthB = b.v*inverseB;
	const float uOverDepthMid = 0.5f*(uOverDepthA+uOverDepthB);
	const float vOverDepthMid = 0.5f*(vOverDepthA+vOverDepthB);
	const float duPerPixel = ((uOverDepthB-uOverDepthA)*inverseMid-
		uOverDepthMid*(inverseB-inverseA))/inverseMidSquared/screenLength;
	const float dvPerPixel = ((vOverDepthB-vOverDepthA)*inverseMid-
		vOverDepthMid*(inverseB-inverseA))/inverseMidSquared/screenLength;
	const float footprint = std::sqrt(duPerPixel*duPerPixel*sampler->width*sampler->width+
		dvPerPixel*dvPerPixel*sampler->height*sampler->height);
	if(!(footprint > 1.0f) || !std::isfinite(footprint))
		return 0.0f;
	return std::max(0.0f, std::min((float)sampler->mipLevelCount, std::log2(footprint)));
}

static float estimateScreenTextureLod(const TextureSampler &sampler, float pixelWidth, float pixelHeight)
{
	// Estimate texture minification for a rectangle mapped across framebuffer pixels
	if(!sampler.mipmapped || sampler.mipLevelCount <= 0 ||
	   !(pixelWidth > 0.0f) || !(pixelHeight > 0.0f))
		return 0.0f;
	const float footprint = std::max(sampler.width/pixelWidth, sampler.height/pixelHeight);
	if(!(footprint > 1.0f) || !std::isfinite(footprint))
		return 0.0f;
	return std::max(0.0f, std::min((float)sampler.mipLevelCount, std::log2(footprint)));
}

bool sampleTextureLinear(const TextureSampler &sampler, float u, float v, rw::RGBA &texel);

static bool sampleTextureBase(const TextureSampler &sampler, float u, float v, rw::RGBA &texel)
{
	// Sample using the selected filter after applying RenderWare address modes
	if(!std::isfinite(u) || !std::isfinite(v))
		return false;
	// Use the same point or linear filter selected by the RenderWare texture
	//+ rouz edit (ChatGPT)
	if(sampler.linear)
		return sampleTextureLinear(sampler, u, v, texel);
	//- rouz edit (ChatGPT)
	v = 1.0f-v;
	if(sampler.wrap){
		// The common repeat mode needs no clamp, mirror or border handling.
		u -= std::floor(u);
		v -= std::floor(v);
	}else{
		u = addressCoordinate(u, sampler.addressU);
		v = addressCoordinate(v, sampler.addressV);
		if(u < 0.0f || v < 0.0f)
			return false;
	}
	const int x = std::min(sampler.width-1, (int)(u*sampler.width));
	int y = std::min(sampler.height-1, (int)(v*sampler.height));
	// Convert the GL sampler's bottom origin to ordinary CPU image rows
	if(sampler.topDown)
		y = sampler.height-1-y;
	const rw::uint8 *pixel = sampler.pixels + (size_t)y*sampler.stride + (size_t)x*4;
	texel = { pixel[0], pixel[1], pixel[2], pixel[3] };
	return true;
}

static bool sampleTextureLevel(const TextureSampler &sampler, int level,
	float u, float v, rw::RGBA &texel)
{
	// Select one cached mip level before applying the existing point or bilinear sampler
	TextureSampler selected = sampler;
	if(level > 0){
		const CachedMipLevel &mip = sampler.mipLevels[level-1];
		selected.pixels = mip.pixels;
		selected.width = mip.width;
		selected.height = mip.height;
		selected.stride = mip.stride;
	}
	selected.mipLevels = nullptr;
	selected.mipLevelCount = 0;
	selected.mipmapped = false;
	return sampleTextureBase(selected, u, v, texel);
}

static bool sampleTexture(const TextureSampler &sampler, float u, float v, rw::RGBA &texel)
{
	//+ rouz edit (ChatGPT)
	// Return opaque middle grey so untextured materials remain distinguishable without brightening the scene
	if(sampler.solidColor){
		texel = { 128, 128, 128, 255 };
		return true;
	}
	//- rouz edit (ChatGPT)
	// Blend neighboring mip levels when the texture footprint is smaller than one screen pixel
	if(!sampler.mipmapped || sampler.mipLevelCount <= 0 || !std::isfinite(sampler.lod) || sampler.lod <= 0.0f)
		return sampleTextureBase(sampler, u, v, texel);
	const float lod = std::max(0.0f, std::min((float)sampler.mipLevelCount, sampler.lod));
	if(!sampler.mipLinear){
		const int level = std::min(sampler.mipLevelCount, (int)std::floor(lod+0.5f));
		return sampleTextureLevel(sampler, level, u, v, texel);
	}
	const int lowerLevel = (int)std::floor(lod);
	const int upperLevel = std::min(sampler.mipLevelCount, lowerLevel+1);
	const float levelWeight = lod-lowerLevel;
	//+ rouz edit (ChatGPT)
	// Sample one mip level when its selected weight is exact
	if(levelWeight <= 0.0f || upperLevel == lowerLevel)
		return sampleTextureLevel(sampler, lowerLevel, u, v, texel);
	//- rouz edit (ChatGPT)
	rw::RGBA lower = {}, upper = {};
	const bool haveLower = sampleTextureLevel(sampler, lowerLevel, u, v, lower);
	const bool haveUpper = upperLevel != lowerLevel &&
		sampleTextureLevel(sampler, upperLevel, u, v, upper);
	if(!haveLower && !haveUpper)
		return false;
	if(!haveLower){
		texel = upper;
		return true;
	}
	if(!haveUpper || upperLevel == lowerLevel){
		texel = lower;
		return true;
	}
	// Interpolate all channels between the selected mip samples
	const rw::uint8 low[4] = { lower.red, lower.green, lower.blue, lower.alpha };
	const rw::uint8 high[4] = { upper.red, upper.green, upper.blue, upper.alpha };
	rw::uint8 result[4];
	// Keep straight-alpha colors clean while mixing their mip levels
	const float lowAlpha = low[3]*(1.f/255.f), highAlpha = high[3]*(1.f/255.f);
	const float mixedAlpha = lowAlpha*(1.0f-levelWeight)+highAlpha*levelWeight;
	for(int channel = 0; channel < 3; channel++){
		const float mixedColor = mixedAlpha > 0.0f
			? (low[channel]*lowAlpha*(1.0f-levelWeight)+high[channel]*highAlpha*levelWeight)/mixedAlpha : 0.0f;
		result[channel] = (rw::uint8)(mixedColor+0.5f);
	}
	result[3] = (rw::uint8)(mixedAlpha*255.0f+0.5f);
	texel = { result[0], result[1], result[2], result[3] };
	return true;
}

static void transformTextureCoordinate(const rw::Matrix *matrix, float *u, float *v)
{
	// Apply a RenderWare affine transform to one texture coordinate pair
	if(!matrix || !u || !v)
		return;
	rw::V3d source = { *u, *v, 0.0f };
	rw::V3d transformed;
	rw::V3d::transformPoints(&transformed, &source, 1, matrix);
	*u = transformed.x;
	*v = transformed.y;
}

bool sampleTextureLinear(const TextureSampler &sampler, float u, float v, rw::RGBA &texel)
{
	// Bilinearly sample a texture with its configured address modes
	if(!std::isfinite(u) || !std::isfinite(v) || sampler.width <= 0 || sampler.height <= 0)
		return false;
	const bool wrapU = sampler.addressU == rw::Texture::WRAP;
	const bool wrapV = sampler.addressV == rw::Texture::WRAP;
	if(wrapU)
		u -= std::floor(u);
	else{
		u = addressCoordinate(u, sampler.addressU);
		if(u < 0.0f)
			return false;
	}
	if(wrapV)
		v -= std::floor(v);
	else{
		v = addressCoordinate(v, sampler.addressV);
		if(v < 0.0f)
			return false;
	}
	// Convert shader coordinates to the CPU image's row orientation
	const float imageV = sampler.topDown ? v : 1.0f-v;
	float x = u*sampler.width-0.5f;
	float y = imageV*sampler.height-0.5f;
	if(!wrapU)
		x = std::max(0.0f, std::min((float)(sampler.width-1), x));
	if(!wrapV)
		y = std::max(0.0f, std::min((float)(sampler.height-1), y));
	const int rawX0 = (int)std::floor(x), rawY0 = (int)std::floor(y);
	const int rawX1 = rawX0+1, rawY1 = rawY0+1;
	//+ rouz edit (ChatGPT)
	// Wrap the neighboring texels with boundary checks because both indices are at most one texel outside the image
	const int x0 = wrapU ? (rawX0 < 0 ? sampler.width-1 : rawX0) : std::min(sampler.width-1, rawX0);
	const int x1 = wrapU ? (rawX1 == sampler.width ? 0 : rawX1) : std::min(sampler.width-1, rawX1);
	const int y0 = wrapV ? (rawY0 < 0 ? sampler.height-1 : rawY0) : std::min(sampler.height-1, rawY0);
	const int y1 = wrapV ? (rawY1 == sampler.height ? 0 : rawY1) : std::min(sampler.height-1, rawY1);
	//- rouz edit (ChatGPT)
	const float tx = x-rawX0, ty = y-rawY0;
	const rw::uint8 *p00 = sampler.pixels + (size_t)y0*sampler.stride + (size_t)x0*4;
	const rw::uint8 *p10 = sampler.pixels + (size_t)y0*sampler.stride + (size_t)x1*4;
	const rw::uint8 *p01 = sampler.pixels + (size_t)y1*sampler.stride + (size_t)x0*4;
	const rw::uint8 *p11 = sampler.pixels + (size_t)y1*sampler.stride + (size_t)x1*4;
	rw::uint8 *channels[4] = { &texel.red, &texel.green, &texel.blue, &texel.alpha };
	for(int channel = 0; channel < 4; channel++){
		const float top = p00[channel]*(1.0f-tx) + p10[channel]*tx;
		const float bottom = p01[channel]*(1.0f-tx) + p11[channel]*tx;
		*channels[channel] = (rw::uint8)(top*(1.0f-ty) + bottom*ty);
	}
	return true;
}

float edge(const ScreenVertex &a, const ScreenVertex &b, float x, float y)
{
	return (x-a.x)*(b.y-a.y) - (y-a.y)*(b.x-a.x);
}

struct PixelPlane {
	float row, dx, dy;
};

struct StencilState {
	rw::uint32 function, reference, compareMask, writeMask;
	rw::uint32 fail, depthFail, pass;
	bool enabled;
};

StencilState getStencilState()
{
	// Capture the current RenderWare stencil test and write operations
	StencilState state = {};
	state.enabled = rw::GetRenderState(rw::STENCILENABLE) != 0;
	if(!state.enabled)
		return state;
	state.function = rw::GetRenderState(rw::STENCILFUNCTION);
	state.reference = rw::GetRenderState(rw::STENCILFUNCTIONREF) & 0xFF;
	state.compareMask = rw::GetRenderState(rw::STENCILFUNCTIONMASK) & 0xFF;
	state.writeMask = rw::GetRenderState(rw::STENCILFUNCTIONWRITEMASK) & 0xFF;
	state.fail = rw::GetRenderState(rw::STENCILFAIL);
	state.depthFail = rw::GetRenderState(rw::STENCILZFAIL);
	state.pass = rw::GetRenderState(rw::STENCILPASS);
	return state;
}

bool stencilTestPasses(const StencilState &state, rw::uint8 value)
{
	// Compare the masked reference value with the masked stencil value
	const rw::uint32 stored = value & state.compareMask;
	const rw::uint32 reference = state.reference & state.compareMask;
	switch(state.function){
	case rw::STENCILNEVER: return false;
	case rw::STENCILLESS: return reference < stored;
	case rw::STENCILLESSEQUAL: return reference <= stored;
	case rw::STENCILGREATER: return reference > stored;
	case rw::STENCILEQUAL: return stored == reference;
	case rw::STENCILNOTEQUAL: return stored != reference;
	case rw::STENCILGREATEREQUAL: return reference >= stored;
	case rw::STENCILALWAYS: return true;
	default: return true;
	}
}

void applyStencilOperation(const StencilState &state, size_t index, rw::uint32 operation)
{
	// Apply the stencil operation while preserving bits outside its write mask
	if(!stencils || !state.enabled || operation == rw::STENCILKEEP)
		return;
	rw::uint8 current = stencils[index];
	rw::uint8 replacement = current;
	if(operation == rw::STENCILZERO)
		replacement = 0;
	else if(operation == rw::STENCILREPLACE)
		replacement = (rw::uint8)state.reference;
	else if(operation == rw::STENCILINCSAT)
		replacement = current == 255 ? 255 : (rw::uint8)(current+1);
	else if(operation == rw::STENCILDECSAT)
		replacement = current == 0 ? 0 : (rw::uint8)(current-1);
	else if(operation == rw::STENCILINVERT)
		replacement = (rw::uint8)~current;
	else if(operation == rw::STENCILINC)
		replacement = (rw::uint8)(current+1);
	else if(operation == rw::STENCILDEC)
		replacement = (rw::uint8)(current-1);
	stencils[index] = (rw::uint8)((current & (rw::uint8)~state.writeMask) |
		(replacement & (rw::uint8)state.writeMask));
}

struct FogState {
	float start, end, range;
	rw::RGBA color;
	bool enabled;
};

FogState getFogState(const rw::Camera *camera)
{
	// Capture the camera fog range and RenderWare's packed fog color
	FogState fog = {};
	if(!camera)
		return fog;
	fog.enabled = rw::GetRenderState(rw::FOGENABLE) != 0 && renderFogStage.load(std::memory_order_relaxed) != 0;
	fog.start = camera->fogPlane;
	fog.end = camera->farPlane;
	fog.range = fog.start != fog.end ? 1.0f/(fog.start-fog.end) : 0.0f;
	const rw::uint32 packed = rw::GetRenderState(rw::FOGCOLOR);
	fog.color = { (rw::uint8)packed, (rw::uint8)(packed >> 8),
		(rw::uint8)(packed >> 16), (rw::uint8)(packed >> 24) };
	return fog;
}

void applyFog(float color[3], float distance, const FogState &fog)
{
	// Blend visible color toward the fog color using librw's linear fog curve
	if(!fog.enabled || fog.range == 0.0f || !std::isfinite(fog.range) || !std::isfinite(distance))
		return;
	const float visibility = std::max(0.0f, std::min(1.0f, (distance-fog.end)*fog.range));
	const float fogColor[3] = { fog.color.red*(1.f/255.f), fog.color.green*(1.f/255.f), fog.color.blue*(1.f/255.f) };
	for(int channel = 0; channel < 3; channel++)
		color[channel] = fogColor[channel]+(color[channel]-fogColor[channel])*visibility;
}

void applyFog(rw::RGBA &color, float distance, const FogState &fog)
{
	// Preserve byte colors while applying the same linear fog to atomic pixels
	float channels[3] = { color.red*(1.f/255.f), color.green*(1.f/255.f), color.blue*(1.f/255.f) };
	applyFog(channels, distance, fog);
	color.red = (rw::uint8)(255.0f*channels[0]);
	color.green = (rw::uint8)(255.0f*channels[1]);
	color.blue = (rw::uint8)(255.0f*channels[2]);
}

int clipDepthPlane(const ScreenVertex *input, int count, ScreenVertex *output, float plane, bool keepGreater)
{
	// Clip in camera space so intersections preserve material and environment coordinates
	int outputCount = 0;
	ScreenVertex previous = input[count-1];
	bool previousInside = keepGreater ? previous.z >= plane : previous.z <= plane;
	for(int i = 0; i < count; i++){
		const ScreenVertex &current = input[i];
		const bool currentInside = keepGreater ? current.z >= plane : current.z <= plane;
		if(previousInside != currentInside){
			const float t = (plane-previous.z)/(current.z-previous.z);
			ScreenVertex intersection = previous;
			intersection.cameraX += t*(current.cameraX-previous.cameraX);
			intersection.cameraY += t*(current.cameraY-previous.cameraY);
			intersection.z = plane;
			intersection.u += t*(current.u-previous.u);
			intersection.v += t*(current.v-previous.v);
			intersection.u2 += t*(current.u2-previous.u2);
			intersection.v2 += t*(current.v2-previous.v2);
			intersection.dualU += t*(current.dualU-previous.dualU);
			intersection.dualV += t*(current.dualV-previous.dualV);
			// Preserve transformed normals so material-specific environment frames can map clipped vertices
			//+ rouz edit (ChatGPT)
			intersection.normalX += t*(current.normalX-previous.normalX);
			intersection.normalY += t*(current.normalY-previous.normalY);
			intersection.normalZ += t*(current.normalZ-previous.normalZ);
			//- rouz edit (ChatGPT)
			// Preserve gloss and rim lighting inputs when a triangle crosses a depth plane
			//+ rouz edit (ChatGPT)
			intersection.glossLightZ += t*(current.glossLightZ-previous.glossLightZ);
			intersection.rimRed += t*(current.rimRed-previous.rimRed);
			intersection.rimGreen += t*(current.rimGreen-previous.rimGreen);
			intersection.rimBlue += t*(current.rimBlue-previous.rimBlue);
			intersection.vehicleEnvU += t*(current.vehicleEnvU-previous.vehicleEnvU);
			intersection.vehicleEnvV += t*(current.vehicleEnvV-previous.vehicleEnvV);
			intersection.vehicleReflectBase += t*(current.vehicleReflectBase-previous.vehicleReflectBase);
			intersection.vehicleSpecRed += t*(current.vehicleSpecRed-previous.vehicleSpecRed);
			intersection.vehicleSpecGreen += t*(current.vehicleSpecGreen-previous.vehicleSpecGreen);
			intersection.vehicleSpecBlue += t*(current.vehicleSpecBlue-previous.vehicleSpecBlue);
			//- rouz edit (ChatGPT)
			intersection.color.red = (rw::uint8)(previous.color.red + t*((float)current.color.red-previous.color.red));
			intersection.color.green = (rw::uint8)(previous.color.green + t*((float)current.color.green-previous.color.green));
			intersection.color.blue = (rw::uint8)(previous.color.blue + t*((float)current.color.blue-previous.color.blue));
			intersection.color.alpha = (rw::uint8)(previous.color.alpha + t*((float)current.color.alpha-previous.color.alpha));
			intersection.lightRed += t*(current.lightRed-previous.lightRed);
			intersection.lightGreen += t*(current.lightGreen-previous.lightGreen);
			intersection.lightBlue += t*(current.lightBlue-previous.lightBlue);
			output[outputCount++] = intersection;
		}
		if(currentInside)
			output[outputCount++] = current;
		previous = current;
		previousInside = currentInside;
	}
	return outputCount;
}

struct AlphaTestState {
	bool enabled, gsEmulation;
	rw::uint32 function, reference, gsReference;
};

bool immediateAlphaTestPasses(const AlphaTestState &state, float alpha)
{
	// Apply the alpha comparisons supported by the active RenderWare backend
	if(!state.enabled || state.gsEmulation)
		return true;
	const float reference = state.reference*(1.f/255.f);
	if(state.function == rw::ALPHAGREATEREQUAL)
		return alpha >= reference;
	if(state.function == rw::ALPHALESS)
		return alpha < reference;
	return true;
}

AlphaTestState getAlphaTestState()
{
	// Capture GL and GS alpha-test settings used by the active draw
	return { rw::gl3::getAlphaTest() != 0,
		rw::GetRenderState(rw::GSALPHATEST) != 0 && rw::gl3::getAlphaBlend() != 0,
		rw::GetRenderState(rw::ALPHATESTFUNC), rw::GetRenderState(rw::ALPHATESTREF),
		rw::GetRenderState(rw::GSALPHATESTREF) };
}

struct DeferredTriangle {
	ScreenVertex vertices[3];
	rw::RGBA color;
	bool hasVertexColors;
	rw::RGBAf ambient;
	float surfaceAmbient, surfaceDiffuse;
	rw::Texture *source;
	const CachedTexture *cached;
	EnvironmentState environment;
	DualTextureState dualTexture;
	GlossState gloss;
	VehicleState vehicleState;
	rw::uint32 cullMode;
	FogState fog;
	StencilState stencil;
	AlphaTestState alphaTest;
	bool vertexAlphaEnabled, depthWrite, dynamicLighting, rimlight, vehicle;
	rw::uint32 sourceBlend, destinationBlend;
	float alphaMultiplier, sortDepth;
};

std::vector<DeferredTriangle> transparentTriangles;

struct OffscreenTargetState {
	rw::RGBA *pixels, *savedPixels;
	float *depths, *savedDepths;
	rw::uint8 *stencils, *savedStencils;
	float *tileDepthBounds, *savedTileDepthBounds;
	int width, height, tilesAcross;
	int savedWidth, savedHeight, savedTilesAcross;
	Framebuffer savedFramebuffer;
	bool savedFramePending, savedCapturingWorldEffects, savedDeferTransparentTriangles;
	bool savedScreenTextureValid, savedImmediateHasMatrix;
	float savedAtomicAlphaMultiplier;
	const RwIm3DVertex *savedImmediateVertices;
	int savedImmediateVertexCount;
	rw::Matrix savedImmediateMatrix;
	std::vector<DeferredTriangle> savedTransparentTriangles;
	bool active;
};
static OffscreenTargetState offscreenTarget = {};

bool triangleUsesAlpha(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c,
	const rw::RGBA &color, bool hasVertexColors, const CachedTexture *cached,
	const CachedTexture *secondaryCached,
	bool vertexAlphaEnabled, float alphaMultiplier)
{
	// Defer triangles whose material, texture, vertex colors, or instance fade needs blending
	return color.alpha < 255 || alphaMultiplier < 1.0f || vertexAlphaEnabled ||
		(cached && !cached->opaque) || (secondaryCached && !secondaryCached->opaque) || (hasVertexColors &&
		(a.color.alpha < 255 || b.color.alpha < 255 || c.color.alpha < 255));
}

struct TriangleRaster {
	float rowW0, rowW1, w0dx, w0dy, w1dx, w1dy;
	PixelPlane inverseDepth, uOverZ, vOverZ;
	PixelPlane envUOverZ, envVOverZ;
	PixelPlane dualUOverZ, dualVOverZ;
	PixelPlane glossLightZOverZ;
	PixelPlane rimRedOverZ, rimGreenOverZ, rimBlueOverZ;
	PixelPlane vehicleEnvUOverZ, vehicleEnvVOverZ, vehicleReflectBaseOverZ;
	PixelPlane vehicleSpecRedOverZ, vehicleSpecGreenOverZ, vehicleSpecBlueOverZ;
	PixelPlane redOverZ, greenOverZ, blueOverZ, alphaOverZ;
	PixelPlane lightRedOverZ, lightGreenOverZ, lightBlueOverZ;
	float ambientRed, ambientGreen, ambientBlue;
	float surfaceDiffuse;
	rw::RGBA color, flatShaded;
	FogState fog;
	StencilState stencil;
	AlphaTestState alphaTest;
	rw::uint32 sourceBlend, destinationBlend;
	float alphaMultiplier;
	const TextureSampler *sampler;
	const TextureSampler *envSampler;
	const TextureSampler *bumpSampler;
	const TextureSampler *dualSampler;
	const TextureSampler *glossSampler;
	const TextureSampler *vehicleEnvSampler;
	rw::RGBA envColor;
	float envCoefficient;
	float bumpCoefficient;
	float dualCoefficient;
	float glossMultiplier;
	float vehicleFresnel, vehicleShininess, vehicleSpecularity, vehicleLightStrength;
	rw::uint32 dualSourceBlend, dualDestinationBlend;
	int left, top, textured, prelit;
	bool alphaBlend, depthWrite, dynamicLighting, rimlight, vehicle, environment, envFramebufferAlpha, envApplyLight;
	bool dualLightmap;
	bool gloss;
	unsigned int coveredCount, texturedCount;
};

float effectBlendFactor(rw::uint32 mode, float source, float destination, float alpha,
	float destinationAlpha = 1.0f);

PixelPlane makePixelPlane(const TriangleRaster *raster, float av, float bv, float cv)
{
	// Build the starting value and horizontal and vertical steps for one attribute
	PixelPlane plane;
	plane.row = cv+(av-cv)*raster->rowW0+(bv-cv)*raster->rowW1;
	plane.dx = (av-cv)*raster->w0dx+(bv-cv)*raster->w1dx;
	plane.dy = (av-cv)*raster->w0dy+(bv-cv)*raster->w1dy;
	return plane;
}

float pixelPlaneAt(const PixelPlane *plane, float offsetX, float offsetY)
{
	// Find the value of a plane at the first pixel of a rectangle
	return plane->row+plane->dx*offsetX+plane->dy*offsetY;
}

static float estimateTextureLod(const TriangleRaster *raster, const PixelPlane *uOverZ,
	const PixelPlane *vOverZ, const TextureSampler *sampler, int left, int top, int right, int bottom)
{
	// Estimate minification from perspective-correct UV derivatives near the triangle center
	if(!raster || !uOverZ || !vOverZ || !sampler || !sampler->mipmapped ||
	   sampler->mipLevelCount <= 0 || sampler->width <= 0 || sampler->height <= 0)
		return 0.0f;
	const float centerX = (left+right)*0.5f-raster->left;
	const float centerY = (top+bottom)*0.5f-raster->top;
	const float inverseDepth = pixelPlaneAt(&raster->inverseDepth, centerX, centerY);
	if(!(inverseDepth > 0.0f) || !std::isfinite(inverseDepth))
		return 0.0f;
	const float inverseDepthSquared = inverseDepth*inverseDepth;
	const float uOverDepth = pixelPlaneAt(uOverZ, centerX, centerY);
	const float vOverDepth = pixelPlaneAt(vOverZ, centerX, centerY);
	const float inverseDepthX = raster->inverseDepth.dx;
	const float inverseDepthY = raster->inverseDepth.dy;
	const float duDx = (uOverZ->dx*inverseDepth-uOverDepth*inverseDepthX)/inverseDepthSquared;
	const float dvDx = (vOverZ->dx*inverseDepth-vOverDepth*inverseDepthX)/inverseDepthSquared;
	const float duDy = (uOverZ->dy*inverseDepth-uOverDepth*inverseDepthY)/inverseDepthSquared;
	const float dvDy = (vOverZ->dy*inverseDepth-vOverDepth*inverseDepthY)/inverseDepthSquared;
	const float footprintX = std::sqrt(duDx*duDx*sampler->width*sampler->width+
		dvDx*dvDx*sampler->height*sampler->height);
	const float footprintY = std::sqrt(duDy*duDy*sampler->width*sampler->width+
		dvDy*dvDy*sampler->height*sampler->height);
	// Use balanced nearest mip detail for ordinary textured triangles
	//+ rouz edit (ChatGPT)
	const float footprint = !sampler->linear && !sampler->mipLinear
		? std::sqrt(std::max(1.0f, footprintX)*std::max(1.0f, footprintY))
		: std::max(footprintX, footprintY);
	//- rouz edit (ChatGPT)
	if(!(footprint > 1.0f) || !std::isfinite(footprint))
		return 0.0f;
	const float lod = std::log2(footprint);
	return std::max(0.0f, std::min((float)sampler->mipLevelCount, lod));
}

static int estimateNearestMipLevel(const TriangleRaster *raster,
	const TextureSampler *sampler, int left, int top, int right, int bottom)
{
	// Compute the same perspective footprint without square roots or logarithms
	if(!sampler || sampler->mipLevelCount <= 0 || sampler->width <= 0 || sampler->height <= 0)
		return 0;
	const float centerX = (left+right)*0.5f-raster->left;
	const float centerY = (top+bottom)*0.5f-raster->top;
	const float inverseDepth = pixelPlaneAt(&raster->inverseDepth, centerX, centerY);
	if(!(inverseDepth > 0.0f) || !std::isfinite(inverseDepth))
		return 0;
	const float inverseDepthSquared = inverseDepth*inverseDepth;
	const float uOverDepth = pixelPlaneAt(&raster->uOverZ, centerX, centerY);
	const float vOverDepth = pixelPlaneAt(&raster->vOverZ, centerX, centerY);
	const float duDx = (raster->uOverZ.dx*inverseDepth-
		uOverDepth*raster->inverseDepth.dx)/inverseDepthSquared;
	const float dvDx = (raster->vOverZ.dx*inverseDepth-
		vOverDepth*raster->inverseDepth.dx)/inverseDepthSquared;
	const float duDy = (raster->uOverZ.dy*inverseDepth-
		uOverDepth*raster->inverseDepth.dy)/inverseDepthSquared;
	const float dvDy = (raster->vOverZ.dy*inverseDepth-
		vOverDepth*raster->inverseDepth.dy)/inverseDepthSquared;
	const float textureWidth = (float)sampler->width;
	const float textureHeight = (float)sampler->height;
	const float footprintX = duDx*duDx*textureWidth*textureWidth+
		dvDx*dvDx*textureHeight*textureHeight;
	const float footprintY = duDy*duDy*textureWidth*textureWidth+
		dvDy*dvDy*textureHeight*textureHeight;
	// Balance both screen directions so a foreshortened road keeps its markings
	//+ rouz edit (ChatGPT)
	const float footprintProduct = std::max(1.0f, footprintX)*std::max(1.0f, footprintY);
	if(!std::isfinite(footprintProduct))
		return 0;
	// Cross one fourth-power geometric threshold for each nearest mip level
	int level = 0;
	float threshold = 4.0f;
	while(level < sampler->mipLevelCount && footprintProduct >= threshold){
		level++;
		threshold *= 16.0f;
	}
	//- rouz edit (ChatGPT)
	return level;
}

// Inline into the fixed-mode wrappers so unused planes and pixel branches disappear.
#if defined(_MSC_VER)
#define SOFTWARE_POLYGONS_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define SOFTWARE_POLYGONS_INLINE inline __attribute__((always_inline))
#else
#define SOFTWARE_POLYGONS_INLINE inline
#endif
static SOFTWARE_POLYGONS_INLINE void shadeRect(TriangleRaster *raster,
	int rectLeft, int rectTop, int rectRight, int rectBottom,
	int textured, int prelit, int fullCoverage)
{
	// Start each edge and attribute at the rectangle's top left pixel
	const float offsetX = (float)(rectLeft-raster->left);
	const float offsetY = (float)(rectTop-raster->top);
	float rectW0 = raster->rowW0+raster->w0dx*offsetX+raster->w0dy*offsetY;
	float rectW1 = raster->rowW1+raster->w1dx*offsetX+raster->w1dy*offsetY;
	float rowInvz = pixelPlaneAt(&raster->inverseDepth, offsetX, offsetY);
	float rowUoz = pixelPlaneAt(&raster->uOverZ, offsetX, offsetY);
	float rowVoz = pixelPlaneAt(&raster->vOverZ, offsetX, offsetY);
	// Carry reflected and lightmap coordinates across each scanline
	//+ rouz edit (ChatGPT)
	float rowEnvUoz = pixelPlaneAt(&raster->envUOverZ, offsetX, offsetY);
	float rowEnvVoz = pixelPlaneAt(&raster->envVOverZ, offsetX, offsetY);
	float rowDualUoz = pixelPlaneAt(&raster->dualUOverZ, offsetX, offsetY);
	float rowDualVoz = pixelPlaneAt(&raster->dualVOverZ, offsetX, offsetY);
	// Carry the wet-road light direction across scanlines
	//+ rouz edit (ChatGPT)
	float rowGlossLightZoz = pixelPlaneAt(&raster->glossLightZOverZ, offsetX, offsetY);
	float rowRimRedoz = pixelPlaneAt(&raster->rimRedOverZ, offsetX, offsetY);
	float rowRimGreenoz = pixelPlaneAt(&raster->rimGreenOverZ, offsetX, offsetY);
	float rowRimBlueoz = pixelPlaneAt(&raster->rimBlueOverZ, offsetX, offsetY);
	float rowVehicleEnvUoz = pixelPlaneAt(&raster->vehicleEnvUOverZ, offsetX, offsetY);
	float rowVehicleEnvVoz = pixelPlaneAt(&raster->vehicleEnvVOverZ, offsetX, offsetY);
	float rowVehicleReflectBaseoz = pixelPlaneAt(&raster->vehicleReflectBaseOverZ, offsetX, offsetY);
	float rowVehicleSpecRedoz = pixelPlaneAt(&raster->vehicleSpecRedOverZ, offsetX, offsetY);
	float rowVehicleSpecGreenz = pixelPlaneAt(&raster->vehicleSpecGreenOverZ, offsetX, offsetY);
	float rowVehicleSpecBlueoz = pixelPlaneAt(&raster->vehicleSpecBlueOverZ, offsetX, offsetY);
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
	float rowRoz = pixelPlaneAt(&raster->redOverZ, offsetX, offsetY);
	float rowGoz = pixelPlaneAt(&raster->greenOverZ, offsetX, offsetY);
	float rowBoz = pixelPlaneAt(&raster->blueOverZ, offsetX, offsetY);
	float rowAoz = pixelPlaneAt(&raster->alphaOverZ, offsetX, offsetY);
	float rowLightRoz = pixelPlaneAt(&raster->lightRedOverZ, offsetX, offsetY);
	float rowLightGoz = pixelPlaneAt(&raster->lightGreenOverZ, offsetX, offsetY);
	float rowLightBoz = pixelPlaneAt(&raster->lightBlueOverZ, offsetX, offsetY);
	unsigned int coveredCount = 0, texturedCount = 0;
	// Delay depth and stencil changes until alpha-tested fragments are known to survive
	//+ rouz edit (ChatGPT)
	const bool lateDepthStencil = raster->alphaTest.enabled && !raster->alphaTest.gsEmulation;
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Choose minification levels for this tile so perspective gradients stay local
	TextureSampler blockSampler = {}, blockEnvSampler = {}, blockBumpSampler = {}, blockDualSampler = {}, blockGlossSampler = {}, blockVehicleEnvSampler = {};
	const TextureSampler *textureSampler = raster->sampler;
	const TextureSampler *envSampler = raster->envSampler;
	const TextureSampler *bumpSampler = raster->bumpSampler;
	const TextureSampler *dualSampler = raster->dualSampler;
	const TextureSampler *glossSampler = raster->glossSampler;
	const TextureSampler *vehicleEnvSampler = raster->vehicleEnvSampler;
	if(textureSampler && textureSampler->mipmapped && textureSampler->mipLevelCount > 0){
		blockSampler = *textureSampler;
		blockSampler.lod = estimateTextureLod(raster, &raster->uOverZ, &raster->vOverZ,
			textureSampler, rectLeft, rectTop, rectRight, rectBottom);
		textureSampler = &blockSampler;
	}
	if(envSampler && envSampler->mipmapped && envSampler->mipLevelCount > 0){
		blockEnvSampler = *envSampler;
		blockEnvSampler.lod = estimateTextureLod(raster, &raster->envUOverZ, &raster->envVOverZ,
			envSampler, rectLeft, rectTop, rectRight, rectBottom);
		envSampler = &blockEnvSampler;
	}
	if(bumpSampler && bumpSampler->mipmapped && bumpSampler->mipLevelCount > 0){
		blockBumpSampler = *bumpSampler;
		blockBumpSampler.lod = estimateTextureLod(raster, &raster->uOverZ, &raster->vOverZ,
			bumpSampler, rectLeft, rectTop, rectRight, rectBottom);
		bumpSampler = &blockBumpSampler;
	}
	if(dualSampler && dualSampler->mipmapped && dualSampler->mipLevelCount > 0){
		blockDualSampler = *dualSampler;
		blockDualSampler.lod = estimateTextureLod(raster, &raster->dualUOverZ, &raster->dualVOverZ,
			dualSampler, rectLeft, rectTop, rectRight, rectBottom);
		dualSampler = &blockDualSampler;
	}
	// Select a local gloss mip level using the road's material UV derivatives
	//+ rouz edit (ChatGPT)
	if(glossSampler && glossSampler->mipmapped && glossSampler->mipLevelCount > 0){
		blockGlossSampler = *glossSampler;
		blockGlossSampler.lod = estimateTextureLod(raster, &raster->uOverZ, &raster->vOverZ,
			glossSampler, rectLeft, rectTop, rectRight, rectBottom);
		glossSampler = &blockGlossSampler;
	}
	// Choose a local mip level for the reflected vehicle environment map
	//+ rouz edit (ChatGPT)
	if(vehicleEnvSampler && vehicleEnvSampler->mipmapped && vehicleEnvSampler->mipLevelCount > 0){
		blockVehicleEnvSampler = *vehicleEnvSampler;
		blockVehicleEnvSampler.lod = estimateTextureLod(raster, &raster->vehicleEnvUOverZ,
			&raster->vehicleEnvVOverZ, vehicleEnvSampler, rectLeft, rectTop, rectRight, rectBottom);
		vehicleEnvSampler = &blockVehicleEnvSampler;
	}
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)

	// Advance edges and attributes across each row and shade pixels that pass depth
	for(int y = rectTop; y <= rectBottom; y++){
		float w0 = rectW0, w1 = rectW1;
		float invz = rowInvz, uoz = rowUoz, voz = rowVoz;
	//+ rouz edit (ChatGPT)
		// Advance reflection and lightmap coordinates from this row's origin
		float envUoz = rowEnvUoz, envVoz = rowEnvVoz;
		float dualUoz = rowDualUoz, dualVoz = rowDualVoz;
		// Interpolate the glossy highlight direction at each covered pixel
		//+ rouz edit (ChatGPT)
		float glossLightZoz = rowGlossLightZoz;
		float rimRedoz = rowRimRedoz, rimGreenoz = rowRimGreenoz, rimBlueoz = rowRimBlueoz;
		float vehicleEnvUoz = rowVehicleEnvUoz, vehicleEnvVoz = rowVehicleEnvVoz;
		float vehicleReflectBaseoz = rowVehicleReflectBaseoz;
		float vehicleSpecRedoz = rowVehicleSpecRedoz;
		float vehicleSpecGreenz = rowVehicleSpecGreenz;
		float vehicleSpecBlueoz = rowVehicleSpecBlueoz;
		//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
		float roz = rowRoz, goz = rowGoz, boz = rowBoz, aoz = rowAoz;
		float lightRoz = rowLightRoz, lightGoz = rowLightGoz, lightBoz = rowLightBoz;
		for(int x = rectLeft; x <= rectRight;
			x++, w0 += raster->w0dx, w1 += raster->w1dx, invz += raster->inverseDepth.dx,
				uoz += raster->uOverZ.dx, voz += raster->vOverZ.dx,
				envUoz += raster->envUOverZ.dx, envVoz += raster->envVOverZ.dx,
				dualUoz += raster->dualUOverZ.dx, dualVoz += raster->dualVOverZ.dx,
				glossLightZoz += raster->glossLightZOverZ.dx,
				rimRedoz += raster->rimRedOverZ.dx, rimGreenoz += raster->rimGreenOverZ.dx,
				rimBlueoz += raster->rimBlueOverZ.dx,
				vehicleEnvUoz += raster->vehicleEnvUOverZ.dx,
				vehicleEnvVoz += raster->vehicleEnvVOverZ.dx,
				vehicleReflectBaseoz += raster->vehicleReflectBaseOverZ.dx,
				vehicleSpecRedoz += raster->vehicleSpecRedOverZ.dx,
				vehicleSpecGreenz += raster->vehicleSpecGreenOverZ.dx,
				vehicleSpecBlueoz += raster->vehicleSpecBlueOverZ.dx,
				roz += raster->redOverZ.dx, goz += raster->greenOverZ.dx, boz += raster->blueOverZ.dx,
			aoz += raster->alphaOverZ.dx, lightRoz += raster->lightRedOverZ.dx,
			lightGoz += raster->lightGreenOverZ.dx, lightBoz += raster->lightBlueOverZ.dx){
			if(!fullCoverage && (w0 < 0.0f || w1 < 0.0f || 1.0f-w0-w1 < 0.0f))
				continue;
			const size_t index = (size_t)y*width+x;
			// Keep the fast early depth and stencil checks for fragments without alpha testing
			//+ rouz edit (ChatGPT)
			if(!lateDepthStencil){
				if(raster->stencil.enabled && !stencilTestPasses(raster->stencil, stencils[index])){
					applyStencilOperation(raster->stencil, index, raster->stencil.fail);
					continue;
				}
				if(!(invz > depths[index])){
					applyStencilOperation(raster->stencil, index, raster->stencil.depthFail);
					continue;
				}
			}
			//- rouz edit (ChatGPT)
			const float z = (prelit || textured || raster->environment || raster->bumpSampler || raster->dualSampler || raster->gloss || raster->rimlight || raster->vehicle ||
				raster->fog.enabled || raster->dynamicLighting) ? 1.0f/invz : 0.0f;
			rw::RGBA shaded = raster->flatShaded;
			rw::RGBA texel = { 255, 255, 255, 255 };
			rw::RGBA envTexel = { 0, 0, 0, 255 };
			rw::RGBA bumpTexel = { 128, 128, 0, 255 };
			rw::RGBA dualTexel = { 255, 255, 255, 255 };
			//+ rouz edit (ChatGPT)
			// Sample the bump map once and offset the base texture and reflection coordinates
			float materialU = uoz*z, materialV = voz*z;
			float bumpOffsetU = 0.0f, bumpOffsetV = 0.0f;
			if(bumpSampler && sampleTexture(*bumpSampler, materialU, materialV, bumpTexel)){
				// Scale the signed bump vector by a bounded number of source texels
				//+ rouz edit (ChatGPT)
				const float bumpMapSize = (float)std::max(1,
					std::max(raster->bumpSampler->width, raster->bumpSampler->height));
				const float bumpScale = std::isfinite(raster->bumpCoefficient)
					? std::max(-0.05f, std::min(0.05f, raster->bumpCoefficient/bumpMapSize)) : 0.0f;
				//- rouz edit (ChatGPT)
				bumpOffsetU = ((float)bumpTexel.red-128.0f)*(bumpScale/127.0f);
				bumpOffsetV = ((float)bumpTexel.green-128.0f)*(bumpScale/127.0f);
			}
			//- rouz edit (ChatGPT)
			// Sample the material texture and calculate its interpolated opacity
			if(textured){
				//+ rouz edit (ChatGPT)
				// Keep base UVs unchanged when the bump map also drives an environment reflection
				const float baseOffsetU = raster->environment ? 0.0f : bumpOffsetU;
				const float baseOffsetV = raster->environment ? 0.0f : bumpOffsetV;
				//- rouz edit (ChatGPT)
				if(!sampleTexture(*textureSampler, materialU+baseOffsetU, materialV+baseOffsetV, texel))
					continue;
			}
			//+ rouz edit (ChatGPT)
			// Sample the environment map independently so a missing reflection texel keeps the base material
			if(raster->environment)
				sampleTexture(*envSampler, envUoz*z+bumpOffsetU, envVoz*z+bumpOffsetV, envTexel);
			//- rouz edit (ChatGPT)
			// Sample the optional second material texture
			//+ rouz edit (ChatGPT)
			if(dualSampler && raster->dualCoefficient != 0.0f)
				sampleTexture(*dualSampler, dualUoz*z, dualVoz*z, dualTexel);
			//- rouz edit (ChatGPT)
			// Sample the Neo vehicle environment map using its reflected UVs
			//+ rouz edit (ChatGPT)
			rw::RGBA vehicleEnvTexel = { 0, 0, 0, 255 };
			if(vehicleEnvSampler)
				sampleTexture(*vehicleEnvSampler, vehicleEnvUoz*z, vehicleEnvVoz*z, vehicleEnvTexel);
			//- rouz edit (ChatGPT)
			// Sample the gloss map with the road material coordinates
			//+ rouz edit (ChatGPT)
			rw::RGBA glossTexel = { 0, 0, 0, 0 };
			if(glossSampler)
				sampleTexture(*glossSampler, materialU, materialV, glossTexel);
			//- rouz edit (ChatGPT)
			const float vertexAlpha = prelit ? std::max(0.0f, std::min(1.0f, aoz*z*(1.f/255.f))) : 1.0f;
			const float materialAlpha = (shaded.alpha*(1.f/255.f))*vertexAlpha*(texel.alpha*(1.f/255.f))*raster->alphaMultiplier;
			float sourceAlpha = materialAlpha;
			// Include the generic MatFX pass alpha before the framebuffer alpha test
			//+ rouz edit (ChatGPT)
			if(raster->dualSampler && !raster->dualLightmap && raster->dualCoefficient != 0.0f){
				const float dualAlpha = dualTexel.alpha*(1.f/255.f);
				sourceAlpha = dualAlpha*effectBlendFactor(raster->dualSourceBlend, dualAlpha,
					materialAlpha, dualAlpha, materialAlpha)+materialAlpha*effectBlendFactor(
					raster->dualDestinationBlend, dualAlpha, materialAlpha, dualAlpha, materialAlpha);
				sourceAlpha = std::max(0.0f, std::min(1.0f, sourceAlpha));
			}
			//- rouz edit (ChatGPT)
			// Apply librw's active alpha function and reference before stencil pass operations
			//+ rouz edit (ChatGPT)
			if(raster->alphaTest.enabled && !raster->alphaTest.gsEmulation){
				const float alphaReference = raster->alphaTest.reference*(1.f/255.f);
				if((raster->alphaTest.function == rw::ALPHAGREATEREQUAL && sourceAlpha < alphaReference) ||
				   (raster->alphaTest.function == rw::ALPHALESS && sourceAlpha >= alphaReference))
					continue;
			}
			//- rouz edit (ChatGPT)
			// Preserve binary cutouts for opaque materials and blend fractional alpha
			if(raster->alphaBlend){
				if(sourceAlpha <= 0.0f)
					continue;
			}else if(textured ? (unsigned)shaded.alpha*texel.alpha < 128u*255u : shaded.alpha < 128)
				continue;
			// Apply stencil and depth tests after alpha rejection for masked fragments
			//+ rouz edit (ChatGPT)
			if(lateDepthStencil){
				if(raster->stencil.enabled && !stencilTestPasses(raster->stencil, stencils[index])){
					applyStencilOperation(raster->stencil, index, raster->stencil.fail);
					continue;
				}
				if(!(invz > depths[index])){
					applyStencilOperation(raster->stencil, index, raster->stencil.depthFail);
					continue;
				}
			}
			//- rouz edit (ChatGPT)
			// Commit the stencil pass after the fragment survives the material alpha test
			applyStencilOperation(raster->stencil, index, raster->stencil.pass);
			// Apply perspective corrected prelight and the material color
			//+ rouz edit (ChatGPT)
			// Combine prelight, ambient, and interpolated world lights like librw's vertex shader
			const float baseRed = prelit ? roz*z*(1.f/255.f) : 0.0f;
			const float baseGreen = prelit ? goz*z*(1.f/255.f) : 0.0f;
			const float baseBlue = prelit ? boz*z*(1.f/255.f) : 0.0f;
			const float litRed = std::max(0.0f, std::min(1.0f,
				baseRed+raster->ambientRed+lightRoz*z*raster->surfaceDiffuse+rimRedoz*z));
			const float litGreen = std::max(0.0f, std::min(1.0f,
				baseGreen+raster->ambientGreen+lightGoz*z*raster->surfaceDiffuse+rimGreenoz*z));
			const float litBlue = std::max(0.0f, std::min(1.0f,
				baseBlue+raster->ambientBlue+lightBoz*z*raster->surfaceDiffuse+rimBlueoz*z));
			shaded.red = (rw::uint8)(raster->color.red*litRed);
			shaded.green = (rw::uint8)(raster->color.green*litGreen);
			shaded.blue = (rw::uint8)(raster->color.blue*litBlue);
			//- rouz edit (ChatGPT)
			// Modulate RGB only for texels that passed the alpha test.
			if(textured){
				shaded.red = (rw::uint8)((unsigned)shaded.red*texel.red/255);
				shaded.green = (rw::uint8)((unsigned)shaded.green*texel.green/255);
				shaded.blue = (rw::uint8)((unsigned)shaded.blue*texel.blue/255);
			}
			// Apply the second MatFX texture as either a lightmap or a blend pass
			//+ rouz edit (ChatGPT)
			if(raster->dualSampler && raster->dualCoefficient != 0.0f){
				if(raster->dualLightmap){
					const float lightmap[3] = { dualTexel.red*(1.f/255.f), dualTexel.green*(1.f/255.f),
						dualTexel.blue*(1.f/255.f) };
					rw::uint8 *channels[3] = { &shaded.red, &shaded.green, &shaded.blue };
					for(int channel = 0; channel < 3; channel++){
						const float multiplier = 1.0f+raster->dualCoefficient*(lightmap[channel]-1.0f);
						*channels[channel] = (rw::uint8)(255.0f*std::max(0.0f,
							std::min(1.0f, (*channels[channel]*(1.f/255.f))*multiplier)));
					}
				}else{
					// Blend the generic secondary texture with the base material
					const float dualAlpha = dualTexel.alpha*(1.f/255.f);
					const rw::uint8 dualChannels[3] = { dualTexel.red, dualTexel.green, dualTexel.blue };
					rw::uint8 *channels[3] = { &shaded.red, &shaded.green, &shaded.blue };
					// Apply the MatFX blend factors to each shaded color channel
					for(int channel = 0; channel < 3; channel++){
						const float sourceColor = dualChannels[channel]*(1.f/255.f);
						const float destinationColor = *channels[channel]*(1.f/255.f);
						const float blended = sourceColor*effectBlendFactor(raster->dualSourceBlend,
							sourceColor, destinationColor, dualAlpha, materialAlpha)+destinationColor*
							effectBlendFactor(raster->dualDestinationBlend, sourceColor,
							destinationColor, dualAlpha, materialAlpha);
						*channels[channel] = (rw::uint8)(255.0f*std::max(0.0f,
							std::min(1.0f, blended)));
					}
				}
			}
			//- rouz edit (ChatGPT)
			// Mix the base vehicle paint with the reflected environment at the shader's Fresnel weight
			//+ rouz edit (ChatGPT)
			if(raster->vehicle && vehicleEnvSampler){
				const float reflectionBase = vehicleReflectBaseoz*z;
				const float reflectionAmount = ((1.0f-raster->vehicleFresnel)*reflectionBase+
					raster->vehicleFresnel)*raster->vehicleShininess;
				shaded.red = (rw::uint8)std::max(0.0f, std::min(255.0f,
					shaded.red+(vehicleEnvTexel.red-shaded.red)*reflectionAmount));
				shaded.green = (rw::uint8)std::max(0.0f, std::min(255.0f,
					shaded.green+(vehicleEnvTexel.green-shaded.green)*reflectionAmount));
				shaded.blue = (rw::uint8)std::max(0.0f, std::min(255.0f,
					shaded.blue+(vehicleEnvTexel.blue-shaded.blue)*reflectionAmount));
			}
			//- rouz edit (ChatGPT)
			// Apply camera fog after texture and prelight modulation
			//+ rouz edit (ChatGPT)
			if(raster->fog.enabled)
				applyFog(shaded, z, raster->fog);
			//- rouz edit (ChatGPT)
			// Match the Neo vehicle shader's premultiplied base and fogged specular output
			//+ rouz edit (ChatGPT)
			if(raster->vehicle){
				float fogVisibility = 1.0f;
				if(raster->fog.enabled && raster->fog.range != 0.0f &&
				   std::isfinite(raster->fog.range) && std::isfinite(z))
					fogVisibility = std::max(0.0f, std::min(1.0f,
						(z-raster->fog.end)*raster->fog.range));
				const float specularScale = raster->vehicleSpecularity*raster->vehicleLightStrength*
					fogVisibility*255.0f;
				shaded.red = (rw::uint8)std::max(0.0f, std::min(255.0f,
					shaded.red*sourceAlpha+vehicleSpecRedoz*z*specularScale));
				shaded.green = (rw::uint8)std::max(0.0f, std::min(255.0f,
					shaded.green*sourceAlpha+vehicleSpecGreenz*z*specularScale));
				shaded.blue = (rw::uint8)std::max(0.0f, std::min(255.0f,
					shaded.blue*sourceAlpha+vehicleSpecBlueoz*z*specularScale));
			}
			//- rouz edit (ChatGPT)
			//+ rouz edit (ChatGPT)
			// Blend a MatFX environment pass over its base material and destination
			if(raster->environment){
				float fogVisibility = 1.0f;
				//+ rouz edit (ChatGPT)
				if(raster->fog.enabled && raster->fog.range != 0.0f &&
				   std::isfinite(raster->fog.range) && std::isfinite(z))
					fogVisibility = std::max(0.0f, std::min(1.0f,
						(z-raster->fog.end)*raster->fog.range));
				//- rouz edit (ChatGPT)
				const float framebufferAlpha = raster->envFramebufferAlpha ? sourceAlpha : 1.0f;
				const float envFactor = raster->envCoefficient*framebufferAlpha*fogVisibility;
				rw::RGBA &destination = pixels[index];
				rw::uint8 *channels[3] = { &destination.red, &destination.green, &destination.blue };
				const rw::uint8 sourceChannels[3] = { shaded.red, shaded.green, shaded.blue };
				const rw::uint8 environmentChannels[3] = { raster->envColor.red,
					raster->envColor.green, raster->envColor.blue };
				const rw::uint8 environmentTexels[3] = { envTexel.red, envTexel.green, envTexel.blue };
				const rw::uint8 lightChannels[3] = { (rw::uint8)(255.0f*(raster->envApplyLight ? litRed : 1.0f)),
					(rw::uint8)(255.0f*(raster->envApplyLight ? litGreen : 1.0f)),
					(rw::uint8)(255.0f*(raster->envApplyLight ? litBlue : 1.0f)) };
				for(int channel = 0; channel < 3; channel++){
					const float sourceColor = sourceChannels[channel]*(1.f/255.f);
					const float destinationColor = *channels[channel]*(1.f/255.f);
					const float envColor = (environmentChannels[channel]*(1.f/255.f))*
						(environmentTexels[channel]*(1.f/255.f));
					const float envComponent = envColor*(lightChannels[channel]*(1.f/255.f))*envFactor;
					const float blended = sourceColor*sourceAlpha+envComponent+
						(raster->alphaBlend ? destinationColor*(1.0f-sourceAlpha) : 0.0f);
					*channels[channel] = (rw::uint8)(255.0f*std::max(0.0f, std::min(1.0f, blended)));
				}
			}else if(raster->alphaBlend){
				// Blend transparent materials over the existing framebuffer color
				rw::RGBA &destination = pixels[index];
				rw::uint8 *channels[3] = { &destination.red, &destination.green, &destination.blue };
				const rw::uint8 sourceChannels[3] = { shaded.red, shaded.green, shaded.blue };
				for(int channel = 0; channel < 3; channel++){
					const float sourceColor = sourceChannels[channel]*(1.f/255.f);
					const float destinationColor = *channels[channel]*(1.f/255.f);
					const float blended = sourceColor*effectBlendFactor(raster->sourceBlend, sourceColor, destinationColor, sourceAlpha)+
						destinationColor*effectBlendFactor(raster->destinationBlend, sourceColor, destinationColor, sourceAlpha);
					*channels[channel] = (rw::uint8)(255.0f*std::max(0.0f, std::min(1.0f, blended)));
				}
			}else
				pixels[index] = shaded;
			// Add the Neo wet-road highlight after the base surface has passed depth testing
			//+ rouz edit (ChatGPT)
			if(glossSampler){
				const float glossZ = glossLightZoz*z;
				const float glossZ2 = glossZ*glossZ;
				float glossAmount = glossZ2*glossZ2*glossZ2*glossZ2*raster->glossMultiplier;
				if(raster->fog.enabled && raster->fog.range != 0.0f &&
				   std::isfinite(raster->fog.range) && std::isfinite(z))
					glossAmount *= std::max(0.0f, std::min(1.0f,
						(z-raster->fog.end)*raster->fog.range));
				if(std::isfinite(glossAmount) && glossAmount != 0.0f){
					rw::RGBA &destination = pixels[index];
					destination.red = (rw::uint8)std::max(0.0f, std::min(255.0f,
						destination.red+glossTexel.red*glossAmount));
					destination.green = (rw::uint8)std::max(0.0f, std::min(255.0f,
						destination.green+glossTexel.green*glossAmount));
					destination.blue = (rw::uint8)std::max(0.0f, std::min(255.0f,
						destination.blue+glossTexel.blue*glossAmount));
				}
			}
			//- rouz edit (ChatGPT)
			//- rouz edit (ChatGPT)
			// Keep the GS low-alpha pass from replacing the depth written by solid texels
			//+ rouz edit (ChatGPT)
			if(raster->depthWrite && (!raster->alphaTest.gsEmulation ||
			   sourceAlpha >= raster->alphaTest.gsReference*(1.f/255.f)))
				depths[index] = invz;
			//- rouz edit (ChatGPT)
			pixels[index].alpha = 255;
			coveredCount++;
			if(textured || raster->dualSampler || raster->glossSampler)
				texturedCount++;
		}
		rectW0 += raster->w0dy;
		rectW1 += raster->w1dy;
		rowInvz += raster->inverseDepth.dy;
		rowUoz += raster->uOverZ.dy;
		rowVoz += raster->vOverZ.dy;
		rowDualUoz += raster->dualUOverZ.dy;
		rowDualVoz += raster->dualVOverZ.dy;
		rowGlossLightZoz += raster->glossLightZOverZ.dy;
		rowRimRedoz += raster->rimRedOverZ.dy;
		rowRimGreenoz += raster->rimGreenOverZ.dy;
		rowRimBlueoz += raster->rimBlueOverZ.dy;
		// Advance Neo vehicle reflection and specular planes with the other perspective attributes
		//+ rouz edit (ChatGPT)
		rowVehicleEnvUoz += raster->vehicleEnvUOverZ.dy;
		rowVehicleEnvVoz += raster->vehicleEnvVOverZ.dy;
		rowVehicleReflectBaseoz += raster->vehicleReflectBaseOverZ.dy;
		rowVehicleSpecRedoz += raster->vehicleSpecRedOverZ.dy;
		rowVehicleSpecGreenz += raster->vehicleSpecGreenOverZ.dy;
		rowVehicleSpecBlueoz += raster->vehicleSpecBlueOverZ.dy;
		//- rouz edit (ChatGPT)
		rowRoz += raster->redOverZ.dy;
		rowGoz += raster->greenOverZ.dy;
		rowBoz += raster->blueOverZ.dy;
		rowAoz += raster->alphaOverZ.dy;
		rowLightRoz += raster->lightRedOverZ.dy;
		rowLightGoz += raster->lightGreenOverZ.dy;
		rowLightBoz += raster->lightBlueOverZ.dy;
	}
	// Accumulate diagnostic counters after the rectangle is finished
	raster->coveredCount += coveredCount;
	raster->texturedCount += texturedCount;
}

static SOFTWARE_POLYGONS_INLINE void shadeRectSimpleOpaque(TriangleRaster *raster,
	int rectLeft, int rectTop, int rectRight, int rectBottom, int fullCoverage)
{
	// Advance coverage, depth, prelight, and nearest texture coordinates for opaque triangles
	const float offsetX = (float)(rectLeft-raster->left);
	const float offsetY = (float)(rectTop-raster->top);
	float rowW0 = raster->rowW0+raster->w0dx*offsetX+raster->w0dy*offsetY;
	float rowW1 = raster->rowW1+raster->w1dx*offsetX+raster->w1dy*offsetY;
	float rowInvz = pixelPlaneAt(&raster->inverseDepth, offsetX, offsetY);
	float rowRed = pixelPlaneAt(&raster->redOverZ, offsetX, offsetY);
	float rowGreen = pixelPlaneAt(&raster->greenOverZ, offsetX, offsetY);
	float rowBlue = pixelPlaneAt(&raster->blueOverZ, offsetX, offsetY);
	//+ rouz edit (ChatGPT)
	// Keep perspective-correct UVs for the nearest textured path
	const TextureSampler *textureSampler = raster->sampler;
	const bool nearestTexture = raster->textured && !textureSampler->solidColor;
	// Select one nearest mip level for the current block before shading pixels
	//+ rouz edit (ChatGPT)
	TextureSampler selectedMipSampler = {};
	if(nearestTexture && textureSampler->mipmapped && textureSampler->mipLevelCount > 0){
		// Select the same nearest level with squared footprint thresholds
		//+ rouz edit (ChatGPT)
		const int level = estimateNearestMipLevel(raster, textureSampler,
			rectLeft, rectTop, rectRight, rectBottom);
		//- rouz edit (ChatGPT)
		selectedMipSampler = *textureSampler;
		if(level > 0){
			const CachedMipLevel &mip = textureSampler->mipLevels[level-1];
			selectedMipSampler.pixels = mip.pixels;
			selectedMipSampler.width = mip.width;
			selectedMipSampler.height = mip.height;
			selectedMipSampler.stride = mip.stride;
		}
		selectedMipSampler.mipmapped = false;
		textureSampler = &selectedMipSampler;
	}
	//- rouz edit (ChatGPT)
	const bool nearestWrap = nearestTexture && textureSampler->wrap;
	// Keep common wrapped texture dimensions and storage outside the pixel loop
	//+ rouz edit (ChatGPT)
	const int texWidth = nearestWrap ? textureSampler->width : 0;
	const int texHeight = nearestWrap ? textureSampler->height : 0;
	const int texStride = nearestWrap ? textureSampler->stride : 0;
	const bool texTopDown = nearestWrap && textureSampler->topDown;
	const rw::uint8 *texturePixels = nearestWrap ? textureSampler->pixels : nullptr;
	//- rouz edit (ChatGPT)
	float rowUoz = nearestTexture ? pixelPlaneAt(&raster->uOverZ, offsetX, offsetY) : 0.0f;
	float rowVoz = nearestTexture ? pixelPlaneAt(&raster->vOverZ, offsetX, offsetY) : 0.0f;
	//- rouz edit (ChatGPT)
	unsigned int covered = 0;
	for(int y = rectTop; y <= rectBottom; y++){
		float w0 = rowW0, w1 = rowW1, invz = rowInvz;
		float red = rowRed, green = rowGreen, blue = rowBlue;
		//+ rouz edit (ChatGPT)
		// Start texture coordinates at the current row
		float uoz = rowUoz, voz = rowVoz;
		//- rouz edit (ChatGPT)
		for(int x = rectLeft; x <= rectRight; x++){
			if((fullCoverage || (w0 >= 0.0f && w1 >= 0.0f && 1.0f-w0-w1 >= 0.0f)) &&
			   invz > depths[(size_t)y*width+x]){
				//+ rouz edit (ChatGPT)
				// Fetch the nearest texel before writing colour or depth
				const float z = (raster->prelit || nearestTexture) ? 1.0f/invz : 0.0f;
				rw::RGBA texel = { 255, 255, 255, 255 };
				bool haveTexel = true;
				// Read ordinary wrapped textures directly without general sampler dispatch
				//+ rouz edit (ChatGPT)
				if(nearestWrap){
					float texU = uoz*z, texV = 1.0f-voz*z;
					if(std::isfinite(texU) && std::isfinite(texV)){
						// Skip wrapping arithmetic for coordinates already in the first period
						//+ rouz edit (ChatGPT)
						if(texU < 0.0f || texU >= 1.0f)
							texU -= std::floor(texU);
						if(texV < 0.0f || texV >= 1.0f)
							texV -= std::floor(texV);
						const int texX = std::min(texWidth-1, (int)(texU*texWidth));
						int texY = std::min(texHeight-1, (int)(texV*texHeight));
						if(texTopDown)
							texY = texHeight-1-texY;
						const rw::uint8 *source = texturePixels+(size_t)texY*texStride+(size_t)texX*4;
						//- rouz edit (ChatGPT)
						texel = { source[0], source[1], source[2], source[3] };
					}else
						haveTexel = false;
				}else if(nearestTexture)
					haveTexel = sampleTexture(*textureSampler, uoz*z, voz*z, texel);
				else if(raster->textured)
					texel = { 128, 128, 128, 255 };
				//- rouz edit (ChatGPT)
				// Shade only pixels with a valid nearest texel
				if(haveTexel){
				//- rouz edit (ChatGPT)
					const float lightRed = std::max(0.0f, std::min(1.0f,
					(raster->prelit ? red*z*(1.0f/255.0f) : 0.0f)+raster->ambientRed));
					const float lightGreen = std::max(0.0f, std::min(1.0f,
					(raster->prelit ? green*z*(1.0f/255.0f) : 0.0f)+raster->ambientGreen));
					const float lightBlue = std::max(0.0f, std::min(1.0f,
					(raster->prelit ? blue*z*(1.0f/255.0f) : 0.0f)+raster->ambientBlue));
					const size_t index = (size_t)y*width+x;
					// Apply the same byte rounding as the ordinary material path
					//+ rouz edit (ChatGPT)
					rw::uint8 shadedRed = (rw::uint8)(raster->color.red*lightRed);
					rw::uint8 shadedGreen = (rw::uint8)(raster->color.green*lightGreen);
					rw::uint8 shadedBlue = (rw::uint8)(raster->color.blue*lightBlue);
					if(raster->textured){
						// Modulate each channel by its sampled texel
						//+ rouz edit (ChatGPT)
						shadedRed = (rw::uint8)((unsigned)shadedRed*texel.red/255);
						shadedGreen = (rw::uint8)((unsigned)shadedGreen*texel.green/255);
						shadedBlue = (rw::uint8)((unsigned)shadedBlue*texel.blue/255);
						//- rouz edit (ChatGPT)
					}
					pixels[index] = { shadedRed, shadedGreen, shadedBlue, 255 };
					//- rouz edit (ChatGPT)
					depths[index] = invz;
					covered++;
				//+ rouz edit (ChatGPT)
				// Leave border-addressed pixels untouched when no texel exists
				}
				//- rouz edit (ChatGPT)
			}
			w0 += raster->w0dx;
			w1 += raster->w1dx;
			invz += raster->inverseDepth.dx;
			red += raster->redOverZ.dx;
			green += raster->greenOverZ.dx;
			blue += raster->blueOverZ.dx;
			//+ rouz edit (ChatGPT)
			// Advance perspective texture coordinates with the pixel
			uoz += raster->uOverZ.dx;
			voz += raster->vOverZ.dx;
			//- rouz edit (ChatGPT)
		}
		rowW0 += raster->w0dy;
		rowW1 += raster->w1dy;
		rowInvz += raster->inverseDepth.dy;
		rowRed += raster->redOverZ.dy;
		rowGreen += raster->greenOverZ.dy;
		rowBlue += raster->blueOverZ.dy;
		//+ rouz edit (ChatGPT)
		// Advance texture coordinates with the next row
		rowUoz += raster->uOverZ.dy;
		rowVoz += raster->vOverZ.dy;
		//- rouz edit (ChatGPT)
	}
	raster->coveredCount += covered;
	if(raster->textured)
		raster->texturedCount += covered;
}

static void shadeRectSimpleOpaquePartial(TriangleRaster *raster, int left, int top, int right, int bottom)
{
	// Shade partially covered blocks with edge tests
	shadeRectSimpleOpaque(raster, left, top, right, bottom, 0);
}

static void shadeRectSimpleOpaqueFull(TriangleRaster *raster, int left, int top, int right, int bottom)
{
	// Shade fully covered blocks without edge tests
	shadeRectSimpleOpaque(raster, left, top, right, bottom, 1);
}

#undef SOFTWARE_POLYGONS_INLINE

// Fixed flags let each ordinary C-style function keep only the work it needs.
static void shadeRectFlatPartial(TriangleRaster *raster, int left, int top, int right, int bottom)
{
	shadeRect(raster, left, top, right, bottom, 0, 0, 0);
}

static void shadeRectPrelitPartial(TriangleRaster *raster, int left, int top, int right, int bottom)
{
	shadeRect(raster, left, top, right, bottom, 0, 1, 0);
}

static void shadeRectTexturedPartial(TriangleRaster *raster, int left, int top, int right, int bottom)
{
	shadeRect(raster, left, top, right, bottom, 1, 0, 0);
}

static void shadeRectTexturedPrelitPartial(TriangleRaster *raster, int left, int top, int right, int bottom)
{
	shadeRect(raster, left, top, right, bottom, 1, 1, 0);
}

static void shadeRectFlatFull(TriangleRaster *raster, int left, int top, int right, int bottom)
{
	shadeRect(raster, left, top, right, bottom, 0, 0, 1);
}

static void shadeRectPrelitFull(TriangleRaster *raster, int left, int top, int right, int bottom)
{
	shadeRect(raster, left, top, right, bottom, 0, 1, 1);
}

static void shadeRectTexturedFull(TriangleRaster *raster, int left, int top, int right, int bottom)
{
	shadeRect(raster, left, top, right, bottom, 1, 0, 1);
}

static void shadeRectTexturedPrelitFull(TriangleRaster *raster, int left, int top, int right, int bottom)
{
	shadeRect(raster, left, top, right, bottom, 1, 1, 1);
}

void drawTriangle(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c,
	const rw::RGBA &color, bool hasVertexColors, const rw::RGBAf &ambient,
	float surfaceAmbient, float surfaceDiffuse, bool dynamicLighting, bool rimlight, bool vehicleAtomic,
	rw::Texture *source, const CachedTexture *cached, const EnvironmentState &environment,
	const DualTextureState &dualTexture, const GlossState &gloss, const VehicleState &vehicle,
	rw::uint32 cullMode,
	const FogState &fog, const StencilState &stencil, const AlphaTestState &alphaTest,
	bool vertexAlphaEnabled,
	bool depthWrite, rw::uint32 sourceBlend, rw::uint32 destinationBlend, float alphaMultiplier)
{
	// Reject invalid projected coordinates and degenerate triangles
	if(!std::isfinite(a.x) || !std::isfinite(a.y) || !std::isfinite(b.x) || !std::isfinite(b.y) ||
	   !std::isfinite(c.x) || !std::isfinite(c.y) || !std::isfinite(a.z) ||
	   !std::isfinite(b.z) || !std::isfinite(c.z)){
		framebuffer.nonFiniteTriangles++;
		return;
	}
	const float area = edge(a, b, c.x, c.y);
	if(!std::isfinite(area) || std::fabs(area) < 0.001f){
		framebuffer.degenerateTriangles++;
		return;
	}
	// Cull faces with the same winding as RenderWare's OpenGL state
	//+ rouz edit (ChatGPT)
	if((cullMode == rw::CULLBACK && area < 0.0f) ||
	   (cullMode == rw::CULLFRONT && area > 0.0f)){
		framebuffer.culledTriangles++;
		return;
	}
	//- rouz edit (ChatGPT)
	// Clamp the triangle's bounding box to the framebuffer
	const float minX = std::min(a.x, std::min(b.x, c.x));
	const float maxX = std::max(a.x, std::max(b.x, c.x));
	const float minY = std::min(a.y, std::min(b.y, c.y));
	const float maxY = std::max(a.y, std::max(b.y, c.y));
	if(maxX < 0.0f || minX >= width || maxY < 0.0f || minY >= height){
		framebuffer.offscreenTriangles++;
		return;
	}
	const int left = (int)std::floor(std::max(0.0f, minX));
	const int right = (int)std::ceil(std::min((float)(width-1), maxX));
	const int top = (int)std::floor(std::max(0.0f, minY));
	const int bottom = (int)std::ceil(std::min((float)(height-1), maxY));

	// Prepare edge steps, perspective planes, and constant triangle shading
	TriangleRaster raster = {};
	raster.left = left;
	raster.top = top;
	raster.textured = cached != nullptr;
	raster.prelit = hasVertexColors;
	raster.color = color;
	raster.flatShaded = color;
	raster.fog = fog;
	raster.stencil = stencil;
	raster.alphaTest = alphaTest;
	raster.sourceBlend = sourceBlend;
	raster.destinationBlend = destinationBlend;
		raster.alphaMultiplier = alphaMultiplier;
		raster.depthWrite = depthWrite;
		raster.dynamicLighting = dynamicLighting;
		raster.rimlight = rimlight;
		raster.vehicle = vehicleAtomic && vehicle.enabled;
		// Set reflection properties for materials that have a decoded environment map
		//+ rouz edit (ChatGPT)
		raster.environment = !raster.vehicle && environment.cached && environment.coefficient != 0.0f;
		raster.envColor = environment.color;
	raster.envCoefficient = environment.coefficient;
	raster.envFramebufferAlpha = environment.framebufferAlpha;
	raster.envApplyLight = environment.applyLight;
	raster.bumpCoefficient = environment.bumpCoefficient;
	//- rouz edit (ChatGPT)
		// Carry a decoded second texture into the world lightmap fragment path
		//+ rouz edit (ChatGPT)
		raster.dualCoefficient = dualTexture.coefficient;
		raster.dualSourceBlend = dualTexture.sourceBlend;
		raster.dualDestinationBlend = dualTexture.destinationBlend;
		raster.dualLightmap = dualTexture.lightmap;
		//- rouz edit (ChatGPT)
		// Keep the optional wet-road gloss pass attached to the base triangle
		//+ rouz edit (ChatGPT)
		raster.gloss = gloss.cached && std::isfinite(gloss.multiplier) && gloss.multiplier != 0.0f;
		raster.glossMultiplier = gloss.multiplier;
		//- rouz edit (ChatGPT)
		// Scale vehicle lighting and capture its per-material reflection controls
		//+ rouz edit (ChatGPT)
		raster.vehicleFresnel = vehicle.fresnel;
		raster.vehicleShininess = vehicle.shininess;
		raster.vehicleSpecularity = vehicle.specularity;
		raster.vehicleLightStrength = vehicle.lightStrength;
		//- rouz edit (ChatGPT)
		// Include transparency from the active secondary sampler in this triangle
		//+ rouz edit (ChatGPT)
		raster.alphaBlend = triangleUsesAlpha(a, b, c, color, hasVertexColors,
			cached, dualTexture.coefficient != 0.0f ? dualTexture.cached : nullptr,
			vertexAlphaEnabled, alphaMultiplier);
		//- rouz edit (ChatGPT)
	raster.ambientRed = ambient.red*surfaceAmbient*(raster.vehicle ? raster.vehicleLightStrength : 1.0f);
	raster.ambientGreen = ambient.green*surfaceAmbient*(raster.vehicle ? raster.vehicleLightStrength : 1.0f);
	raster.ambientBlue = ambient.blue*surfaceAmbient*(raster.vehicle ? raster.vehicleLightStrength : 1.0f);
	raster.surfaceDiffuse = surfaceDiffuse*(raster.vehicle ? raster.vehicleLightStrength : 1.0f);
	const float inverseArea = 1.0f/area;
	raster.rowW0 = edge(b, c, left+0.5f, top+0.5f)*inverseArea;
	raster.rowW1 = edge(c, a, left+0.5f, top+0.5f)*inverseArea;
	raster.w0dx = (c.y-b.y)*inverseArea;
	raster.w0dy = (b.x-c.x)*inverseArea;
	raster.w1dx = (a.y-c.y)*inverseArea;
	raster.w1dy = (c.x-a.x)*inverseArea;
	const float az = 1.0f/a.z, bz = 1.0f/b.z, cz = 1.0f/c.z;
	raster.inverseDepth = makePixelPlane(&raster, az, bz, cz);
	TextureSampler sampler = {};
	if(raster.textured){
		const rw::Image *image = cached->image;
		sampler.pixels = image->pixels;
		sampler.width = image->width;
		sampler.height = image->height;
		sampler.stride = image->stride;
		//+ rouz edit (ChatGPT)
		// Attach mip levels and filtering modes from the cached material texture
		sampler.mipLevels = cached->mipLevels;
		sampler.mipLevelCount = cached->mipLevelCount;
		//- rouz edit (ChatGPT)
		sampler.topDown = cached->topDown;
		sampler.linear = filterUsesLinear(source->getFilter());
		sampler.mipmapped = filterUsesMipmaps(source->getFilter());
		sampler.mipLinear = filterBlendsMipLevels(source->getFilter());
		sampler.addressU = (rw::Texture::Addressing)((source->filterAddressing >> 8) & 0xF);
		sampler.addressV = (rw::Texture::Addressing)((source->filterAddressing >> 12) & 0xF);
		sampler.wrap = sampler.addressU == rw::Texture::WRAP && sampler.addressV == rw::Texture::WRAP;
		applyTextureSamplingMode(&sampler, materialTextureSamplingMode);
		raster.sampler = &sampler;
		// Solid-colour sampling never uses texture coordinates
		//+ rouz edit (ChatGPT)
		if(!sampler.solidColor){
			raster.uOverZ = makePixelPlane(&raster, a.u*az, b.u*bz, c.u*cz);
			raster.vOverZ = makePixelPlane(&raster, a.v*az, b.v*bz, c.v*cz);
		}
		//- rouz edit (ChatGPT)
	}
	//+ rouz edit (ChatGPT)
	// Preserve perspective-correct coordinates when bump or gloss has no base texture
	if(materialTextureSamplingMode != TEXTURE_SAMPLING_SOLID_COLOR && !raster.textured && ((environment.bumpCached && std::isfinite(environment.bumpCoefficient) &&
	   environment.bumpCoefficient != 0.0f) || raster.gloss)){
		raster.uOverZ = makePixelPlane(&raster, a.u*az, b.u*bz, c.u*cz);
		raster.vOverZ = makePixelPlane(&raster, a.v*az, b.v*bz, c.v*cz);
	}
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Resolve a second perspective-correct sampler for the material environment map
	TextureSampler envSampler = {};
	if(raster.environment){
		const rw::Image *image = environment.cached->image;
		envSampler.pixels = image->pixels;
		envSampler.width = image->width;
		envSampler.height = image->height;
		envSampler.stride = image->stride;
		envSampler.mipLevels = environment.cached->mipLevels;
		envSampler.mipLevelCount = environment.cached->mipLevelCount;
		envSampler.topDown = environment.cached->topDown;
		envSampler.linear = filterUsesLinear(environment.source->getFilter());
		envSampler.mipmapped = filterUsesMipmaps(environment.source->getFilter());
		envSampler.mipLinear = filterBlendsMipLevels(environment.source->getFilter());
		envSampler.addressU = (rw::Texture::Addressing)((environment.source->filterAddressing >> 8) & 0xF);
		envSampler.addressV = (rw::Texture::Addressing)((environment.source->filterAddressing >> 12) & 0xF);
		envSampler.wrap = envSampler.addressU == rw::Texture::WRAP && envSampler.addressV == rw::Texture::WRAP;
		applyTextureSamplingMode(&envSampler, materialTextureSamplingMode);
		raster.envSampler = &envSampler;
		raster.envUOverZ = makePixelPlane(&raster, a.u2*az, b.u2*bz, c.u2*cz);
		raster.envVOverZ = makePixelPlane(&raster, a.v2*az, b.v2*bz, c.v2*cz);
	}
	//+ rouz edit (ChatGPT)
	// Resolve the bump sampler for standalone and environment-mapped bump effects
	TextureSampler bumpSampler = {};
	if(environment.bumpCached &&
	   std::isfinite(environment.bumpCoefficient) && environment.bumpCoefficient != 0.0f){
		const rw::Image *image = environment.bumpCached->image;
		bumpSampler.pixels = image->pixels;
		bumpSampler.width = image->width;
		bumpSampler.height = image->height;
		bumpSampler.stride = image->stride;
		bumpSampler.mipLevels = environment.bumpCached->mipLevels;
		bumpSampler.mipLevelCount = environment.bumpCached->mipLevelCount;
		bumpSampler.topDown = environment.bumpCached->topDown;
		bumpSampler.linear = filterUsesLinear(environment.bumpSource->getFilter());
		bumpSampler.mipmapped = filterUsesMipmaps(environment.bumpSource->getFilter());
		bumpSampler.mipLinear = filterBlendsMipLevels(environment.bumpSource->getFilter());
		bumpSampler.addressU = (rw::Texture::Addressing)((environment.bumpSource->filterAddressing >> 8) & 0xF);
		bumpSampler.addressV = (rw::Texture::Addressing)((environment.bumpSource->filterAddressing >> 12) & 0xF);
		bumpSampler.wrap = bumpSampler.addressU == rw::Texture::WRAP && bumpSampler.addressV == rw::Texture::WRAP;
		applyTextureSamplingMode(&bumpSampler, materialTextureSamplingMode);
		raster.bumpSampler = &bumpSampler;
	}
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
	// Configure perspective interpolation for the optional second material UV set
	//+ rouz edit (ChatGPT)
	TextureSampler dualSampler = {};
	if(dualTexture.cached && dualTexture.coefficient != 0.0f){
		const rw::Image *image = dualTexture.cached->image;
		dualSampler.pixels = image->pixels;
		dualSampler.width = image->width;
		dualSampler.height = image->height;
		dualSampler.stride = image->stride;
		dualSampler.mipLevels = dualTexture.cached->mipLevels;
		dualSampler.mipLevelCount = dualTexture.cached->mipLevelCount;
		dualSampler.topDown = dualTexture.cached->topDown;
		dualSampler.linear = filterUsesLinear(dualTexture.source->getFilter());
		dualSampler.mipmapped = filterUsesMipmaps(dualTexture.source->getFilter());
		dualSampler.mipLinear = filterBlendsMipLevels(dualTexture.source->getFilter());
		dualSampler.addressU = (rw::Texture::Addressing)((dualTexture.source->filterAddressing >> 8) & 0xF);
		dualSampler.addressV = (rw::Texture::Addressing)((dualTexture.source->filterAddressing >> 12) & 0xF);
		dualSampler.wrap = dualSampler.addressU == rw::Texture::WRAP && dualSampler.addressV == rw::Texture::WRAP;
		applyTextureSamplingMode(&dualSampler, materialTextureSamplingMode);
		raster.dualSampler = &dualSampler;
		raster.dualUOverZ = makePixelPlane(&raster, a.dualU*az, b.dualU*bz, c.dualU*cz);
		raster.dualVOverZ = makePixelPlane(&raster, a.dualV*az, b.dualV*bz, c.dualV*cz);
	}
	//- rouz edit (ChatGPT)
	// Prepare the cached wet-road gloss texture with its own filter settings
	//+ rouz edit (ChatGPT)
	TextureSampler glossSampler = {};
	if(raster.gloss){
		const rw::Image *image = gloss.cached->image;
		glossSampler.pixels = image->pixels;
		glossSampler.width = image->width;
		glossSampler.height = image->height;
		glossSampler.stride = image->stride;
		glossSampler.mipLevels = gloss.cached->mipLevels;
		glossSampler.mipLevelCount = gloss.cached->mipLevelCount;
		glossSampler.topDown = gloss.cached->topDown;
		glossSampler.linear = filterUsesLinear(gloss.source->getFilter());
		glossSampler.mipmapped = filterUsesMipmaps(gloss.source->getFilter());
		glossSampler.mipLinear = filterBlendsMipLevels(gloss.source->getFilter());
		glossSampler.addressU = (rw::Texture::Addressing)((gloss.source->filterAddressing >> 8) & 0xF);
		glossSampler.addressV = (rw::Texture::Addressing)((gloss.source->filterAddressing >> 12) & 0xF);
		glossSampler.wrap = glossSampler.addressU == rw::Texture::WRAP &&
			glossSampler.addressV == rw::Texture::WRAP;
		applyTextureSamplingMode(&glossSampler, materialTextureSamplingMode);
		raster.glossSampler = &glossSampler;
	}
	//- rouz edit (ChatGPT)
	// Prepare the vehicle's reflected environment texture and UV planes
	//+ rouz edit (ChatGPT)
	TextureSampler vehicleEnvSampler = {};
	if(raster.vehicle && vehicle.environmentCached && vehicle.environmentSource){
		const rw::Image *image = vehicle.environmentCached->image;
		vehicleEnvSampler.pixels = image->pixels;
		vehicleEnvSampler.width = image->width;
		vehicleEnvSampler.height = image->height;
		vehicleEnvSampler.stride = image->stride;
		vehicleEnvSampler.mipLevels = vehicle.environmentCached->mipLevels;
		vehicleEnvSampler.mipLevelCount = vehicle.environmentCached->mipLevelCount;
		vehicleEnvSampler.topDown = vehicle.environmentCached->topDown;
		vehicleEnvSampler.linear = filterUsesLinear(vehicle.environmentSource->getFilter());
		vehicleEnvSampler.mipmapped = filterUsesMipmaps(vehicle.environmentSource->getFilter());
		vehicleEnvSampler.mipLinear = filterBlendsMipLevels(vehicle.environmentSource->getFilter());
		vehicleEnvSampler.addressU = (rw::Texture::Addressing)((vehicle.environmentSource->filterAddressing >> 8) & 0xF);
		vehicleEnvSampler.addressV = (rw::Texture::Addressing)((vehicle.environmentSource->filterAddressing >> 12) & 0xF);
		vehicleEnvSampler.wrap = vehicleEnvSampler.addressU == rw::Texture::WRAP &&
			vehicleEnvSampler.addressV == rw::Texture::WRAP;
		applyTextureSamplingMode(&vehicleEnvSampler, materialTextureSamplingMode);
		raster.vehicleEnvSampler = &vehicleEnvSampler;
	}
	//- rouz edit (ChatGPT)
	if(raster.prelit){
		raster.redOverZ = makePixelPlane(&raster, a.color.red*az, b.color.red*bz, c.color.red*cz);
		raster.greenOverZ = makePixelPlane(&raster, a.color.green*az, b.color.green*bz, c.color.green*cz);
		raster.blueOverZ = makePixelPlane(&raster, a.color.blue*az, b.color.blue*bz, c.color.blue*cz);
		raster.alphaOverZ = makePixelPlane(&raster, a.color.alpha*az, b.color.alpha*bz, c.color.alpha*cz);
	}else{
		raster.alphaOverZ = makePixelPlane(&raster, 255.0f*az, 255.0f*bz, 255.0f*cz);
	}
	// Build optional lighting planes only for enabled shader passes
	//+ rouz edit (ChatGPT)
	if(raster.dynamicLighting){
		raster.lightRedOverZ = makePixelPlane(&raster, a.lightRed*az, b.lightRed*bz, c.lightRed*cz);
		raster.lightGreenOverZ = makePixelPlane(&raster, a.lightGreen*az, b.lightGreen*bz, c.lightGreen*cz);
		raster.lightBlueOverZ = makePixelPlane(&raster, a.lightBlue*az, b.lightBlue*bz, c.lightBlue*cz);
	}
	//- rouz edit (ChatGPT)
	// Interpolate the wet-road highlight direction with the same perspective correction
	//+ rouz edit (ChatGPT)
	if(raster.gloss)
		raster.glossLightZOverZ = makePixelPlane(&raster,
			a.glossLightZ*az, b.glossLightZ*bz, c.glossLightZ*cz);
	// Carry the reflected environment coordinates and specular highlights across pixels
	//+ rouz edit (ChatGPT)
	// Build vehicle reflection and specular planes only for vehicle materials
	//+ rouz edit (ChatGPT)
	if(raster.vehicle){
		raster.vehicleEnvUOverZ = makePixelPlane(&raster, a.vehicleEnvU*az, b.vehicleEnvU*bz, c.vehicleEnvU*cz);
		raster.vehicleEnvVOverZ = makePixelPlane(&raster, a.vehicleEnvV*az, b.vehicleEnvV*bz, c.vehicleEnvV*cz);
		raster.vehicleReflectBaseOverZ = makePixelPlane(&raster,
			a.vehicleReflectBase*az, b.vehicleReflectBase*bz, c.vehicleReflectBase*cz);
		raster.vehicleSpecRedOverZ = makePixelPlane(&raster,
			a.vehicleSpecRed*az, b.vehicleSpecRed*bz, c.vehicleSpecRed*cz);
		raster.vehicleSpecGreenOverZ = makePixelPlane(&raster,
			a.vehicleSpecGreen*az, b.vehicleSpecGreen*bz, c.vehicleSpecGreen*cz);
		raster.vehicleSpecBlueOverZ = makePixelPlane(&raster,
			a.vehicleSpecBlue*az, b.vehicleSpecBlue*bz, c.vehicleSpecBlue*cz);
	}
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
	// Interpolate the ped rim-light colors from the same vertex shader inputs
	//+ rouz edit (ChatGPT)
	// Build ped rim-light planes only when the rim pass is active
	//+ rouz edit (ChatGPT)
	if(raster.rimlight){
		raster.rimRedOverZ = makePixelPlane(&raster, a.rimRed*az, b.rimRed*bz, c.rimRed*cz);
		raster.rimGreenOverZ = makePixelPlane(&raster, a.rimGreen*az, b.rimGreen*bz, c.rimGreen*cz);
		raster.rimBlueOverZ = makePixelPlane(&raster, a.rimBlue*az, b.rimBlue*bz, c.rimBlue*cz);
	}
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)

	// Select both coverage paths once per triangle, outside the pixel loop.
	typedef void (*ShadeRect)(TriangleRaster *, int, int, int, int);
	static const ShadeRect partialShaders[] = {
		shadeRectFlatPartial, shadeRectPrelitPartial,
		shadeRectTexturedPartial, shadeRectTexturedPrelitPartial
	};
	static const ShadeRect fullShaders[] = {
		shadeRectFlatFull, shadeRectPrelitFull,
		shadeRectTexturedFull, shadeRectTexturedPrelitFull
	};
	const int shaderIndex = raster.textured*2+raster.prelit;
	// Select a reduced opaque shader when only constant grey and prelight remain
	//+ rouz edit (ChatGPT)
	const bool alphaAlwaysPass = (!raster.alphaTest.gsEmulation || raster.alphaTest.gsReference < 255) &&
		(!raster.alphaTest.enabled || raster.alphaTest.gsEmulation ||
		 raster.alphaTest.function == rw::ALPHAALWAYS ||
		 (raster.alphaTest.function == rw::ALPHAGREATEREQUAL && raster.alphaTest.reference < 255));
	const bool simpleOpaque = raster.depthWrite && !raster.alphaBlend && !raster.stencil.enabled &&
		alphaAlwaysPass && !raster.fog.enabled &&
		!raster.dynamicLighting && !raster.rimlight && !raster.vehicle && !raster.environment &&
		!raster.bumpSampler && !raster.dualSampler && !raster.glossSampler &&
		(!raster.textured || (raster.sampler && (raster.sampler->solidColor ||
			(!raster.sampler->linear && !raster.sampler->mipLinear && cached->opaque))));
	const ShadeRect shadePartial = simpleOpaque ? shadeRectSimpleOpaquePartial : partialShaders[shaderIndex];
	const ShadeRect shadeFull = simpleOpaque ? shadeRectSimpleOpaqueFull : fullShaders[shaderIndex];
	if(simpleOpaque)
		framebuffer.simpleOpaqueTriangles++;
	//- rouz edit (ChatGPT)
	// Only fully covered opaque blocks can raise a conservative depth bound
	//+ rouz edit (ChatGPT)
	const bool finiteUVs = std::isfinite(a.u) && std::isfinite(a.v) &&
		std::isfinite(b.u) && std::isfinite(b.v) && std::isfinite(c.u) && std::isfinite(c.v) &&
		std::fabs(a.u) < 1000000.0f && std::fabs(a.v) < 1000000.0f &&
		std::fabs(b.u) < 1000000.0f && std::fabs(b.v) < 1000000.0f &&
		std::fabs(c.u) < 1000000.0f && std::fabs(c.v) < 1000000.0f;
	const bool opaqueTile = !raster.stencil.enabled && !raster.alphaBlend && raster.depthWrite && color.alpha >= 128 &&
		(!raster.textured || (cached->opaque && finiteUVs &&
		 sampler.addressU != rw::Texture::BORDER && sampler.addressV != rw::Texture::BORDER));
	//- rouz edit (ChatGPT)

	// Draw small triangles directly and classify eight pixel blocks on larger ones
	if((right-left+1)*(bottom-top+1) <= 256)
		shadePartial(&raster, left, top, right, bottom);
	else{
		const int blockSize = 8;
		const float w2dx = -raster.w0dx-raster.w1dx;
		const float w2dy = -raster.w0dy-raster.w1dy;
		// Align blocks to the framebuffer so other triangles can reuse their depth bounds
		//+ rouz edit (ChatGPT)
		for(int tileTop = top & ~(blockSize-1); tileTop <= bottom; tileTop += blockSize)
			for(int tileLeft = left & ~(blockSize-1); tileLeft <= right; tileLeft += blockSize){
				const int blockLeft = std::max(left, tileLeft);
				const int blockTop = std::max(top, tileTop);
				const int blockRight = std::min(right, tileLeft+blockSize-1);
				const int blockBottom = std::min(bottom, tileTop+blockSize-1);
				const size_t tileIndex = (size_t)(tileTop/blockSize)*tilesAcross+tileLeft/blockSize;
				const float spanX = (float)(blockRight-blockLeft);
				const float spanY = (float)(blockBottom-blockTop);
				const float originW0 = raster.rowW0+raster.w0dx*(blockLeft-left)+raster.w0dy*(blockTop-top);
				const float originW1 = raster.rowW1+raster.w1dx*(blockLeft-left)+raster.w1dy*(blockTop-top);
				const float originW2 = 1.0f-originW0-originW1;
				const float minW0 = originW0+std::min(0.0f, spanX*raster.w0dx)+std::min(0.0f, spanY*raster.w0dy);
				const float minW1 = originW1+std::min(0.0f, spanX*raster.w1dx)+std::min(0.0f, spanY*raster.w1dy);
				const float minW2 = originW2+std::min(0.0f, spanX*w2dx)+std::min(0.0f, spanY*w2dy);
				const float maxW0 = originW0+std::max(0.0f, spanX*raster.w0dx)+std::max(0.0f, spanY*raster.w0dy);
				const float maxW1 = originW1+std::max(0.0f, spanX*raster.w1dx)+std::max(0.0f, spanY*raster.w1dy);
				const float maxW2 = originW2+std::max(0.0f, spanX*w2dx)+std::max(0.0f, spanY*w2dy);
				const float margin = 0.00001f;
				if(maxW0 < -margin || maxW1 < -margin || maxW2 < -margin)
					continue;
				const int fullCoverage = minW0 > margin && minW1 > margin && minW2 > margin;
				// Skip blocks whose nearest triangle depth is behind every stored pixel
				const float originInvz = pixelPlaneAt(&raster.inverseDepth, (float)(blockLeft-left), (float)(blockTop-top));
				const float depthX = spanX*raster.inverseDepth.dx;
				const float depthY = spanY*raster.inverseDepth.dy;
				const float depthMargin = std::fabs(originInvz)*0.00001f+0.0000001f;
				const float maxInvz = originInvz+std::max(0.0f, depthX)+std::max(0.0f, depthY)+depthMargin;
				if(maxInvz <= tileDepthBounds[tileIndex]){
					framebuffer.tileDepthRejectedBlocks++;
					continue;
				}
				(fullCoverage ? shadeFull : shadePartial)(&raster, blockLeft, blockTop, blockRight, blockBottom);
				// Advance the lower bound only when the entire tile got an opaque depth candidate
				if(opaqueTile && fullCoverage && blockLeft == tileLeft && blockTop == tileTop &&
				   blockRight == std::min(width-1, tileLeft+blockSize-1) &&
				   blockBottom == std::min(height-1, tileTop+blockSize-1)){
					const float minInvz = originInvz+std::min(0.0f, depthX)+std::min(0.0f, depthY)-depthMargin;
					if(std::isfinite(minInvz) && minInvz > tileDepthBounds[tileIndex])
						tileDepthBounds[tileIndex] = minInvz;
				}
			}
		//- rouz edit (ChatGPT)
	}
	// Update framebuffer counters once after the triangle is complete
	framebuffer.coveredPixels += raster.coveredCount;
	if(simpleOpaque)
		framebuffer.simpleOpaquePixels += raster.coveredCount;
	framebuffer.totalCoveredPixels += raster.coveredCount;
	framebuffer.texturedPixels += raster.texturedCount;
}

float effectBlendFactor(rw::uint32 mode, float source, float destination, float alpha,
	float destinationAlpha)
{
	// Resolve the blend factors used by shadows, skidmarks, glass, and rubbish
	switch(mode){
	case rw::BLENDZERO: return 0.0f;
	case rw::BLENDONE: return 1.0f;
	case rw::BLENDSRCCOLOR: return source;
	case rw::BLENDINVSRCCOLOR: return 1.0f-source;
	case rw::BLENDSRCALPHA: return alpha;
	case rw::BLENDINVSRCALPHA: return 1.0f-alpha;
	case rw::BLENDDESTCOLOR: return destination;
	case rw::BLENDINVDESTCOLOR: return 1.0f-destination;
	case rw::BLENDDESTALPHA: return destinationAlpha;
	case rw::BLENDINVDESTALPHA: return 1.0f-destinationAlpha;
	case rw::BLENDSRCALPHASAT: return std::min(alpha, 1.0f-destinationAlpha);
	default: return 1.0f;
	}
}

void drawImmediateTriangle(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c,
	const TextureSampler *sampler, rw::uint32 sourceBlend, rw::uint32 destinationBlend,
	bool depthTest, bool depthWrite, rw::uint32 cullMode, const FogState &fog, const StencilState &stencil,
	const AlphaTestState &alphaTest, const TextureSampler *secondarySampler = nullptr)
{
	// Reject invalid or offscreen effect triangles before rasterizing pixels
	const float area = edge(a, b, c.x, c.y);
	if(!std::isfinite(area) || std::fabs(area) < 0.001f)
		return;
	// Match the atomic rasterizer's front-face rule for immediate effect triangles
	//+ rouz edit (ChatGPT)
	if((cullMode == rw::CULLBACK && area < 0.0f) ||
	   (cullMode == rw::CULLFRONT && area > 0.0f)){
		framebuffer.culledTriangles++;
		return;
	}
	//- rouz edit (ChatGPT)
	const float minX = std::min(a.x, std::min(b.x, c.x));
	const float maxX = std::max(a.x, std::max(b.x, c.x));
	const float minY = std::min(a.y, std::min(b.y, c.y));
	const float maxY = std::max(a.y, std::max(b.y, c.y));
	if(!std::isfinite(minX) || !std::isfinite(maxX) || !std::isfinite(minY) || !std::isfinite(maxY) ||
	   maxX < 0.0f || minX >= width || maxY < 0.0f || minY >= height)
		return;
	const int left = (int)std::floor(std::max(0.0f, minX));
	const int right = (int)std::ceil(std::min((float)(width-1), maxX));
	const int top = (int)std::floor(std::max(0.0f, minY));
	const int bottom = (int)std::ceil(std::min((float)(height-1), maxY));
	const float inverseArea = 1.0f/area;
	const float az = 1.0f/a.z, bz = 1.0f/b.z, cz = 1.0f/c.z;
	// Keep shared screen-space layers at exactly the same depth across both triangles
	//+ rouz edit (ChatGPT)
	const bool flatDepth = a.z == b.z && b.z == c.z;
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Pick a mip level from this world or screen effect triangle's UV footprint
	TextureSampler mipSampler = {};
	const TextureSampler *drawSampler = sampler;
	if(sampler && sampler->mipmapped && sampler->mipLevelCount > 0){
		mipSampler = *sampler;
		mipSampler.lod = estimateImmediateTextureLod(a, b, c, sampler);
		drawSampler = &mipSampler;
	}
	//- rouz edit (ChatGPT)
	// Delay depth and stencil changes until alpha-tested effects survive their texture sample
	//+ rouz edit (ChatGPT)
	const bool lateDepthStencil = alphaTest.enabled && !alphaTest.gsEmulation;
	//- rouz edit (ChatGPT)
	// Interpolate vertex color and UVs in perspective before applying blend states
	//+ rouz edit (ChatGPT)
	// Step barycentric edges across short spans and rows instead of evaluating two edge equations per pixel
	const float w0dx = (c.y-b.y)*inverseArea;
	const float w0dy = (b.x-c.x)*inverseArea;
	const float w1dx = (a.y-c.y)*inverseArea;
	const float w1dy = (c.x-a.x)*inverseArea;
	float rowW0 = edge(b, c, left+0.5f, top+0.5f)*inverseArea;
	float rowW1 = edge(c, a, left+0.5f, top+0.5f)*inverseArea;
	for(int y = top; y <= bottom; y++, rowW0 += w0dy, rowW1 += w1dy)
		for(int spanLeft = left; spanLeft <= right; spanLeft += 32){
			const int spanRight = std::min(right, spanLeft+31);
			float w0 = rowW0+(spanLeft-left)*w0dx;
			float w1 = rowW1+(spanLeft-left)*w1dx;
			for(int x = spanLeft; x <= spanRight; x++, w0 += w0dx, w1 += w1dx){
				const float w2 = 1.0f-w0-w1;
				if(w0 < 0.0f || w1 < 0.0f || w2 < 0.0f)
					continue;
				const float invz = flatDepth ? az : w0*az+w1*bz+w2*cz;
				const size_t pixelIndex = (size_t)y*width+x;
				if(invz <= 0.0f)
					continue;
				// Keep early tests for effects whose alpha cannot discard a fragment
				//+ rouz edit (ChatGPT)
				if(!lateDepthStencil){
					if(stencil.enabled && !stencilTestPasses(stencil, stencils[pixelIndex])){
						applyStencilOperation(stencil, pixelIndex, stencil.fail);
						continue;
					}
					if(depthTest && invz < depths[pixelIndex]){
						applyStencilOperation(stencil, pixelIndex, stencil.depthFail);
						continue;
					}
				}
				//- rouz edit (ChatGPT)
				const float weightA = w0*az/invz;
				const float weightB = w1*bz/invz;
				const float weightC = w2*cz/invz;
				rw::RGBA texel = { 255, 255, 255, 255 };
				if(drawSampler && !sampleTexture(*drawSampler,
				    weightA*a.u+weightB*b.u+weightC*c.u,
				    weightA*a.v+weightB*b.v+weightC*c.v, texel))
					continue;
				//+ rouz edit (ChatGPT)
				// Combine the droplet mask with the saved scene sample before blending
				rw::RGBA secondaryTexel = { 255, 255, 255, 255 };
				if(secondarySampler && !sampleTextureLinear(*secondarySampler,
				    weightA*a.u2+weightB*b.u2+weightC*c.u2,
				    weightA*a.v2+weightB*b.v2+weightC*c.v2, secondaryTexel))
					continue;
				const float alpha = (weightA*a.color.alpha+weightB*b.color.alpha+weightC*c.color.alpha)*texel.alpha*secondaryTexel.alpha/(255.0f*255.0f*255.0f);
				if(alpha <= 0.0f)
					continue;
				// Apply the captured alpha test before committing stencil or color
				//+ rouz edit (ChatGPT)
				if(alphaTest.enabled && !alphaTest.gsEmulation){
					const float alphaReference = alphaTest.reference*(1.f/255.f);
					if((alphaTest.function == rw::ALPHAGREATEREQUAL && alpha < alphaReference) ||
					   (alphaTest.function == rw::ALPHALESS && alpha >= alphaReference))
						continue;
				}
				// Apply late stencil and depth tests after texture alpha rejection
				//+ rouz edit (ChatGPT)
				if(lateDepthStencil){
					if(stencil.enabled && !stencilTestPasses(stencil, stencils[pixelIndex])){
						applyStencilOperation(stencil, pixelIndex, stencil.fail);
						continue;
					}
					if(depthTest && invz < depths[pixelIndex]){
						applyStencilOperation(stencil, pixelIndex, stencil.depthFail);
						continue;
					}
				}
				//- rouz edit (ChatGPT)
				applyStencilOperation(stencil, pixelIndex, stencil.pass);
				//- rouz edit (ChatGPT)
				float source[3] = {
					(weightA*a.color.red+weightB*b.color.red+weightC*c.color.red)*texel.red*secondaryTexel.red/(255.0f*255.0f*255.0f),
					(weightA*a.color.green+weightB*b.color.green+weightC*c.color.green)*texel.green*secondaryTexel.green/(255.0f*255.0f*255.0f),
					(weightA*a.color.blue+weightB*b.color.blue+weightC*c.color.blue)*texel.blue*secondaryTexel.blue/(255.0f*255.0f*255.0f)
				};
				//- rouz edit (ChatGPT)
				// Apply the current camera fog before blending the effect into the scene
				//+ rouz edit (ChatGPT)
				applyFog(source, 1.0f/invz, fog);
				//- rouz edit (ChatGPT)
				rw::RGBA &destination = pixels[pixelIndex];
				rw::uint8 *channels[3] = { &destination.red, &destination.green, &destination.blue };
				for(int channel = 0; channel < 3; channel++){
					const float target = *channels[channel]*(1.f/255.f);
					const float mixed = source[channel]*effectBlendFactor(sourceBlend, source[channel], target, alpha)+
						target*effectBlendFactor(destinationBlend, source[channel], target, alpha);
					*channels[channel] = (rw::uint8)(255.0f*std::max(0.0f, std::min(1.0f, mixed)));
				}
				// Preserve the GS threshold when immediate effect triangles write depth
				//+ rouz edit (ChatGPT)
				if(depthWrite && (!alphaTest.gsEmulation || alpha >= alphaTest.gsReference*(1.f/255.f)))
					depths[pixelIndex] = invz;
				//- rouz edit (ChatGPT)
				framebuffer.coveredPixels++;
				framebuffer.totalCoveredPixels++;
			}
		}
	//- rouz edit (ChatGPT)
}

void drawImmediate2DPoint(const ScreenVertex &vertex, const TextureSampler *sampler,
	rw::uint32 sourceBlend, rw::uint32 destinationBlend, bool depthTest, bool depthWrite,
	const FogState &fog, const StencilState &stencil, const AlphaTestState &alphaTest)
{
	// Blend one screen-space point into the CPU framebuffer
	// Reject invalid coordinates and points outside the framebuffer
	if(!std::isfinite(vertex.x) || !std::isfinite(vertex.y) || !std::isfinite(vertex.z))
		return;
	if(vertex.x < 0.0f || vertex.x >= width || vertex.y < 0.0f || vertex.y >= height || vertex.z <= 0.0f)
		return;
	const int x = (int)std::floor(vertex.x);
	const int y = (int)std::floor(vertex.y);
	const float inverseZ = 1.0f/vertex.z;
	const size_t index = (size_t)y*width+x;
	// Keep early depth and stencil failures out of point texture sampling
	//+ rouz edit (ChatGPT)
	const bool lateDepthStencil = alphaTest.enabled && !alphaTest.gsEmulation;
	if(!lateDepthStencil){
		if(stencil.enabled && !stencilTestPasses(stencil, stencils[index])){
			applyStencilOperation(stencil, index, stencil.fail);
			return;
		}
		if(depthTest && inverseZ < depths[index]){
			applyStencilOperation(stencil, index, stencil.depthFail);
			return;
		}
	}
	//- rouz edit (ChatGPT)
	rw::RGBA texel = { 255, 255, 255, 255 };
	if(sampler && !sampleTexture(*sampler, vertex.u, vertex.v, texel))
		return;
	const float alpha = vertex.color.alpha*texel.alpha/(255.0f*255.0f);
	// Discard transparent points that fail the bound alpha test
	//+ rouz edit (ChatGPT)
	if(alpha <= 0.0f || !immediateAlphaTestPasses(alphaTest, alpha))
		return;
	//- rouz edit (ChatGPT)
	// Run deferred point tests after alpha rejection, matching triangle effects
	//+ rouz edit (ChatGPT)
	if(lateDepthStencil){
		if(stencil.enabled && !stencilTestPasses(stencil, stencils[index])){
			applyStencilOperation(stencil, index, stencil.fail);
			return;
		}
		if(depthTest && inverseZ < depths[index]){
			applyStencilOperation(stencil, index, stencil.depthFail);
			return;
		}
	}
	applyStencilOperation(stencil, index, stencil.pass);
	//- rouz edit (ChatGPT)
	float source[3] = {
		vertex.color.red*texel.red/(255.0f*255.0f),
		vertex.color.green*texel.green/(255.0f*255.0f),
		vertex.color.blue*texel.blue/(255.0f*255.0f)
	};
	// Apply the current camera fog before blending the point into the scene
	//+ rouz edit (ChatGPT)
	applyFog(source, vertex.z, fog);
	//- rouz edit (ChatGPT)
	rw::RGBA &destination = pixels[index];
	rw::uint8 *channels[3] = { &destination.red, &destination.green, &destination.blue };
	for(int channel = 0; channel < 3; channel++){
		const float target = *channels[channel]*(1.f/255.f);
		const float mixed = source[channel]*effectBlendFactor(sourceBlend, source[channel], target, alpha)+
			target*effectBlendFactor(destinationBlend, source[channel], target, alpha);
		*channels[channel] = (rw::uint8)(255.0f*std::max(0.0f, std::min(1.0f, mixed)));
	}
	// Preserve the GS alpha threshold before writing point depth
	//+ rouz edit (ChatGPT)
	if(depthWrite && (!alphaTest.gsEmulation || alpha >= alphaTest.gsReference*(1.f/255.f)))
		depths[index] = inverseZ;
	//- rouz edit (ChatGPT)
	framebuffer.coveredPixels++;
	framebuffer.totalCoveredPixels++;
}

void drawImmediate2DLine(const ScreenVertex &a, const ScreenVertex &b,
	const TextureSampler *sampler, rw::uint32 sourceBlend, rw::uint32 destinationBlend,
	bool depthTest, bool depthWrite, const FogState &fog,
	const StencilState &stencil, const AlphaTestState &alphaTest)
{
	// Clip screen-space effect lines to the framebuffer before stepping
	if(width <= 0 || height <= 0 || !std::isfinite(a.x) || !std::isfinite(a.y) ||
	   !std::isfinite(b.x) || !std::isfinite(b.y) || !std::isfinite(a.z) ||
	   !std::isfinite(b.z) || a.z <= 0.0f || b.z <= 0.0f)
		return;
	const float dx = b.x-a.x;
	const float dy = b.y-a.y;
	// Reject overflowing screen deltas before clipping the segment
	if(!std::isfinite(dx) || !std::isfinite(dy))
		return;
	float enter = 0.0f, leave = 1.0f;
	const float clipP[4] = { -dx, dx, -dy, dy };
	const float clipQ[4] = { a.x, (float)(width-1)-a.x, a.y, (float)(height-1)-a.y };
	// Apply each framebuffer edge to the visible interval along the segment
	for(int edgeIndex = 0; edgeIndex < 4; edgeIndex++){
		if(clipP[edgeIndex] == 0.0f){
			if(clipQ[edgeIndex] < 0.0f)
				return;
		}else{
			const float intersection = clipQ[edgeIndex]/clipP[edgeIndex];
			if(!std::isfinite(intersection))
				return;
			if(clipP[edgeIndex] < 0.0f)
				enter = std::max(enter, intersection);
			else
				leave = std::min(leave, intersection);
			if(enter > leave)
				return;
		}
	}
	const float clippedSpan = std::max(std::fabs(dx*(leave-enter)), std::fabs(dy*(leave-enter)));
	// Bound work after clipping in case malformed coordinates overflowed the interval
	if(!std::isfinite(clippedSpan) || clippedSpan <= 0.0f || clippedSpan > 4.0f*(width+height))
		return;
	const int steps = std::max(1, (int)std::ceil(clippedSpan));
	const float inverseA = 1.0f/a.z;
	const float inverseB = 1.0f/b.z;
	// Reject invalid reciprocal depths before interpolating fragment attributes
	if(!std::isfinite(inverseA) || !std::isfinite(inverseB))
		return;
	// Select a mip level from the perspective-correct line texture footprint
	//+ rouz edit (ChatGPT)
	TextureSampler mipSampler = {};
	const TextureSampler *drawSampler = sampler;
	if(sampler && sampler->mipmapped){
		mipSampler = *sampler;
		mipSampler.lod = estimateImmediateLineTextureLod(a, b, sampler, std::sqrt(dx*dx+dy*dy));
		drawSampler = &mipSampler;
	}
	//- rouz edit (ChatGPT)
	// Delay line depth and stencil tests until texture alpha is available when needed
	//+ rouz edit (ChatGPT)
	const bool lateDepthStencil = alphaTest.enabled && !alphaTest.gsEmulation;
	//- rouz edit (ChatGPT)
	// Step across the line and apply its texture, depth, and blend state
	for(int step = 0; step <= steps; step++){
		const float t = (float)step/steps;
		const float segmentT = enter+t*(leave-enter);
		const int x = (int)std::floor(a.x+segmentT*dx);
		const int y = (int)std::floor(a.y+segmentT*dy);
		if(x < 0 || x >= width || y < 0 || y >= height)
			continue;
		const float inverseZ = (1.0f-segmentT)*inverseA+segmentT*inverseB;
		const size_t index = (size_t)y*width+x;
		if(inverseZ <= 0.0f)
			continue;
		// Keep early line depth and stencil failures out of texture sampling
		//+ rouz edit (ChatGPT)
		if(!lateDepthStencil){
			if(stencil.enabled && !stencilTestPasses(stencil, stencils[index])){
				applyStencilOperation(stencil, index, stencil.fail);
				continue;
			}
			if(depthTest && inverseZ < depths[index]){
				applyStencilOperation(stencil, index, stencil.depthFail);
				continue;
			}
		}
		//- rouz edit (ChatGPT)
		const float weightA = (1.0f-segmentT)*inverseA/inverseZ;
		const float weightB = segmentT*inverseB/inverseZ;
		rw::RGBA texel = { 255, 255, 255, 255 };
		if(drawSampler && !sampleTexture(*drawSampler,
		    weightA*a.u+weightB*b.u, weightA*a.v+weightB*b.v, texel))
			continue;
		const float alpha = (weightA*a.color.alpha+weightB*b.color.alpha)*texel.alpha/(255.0f*255.0f);
		// Discard transparent lines that fail the bound alpha test
		//+ rouz edit (ChatGPT)
		if(alpha <= 0.0f || !immediateAlphaTestPasses(alphaTest, alpha))
			continue;
		//- rouz edit (ChatGPT)
		// Run deferred line tests after alpha rejection, matching triangle effects
		//+ rouz edit (ChatGPT)
		if(lateDepthStencil){
			if(stencil.enabled && !stencilTestPasses(stencil, stencils[index])){
				applyStencilOperation(stencil, index, stencil.fail);
				continue;
			}
			if(depthTest && inverseZ < depths[index]){
				applyStencilOperation(stencil, index, stencil.depthFail);
				continue;
			}
		}
		applyStencilOperation(stencil, index, stencil.pass);
		//- rouz edit (ChatGPT)
		float source[3] = {
			(weightA*a.color.red+weightB*b.color.red)*texel.red/(255.0f*255.0f),
			(weightA*a.color.green+weightB*b.color.green)*texel.green/(255.0f*255.0f),
			(weightA*a.color.blue+weightB*b.color.blue)*texel.blue/(255.0f*255.0f)
		};
		// Apply the current camera fog before blending the line into the scene
		//+ rouz edit (ChatGPT)
		applyFog(source, 1.0f/inverseZ, fog);
		//- rouz edit (ChatGPT)
		rw::RGBA &destination = pixels[index];
		rw::uint8 *channels[3] = { &destination.red, &destination.green, &destination.blue };
		for(int channel = 0; channel < 3; channel++){
			const float target = *channels[channel]*(1.f/255.f);
			const float mixed = source[channel]*effectBlendFactor(sourceBlend, source[channel], target, alpha)+
				target*effectBlendFactor(destinationBlend, source[channel], target, alpha);
			*channels[channel] = (rw::uint8)(255.0f*std::max(0.0f, std::min(1.0f, mixed)));
		}
		// Preserve the GS alpha threshold before writing line depth
		//+ rouz edit (ChatGPT)
		if(depthWrite && (!alphaTest.gsEmulation || alpha >= alphaTest.gsReference*(1.f/255.f)))
			depths[index] = inverseZ;
		//- rouz edit (ChatGPT)
		framebuffer.coveredPixels++;
		framebuffer.totalCoveredPixels++;
	}
}

void drawImmediateLine(ScreenVertex a, ScreenVertex b, const TextureSampler *sampler, rw::uint32 sourceBlend,
	rw::uint32 destinationBlend, bool depthTest, bool depthWrite, const FogState &fog,
	const StencilState &stencil, const AlphaTestState &alphaTest,
	float nearPlane, float farPlane, bool perspective)
{
	// Clip world lines to the valid camera depth range before projecting them
	if(!std::isfinite(a.z) || !std::isfinite(b.z) || !std::isfinite(nearPlane) ||
	   !std::isfinite(farPlane) || nearPlane <= 0.0f || farPlane <= nearPlane)
		return;
	if(a.z < nearPlane && b.z < nearPlane || a.z > farPlane && b.z > farPlane)
		return;
	// Interpolate clipped endpoints while preserving their vertex colors
	auto interpolate = [](const ScreenVertex &from, const ScreenVertex &to, float t){
		ScreenVertex v = from;
		v.cameraX += t*(to.cameraX-from.cameraX);
		v.cameraY += t*(to.cameraY-from.cameraY);
		v.z += t*(to.z-from.z);
		v.u += t*(to.u-from.u);
		v.v += t*(to.v-from.v);
		v.color.red = (rw::uint8)(from.color.red+t*((float)to.color.red-from.color.red));
		v.color.green = (rw::uint8)(from.color.green+t*((float)to.color.green-from.color.green));
		v.color.blue = (rw::uint8)(from.color.blue+t*((float)to.color.blue-from.color.blue));
		v.color.alpha = (rw::uint8)(from.color.alpha+t*((float)to.color.alpha-from.color.alpha));
		return v;
	};
	// Move clipped endpoints onto the near and far planes while preserving color
	if(a.z < nearPlane){ float t=(nearPlane-a.z)/(b.z-a.z); a=interpolate(a,b,t); }
	if(b.z < nearPlane){ float t=(nearPlane-b.z)/(a.z-b.z); b=interpolate(b,a,t); }
	if(a.z > farPlane){ float t=(farPlane-a.z)/(b.z-a.z); a=interpolate(a,b,t); }
	if(b.z > farPlane){ float t=(farPlane-b.z)/(a.z-b.z); b=interpolate(b,a,t); }
	const float divisorA = perspective ? a.z : 1.0f;
	const float divisorB = perspective ? b.z : 1.0f;
	// Map clipped world endpoints to the CPU framebuffer's row origin
	a.x = width*a.cameraX/divisorA; a.y = height-height*a.cameraY/divisorA;
	b.x = width*b.cameraX/divisorB; b.y = height-height*b.cameraY/divisorB;
	// Submit the projected world line through the viewport-clipped rasterizer
	drawImmediate2DLine(a, b, sampler, sourceBlend, destinationBlend,
		depthTest, depthWrite, fog, stencil, alphaTest);
}

struct NormalTransform {
	rw::V3d right, up, at;
	float inverseDeterminant;
	bool valid;
};

static NormalTransform makeNormalTransform(const rw::Matrix *matrix)
{
	// Precompute an inverse-transpose basis for one object or bone matrix
	const rw::V3d &right = matrix->right;
	const rw::V3d &up = matrix->up;
	const rw::V3d &at = matrix->at;
	NormalTransform transform = {};
	transform.right = {
		up.y*at.z-up.z*at.y,
		up.z*at.x-up.x*at.z,
		up.x*at.y-up.y*at.x
	};
	transform.up = {
		at.y*right.z-at.z*right.y,
		at.z*right.x-at.x*right.z,
		at.x*right.y-at.y*right.x
	};
	transform.at = {
		right.y*up.z-right.z*up.y,
		right.z*up.x-right.x*up.z,
		right.x*up.y-right.y*up.x
	};
	const float determinant = right.x*transform.right.x+
		right.y*transform.right.y+right.z*transform.right.z;
	if(std::isfinite(determinant) && std::fabs(determinant) > 0.00000001f){
		transform.inverseDeterminant = 1.0f/determinant;
		transform.valid = true;
	}
	return transform;
}

static void transformNormal(rw::V3d *output, const rw::V3d *normal,
	const NormalTransform &transform, const rw::Matrix *fallbackMatrix)
{
	// Apply the cached inverse transpose and retain a fallback for singular transforms
	if(transform.valid){
		output->x = (transform.right.x*normal->x+transform.up.x*normal->y+
			transform.at.x*normal->z)*transform.inverseDeterminant;
		output->y = (transform.right.y*normal->x+transform.up.y*normal->y+
			transform.at.y*normal->z)*transform.inverseDeterminant;
		output->z = (transform.right.z*normal->x+transform.up.z*normal->y+
			transform.at.z*normal->z)*transform.inverseDeterminant;
	}else
		rw::V3d::transformVectors(output, normal, 1, fallbackMatrix);
}

void renderAtomic(rw::ObjPipeline *, rw::Atomic *atomic)
{
	// Count every pipeline entry before checking whether CPU geometry is available
	framebuffer.pipelineCalls++;
	framebuffer.totalPipelineCalls++;
	// Skip the geometry stage when its runtime option is disabled
	if(!renderGeometryStage.load(std::memory_order_relaxed))
		return;
	// Read optional material and transparency settings once per atomic
	//+ rouz edit (ChatGPT)
	const bool deferTransparency = deferTransparentStage.load(std::memory_order_relaxed) != 0;
	const bool advancedMaterials = renderAdvancedMaterialsStage.load(std::memory_order_relaxed) != 0;
	const bool dynamicLights = renderDynamicLightsStage.load(std::memory_order_relaxed) != 0;
	//- rouz edit (ChatGPT)
	// Skip geometry without CPU vertices and cameras outside the scene pass
	rw::Camera *camera = rw::engine->currentCamera;
	rw::Geometry *geometry = atomic->geometry;
	if(!camera || !geometry || !geometry->triangles || !geometry->morphTargets ||
	   !geometry->morphTargets[0].vertices || width == 0 || height == 0){
		framebuffer.missingCpuGeometry++;
		return;
	}
	// Count atomics that reach the CPU rasterizer
	framebuffer.submittedAtomics++;
	//+ rouz edit (ChatGPT)
	// Collect the same world lights that librw sends to its OpenGL vertex shader
	struct CpuLightState {
		int type;
		rw::V3d position, direction;
		rw::RGBAf color;
		float radius, minusCosAngle;
	};
	rw::RGBAf ambient = {};
	// Keep separate directional and local light capacity from the RenderWare light lists
	//+ rouz edit (ChatGPT)
	const int maxCpuLights = 16;
	CpuLightState cpuLights[maxCpuLights] = {};
	//- rouz edit (ChatGPT)
	int cpuLightCount = 0;
	if(dynamicLights && (geometry->flags & rw::Geometry::LIGHT) && rw::engine->currentWorld){
		rw::WorldLights worldLights = {};
		rw::Light *directionalLights[8] = {};
		rw::Light *localLights[8] = {};
		worldLights.directionals = directionalLights;
		worldLights.numDirectionals = 8;
		worldLights.locals = localLights;
		worldLights.numLocals = 8;
		rw::engine->currentWorld->enumerateLights(atomic, &worldLights);
		ambient = worldLights.ambient;
		for(int i = 0; i < worldLights.numDirectionals && cpuLightCount < maxCpuLights; i++){
			rw::Light *light = worldLights.directionals[i];
			if(!light || !light->getFrame())
				continue;
			const rw::Matrix *lightMatrix = light->getFrame()->getLTM();
			CpuLightState &state = cpuLights[cpuLightCount++];
			state.type = light->getType();
			state.direction = lightMatrix->at;
			state.color = light->color;
		}
		for(int i = 0; i < worldLights.numLocals && cpuLightCount < maxCpuLights; i++){
			rw::Light *light = worldLights.locals[i];
			if(!light || !light->getFrame())
				continue;
			const rw::Matrix *lightMatrix = light->getFrame()->getLTM();
			CpuLightState &state = cpuLights[cpuLightCount++];
			state.type = light->getType();
			state.position = lightMatrix->pos;
			state.direction = lightMatrix->at;
			state.color = light->color;
			state.radius = light->radius;
			state.minusCosAngle = light->minusCosAngle;
		}
	}else if((geometry->flags & rw::Geometry::LIGHT) && pAmbient &&
		(pAmbient->getFlags() & rw::Light::LIGHTATOMICS))
		ambient = pAmbient->color;
	const bool hasVertexNormals = (geometry->flags & rw::Geometry::NORMALS) &&
		geometry->morphTargets[0].normals != nullptr;
	// Skip world normal transforms when neither lighting nor advanced materials use them
	const bool needsVertexNormals = hasVertexNormals && (dynamicLights || advancedMaterials);
	const bool dynamicLighting = hasVertexNormals && cpuLightCount > 0;
	//- rouz edit (ChatGPT)
	// Capture the caller's cull mode once for every triangle in this atomic
	const rw::uint32 cullMode = rw::GetRenderState(rw::CULLMODE);
	// Capture the GL alpha-test function and reference for this atomic
	//+ rouz edit (ChatGPT)
	const AlphaTestState alphaTest = getAlphaTestState();
	//- rouz edit (ChatGPT)
	// Capture the caller's fog state once for every triangle in this atomic
	//+ rouz edit (ChatGPT)
	const FogState fog = getFogState(camera);
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Capture stencil state once for all triangles in this atomic
	const StencilState stencil = getStencilState();
	//- rouz edit (ChatGPT)
	// Capture alpha blending and depth writing once for all triangles in this atomic
	//+ rouz edit (ChatGPT)
	const bool vertexAlphaEnabled = rw::GetRenderState(rw::VERTEXALPHA) != 0;
	const bool depthWrite = rw::GetRenderState(rw::ZWRITEENABLE) != 0;
	rw::uint32 sourceBlend = rw::GetRenderState(rw::SRCBLEND);
	const rw::uint32 destinationBlend = rw::GetRenderState(rw::DESTBLEND);
	const float alphaMultiplier = atomicAlphaMultiplier;
	//- rouz edit (ChatGPT)
	// Activate registered road gloss and ped rim lighting when their custom pipes are enabled
	//+ rouz edit (ChatGPT)
	// Skip expensive optional material setup while retaining base textures and UV transforms
	//+ rouz edit (ChatGPT)
	bool glossAtomic = advancedMaterials && glossAtomics.find(atomic) != glossAtomics.end();
	bool rimlightAtomic = advancedMaterials && rimlightAtomics.find(atomic) != rimlightAtomics.end();
	bool vehicleAtomic = advancedMaterials && vehicleAtomics.find(atomic) != vehicleAtomics.end();
	//- rouz edit (ChatGPT)
	// Track vehicle materials that need recursion protection in environment maps
	bool reflectedVehicleAtomic = false;
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
	// Avoid sampling the environment texture while rendering that texture
	reflectedVehicleAtomic = vehicleAtomic && CustomPipes::VehiclePipeSwitch == CustomPipes::VEHICLEPIPE_NEO &&
		CustomPipes::bRenderingEnvMap;
	if(reflectedVehicleAtomic)
		vehicleAtomic = false;
	rw::V3d glossLightDirection = { 0.0f, 0.0f, 0.0f };
	rw::V3d glossEyePosition = { 0.0f, 0.0f, 0.0f };
	rw::V3d rimViewDirection = { 0.0f, 0.0f, 0.0f };
	float rimOffset = 0.0f, rimScale = 0.0f, rimIntensity = 0.0f;
	float rimRampStart[3] = {}, rimRampEnd[3] = {};
	rw::V3d vehicleEyePosition = { 0.0f, 0.0f, 0.0f };
	float vehiclePower = 0.0f, vehicleFresnel = 0.0f, vehicleSpecularity = 0.0f;
	float vehicleLightStrength = 0.0f, vehicleSpecColor[3] = {};
	rw::V3d vehicleSpecDirections[5] = {};
	rw::RGBAf vehicleSpecColors[5] = {};
	float vehicleSpecPowers[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	if(glossAtomic && CustomPipes::GlossEnable && pDirect && pDirect->getFrame() && camera->getFrame()){
		glossLightDirection = pDirect->getFrame()->getLTM()->at;
		glossEyePosition = camera->getFrame()->getLTM()->pos;
	}else
		glossAtomic = false;
	if(rimlightAtomic && CustomPipes::RimlightEnable && camera->getFrame()){
		const CustomPipes::Color start = CustomPipes::RampStart.Get();
		const CustomPipes::Color end = CustomPipes::RampEnd.Get();
		rimViewDirection = camera->getFrame()->getLTM()->at;
		rimOffset = CustomPipes::Offset.Get();
		rimScale = CustomPipes::Scale.Get();
		rimIntensity = CustomPipes::Scaling.Get()*CustomPipes::RimlightMult;
		rimRampStart[0] = start.r; rimRampStart[1] = start.g; rimRampStart[2] = start.b;
		rimRampEnd[0] = end.r; rimRampEnd[1] = end.g; rimRampEnd[2] = end.b;
	}else
		rimlightAtomic = false;
	if(vehicleAtomic && CustomPipes::VehiclePipeSwitch == CustomPipes::VEHICLEPIPE_NEO &&
	   camera->getFrame()){
		vehicleEyePosition = camera->getFrame()->getLTM()->pos;
		vehiclePower = CustomPipes::Power.Get();
		vehicleFresnel = CustomPipes::Fresnel.Get();
		vehicleSpecularity = CustomPipes::VehicleSpecularity;
		const CustomPipes::Color specular = CustomPipes::SpecColor.Get();
		vehicleLightStrength = specular.a;
		vehicleSpecColor[0] = specular.r;
		vehicleSpecColor[1] = specular.g;
		vehicleSpecColor[2] = specular.b;
		if(pDirect && pDirect->getFrame()){
			vehicleSpecDirections[0] = pDirect->getFrame()->getLTM()->at;
			vehicleSpecColors[0].red = specular.r;
			vehicleSpecColors[0].green = specular.g;
			vehicleSpecColors[0].blue = specular.b;
			vehicleSpecPowers[0] = vehiclePower;
		}else
			vehicleAtomic = false;
		for(int light = 0; vehicleAtomic && light < 4; light++){
			rw::Light *extra = pExtraDirectionals[light];
			if(!extra || !(extra->getFlags() & rw::Light::LIGHTATOMICS) || !extra->getFrame())
				continue;
			vehicleSpecDirections[light+1] = extra->getFrame()->getLTM()->at;
			vehicleSpecColors[light+1] = extra->color;
			vehicleSpecPowers[light+1] = vehiclePower*2.0f;
		}
	}else
		vehicleAtomic = false;
	if(vehicleAtomic)
		sourceBlend = rw::BLENDONE;
#else
	glossAtomic = false;
	rimlightAtomic = false;
	vehicleAtomic = false;
#endif
	//- rouz edit (ChatGPT)

	// Project vertices with RenderWare's world to screen matrix used by camera culling
	std::vector<ScreenVertex> transformed((size_t)geometry->numVertices);
	const rw::Matrix *model = atomic->getFrame()->getLTM();
	// Combine the model and camera transforms once for this atomic
	//+ rouz edit (ChatGPT)
	rw::Matrix modelView;
	rw::Matrix::mult(&modelView, model, &camera->viewMatrix);
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Cache the object normal transform only when this geometry has normals
	NormalTransform modelNormalTransform = {};
	if(needsVertexNormals)
		modelNormalTransform = makeNormalTransform(model);
	//- rouz edit (ChatGPT)
	// Build the same skin matrices as the GL pipeline for animated clump atomics
	rw::Skin *skin = rw::Skin::get(geometry);
	rw::HAnimHierarchy *hierarchy = skin ? rw::Skin::getHierarchy(atomic) : nullptr;
	std::vector<rw::Matrix> boneMatrices;
	//+ rouz edit (ChatGPT)
	// Keep a matching cached normal transform for every skin matrix
	std::vector<NormalTransform> boneNormalTransforms;
	//- rouz edit (ChatGPT)
	if(skin && hierarchy && hierarchy->matrices && skin->numBones == hierarchy->numNodes){
		boneMatrices.resize((size_t)skin->numBones);
		//+ rouz edit (ChatGPT)
		// Allocate cached bone transforms only when animated normals need them
		if(needsVertexNormals)
			boneNormalTransforms.resize((size_t)skin->numBones);
		//- rouz edit (ChatGPT)
		const rw::Matrix *inverseMatrices = (const rw::Matrix*)skin->inverseMatrices;
		rw::Matrix inverseModel;
		if(!(hierarchy->flags & rw::HAnimHierarchy::LOCALSPACEMATRICES))
			rw::Matrix::invert(&inverseModel, model);
		for(int bone = 0; bone < skin->numBones; bone++){
			rw::Matrix inverseBone = inverseMatrices[bone];
			inverseBone.flags = 0;
			if(hierarchy->flags & rw::HAnimHierarchy::LOCALSPACEMATRICES)
				rw::Matrix::mult(&boneMatrices[bone], &inverseBone, &hierarchy->matrices[bone]);
			else{
				rw::Matrix animatedBone;
				rw::Matrix::mult(&animatedBone, &hierarchy->matrices[bone], &inverseModel);
				rw::Matrix::mult(&boneMatrices[bone], &inverseBone, &animatedBone);
			}
			//+ rouz edit (ChatGPT)
			// Precompute this bone's inverse transpose only when normals need it
			if(needsVertexNormals)
				boneNormalTransforms[bone] = makeNormalTransform(&boneMatrices[bone]);
			//- rouz edit (ChatGPT)
		}
	}
	for(int i = 0; i < geometry->numVertices; i++){
		rw::V3d projected;
		// Blend each bind pose vertex by its animated bone matrices
		const rw::V3d &bindVertex = geometry->morphTargets[0].vertices[i];
		rw::V3d local = bindVertex;
		if(!boneMatrices.empty() && skin->weights && skin->indices){
			rw::V3d blended = { 0.0f, 0.0f, 0.0f };
			float totalWeight = 0.0f;
			for(int influence = 0; influence < 4; influence++){
				const float weight = skin->weights[(size_t)i*4+influence];
				const rw::uint8 bone = skin->indices[(size_t)i*4+influence];
				if(weight <= 0.0f || bone >= boneMatrices.size())
					continue;
				rw::V3d posed;
				rw::V3d::transformPoints(&posed, &bindVertex, 1, &boneMatrices[bone]);
				blended.x += posed.x*weight;
				blended.y += posed.y*weight;
				blended.z += posed.z*weight;
				totalWeight += weight;
			}
			if(totalWeight > 0.0f)
				local = blended;
		}
		// Project each posed vertex with the combined matrix
		rw::V3d::transformPoints(&projected, &local, 1, &modelView);
		// Capture the first vertex and projection inputs for debugger inspection
		if(framebuffer.submittedAtomics == 1 && i == 0){
			//+ rouz edit (ChatGPT)
			rw::V3d world;
			rw::V3d::transformPoints(&world, &local, 1, model);
			const rw::V3d &local = geometry->morphTargets[0].vertices[i];
			framebuffer.firstLocal[0] = local.x; framebuffer.firstLocal[1] = local.y; framebuffer.firstLocal[2] = local.z;
			framebuffer.firstWorld[0] = world.x; framebuffer.firstWorld[1] = world.y; framebuffer.firstWorld[2] = world.z;
			framebuffer.firstProjected[0] = projected.x; framebuffer.firstProjected[1] = projected.y; framebuffer.firstProjected[2] = projected.z;
			framebuffer.cameraViewWindow[0] = camera->viewWindow.x; framebuffer.cameraViewWindow[1] = camera->viewWindow.y;
			framebuffer.cameraViewOffset[0] = camera->viewOffset.x; framebuffer.cameraViewOffset[1] = camera->viewOffset.y;
			framebuffer.viewMatrixX[0] = camera->viewMatrix.right.x; framebuffer.viewMatrixX[1] = camera->viewMatrix.up.x;
			framebuffer.viewMatrixX[2] = camera->viewMatrix.at.x; framebuffer.viewMatrixX[3] = camera->viewMatrix.pos.x;
			framebuffer.viewMatrixY[0] = camera->viewMatrix.right.y; framebuffer.viewMatrixY[1] = camera->viewMatrix.up.y;
			framebuffer.viewMatrixY[2] = camera->viewMatrix.at.y; framebuffer.viewMatrixY[3] = camera->viewMatrix.pos.y;
			//- rouz edit (ChatGPT)
		}
		transformed[i].z = projected.z;
		// Preserve projected camera coordinates for near and far plane clipping
		transformed[i].cameraX = projected.x;
		transformed[i].cameraY = projected.y;
		// Preserve the first UV set for perspective corrected material sampling
		if(geometry->numTexCoordSets > 0 && geometry->texCoords[0]){
			transformed[i].u = geometry->texCoords[0][i].u;
			transformed[i].v = geometry->texCoords[0][i].v;
		}else
			transformed[i].u = transformed[i].v = 0.0f;
		// Preserve the second UV set used by the custom world lightmap pass
		//+ rouz edit (ChatGPT)
		if(geometry->numTexCoordSets > 1 && geometry->texCoords[1]){
			transformed[i].dualU = geometry->texCoords[1][i].u;
			transformed[i].dualV = geometry->texCoords[1][i].v;
		}else
			transformed[i].dualU = transformed[i].dualV = 0.0f;
		//- rouz edit (ChatGPT)
		// Preserve each vertex's prelight color for smooth face shading
		transformed[i].color = geometry->colors ? geometry->colors[i] : rw::RGBA{ 255, 255, 255, 255 };
		// Start the wet-road highlight at zero before its optional vertex calculation
		transformed[i].glossLightZ = 0.0f;
		// Start ped rim lighting at zero before its optional normal calculation
		transformed[i].rimRed = transformed[i].rimGreen = transformed[i].rimBlue = 0.0f;
		// Start Neo vehicle reflection and specular inputs at zero for ordinary atomics
		transformed[i].vehicleEnvU = transformed[i].vehicleEnvV = 0.0f;
		transformed[i].vehicleReflectBase = 0.0f;
		transformed[i].vehicleSpecRed = transformed[i].vehicleSpecGreen = transformed[i].vehicleSpecBlue = 0.0f;
		//+ rouz edit (ChatGPT)
		// Transform normals for environment maps and evaluate active world lights
		if(needsVertexNormals){
			const rw::V3d &bindNormal = geometry->morphTargets[0].normals[i];
			rw::V3d localNormal = bindNormal;
			if(!boneMatrices.empty() && skin->weights && skin->indices){
				rw::V3d blendedNormal = { 0.0f, 0.0f, 0.0f };
				float totalWeight = 0.0f;
				for(int influence = 0; influence < 4; influence++){
					const float weight = skin->weights[(size_t)i*4+influence];
					const rw::uint8 bone = skin->indices[(size_t)i*4+influence];
					if(weight <= 0.0f || bone >= boneMatrices.size())
						continue;
					rw::V3d posedNormal;
					transformNormal(&posedNormal, &bindNormal, boneNormalTransforms[bone], &boneMatrices[bone]);
					blendedNormal.x += posedNormal.x*weight;
					blendedNormal.y += posedNormal.y*weight;
					blendedNormal.z += posedNormal.z*weight;
					totalWeight += weight;
				}
				if(totalWeight > 0.0f)
					localNormal = blendedNormal;
			}
			rw::V3d worldNormal;
			// Transform and normalize normals correctly for scaled atomics
			transformNormal(&worldNormal, &localNormal, modelNormalTransform, model);
			// Normalize transformed normals before diffuse, reflection, and rim calculations
			const float worldNormalLengthSquared = worldNormal.x*worldNormal.x+
				worldNormal.y*worldNormal.y+worldNormal.z*worldNormal.z;
			if(std::isfinite(worldNormalLengthSquared) && worldNormalLengthSquared > 0.00000001f){
				const float inverseWorldNormalLength = 1.0f/std::sqrt(worldNormalLengthSquared);
				worldNormal.x *= inverseWorldNormalLength;
				worldNormal.y *= inverseWorldNormalLength;
				worldNormal.z *= inverseWorldNormalLength;
			}
			//- rouz edit (ChatGPT)
			// Retain the world normal until the triangle's environment frame is known
			transformed[i].normalX = worldNormal.x;
			transformed[i].normalY = worldNormal.y;
			transformed[i].normalZ = worldNormal.z;
			// Reproduce the Neo vehicle environment coordinates and directional specular lights
			//+ rouz edit (ChatGPT)
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
			if(vehicleAtomic){
				rw::V3d worldPosition;
				rw::V3d::transformPoints(&worldPosition, &local, 1, model);
				const float viewX = vehicleEyePosition.x-worldPosition.x;
				const float viewY = vehicleEyePosition.y-worldPosition.y;
				const float viewZ = vehicleEyePosition.z-worldPosition.z;
				const float viewLength = std::sqrt(viewX*viewX+viewY*viewY+viewZ*viewZ);
				if(viewLength > 0.0f){
					const float viewDirX = viewX/viewLength;
					const float viewDirY = viewY/viewLength;
					const float viewDirZ = viewZ/viewLength;
					const float normalView = worldNormal.x*viewDirX+
						worldNormal.y*viewDirY+worldNormal.z*viewDirZ;
					const float reflectedX = worldNormal.x*normalView*2.0f-viewDirX;
					const float reflectedY = worldNormal.y*normalView*2.0f-viewDirY;
					const float edge = 1.0f-std::max(0.0f, std::min(1.0f, normalView));
				const float edge2 = edge*edge;
				transformed[i].vehicleEnvU = reflectedX*0.5f+0.5f;
				transformed[i].vehicleEnvV = reflectedY*0.5f+0.5f;
				transformed[i].vehicleReflectBase = edge2*edge2*edge;
				for(int light = 0; light < 5; light++){
					const float halfX = viewDirX-vehicleSpecDirections[light].x;
					const float halfY = viewDirY-vehicleSpecDirections[light].y;
					const float halfZ = viewDirZ-vehicleSpecDirections[light].z;
					const float halfLength = std::sqrt(halfX*halfX+halfY*halfY+halfZ*halfZ);
					if(halfLength <= 0.0f)
						continue;
					const float specDot = std::max(0.0f, std::min(1.0f,
						(worldNormal.x*halfX+worldNormal.y*halfY+worldNormal.z*halfZ)/halfLength));
					const float specFactor = std::pow(specDot, vehicleSpecPowers[light]);
					transformed[i].vehicleSpecRed += specFactor*vehicleSpecColors[light].red;
					transformed[i].vehicleSpecGreen += specFactor*vehicleSpecColors[light].green;
					transformed[i].vehicleSpecBlue += specFactor*vehicleSpecColors[light].blue;
				}
			}
		}
#endif
			//- rouz edit (ChatGPT)
			// Reproduce the animated Neo rim ramp on ped vertices
			//+ rouz edit (ChatGPT)
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
			if(rimlightAtomic){
				const float rimEnable = geometry->flags & rw::Geometry::LIGHT ? rimIntensity : 0.0f;
				const float normalView = worldNormal.x*rimViewDirection.x+
					worldNormal.y*rimViewDirection.y+worldNormal.z*rimViewDirection.z;
				const float factor = rimOffset-rimScale*normalView;
				const float ramps[3] = {
					rimRampEnd[0]+(rimRampStart[0]-rimRampEnd[0])*factor,
					rimRampEnd[1]+(rimRampStart[1]-rimRampEnd[1])*factor,
					rimRampEnd[2]+(rimRampStart[2]-rimRampEnd[2])*factor
				};
				transformed[i].rimRed = std::max(0.0f, std::min(1.0f, ramps[0]*rimEnable));
				transformed[i].rimGreen = std::max(0.0f, std::min(1.0f, ramps[1]*rimEnable));
				transformed[i].rimBlue = std::max(0.0f, std::min(1.0f, ramps[2]*rimEnable));
			}
#endif
			//- rouz edit (ChatGPT)
			if(dynamicLighting){
				rw::V3d worldPosition;
				rw::V3d::transformPoints(&worldPosition, &local, 1, model);
				for(int lightIndex = 0; lightIndex < cpuLightCount; lightIndex++){
					const CpuLightState &light = cpuLights[lightIndex];
					float lambert = 0.0f, attenuation = 1.0f;
					if(light.type == rw::Light::DIRECTIONAL){
						lambert = std::max(0.0f, -(worldNormal.x*light.direction.x+
							worldNormal.y*light.direction.y+worldNormal.z*light.direction.z));
					}else if((light.type == rw::Light::POINT || light.type == rw::Light::SPOT ||
						light.type == rw::Light::SOFTSPOT) && light.radius > 0.0f){
						const float dirX = worldPosition.x-light.position.x;
						const float dirY = worldPosition.y-light.position.y;
						const float dirZ = worldPosition.z-light.position.z;
						const float distanceSquared = dirX*dirX+dirY*dirY+dirZ*dirZ;
						const float distance = std::sqrt(distanceSquared);
						if(distance > 0.0f){
							const float inverseDistance = 1.0f/distance;
							const float nx = dirX*inverseDistance;
							const float ny = dirY*inverseDistance;
							const float nz = dirZ*inverseDistance;
							lambert = std::max(0.0f, -(worldNormal.x*nx+worldNormal.y*ny+worldNormal.z*nz));
							attenuation = std::max(0.0f, 1.0f-distance/light.radius);
							if(light.type == rw::Light::SPOT || light.type == rw::Light::SOFTSPOT){
								const float pcos = nx*light.direction.x+ny*light.direction.y+nz*light.direction.z;
								const float ccos = -light.minusCosAngle;
								const float falloffRange = 1.0f-ccos;
								const float falloff = std::fabs(falloffRange) > 0.000001f
									? (pcos-ccos)/falloffRange : (pcos >= ccos ? 1.0f : -1.0f);
								if(falloff < 0.0f)
									lambert = 0.0f;
								lambert *= std::max(falloff, light.type == rw::Light::SPOT ? 1.0f : 0.0f);
							}
						}
					}
					// Accumulate overlapping spotlights with the rest of the active world lights
					//+ rouz edit (ChatGPT)
					transformed[i].lightRed += lambert*attenuation*light.color.red;
					transformed[i].lightGreen += lambert*attenuation*light.color.green;
					transformed[i].lightBlue += lambert*attenuation*light.color.blue;
					//- rouz edit (ChatGPT)
				}
			}
		}
		//- rouz edit (ChatGPT)
		// Match the Neo gloss shader's view and direct-light direction per road vertex
		//+ rouz edit (ChatGPT)
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
		if(glossAtomic){
			rw::V3d worldPosition;
			rw::V3d::transformPoints(&worldPosition, &local, 1, model);
			const float viewX = glossEyePosition.x-worldPosition.x;
			const float viewY = glossEyePosition.y-worldPosition.y;
			const float viewZ = glossEyePosition.z-worldPosition.z;
			const float viewLength = std::sqrt(viewX*viewX+viewY*viewY+viewZ*viewZ);
			if(viewLength > 0.0f){
				const float lightX = viewX/viewLength-glossLightDirection.x;
				const float lightY = viewY/viewLength-glossLightDirection.y;
				const float lightZ = viewZ/viewLength-glossLightDirection.z;
				const float lightLength = std::sqrt(lightX*lightX+lightY*lightY+lightZ*lightZ);
				transformed[i].glossLightZ = lightLength > 0.0f ? lightZ/lightLength : 0.0f;
			}
		}
#endif
		//- rouz edit (ChatGPT)
		if(projected.z > camera->nearPlane){
			// Convert normalized camera coordinates to CPU framebuffer pixels
			const float divisor = camera->projection == rw::Camera::PERSPECTIVE ? projected.z : 1.0f;
			transformed[i].x = width*projected.x/divisor;
			transformed[i].y = height-height*projected.y/divisor;
			// Capture the first framebuffer coordinate after perspective division
			if(framebuffer.submittedAtomics == 1 && i == 0){
				framebuffer.firstScreen[0] = transformed[i].x;
				framebuffer.firstScreen[1] = transformed[i].y;
			}
			// Record the projected extent of vertices in front of the camera
			if(std::isfinite(transformed[i].x) && std::isfinite(transformed[i].y)){
				framebuffer.minScreenX = std::min(framebuffer.minScreenX, transformed[i].x);
				framebuffer.maxScreenX = std::max(framebuffer.maxScreenX, transformed[i].x);
				framebuffer.minScreenY = std::min(framebuffer.minScreenY, transformed[i].y);
				framebuffer.maxScreenY = std::max(framebuffer.maxScreenY, transformed[i].y);
			}
		}
	}

	// Resolve each material's color and decoded texture once per atomic
	//+ rouz edit (ChatGPT)
	std::vector<MaterialState> materialStates((size_t)geometry->matList.numMaterials);
	MaterialState fallback = {};
	fallback.color = { 190, 190, 190, 255 };
	fallback.blendColor = fallback.color;
	fallback.surfaceAmbient = 1.0f;
	fallback.surfaceDiffuse = 1.0f;
	fallback.resolved = true;
	const bool hasTexCoords = geometry->numTexCoordSets > 0 && geometry->texCoords[0];
	//- rouz edit (ChatGPT)
	// Rasterize each source triangle with its material color and texture
	for(int i = 0; i < geometry->numTriangles; i++){
		const rw::Triangle &tri = geometry->triangles[i];
		if(tri.v[0] >= geometry->numVertices || tri.v[1] >= geometry->numVertices || tri.v[2] >= geometry->numVertices)
			continue;
		const ScreenVertex &a = transformed[tri.v[0]];
		const ScreenVertex &b = transformed[tri.v[1]];
		const ScreenVertex &c = transformed[tri.v[2]];
		// Skip clipping when all vertices are already inside both depth planes
		//+ rouz edit (ChatGPT)
		const ScreenVertex sourceVertices[3] = { a, b, c };
		ScreenVertex nearVertices[6], farVertices[6];
		const ScreenVertex *polygon = sourceVertices;
		int polygonCount = 3;
		if(!(a.z > camera->nearPlane && b.z > camera->nearPlane && c.z > camera->nearPlane &&
		     a.z < camera->farPlane && b.z < camera->farPlane && c.z < camera->farPlane)){
			const int nearCount = clipDepthPlane(sourceVertices, 3, nearVertices, camera->nearPlane, true);
			polygonCount = nearCount >= 3
				? clipDepthPlane(nearVertices, nearCount, farVertices, camera->farPlane, false) : 0;
			if(polygonCount < 3){
				framebuffer.depthRejectedTriangles++;
				continue;
			}
			for(int vertex = 0; vertex < polygonCount; vertex++){
				const float divisor = camera->projection == rw::Camera::PERSPECTIVE ? farVertices[vertex].z : 1.0f;
				farVertices[vertex].x = width*farVertices[vertex].cameraX/divisor;
				farVertices[vertex].y = height-height*farVertices[vertex].cameraY/divisor;
			}
			polygon = farVertices;
		}else
			framebuffer.triviallyUnclippedTriangles++;
		//- rouz edit (ChatGPT)
		// Reuse the material state across triangles sharing the same material ID
		//+ rouz edit (ChatGPT)
		MaterialState *state = &fallback;
		if(tri.matId < geometry->matList.numMaterials && geometry->matList.materials[tri.matId]){
			state = &materialStates[tri.matId];
			if(!state->resolved){
				rw::Material *material = geometry->matList.materials[tri.matId];
				state->color = geometry->flags & rw::Geometry::MODULATE ? material->color : rw::RGBA{ 255, 255, 255, 255 };
				state->blendColor = material->color;
				state->surfaceAmbient = material->surfaceProps.ambient;
				state->surfaceDiffuse = material->surfaceProps.diffuse;
				state->source = material->texture;
				state->cached = hasTexCoords ? getCachedTexture(state->source) : nullptr;
				// Resolve the active Neo vehicle environment and specular settings
				//+ rouz edit (ChatGPT)
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
				if(vehicleAtomic){
					state->vehicleState.enabled = true;
					state->vehicleState.environmentSource = CustomPipes::EnvMapTex;
					state->vehicleState.environmentCached = getCachedTexture(state->vehicleState.environmentSource);
					state->vehicleState.fresnel = vehicleFresnel;
					state->vehicleState.shininess = material->surfaceProps.specular*CustomPipes::VehicleShininess;
					state->vehicleState.specularity = material->surfaceProps.specular == 0.0f
						? 0.0f : vehicleSpecularity;
					state->vehicleState.lightStrength = vehicleLightStrength;
				}
#endif
				//- rouz edit (ChatGPT)
				// Resolve the gloss dictionary texture only for enabled wet-road atomics
				//+ rouz edit (ChatGPT)
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
				if(glossAtomic && material->texture && hasTexCoords){
					state->gloss.source = CustomPipes::GetGlossTex(material);
					state->gloss.cached = state->gloss.source ? getCachedTexture(state->gloss.source) : nullptr;
					state->gloss.multiplier = CustomPipes::GlossMult;
				}
#endif
				//- rouz edit (ChatGPT)
				rw::MatFX *matfx = (vehicleAtomic || reflectedVehicleAtomic) ? nullptr : rw::MatFX::get(material);
				// Retain the current UV animation matrices for this material submission
				//+ rouz edit (ChatGPT)
				if(matfx && (matfx->type == rw::MatFX::UVTRANSFORM ||
					matfx->type == rw::MatFX::DUALUVTRANSFORM))
					matfx->getUVTransformMatrices(&state->baseUVTransform, &state->dualUVTransform);
				//- rouz edit (ChatGPT)
				// Resolve reflection data from both supported environment map material types
				//+ rouz edit (ChatGPT)
				const int environmentIndex = advancedMaterials && matfx ? matfx->getEffectIndex(rw::MatFX::ENVMAP) : -1;
				if(environmentIndex >= 0){
					const rw::MatFX::Env &environment = matfx->fx[environmentIndex].env;
					state->environment.source = environment.tex;
					state->environment.cached = getCachedTexture(environment.tex);
					// Select the camera or material frame used to orient the reflection map
					state->environment.normalMatrix = camera->viewMatrix;
					state->environment.normalMatrix.pos.set(0.0f, 0.0f, 0.0f);
					if(environment.frame){
						// Keep camera-space mapping if a malformed environment frame cannot be inverted
						//+ rouz edit (ChatGPT)
						rw::Matrix inverseFrame = state->environment.normalMatrix;
						rw::Matrix *environmentFrameMatrix = environment.frame->getLTM();
						if(environmentFrameMatrix && rw::Matrix::invert(&inverseFrame, environmentFrameMatrix)){
							inverseFrame.pos.set(0.0f, 0.0f, 0.0f);
							state->environment.normalMatrix = inverseFrame;
						}
						//- rouz edit (ChatGPT)
					}
					state->environment.coefficient = environment.coefficient;
					state->environment.framebufferAlpha = environment.fbAlpha != 0;
					state->environment.applyLight = rw::MatFX::envMapApplyLight;
					state->environment.color = rw::MatFX::envMapUseMatColor
						? material->color : rw::MatFX::envMapColor;
				}
				// Resolve a bump texture for standalone and environment-mapped bump effects
				//+ rouz edit (ChatGPT)
				const int bumpIndex = advancedMaterials && matfx ? matfx->getEffectIndex(rw::MatFX::BUMPMAP) : -1;
				if(bumpIndex >= 0 && hasTexCoords){
					state->environment.bumpSource = matfx->getBumpTexture();
					state->environment.bumpCached = state->environment.bumpSource
						? getCachedTexture(state->environment.bumpSource) : nullptr;
					state->environment.bumpCoefficient = matfx->getBumpCoefficient();
				}
				//- rouz edit (ChatGPT)
				// Resolve the enabled dual-texture world lightmap state
				//+ rouz edit (ChatGPT)
				bool useWorldLightmap = false;
#if defined(EXTENDED_PIPELINES) && defined(LIBRW)
				if(advancedMaterials && CustomPipes::LightmapEnable && geometry->numTexCoordSets > 1 &&
				   geometry->texCoords[1] && matfx &&
				   (matfx->type == rw::MatFX::DUAL || matfx->type == rw::MatFX::DUALUVTRANSFORM)){
					state->dualTexture.source = matfx->getDualTexture();
					state->dualTexture.cached = state->dualTexture.source
						? getCachedTexture(state->dualTexture.source) : nullptr;
					if(state->dualTexture.cached){
						state->dualTexture.coefficient = CustomPipes::WorldLightmapBlend.Get()*CustomPipes::LightmapMult;
						state->dualTexture.lightmap = true;
						state->dualTexture.usesSecondaryUV = true;
						useWorldLightmap = true;
					}
				}
#endif
				// Resolve the regular MatFX blend when no world lightmap pass applies
				//+ rouz edit (ChatGPT)
				if(advancedMaterials && !useWorldLightmap && hasTexCoords && matfx &&
				   (matfx->type == rw::MatFX::DUAL || matfx->type == rw::MatFX::DUALUVTRANSFORM)){
					state->dualTexture.source = matfx->getDualTexture();
					state->dualTexture.cached = state->dualTexture.source
						? getCachedTexture(state->dualTexture.source) : nullptr;
					if(state->dualTexture.cached){
						state->dualTexture.coefficient = 1.0f;
						state->dualTexture.sourceBlend = matfx->getDualSrcBlend();
						state->dualTexture.destinationBlend = matfx->getDualDestBlend();
						state->dualTexture.lightmap = false;
						state->dualTexture.usesSecondaryUV = false;
					}
				}
				//- rouz edit (ChatGPT)
				//- rouz edit (ChatGPT)
				//- rouz edit (ChatGPT)
				state->resolved = true;
			}
		}
		//- rouz edit (ChatGPT)
		// Count triangles that pass the camera depth checks
		framebuffer.submittedTriangles++;
		// Count triangles whose material has a CPU texture
		if(state->cached || (state->dualTexture.cached && state->dualTexture.coefficient != 0.0f) || state->gloss.cached)
			framebuffer.texturedTriangles++;
		// Map clipped world normals through the active material environment frame
		//+ rouz edit (ChatGPT)
		ScreenVertex environmentVertices[6];
		for(int vertex = 0; vertex < polygonCount; vertex++){
			environmentVertices[vertex] = polygon[vertex];
			// Transform both animated material UV channels after depth clipping
			//+ rouz edit (ChatGPT)
			transformTextureCoordinate(state->baseUVTransform,
				&environmentVertices[vertex].u, &environmentVertices[vertex].v);
			float dualU = state->dualTexture.usesSecondaryUV
				? polygon[vertex].dualU : polygon[vertex].u;
			float dualV = state->dualTexture.usesSecondaryUV
				? polygon[vertex].dualV : polygon[vertex].v;
			transformTextureCoordinate(state->dualUVTransform, &dualU, &dualV);
			environmentVertices[vertex].dualU = dualU;
			environmentVertices[vertex].dualV = dualV;
			//- rouz edit (ChatGPT)
		}
		if(state->environment.cached && state->environment.coefficient != 0.0f){
			const float envUScale = rw::MatFX::envMapFlipU ? -0.5f : 0.5f;
			for(int vertex = 0; vertex < polygonCount; vertex++){
				const rw::V3d worldNormal = { polygon[vertex].normalX,
					polygon[vertex].normalY, polygon[vertex].normalZ };
				rw::V3d environmentNormal;
				rw::V3d::transformVectors(&environmentNormal, &worldNormal, 1,
					&state->environment.normalMatrix);
				environmentVertices[vertex].u2 = 0.5f+environmentNormal.x*envUScale;
				environmentVertices[vertex].v2 = 0.5f-environmentNormal.y*0.5f;
			}
		}
		const ScreenVertex *renderPolygon = environmentVertices;
		//- rouz edit (ChatGPT)
		// Submit opaque pieces now and retain blended pieces until opaque world rendering finishes
		//+ rouz edit (ChatGPT)
		for(int vertex = 1; vertex+1 < polygonCount; vertex++){
			const ScreenVertex &triangleB = renderPolygon[vertex];
			const ScreenVertex &triangleC = renderPolygon[vertex+1];
			// Use the material color for surfaces that enter a transparent blend pass
			const bool useBlendColor = state->blendColor.alpha < 255 || alphaMultiplier < 1.0f ||
				vertexAlphaEnabled || (state->cached && !state->cached->opaque) ||
				(state->dualTexture.coefficient != 0.0f && state->dualTexture.cached &&
				 !state->dualTexture.cached->opaque) ||
				(geometry->colors && (renderPolygon[0].color.alpha < 255 || triangleB.color.alpha < 255 ||
				triangleC.color.alpha < 255));
			const rw::RGBA triangleColor = useBlendColor ? state->blendColor : state->color;
			// Classify only when the sorted transparent pass is enabled
			//+ rouz edit (ChatGPT)
			const bool transparent = deferTransparentTriangles && deferTransparency &&
				triangleUsesAlpha(renderPolygon[0], triangleB, triangleC,
			   triangleColor, geometry->colors != nullptr, state->cached,
			   state->dualTexture.coefficient != 0.0f ? state->dualTexture.cached : nullptr,
			   vertexAlphaEnabled, alphaMultiplier);
			if(transparent){
			//- rouz edit (ChatGPT)
				DeferredTriangle deferred = {};
				deferred.vertices[0] = renderPolygon[0];
				deferred.vertices[1] = triangleB;
				deferred.vertices[2] = triangleC;
				deferred.color = triangleColor;
				deferred.hasVertexColors = geometry->colors != nullptr;
				deferred.ambient = ambient;
				deferred.surfaceAmbient = state->surfaceAmbient;
				deferred.surfaceDiffuse = state->surfaceDiffuse;
				deferred.dynamicLighting = dynamicLighting;
				deferred.rimlight = rimlightAtomic;
				deferred.vehicle = vehicleAtomic;
				deferred.source = state->source;
				deferred.cached = state->cached;
				deferred.environment = state->environment;
				deferred.dualTexture = state->dualTexture;
				deferred.gloss = state->gloss;
				deferred.vehicleState = state->vehicleState;
				deferred.cullMode = cullMode;
				deferred.fog = fog;
				deferred.stencil = stencil;
				deferred.alphaTest = alphaTest;
				deferred.vertexAlphaEnabled = vertexAlphaEnabled;
				deferred.depthWrite = depthWrite;
				deferred.sourceBlend = sourceBlend;
				deferred.destinationBlend = destinationBlend;
				deferred.alphaMultiplier = alphaMultiplier;
				const float meanDepth = (renderPolygon[0].z+triangleB.z+triangleC.z)/3.0f;
				deferred.sortDepth = std::isfinite(meanDepth) ? meanDepth : 0.0f;
				transparentTriangles.push_back(deferred);
			}else
				drawTriangle(renderPolygon[0], triangleB, triangleC, triangleColor,
					geometry->colors != nullptr, ambient, state->surfaceAmbient, state->surfaceDiffuse,
					dynamicLighting, rimlightAtomic, vehicleAtomic,
					state->source, state->cached, state->environment, state->dualTexture, state->gloss, state->vehicleState,
					cullMode, fog, stencil, alphaTest, vertexAlphaEnabled, depthWrite,
					sourceBlend, destinationBlend, alphaMultiplier);
		}
		//- rouz edit (ChatGPT)
	}
}

}

void Install()
{
	// Replace only the default OpenGL geometry pipeline for this prototype
	if(rw::platform != rw::PLATFORM_GL3 || pipeline)
		return;
	pipeline = rw::ObjPipeline::create();
	pipeline->init(rw::PLATFORM_GL3);
	pipeline->impl.render = renderAtomic;
	originalPipeline = rw::engine->driver[rw::PLATFORM_GL3]->defaultPipeline;
	rw::engine->driver[rw::PLATFORM_GL3]->defaultPipeline = pipeline;
}

void Shutdown()
{
	// Release CPU and GPU framebuffer storage before the RenderWare engine stops
	// Restore the main scene if shutdown interrupts an auxiliary camera render
	if(offscreenTarget.active)
		EndOffscreenFrame(nullptr);
	// Clear auxiliary camera state before freeing its target buffers
	offscreenFrameActive = false;
	SOFTWARE_CIT_FREE(offscreenTarget.pixels);
	SOFTWARE_CIT_FREE(offscreenTarget.depths);
	SOFTWARE_CIT_FREE(offscreenTarget.stencils);
	SOFTWARE_CIT_FREE(offscreenTarget.tileDepthBounds);
	offscreenTarget = {};
	//+ rouz edit (ChatGPT)
	// Drop pending geometry, model tags, and per-instance material opacity
	transparentTriangles.clear();
	glossAtomics.clear();
	rimlightAtomics.clear();
	vehicleAtomics.clear();
	deferTransparentTriangles = false;
	atomicAlphaMultiplier = 1.0f;
	//- rouz edit (ChatGPT)
	// Release decoded material textures while the RenderWare allocator is active
	for(auto &item : textureCache)
		destroyCachedTexture(&item.second);
	textureCache.clear();
	// Release immediate effect texture copies before shutting down RenderWare
	//+ rouz edit (ChatGPT)
	for(auto &item : effectTextureCache)
		destroyCachedTexture(&item.second);
	effectTextureCache.clear();
	capturingWorldEffects = false;
	immediateVertices = nullptr;
	immediateVertexCount = 0;
	//+ rouz edit (ChatGPT)
	// Release the scene camera reference before RenderWare shuts down
	mainFrameCamera = nullptr;
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
	textureCacheBytes = 0;
	cacheFrame = 0;
	if(texture){
		texture->destroy();
		texture = nullptr;
	}
	SOFTWARE_CIT_FREE(pixels);
	SOFTWARE_CIT_FREE(depths);
	//+ rouz edit (ChatGPT)
	// Release the per-pixel stencil buffer before shutting down CIT Alloc
	SOFTWARE_CIT_FREE(stencils);
	stencils = nullptr;
	//- rouz edit (ChatGPT)
	SOFTWARE_CIT_FREE(tileDepthBounds);
	// Release temporal framebuffer history before shutting down CIT Alloc
	//+ rouz edit (ChatGPT)
	if(previousFramePixels)
		SOFTWARE_CIT_FREE(previousFramePixels);
	previousFramePixels = nullptr;
	previousFrameWidth = previousFrameHeight = 0;
	previousFrameValid = false;
	//+ rouz edit (ChatGPT)
	// Release the temporary source image used by screen-space refraction effects
	if(screenTexturePixels)
		SOFTWARE_CIT_FREE(screenTexturePixels);
	screenTexturePixels = nullptr;
	screenTextureWidth = screenTextureHeight = 0;
	screenTextureValid = false;
	//- rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
	pixels = nullptr;
	depths = nullptr;
	tileDepthBounds = nullptr;
	tilesAcross = 0;
	width = height = 0;
	framebuffer = {};
	// Restore the previous pipeline before releasing the CPU render hook
	if(pipeline){
		rw::engine->driver[rw::PLATFORM_GL3]->defaultPipeline = originalPipeline;
		pipeline->destroy();
		pipeline = nullptr;
		originalPipeline = nullptr;
	}
}

void Attach(rw::Atomic *atomic, bool gloss, bool rimlight, bool vehicle)
{
	// Route atomics through the CPU path and retain their custom effect tags
	//+ rouz edit (ChatGPT)
	if(atomic && gloss)
		glossAtomics.insert(atomic);
	else if(atomic)
		glossAtomics.erase(atomic);
	if(atomic && rimlight)
		rimlightAtomics.insert(atomic);
	else if(atomic)
		rimlightAtomics.erase(atomic);
	if(atomic && vehicle)
		vehicleAtomics.insert(atomic);
	else if(atomic)
		vehicleAtomics.erase(atomic);
	if(atomic && pipeline)
		atomic->pipeline = pipeline;
	//- rouz edit (ChatGPT)
}

void AttachRimlightClump(rw::Clump *clump)
{
	// Register every ped atomic for the CPU rim-light pass
	if(!clump)
		return;
	FORLIST(link, clump->atomics){
		rw::Atomic *atomic = rw::Atomic::fromClump(link);
		Attach(atomic, false, true, false);
	}
}

void AttachVehicleClump(rw::Clump *clump)
{
	// Register every vehicle atomic for the Neo reflection and specular pass
	if(!clump)
		return;
	FORLIST(link, clump->atomics){
		rw::Atomic *atomic = rw::Atomic::fromClump(link);
		Attach(atomic, false, false, true);
	}
}

void RenderAtomic(rw::Atomic *atomic, int alpha)
{
	// Apply an instance fade multiplier only for the duration of this atomic submission
	//+ rouz edit (ChatGPT)
	if(atomic){
		const float previousAlphaMultiplier = atomicAlphaMultiplier;
		atomicAlphaMultiplier = std::max(0, std::min(255, alpha))*(1.f/255.f);
		renderAtomic(pipeline, atomic);
		atomicAlphaMultiplier = previousAlphaMultiplier;
	}
	//- rouz edit (ChatGPT)
}

void FlushTransparentTriangles()
{
	// Composite deferred transparent geometry from farthest to nearest
	std::stable_sort(transparentTriangles.begin(), transparentTriangles.end(),
		[](const DeferredTriangle &a, const DeferredTriangle &b){ return a.sortDepth > b.sortDepth; });
	for(const DeferredTriangle &triangle : transparentTriangles)
			drawTriangle(triangle.vertices[0], triangle.vertices[1], triangle.vertices[2],
			triangle.color, triangle.hasVertexColors, triangle.ambient, triangle.surfaceAmbient,
			triangle.surfaceDiffuse, triangle.dynamicLighting, triangle.rimlight, triangle.vehicle,
			triangle.source, triangle.cached, triangle.environment, triangle.dualTexture, triangle.gloss, triangle.vehicleState,
			triangle.cullMode, triangle.fog, triangle.stencil,
			triangle.alphaTest, triangle.vertexAlphaEnabled, triangle.depthWrite, triangle.sourceBlend,
			triangle.destinationBlend, triangle.alphaMultiplier);
	transparentTriangles.clear();
	deferTransparentTriangles = false;
}

void RenderClump(rw::Clump *clump)
{
	// Submit visible vehicle and ped parts while preserving their frame transforms
	if(!clump)
		return;
	FORLIST(link, clump->atomics){
		rw::Atomic *atomic = rw::Atomic::fromClump(link);
		if(atomic->object.object.flags & rw::Atomic::RENDER)
			renderAtomic(pipeline, atomic);
	}
}

void RenderVehicleClump(rw::Clump *clump)
{
	// Let vehicle callbacks select body LODs, wheels, and visible components
	if(!clump)
		return;
	FORLIST(link, clump->atomics){
		rw::Atomic *atomic = rw::Atomic::fromClump(link);
		if(!(atomic->object.object.flags & rw::Atomic::RENDER))
			continue;
		// Submit parts without a game visibility callback through the CPU path
		if(atomic->renderCB == rw::Atomic::defaultRenderCB)
			renderAtomic(pipeline, atomic);
		else
			atomic->render();
	}
}

void BeginWorldEffects()
{
	// Enable immediate capture only while a software frame is being assembled
	capturingWorldEffects = framePending;
	immediateVertices = nullptr;
	immediateVertexCount = 0;
}

void EndWorldEffects()
{
	// Stop capturing before the CPU framebuffer is uploaded as a GPU texture
	capturingWorldEffects = false;
	immediateVertices = nullptr;
	immediateVertexCount = 0;
}

bool CapturingWorldEffects()
{
	// Accept draws from the main camera or the active CPU offscreen target
	return framePending && capturingWorldEffects &&
		(offscreenTarget.active || (rw::engine && rw::engine->currentCamera == mainFrameCamera));
}

bool FramePending()
{
	// Report whether the current CPU frame still needs its final upload
	return framePending;
}

bool BeginOffscreenFrame(rw::Camera *camera, const rw::RGBA &clearColor)
{
	// Preserve the existing solid-clear API for auxiliary renders without a sky gradient
	return BeginOffscreenFrame(camera, clearColor, clearColor);
}

bool BeginOffscreenFrame(rw::Camera *camera, const rw::RGBA &topColor, const rw::RGBA &bottomColor)
{
	// Accept auxiliary camera renders before the main software frame has started
	//+ rouz edit (ChatGPT)
	if(offscreenTarget.active || !camera || !camera->frameBuffer ||
	   camera->frameBuffer->width <= 0 || camera->frameBuffer->height <= 0)
		return false;
	//- rouz edit (ChatGPT)
	const int targetWidth = camera->frameBuffer->width;
	const int targetHeight = camera->frameBuffer->height;
	if(targetWidth != offscreenTarget.width || targetHeight != offscreenTarget.height){
		const size_t pixelCount = (size_t)targetWidth*targetHeight;
		const int targetTilesAcross = (targetWidth+7)/8;
		const size_t tileCount = (size_t)targetTilesAcross*((targetHeight+7)/8);
		rw::RGBA *newPixels = (rw::RGBA*)SOFTWARE_CIT_MALLOC(pixelCount*sizeof(rw::RGBA));
		float *newDepths = (float*)SOFTWARE_CIT_MALLOC(pixelCount*sizeof(float));
		rw::uint8 *newStencils = (rw::uint8*)SOFTWARE_CIT_MALLOC(pixelCount*sizeof(rw::uint8));
		float *newTileDepthBounds = (float*)SOFTWARE_CIT_MALLOC(tileCount*sizeof(float));
		if(!newPixels || !newDepths || !newStencils || !newTileDepthBounds){
			SOFTWARE_CIT_FREE(newPixels);
			SOFTWARE_CIT_FREE(newDepths);
			SOFTWARE_CIT_FREE(newStencils);
			SOFTWARE_CIT_FREE(newTileDepthBounds);
			return false;
		}
		SOFTWARE_CIT_FREE(offscreenTarget.pixels);
		SOFTWARE_CIT_FREE(offscreenTarget.depths);
		SOFTWARE_CIT_FREE(offscreenTarget.stencils);
		SOFTWARE_CIT_FREE(offscreenTarget.tileDepthBounds);
		offscreenTarget.pixels = newPixels;
		offscreenTarget.depths = newDepths;
		offscreenTarget.stencils = newStencils;
		offscreenTarget.tileDepthBounds = newTileDepthBounds;
		offscreenTarget.width = targetWidth;
		offscreenTarget.height = targetHeight;
		offscreenTarget.tilesAcross = targetTilesAcross;
	}
	// Preserve the active scene target and its counters until the auxiliary pass completes
	offscreenTarget.savedPixels = pixels;
	offscreenTarget.savedDepths = depths;
	offscreenTarget.savedStencils = stencils;
	offscreenTarget.savedTileDepthBounds = tileDepthBounds;
	offscreenTarget.savedWidth = width;
	offscreenTarget.savedHeight = height;
	offscreenTarget.savedTilesAcross = tilesAcross;
	offscreenTarget.savedFramebuffer = framebuffer;
	offscreenTarget.savedFramePending = framePending;
	offscreenTarget.savedCapturingWorldEffects = capturingWorldEffects;
	offscreenTarget.savedDeferTransparentTriangles = deferTransparentTriangles;
	offscreenTarget.savedScreenTextureValid = screenTextureValid;
	offscreenTarget.savedAtomicAlphaMultiplier = atomicAlphaMultiplier;
	offscreenTarget.savedImmediateVertices = immediateVertices;
	offscreenTarget.savedImmediateVertexCount = immediateVertexCount;
	offscreenTarget.savedImmediateMatrix = immediateMatrix;
	offscreenTarget.savedImmediateHasMatrix = immediateHasMatrix;
	offscreenTarget.savedTransparentTriangles.swap(transparentTriangles);
	// Clear the auxiliary target to a vertical sky gradient before submitting geometry
	//+ rouz edit (ChatGPT)
	pixels = offscreenTarget.pixels;
	depths = offscreenTarget.depths;
	stencils = offscreenTarget.stencils;
	tileDepthBounds = offscreenTarget.tileDepthBounds;
	width = offscreenTarget.width;
	height = offscreenTarget.height;
	tilesAcross = offscreenTarget.tilesAcross;
	for(int y = 0; y < height; y++){
		const float t = height > 1 ? (float)y/(height-1) : 0.0f;
		const rw::RGBA sky = {
			(rw::uint8)(topColor.red+(bottomColor.red-topColor.red)*t),
			(rw::uint8)(topColor.green+(bottomColor.green-topColor.green)*t),
			(rw::uint8)(topColor.blue+(bottomColor.blue-topColor.blue)*t),
			(rw::uint8)(topColor.alpha+(bottomColor.alpha-topColor.alpha)*t)
		};
		std::fill(pixels+(size_t)y*width, pixels+(size_t)(y+1)*width, sky);
	}
	//- rouz edit (ChatGPT)
	std::fill(depths, depths+(size_t)width*height, 0.0f);
	std::fill(stencils, stencils+(size_t)width*height, (rw::uint8)0);
	std::fill(tileDepthBounds, tileDepthBounds+(size_t)tilesAcross*((height+7)/8), 0.0f);
	framebuffer = {};
	framebuffer.width = width;
	framebuffer.height = height;
	framebuffer.pixels = pixels;
	framebuffer.depths = depths;
	framebuffer.cachedTextures = (unsigned int)textureCache.size();
	framePending = true;
	capturingWorldEffects = true;
	deferTransparentTriangles = false;
	screenTextureValid = false;
	atomicAlphaMultiplier = 1.0f;
	immediateVertices = nullptr;
	immediateVertexCount = 0;
	immediateHasMatrix = false;
	// Mark the auxiliary target active while its CPU pixels are installed
	offscreenFrameActive = true;
	offscreenTarget.active = true;
	return true;
}

bool BeginOffscreenFrame(rw::Camera *camera, rw::Raster *initialRaster)
{
	// Import an existing camera raster before applying a partial software pass
	const rw::RGBA clearColor = { 0, 0, 0, 0 };
	if(!initialRaster || initialRaster->width <= 0 || initialRaster->height <= 0 ||
	   !BeginOffscreenFrame(camera, clearColor))
		return false;
	bool topDown = true;
	rw::Image *image = readTextureImage(initialRaster, topDown);
	// Restore the main framebuffer if the source raster cannot be decoded
	if(!image || !image->pixels || image->width <= 0 || image->height <= 0 ||
	   image->stride < image->width*4){
		if(image)
			image->destroy();
		EndOffscreenFrame(nullptr);
		return false;
	}
	// Scale the decoded raster into the CPU target while preserving its row origin
	for(int y = 0; y < height; y++){
		int sourceY = y*image->height/height;
		if(!topDown)
			sourceY = image->height-1-sourceY;
		for(int x = 0; x < width; x++){
			const int sourceX = x*image->width/width;
			const rw::uint8 *source = image->pixels+(size_t)sourceY*image->stride+(size_t)sourceX*4;
			std::memcpy(&pixels[(size_t)y*width+x], source, sizeof(rw::RGBA));
		}
	}
	image->destroy();
	return true;
}

void PrimeTexture(rw::Texture *source)
{
	// Decode dynamic camera textures before a render pass begins writing into them
	if(source)
		getCachedTexture(source);
}

void ApplyTextureMask(rw::Texture *source)
{
	// Multiply an offscreen image by the mask texture used on the hardware environment pass
	if(!offscreenTarget.active || !source || !pixels || width <= 0 || height <= 0)
		return;
	const CachedTexture *cached = getCachedTexture(source);
	if(!cached || !cached->image || !cached->image->pixels)
		return;
	TextureSampler sampler = {};
	sampler.pixels = cached->image->pixels;
	sampler.width = cached->image->width;
	sampler.height = cached->image->height;
	sampler.stride = cached->image->stride;
	sampler.topDown = cached->topDown;
	sampler.linear = filterUsesLinear(source->getFilter());
	sampler.addressU = (rw::Texture::Addressing)((source->filterAddressing >> 8) & 0xF);
	sampler.addressV = (rw::Texture::Addressing)((source->filterAddressing >> 12) & 0xF);
	sampler.wrap = sampler.addressU == rw::Texture::WRAP && sampler.addressV == rw::Texture::WRAP;
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++){
			rw::RGBA mask;
			if(!sampleTexture(sampler, (x+0.5f)/width, (y+0.5f)/height, mask))
				continue;
			rw::RGBA &destination = pixels[(size_t)y*width+x];
			destination.red = (rw::uint8)((unsigned)destination.red*mask.red/255);
			destination.green = (rw::uint8)((unsigned)destination.green*mask.green/255);
			destination.blue = (rw::uint8)((unsigned)destination.blue*mask.blue/255);
			destination.alpha = (rw::uint8)((unsigned)destination.alpha*mask.alpha/255);
		}
}

bool EndOffscreenFrame(rw::Raster *raster)
{
	// Upload the auxiliary CPU image before restoring the main scene framebuffer
	if(!offscreenTarget.active)
		return false;
	bool uploaded = false;
	if(raster && raster->width > 0 && raster->height > 0){
		rw::uint8 *destination = raster->lock(0, rw::Raster::LOCKWRITE | rw::Raster::LOCKNOFETCH);
		if(destination){
			for(int y = 0; y < raster->height; y++){
				const int sourceY = (raster->height-1-y)*height/raster->height;
				for(int x = 0; x < raster->width; x++){
					const int sourceX = x*width/raster->width;
					const rw::RGBA &pixel = pixels[(size_t)sourceY*width+sourceX];
					std::memcpy(destination+(size_t)y*raster->stride+(size_t)x*sizeof(rw::RGBA),
						&pixel, sizeof(rw::RGBA));
				}
			}
			raster->unlock(0);
			uploaded = true;
		}
	}
	// Restore the scene buffers, counters, and any geometry collected before the offscreen pass
	transparentTriangles.clear();
	transparentTriangles.swap(offscreenTarget.savedTransparentTriangles);
	pixels = offscreenTarget.savedPixels;
	depths = offscreenTarget.savedDepths;
	stencils = offscreenTarget.savedStencils;
	tileDepthBounds = offscreenTarget.savedTileDepthBounds;
	width = offscreenTarget.savedWidth;
	height = offscreenTarget.savedHeight;
	tilesAcross = offscreenTarget.savedTilesAcross;
	framebuffer = offscreenTarget.savedFramebuffer;
	framePending = offscreenTarget.savedFramePending;
	capturingWorldEffects = offscreenTarget.savedCapturingWorldEffects;
	deferTransparentTriangles = offscreenTarget.savedDeferTransparentTriangles;
	screenTextureValid = offscreenTarget.savedScreenTextureValid;
	atomicAlphaMultiplier = offscreenTarget.savedAtomicAlphaMultiplier;
	immediateVertices = offscreenTarget.savedImmediateVertices;
	immediateVertexCount = offscreenTarget.savedImmediateVertexCount;
	immediateMatrix = offscreenTarget.savedImmediateMatrix;
	immediateHasMatrix = offscreenTarget.savedImmediateHasMatrix;
	offscreenTarget.active = false;
	// Return cache refreshes to the main software frame cadence
	offscreenFrameActive = false;
	return uploaded;
}

bool BeginScreenTextureSampling()
{
	// Snapshot the finished scene so refractive screen effects read stable pixels
	if(!framePending || !CapturingWorldEffects() || !pixels || width <= 0 || height <= 0)
		return false;
	screenTextureValid = false;
	if(screenTextureWidth != width || screenTextureHeight != height){
		const size_t count = (size_t)width*height;
		rw::RGBA *newPixels = (rw::RGBA*)SOFTWARE_CIT_MALLOC(count*sizeof(rw::RGBA));
		if(!newPixels)
			return false;
		if(screenTexturePixels)
			SOFTWARE_CIT_FREE(screenTexturePixels);
		screenTexturePixels = newPixels;
		screenTextureWidth = width;
		screenTextureHeight = height;
	}
	// Preserve this frame before the first droplet modifies the CPU framebuffer
	std::memcpy(screenTexturePixels, pixels, (size_t)width*height*sizeof(rw::RGBA));
	screenTextureValid = true;
	return true;
}

void BeginImmediate(const void *vertices, int count, const void *matrix, bool hasUV)
{
	// Retain the batch vertices and optional local transform until its indexed draw
	immediateVertices = (const RwIm3DVertex*)vertices;
	immediateVertexCount = count;
	immediateHasMatrix = matrix != nullptr;
	//+ rouz edit (ChatGPT)
	// Capture whether this batch supplies texture coordinates
	immediateHasUV = hasUV;
	//- rouz edit (ChatGPT)
	if(matrix)
		immediateMatrix = *(const rw::Matrix*)matrix;
}

void RenderImmediateIndexed(int primitiveType, const unsigned short *indices, int count)
{
	// Accept triangle lists with CPU vertices during an active camera update
	rw::Camera *camera = rw::engine->currentCamera;
	if(!CapturingWorldEffects() || !camera || !immediateVertices || immediateVertexCount <= 0 ||
	   !indices || count <= 0 || !pixels ||
	   (primitiveType != rw::PRIMTYPETRILIST && primitiveType != rw::PRIMTYPELINELIST &&
	    primitiveType != rw::PRIMTYPEPOLYLINE && primitiveType != rw::PRIMTYPETRISTRIP &&
	    primitiveType != rw::PRIMTYPETRIFAN && primitiveType != rw::PRIMTYPEPOINTLIST))
		return;
	// Capture fog settings for this immediate world-effect batch
	//+ rouz edit (ChatGPT)
	const FogState fog = getFogState(camera);
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Capture whether RenderWare enables per-vertex alpha for this immediate batch
	const bool vertexAlphaEnabled = rw::GetRenderState(rw::VERTEXALPHA) != 0;
	//- rouz edit (ChatGPT)
	rw::Matrix modelView = camera->viewMatrix;
	if(immediateHasMatrix)
		rw::Matrix::mult(&modelView, &immediateMatrix, &camera->viewMatrix);
	std::vector<ScreenVertex> projected((size_t)immediateVertexCount);
	// Transform effect vertices with the same camera projection as world atomics
	for(int i = 0; i < immediateVertexCount; i++){
		const RwIm3DVertex &source = immediateVertices[i];
		rw::V3d cameraPoint;
		rw::V3d::transformPoints(&cameraPoint, &source.position, 1, &modelView);
		ScreenVertex &vertex = projected[i];
		vertex.cameraX = cameraPoint.x;
		vertex.cameraY = cameraPoint.y;
		vertex.z = cameraPoint.z;
		vertex.u = source.u;
		vertex.v = source.v;
		//+ rouz edit (ChatGPT)
		// Ignore immediate vertex opacity when the active RenderWare state disables it
		vertex.color = { source.r, source.g, source.b, (rw::uint8)(vertexAlphaEnabled ? source.a : 255) };
		//- rouz edit (ChatGPT)
		if(vertex.z >= camera->nearPlane){
			const float divisor = camera->projection == rw::Camera::PERSPECTIVE ? vertex.z : 1.0f;
			vertex.x = width*vertex.cameraX/divisor;
			vertex.y = height-height*vertex.cameraY/divisor;
		}
	}
	// Resolve the effect texture and blend state once per indexed batch
	rw::Raster *raster = immediateHasUV ? (rw::Raster*)rw::GetRenderStatePtr(rw::TEXTURERASTER) : nullptr;
	const CachedTexture *cached = getCachedEffectTexture(raster);
	// Skip a textured effect when its raster cannot be decoded on the CPU
	if(raster && !cached)
		return;
	TextureSampler sampler = {};
	if(cached){
		//+ rouz edit (ChatGPT)
		// Apply the active point, linear, or mipmapped filter to world effect textures
		const rw::uint32 filter = rw::GetRenderState(rw::TEXTUREFILTER);
		sampler.mipLevels = cached->mipLevels;
		sampler.mipLevelCount = cached->mipLevelCount;
		sampler.mipmapped = filterUsesMipmaps(filter);
		sampler.mipLinear = filterBlendsMipLevels(filter);
		//- rouz edit (ChatGPT)
		sampler.pixels = cached->image->pixels;
		sampler.width = cached->image->width;
		sampler.height = cached->image->height;
		sampler.stride = cached->image->stride;
		sampler.topDown = cached->topDown;
		sampler.linear = filterUsesLinear(filter);
		sampler.addressU = (rw::Texture::Addressing)rw::GetRenderState(rw::TEXTUREADDRESSU);
		sampler.addressV = (rw::Texture::Addressing)rw::GetRenderState(rw::TEXTUREADDRESSV);
		if(!sampler.addressU) sampler.addressU = rw::Texture::WRAP;
		if(!sampler.addressV) sampler.addressV = rw::Texture::WRAP;
		sampler.wrap = sampler.addressU == rw::Texture::WRAP && sampler.addressV == rw::Texture::WRAP;
		applyTextureSamplingMode(&sampler, worldEffectTextureSamplingMode);
	}
	const rw::uint32 sourceBlend = rw::GetRenderState(rw::SRCBLEND);
	const rw::uint32 destinationBlend = rw::GetRenderState(rw::DESTBLEND);
	const bool depthTest = rw::GetRenderState(rw::ZTESTENABLE) != 0;
	const bool depthWrite = rw::GetRenderState(rw::ZWRITEENABLE) != 0;
	// Capture the immediate effect cull mode once for this batch
	//+ rouz edit (ChatGPT)
	const rw::uint32 cullMode = rw::GetRenderState(rw::CULLMODE);
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Capture stencil rules once for this immediate batch
	const StencilState stencil = getStencilState();
	//- rouz edit (ChatGPT)
	// Capture alpha-test settings for immediate textured triangles
	//+ rouz edit (ChatGPT)
	const AlphaTestState alphaTest = getAlphaTestState();
	//- rouz edit (ChatGPT)
	// Rasterize antenna and rope line batches with the same scene depth buffer
	if(primitiveType == rw::PRIMTYPELINELIST || primitiveType == rw::PRIMTYPEPOLYLINE){
		const int stride = primitiveType == rw::PRIMTYPEPOLYLINE ? 1 : 2;
		for(int i = 0; i+1 < count; i += stride){
			if(indices[i] >= immediateVertexCount || indices[i+1] >= immediateVertexCount)
				continue;
			// Keep the bound effect texture when rasterizing a 3D line primitive
			//+ rouz edit (ChatGPT)
			drawImmediateLine(projected[indices[i]], projected[indices[i+1]], cached ? &sampler : nullptr, sourceBlend,
				destinationBlend, depthTest, depthWrite, fog, stencil, alphaTest, camera->nearPlane, camera->farPlane,
				camera->projection == rw::Camera::PERSPECTIVE);
			//- rouz edit (ChatGPT)
		}
		return;
	}
	// Rasterize immediate point lists as single pixels in the framebuffer
	if(primitiveType == rw::PRIMTYPEPOINTLIST){
		// Apply the captured tests and texture state to each world-space point
		//+ rouz edit (ChatGPT)
		for(int i = 0; i < count; i++){
			if(indices[i] < immediateVertexCount &&
			   projected[indices[i]].z >= camera->nearPlane && projected[indices[i]].z <= camera->farPlane)
				drawImmediate2DPoint(projected[indices[i]], cached ? &sampler : nullptr,
					sourceBlend, destinationBlend, depthTest, depthWrite, fog, stencil, alphaTest);
		}
		//- rouz edit (ChatGPT)
		return;
	}
	// Clip each immediate triangle against both depth planes before blending
	auto drawTriangle = [&](int ia, int ib, int ic){
		if(ia < 0 || ib < 0 || ic < 0 || ia >= immediateVertexCount ||
		   ib >= immediateVertexCount || ic >= immediateVertexCount)
			return;
		const ScreenVertex source[3] = { projected[ia], projected[ib], projected[ic] };
		ScreenVertex nearVertices[6], farVertices[6];
		int nearCount = clipDepthPlane(source, 3, nearVertices, camera->nearPlane, true);
		if(nearCount < 3)
			return;
		int polygonCount = clipDepthPlane(nearVertices, nearCount, farVertices, camera->farPlane, false);
		for(int vertex = 0; vertex < polygonCount; vertex++){
			const float divisor = camera->projection == rw::Camera::PERSPECTIVE ? farVertices[vertex].z : 1.0f;
			farVertices[vertex].x = width*farVertices[vertex].cameraX/divisor;
			farVertices[vertex].y = height-height*farVertices[vertex].cameraY/divisor;
		}
		for(int vertex = 1; vertex+1 < polygonCount; vertex++)
			drawImmediateTriangle(farVertices[0], farVertices[vertex], farVertices[vertex+1],
				cached ? &sampler : nullptr, sourceBlend, destinationBlend, depthTest, depthWrite, cullMode, fog, stencil,
				alphaTest);
	};
	// Preserve list, strip, and fan topology when submitting immediate triangles
	if(primitiveType == rw::PRIMTYPETRILIST){
		for(int i = 0; i+2 < count; i += 3)
			drawTriangle(indices[i], indices[i+1], indices[i+2]);
	}else if(primitiveType == rw::PRIMTYPETRISTRIP){
		// Preserve alternating face winding for immediate world-effect strips
		//+ rouz edit (ChatGPT)
		for(int i = 0; i+2 < count; i++){
			if(i & 1)
				drawTriangle(indices[i+1], indices[i], indices[i+2]);
			else
				drawTriangle(indices[i], indices[i+1], indices[i+2]);
		}
		//- rouz edit (ChatGPT)
	}else if(primitiveType == rw::PRIMTYPETRIFAN){
		for(int i = 1; i+1 < count; i++)
			drawTriangle(indices[0], indices[i], indices[i+1]);
	}
}

void RenderImmediatePrimitive(int primitiveType)
{
	// Generate sequential indices for nonindexed immediate geometry
	if(immediateVertexCount <= 0 || immediateVertexCount > 65535)
		return;
	std::vector<unsigned short> indices((size_t)immediateVertexCount);
	for(int i = 0; i < immediateVertexCount; i++)
		indices[i] = (unsigned short)i;
	RenderImmediateIndexed(primitiveType, indices.data(), immediateVertexCount);
}

static bool drawImmediateGradientQuad(const ScreenVertex *v, int primitiveType, int vertexCount,
	const unsigned short *indices, const TextureSampler *secondarySampler, const CachedTexture *cached,
	rw::uint32 sourceBlend, rw::uint32 destinationBlend, bool depthTest, bool depthWrite,
	rw::uint32 cullMode, const FogState &fog, const StencilState &stencil, const AlphaTestState &alphaTest)
{
	// Accept the untextured flat-depth gradient fan emitted by the horizon bands
	if(primitiveType != rw::PRIMTYPETRIFAN || vertexCount != 4 || indices || cached || secondarySampler ||
	   depthTest || depthWrite || stencil.enabled || sourceBlend != rw::BLENDSRCALPHA ||
	   destinationBlend != rw::BLENDINVSRCALPHA)
		return false;
	if(v[0].color.red != v[1].color.red || v[0].color.green != v[1].color.green ||
	   v[0].color.blue != v[1].color.blue || v[0].color.alpha != v[1].color.alpha ||
	   v[2].color.red != v[3].color.red || v[2].color.green != v[3].color.green ||
	   v[2].color.blue != v[3].color.blue || v[2].color.alpha != v[3].color.alpha ||
	   v[0].z != v[1].z || v[0].z != v[2].z || v[0].z != v[3].z)
		return false;
	for(int i = 0; i < 4; i++)
		if(!std::isfinite(v[i].x) || !std::isfinite(v[i].y) || !std::isfinite(v[i].z))
			return false;
	const ScreenVertex &bottomLeft = v[0], &bottomRight = v[1];
	const ScreenVertex &topRight = v[2], &topLeft = v[3];
	if(std::fabs(topLeft.x+bottomRight.x-topRight.x-bottomLeft.x) > 0.25f ||
	   std::fabs(topLeft.y+bottomRight.y-topRight.y-bottomLeft.y) > 0.25f)
		return false;
	const float area = edge(v[0], v[1], v[2].x, v[2].y);
	const float secondArea = edge(v[0], v[2], v[3].x, v[3].y);
	if(!std::isfinite(area) || !std::isfinite(secondArea) || std::fabs(area) < 0.001f ||
	   std::fabs(secondArea) < 0.001f || area*secondArea <= 0.0f)
		return false;
	if((cullMode == rw::CULLBACK && area < 0.0f) || (cullMode == rw::CULLFRONT && area > 0.0f)){
		framebuffer.culledTriangles += 2;
		return true;
	}

	// Build an affine top-to-bottom colour gradient and fog its endpoints once
	const float topX = topRight.x-topLeft.x, topY = topRight.y-topLeft.y;
	const float determinant = topX*(bottomLeft.y-topLeft.y)-topY*(bottomLeft.x-topLeft.x);
	if(std::fabs(determinant) < 0.001f)
		return false;
	const float stepX = -topY/determinant;
	const float stepY = topX/determinant;
	float topColor[3] = { topLeft.color.red/255.0f, topLeft.color.green/255.0f, topLeft.color.blue/255.0f };
	float bottomColor[3] = { bottomLeft.color.red/255.0f, bottomLeft.color.green/255.0f, bottomLeft.color.blue/255.0f };
	applyFog(topColor, topLeft.z, fog);
	applyFog(bottomColor, topLeft.z, fog);
	const float topAlpha = topLeft.color.alpha/255.0f;
	const float alphaStep = (bottomLeft.color.alpha-topLeft.color.alpha)/255.0f;
	const float minY = std::min(std::min(v[0].y,v[1].y),std::min(v[2].y,v[3].y));
	const float maxY = std::max(std::max(v[0].y,v[1].y),std::max(v[2].y,v[3].y));
	const int firstY = std::max(0, (int)std::ceil(minY-0.5f));
	const int lastY = std::min(height-1, (int)std::floor(maxY-0.5f));
	if(lastY < firstY)
		return true;

	// Intersect each framebuffer row with the quad and shade only its covered span
	for(int y = firstY; y <= lastY; y++){
		const float sampleY = y+0.5f;
		float firstX = std::numeric_limits<float>::infinity();
		float lastX = -std::numeric_limits<float>::infinity();
		for(int side = 0; side < 4; side++){
			const ScreenVertex &a = v[side], &b = v[(side+1)&3];
			if(sampleY < std::min(a.y,b.y) || sampleY >= std::max(a.y,b.y))
				continue;
			const float hitX = a.x+(sampleY-a.y)*(b.x-a.x)/(b.y-a.y);
			firstX = std::min(firstX, hitX);
			lastX = std::max(lastX, hitX);
		}
		if(!std::isfinite(firstX) || !std::isfinite(lastX) || lastX < 0.0f || firstX >= width)
			continue;
		const int firstPixel = std::max(0, (int)std::ceil(firstX-0.5f));
		const int lastPixel = std::min(width-1, (int)std::floor(lastX-0.5f));
		float t = stepX*(firstPixel+0.5f-topLeft.x)+stepY*(sampleY-topLeft.y);
		for(int x = firstPixel; x <= lastPixel; x++, t += stepX){
			const float factor = std::max(0.0f, std::min(1.0f,t));
			const float alpha = topAlpha+alphaStep*factor;
			if(alpha <= 0.0f || !immediateAlphaTestPasses(alphaTest, alpha))
				continue;
			rw::RGBA &destination = pixels[(size_t)y*width+x];
			float color[3];
			for(int channel = 0; channel < 3; channel++)
				color[channel] = topColor[channel]+(bottomColor[channel]-topColor[channel])*factor;
			if(alpha >= 1.0f){
				destination.red = (rw::uint8)(255.0f*color[0]);
				destination.green = (rw::uint8)(255.0f*color[1]);
				destination.blue = (rw::uint8)(255.0f*color[2]);
			}else{
				destination.red = (rw::uint8)(255.0f*color[0]*alpha+destination.red*(1.0f-alpha));
				destination.green = (rw::uint8)(255.0f*color[1]*alpha+destination.green*(1.0f-alpha));
				destination.blue = (rw::uint8)(255.0f*color[2]*alpha+destination.blue*(1.0f-alpha));
			}
			framebuffer.coveredPixels++;
			framebuffer.totalCoveredPixels++;
		}
	}
	return true;
}

static void renderImmediate2D(int primitiveType, const void *vertexData, int vertexCount,
	const unsigned short *indices, int indexCount, const float *secondaryUVs,
	const TextureSampler *secondarySampler, float secondaryScaleX, float secondaryScaleY)
{
	// Accept screen-space primitives only during the software world-effects pass
	rw::Camera *camera = rw::engine->currentCamera;
	if(!CapturingWorldEffects() || !camera || !camera->frameBuffer || !vertexData ||
	   camera->frameBuffer->width <= 0 || camera->frameBuffer->height <= 0 ||
	   vertexCount <= 0 || !pixels || (indices && indexCount <= 0) ||
	   (secondaryUVs && !secondarySampler))
		return;
	// Capture fog settings for this screen-space effect batch
	//+ rouz edit (ChatGPT)
	const FogState fog = getFogState(camera);
	//- rouz edit (ChatGPT)
	// Convert RenderWare screen coordinates into the reduced CPU framebuffer
	const RwIm2DVertex *source = (const RwIm2DVertex*)vertexData;
	//+ rouz edit (ChatGPT)
	// Capture whether RenderWare enables per-vertex alpha for this screen-space batch
	const bool vertexAlphaEnabled = rw::GetRenderState(rw::VERTEXALPHA) != 0;
	//- rouz edit (ChatGPT)
	const float scaleX = (float)width/camera->frameBuffer->width;
	const float scaleY = (float)height/camera->frameBuffer->height;
	std::vector<ScreenVertex> projected((size_t)vertexCount);
	// Convert each Im2D vertex into a CPU pixel coordinate and depth value
	for(int i = 0; i < vertexCount; i++){
		ScreenVertex &vertex = projected[i];
		const float cameraZ = source[i].w;
		vertex.x = source[i].x*scaleX;
		vertex.y = source[i].y*scaleY;
		// Convert OpenGL Im2D screen depth into the CPU buffer's reciprocal camera depth
		//+ rouz edit (ChatGPT)
		const float screenZ = source[i].z;
		const float depthFraction = (screenZ+1.0f)*0.5f;
		const float inverseScreenDepth = (1.0f-depthFraction)/camera->nearPlane+
			depthFraction/camera->farPlane;
		vertex.z = std::isfinite(screenZ) && screenZ >= -1.0f && screenZ <= 1.0f &&
			inverseScreenDepth > 0.0f ? 1.0f/inverseScreenDepth :
			(std::isfinite(cameraZ) && cameraZ >= camera->nearPlane && cameraZ <= camera->farPlane
				? cameraZ : camera->nearPlane);
		//- rouz edit (ChatGPT)
		vertex.cameraX = vertex.x;
		vertex.cameraY = vertex.y;
		vertex.u = source[i].u;
		vertex.v = source[i].v;
		//+ rouz edit (ChatGPT)
		// Keep optional screen-sample coordinates in the CPU framebuffer's normalized space
		vertex.u2 = secondaryUVs ? secondaryUVs[i*2]*secondaryScaleX : 0.0f;
		vertex.v2 = secondaryUVs ? secondaryUVs[i*2+1]*secondaryScaleY : 0.0f;
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Ignore screen-space vertex opacity when the active RenderWare state disables it
		vertex.color = { source[i].r, source[i].g, source[i].b,
			(rw::uint8)(vertexAlphaEnabled ? source[i].a : 255) };
		//- rouz edit (ChatGPT)
	}
	// Resolve texture, blend, and depth state once for this screen-space batch
	rw::Raster *raster = (rw::Raster*)rw::GetRenderStatePtr(rw::TEXTURERASTER);
	const CachedTexture *cached = getCachedEffectTexture(raster);
	if(raster && !cached)
		return;
	TextureSampler sampler = {};
	// Copy sampler settings for the currently bound effect raster
	if(cached){
		//+ rouz edit (ChatGPT)
		// Apply the active point, linear, or mipmapped filter to screen effect textures
		const rw::uint32 filter = rw::GetRenderState(rw::TEXTUREFILTER);
		sampler.mipLevels = cached->mipLevels;
		sampler.mipLevelCount = cached->mipLevelCount;
		sampler.mipmapped = filterUsesMipmaps(filter);
		sampler.mipLinear = filterBlendsMipLevels(filter);
		//- rouz edit (ChatGPT)
		sampler.pixels = cached->image->pixels;
		sampler.width = cached->image->width;
		sampler.height = cached->image->height;
		sampler.stride = cached->image->stride;
		sampler.topDown = cached->topDown;
		sampler.linear = filterUsesLinear(filter);
		sampler.addressU = (rw::Texture::Addressing)rw::GetRenderState(rw::TEXTUREADDRESSU);
		sampler.addressV = (rw::Texture::Addressing)rw::GetRenderState(rw::TEXTUREADDRESSV);
		if(!sampler.addressU) sampler.addressU = rw::Texture::WRAP;
		if(!sampler.addressV) sampler.addressV = rw::Texture::WRAP;
		sampler.wrap = sampler.addressU == rw::Texture::WRAP && sampler.addressV == rw::Texture::WRAP;
	}
	const rw::uint32 sourceBlend = rw::GetRenderState(rw::SRCBLEND);
	const rw::uint32 destinationBlend = rw::GetRenderState(rw::DESTBLEND);
	const bool depthTest = rw::GetRenderState(rw::ZTESTENABLE) != 0;
	const bool depthWrite = rw::GetRenderState(rw::ZWRITEENABLE) != 0;
	// Capture the screen effect cull mode once for this batch
	//+ rouz edit (ChatGPT)
	const rw::uint32 cullMode = rw::GetRenderState(rw::CULLMODE);
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Capture stencil rules once for this screen-space batch
	const StencilState stencil = getStencilState();
	//- rouz edit (ChatGPT)
	// Capture alpha-test settings for screen-space textured triangles
	//+ rouz edit (ChatGPT)
	const AlphaTestState alphaTest = getAlphaTestState();
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Use a row-span gradient path for the untextured horizon quads
	if(drawImmediateGradientQuad(projected.data(), primitiveType, vertexCount, indices,
		secondarySampler, cached, sourceBlend, destinationBlend, depthTest, depthWrite,
		cullMode, fog, stencil, alphaTest))
		return;
	//- rouz edit (ChatGPT)
	const int count = indices ? indexCount : vertexCount;
	auto getIndex = [indices](int index){ return indices ? (int)indices[index] : index; };
	// Validate every referenced triangle before handing it to the rasterizer
	auto drawTriangle = [&](int ia, int ib, int ic){
		if(ia < 0 || ib < 0 || ic < 0 || ia >= vertexCount || ib >= vertexCount || ic >= vertexCount)
			return;
		drawImmediateTriangle(projected[ia], projected[ib], projected[ic], cached ? &sampler : nullptr,
			sourceBlend, destinationBlend, depthTest, depthWrite, cullMode, fog, stencil, alphaTest, secondarySampler);
	};
	// Preserve the primitive topology while rasterizing indexed and nonindexed draws
	if(primitiveType == rw::PRIMTYPELINELIST || primitiveType == rw::PRIMTYPEPOLYLINE){
		const int stride = primitiveType == rw::PRIMTYPEPOLYLINE ? 1 : 2;
		for(int i = 0; i+1 < count; i += stride){
			const int a = getIndex(i), b = getIndex(i+1);
			// Apply the captured tests and texture state to each screen-space line
			//+ rouz edit (ChatGPT)
			if(a >= 0 && b >= 0 && a < vertexCount && b < vertexCount)
				drawImmediate2DLine(projected[a], projected[b], cached ? &sampler : nullptr,
					sourceBlend, destinationBlend, depthTest, depthWrite, fog, stencil, alphaTest);
			//- rouz edit (ChatGPT)
		}
	}else if(primitiveType == rw::PRIMTYPETRILIST){
		for(int i = 0; i+2 < count; i += 3)
			drawTriangle(getIndex(i), getIndex(i+1), getIndex(i+2));
	}else if(primitiveType == rw::PRIMTYPETRISTRIP){
		// Preserve alternating face winding for immediate screen-effect strips
		//+ rouz edit (ChatGPT)
		for(int i = 0; i+2 < count; i++){
			if(i & 1)
				drawTriangle(getIndex(i+1), getIndex(i), getIndex(i+2));
			else
				drawTriangle(getIndex(i), getIndex(i+1), getIndex(i+2));
		}
		//- rouz edit (ChatGPT)
	}else if(primitiveType == rw::PRIMTYPETRIFAN){
		for(int i = 1; i+1 < count; i++)
			drawTriangle(getIndex(0), getIndex(i), getIndex(i+1));
	}else if(primitiveType == rw::PRIMTYPEPOINTLIST){
		for(int i = 0; i < count; i++){
			const int index = getIndex(i);
			// Apply the captured tests and texture state to each screen-space point
			//+ rouz edit (ChatGPT)
			if(index >= 0 && index < vertexCount)
				drawImmediate2DPoint(projected[index], cached ? &sampler : nullptr,
					sourceBlend, destinationBlend, depthTest, depthWrite, fog, stencil, alphaTest);
			//- rouz edit (ChatGPT)
		}
	}
}

void RenderImmediate2D(int primitiveType, const void *vertices, int vertexCount,
	const unsigned short *indices, int indexCount)
{
	// Preserve the ordinary single-texture screen primitive path
	renderImmediate2D(primitiveType, vertices, vertexCount, indices, indexCount, nullptr, nullptr, 1.0f, 1.0f);
}

void RenderImmediate2DUV2(int primitiveType, const void *vertices, int vertexCount,
	const unsigned short *indices, int indexCount, const float *secondaryUVs,
	int sourceTextureWidth, int sourceTextureHeight)
{
	// Require the CPU scene snapshot and valid source texture dimensions
	rw::Camera *camera = rw::engine->currentCamera;
	if(!CapturingWorldEffects() || !secondaryUVs || !screenTextureValid || !screenTexturePixels || !camera || !camera->frameBuffer ||
	   camera->frameBuffer->width <= 0 || camera->frameBuffer->height <= 0 ||
	   sourceTextureWidth <= 0 || sourceTextureHeight <= 0)
		return;
	// Use linear clamp sampling to reproduce the screen texture's configured filter
	TextureSampler screenSampler = {};
	screenSampler.pixels = (const rw::uint8*)screenTexturePixels;
	screenSampler.width = screenTextureWidth;
	screenSampler.height = screenTextureHeight;
	screenSampler.stride = screenTextureWidth*4;
	screenSampler.topDown = true;
	screenSampler.addressU = rw::Texture::CLAMP;
	screenSampler.addressV = rw::Texture::CLAMP;
	renderImmediate2D(primitiveType, vertices, vertexCount, indices, indexCount, secondaryUVs,
		&screenSampler, (float)sourceTextureWidth/camera->frameBuffer->width,
		(float)sourceTextureHeight/camera->frameBuffer->height);
}

void RenderScreenRefraction(float left, float top, float right, float bottom,
	int screenWidth, int screenHeight, rw::Raster *maskRaster,
	float leftUOffset, float topVOffset, float rightUOffset, float bottomVOffset, int strength)
{
	// Apply a masked, bilinear sample of the saved scene inside one screen rectangle
	if(!framePending || !CapturingWorldEffects() || !screenTextureValid || !pixels ||
	   width <= 0 || height <= 0 || screenWidth <= 0 || screenHeight <= 0 ||
	   right <= left || bottom <= top || strength <= 0)
		return;
	// Resolve the optional effect mask and the stable scene snapshot once per rectangle
	TextureSampler maskSampler = {};
	const CachedTexture *mask = getCachedEffectTexture(maskRaster);
	// Configure texture sampling when the mask raster is available on the CPU
	if(mask){
		//+ rouz edit (ChatGPT)
		// Preserve the active mip filter when sampling screen-space refraction masks
		const rw::uint32 filter = rw::GetRenderState(rw::TEXTUREFILTER);
		maskSampler.mipLevels = mask->mipLevels;
		maskSampler.mipLevelCount = mask->mipLevelCount;
		maskSampler.mipmapped = filterUsesMipmaps(filter);
		maskSampler.mipLinear = filterBlendsMipLevels(filter);
		//- rouz edit (ChatGPT)
		maskSampler.pixels = mask->image->pixels;
		maskSampler.width = mask->image->width;
		maskSampler.height = mask->image->height;
		maskSampler.stride = mask->image->stride;
		maskSampler.topDown = mask->topDown;
		maskSampler.linear = filterUsesLinear(filter);
		maskSampler.addressU = rw::Texture::CLAMP;
		maskSampler.addressV = rw::Texture::CLAMP;
	}
	TextureSampler sceneSampler = {};
	sceneSampler.pixels = (const rw::uint8*)screenTexturePixels;
	sceneSampler.width = screenTextureWidth;
	sceneSampler.height = screenTextureHeight;
	sceneSampler.stride = screenTextureWidth*4;
	sceneSampler.topDown = true;
	sceneSampler.addressU = rw::Texture::CLAMP;
	sceneSampler.addressV = rw::Texture::CLAMP;
	// Convert logical screen coordinates to the reduced CPU framebuffer
	const float scaleX = (float)width/screenWidth;
	const float scaleY = (float)height/screenHeight;
	const int firstX = std::max(0, (int)std::floor(left*scaleX));
	const int lastX = std::min(width, (int)std::ceil(right*scaleX));
	const int firstY = std::max(0, (int)std::floor(top*scaleY));
	const int lastY = std::min(height, (int)std::ceil(bottom*scaleY));
	//+ rouz edit (ChatGPT)
	// Select a mip level based on the refraction mask footprint at framebuffer resolution
	if(mask)
		maskSampler.lod = estimateScreenTextureLod(maskSampler,
			(right-left)*scaleX, (bottom-top)*scaleY);
	//- rouz edit (ChatGPT)
	const float inverseWidth = 1.0f/(right-left);
	const float inverseHeight = 1.0f/(bottom-top);
	const float blendStrength = std::min(255, strength)*(1.f/255.f);
	// Refract covered pixels while leaving the original scene snapshot unchanged
	for(int y = firstY; y < lastY; y++)
		for(int x = firstX; x < lastX; x++){
			const float screenX = (x+0.5f)/scaleX;
			const float screenY = (y+0.5f)/scaleY;
			const float tx = (screenX-left)*inverseWidth;
			const float ty = (screenY-top)*inverseHeight;
			if(tx < 0.0f || tx > 1.0f || ty < 0.0f || ty > 1.0f)
				continue;
			rw::RGBA maskColor = { 255, 255, 255, 255 };
			if(mask && !sampleTexture(maskSampler, tx, ty, maskColor))
				continue;
			const float amount = blendStrength*maskColor.alpha*(1.f/255.f);
			if(amount <= 0.0f)
				continue;
			const float offsetU = leftUOffset + (rightUOffset-leftUOffset)*tx;
			const float offsetV = topVOffset + (bottomVOffset-topVOffset)*ty;
			rw::RGBA refracted;
			if(!sampleTextureLinear(sceneSampler, screenX/screenWidth+offsetU,
			   screenY/screenHeight+offsetV, refracted))
				continue;
			rw::RGBA &destination = pixels[(size_t)y*width+x];
			destination.red = (rw::uint8)(destination.red*(1.0f-amount)+refracted.red*amount);
			destination.green = (rw::uint8)(destination.green*(1.0f-amount)+refracted.green*amount);
			destination.blue = (rw::uint8)(destination.blue*(1.0f-amount)+refracted.blue*amount);
		}
}

void RenderScreenTexture(float left, float top, float right, float bottom,
	int screenWidth, int screenHeight, rw::Raster *raster,
	int red, int green, int blue, int alpha, bool additive)
{
	// Blend one cached effect texture across a screen-space rectangle
	if(!framePending || !CapturingWorldEffects() || !pixels || !raster ||
	   width <= 0 || height <= 0 || screenWidth <= 0 || screenHeight <= 0 ||
	   right <= left || bottom <= top || alpha <= 0)
		return;
	// Resolve the texture and use clamp addressing for screen overlays
	const CachedTexture *cached = getCachedEffectTexture(raster);
	if(!cached)
		return;
	// Copy the decoded effect raster into the sampler used by the pixel loop
	TextureSampler sampler = {};
	sampler.pixels = cached->image->pixels;
	sampler.width = cached->image->width;
	sampler.height = cached->image->height;
	sampler.stride = cached->image->stride;
	sampler.topDown = cached->topDown;
	//+ rouz edit (ChatGPT)
	// Apply the active mip filter and select a level for the reduced screen rectangle
	const rw::uint32 filter = rw::GetRenderState(rw::TEXTUREFILTER);
	sampler.mipLevels = cached->mipLevels;
	sampler.mipLevelCount = cached->mipLevelCount;
	sampler.mipmapped = filterUsesMipmaps(filter);
	sampler.mipLinear = filterBlendsMipLevels(filter);
	sampler.linear = filterUsesLinear(filter);
	//- rouz edit (ChatGPT)
	sampler.addressU = rw::Texture::CLAMP;
	sampler.addressV = rw::Texture::CLAMP;
	// Convert logical screen coordinates to the reduced CPU framebuffer
	const float scaleX = (float)width/screenWidth;
	const float scaleY = (float)height/screenHeight;
	const int firstX = std::max(0, (int)std::floor(left*scaleX));
	const int lastX = std::min(width, (int)std::ceil(right*scaleX));
	const int firstY = std::max(0, (int)std::floor(top*scaleY));
	const int lastY = std::min(height, (int)std::ceil(bottom*scaleY));
	//+ rouz edit (ChatGPT)
	// Choose a mip level from the texture footprint in the reduced framebuffer
	sampler.lod = estimateScreenTextureLod(sampler, (right-left)*scaleX, (bottom-top)*scaleY);
	//- rouz edit (ChatGPT)
	const float inverseWidth = 1.0f/(right-left);
	const float inverseHeight = 1.0f/(bottom-top);
	const float tint[3] = { std::max(0, std::min(255, red))*(1.f/255.f),
		std::max(0, std::min(255, green))*(1.f/255.f), std::max(0, std::min(255, blue))*(1.f/255.f) };
	// Sample and blend the effect texture over each covered framebuffer pixel
	for(int y = firstY; y < lastY; y++)
		for(int x = firstX; x < lastX; x++){
			const float screenX = (x+0.5f)/scaleX;
			const float screenY = (y+0.5f)/scaleY;
			const float tx = (screenX-left)*inverseWidth;
			const float ty = (screenY-top)*inverseHeight;
			if(tx < 0.0f || tx > 1.0f || ty < 0.0f || ty > 1.0f)
				continue;
			rw::RGBA texel;
			if(!sampleTexture(sampler, tx, ty, texel))
				continue;
			const float amount = texel.alpha*std::min(255, alpha)/(255.0f*255.0f);
			if(amount <= 0.0f)
				continue;
			rw::RGBA &destination = pixels[(size_t)y*width+x];
			rw::uint8 *channels[3] = { &destination.red, &destination.green, &destination.blue };
			const float source[3] = { texel.red*tint[0], texel.green*tint[1], texel.blue*tint[2] };
			for(int channel = 0; channel < 3; channel++){
				const float blended = additive ? *channels[channel]+source[channel]*amount :
					*channels[channel]*(1.0f-amount)+source[channel]*amount;
				*channels[channel] = (rw::uint8)std::max(0.0f, std::min(255.0f, blended));
			}
		}
}

void EndImmediate()
{
	// Release references to an immediate batch after its RenderWare end call
	immediateVertices = nullptr;
	immediateVertexCount = 0;
	//+ rouz edit (ChatGPT)
	// Forget texture-coordinate state after ending this batch
	immediateHasUV = false;
	//- rouz edit (ChatGPT)
}

void BeginFrame(rw::Camera *camera, int displayWidth, int displayHeight,
	const rw::RGBA &top, const rw::RGBA &bottom)
{
	// Retire any previous pending frame before validating the new dimensions
	framePending = false;
	capturingWorldEffects = false;
	//+ rouz edit (ChatGPT)
	// Clear the previous camera owner before preparing a new scene frame
	mainFrameCamera = nullptr;
	//- rouz edit (ChatGPT)
	// Discard stale transparent geometry before collecting this scene's triangles
	//+ rouz edit (ChatGPT)
	transparentTriangles.clear();
	deferTransparentTriangles = false;
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Invalidate the previous scene snapshot before clearing this frame
	screenTextureValid = false;
	//- rouz edit (ChatGPT)
	// Keep the first implementation at a bounded CPU raster resolution
	if(!camera || displayWidth <= 0 || displayHeight <= 0)
		return;
	// Retire decoded textures after streaming stops using them
	cacheFrame++;
	trimTextureCache();
	// Retire decoded effect rasters after they stop appearing in the scene
	//+ rouz edit (ChatGPT)
	for(auto it = effectTextureCache.begin(); it != effectTextureCache.end(); ){
		if(cacheFrame-it->second.lastUsed > 120){
			destroyCachedTexture(&it->second);
			it = effectTextureCache.erase(it);
		}else
			++it;
	}
	//- rouz edit (ChatGPT)
	// Limit the CPU framebuffer width while preserving the window's aspect ratio
	const int newWidth = std::min(displayWidth, std::max(1, maxFramebufferWidthPixels));
	const int newHeight = std::max(1, displayHeight*newWidth/displayWidth);
	if(newWidth != width || newHeight != height){
		// Allocate the CPU framebuffer directly in CIT Alloc with this source location
		const size_t count = (size_t)newWidth*newHeight;
		rw::RGBA *newPixels = (rw::RGBA*)SOFTWARE_CIT_MALLOC(count*sizeof(rw::RGBA));
		float *newDepths = (float*)SOFTWARE_CIT_MALLOC(count*sizeof(float));
		//+ rouz edit (ChatGPT)
		// Allocate one stencil byte for every CPU framebuffer pixel
		rw::uint8 *newStencils = (rw::uint8*)SOFTWARE_CIT_MALLOC(count*sizeof(rw::uint8));
		//- rouz edit (ChatGPT)
		// Allocate one conservative depth bound for each aligned 8 by 8 tile
		//+ rouz edit (ChatGPT)
		const int newTilesAcross = (newWidth+7)/8;
		const size_t tileCount = (size_t)newTilesAcross*((newHeight+7)/8);
		float *newTileDepthBounds = (float*)SOFTWARE_CIT_MALLOC(tileCount*sizeof(float));
		if(!newPixels || !newDepths || !newStencils || !newTileDepthBounds){
			SOFTWARE_CIT_FREE(newPixels);
			SOFTWARE_CIT_FREE(newDepths);
			//+ rouz edit (ChatGPT)
			// Free the stencil allocation with the rest of a failed framebuffer allocation
			SOFTWARE_CIT_FREE(newStencils);
			//- rouz edit (ChatGPT)
			SOFTWARE_CIT_FREE(newTileDepthBounds);
			return;
		}
		//- rouz edit (ChatGPT)
		rw::Raster *newTexture = rw::Raster::create(newWidth, newHeight, 32, rw::Raster::TEXTURE | rw::Raster::C8888);
		if(!newTexture){
			SOFTWARE_CIT_FREE(newPixels);
			SOFTWARE_CIT_FREE(newDepths);
			//+ rouz edit (ChatGPT)
			// Free the stencil allocation when the presentation raster cannot be created
			SOFTWARE_CIT_FREE(newStencils);
			//- rouz edit (ChatGPT)
			SOFTWARE_CIT_FREE(newTileDepthBounds);
			return;
		}
		// Replace the previous framebuffer only after all new storage is ready
		if(texture)
			texture->destroy();
		SOFTWARE_CIT_FREE(pixels);
		SOFTWARE_CIT_FREE(depths);
		//+ rouz edit (ChatGPT)
		// Replace the old stencil storage only after all new buffers are ready
		SOFTWARE_CIT_FREE(stencils);
		//- rouz edit (ChatGPT)
		SOFTWARE_CIT_FREE(tileDepthBounds);
		texture = newTexture;
		pixels = newPixels;
		depths = newDepths;
		//+ rouz edit (ChatGPT)
		// Install stencil storage alongside its framebuffer dimensions
		stencils = newStencils;
		//- rouz edit (ChatGPT)
		tileDepthBounds = newTileDepthBounds;
		tilesAcross = newTilesAcross;
		width = newWidth;
		height = newHeight;
	}
	// Clear the CPU framebuffer to an opaque sky gradient before polygon submission
	for(int y = 0; y < height; y++){
		const float t = height > 1 ? (float)y/(height-1) : 0.0f;
		rw::RGBA clear = {
			(rw::uint8)(top.red + (bottom.red-top.red)*t),
			(rw::uint8)(top.green + (bottom.green-top.green)*t),
			(rw::uint8)(top.blue + (bottom.blue-top.blue)*t),
			255
		};
		std::fill(pixels + (size_t)y*width, pixels + (size_t)(y+1)*width, clear);
	}
	// Reset depth and publish the live CPU buffer for debugger inspection
	std::fill(depths, depths + (size_t)width*height, 0.0f);
	//+ rouz edit (ChatGPT)
	// Clear stencil before the world pass marks hull openings and similar masks
	std::fill(stencils, stencils + (size_t)width*height, (rw::uint8)0);
	//- rouz edit (ChatGPT)
	// Reset tile bounds alongside per-pixel depth for the new scene frame
	std::fill(tileDepthBounds, tileDepthBounds + (size_t)tilesAcross*((height+7)/8), 0.0f);
	framebuffer.width = width;
	framebuffer.height = height;
	framebuffer.pixels = pixels;
	framebuffer.depths = depths;
	framebuffer.pipelineCalls = 0;
	framebuffer.missingCpuGeometry = 0;
	framebuffer.submittedAtomics = 0;
	framebuffer.submittedTriangles = 0;
	framebuffer.depthRejectedTriangles = 0;
	framebuffer.triviallyUnclippedTriangles = 0;
	framebuffer.tileDepthRejectedBlocks = 0;
	framebuffer.offscreenTriangles = 0;
	// Reset projected bounds so the debugger shows the current frame only
	framebuffer.nonFiniteTriangles = 0;
	framebuffer.minScreenX = std::numeric_limits<float>::infinity();
	framebuffer.maxScreenX = -std::numeric_limits<float>::infinity();
	framebuffer.minScreenY = std::numeric_limits<float>::infinity();
	framebuffer.maxScreenY = -std::numeric_limits<float>::infinity();
	framebuffer.degenerateTriangles = 0;
	framebuffer.culledTriangles = 0;
	framebuffer.coveredPixels = 0;
	framebuffer.simpleOpaqueTriangles = 0;
	framebuffer.simpleOpaquePixels = 0;
	framebuffer.texturedTriangles = 0;
	framebuffer.texturedPixels = 0;
	framebuffer.cachedTextures = (unsigned int)textureCache.size();
	framebuffer.textureUploaded = false;
	// Keep the framebuffer active until the end-of-frame presentation
	framePending = true;
	//+ rouz edit (ChatGPT)
	// Bind software capture to the camera that owns this scene framebuffer
	mainFrameCamera = camera;
	//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	deferTransparentTriangles = true;
	//- rouz edit (ChatGPT)
}

void ApplyColourFilter(int mode, int red, int green, int blue, float intensity)
{
	// Reproduce the game's normal or mobile color filter inside the CPU framebuffer
	if(!pixels || !renderColourFilterStage.load(std::memory_order_relaxed) || (mode != 2 && mode != 3))
		return;
	const int colors[3] = { red, green, blue };
	rw::uint8 lookup[3][256];
	for(int channel = 0; channel < 3; channel++){
		const float tint = colors[channel]*intensity*(1.f/255.f);
		const float doubleTint = std::min(1.0f, std::max(0.0f, tint*2.0f));
		const float gain = doubleTint*(30.0f*(1.f/255.f))+2.0f*tint;
		const float mobileMult = (colors[channel]-64)/256.0f+1.4f;
		const float mobileAdd = colors[channel]/1536.0f-0.05f;
		for(int value = 0; value < 256; value++){
			const float original = value*(1.f/255.f);
			float filtered = original;
			if(mode == 2){
				// Match the five color filter shader iterations for each input byte
				for(int step = 0; step < 5; step++)
					filtered = std::max(0.0f, std::min(1.0f, original*(1.0f-30.0f*(1.f/255.f))+filtered*gain));
			}else
				filtered = std::max(0.0f, std::min(1.0f, original*mobileMult+mobileAdd));
			lookup[channel][value] = (rw::uint8)(filtered*255.0f+0.5f);
		}
	}
	// Apply the precomputed channel maps to every CPU pixel before texture upload
	const size_t count = (size_t)width*height;
	for(size_t i = 0; i < count; i++){
		pixels[i].red = lookup[0][pixels[i].red];
		pixels[i].green = lookup[1][pixels[i].green];
		pixels[i].blue = lookup[2][pixels[i].blue];
	}
}

void ApplyPreviousFrameOverlay(int red, int green, int blue, int alpha)
{
	// Composite the previous software frame with the supplied tint and opacity
	if(!framePending || !pixels || !previousFramePixels || !previousFrameValid ||
	   previousFrameWidth != width || previousFrameHeight != height || alpha <= 0)
		return;
	const unsigned int opacity = (unsigned int)std::min(255, alpha);
	const unsigned int tint[3] = {
		(unsigned int)std::max(0, std::min(255, red)),
		(unsigned int)std::max(0, std::min(255, green)),
		(unsigned int)std::max(0, std::min(255, blue))
	};
	const size_t count = (size_t)width*height;
	for(size_t i = 0; i < count; i++){
		const unsigned int current[3] = { pixels[i].red, pixels[i].green, pixels[i].blue };
		const unsigned int previous[3] = { previousFramePixels[i].red, previousFramePixels[i].green, previousFramePixels[i].blue };
		rw::uint8 *destination[3] = { &pixels[i].red, &pixels[i].green, &pixels[i].blue };
		for(int channel = 0; channel < 3; channel++){
			const unsigned int numerator = current[channel]*(255-opacity)*255u+
				previous[channel]*tint[channel]*opacity+32512u;
			*destination[channel] = (rw::uint8)(numerator/65025u);
		}
	}
}

void ApplyPreviousFrameBlur(int red, int green, int blue, bool offset)
{
	// Reproduce the normal postfx blur overlay with CPU history samples
	if(!framePending || !pixels || !previousFramePixels || !previousFrameValid ||
	   previousFrameWidth != width || previousFrameHeight != height)
		return;
	const unsigned int tint[3] = {
		(unsigned int)std::max(0, std::min(255, red)),
		(unsigned int)std::max(0, std::min(255, green)),
		(unsigned int)std::max(0, std::min(255, blue))
	};
	const unsigned int doubledTint[3] = {
		std::min(255u, tint[0]*2u), std::min(255u, tint[1]*2u), std::min(255u, tint[2]*2u)
	};
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++){
			const int shiftedX = offset ? std::max(0, x-1) : x;
			const int shiftedY = offset ? std::max(0, y-1) : y;
			const size_t index = (size_t)y*width+x;
			const size_t shiftedIndex = (size_t)shiftedY*width+shiftedX;
			const unsigned int current[3] = { pixels[index].red, pixels[index].green, pixels[index].blue };
			const unsigned int previous[3] = { previousFramePixels[index].red, previousFramePixels[index].green, previousFramePixels[index].blue };
			const unsigned int shifted[3] = { previousFramePixels[shiftedIndex].red, previousFramePixels[shiftedIndex].green, previousFramePixels[shiftedIndex].blue };
			rw::uint8 *destination[3] = { &pixels[index].red, &pixels[index].green, &pixels[index].blue };
			for(int channel = 0; channel < 3; channel++){
				unsigned int value = (current[channel]*225u*255u+
					shifted[channel]*doubledTint[channel]*30u+32512u)/65025u;
				value = std::min(255u, value+(previous[channel]*tint[channel]+127u)/255u);
				value = std::min(255u, value+((offset ? shifted[channel] : previous[channel])*tint[channel]+127u)/255u);
				*destination[channel] = (rw::uint8)value;
			}
		}
}

void SavePreviousFrame(bool enabled)
{
	// Keep a world-only history frame for motion effects that need temporal input
	if(!enabled || !framePending || !pixels){
		previousFrameValid = false;
		return;
	}
	if(previousFramePixels && (previousFrameWidth != width || previousFrameHeight != height)){
		SOFTWARE_CIT_FREE(previousFramePixels);
		previousFramePixels = nullptr;
		previousFrameValid = false;
	}
	if(!previousFramePixels){
		previousFramePixels = (rw::RGBA*)SOFTWARE_CIT_MALLOC((size_t)width*height*sizeof(rw::RGBA));
		if(!previousFramePixels){
			previousFrameWidth = previousFrameHeight = 0;
			previousFrameValid = false;
			return;
		}
		previousFrameWidth = width;
		previousFrameHeight = height;
	}
	std::memcpy(previousFramePixels, pixels, (size_t)width*height*sizeof(rw::RGBA));
	previousFrameValid = true;
}

void Present()
{
	// Consume this CPU frame before attempting the final texture upload
	if(!framePending)
		return;
	framePending = false;
	capturingWorldEffects = false;
	immediateVertices = nullptr;
	immediateVertexCount = 0;
	// Upload the CPU framebuffer as a texture and draw it over the scene
	if(!texture || !pixels)
		return;
	rw::uint8 *destination = texture->lock(0, rw::Raster::LOCKWRITE | rw::Raster::LOCKNOFETCH);
	if(!destination)
		return;
	// Convert CPU rows to the texture's row origin during the final upload
	for(int y = 0; y < height; y++)
		std::memcpy(destination + (size_t)y*texture->stride, &pixels[(size_t)(height-1-y)*width], (size_t)width*sizeof(rw::RGBA));
	texture->unlock(0);
	framebuffer.textureUploaded = true;
	framebuffer.totalUploadedFrames++;

	// Preserve render states for the caller after the final upload
	const rw::uint32 oldZTest = rw::GetRenderState(rw::ZTESTENABLE);
	const rw::uint32 oldZWrite = rw::GetRenderState(rw::ZWRITEENABLE);
	const rw::uint32 oldVertexAlpha = rw::GetRenderState(rw::VERTEXALPHA);
	const rw::uint32 oldSrcBlend = rw::GetRenderState(rw::SRCBLEND);
	const rw::uint32 oldDestBlend = rw::GetRenderState(rw::DESTBLEND);
	const rw::uint32 oldCull = rw::GetRenderState(rw::CULLMODE);
	const rw::uint32 oldFilter = rw::GetRenderState(rw::TEXTUREFILTER);
	const rw::uint32 oldAlphaFunc = rw::GetRenderState(rw::ALPHATESTFUNC);
	const rw::uint32 oldFog = rw::GetRenderState(rw::FOGENABLE);
	rw::Raster *oldTexture = (rw::Raster*)rw::GetRenderStatePtr(rw::TEXTURERASTER);
	rw::SetRenderState(rw::ZTESTENABLE, 0);
	rw::SetRenderState(rw::ZWRITEENABLE, 0);
	rw::SetRenderState(rw::VERTEXALPHA, 1);
	rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
	rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
	rw::SetRenderState(rw::CULLMODE, rw::CULLNONE);
	rw::SetRenderState(rw::TEXTUREFILTER, rw::Texture::NEAREST);
	rw::SetRenderState(rw::ALPHATESTFUNC, rw::ALPHAALWAYS);
	rw::SetRenderState(rw::FOGENABLE, 0);
	rw::SetRenderStatePtr(rw::TEXTURERASTER, texture);

	// Present a full screen textured quad through the existing Im2D path
	RwIm2DVertex vertices[4];
	const float xs[4] = { 0.0f, 0.0f, (float)rw::engine->currentCamera->frameBuffer->width, (float)rw::engine->currentCamera->frameBuffer->width };
	const float ys[4] = { 0.0f, (float)rw::engine->currentCamera->frameBuffer->height, (float)rw::engine->currentCamera->frameBuffer->height, 0.0f };
	const float us[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	const float vs[4] = { 0.0f, 1.0f, 1.0f, 0.0f };
	for(int i = 0; i < 4; i++){
		vertices[i].setScreenX(xs[i]);
		vertices[i].setScreenY(ys[i]);
		vertices[i].setScreenZ(rw::im2d::GetNearZ());
		vertices[i].setCameraZ(rw::engine->currentCamera->nearPlane);
		vertices[i].setRecipCameraZ(1.0f/rw::engine->currentCamera->nearPlane);
		vertices[i].setColor(255, 255, 255, 255);
		vertices[i].setU(us[i], 1.0f);
		vertices[i].setV(vs[i], 1.0f);
	}
	rw::im2d::RenderPrimitive(rw::PRIMTYPETRIFAN, vertices, 4);
	rw::SetRenderStatePtr(rw::TEXTURERASTER, oldTexture);
	rw::SetRenderState(rw::FOGENABLE, oldFog);
	rw::SetRenderState(rw::ALPHATESTFUNC, oldAlphaFunc);
	rw::SetRenderState(rw::TEXTUREFILTER, oldFilter);
	rw::SetRenderState(rw::CULLMODE, oldCull);
	rw::SetRenderState(rw::DESTBLEND, oldDestBlend);
	rw::SetRenderState(rw::SRCBLEND, oldSrcBlend);
	rw::SetRenderState(rw::VERTEXALPHA, oldVertexAlpha);
	rw::SetRenderState(rw::ZWRITEENABLE, oldZWrite);
	rw::SetRenderState(rw::ZTESTENABLE, oldZTest);
}

}
#endif
//- rouz edit (ChatGPT)
