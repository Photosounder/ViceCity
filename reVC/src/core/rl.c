#include "rl.h"

//+ rouz edit (ChatGPT)
#define CITA_WIN_IMPLEMENTATION
#include <cita_windows.h>
#undef CITA_WIN_IMPLEMENTATION
//- rouz edit (ChatGPT)

#define RL_EXCL_STRICMP
//+ rouz edit (ChatGPT)
// Preserve rouziclib's local far variable and the empty Windows FAR qualifier
#ifdef _WIN32
#pragma push_macro("far")
#pragma push_macro("FAR")
#undef FAR
#define FAR
#undef far
#endif
#include <rouziclib/rouziclib.c>
#ifdef _WIN32
#pragma pop_macro("FAR")
#pragma pop_macro("far")
#endif
//- rouz edit (ChatGPT)
