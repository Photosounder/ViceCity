//+ rouz edit (ChatGPT)
#include "common.h"
#include "ModelInfo.h"
#include "TimeModel_backend.h"

TimeModelData *TimeModel_BackendData(void *model)
{
    // Return the original time-specific record within the native model
    return &((CTimeModelInfo*)((CBaseModelInfo*)model))->m_timeData;
}
//- rouz edit (ChatGPT)
