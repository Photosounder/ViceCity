//+ rouz edit (ChatGPT)
#include "Remote_backend.h"
#include "Timer.h"
#include "EntityStatus.h"
#include "VehicleAppearance.h"
#include "CameraModes.h"
#include <stddef.h>

void
Remote_GivePlayerRemoteControlledCar(float x, float y, float z, float rot, uint16_t model)
{
    // Construct the original native car before applying the complete C control sequence
    RemoteVehicle *car=Remote_BackendAllocate(model);
	bool found;

	z = Remote_BackendBaseDistance(car) + Remote_BackendGroundZ(x, y, z + 2.0f, &found);

	Remote_BackendRotate(car,rot);
	Remote_BackendPosition(car,x,y,z);
	Remote_BackendStatus(car,STATUS_PLAYER_REMOTE);
	Remote_BackendLocked(car,true);

	Remote_BackendJoinRoad(car);
	*Remote_BackendCarMission(car) = REMOTE_MISSION_NONE;
	*Remote_BackendTempAction(car) = REMOTE_TEMPACT_NONE;
	*Remote_BackendDrivingStyle(car) = REMOTE_DRIVINGSTYLE_STOP_FOR_CARS;
	*Remote_BackendCruiseSpeed(car) = *Remote_BackendMaxTrafficSpeed(car) = 9.0f;
	*Remote_BackendNextLane(car) = *Remote_BackendCurrentLane(car) = 0;
	Remote_BackendEngineOn(car,true);
	Remote_BackendAddWorld(car);
	if (Remote_BackendPlayerVehicle() != NULL) 
		Remote_BackendStatus(Remote_BackendPlayerVehicle(),STATUS_PLAYER_DISABLED);

	*Remote_BackendRemoteSlot() = car;
	Remote_BackendRegisterReference(*Remote_BackendRemoteSlot(),Remote_BackendRemoteSlot());
	if (Remote_BackendAppearance(car) == VEHICLE_APPEARANCE_PLANE || Remote_BackendAppearance(car) == VEHICLE_APPEARANCE_HELI) {
		Remote_BackendTakeCamera(car,CAMERA_MODE_CAM_ON_A_STRING);
		Remote_BackendZoom(0);
	} else
		Remote_BackendTakeCamera(car,CAMERA_MODE_BEHINDCAR);
}

void
Remote_TakeRemoteControlledCarFromPlayer(bool blowUp)
{
    // Retain the original ownership transition and every live player-state write
	if (*Remote_BackendCreatedBy(*Remote_BackendRemoteSlot()) == REMOTE_MISSION_VEHICLE) {
		*Remote_BackendCreatedBy(*Remote_BackendRemoteSlot()) = REMOTE_RANDOM_VEHICLE;
		(*Remote_BackendMissionCars())--;
		(*Remote_BackendRandomCars())++;
	}
	Remote_BackendLocked(*Remote_BackendRemoteSlot(),false);
	*Remote_BackendLostTime() = (*Timer_GetTimeInMilliseconds());
	*Remote_BackendRemoteMode() = true;
	*Remote_BackendBlowUp() = blowUp;
	*Remote_BackendLostFlag() = true;
}
//- rouz edit (ChatGPT)
