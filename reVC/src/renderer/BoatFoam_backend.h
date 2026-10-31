//+ rouz edit (ChatGPT)
#pragma once
#include "BoatFoam.h"
typedef struct BoatFoamVector { float x,y,z; } BoatFoamVector;
typedef struct BoatFoamMatrix { float rx,ry,rz,fx,fy,fz,ux,uy,uz,px,py,pz; } BoatFoamMatrix;
#ifdef __cplusplus
extern "C" {
#endif
BoatFoamVector BoatFoam_BackendCameraForward(void);
BoatFoamVector BoatFoam_BackendMatrixPosition(void *matrix);
BoatFoamVector BoatFoam_BackendMatrixForward(void *matrix);
BoatFoamVector BoatFoam_BackendMatrixRight(void *matrix);
BoatFoamVector BoatFoam_BackendMatrixUp(void *matrix);
BoatFoamMatrix BoatFoam_BackendReadMatrix(void *matrix);
void BoatFoam_BackendParticle(int32_t type,BoatFoamVector position,float size);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
