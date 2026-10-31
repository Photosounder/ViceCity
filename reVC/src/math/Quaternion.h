//+ rouz edit (ChatGPT)
#pragma once
typedef struct CQuaternion {float x,y,z,w;} CQuaternion;
typedef struct QuaternionVectorRef {const float *x,*y,*z;} QuaternionVectorRef;
typedef struct QuaternionVectorOut {float *x,*y,*z;} QuaternionVectorOut;
typedef struct QuaternionMatrixRef {QuaternionVectorRef right,up,at;} QuaternionMatrixRef;
typedef struct QuaternionMatrixOut {QuaternionVectorOut right,up,at;} QuaternionMatrixOut;
#ifdef __cplusplus
extern "C" {
#endif
CQuaternion Quaternion_Make(float x,float y,float z,float w);
float Quaternion_Magnitude(const CQuaternion *self);
float Quaternion_MagnitudeSqr(const CQuaternion *self);
void Quaternion_Normalise(CQuaternion *self);
void Quaternion_Multiply(CQuaternion *self,const CQuaternion *q1,const CQuaternion *q2);
void Quaternion_Invert(CQuaternion *self);
const CQuaternion * Quaternion_AddAssign(CQuaternion *self,const CQuaternion *right);
const CQuaternion * Quaternion_SubAssign(CQuaternion *self,const CQuaternion *right);
const CQuaternion * Quaternion_ScaleAssign(CQuaternion *self,float right);
const CQuaternion * Quaternion_DivideAssign(CQuaternion *self,float right);
CQuaternion Quaternion_Negate(CQuaternion value);
float Quaternion_Dot(CQuaternion q1,CQuaternion q2);
CQuaternion Quaternion_Add(CQuaternion left,CQuaternion right);
CQuaternion Quaternion_Sub(CQuaternion left,CQuaternion right);
CQuaternion Quaternion_Scale(CQuaternion left,float right);
CQuaternion Quaternion_LeftScale(float left,CQuaternion right);
CQuaternion Quaternion_Divide(CQuaternion left,float right);
void Quaternion_Slerp(CQuaternion *self,const CQuaternion *q1,const CQuaternion *q2,float theta,float invSin,float t);
void Quaternion_GetAxisAngle(const CQuaternion *self,QuaternionVectorOut axis,float *angle);
void Quaternion_SetAxisAngle(CQuaternion *self,QuaternionVectorRef axis,float angle);
void Quaternion_GetMatrix(const CQuaternion *self,QuaternionMatrixOut matrix);
void Quaternion_SetMatrix(CQuaternion *self,QuaternionMatrixRef matrix);
void Quaternion_GetEuler(const CQuaternion *self,float *f1,float *f2,float *f3);
void Quaternion_SetEuler(CQuaternion *self,float f1,float f2,float f3);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
