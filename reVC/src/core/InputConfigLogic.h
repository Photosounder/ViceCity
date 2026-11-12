//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "InputState.h"
#include "ControllerActionType.h"

#ifdef __cplusplus
extern "C" {
#endif
// Which situations an action is used in
int InputConfig_GetActionType(int action);
// Free a key that is about to be bound to an action from every action that cannot share it, classicControls is true for the classic control method
void InputConfig_DeleteMatchingActionInitiators(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int action, int key, int type, bool classicControls);
// Whether a mouse button is already used by a vehicle or common action
bool InputConfig_IsAnyVehicleActionAssignedToMouseKey(InputControllerBinding bindings[][MAX_CONTROLLERTYPES], int key);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
