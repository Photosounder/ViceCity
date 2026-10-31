//+ rouz edit (ChatGPT)
#include "Bridge_backend.h"
#include "Timer.h"
#include <stddef.h>
CEntity * Bridge_pLiftRoad;
CEntity * Bridge_pLiftPart;
CEntity * Bridge_pWeight;
int32_t Bridge_State;
int32_t Bridge_OldState;
float Bridge_DefaultZLiftPart;
float Bridge_DefaultZLiftRoad;
float Bridge_DefaultZLiftWeight;
float Bridge_OldLift;
uint32_t Bridge_TimeOfBridgeBecomingOperational;

void Bridge_Init(void)
{
    // Preserve the complete original Init operation and ordered live accesses
#ifdef GTA_BRIDGE
	Bridge_FindBridgeEntities();
	Bridge_OldLift = -1.0f;
	if (Bridge_pLiftPart && Bridge_pWeight)
	{
		Bridge_DefaultZLiftPart   = Bridge_BackendPositionZ(Bridge_pLiftPart);
		Bridge_DefaultZLiftWeight = Bridge_BackendPositionZ(Bridge_pWeight);

		if (Bridge_pLiftRoad)
			Bridge_DefaultZLiftRoad = Bridge_BackendPositionZ(Bridge_pLiftRoad);

		Bridge_BackendSetLights(-330.0, -230.0, -700.0, -588.0, true);
	}
#endif
}

void Bridge_Update(void)
{
    // Preserve the complete original Update operation and ordered live accesses
#ifdef GTA_BRIDGE
	if (!Bridge_pLiftPart || !Bridge_pWeight)
		return;

	Bridge_OldState = Bridge_State;

	float liftHeight;

	// Set bridge height and state
	if (Bridge_BackendCommercialPassed())
	{
		if (Bridge_TimeOfBridgeBecomingOperational == 0)
			Bridge_TimeOfBridgeBecomingOperational = (*Timer_GetTimeInMilliseconds());

		// Time remaining for bridge to become operational
		// uint16_t, so after about a minute it overflows to 0 and the cycle repeats
		uint16_t timeElapsed = (*Timer_GetTimeInMilliseconds()) - Bridge_TimeOfBridgeBecomingOperational;

		// Calculate lift part height and bridge state
		if (timeElapsed < 10000)
		{
			Bridge_State = STATE_LIFT_PART_MOVING_DOWN;
			liftHeight = 25.0f - timeElapsed / 10000.0f * 25.0f;
		}
		else if (timeElapsed < 40000)
		{
			liftHeight = 0.0f;
			Bridge_State = STATE_LIFT_PART_IS_DOWN;
		}
		else if (timeElapsed < 50000)
		{
			liftHeight = 0.0f;
			Bridge_State = STATE_LIFT_PART_ABOUT_TO_MOVE_UP;
		}
		else if (timeElapsed < 60000)
		{
			Bridge_State = STATE_LIFT_PART_MOVING_UP;
			liftHeight = (timeElapsed - 50000) / 10000.0f * 25.0f;
		}
		else
		{
			liftHeight = 25.0f;
			Bridge_State = STATE_LIFT_PART_IS_UP;
		}
	}
	else
	{
		liftHeight = 25.0f;
		Bridge_TimeOfBridgeBecomingOperational = 0;
		Bridge_State = STATE_BRIDGE_LOCKED;
	}

	// Move bridge part
	if (liftHeight != Bridge_OldLift)
	{
		Bridge_BackendSetPositionZ(Bridge_pLiftPart,Bridge_DefaultZLiftPart + liftHeight);
		Bridge_BackendUpdateMatrix(Bridge_pLiftPart);
		Bridge_BackendUpdateFrame(Bridge_pLiftPart);
		if (Bridge_pLiftRoad)
		{
			Bridge_BackendSetPositionZ(Bridge_pLiftRoad,Bridge_DefaultZLiftRoad + liftHeight);
			Bridge_BackendUpdateMatrix(Bridge_pLiftRoad);
			Bridge_BackendUpdateFrame(Bridge_pLiftRoad);
		}
		Bridge_BackendSetPositionZ(Bridge_pWeight,Bridge_DefaultZLiftWeight - liftHeight);
		Bridge_BackendUpdateMatrix(Bridge_pWeight);
		Bridge_BackendUpdateFrame(Bridge_pWeight);

		Bridge_OldLift = liftHeight;
	}

	if (Bridge_State == STATE_LIFT_PART_ABOUT_TO_MOVE_UP && Bridge_OldState == STATE_LIFT_PART_IS_DOWN)
		Bridge_BackendSetLights(-330.0, -230.0, -700.0, -588.0, true);
	else if (Bridge_State == STATE_LIFT_PART_IS_DOWN && Bridge_OldState == STATE_LIFT_PART_MOVING_DOWN)
		Bridge_BackendSetLights(-330.0, -230.0, -700.0, -588.0, false);
#endif
}

bool Bridge_ShouldLightsBeFlashing(void)
{
    // Preserve the complete original ShouldLightsBeFlashing operation and ordered live accesses
#ifdef GTA_BRIDGE
	return Bridge_State != STATE_LIFT_PART_IS_DOWN;
#else
	return false;
#endif
}

void Bridge_FindBridgeEntities(void)
{
    // Preserve the complete original FindBridgeEntities operation and ordered live accesses
#ifdef GTA_BRIDGE
	Bridge_pWeight   = NULL;
	Bridge_pLiftRoad = NULL;
	Bridge_pLiftPart = NULL;

	// Access raw storage through the C store or pool API
	for (int i = CPool_GetSize(Bridge_BackendBuildingPool())-1; i >= 0; i--) {
		// Access raw storage through the C store or pool API
		CEntity *entry = Bridge_BackendBuilding(CPool_GetSlot(Bridge_BackendBuildingPool(),i));

		if (entry)
		{
			if (Bridge_BackendModel(entry) == Bridge_BackendLiftModel())
				Bridge_pLiftPart = entry;
			else if (Bridge_BackendModel(entry) == Bridge_BackendRoadModel())
				Bridge_pLiftRoad = entry;
			else if (Bridge_BackendModel(entry) == Bridge_BackendWeightModel())
				Bridge_pWeight = entry;
		}
	}
#endif
}

bool Bridge_ThisIsABridgeObjectMovingUp(int32_t index)
{
    // Preserve the complete original ThisIsABridgeObjectMovingUp operation and ordered live accesses
#ifdef GTA_BRIDGE
	if (index != Bridge_BackendRoadModel() && index != Bridge_BackendLiftModel())
		return false;

	return Bridge_State == STATE_LIFT_PART_ABOUT_TO_MOVE_UP || Bridge_State == STATE_LIFT_PART_MOVING_UP;
#else
	return false;
#endif
}
//- rouz edit (ChatGPT)
