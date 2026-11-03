//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ControllerActions.h"
typedef uint16_t MessagesActionNames[MAX_CONTROLLERACTIONS][ACTIONNAME_LENGTH];
#ifdef __cplusplus
extern "C" {
#endif
void Messages_BackendDefinedState(void);
bool Messages_BackendJapanese(void);
MessagesActionNames *Messages_BackendActionNames(void);
void Messages_BackendCommandKeys(uint16_t action,uint16_t *output,uint16_t length);
void Messages_BackendSetBigMessage(uint16_t *message,uint16_t style);
void Messages_BackendSetMessage(uint16_t *message);
void Messages_BackendClearHud(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
