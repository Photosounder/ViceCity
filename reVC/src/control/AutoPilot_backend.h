//+ rouz edit (ChatGPT)
#pragma once
#include "AutoPilot.h"
#ifdef __cplusplus
struct CCarPathLink;
extern "C" {
#else
typedef struct CCarPathLink CCarPathLink;
#endif
CCarPathLink * AutoPilot_BackendLink(uint32_t index);
float AutoPilot_BackendX(CCarPathLink *link);
float AutoPilot_BackendY(CCarPathLink *link);
float AutoPilot_BackendDirX(CCarPathLink *link);
float AutoPilot_BackendDirY(CCarPathLink *link);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
