//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
bool Pager_BackendWidescreen(void);
bool Pager_BackendDrawHud(void);
uint16_t Pager_BackendBigMessage0(void);
uint16_t Pager_BackendBigMessage2(void);
void Pager_BackendSetMessage(uint16_t *message);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
