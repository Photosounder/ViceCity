//+ rouz edit (Claude)
#include "PedAttractorMath.h"
#include "PedAttractorTypes.h"
#include "VectorMath.h"

void PedAttractor_ComputeEffectPos(const float m[4][4], const float *effectPos, float *out)
{
	// Matrix position plus the effect offset rotated by the matrix
	VectorMath3 v = {effectPos[0], effectPos[1], effectPos[2]};
	VectorMath3 r = VectorMath_MatrixTimesVector3x3(m, v);
	out[0] = m[3][0] + r.x;
	out[1] = m[3][1] + r.y;
	out[2] = m[3][2] + r.z;
}

void PedAttractor_ComputeEffectDir(const float m[4][4], const float *effectDir, float *out)
{
	// The direction rotated by the matrix
	VectorMath3 v = {effectDir[0], effectDir[1], effectDir[2]};
	VectorMath3 r = VectorMath_MatrixTimesVector3x3(m, v);
	out[0] = r.x;
	out[1] = r.y;
	out[2] = r.z;
}

bool PedAttractor_IsPedOnUsableSide(int type, const float *useDir, const float *effectPos, const float *pedPos, float *outUseDir)
{
	float ud[3] = {useDir[0], useDir[1], useDir[2]};
	float dp = -(ud[0]*effectPos[0] + ud[1]*effectPos[1] + ud[2]*effectPos[2]);

	// These effects face the other way
	if (type == ATTRACTOR_ATM || type == ATTRACTOR_PIZZA || type == ATTRACTOR_ICECREAM) {
		ud[0] = -ud[0];
		ud[1] = -ud[1];
		ud[2] = -ud[2];
		dp = -dp;
	}
	outUseDir[0] = ud[0];
	outUseDir[1] = ud[1];
	outUseDir[2] = ud[2];

	// The ped must be in front of the effect plane and within the use cone
	if (dp + (effectPos[0]*pedPos[0] + effectPos[1]*pedPos[1] + effectPos[2]*pedPos[2]) > 0.0f) {
		VectorMath3 toAttractor = {pedPos[0] - effectPos[0], pedPos[1] - effectPos[1], pedPos[2] - effectPos[2]};
		VectorMath_Normalise(&toAttractor);
		if (toAttractor.x*ud[0] + toAttractor.y*ud[1] + toAttractor.z*ud[2] > 0.25f)
			return true;
	}
	return false;
}
//- rouz edit (Claude)
