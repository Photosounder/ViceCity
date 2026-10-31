//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ColBox.h"
#include "ColSphere.h"
#include "ColLine.h"
#include "ColPoint.h"
#include "ColTriangle.h"
#include "LinkList.h"
typedef struct CColModel {
    CSphere boundingSphere;
    CBox boundingBox;
    int16_t numSpheres,numBoxes,numTriangles;
    int8_t numLines;
    uint8_t level;
    bool ownsCollisionVolumes;
    CColSphere *spheres;
    CColLine *lines;
    CColBox *boxes;
    CompressedVector *vertices;
    CColTriangle *triangles;
    CColTrianglePlane *trianglePlanes;
} CColModel;
#ifdef __cplusplus
extern "C" {
#endif
void ColModel_Init(CColModel *self);
void ColModel_Destroy(CColModel *self);
void ColModel_RemoveCollisionVolumes(CColModel *self);
void ColModel_CalculateTrianglePlanes(CColModel *self);
void ColModel_RemoveTrianglePlanes(CColModel *self);
void ColModel_SetLinkPtr(CColModel *self,CLink *lptr);
CLink * ColModel_GetLinkPtr(CColModel *self);
CollisionVector3 ColModel_GetTrianglePoint(const CColModel *self,int32_t i);
CColModel * ColModel_Assign(CColModel *self,const CColModel *other);
CColModel ColModel_InitStatic(CColModel *self,void (*cleanup)(void));
void ColModel_DestroyArray(CColModel *models,int32_t count);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
