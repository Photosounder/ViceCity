//+ rouz edit (Claude)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

// All functions work on the row-major float[4][4] layout of CMatrix (right, forward, up, position rows)
void MatrixMath_SetUnity(float m[4][4]);
void MatrixMath_ResetOrientation(float m[4][4]);
void MatrixMath_SetScale(float m[4][4], float s);
void MatrixMath_SetTranslate(float m[4][4], float x, float y, float z);
void MatrixMath_SetRotateXOnly(float m[4][4], float angle);
void MatrixMath_SetRotateYOnly(float m[4][4], float angle);
void MatrixMath_SetRotateZOnly(float m[4][4], float angle);
void MatrixMath_SetRotate(float m[4][4], float xAngle, float yAngle, float zAngle);
void MatrixMath_RotateX(float m[4][4], float x);
void MatrixMath_RotateY(float m[4][4], float y);
void MatrixMath_RotateZ(float m[4][4], float z);
void MatrixMath_Rotate(float m[4][4], float x, float y, float z);
void MatrixMath_Reorthogonalise(float m[4][4]);
void MatrixMath_Multiply(float out[4][4], const float m1[4][4], const float m2[4][4]);
void MatrixMath_Invert(float dst[4][4], const float src[4][4]);

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
