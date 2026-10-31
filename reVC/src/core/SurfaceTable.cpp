//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Weather.h"
#include "ColPoint.h"
#include "SurfaceTable_backend.h"
static_assert(sizeof(work_buff)==SURFACE_TABLE_WORK_BUFFER_SIZE,"Actual shared work buffer extent");
uint8_t *SurfaceTable_BackendWorkBuffer(void)
{
    // Borrow the original main-module work buffer without a copy or ownership change
    return work_buff;
}
float SurfaceTable_BackendWetRoads(void)
{
    // Read the actual live native wetness value at the original branch
    return Weather_WetRoads;
}
uint8_t SurfaceTable_BackendSurfaceA(const CColPoint *point)
{
    // Read the original first native collision surface without a layout cast
    return point->surfaceA;
}
uint8_t SurfaceTable_BackendSurfaceB(const CColPoint *point)
{
    // Read the original second native collision surface without a layout cast
    return point->surfaceB;
}
//- rouz edit (ChatGPT)
