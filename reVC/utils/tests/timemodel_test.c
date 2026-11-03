#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "TimeModel_backend.h"
#include "ModelInfoType.h"

static const char *names[5];
static int visits;
static void *nonTimeModel;
static void *updatedModel;
static TimeModelData sourceData;
void *ModelRegistry_Get(int index)
{
    // Record the search order and return the configured model when present
    ++visits;
    return index<5 && names[index] ? (void *)&names[index] : 0;
}
const char *ModelRegistry_Name(void *model)
{
    // Borrow the configured candidate name
    return *(const char **)model;
}
int ModelRegistry_Type(void *model)
{
    // Treat configured test entries as time models
    return model==nonTimeModel ? MITYPE_PED : MITYPE_TIME;
}
TimeModelData *TimeModel_BackendData(void *model)
{
    // Return the C record belonging to the selected source model
    updatedModel=model;
    return &sourceData;
}
bool General_faststrncmp(const char *left,const char *right,uint32_t count)
{
    // Match the comparison contract used by the original lookup
    while (*left && count--) { if (*left++ != *right++) return true; }
    return false;
}
int main(void)
{
    // Preserve the existing time values while initializing the link sentinel
    TimeModelData data={12,23,7};
    TimeModelData_Init(&data);
    assert(data.timeOn==12 && data.timeOff==23 && data.otherTimeModelID==-1);
    // Find the earliest opposite-time candidate and preserve its index
    int index=-7;
    names[1]="shop_dy";names[3]="shop_dy";
    assert(TimeModel_FindOther("shop_nt",&index,21)==&names[1] && index==1 && visits==2);
    // Skip matching names when the model is not a time model
    nonTimeModel=&names[1];visits=0;index=-7;
    assert(TimeModel_FindOther("shop_nt",&index,21)==&names[3] && index==3 && visits==4);
    nonTimeModel=0;
    // Link the source model only after the counterpart is found
    const char *source="shop_nt";
    assert(TimeModel_LinkOther(&source,21)==&names[1] && updatedModel==&source && sourceData.otherTimeModelID==1);
    // Check the day-to-night swap and unchanged index when no suffix exists
    names[1]="shop_nt";visits=0;index=-7;
    assert(TimeModel_FindOther("shop_dy",&index,21)==&names[1] && index==1);
    visits=0;index=-7;
    assert(!TimeModel_FindOther("shop",&index,21) && index==-7 && visits==0);
    // Preserve the original prefix comparison behavior
    names[1]="shop_dy_extra";index=-7;
    assert(TimeModel_FindOther("shop_nt",&index,21)==&names[1] && index==1);
    index=-7;
    // Check that a missing counterpart leaves the identifier unchanged
    names[1]=names[3]=0;visits=0;
    assert(!TimeModel_FindOther("shop_nt",&index,21) && index==-7 && visits>5);
    updatedModel=0;
    assert(!TimeModel_LinkOther(&source,21) && updatedModel==0);
    return 0;
}
