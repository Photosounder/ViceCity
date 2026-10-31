//+ rouz edit (ChatGPT)
#pragma once

enum eDoorState
{
	DOORST_SWINGING,
	// actually wrong though,
	// OPEN is really MAX_ANGLE and CLOSED is MIN_ANGLE
	DOORST_OPEN,
	DOORST_CLOSED
};

enum eDoorStatus
{
	DOOR_STATUS_OK,
	DOOR_STATUS_SMASHED,
	DOOR_STATUS_SWINGING,
	DOOR_STATUS_MISSING
};

enum { VEHICLE_DOOR_COUNT = 6 };
//- rouz edit (ChatGPT)
