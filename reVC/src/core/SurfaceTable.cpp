//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Weather.h"
#include "ColPoint.h"
#include "SurfaceTable_backend.h"
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
