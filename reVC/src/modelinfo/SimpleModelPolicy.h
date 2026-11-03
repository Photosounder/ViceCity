//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void SimpleModel_SetupBigBuilding(void *model,float firstDistance,float lodDistance,int minID,int maxID);
void SimpleModel_FindRelated(void *model,int minID,int maxID);
void SimpleModel_SetDistances(float *destination,const float *source,float scale);
void SimpleModel_IncreaseAlpha(uint8_t *alpha);
float SimpleModel_LargestDistance(const float *distances,int numAtomics,int firstDamaged,int isDamaged,float multiplier);
int SimpleModel_AtomicIndex(const float *distances,int numAtomics,int firstDamaged,int isDamaged,float distance,float multiplier);
int SimpleModel_FirstAtomicIndex(const float *distances,float distance,float multiplier);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
