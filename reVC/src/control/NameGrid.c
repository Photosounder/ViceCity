//+ rouz edit (Claude)
#include "NameGrid.h"

// TODO: reverse mobile code

void CPlayerName_DisplayName(CPlayerName *self, int unused)
{
	// TODO
}

void CRow_SetLetter(CRow *self, int index, uint16_t *letters)
{
	// TODO
}

void CGrid_ProcessAnyLeftJustDown(CGrid *self)
{
	// Step the second selector back
	self->unk_int2--;
}

void CGrid_ProcessAnyRightJustDown(CGrid *self)
{
	// Step the second selector forward
	self->unk_int2++;
}

void CGrid_ProcessAnyUpJustDown(CGrid *self)
{
	// Step the first selector back
	self->unk_int1--;
}

void CGrid_ProcessAnyDownJustDown(CGrid *self)
{
	// Step the first selector forward
	self->unk_int1++;
}

void CGrid_AllDoneMakePlayerName(CGrid *self)
{
	// TODO
}

void CGrid_ProcessDPadCrossJustDown(CGrid *self)
{
	// TODO
}

void CGrid_DisplayGrid(CGrid *self)
{
	// TODO
}

void CGrid_ProcessControllerInput(CGrid *self)
{
	// TODO
}

void CGrid_Process(CGrid *self)
{
	// Handle input, draw the grid, then draw the player name
	CGrid_ProcessControllerInput(self);
	CGrid_DisplayGrid(self);
	CPlayerName_DisplayName(&self->playerName, 2 * self->playerName.unk_4c);
}
//- rouz edit (Claude)
