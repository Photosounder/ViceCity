//+ rouz edit (Claude)
#include "common.h"
#include "MatrixMath.h"

CMatrix::CMatrix(void)
{
	m_attachment = nil;
	m_hasRwMatrix = false;
}

CMatrix::CMatrix(CMatrix const &m)
{
	m_attachment = nil;
	m_hasRwMatrix = false;
	*this = m;
}

CMatrix::CMatrix(RwMatrix *matrix, bool owner)
{
	m_attachment = nil;
	Attach(matrix, owner);
}

CMatrix::~CMatrix(void)
{
	if (m_hasRwMatrix && m_attachment)
		RwMatrixDestroy(m_attachment);
}

void
CMatrix::Attach(RwMatrix *matrix, bool owner)
{
#ifdef FIX_BUGS
	if (m_attachment && m_hasRwMatrix)
#else
	if (m_hasRwMatrix && m_attachment)
#endif
		RwMatrixDestroy(m_attachment);
	m_attachment = matrix;
	m_hasRwMatrix = owner;
	Update();
}

void
CMatrix::AttachRW(RwMatrix *matrix, bool owner)
{
	if (m_hasRwMatrix && m_attachment)
		RwMatrixDestroy(m_attachment);
	m_attachment = matrix;
	m_hasRwMatrix = owner;
	UpdateRW();
}

void
CMatrix::Detach(void)
{
	if (m_hasRwMatrix && m_attachment)
		RwMatrixDestroy(m_attachment);
	m_attachment = nil;
}

void
CMatrix::Update(void)
{
	GetRight() = m_attachment->right;
	GetForward() = m_attachment->up;
	GetUp() = m_attachment->at;
	GetPosition() = m_attachment->pos;
}

void
CMatrix::UpdateRW(void)
{
	if (m_attachment) {
		m_attachment->right = GetRight();
		m_attachment->up = GetForward();
		m_attachment->at = GetUp();
		m_attachment->pos = GetPosition();
		RwMatrixUpdate(m_attachment);
	}
}

void
CMatrix::operator=(CMatrix const &rhs)
{
	memcpy(this, &rhs, sizeof(f));
	if (m_attachment)
		UpdateRW();
}

void
CMatrix::CopyOnlyMatrix(const CMatrix &other)
{
	memcpy(this, &other, sizeof(f));
}

CMatrix &
CMatrix::operator+=(CMatrix const &rhs)
{
	GetRight() += rhs.GetRight();
	GetForward() += rhs.GetForward();
	GetUp() += rhs.GetUp();
	GetPosition() += rhs.GetPosition();
	return *this;
}

void
CMatrix::SetUnity(void)
{
	// Delegate to the C implementation
	MatrixMath_SetUnity(f);
}

void
CMatrix::ResetOrientation(void)
{
	// Delegate to the C implementation
	MatrixMath_ResetOrientation(f);
}

void
CMatrix::SetScale(float s)
{
	// Delegate to the C implementation
	MatrixMath_SetScale(f, s);
}

void
CMatrix::SetTranslate(float x, float y, float z)
{
	// Delegate to the C implementation
	MatrixMath_SetTranslate(f, x, y, z);
}

void
CMatrix::SetRotateXOnly(float angle)
{
	// Delegate to the C implementation
	MatrixMath_SetRotateXOnly(f, angle);
}

void
CMatrix::SetRotateYOnly(float angle)
{
	// Delegate to the C implementation
	MatrixMath_SetRotateYOnly(f, angle);
}

void
CMatrix::SetRotateZOnly(float angle)
{
	// Delegate to the C implementation
	MatrixMath_SetRotateZOnly(f, angle);
}

void
CMatrix::SetRotateX(float angle)
{
	SetRotateXOnly(angle);
	px = 0.0f;
	py = 0.0f;
	pz = 0.0f;
}


void
CMatrix::SetRotateY(float angle)
{
	SetRotateYOnly(angle);
	px = 0.0f;
	py = 0.0f;
	pz = 0.0f;
}

void
CMatrix::SetRotateZ(float angle)
{
	SetRotateZOnly(angle);
	px = 0.0f;
	py = 0.0f;
	pz = 0.0f;
}

void
CMatrix::SetRotate(float xAngle, float yAngle, float zAngle)
{
	// Delegate to the C implementation
	MatrixMath_SetRotate(f, xAngle, yAngle, zAngle);
}

void
CMatrix::RotateX(float x)
{
	// Delegate to the C implementation
	MatrixMath_RotateX(f, x);
}

void
CMatrix::RotateY(float y)
{
	// Delegate to the C implementation
	MatrixMath_RotateY(f, y);
}

void
CMatrix::RotateZ(float z)
{
	// Delegate to the C implementation
	MatrixMath_RotateZ(f, z);
}

void
CMatrix::Rotate(float x, float y, float z)
{
	// Delegate to the C implementation
	MatrixMath_Rotate(f, x, y, z);
}

CMatrix &
CMatrix::operator*=(CMatrix const &rhs)
{
	// TODO: VU0 code
	*this = *this * rhs;
	return *this;
}

void
CMatrix::Reorthogonalise(void)
{
	// Delegate to the C implementation
	MatrixMath_Reorthogonalise(f);
}

CMatrix
operator*(const CMatrix &m1, const CMatrix &m2)
{
	// TODO: VU0 code
	// Delegate to the C implementation
	CMatrix out;
	MatrixMath_Multiply(out.f, m1.f, m2.f);
	return out;
}

CMatrix &
Invert(const CMatrix &src, CMatrix &dst)
{
	// TODO: VU0 code
	// Delegate to the C implementation
	MatrixMath_Invert(dst.f, src.f);

	return dst;
}

void
CMatrix::CopyToRwMatrix(RwMatrix* matrix)
{
	matrix->right = GetRight();
	matrix->up = GetForward();
	matrix->at = GetUp();
	matrix->pos = GetPosition();
	RwMatrixUpdate(matrix);
}

CMatrix
Invert(const CMatrix &matrix)
{
	CMatrix inv;
	return Invert(matrix, inv);
}

void
CCompressedMatrixNotAligned::CompressFromFullMatrix(CMatrix &other)
{
	m_rightX = 127.0f * other.GetRight().x;
	m_rightY = 127.0f * other.GetRight().y;
	m_rightZ = 127.0f * other.GetRight().z;
	m_upX = 127.0f * other.GetForward().x;
	m_upY = 127.0f * other.GetForward().y;
	m_upZ = 127.0f * other.GetForward().z;
	m_vecPos = other.GetPosition();
}

void
CCompressedMatrixNotAligned::DecompressIntoFullMatrix(CMatrix &other)
{
	other.GetRight().x = m_rightX / 127.0f;
	other.GetRight().y = m_rightY / 127.0f;
	other.GetRight().z = m_rightZ / 127.0f;
	other.GetForward().x = m_upX / 127.0f;
	other.GetForward().y = m_upY / 127.0f;
	other.GetForward().z = m_upZ / 127.0f;
	other.GetUp() = CrossProduct(other.GetRight(), other.GetForward());
	other.GetPosition() = m_vecPos;
	other.Reorthogonalise();
}//- rouz edit (Claude)
