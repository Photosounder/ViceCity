#pragma once

//+ rouz edit (ChatGPT)
#include <stddef.h>

#if defined(__MWERKS__)
#define REVC_STATIC_ASSERT(condition, message)
#elif defined(__cplusplus)
#define REVC_STATIC_ASSERT(condition, message) static_assert(condition, message)
#else
#define REVC_STATIC_ASSERT(condition, message) _Static_assert(condition, message)
#endif

#ifdef CHECK_STRUCT_SIZES
#define VALIDATE_SIZE(struc, size) REVC_STATIC_ASSERT(sizeof(struc) == (size), "Invalid structure size: " #struc)
#else
#define VALIDATE_SIZE(struc, size)
#endif
#define VALIDATE_OFFSET(struc, member, offset) REVC_STATIC_ASSERT(offsetof(struc, member) == (offset), "The offset of " #member " in " #struc " is not " #offset "...")
//- rouz edit (ChatGPT)
