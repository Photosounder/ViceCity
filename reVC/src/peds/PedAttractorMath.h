//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Position of an effect in the world and its directions rotated into the world, m is a CMatrix as float[4][4]
void PedAttractor_ComputeEffectPos(const float m[4][4], const float *effectPos, float *out);
void PedAttractor_ComputeEffectDir(const float m[4][4], const float *effectDir, float *out);
// Whether the ped stands on the usable side of an effect and faces it, the line test is left to the caller, shelters have no such test
bool PedAttractor_IsPedOnUsableSide(int type, const float *useDir, const float *effectPos, const float *pedPos, float *outUseDir);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
