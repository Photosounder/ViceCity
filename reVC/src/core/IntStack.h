#pragma once

//+ rouz edit (ChatGPT)
#include <stdint.h>

typedef struct CIntStack16 {
	int32_t values[16];
	uint32_t sp;
} CIntStack16;

static inline void
CIntStack16_Init(CIntStack16 *stack)
{
	// Reset the stack counter without altering its embedded values
	stack->sp = 0;
}

static inline void
CIntStack16_Push(CIntStack16 *stack, int32_t value)
{
	// Append the value using the same counter update as the original stack
	stack->values[stack->sp++] = value;
}

static inline int32_t
CIntStack16_Pop(CIntStack16 *stack)
{
	// Remove and return the most recently pushed value
	return stack->values[--stack->sp];
}

typedef struct CIntStack4000 {
	int32_t values[4000];
	uint32_t sp;
} CIntStack4000;

static inline void
CIntStack4000_Init(CIntStack4000 *stack)
{
	// Reset the stack counter without altering its embedded values
	stack->sp = 0;
}

static inline void
CIntStack4000_Push(CIntStack4000 *stack, int32_t value)
{
	// Append the value using the same counter update as the original stack
	stack->values[stack->sp++] = value;
}

static inline int32_t
CIntStack4000_Pop(CIntStack4000 *stack)
{
	// Remove and return the most recently pushed value
	return stack->values[--stack->sp];
}

//- rouz edit (ChatGPT)
