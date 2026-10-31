//+ rouz edit (ChatGPT)
#pragma once
#include "SurfaceTable.h"
enum { SURFACE_TABLE_WORK_BUFFER_SIZE=55000 };
#ifdef __cplusplus
extern "C" {
#endif
uint8_t *SurfaceTable_BackendWorkBuffer(void);
float SurfaceTable_BackendWetRoads(void);
uint8_t SurfaceTable_BackendSurfaceA(const CColPoint *point);
uint8_t SurfaceTable_BackendSurfaceB(const CColPoint *point);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
