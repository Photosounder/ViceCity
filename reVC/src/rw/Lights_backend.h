//+ rouz edit (ChatGPT)
#pragma once
#include "Lights.h"
#include <stdint.h>
#include <stdbool.h>
typedef struct LightsVector {float x,y,z;} LightsVector;
typedef struct LightsMatrix {LightsVector right;uint32_t flags;LightsVector up;uint32_t pad1;LightsVector at;uint32_t pad2;LightsVector pos;uint32_t pad3;} LightsMatrix;
typedef struct LightsVectorOut {float *x,*y,*z;} LightsVectorOut;
enum {LIGHTS_AMBIENT,LIGHTS_DIRECTIONAL};
#ifdef __cplusplus
extern "C" {
#endif
bool Lights_HostBlurColours(void);
void Lights_HostColour(void *light,const LightsColour *colour);
void Lights_HostRotate(void *frame,const LightsVector *axis,float angle);
void Lights_HostTransform(void *frame,const LightsMatrix *matrix);
LightsVectorOut Lights_HostAt(void *matrix);
float Lights_HostMultiplier(void);
int Lights_HostBrightness(void);
void * Lights_HostCreate(int kind);
void Lights_HostDestroy(void *light);
void Lights_HostFlags(void *light,int active);
void Lights_HostRadius(void *light,float radius);
void * Lights_HostCreateFrame(void);
void Lights_HostDestroyFrame(void *frame);
void * Lights_HostFrame(void *light);
void Lights_HostSetFrame(void *light,void *frame);
void * Lights_HostMatrix(void *frame);
void Lights_HostUpdateMatrix(void *matrix);
void Lights_HostUpdateObjects(void *frame);
void Lights_HostAdd(void *world,void *light);
void Lights_HostRemove(void *world,void *light);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
