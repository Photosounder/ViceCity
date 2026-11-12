//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>

// TODO: reverse mobile code

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CPlayerName {
	float x;
	float y;
	uint16_t unk_8[34];
	int unk_4c;
} CPlayerName;

typedef struct CRow {
	int unk_0;
	int unk_4;
	uint16_t unk_8[20];
	int unk_30;
} CRow;

typedef struct CGrid {
	CRow rows[5];
	int unk_int1;
	int unk_int2;
	int unk_int3;
	float unk_float1;
	float unk_float2;
	CPlayerName playerName;
	char unk2[4];
	char unk3[4];
} CGrid;

void CPlayerName_DisplayName(CPlayerName *self, int unused);
void CRow_SetLetter(CRow *self, int index, uint16_t *letters);
void CGrid_ProcessAnyLeftJustDown(CGrid *self);
void CGrid_ProcessAnyRightJustDown(CGrid *self);
void CGrid_ProcessAnyUpJustDown(CGrid *self);
void CGrid_ProcessAnyDownJustDown(CGrid *self);
void CGrid_AllDoneMakePlayerName(CGrid *self);
void CGrid_ProcessDPadCrossJustDown(CGrid *self);
void CGrid_DisplayGrid(CGrid *self);
void CGrid_ProcessControllerInput(CGrid *self);
void CGrid_Process(CGrid *self);

#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
