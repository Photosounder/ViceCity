//+ rouz edit (Claude)
#pragma once
#include "MatrixData.h"

// C++ sees the CMatrix class, C sees its plain data, which is the same memory
#ifdef __cplusplus
class CMatrix : public CMatrixData
{
public:
	CMatrix(void);
	CMatrix(CMatrix const &m);
	CMatrix(RwMatrix *matrix, bool owner = false);
	CMatrix(float scale){
		m_attachment = nil;
		m_hasRwMatrix = false;
		SetScale(scale);
	}
	~CMatrix(void);
	void Attach(RwMatrix *matrix, bool owner = false);
	void AttachRW(RwMatrix *matrix, bool owner = false);
	void Detach(void);
	void Update(void);
	void UpdateRW(void);
	void operator=(CMatrix const &rhs);
	CMatrix &operator+=(CMatrix const &rhs);
	CMatrix &operator*=(CMatrix const &rhs);

	CVector &GetPosition(void) { return *(CVector*)&px; }
	CVector &GetRight(void) { return *(CVector*)&rx; }
	CVector &GetForward(void) { return *(CVector*)&fx; }
	CVector &GetUp(void) { return *(CVector*)&ux; }

	const CVector &GetPosition(void) const { return *(CVector*)&px; }
	const CVector &GetRight(void) const { return *(CVector*)&rx; }
	const CVector &GetForward(void) const { return *(CVector*)&fx; }
	const CVector &GetUp(void) const { return *(CVector*)&ux; }


	void SetTranslate(float x, float y, float z);
	void SetTranslate(const CVector &trans){ SetTranslate(trans.x, trans.y, trans.z); }
	void Translate(float x, float y, float z){
		px += x;
		py += y;
		pz += z;
	}
	void Translate(const CVector &trans){ Translate(trans.x, trans.y, trans.z); }

	void SetScale(float s);
	void Scale(float scale)
	{
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				f[i][j] *= scale;
	}
	void Scale(float sx, float sy, float sz)
	{
		for (int i = 0; i < 3; i++){
			f[i][0] *= sx;
			f[i][1] *= sy;
			f[i][2] *= sz;
		}
	}


	void SetRotateXOnly(float angle);
	void SetRotateYOnly(float angle);
	void SetRotateZOnly(float angle);
	void SetRotateX(float angle);
	void SetRotateY(float angle);
	void SetRotateZ(float angle);
	void SetRotate(float xAngle, float yAngle, float zAngle);
	void Rotate(float x, float y, float z);
	void RotateX(float x);
	void RotateY(float y);
	void RotateZ(float z);

	void Reorthogonalise(void);
	void CopyOnlyMatrix(const CMatrix &other);
	void SetUnity(void);
	void ResetOrientation(void);
	
	void CopyToRwMatrix(RwMatrix* matrix);
	
	void SetTranslateOnly(float x, float y, float z) {
		px = x;
		py = y;
		pz = z;
	}
	void SetTranslateOnly(const CVector& pos) {
		SetTranslateOnly(pos.x, pos.y, pos.z);
	}
	void CheckIntegrity(){}
};


CMatrix &Invert(const CMatrix &src, CMatrix &dst);
CMatrix Invert(const CMatrix &matrix);
CMatrix operator*(const CMatrix &m1, const CMatrix &m2);
inline CVector MultiplyInverse(const CMatrix &mat, const CVector &vec)
{
	CVector v(vec.x - mat.px, vec.y - mat.py, vec.z - mat.pz);
	return CVector(
		mat.rx * v.x + mat.ry * v.y + mat.rz * v.z,
		mat.fx * v.x + mat.fy * v.y + mat.fz * v.z,
		mat.ux * v.x + mat.uy * v.y + mat.uz * v.z);
}



class CCompressedMatrixNotAligned
{
	CVector m_vecPos;
	int8 m_rightX;
	int8 m_rightY;
	int8 m_rightZ;
	int8 m_upX;
	int8 m_upY;
	int8 m_upZ;
public:
	void CompressFromFullMatrix(CMatrix &other);
	void DecompressIntoFullMatrix(CMatrix &other);
};

class CCompressedMatrix : public CCompressedMatrixNotAligned
{
	int _alignment; // no clue what would this align to
};

static_assert(sizeof(CMatrix) == sizeof(CMatrixData), "CMatrix must hold no data beyond CMatrixData so C sees the same layout");
#else
// The C view of CMatrix has the same fields as CMatrixData under the struct tag CMatrix, which older C headers already declare
typedef struct CMatrix {
	CMATRIXDATA_FIELDS
} CMatrix;
#endif
//- rouz edit (Claude)
