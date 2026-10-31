//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef struct AntennasVector {
	float x, y, z;
} AntennasVector;
typedef struct Antenna {
	bool active;
	bool updatedLastFrame;
	uint32_t id;
	float segmentLength;
	AntennasVector pos[6];
	AntennasVector speed[6];
} Antenna;
#ifdef __cplusplus
extern "C" {
#endif
void Antennas_Init(void);
void Antennas_Update(void);
void Antennas_RegisterOne(uint32_t id, AntennasVector dir, AntennasVector position, float length);
void Antennas_Render(void);
void Antenna_Update(Antenna *antenna, AntennasVector dir, AntennasVector basepos);
#ifdef REVC_SOFTWARE_POLYGONS
void Antennas_RegisterOneForEnvMap(uint32_t id, AntennasVector dir, AntennasVector position, float length);
void Antennas_RenderForEnvMap(void);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
