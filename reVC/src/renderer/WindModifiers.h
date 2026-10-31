//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct WindModifiersVector {
	float x, y, z;
} WindModifiersVector;
typedef struct WindModifiersRecord {
	WindModifiersVector m_pos;
	int32_t m_type;
} WindModifiersRecord;
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t WindModifiers_Number;
void WindModifiers_RegisterOne(WindModifiersVector pos, int32_t type);
bool WindModifiers_FindWindModifier(WindModifiersVector pos, float *x, float *y);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
