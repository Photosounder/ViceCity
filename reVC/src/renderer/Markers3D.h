//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct Markers3DVector { float x,y,z; } Markers3DVector;
typedef struct Markers3DVector2D { float x,y; } Markers3DVector2D;
enum { MARKERTYPE_0=0,MARKERTYPE_ARROW,MARKERTYPE_2,MARKERTYPE_3,MARKERTYPE_CYLINDER,NUMMARKERTYPES,MARKERTYPE_INVALID=0x101 };
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t Markers3D_NumActiveMarkers;
void Markers3D_Init(void);
void Markers3D_Shutdown(void);
void Markers3D_Render(void);
void Markers3D_RenderForEnvMap(void *camera);
void Markers3D_Update(void);
void *Markers3D_PlaceMarker(uint32_t identifier,uint16_t type,Markers3DVector *pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate);
void Markers3D_PlaceMarkerSet(uint32_t identifier,uint16_t type,Markers3DVector *pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate);
#ifdef __cplusplus
}
struct C3dMarker;
class CVector;
C3dMarker *Markers3D_PlaceMarkerLegacy(uint32_t identifier,uint16_t type,CVector &pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate);
void Markers3D_PlaceMarkerSetLegacy(uint32_t identifier,uint16_t type,CVector &pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate);
#endif
//- rouz edit (ChatGPT)
