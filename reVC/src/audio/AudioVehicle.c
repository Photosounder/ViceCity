//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "../modelinfo/VehicleTypes.h"
#include "../modelinfo/VehicleAppearance.h"
#include "../modelinfo/ModelIds.h"
#include "../entities/EntityStatus.h"
#include "../core/GameAreas.h"
#include <stddef.h>

static float AudioVehicle_Velocity(CVehicle *vehicle)
{
    // Apply every original dot-product term to the resolved game vectors
    AudioSoundPosition speed, direction;
    AudioVehicleHost_VelocityVectors(vehicle, &speed, &direction);
    return speed.x * direction.x + speed.y * direction.y + speed.z * direction.z;
}

void
AudioVehicle_Process(cAudioManager *manager, CVehicle *veh)
{
	// Initialize the audio context explicitly before processing it
	cVehicleParams params;
	VehicleParams_Init(&params);

	// Keep the player exemption before rejecting unsupported areas
	if (AudioGame_FindVehicle() != veh) {
		switch (AudioDispatchHost_Area())
		{
		case AREA_OVALRING:
		case AREA_BLOOD:
		case AREA_DIRT:
		case AREA_EVERYWHERE:
		case AREA_MALL:
		case AREA_MAIN_MAP:
			break;
		default:
			return;
		}
	}

	// Copy the game position into the C sound record
	AudioReleaseHost_Position(veh, &manager->m_sQueueSample.m_vecPos);
	params.m_bDistanceCalculated = 0;
	params.m_pVehicle = veh;
	params.m_fDistance = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
	// Select the current handling transmission and capture the initial audio-table index
	params.m_pTransmission = AudioVehicleHost_Handling(veh) != NULL ? AudioVehicleHost_Transmission(AudioVehicleHost_Handling(veh)) : NULL;
	params.m_nIndex = AudioVehicleReverseHost_Model(veh) - MI_FIRST_VEHICLE;
	// Calculate velocity from simple traffic speed or the complete movement dot product
	if (AudioVehicleEngineHost_Status(veh) == STATUS_SIMPLE)
		params.m_fVelocityChange = AudioVehicleHost_TrafficSpeed(veh) * 0.02f;
	else
		params.m_fVelocityChange = AudioVehicle_Velocity(veh);
	// Capture vehicle type before any sound generator can change the parameter record
	params.m_VehicleType = AudioVehicleHost_Type(veh);
		
	// Retain the restricted mall generator order and its early return
	if (AudioDispatchHost_Area() == AREA_MALL && AudioGame_FindVehicle() != veh) {
		AudioVehicleOneShots_Process(manager, &params);
		AudioVehicleHorn_Siren(manager, &params);
		AudioVehicleCondition_Damage(manager, &params);
		return;
	}
	// Route each captured vehicle type through the original generator sequence
	switch (params.m_VehicleType) {
	case VEHICLE_TYPE_CAR:
		AudioVehiclePlayer_UpdateGas(veh, params.m_VehicleType);
		// Read each remote-control model comparison at its original live query point
		if (AudioVehicleReverseHost_Model(veh) == MI_RCBANDIT || AudioVehicleReverseHost_Model(veh) == MI_RCBARON) {
			AudioVehicleModel_Car(manager, &params);
			AudioVehicleCondition_Damage(manager, &params);
		} else if (AudioVehicleReverseHost_Model(veh) == MI_RCRAIDER || AudioVehicleReverseHost_Model(veh) == MI_RCGOBLIN) {
			AudioVehicleModel_Heli(manager, &params);
			AudioVehicleCondition_Damage(manager, &params);
		} else {
			// Choose car sound families from the live appearance query
			switch (AudioVehicleHost_Appearance(veh)) {
			case VEHICLE_APPEARANCE_HELI:
				AudioAircraft_Heli(manager, &params);
				AudioVehicleRoad_FlatTyre(manager, &params);
				AudioVehicleCondition_Damage(manager, &params);
				break;
			case VEHICLE_APPEARANCE_BOAT:
			case VEHICLE_APPEARANCE_PLANE:
				break;
			default:
				// Run ordinary car layers only when dry-road processing accepts the vehicle
				// Retain the bike road gate and its original engine and damage order
		if (AudioVehicleRoad_Dry(manager, &params)) {
					AudioVehicleReverse_Process(manager, &params);
					if (AudioVehicleRoadHost_WetRoads() > 0.0f)
						AudioVehicleRoad_Wet(manager, &params);
					AudioVehicleSkid_Process(manager, &params);
					AudioVehicleRoad_FlatTyre(manager, &params);
					AudioVehicleHorn_Horn(manager, &params);
					AudioVehicleHorn_Siren(manager, &params);
					if (AudioVehicleWarning_UsesReverse(params.m_nIndex))
						AudioVehicleWarning_Reverse(manager, &params);
					if(AudioVehicleWarning_HasAirBrakes(params.m_nIndex))
						AudioVehicleWarning_AirBrakes(manager, &params);
					AudioVehicleCondition_BombTick(manager, &params);
					AudioVehicleEngine_Process(manager, &params);
					AudioVehicleCondition_Damage(manager, &params);
					AudioVehicleDoors_Process(manager, &params);
				}
				break;
			}
		}
		AudioVehicleOneShots_Process(manager, &params);
		// Save the final parameter velocity on the captured automobile
		AudioVehicleHost_CarVelocity(veh, params.m_fVelocityChange);
		break;
	case VEHICLE_TYPE_BOAT:
		// Use helicopter audio for Skimmer before the shared boat layers
		if (AudioVehicleReverseHost_Model(veh) == MI_SKIMMER)
			AudioAircraft_Heli(manager, &params);
		else
			AudioBoat_Engine(manager, &params);
		AudioBoat_Water(manager, &params);
		AudioVehicleOneShots_Process(manager, &params);
		break;
	case VEHICLE_TYPE_HELI: 
		AudioAircraft_Heli(manager, &params);
		AudioVehicleOneShots_Process(manager, &params);
		break;
	case VEHICLE_TYPE_PLANE:
		AudioPlane_Plane(manager, &params);
		AudioVehicleOneShots_Process(manager, &params);
		AudioVehicleRoad_FlatTyre(manager, &params);
		break;
	case VEHICLE_TYPE_BIKE:
		AudioVehiclePlayer_UpdateGas(veh, params.m_VehicleType);
		// Retain the bike road gate and its original engine and damage order
		if (AudioVehicleRoad_Dry(manager, &params)) {
			if (AudioVehicleRoadHost_WetRoads() > 0.0f)
				AudioVehicleRoad_Wet(manager, &params);
			AudioVehicleSkid_Process(manager, &params);
			AudioVehicleHorn_Horn(manager, &params);
			AudioVehicleHorn_Siren(manager, &params);
			AudioVehicleCondition_BombTick(manager, &params);
			AudioVehicleCondition_Damage(manager, &params);
			AudioVehicleEngine_Process(manager, &params);
			AudioVehicleRoad_FlatTyre(manager, &params);
		}
		AudioVehicleOneShots_Process(manager, &params);
		// Save the final parameter velocity on the captured bike
		AudioVehicleHost_BikeVelocity(veh, params.m_fVelocityChange);
		break;
	default:
		break;
	}
	// Process rain after every supported or default vehicle routing branch
	AudioVehicleRain_Process(manager, &params);
}
//- rouz edit (ChatGPT)
