//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#if defined(LIBRW) && (defined(RW_GL3) || defined(RW_WDGL)) && !defined(RW_OPENGL)
#define RW_OPENGL
#endif
#ifdef __cplusplus
extern "C" {
#endif


#pragma once

extern bool bDebugRenderGroups;
extern bool gPS2alphaTest;
extern bool gBackfaceCulling;

void RwHelper_SetTextureRaster(void *raster); // rouz edit (ChatGPT)
void *RwHelper_TextureRaster(void *texture); // rouz edit (ChatGPT)
void *RwHelper_ReadTexture(const char *name,const char *mask); // rouz edit (ChatGPT)
void RwHelper_DestroyTexture(void *texture); // rouz edit (ChatGPT)
void OpenCharsetSafe(void);
void CreateDebugFont(void);
void DestroyDebugFont(void);
void ObrsPrintfString(const char *str, short x, short y);
void FlushObrsPrintfs(void);
void DefinedState(void);
void SetCullMode(uint32_t mode);
void *RwHelper_GetFirstChild(void *frame);
void *GetFirstObject(void *frame);
void *GetFirstAtomic(void *clump);
void *GetFirstTexture(void *txd);

bool IsClumpSkinned(void *clump);
void *GetAnimHierarchyFromSkinClump(void *clump);	
void *GetAnimHierarchyFromClump(void *clump);	
void SkinGetBonePositionsToTable(void *clump, void *boneTable);
void *HAnimAnimationCreateForHierarchy(void *hier);
void RenderSkeleton(void *hier);

int32_t Im2DRenderQuad(float x1, float y1, float x2, float y2, float z, float recipCamZ, float uvOffset);
typedef struct RwHelperSphereRef {float *x,*y,*z,*radius;} RwHelperSphereRef;
void *RwHelper_ClumpSphereView(void *clump,RwHelperSphereRef sphere,bool useLTM);
void *RpClumpGetBoundingSphere(void *clump, void *sphere, bool useLTM);

#include "TexRead.h"

#include "ClumpRead.h"

void CameraSize(void *camera,
                void *rect,
                float viewWindow,
                float aspectRatio);
void CameraDestroy(void *camera);
void *CameraCreate(int32_t width,
                       int32_t height,
                       int32_t zBuffer);

					   


#if defined(FIX_BUGS) && defined (GTA_PC)
void SetAlphaTest(uint32_t alpharef);
void RestoreAlphaTest(void);
#else
#define SetAlphaTest(a) (0)
#define RestoreAlphaTest() (0)
#endif

void PushRendergroup(const char *name);
void PopRendergroup(void);
void *RwHelper_ConvertPlatformAtomic(void *atomic,void *data);
#ifdef __cplusplus
}
#ifdef RWCORE_H
RpAtomic *AtomicRemoveAnimFromSkinCB(RpAtomic *atomic, void *data);
RpAtomic *ConvertPlatformAtomic(RpAtomic *atomic, void *data);
#endif
#endif
//- rouz edit (ChatGPT)
