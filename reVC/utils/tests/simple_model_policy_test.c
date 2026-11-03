#include <assert.h>
#include "SimpleModelPolicy.h"
#include <stdbool.h>
#include <string.h>

typedef struct TestModel { const char *name; int simple; } TestModel;
static TestModel *models[5];
static TestModel *selected;
static int assertions;
static int markedBig,drawLast,drawCleared;
static float nearDistance;
void *SimpleModel_BackendRelated(void *model) {
    // Return the simulated current relation
    assert(model==models[0]);return selected;
}
void SimpleModel_BackendMarkBig(void *model) {
    // Record the native big-building flag update
    assert(model==models[0]);++markedBig;
}
float SimpleModel_BackendRelatedNearDistance(void *model) {
    // Provide the related model distance at the native boundary
    assert(model==selected);return 45.0f;
}
void SimpleModel_BackendSetNearDistance(void *model,float distance) {
    // Capture the chosen near distance
    assert(model==models[0]);nearDistance=distance;
}
int SimpleModel_BackendDrawLast(void *model) {
    // Return the simulated draw-last flag
    assert(model==models[0]);return drawLast;
}
void SimpleModel_BackendClearDrawLast(void *model) {
    // Record the native flag clear and diagnostic
    assert(model==models[0]);drawLast=0;++drawCleared;
}
void *ModelRegistry_Get(int id) {
    // Return the test table entry in its original order
    return models[id];
}
const char *ModelRegistry_Name(void *model) {
    // Return the test candidate name
    return ((TestModel*)model)->name;
}
void SimpleModel_BackendRequireSimple(void *model) {
    // Record the type assertion before the related model is stored
    assert(((TestModel*)model)->simple && !selected);
    ++assertions;
}
void SimpleModel_BackendSetRelated(void *model,void *related) {
    // Capture the matched candidate after the assertion
    assert(model==models[0] && assertions==1);
    selected=related;
}
bool General_faststrcmp(const char *a,const char *b) {
    // Match the native string comparison contract
    return strcmp(a,b)!=0;
}

int main(void)
{
    // Find the first matching suffix while skipping the source model
    TestModel source={"abcbuilding",1}, mismatch={"abcother",1}, match={"xyzbuilding",1};
    models[0]=&source;models[1]=0;models[2]=&mismatch;models[3]=&match;models[4]=&match;
    SimpleModel_FindRelated(&source,0,4);
    assert(selected==&match && assertions==1);
    // Leave the relation untouched when the inclusive range has no match
    selected=0;assertions=0;
    SimpleModel_FindRelated(&source,0,2);
    assert(!selected && assertions==0);
    // Apply the big-building setup only when the threshold is crossed
    markedBig=0;nearDistance=-1.0f;drawLast=1;drawCleared=0;
    SimpleModel_SetupBigBuilding(&source,300.0f,300.0f,0,4);
    assert(!markedBig && nearDistance==-1.0f);
    SimpleModel_SetupBigBuilding(&source,301.0f,300.0f,0,4);
    assert(markedBig==1 && selected==&match && nearDistance==45.0f && drawCleared==1);
    SimpleModel_SetupBigBuilding(&source,301.0f,300.0f,0,4);
    assert(markedBig==1);
    // Preserve the zero-distance fallback when no relation exists
    selected=0;assertions=0;nearDistance=-1.0f;
    SimpleModel_SetupBigBuilding(&source,301.0f,300.0f,0,2);
    assert(markedBig==2 && !selected && nearDistance==0.0f);
    // Verify fade increment and saturation at the original threshold
    uint8_t alpha=0xDE;
    SimpleModel_IncreaseAlpha(&alpha);assert(alpha==0xEE);
    SimpleModel_IncreaseAlpha(&alpha);assert(alpha==0xFE);
    SimpleModel_IncreaseAlpha(&alpha);assert(alpha==0xFF);
    // Scale each configured distance while allowing the source and target to coincide
    float scaled[3]={1.0f,2.0f,3.0f};
    SimpleModel_SetDistances(scaled,scaled,2.0f);
    assert(scaled[0]==2.0f && scaled[1]==4.0f && scaled[2]==6.0f);
    // Select the last undamaged or damaged distance with the camera multiplier
    float distances[3]={10.0f,20.0f,30.0f};
    assert(SimpleModel_LargestDistance(distances,3,0,0,2.0f)==60.0f);
    assert(SimpleModel_LargestDistance(distances,3,2,0,2.0f)==40.0f);
    assert(SimpleModel_LargestDistance(distances,3,2,1,2.0f)==60.0f);
    // Verify strict boundaries, damage selection, and no eligible atomic
    assert(SimpleModel_AtomicIndex(distances,3,2,0,19.0f,2.0f)==0);
    assert(SimpleModel_AtomicIndex(distances,3,2,0,20.0f,2.0f)==1);
    assert(SimpleModel_AtomicIndex(distances,3,2,1,1.0f,2.0f)==2);
    assert(SimpleModel_AtomicIndex(distances,3,2,1,60.0f,2.0f)==-1);
    assert(SimpleModel_FirstAtomicIndex(distances,19.0f,2.0f)==0);
    assert(SimpleModel_FirstAtomicIndex(distances,20.0f,2.0f)==-1);
    return 0;
}
