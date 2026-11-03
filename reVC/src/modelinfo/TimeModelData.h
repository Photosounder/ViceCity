//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
typedef struct TimeModelData {
    int32_t timeOn;
    int32_t timeOff;
    int32_t otherTimeModelID;
} TimeModelData;
#ifdef __cplusplus
extern "C" {
#endif
void TimeModelData_Init(TimeModelData *data);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
