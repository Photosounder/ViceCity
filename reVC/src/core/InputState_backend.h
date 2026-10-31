//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
bool InputState_BackendHasMouse(void);
bool InputState_BackendMenuActive(void);
bool InputState_BackendNeedsExclusive(void);
void InputState_BackendInitialiseMouse(bool exclusive);
void InputState_BackendCapabilities(uint32_t *buttons,uint32_t *axes);
double InputState_BackendCursorX(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
