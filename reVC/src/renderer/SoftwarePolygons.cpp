//+ rouz edit (ChatGPT)
#if defined(REVC_SOFTWARE_POLYGONS) && defined(RW_GL3)
#include "rwcore.h"
#include "rpworld.h" // rouz edit (ChatGPT)
#include "SoftwarePolygons.h"
#include "Lights.h" // rouz edit (ChatGPT)

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <unordered_map> // rouz edit (ChatGPT)
#include <vector>

//+ rouz edit (ChatGPT)
extern "C" void *cita_win_malloc(size_t, const char *, const char *, int);
extern "C" void cita_win_free(void *, const char *, const char *, int);
#define SOFTWARE_CIT_MALLOC(size) cita_win_malloc((size), __FILE__, __func__, __LINE__)
#define SOFTWARE_CIT_FREE(ptr) cita_win_free((ptr), __FILE__, __func__, __LINE__)
//- rouz edit (ChatGPT)

namespace SoftwarePolygons {
//+ rouz edit (ChatGPT)
Framebuffer framebuffer = {};
//- rouz edit (ChatGPT)
namespace {

struct ScreenVertex {
	float x, y, z;
	float cameraX, cameraY; // rouz edit (ChatGPT)
	float u, v; // rouz edit (ChatGPT)
	rw::RGBA color; // rouz edit (ChatGPT)
};

//+ rouz edit (ChatGPT)
struct CachedTexture {
	rw::Raster *raster;
	rw::Image *image;
	int width, height, format;
	char name[32], mask[32];
	bool topDown;
	unsigned long long lastUsed;
};

struct TextureSampler {
	const rw::uint8 *pixels;
	int width, height, stride;
	bool topDown;
	rw::Texture::Addressing addressU, addressV;
};
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
int width, height;
rw::RGBA *pixels;
float *depths;
//- rouz edit (ChatGPT)
rw::Raster *texture;
rw::ObjPipeline *pipeline;
rw::ObjPipeline *originalPipeline; // rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
std::unordered_map<rw::Texture*, CachedTexture> textureCache;
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
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: dxt = 3; break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: dxt = 5; break;
		default: return nullptr;
		}
		if(raster->width < 4 || raster->height < 4 || raster->width%4 || raster->height%4)
			return nullptr;
		rw::uint8 *compressed = raster->lock(0, rw::Raster::LOCKREAD);
		if(!compressed)
			return nullptr;
		rw::Image *image = rw::Image::create(raster->width, raster->height, 32);
		if(image){
			image->allocate();
			image->setPixelsDXT(dxt, compressed);
			// Keep RGB DXT1's fourth color opaque instead of treating it as cutout alpha
			if(native->internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
				for(int y = 0; y < image->height; y++)
					for(int x = 0; x < image->width; x++)
						image->pixels[(size_t)y*image->stride + (size_t)x*4 + 3] = 255;
		}
		raster->unlock(0);
		topDown = false;
		return image;
	}
	// Convert ordinary GL textures to RGBA rows with the image origin at the top
	const int format = raster->format & 0xF00;
	if(format != rw::Raster::C8888 && format != rw::Raster::C888 && format != rw::Raster::C1555)
		return nullptr;
	rw::Image *image = raster->toImage();
	if(image && image->depth != 32)
		image->convertTo32();
	topDown = true;
	return image;
}

void trimTextureCache()
{
	// Release streamed textures that have not been used recently
	for(auto it = textureCache.begin(); it != textureCache.end(); ){
		if(cacheFrame - it->second.lastUsed > 120){
			if(it->second.image){
				textureCacheBytes -= (size_t)it->second.image->stride*it->second.image->height;
				it->second.image->destroy();
			}
			it = textureCache.erase(it);
		}else
			++it;
	}
	// Bound retained texture copies while preserving the most recently used images
	while(textureCacheBytes > textureCacheLimit && textureCache.size() > 1){
		auto oldest = std::min_element(textureCache.begin(), textureCache.end(),
			[](const auto &a, const auto &b){ return a.second.lastUsed < b.second.lastUsed; });
		if(oldest->second.image){
			textureCacheBytes -= (size_t)oldest->second.image->stride*oldest->second.image->height;
			oldest->second.image->destroy();
		}
		textureCache.erase(oldest);
	}
}

const CachedTexture *getCachedTexture(rw::Texture *source)
{
	// Reuse decoded pixels only while the material still refers to the same raster
	if(!source || !source->raster || source->raster->platform != rw::PLATFORM_GL3)
		return nullptr;
	rw::Raster *raster = source->raster;
	if(raster->type != rw::Raster::TEXTURE || raster->width <= 0 || raster->height <= 0)
		return nullptr;
	auto found = textureCache.find(source);
	if(found != textureCache.end()){
		CachedTexture &entry = found->second;
		if(entry.raster == raster && entry.width == raster->width && entry.height == raster->height &&
		   entry.format == raster->format && std::memcmp(entry.name, source->name, 32) == 0 &&
		   std::memcmp(entry.mask, source->mask, 32) == 0){
			entry.lastUsed = cacheFrame;
			return entry.image ? &entry : nullptr;
		}
		if(entry.image){
			textureCacheBytes -= (size_t)entry.image->stride*entry.image->height;
			entry.image->destroy();
		}
		textureCache.erase(found);
	}
	// Read and cache the first CPU copy of this static material texture
	CachedTexture entry = {};
	entry.raster = raster;
	entry.width = raster->width;
	entry.height = raster->height;
	entry.format = raster->format;
	std::memcpy(entry.name, source->name, 32);
	std::memcpy(entry.mask, source->mask, 32);
	entry.lastUsed = cacheFrame;
	entry.image = readTextureImage(raster, entry.topDown);
	if(entry.image)
		textureCacheBytes += (size_t)entry.image->stride*entry.image->height;
	auto inserted = textureCache.emplace(source, entry);
	trimTextureCache();
	// Resolve the entry again because the cache trim can evict an older element
	(void)inserted;
	found = textureCache.find(source);
	return found != textureCache.end() && found->second.image ? &found->second : nullptr;
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

bool sampleTexture(const TextureSampler &sampler, float u, float v, rw::RGBA &texel)
{
	// Sample the nearest texel after applying RenderWare address modes
	if(!std::isfinite(u) || !std::isfinite(v))
		return false;
	u = addressCoordinate(u, sampler.addressU);
	v = addressCoordinate(1.0f-v, sampler.addressV);
	if(u < 0.0f || v < 0.0f)
		return false;
	const int x = std::min(sampler.width-1, (int)(u*sampler.width));
	int y = std::min(sampler.height-1, (int)(v*sampler.height));
	// Convert the GL sampler's bottom origin to ordinary CPU image rows
	if(sampler.topDown)
		y = sampler.height-1-y;
	const rw::uint8 *pixel = sampler.pixels + (size_t)y*sampler.stride + (size_t)x*4;
	texel = { pixel[0], pixel[1], pixel[2], pixel[3] };
	return true;
}
//- rouz edit (ChatGPT)

float edge(const ScreenVertex &a, const ScreenVertex &b, float x, float y)
{
	return (x-a.x)*(b.y-a.y) - (y-a.y)*(b.x-a.x);
}

//+ rouz edit (ChatGPT)
struct PixelPlane {
	float row, dx, dy;
};
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
int clipDepthPlane(const ScreenVertex *input, int count, ScreenVertex *output, float plane, bool keepGreater)
{
	// Clip in camera space so the intersection keeps valid UVs and prelight colors
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
			intersection.color.red = (rw::uint8)(previous.color.red + t*((float)current.color.red-previous.color.red));
			intersection.color.green = (rw::uint8)(previous.color.green + t*((float)current.color.green-previous.color.green));
			intersection.color.blue = (rw::uint8)(previous.color.blue + t*((float)current.color.blue-previous.color.blue));
			intersection.color.alpha = (rw::uint8)(previous.color.alpha + t*((float)current.color.alpha-previous.color.alpha));
			output[outputCount++] = intersection;
		}
		if(currentInside)
			output[outputCount++] = current;
		previous = current;
		previousInside = currentInside;
	}
	return outputCount;
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
struct TriangleRaster {
	float rowW0, rowW1, w0dx, w0dy, w1dx, w1dy;
	PixelPlane inverseDepth, uOverZ, vOverZ;
	PixelPlane redOverZ, greenOverZ, blueOverZ;
	float ambientRed, ambientGreen, ambientBlue;
	rw::RGBA color, flatShaded;
	const TextureSampler *sampler;
	int left, top, textured, prelit;
	unsigned int coveredCount, texturedCount;
};

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

void shadeRect(TriangleRaster *raster, int rectLeft, int rectTop, int rectRight, int rectBottom, int fullCoverage)
{
	// Start each edge and attribute at the rectangle's top left pixel
	const float offsetX = (float)(rectLeft-raster->left);
	const float offsetY = (float)(rectTop-raster->top);
	float rectW0 = raster->rowW0+raster->w0dx*offsetX+raster->w0dy*offsetY;
	float rectW1 = raster->rowW1+raster->w1dx*offsetX+raster->w1dy*offsetY;
	float rowInvz = pixelPlaneAt(&raster->inverseDepth, offsetX, offsetY);
	float rowUoz = pixelPlaneAt(&raster->uOverZ, offsetX, offsetY);
	float rowVoz = pixelPlaneAt(&raster->vOverZ, offsetX, offsetY);
	float rowRoz = pixelPlaneAt(&raster->redOverZ, offsetX, offsetY);
	float rowGoz = pixelPlaneAt(&raster->greenOverZ, offsetX, offsetY);
	float rowBoz = pixelPlaneAt(&raster->blueOverZ, offsetX, offsetY);
	unsigned int coveredCount = 0, texturedCount = 0;

	// Advance edges and attributes across each row and shade pixels that pass depth
	for(int y = rectTop; y <= rectBottom; y++){
		float w0 = rectW0, w1 = rectW1;
		float invz = rowInvz, uoz = rowUoz, voz = rowVoz;
		float roz = rowRoz, goz = rowGoz, boz = rowBoz;
		for(int x = rectLeft; x <= rectRight;
			x++, w0 += raster->w0dx, w1 += raster->w1dx, invz += raster->inverseDepth.dx,
			uoz += raster->uOverZ.dx, voz += raster->vOverZ.dx,
			roz += raster->redOverZ.dx, goz += raster->greenOverZ.dx, boz += raster->blueOverZ.dx){
			if(!fullCoverage && (w0 < 0.0f || w1 < 0.0f || 1.0f-w0-w1 < 0.0f))
				continue;
			const size_t index = (size_t)y*width+x;
			if(!(invz > depths[index]))
				continue;
			// Apply perspective corrected prelight and the material color
			const float z = (raster->prelit || raster->textured) ? 1.0f/invz : 0.0f;
			rw::RGBA shaded = raster->flatShaded;
			if(raster->prelit){
				shaded.red = (rw::uint8)(raster->color.red*std::max(0.0f, std::min(1.0f, roz*z/255.0f+raster->ambientRed)));
				shaded.green = (rw::uint8)(raster->color.green*std::max(0.0f, std::min(1.0f, goz*z/255.0f+raster->ambientGreen)));
				shaded.blue = (rw::uint8)(raster->color.blue*std::max(0.0f, std::min(1.0f, boz*z/255.0f+raster->ambientBlue)));
			}
			// Sample cutout textures before committing the depth value
			if(raster->textured){
				rw::RGBA texel;
				if(!sampleTexture(*raster->sampler, uoz*z, voz*z, texel))
					continue;
				shaded.red = (rw::uint8)((unsigned)shaded.red*texel.red/255);
				shaded.green = (rw::uint8)((unsigned)shaded.green*texel.green/255);
				shaded.blue = (rw::uint8)((unsigned)shaded.blue*texel.blue/255);
				shaded.alpha = (rw::uint8)((unsigned)shaded.alpha*texel.alpha/255);
			}
			if(shaded.alpha < 128)
				continue;
			depths[index] = invz;
			pixels[index] = shaded;
			pixels[index].alpha = 255;
			coveredCount++;
			if(raster->textured)
				texturedCount++;
		}
		rectW0 += raster->w0dy;
		rectW1 += raster->w1dy;
		rowInvz += raster->inverseDepth.dy;
		rowUoz += raster->uOverZ.dy;
		rowVoz += raster->vOverZ.dy;
		rowRoz += raster->redOverZ.dy;
		rowGoz += raster->greenOverZ.dy;
		rowBoz += raster->blueOverZ.dy;
	}
	// Accumulate diagnostic counters after the rectangle is finished
	raster->coveredCount += coveredCount;
	raster->texturedCount += texturedCount;
}

void drawTriangle(const ScreenVertex &a, const ScreenVertex &b, const ScreenVertex &c,
	const rw::RGBA &color, bool hasVertexColors, const rw::RGBAf &ambient, float surfaceAmbient,
	rw::Texture *source, const CachedTexture *cached)
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
	raster.ambientRed = ambient.red*surfaceAmbient;
	raster.ambientGreen = ambient.green*surfaceAmbient;
	raster.ambientBlue = ambient.blue*surfaceAmbient;
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
		sampler.topDown = cached->topDown;
		sampler.addressU = (rw::Texture::Addressing)((source->filterAddressing >> 8) & 0xF);
		sampler.addressV = (rw::Texture::Addressing)((source->filterAddressing >> 12) & 0xF);
		raster.sampler = &sampler;
		raster.uOverZ = makePixelPlane(&raster, a.u*az, b.u*bz, c.u*cz);
		raster.vOverZ = makePixelPlane(&raster, a.v*az, b.v*bz, c.v*cz);
	}
	if(raster.prelit){
		raster.redOverZ = makePixelPlane(&raster, a.color.red*az, b.color.red*bz, c.color.red*cz);
		raster.greenOverZ = makePixelPlane(&raster, a.color.green*az, b.color.green*bz, c.color.green*cz);
		raster.blueOverZ = makePixelPlane(&raster, a.color.blue*az, b.color.blue*bz, c.color.blue*cz);
	}else{
		raster.flatShaded.red = (rw::uint8)(color.red*std::max(0.0f, std::min(1.0f, raster.ambientRed)));
		raster.flatShaded.green = (rw::uint8)(color.green*std::max(0.0f, std::min(1.0f, raster.ambientGreen)));
		raster.flatShaded.blue = (rw::uint8)(color.blue*std::max(0.0f, std::min(1.0f, raster.ambientBlue)));
	}

	// Draw small triangles directly and classify eight pixel blocks on larger ones
	if((right-left+1)*(bottom-top+1) <= 256)
		shadeRect(&raster, left, top, right, bottom, false);
	else{
		const int blockSize = 8;
		const float w2dx = -raster.w0dx-raster.w1dx;
		const float w2dy = -raster.w0dy-raster.w1dy;
		for(int blockTop = top; blockTop <= bottom; blockTop += blockSize)
			for(int blockLeft = left; blockLeft <= right; blockLeft += blockSize){
				const int blockRight = std::min(right, blockLeft+blockSize-1);
				const int blockBottom = std::min(bottom, blockTop+blockSize-1);
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
				shadeRect(&raster, blockLeft, blockTop, blockRight, blockBottom, fullCoverage);
			}
	}
	// Update framebuffer counters once after the triangle is complete
	framebuffer.coveredPixels += raster.coveredCount;
	framebuffer.totalCoveredPixels += raster.coveredCount;
	framebuffer.texturedPixels += raster.texturedCount;
}
//- rouz edit (ChatGPT)

void renderAtomic(rw::ObjPipeline *, rw::Atomic *atomic)
{
	// Count every pipeline entry before checking whether CPU geometry is available
	framebuffer.pipelineCalls++; // rouz edit (ChatGPT)
	framebuffer.totalPipelineCalls++; // rouz edit (ChatGPT)
	// Skip geometry without CPU vertices and cameras outside the scene pass
	rw::Camera *camera = rw::engine->currentCamera;
	rw::Geometry *geometry = atomic->geometry;
	if(!camera || !geometry || !geometry->triangles || !geometry->morphTargets ||
	   !geometry->morphTargets[0].vertices || width == 0 || height == 0){
		framebuffer.missingCpuGeometry++; // rouz edit (ChatGPT)
		return;
	}
	// Count atomics that reach the CPU rasterizer
	framebuffer.submittedAtomics++; // rouz edit (ChatGPT)
	// Use the same ambient light source as the OpenGL building and default pipelines
	//+ rouz edit (ChatGPT)
	rw::RGBAf ambient = {};
	if((geometry->flags & rw::Geometry::LIGHT) && pAmbient)
		ambient = pAmbient->color;
	//- rouz edit (ChatGPT)

	// Project vertices with RenderWare's world to screen matrix used by camera culling
	std::vector<ScreenVertex> transformed((size_t)geometry->numVertices);
	const rw::Matrix *model = atomic->getFrame()->getLTM();
	// Build the same skin matrices as the GL pipeline for animated clump atomics
	rw::Skin *skin = rw::Skin::get(geometry);
	rw::HAnimHierarchy *hierarchy = skin ? rw::Skin::getHierarchy(atomic) : nullptr;
	std::vector<rw::Matrix> boneMatrices;
	if(skin && hierarchy && hierarchy->matrices && skin->numBones == hierarchy->numNodes){
		boneMatrices.resize((size_t)skin->numBones);
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
		}
	}
	for(int i = 0; i < geometry->numVertices; i++){
		rw::V3d world, projected;
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
		rw::V3d::transformPoints(&world, &local, 1, model);
		rw::V3d::transformPoints(&projected, &world, 1, &camera->viewMatrix);
		// Capture the first vertex and projection inputs for debugger inspection
		if(framebuffer.submittedAtomics == 1 && i == 0){
			//+ rouz edit (ChatGPT)
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
		transformed[i].cameraX = projected.x; // rouz edit (ChatGPT)
		transformed[i].cameraY = projected.y; // rouz edit (ChatGPT)
		// Preserve the first UV set for perspective corrected material sampling
		if(geometry->numTexCoordSets > 0 && geometry->texCoords[0]){
			transformed[i].u = geometry->texCoords[0][i].u;
			transformed[i].v = geometry->texCoords[0][i].v;
		}else
			transformed[i].u = transformed[i].v = 0.0f;
		// Preserve each vertex's prelight color for smooth face shading
		transformed[i].color = geometry->colors ? geometry->colors[i] : rw::RGBA{ 255, 255, 255, 255 };
		if(projected.z > camera->nearPlane){
			// Convert normalized camera coordinates to CPU framebuffer pixels
			const float divisor = camera->projection == rw::Camera::PERSPECTIVE ? projected.z : 1.0f;
			transformed[i].x = width*projected.x/divisor;
			transformed[i].y = height*projected.y/divisor;
			// Capture the first framebuffer coordinate after perspective division
			if(framebuffer.submittedAtomics == 1 && i == 0){
				framebuffer.firstScreen[0] = transformed[i].x; // rouz edit (ChatGPT)
				framebuffer.firstScreen[1] = transformed[i].y; // rouz edit (ChatGPT)
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

	// Rasterize each source triangle with its material color and texture
	for(int i = 0; i < geometry->numTriangles; i++){
		const rw::Triangle &tri = geometry->triangles[i];
		if(tri.v[0] >= geometry->numVertices || tri.v[1] >= geometry->numVertices || tri.v[2] >= geometry->numVertices)
			continue;
		const ScreenVertex &a = transformed[tri.v[0]];
		const ScreenVertex &b = transformed[tri.v[1]];
		const ScreenVertex &c = transformed[tri.v[2]];
		// Clip partially visible triangles at the camera depth planes
		//+ rouz edit (ChatGPT)
		const ScreenVertex sourceVertices[3] = { a, b, c };
		ScreenVertex nearVertices[6], farVertices[6];
		const int nearCount = clipDepthPlane(sourceVertices, 3, nearVertices, camera->nearPlane, true);
		const int farCount = nearCount >= 3
			? clipDepthPlane(nearVertices, nearCount, farVertices, camera->farPlane, false) : 0;
		if(farCount < 3){
			framebuffer.depthRejectedTriangles++; // rouz edit (ChatGPT)
			continue;
		}
		for(int vertex = 0; vertex < farCount; vertex++){
			const float divisor = camera->projection == rw::Camera::PERSPECTIVE ? farVertices[vertex].z : 1.0f;
			farVertices[vertex].x = width*farVertices[vertex].cameraX/divisor;
			farVertices[vertex].y = height*farVertices[vertex].cameraY/divisor;
		}
		//- rouz edit (ChatGPT)
		rw::RGBA color = { 190, 190, 190, 255 };
		float surfaceAmbient = 1.0f; // rouz edit (ChatGPT)
		rw::Texture *source = nullptr;
		if(tri.matId < geometry->matList.numMaterials && geometry->matList.materials[tri.matId]){
			rw::Material *material = geometry->matList.materials[tri.matId];
			color = geometry->flags & rw::Geometry::MODULATE ? material->color : rw::RGBA{ 255, 255, 255, 255 }; // rouz edit (ChatGPT)
			surfaceAmbient = material->surfaceProps.ambient; // rouz edit (ChatGPT)
			source = material->texture;
		}
		// Count triangles that pass the camera depth checks
		framebuffer.submittedTriangles++; // rouz edit (ChatGPT)
		// Fetch a CPU texture only for geometry with UVs and a textured material
		const CachedTexture *cached = geometry->numTexCoordSets > 0 && geometry->texCoords[0]
			? getCachedTexture(source) : nullptr;
		if(cached)
			framebuffer.texturedTriangles++;
		// Rasterize the clipped polygon as a triangle fan
		for(int vertex = 1; vertex+1 < farCount; vertex++)
			drawTriangle(farVertices[0], farVertices[vertex], farVertices[vertex+1], color,
				geometry->colors != nullptr, ambient, surfaceAmbient, source, cached); // rouz edit (ChatGPT)
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
	originalPipeline = rw::engine->driver[rw::PLATFORM_GL3]->defaultPipeline; // rouz edit (ChatGPT)
	rw::engine->driver[rw::PLATFORM_GL3]->defaultPipeline = pipeline;
}

//+ rouz edit (ChatGPT)
void Shutdown()
{
	// Release CPU and GPU framebuffer storage before the RenderWare engine stops
	// Release decoded material textures while the RenderWare allocator is active
	for(auto &item : textureCache)
		if(item.second.image)
			item.second.image->destroy();
	textureCache.clear();
	textureCacheBytes = 0;
	cacheFrame = 0;
	if(texture){
		texture->destroy();
		texture = nullptr;
	}
	SOFTWARE_CIT_FREE(pixels); // rouz edit (ChatGPT)
	SOFTWARE_CIT_FREE(depths); // rouz edit (ChatGPT)
	pixels = nullptr;
	depths = nullptr;
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
//- rouz edit (ChatGPT)

void Attach(rw::Atomic *atomic)
{
	// Route world atomics with explicit custom pipelines through the CPU path
	if(atomic && pipeline)
		atomic->pipeline = pipeline;
}

//+ rouz edit (ChatGPT)
void RenderAtomic(rw::Atomic *atomic)
{
	// Submit default atomic render callbacks directly to the CPU rasterizer
	if(atomic)
		renderAtomic(pipeline, atomic);
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
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
//- rouz edit (ChatGPT)

void BeginFrame(int displayWidth, int displayHeight, const rw::RGBA &top, const rw::RGBA &bottom) // rouz edit (ChatGPT)
{
	// Keep the first implementation at a bounded CPU raster resolution
	if(displayWidth <= 0 || displayHeight <= 0)
		return;
	// Retire decoded textures after streaming stops using them
	cacheFrame++;
	trimTextureCache();
	const int newWidth = std::min(displayWidth, 640);
	const int newHeight = std::max(1, displayHeight*newWidth/displayWidth);
	if(newWidth != width || newHeight != height){
		// Allocate the CPU framebuffer directly in CIT Alloc with this source location
		const size_t count = (size_t)newWidth*newHeight;
		rw::RGBA *newPixels = (rw::RGBA*)SOFTWARE_CIT_MALLOC(count*sizeof(rw::RGBA)); // rouz edit (ChatGPT)
		float *newDepths = (float*)SOFTWARE_CIT_MALLOC(count*sizeof(float)); // rouz edit (ChatGPT)
		if(!newPixels || !newDepths){
			SOFTWARE_CIT_FREE(newPixels); // rouz edit (ChatGPT)
			SOFTWARE_CIT_FREE(newDepths); // rouz edit (ChatGPT)
			return;
		}
		rw::Raster *newTexture = rw::Raster::create(newWidth, newHeight, 32, rw::Raster::TEXTURE | rw::Raster::C8888);
		if(!newTexture){
			SOFTWARE_CIT_FREE(newPixels); // rouz edit (ChatGPT)
			SOFTWARE_CIT_FREE(newDepths); // rouz edit (ChatGPT)
			return;
		}
		// Replace the previous framebuffer only after all new storage is ready
		if(texture)
			texture->destroy();
		SOFTWARE_CIT_FREE(pixels); // rouz edit (ChatGPT)
		SOFTWARE_CIT_FREE(depths); // rouz edit (ChatGPT)
		texture = newTexture;
		pixels = newPixels;
		depths = newDepths;
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
		std::fill(pixels + (size_t)y*width, pixels + (size_t)(y+1)*width, clear); // rouz edit (ChatGPT)
	}
	// Reset depth and publish the live CPU buffer for debugger inspection
	std::fill(depths, depths + (size_t)width*height, 0.0f); // rouz edit (ChatGPT)
	framebuffer.width = width;
	framebuffer.height = height;
	framebuffer.pixels = pixels; // rouz edit (ChatGPT)
	framebuffer.depths = depths; // rouz edit (ChatGPT)
	framebuffer.pipelineCalls = 0; // rouz edit (ChatGPT)
	framebuffer.missingCpuGeometry = 0; // rouz edit (ChatGPT)
	framebuffer.submittedAtomics = 0;
	framebuffer.submittedTriangles = 0;
	framebuffer.depthRejectedTriangles = 0; // rouz edit (ChatGPT)
	framebuffer.offscreenTriangles = 0; // rouz edit (ChatGPT)
	// Reset projected bounds so the debugger shows the current frame only
	framebuffer.nonFiniteTriangles = 0; // rouz edit (ChatGPT)
	framebuffer.minScreenX = std::numeric_limits<float>::infinity(); // rouz edit (ChatGPT)
	framebuffer.maxScreenX = -std::numeric_limits<float>::infinity(); // rouz edit (ChatGPT)
	framebuffer.minScreenY = std::numeric_limits<float>::infinity(); // rouz edit (ChatGPT)
	framebuffer.maxScreenY = -std::numeric_limits<float>::infinity(); // rouz edit (ChatGPT)
	framebuffer.degenerateTriangles = 0; // rouz edit (ChatGPT)
	framebuffer.coveredPixels = 0; // rouz edit (ChatGPT)
	framebuffer.texturedTriangles = 0; // rouz edit (ChatGPT)
	framebuffer.texturedPixels = 0; // rouz edit (ChatGPT)
	framebuffer.cachedTextures = (unsigned int)textureCache.size(); // rouz edit (ChatGPT)
	framebuffer.textureUploaded = false; // rouz edit (ChatGPT)
}

void Present()
{
	// Upload the CPU framebuffer as a texture and draw it over the scene
	if(!texture || !pixels) // rouz edit (ChatGPT)
		return;
	rw::uint8 *destination = texture->lock(0, rw::Raster::LOCKWRITE | rw::Raster::LOCKNOFETCH);
	if(!destination)
		return;
	for(int y = 0; y < height; y++)
		std::memcpy(destination + (size_t)y*texture->stride, &pixels[(size_t)(height-1-y)*width], (size_t)width*sizeof(rw::RGBA));
	texture->unlock(0);
	framebuffer.textureUploaded = true; // rouz edit (ChatGPT)
	framebuffer.totalUploadedFrames++; // rouz edit (ChatGPT)

	// Preserve render states used by later effects and HUD drawing
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
	RwIm2DVertex vertices[4]; // rouz edit (ChatGPT)
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
