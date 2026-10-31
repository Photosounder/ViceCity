//+ rouz edit (ChatGPT)
#pragma once
#include "Restart.h"
#ifdef __cplusplus
extern "C" {
#endif
RestartPoint Restart_BackendPosition(const RestartPosition *position);
void Restart_BackendSetPosition(RestartPosition *position,RestartPoint value);
RestartPoint Restart_BackendClosestNode(const RestartPosition *position);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
