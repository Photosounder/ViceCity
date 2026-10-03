#include <stdint.h>
#include <stddef.h>
#include "config.h"
#include "AudioStreamHost.h"
typedef uint8_t bool8; typedef uint8_t uint8; typedef uint32_t uint32;
typedef int8_t int8; typedef int32_t int32;
#define FALSE 0
#define TRUE 1
#ifndef MASTER
#define ASSERT(value) ((void)((!!(value)) || (re3_assert(#value, __FILE__, __LINE__, __func__), 0)))
#else
#define ASSERT(value) ((void)(value))
#endif
