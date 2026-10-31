//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "BridgeStates.h"
#ifdef __cplusplus
class CEntity;
extern "C" {
#else
typedef struct CEntity CEntity;
#endif
extern CEntity * Bridge_pLiftRoad;
extern CEntity * Bridge_pLiftPart;
extern CEntity * Bridge_pWeight;
extern int32_t Bridge_State;
extern int32_t Bridge_OldState;
extern float Bridge_DefaultZLiftPart;
extern float Bridge_DefaultZLiftRoad;
extern float Bridge_DefaultZLiftWeight;
extern float Bridge_OldLift;
extern uint32_t Bridge_TimeOfBridgeBecomingOperational;
void Bridge_Init(void);
void Bridge_Update(void);
bool Bridge_ShouldLightsBeFlashing(void);
void Bridge_FindBridgeEntities(void);
bool Bridge_ThisIsABridgeObjectMovingUp(int32_t index);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
