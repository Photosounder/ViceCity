//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "PlaceableData.h"

class CPlaceable : public CPlaceableData
{
public:
	CPlaceable(void);
	const CVector &GetPosition(void) { return m_matrix.GetPosition(); }
	void SetPosition(float x, float y, float z) {
		m_matrix.GetPosition().x = x;
		m_matrix.GetPosition().y = y;
		m_matrix.GetPosition().z = z;
	}
	void SetPosition(const CVector &pos) { m_matrix.GetPosition() = pos; }
	CVector &GetRight(void) { return m_matrix.GetRight(); }
	CVector &GetForward(void) { return m_matrix.GetForward(); }
	CVector &GetUp(void) { return m_matrix.GetUp(); }
	CMatrix &GetMatrix(void) { return m_matrix; }
	void SetMatrix(CMatrix &newMatrix) { m_matrix = newMatrix; }
	void SetTransform(RwMatrix *m) { m_matrix = CMatrix(m, false); }
	void SetHeading(float angle);
	void SetOrientation(float x, float y, float z){
		// Delegate to the C implementation
		Placeable_SetOrientation(this, x, y, z);
	}
	bool IsWithinArea(float x1, float y1, float x2, float y2);
	bool IsWithinArea(float x1, float y1, float z1, float x2, float y2, float z2);
};

VALIDATE_SIZE(CPlaceable, 0x48);
//- rouz edit (Claude)
