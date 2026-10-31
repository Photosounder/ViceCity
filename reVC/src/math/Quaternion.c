//+ rouz edit (ChatGPT)
#include "Quaternion.h"
#include <math.h>
#define QUATERNION_PI 3.1415926535897932384626433832795f
#define QUATERNION_TWOPI (2.0f*QUATERNION_PI)
CQuaternion Quaternion_Make(float x,float y,float z,float w)
{
    // Preserve the original quaternion operation and its field evaluation order
    const CQuaternion value={x,y,z,w};return value;
}
float Quaternion_Magnitude(const CQuaternion *self)
{
    // Preserve the original quaternion operation and its field evaluation order
    return sqrtf(Quaternion_MagnitudeSqr(self));
}
float Quaternion_MagnitudeSqr(const CQuaternion *self)
{
    // Preserve the original quaternion operation and its field evaluation order
    return self->x*self->x+self->y*self->y+self->z*self->z+self->w*self->w;
}
void Quaternion_Normalise(CQuaternion *self)
{
    // Preserve the original quaternion operation and its field evaluation order
    float sq = Quaternion_MagnitudeSqr(self);
	if (sq == 0.0f)
		self->w = 1.0f;
	else {
		float invsqrt = (1.0f/sqrtf(sq));
		self->x *= invsqrt;
		self->y *= invsqrt;
		self->z *= invsqrt;
		self->w *= invsqrt;
	}
}
void Quaternion_Multiply(CQuaternion *self,const CQuaternion *q1,const CQuaternion *q2)
{
    // Preserve the original quaternion operation and its field evaluation order
    self->x = (q2->z * q1->y) - (q1->z * q2->y) + (q1->x * q2->w) + (q2->x * q1->w);
	self->y = (q2->x * q1->z) - (q1->x * q2->z) + (q1->y * q2->w) + (q2->y * q1->w);
	self->z = (q2->y * q1->x) - (q1->y * q2->x) + (q1->z * q2->w) + (q2->z * q1->w);
	self->w = (q2->w * q1->w) - (q2->x * q1->x) - (q2->y * q1->y) - (q2->z * q1->z);
}
void Quaternion_Invert(CQuaternion *self)
{
    // Preserve the original quaternion operation and its field evaluation order
    self->x=-self->x;self->y=-self->y;self->z=-self->z;
}
const CQuaternion * Quaternion_AddAssign(CQuaternion *self,const CQuaternion *right)
{
    // Preserve the original quaternion operation and its field evaluation order
    self->x+=right->x;self->y+=right->y;self->z+=right->z;self->w+=right->w;return self;
}
const CQuaternion * Quaternion_SubAssign(CQuaternion *self,const CQuaternion *right)
{
    // Preserve the original quaternion operation and its field evaluation order
    self->x-=right->x;self->y-=right->y;self->z-=right->z;self->w-=right->w;return self;
}
const CQuaternion * Quaternion_ScaleAssign(CQuaternion *self,float right)
{
    // Preserve the original quaternion operation and its field evaluation order
    self->x*=right;self->y*=right;self->z*=right;self->w*=right;return self;
}
const CQuaternion * Quaternion_DivideAssign(CQuaternion *self,float right)
{
    // Preserve the original quaternion operation and its field evaluation order
    self->x/=right;self->y/=right;self->z/=right;self->w/=right;return self;
}
CQuaternion Quaternion_Negate(CQuaternion value)
{
    // Preserve the original quaternion operation and its field evaluation order
    return Quaternion_Make(-value.x,-value.y,-value.z,-value.w);
}
float Quaternion_Dot(CQuaternion q1,CQuaternion q2)
{
    // Preserve the original quaternion operation and its field evaluation order
    return q1.x*q2.x+q1.y*q2.y+q1.z*q2.z+q1.w*q2.w;
}
CQuaternion Quaternion_Add(CQuaternion left,CQuaternion right)
{
    // Preserve the original quaternion operation and its field evaluation order
    return Quaternion_Make(left.x+right.x,left.y+right.y,left.z+right.z,left.w+right.w);
}
CQuaternion Quaternion_Sub(CQuaternion left,CQuaternion right)
{
    // Preserve the original quaternion operation and its field evaluation order
    return Quaternion_Make(left.x-right.x,left.y-right.y,left.z-right.z,left.w-right.w);
}
CQuaternion Quaternion_Scale(CQuaternion left,float right)
{
    // Preserve the original quaternion operation and its field evaluation order
    return Quaternion_Make(left.x*right,left.y*right,left.z*right,left.w*right);
}
CQuaternion Quaternion_LeftScale(float left,CQuaternion right)
{
    // Preserve the original quaternion operation and its field evaluation order
    return Quaternion_Make(left*right.x,left*right.y,left*right.z,left*right.w);
}
CQuaternion Quaternion_Divide(CQuaternion left,float right)
{
    // Preserve the original quaternion operation and its field evaluation order
    return Quaternion_Make(left.x/right,left.y/right,left.z/right,left.w/right);
}
void Quaternion_Slerp(CQuaternion *self,const CQuaternion *q1,const CQuaternion *q2,float theta,float invSin,float t)
{
    // Preserve the original quaternion operation and its field evaluation order
    if (theta == 0.0f)
		*self = *q2;
	else {
		float w1, w2;
		if (theta > QUATERNION_PI / 2) {
			theta = QUATERNION_PI - theta;
			w1 = sinf((1.0f - t) * theta) * invSin;
			w2 = -sinf(t * theta) * invSin;
		} else {
			w1 = sinf((1.0f - t) * theta) * invSin;
			w2 = sinf(t * theta) * invSin;
		}
		// TODO: VU0 code
		*self = Quaternion_Add(Quaternion_LeftScale(w1,*q1),Quaternion_LeftScale(w2,*q2));
	}
}
void Quaternion_GetAxisAngle(const CQuaternion *self,QuaternionVectorOut axis,float *angle)
{
    // Preserve the original quaternion operation and its field evaluation order
    *angle = acosf(self->w);
	float s = sinf(*angle);

	(*axis.x) = self->x * (1.0f / s);
	(*axis.y) = self->y * (1.0f / s);
	(*axis.z) = self->z * (1.0f / s);
}
void Quaternion_SetAxisAngle(CQuaternion *self,QuaternionVectorRef axis,float angle)
{
    // Preserve the original quaternion operation and its field evaluation order
    float halfCos = cosf(angle * 0.5f);
	float halfSin = sinf(angle * 0.5f);
	self->x = (*axis.x) * halfSin;
	self->y = (*axis.y) * halfSin;
	self->z = (*axis.z) * halfSin;
	self->w = halfCos;
}
void Quaternion_GetMatrix(const CQuaternion *self,QuaternionMatrixOut matrix)
{
    // Preserve the original quaternion operation and its field evaluation order
    float x2 = self->x + self->x;
	float y2 = self->y + self->y;
	float z2 = self->z + self->z;

	float x_2x = self->x * x2;
	float x_2y = self->x * y2;
	float x_2z = self->x * z2;
	float y_2y = self->y * y2;
	float y_2z = self->y * z2;
	float z_2z = self->z * z2;
	float w_2x = self->w * x2;
	float w_2y = self->w * y2;
	float w_2z = self->w * z2;

	(*matrix.right.x) = 1.0f - (y_2y + z_2z);
	(*matrix.up.x) = x_2y - w_2z;
	(*matrix.at.x) = x_2z + w_2y;
	(*matrix.right.y) = x_2y + w_2z;
	(*matrix.up.y) = 1.0f - (x_2x + z_2z);
	(*matrix.at.y) = y_2z - w_2x;
	(*matrix.right.z) = x_2z - w_2y;
	(*matrix.up.z) = y_2z + w_2x;
	(*matrix.at.z) = 1.0f - (x_2x + y_2y);
}
void Quaternion_SetMatrix(CQuaternion *self,QuaternionMatrixRef matrix)
{
    // Preserve the original quaternion operation and its field evaluation order
    float f, s, m;

	f = (*matrix.up.y) + (*matrix.right.x) + (*matrix.at.z);
	if (f >= 0.0f) {
		s = sqrtf(f + 1.0f);
		self->w = 0.5f * s;
		m = 0.5f / s;
		self->x = ((*matrix.up.z) - (*matrix.at.y)) * m;
		self->y = ((*matrix.at.x) - (*matrix.right.z)) * m;
		self->z = ((*matrix.right.y) - (*matrix.up.x)) * m;
		return;
	}

	f = (*matrix.right.x) - (*matrix.up.y) - (*matrix.at.z);
	if (f >= 0.0f) {
		s = sqrtf(f + 1.0f);
		self->x = 0.5f * s;
		m = 0.5f / s;
		self->y = ((*matrix.up.x) + (*matrix.right.y)) * m;
		self->z = ((*matrix.at.x) + (*matrix.right.z)) * m;
		self->w = ((*matrix.up.z) - (*matrix.at.y)) * m;
		return;
	}

	f = (*matrix.up.y) - (*matrix.right.x) - (*matrix.at.z);
	if (f >= 0.0f) {
		s = sqrtf(f + 1.0f);
		self->y = 0.5f * s;
		m = 0.5f / s;
		self->w = ((*matrix.at.x) - (*matrix.right.z)) * m;
		self->x = ((*matrix.up.x) - (*matrix.right.y)) * m;
		self->z = ((*matrix.at.y) + (*matrix.up.z)) * m;
		return;
	}

	f = (*matrix.at.z) - ((*matrix.up.y) + (*matrix.right.x));
	s = sqrtf(f + 1.0f);
	self->z = 0.5f * s;
	m = 0.5f / s;
	self->w = ((*matrix.right.y) - (*matrix.up.x)) * m;
	self->x = ((*matrix.at.x) + (*matrix.right.z)) * m;
	self->y = ((*matrix.at.y) + (*matrix.up.z)) * m;
}
void Quaternion_GetEuler(const CQuaternion *self,float *f1,float *f2,float *f3)
{
    // Preserve the original quaternion operation and its field evaluation order
    struct {float x,y,z;} right,up,at;
    const QuaternionMatrixOut output={{&right.x,&right.y,&right.z},{&up.x,&up.y,&up.z},{&at.x,&at.y,&at.z}};

	Quaternion_GetMatrix(self,output);
	*f3 = atan2f(right.y, up.y);
	if (*f3 < 0.0f)
		*f3 += QUATERNION_TWOPI;
	float s = sinf(*f3);
	float c = cosf(*f3);
	*f1 = atan2f(-at.y, s * right.y + c * up.y);
	if (*f1 < 0.0f)
		*f1 += QUATERNION_TWOPI;
	*f2 = atan2f(-(right.z * c - up.z * s), right.x * c - up.x * s);
	if (*f2 < 0.0f)
		*f2 += QUATERNION_TWOPI;
}
void Quaternion_SetEuler(CQuaternion *self,float f1,float f2,float f3)
{
    // Preserve the original quaternion operation and its field evaluation order
    float c1 = cosf(f1 * 0.5f);
	float c2 = cosf(f2 * 0.5f);
	float c3 = cosf(f3 * 0.5f);
	float s1 = sinf(f1 * 0.5f);
	float s2 = sinf(f2 * 0.5f);
	float s3 = sinf(f3 * 0.5f);
	self->x = ((c2 * c1) * s3) - ((s2 * s1) * c3);
	self->y = ((s1 * c2) * c3) + ((s2 * c1) * s3);
	self->z = ((s2 * c1) * c3) - ((s1 * c2) * s3);
	self->w = ((c2 * c1) * c3) + ((s2 * s1) * s3);
}
//- rouz edit (ChatGPT)
