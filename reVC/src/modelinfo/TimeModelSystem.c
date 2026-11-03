//+ rouz edit (ChatGPT)
#include "TimeModel_backend.h"
#include "ModelRegistry.h"
#include "ModelInfoType.h"
#include "General.h"
#include <string.h>

void TimeModelData_Init(TimeModelData *data)
{
    // Initialize only the original unpaired-model sentinel
    data->otherTimeModelID=-1;
}

void *TimeModel_FindOther(const char *modelName,int *index,unsigned int maxNameLength)
{
    // Swap the first day or night suffix using the original search priority
    char name[40];
    char *suffix;
    strcpy(name,modelName);
    if ((suffix=strstr(name,"_nt"))) strncpy(suffix,"_dy",4);
    else if ((suffix=strstr(name,"_dy"))) strncpy(suffix,"_nt",4);
    else return 0;
    // Search the native table in its original order for a matching time model
    for (int i=0;i<MODELINFOSIZE;++i) {
        void *model=ModelRegistry_Get(i);
        if (model && ModelRegistry_Type(model)==MITYPE_TIME && !General_faststrncmp(name,ModelRegistry_Name(model),maxNameLength)) {
            *index=i;
            return model;
        }
    }
    return 0;
}
void *TimeModel_LinkOther(void *model,unsigned int maxNameLength)
{
    // Search for the paired model and update the source only when one exists
    int index;
    void *other=TimeModel_FindOther(ModelRegistry_Name(model),&index,maxNameLength);
    if (other) TimeModel_BackendData(model)->otherTimeModelID=index;
    return other;
}
//- rouz edit (ChatGPT)
