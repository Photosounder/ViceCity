//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "MatrixData.h"

// The matrix and original position keep their native types in C++ and the plain data layout in C
#ifdef __cplusplus
typedef CMatrix PlaceableMatrix;
typedef xyz_t PlaceableXYZ;
#else
typedef CMatrixData PlaceableMatrix;
typedef struct PlaceableXYZ {double x, y, z;} PlaceableXYZ;
#endif

typedef struct CPlaceableData {
	PlaceableMatrix m_matrix;
	PlaceableXYZ orig_pos;			// keep track of the original location for buildings and such
	int orig_pos_set;
} CPlaceableData;

#ifdef __cplusplus
extern "C" {
#endif
void Placeable_Init(CPlaceableData *self);
void Placeable_SetHeading(CPlaceableData *self, float angle);
void Placeable_SetOrientation(CPlaceableData *self, float x, float y, float z);
bool Placeable_IsWithinArea2D(const CPlaceableData *self, float x1, float y1, float x2, float y2);
bool Placeable_IsWithinArea3D(const CPlaceableData *self, float x1, float y1, float z1, float x2, float y2, float z2);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
