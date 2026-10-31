//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
enum
{
	POWERPOINT_NONE = 0,
	POWERPOINT_HEALTH,
	POWERPOINT_HIDEOUT_INDUSTRIAL,
	POWERPOINT_HIDEOUT_COMMERCIAL,
	POWERPOINT_HIDEOUT_SUBURBAN
};
typedef struct CPowerPoint {uint8_t reserved;} CPowerPoint;
#ifdef __cplusplus
extern "C" {
#endif
void PowerPoint_Update(CPowerPoint *self);
void PowerPoints_Init(void);
void PowerPoints_Update(void);
void PowerPoints_GenerateNewOne(float a,float b,float c,float d,float e,float f,uint8_t type);
void PowerPoints_Save(uint8_t **buffer,uint32_t *size);
void PowerPoints_Load(uint8_t *buffer,uint32_t size);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
