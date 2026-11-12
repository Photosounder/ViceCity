//+ rouz edit (Claude)
#include "common.h"
#include "VectorMath.h"

static inline VectorMath3 ToC(const CVector &v)
{
	// Copy the components into the plain C record
	VectorMath3 r = {v.x, v.y, v.z};
	return r;
}

static inline CVector FromC(VectorMath3 v)
{
	// Copy the components back into the native vector
	return CVector(v.x, v.y, v.z);
}

#ifdef GTA_PS2
#error "VectorMath assumes the float[4][4] CMatrix layout"
#endif

void
CVector::Normalise(void)
{
	// Delegate to the C implementation
	VectorMath3 v = ToC(*this);
	VectorMath_Normalise(&v);
	*this = FromC(v);
}

CVector
CrossProduct(const CVector &v1, const CVector &v2)
{
	// Delegate to the C implementation
	return FromC(VectorMath_Cross(ToC(v1), ToC(v2)));
}

CVector
Multiply3x3(const CMatrix &mat, const CVector &vec)
{
	// TODO: VU0 code
	return FromC(VectorMath_MatrixTimesVector3x3(mat.f, ToC(vec)));
}

CVector
Multiply3x3(const CVector &vec, const CMatrix &mat)
{
	// Delegate to the C implementation
	return FromC(VectorMath_VectorTimesMatrix3x3(ToC(vec), mat.f));
}

CVector
operator*(const CMatrix &mat, const CVector &vec)
{
	// TODO: VU0 code
	return FromC(VectorMath_MatrixTimesVector(mat.f, ToC(vec)));
}
//- rouz edit (Claude)
