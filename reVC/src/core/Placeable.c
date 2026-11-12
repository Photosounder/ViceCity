//+ rouz edit (Claude)
#include "PlaceableData.h"
#include "PlaceableMath.h"
#include "MatrixMath.h"

void Placeable_Init(CPlaceableData *self)
{
	// Start with a unit scale matrix and no recorded original position
	MatrixMath_SetScale(self->m_matrix.f, 1.0f);
	self->orig_pos_set = 0;
}

void Placeable_SetHeading(CPlaceableData *self, float angle)
{
	// Rotate around Z, clear the position then add back the previous one
	float x = self->m_matrix.px;
	float y = self->m_matrix.py;
	float z = self->m_matrix.pz;
	MatrixMath_SetRotateZOnly(self->m_matrix.f, angle);
	self->m_matrix.px = 0.0f;
	self->m_matrix.py = 0.0f;
	self->m_matrix.pz = 0.0f;
	self->m_matrix.px += x;
	self->m_matrix.py += y;
	self->m_matrix.pz += z;
}

void Placeable_SetOrientation(CPlaceableData *self, float x, float y, float z)
{
	// Set a combined rotation (which clears the position) then add back the previous position
	float px = self->m_matrix.px;
	float py = self->m_matrix.py;
	float pz = self->m_matrix.pz;
	MatrixMath_SetRotate(self->m_matrix.f, x, y, z);
	self->m_matrix.px += px;
	self->m_matrix.py += py;
	self->m_matrix.pz += pz;
}

bool Placeable_IsWithinArea2D(const CPlaceableData *self, float x1, float y1, float x2, float y2)
{
	// Test the position against a rectangle
	return PlaceableMath_IsWithinArea2D(self->m_matrix.px, self->m_matrix.py, x1, y1, x2, y2);
}

bool Placeable_IsWithinArea3D(const CPlaceableData *self, float x1, float y1, float z1, float x2, float y2, float z2)
{
	// Test the position against a box
	return PlaceableMath_IsWithinArea3D(self->m_matrix.px, self->m_matrix.py, self->m_matrix.pz, x1, y1, z1, x2, y2, z2);
}
//- rouz edit (Claude)
