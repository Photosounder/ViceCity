//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Copies a zero terminated wide message of at most cap characters, the terminator is written after the copied characters even when the message filled the whole buffer
void Hud_CopyMessage(uint16_t *dst, const uint16_t *src, int32_t cap);
// Stores a big message of 128 characters, with trackChanges the last message is kept up to date and the result tells that the text differs from the previous one
bool Hud_SetBigMessage(uint16_t *big, uint16_t *last, const uint16_t *message, bool trackChanges);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
