//+ rouz edit (ChatGPT)
#pragma once
#include "Markers3D.h"
typedef struct Markers3DFields {
    uint16_t *m_nType;
    bool *m_bIsUsed;
    bool *m_bFindZOnNextPlacement;
    uint32_t *m_nIdentifier;
    uint8_t *red;
    uint8_t *green;
    uint8_t *blue;
    uint8_t *alpha;
    uint16_t *m_nPulsePeriod;
    int16_t *m_nRotateRate;
    uint32_t *m_nStartTime;
    float *m_fPulseFraction;
    float *m_fStdSize;
    float *m_fSize;
    float *m_fBrightness;
    float *m_fCameraRange;
} Markers3DFields;
#ifdef __cplusplus
extern "C" {
#endif
void * Markers3D_BackendMarker(int32_t index);
Markers3DFields Markers3D_BackendFields(void *marker);
void * Markers3D_BackendAtomic(void *marker);
void Markers3D_BackendSetAtomic(void *marker,void *atomic);
void * Markers3D_BackendClump(int32_t index);
void Markers3D_BackendSetClump(int32_t index,void *clump);
void Markers3D_BackendDestroyClump(void *clump);
Markers3DVector2D Markers3D_BackendPlayer(void);
bool Markers3D_BackendCollision(Markers3DVector2D pos);
float Markers3D_BackendGround(float x,float y,float z);
float Markers3D_BackendReadLegacy(const void *pos,int32_t axis);
void Markers3D_BackendWriteLegacy(void *pos,int32_t axis,float value);
Markers3DVector Markers3D_BackendPosition(void *marker);
void Markers3D_BackendSetPosition(void *marker,Markers3DVector pos);
void Markers3D_BackendSetZ(void *marker,float z);
void Markers3D_BackendSetTranslate(void *marker,Markers3DVector pos);
void Markers3D_BackendRotateZ(void *marker,float angle);
void Markers3D_BackendRotateX(void *marker,float angle);
void Markers3D_BackendUpdateRW(void *marker);
void Markers3D_BackendDirectional(void);
float Markers3D_BackendLOD(void);
void Markers3D_BackendCorona(void *marker,float size,float drawDist);
int32_t Markers3D_BackendFindTxd(const char *name);
void Markers3D_BackendPushTxd(void);
void Markers3D_BackendSetTxd(int32_t slot);
void Markers3D_BackendPopTxd(void);
void * Markers3D_BackendLoad(const char *name);
void * Markers3D_BackendCameraFrame(void *camera);
void * Markers3D_BackendFrameMatrix(void *frame);
float Markers3D_BackendNearClip(void *camera);
float Markers3D_BackendFarClip(void *camera);
Markers3DVector Markers3D_BackendMatrixPosition(void *matrix);
Markers3DVector Markers3D_BackendMatrixForward(void *matrix);
void * Markers3D_PlaceMarkerLegacyInput(uint32_t identifier,uint16_t type,void *pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate);
void Markers3D_PlaceMarkerSetLegacyInput(uint32_t identifier,uint16_t type,void *pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate);
typedef struct Markers3DBasis { float *axis[3][3]; } Markers3DBasis;
enum { MARKERS3D_GEOMETRY_MODULATE_MATERIAL_COLOR=0x40 };
void Markers3D_BackendUnity(void *marker);
void * Markers3D_BackendLastAtomic(void *clump);
void * Markers3D_BackendCloneAtomic(void *atomic);
void * Markers3D_BackendCreateFrame(void);
void Markers3D_BackendSetFrame(void *atomic,void *frame);
void Markers3D_BackendDefaultCallback(void *atomic);
void * Markers3D_BackendAtomicGeometry(void *atomic);
uint32_t Markers3D_BackendGeometryFlags(void *geometry);
void Markers3D_BackendSetGeometryFlags(void *geometry,uint32_t flags);
void * Markers3D_BackendGeometryMaterial(void *geometry,int32_t index);
void Markers3D_BackendSetMaterial(void *marker,void *material);
void Markers3D_BackendAttachMarker(void *marker,void *matrix);
void * Markers3D_BackendAtomicFrame(void *atomic);
void Markers3D_BackendDestroyAtomic(void *atomic);
void Markers3D_BackendDestroyFrame(void *frame);
void Markers3D_BackendMaterialColor(void *marker);
void * Markers3D_BackendAttachment(void *marker);
void Markers3D_BackendAttachMatrix(void *matrix,void *attachment);
Markers3DBasis Markers3D_BackendMatrixBasis(void *matrix);
void Markers3D_BackendPublishMatrix(void *matrix);
void Markers3D_BackendUpdateFrame(void *frame);
void Markers3D_BackendBright(float value);
void Markers3D_BackendZWrite(bool enabled);
void Markers3D_BackendDrawAtomic(void *atomic);
void Markers3D_BackendRestoreLighting(void);
void Markers3D_BackendWithRenderMatrix(void *marker,void (*render)(void *,void *));
bool Markers3D_RecordAdd(void *marker,uint32_t identifier,uint16_t type,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate);
void Markers3D_RecordDelete(void *marker);
void Markers3D_RecordRender(void *marker);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
