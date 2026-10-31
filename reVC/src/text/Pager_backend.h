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
void Pager_BackendBrief(uint16_t *text,int32_t n1,int32_t n2,int32_t n3,int32_t n4,int32_t n5,int32_t n6,uint16_t *string);
void Pager_BackendInsertNumbers(uint16_t *str,int32_t n1,int32_t n2,int32_t n3,int32_t n4,int32_t n5,int32_t n6,uint16_t *output);
uint16_t Pager_BackendLength(uint16_t *str);
void Pager_BackendSetMessage(uint16_t *message);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
