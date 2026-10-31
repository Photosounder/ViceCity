//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "Escalator.h"
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t Escalators_Num;
void Escalators_Init(void);
void Escalators_Update(void);
void Escalators_Shutdown(void);
void Escalators_AddOne(EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool movingDown);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
