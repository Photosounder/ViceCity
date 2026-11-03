//+ rouz edit (ChatGPT)
#include "ModelRegistry.h"
#include "SimpleModelPolicy.h"
#include "SimpleModel_backend.h"
#include "General.h"

void SimpleModel_SetupBigBuilding(void *model,float firstDistance,float lodDistance,int minID,int maxID)
{
    // Skip ordinary buildings and models which already have a relation
    if (!(firstDistance>lodDistance) || SimpleModel_BackendRelated(model)) return;
    // Mark the model and search for its related non-LOD model
    SimpleModel_BackendMarkBig(model);
    SimpleModel_FindRelated(model,minID,maxID);
    void *related=SimpleModel_BackendRelated(model);
    if (related) {
        // Derive the near distance and retain the original draw-last diagnostic
        SimpleModel_BackendSetNearDistance(model,SimpleModel_BackendRelatedNearDistance(related));
        if (SimpleModel_BackendDrawLast(model)) SimpleModel_BackendClearDrawLast(model);
    } else {
        // Retain the original fallback when no related model exists
        SimpleModel_BackendSetNearDistance(model,0.0f);
    }
}

void SimpleModel_FindRelated(void *model,int minID,int maxID)
{
    // Search the original inclusive range and compare each candidate suffix
    for (int i=minID;i<=maxID;++i) {
        void *candidate=ModelRegistry_Get(i);
        if (candidate && candidate!=model &&
            !General_faststrcmp(ModelRegistry_Name(model)+3,ModelRegistry_Name(candidate)+3)) {
            // Retain the native assertion before storing the first matching model
            SimpleModel_BackendRequireSimple(candidate);
            SimpleModel_BackendSetRelated(model,candidate);
            return;
        }
    }
}

void SimpleModel_SetDistances(float *destination,const float *source,float scale)
{
    // Scale the three distances in their original assignment order
    destination[0]=source[0]*scale;
    destination[1]=source[1]*scale;
    destination[2]=source[2]*scale;
}

void SimpleModel_IncreaseAlpha(uint8_t *alpha)
{
    // Preserve the original fade increment and saturation threshold
    if (*alpha>=0xEF) *alpha=0xFF;
    else *alpha+=0x10;
}

float SimpleModel_LargestDistance(const float *distances,int numAtomics,int firstDamaged,int isDamaged,float multiplier)
{
    // Select the original last visible or undamaged level of detail
    int index=firstDamaged==0 || isDamaged ? numAtomics-1 : firstDamaged-1;
    return distances[index]*multiplier;
}

int SimpleModel_AtomicIndex(const float *distances,int numAtomics,int firstDamaged,int isDamaged,float distance,float multiplier)
{
    // Find the first eligible level of detail in its original order
    int i=isDamaged ? firstDamaged : 0;
    for (;i<numAtomics;++i)
        if (distance<distances[i]*multiplier) return i;
    return -1;
}

int SimpleModel_FirstAtomicIndex(const float *distances,float distance,float multiplier)
{
    // Check only the nearest level of detail using the original strict comparison
    return distance<distances[0]*multiplier ? 0 : -1;
}
//- rouz edit (ChatGPT)
