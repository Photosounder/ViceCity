//+ rouz edit (Claude)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct VectorMath3 {float x,y,z;} VectorMath3;

// Matrices are the row-major float[4][4] layout of CMatrix (right, forward, up, position rows)
void VectorMath_Normalise(VectorMath3 *v);
VectorMath3 VectorMath_Cross(VectorMath3 a, VectorMath3 b);
VectorMath3 VectorMath_MatrixTimesVector3x3(const float m[4][4], VectorMath3 v);
VectorMath3 VectorMath_VectorTimesMatrix3x3(VectorMath3 v, const float m[4][4]);
VectorMath3 VectorMath_MatrixTimesVector(const float m[4][4], VectorMath3 v);

// Transform n points into 16-byte output slots (xyz written, w untouched) from input points spaced by in_stride bytes
void VectorMath_TransformPoints(float *out, int n, const float m[4][4], const void *in, int in_stride);

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
