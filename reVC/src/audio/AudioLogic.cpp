//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "AudioManager.h"
#include "AudioSoundGame.h"
#include "audio_enums.h"

#include "Automobile.h"
#include "Boat.h"
#include "Bridge.h"
#include "Camera.h"
#include "Cranes.h"
#include "DMAudio.h"
#include "Entity.h"
#include "Explosion.h"
#include "Fire.h"
#include "Garages.h"
#include "General.h"
#include "HandlingMgr.h"
#include "Heli.h"
#include "ModelIndices.h"
#include "MusicManager.h"
#include "Pad.h"
#include "ParticleObject.h"
#include "Ped.h"
#include "Physical.h"
#include "Placeable.h"
#include "Plane.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "Projectile.h"
#include "ProjectileInfo.h"
#include "Replay.h"
#include "Stats.h"
#include "SurfaceTable.h"
#include "Train.h"
#include "Transmission.h"
#include "Vehicle.h"
#include "WaterCannon.h"
#include "Weather.h"
#include "ZoneCull.h"
#include "sampman.h"
#include "Bike.h"
#include "WindModifiers.h"
#include "WindModifiers_backend.h"
#include "Fluff.h"
#include "Script.h"
#include "Wanted.h"
#include "debugmenu.h"

#ifndef GTA_PS2
#define CHANNEL_PLAYER_VEHICLE_ENGINE m_nActiveSamples
#endif













enum
{








	VEHICLE_ENGINE_MAX_DIST = 50,
	VEHICLE_ENGINE_BASE_VOLUME = 75,
	VEHICLE_ENGINE_FULL_VOLUME = 90,








	ENGINE_DAMAGE_VOLUME = 6,
	ENGINE_DAMAGE_ON_FIRE_VOLUME = 60,
























	BOAT_ENGINE_LOW_ACCEL_VOLUME = 45,
	BOAT_ENGINE_HIGH_ACCEL_MIN_VOLUME = 15,
	BOAT_ENGINE_HIGH_ACCEL_VOLUME_MULT = 105,

















	PED_COMMENT_MAX_DIST = 40,
	PED_COMMENT_POLICE_HELI_MAX_DIST = 400,






	//CRANE_MAX_DIST = 80,
	//CRANE_VOLUME = 100,




	MISSION_AUDIO_MAX_DIST = 80,
	MISSION_AUDIO_VOLUME = 80,
};

#pragma region VEHICLE AUDIO
#include "AudioVehicleModels.h"

#include "AudioVehicleDoorTypes.h"


struct tVehicleSampleData {
	eSfxSample m_nAccelerationSampleIndex;
	uint8 m_nBank;
	eSfxSample m_nHornSample;
	int32 m_nHornFrequency;
	uint8 m_nSirenOrAlarmSample;
	int32 m_nSirenOrAlarmFrequency;
	uint8 m_bDoorType;
};

const tVehicleSampleData aVehicleSettings[MAX_CARS] = {
	{SFX_CAR_REV_10, SFX_BANK_PATHFINDER, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9935, OLD_DOOR},
	{SFX_CAR_REV_11, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 11487, SFX_CAR_HORN_JEEP, 9900, OLD_DOOR},
	{SFX_CAR_REV_2, SFX_BANK_PORSCHE, SFX_CAR_HORN_PORSCHE, 11025, SFX_CAR_HORN_JEEP, 9890, NEW_DOOR},
	{SFX_CAR_REV_5, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 29711, SFX_CAR_HORN_JEEP, 9960, TRUCK_DOOR},
	{SFX_CAR_REV_11, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 12893, SFX_CAR_HORN_JEEP, 9500, OLD_DOOR},
	{SFX_CAR_REV_4, SFX_BANK_MERC, SFX_CAR_HORN_BMW328, 10706, SFX_CAR_HORN_JEEP, 9600, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_TRUCK, 29711, SFX_CAR_HORN_JEEP, 9700, NEW_DOOR},
	{SFX_CAR_REV_5, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 29711, SFX_POLICE_SIREN_SLOW, 10588, TRUCK_DOOR},
	{SFX_CAR_REV_5, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 31478, SFX_CAR_HORN_JEEP, 9800, TRUCK_DOOR},
	{SFX_CAR_REV_11, SFX_BANK_PACARD, SFX_CAR_HORN_BMW328, 9538, SFX_CAR_HORN_JEEP, 9900, NEW_DOOR},
	{SFX_CAR_REV_11, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 10842, SFX_CAR_HORN_JEEP, 10000, OLD_DOOR},
	{SFX_CAR_REV_7, SFX_BANK_COBRA, SFX_CAR_HORN_BMW328, 12017, SFX_CAR_HORN_JEEP, 9900, NEW_DOOR},
	{SFX_CAR_REV_9, SFX_BANK_CADILLAC, SFX_CAR_HORN_JEEP, 22293, SFX_CAR_HORN_JEEP, 9800, NEW_DOOR},
	{SFX_CAR_REV_3, SFX_BANK_SPIDER, SFX_CAR_HORN_BUS2, 18000, SFX_CAR_HORN_JEEP, 9700, OLD_DOOR},
	{SFX_CAR_REV_3, SFX_BANK_SPIDER, SFX_CAR_HORN_BUS, 18286, SFX_CAR_HORN_JEEP, 9600, OLD_DOOR},
	{SFX_CAR_REV_2, SFX_BANK_PORSCHE, SFX_CAR_HORN_PORSCHE, 11025, SFX_CAR_HORN_JEEP, 9500, NEW_DOOR},
	{SFX_CAR_REV_3, SFX_BANK_SPIDER, SFX_CAR_HORN_JEEP, 22295, SFX_AMBULANCE_SIREN_SLOW, 12688, OLD_DOOR},
	{SFX_CAR_REV_4, SFX_BANK_MERC, SFX_CAR_HORN_PORSCHE, 9271, SFX_POLICE_SIREN_SLOW, 11471, NEW_DOOR},
	{SFX_CAR_REV_11, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 12170, SFX_CAR_HORN_JEEP, 9400, OLD_DOOR},
	{SFX_CAR_REV_11, SFX_BANK_PACARD, SFX_CAR_HORN_BMW328, 11000, SFX_CAR_HORN_JEEP, 9300, OLD_DOOR},
	{SFX_CAR_REV_10, SFX_BANK_PATHFINDER, SFX_CAR_HORN_BMW328, 10796, SFX_CAR_HORN_JEEP, 9200, NEW_DOOR},
	{SFX_CAR_REV_4, SFX_BANK_MERC, SFX_CAR_HORN_BMW328, 10500, SFX_CAR_HORN_JEEP, 9100, NEW_DOOR},
	{SFX_CAR_REV_10, SFX_BANK_PATHFINDER, SFX_CAR_HORN_PICKUP, 10924, SFX_CAR_HORN_JEEP, 9000, OLD_DOOR},
	{SFX_CAR_REV_3, SFX_BANK_SPIDER, SFX_CAR_HORN_PICKUP, 11025, SFX_ICE_CREAM_TUNE, 11025, OLD_DOOR},
	{SFX_CAR_REV_6, SFX_BANK_HOTROD, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9100, OLD_DOOR},
	{SFX_HELI_APACHE_1, SFX_BANK_HELI_APACHE, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9200, NEW_DOOR},
	{SFX_CAR_REV_4, SFX_BANK_MERC, SFX_CAR_HORN_BMW328, 10706, SFX_POLICE_SIREN_SLOW, 10511, NEW_DOOR},
	{SFX_CAR_REV_3, SFX_BANK_SPIDER, SFX_CAR_HORN_BUS, 17260, SFX_POLICE_SIREN_SLOW, 11029, OLD_DOOR},
	{SFX_CAR_REV_3, SFX_BANK_SPIDER, SFX_CAR_HORN_PICKUP, 8670, SFX_CAR_HORN_JEEP, 9300, OLD_DOOR},
	{SFX_CAR_REV_7, SFX_BANK_COBRA, SFX_CAR_HORN_PORSCHE, 10400, SFX_CAR_HORN_JEEP, 9400, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_POLICE_SIREN_SLOW, 11912, NEW_DOOR},
	{SFX_CAR_REV_5, SFX_BANK_TRUCK, SFX_CAR_HORN_BUS2, 11652, SFX_CAR_HORN_JEEP, 9500, BUS_DOOR},
	{SFX_CAR_REV_5, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 29711, SFX_CAR_HORN_JEEP, 9600, TRUCK_DOOR},
	{SFX_CAR_REV_5, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 28043, SFX_CAR_HORN_JEEP, 9700, TRUCK_DOOR},
	{SFX_CAR_REV_6, SFX_BANK_HOTROD, SFX_CAR_HORN_JEEP, 25400, SFX_CAR_HORN_JEEP, 9800, OLD_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9900, NEW_DOOR},
	{SFX_CAR_REV_17, SFX_BANK_VTWIN, SFX_CAR_HORN_JEEP, 26313, SFX_CAR_HORN_JEEP, 10000, NEW_DOOR},
	{SFX_CAR_REV_5, SFX_BANK_TRUCK, SFX_CAR_HORN_BUS, 16291, SFX_CAR_HORN_JEEP, 10100, BUS_DOOR},
	{SFX_CAR_REV_11, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 10842, SFX_CAR_HORN_JEEP, 9900, OLD_DOOR},
	{SFX_CAR_REV_11, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 10233, SFX_CAR_HORN_JEEP, 9800, NEW_DOOR},
	{SFX_CAR_REV_3, SFX_BANK_SPIDER, SFX_CAR_HORN_PICKUP, 8670, SFX_CAR_HORN_JEEP, 9700, OLD_DOOR},
	{SFX_RC_REV, SFX_BANK_RC, SFX_CAR_HORN_PICKUP, 20000, SFX_CAR_HORN_JEEP, 9600, NEW_DOOR},
	{SFX_CAR_REV_11, SFX_BANK_PACARD, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9500, NEW_DOOR},
	{SFX_CAR_REV_5, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 29000, SFX_CAR_HORN_JEEP, 9400, TRUCK_DOOR},
	{SFX_CAR_REV_1, CAR_SFX_BANKS_OFFSET, SFX_CAR_HORN_BMW328, 9003, SFX_CAR_HORN_JEEP, 9300, NEW_DOOR},
	{SFX_CAR_REV_4, SFX_BANK_MERC, SFX_CAR_HORN_PORSCHE, 12375, SFX_CAR_HORN_JEEP, 9200, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_BUS2, 15554, SFX_CAR_HORN_JEEP, 9100, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_BUS2, 13857, SFX_CAR_HORN_JEEP, 9000, TRUCK_DOOR},
	{SFX_MOPED_REV, SFX_BANK_MOPED, SFX_CAR_HORN_JEEP, 30000, SFX_CAR_HORN_JEEP, 9100, NEW_DOOR},
	{SFX_CAR_REV_7, SFX_BANK_COBRA, SFX_CAR_HORN_JEEP, 22043, SFX_CAR_HORN_JEEP, 9200, OLD_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9300, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9400, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9500, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9600, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 21043, SFX_CAR_HORN_JEEP, 9700, NEW_DOOR},
	{SFX_CAR_REV_5, SFX_BANK_TRUCK, SFX_CAR_HORN_TRUCK, 28043, SFX_CAR_HORN_JEEP, 9800, TRUCK_DOOR},
	{SFX_CAR_REV_3, SFX_BANK_SPIDER, SFX_CAR_HORN_BUS, 18286, SFX_CAR_HORN_JEEP, 9900, OLD_DOOR},
	{SFX_CAR_REV_12, SFX_BANK_GOLF_CART, SFX_CAR_HORN_JEEP, 28500, SFX_CAR_HORN_JEEP, 9800, NEW_DOOR},
	{SFX_CAR_REV_1, CAR_SFX_BANKS_OFFSET, SFX_CAR_HORN_56CHEV, 10842, SFX_CAR_HORN_JEEP, 9700, OLD_DOOR},
	{SFX_CAR_REV_8, SFX_BANK_PONTIAC_SLOW, SFX_CAR_HORN_BUS2, 18000, SFX_CAR_HORN_JEEP, 9700, OLD_DOOR},
	{SFX_SEAPLANE_PRO1, SFX_BANK_PLANE_SEAPLANE, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9700, NEW_DOOR},
	{SFX_CAR_REV_20, SFX_BANK_SPORTS_BIKE, SFX_CAR_HORN_JEEP, 27000, SFX_CAR_HORN_JEEP, 9600, NEW_DOOR},
	{SFX_MOPED_REV, SFX_BANK_MOPED, SFX_CAR_HORN_JEEP, 31000, SFX_CAR_HORN_JEEP, 9500, NEW_DOOR},
	{SFX_CAR_REV_17, SFX_BANK_VTWIN, SFX_CAR_HORN_PICKUP, 11000, SFX_CAR_HORN_JEEP, 9400, NEW_DOOR},
	{SFX_RC_REV, SFX_BANK_RC, SFX_CAR_HORN_JEEP, 30000, SFX_CAR_HORN_JEEP, 15000, NEW_DOOR},
	{SFX_CAR_RC_HELI, SFX_BANK_RC_HELI, SFX_CAR_HORN_JEEP, 30000, SFX_CAR_HORN_JEEP, 15000, NEW_DOOR},
	{SFX_CAR_REV_9, SFX_BANK_CADILLAC, SFX_CAR_HORN_56CHEV, 10300, SFX_CAR_HORN_JEEP, 9100, OLD_DOOR},
	{SFX_CAR_REV_9, SFX_BANK_CADILLAC, SFX_CAR_HORN_56CHEV, 10500, SFX_CAR_HORN_JEEP, 9000, OLD_DOOR},
	{SFX_CAR_REV_19, SFX_BANK_HONDA250, SFX_CAR_HORN_JEEP, 30000, SFX_CAR_HORN_JEEP, 9000, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9100, TRUCK_DOOR},
	{SFX_CAR_REV_3, SFX_BANK_SPIDER, SFX_CAR_HORN_TRUCK, 28000, SFX_CAR_HORN_JEEP, 9200, TRUCK_DOOR},
	{SFX_CAR_REV_7, SFX_BANK_COBRA, SFX_CAR_HORN_PICKUP, 11200, SFX_CAR_HORN_JEEP, 9300, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9400, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9500, NEW_DOOR},
	{SFX_CAR_REV_9, SFX_BANK_CADILLAC, SFX_CAR_HORN_56CHEV, 10700, SFX_CAR_HORN_JEEP, 9600, OLD_DOOR},
	{SFX_CAR_REV_1, CAR_SFX_BANKS_OFFSET, SFX_CAR_HORN_BMW328, 9000, SFX_CAR_HORN_JEEP, 9700, OLD_DOOR},
	{SFX_CAR_REV_6, SFX_BANK_HOTROD, SFX_CAR_HORN_BMW328, 9200, SFX_CAR_HORN_JEEP, 9800, OLD_DOOR},
	{SFX_CAR_REV_7, SFX_BANK_COBRA, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9900, NEW_DOOR},
	{SFX_CAR_REV_11, SFX_BANK_PACARD, SFX_CAR_HORN_56CHEV, 10540, SFX_CAR_HORN_JEEP, 9935, TRUCK_DOOR},
	{SFX_CAR_REV_8, SFX_BANK_PONTIAC_SLOW, SFX_CAR_HORN_PICKUP, 11000, SFX_CAR_HORN_JEEP, 9700, NEW_DOOR},
	{SFX_CAR_REV_2, SFX_BANK_PORSCHE, SFX_CAR_HORN_BMW328, 9500, SFX_CAR_HORN_JEEP, 9800, NEW_DOOR},
	{SFX_CAR_REV_7, SFX_BANK_COBRA, SFX_CAR_HORN_BMW328, 9700, SFX_CAR_HORN_JEEP, 9700, NEW_DOOR},
	{SFX_CAR_REV_8, SFX_BANK_PONTIAC_SLOW, SFX_CAR_HORN_BUS2, 18000, SFX_CAR_HORN_JEEP, 9600, OLD_DOOR},
	{SFX_CAR_REV_3, SFX_BANK_SPIDER, SFX_CAR_HORN_BUS, 18000, SFX_CAR_HORN_JEEP, 9500, TRUCK_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9400, NEW_DOOR},
	{SFX_CAR_REV_8, SFX_BANK_PONTIAC_SLOW, SFX_CAR_HORN_JEEP, 27513, SFX_CAR_HORN_JEEP, 9300, NEW_DOOR},
	{SFX_CAR_REV_8, SFX_BANK_PONTIAC_SLOW, SFX_CAR_HORN_56CHEV, 10700, SFX_CAR_HORN_JEEP, 9200, OLD_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9100, TRUCK_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9000, TRUCK_DOOR},
	{SFX_CAR_REV_10, SFX_BANK_PATHFINDER, SFX_CAR_HORN_BUS2, 18000, SFX_CAR_HORN_JEEP, 9100, TRUCK_DOOR},
	{SFX_CAR_REV_1, CAR_SFX_BANKS_OFFSET, SFX_CAR_HORN_BUS2, 17900, SFX_POLICE_SIREN_SLOW, 10511, TRUCK_DOOR},
	{SFX_CAR_REV_4, SFX_BANK_MERC, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9200, NEW_DOOR},
	{SFX_CAR_REV_8, SFX_BANK_PONTIAC_SLOW, SFX_CAR_HORN_BMW328, 9600, SFX_CAR_HORN_JEEP, 9300, NEW_DOOR},
	{SFX_CAR_REV_4, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9400, NEW_DOOR},
	{SFX_CAR_REV_7, SFX_BANK_COBRA, SFX_CAR_HORN_PORSCHE, 10000, SFX_CAR_HORN_JEEP, 9500, OLD_DOOR},
	{SFX_CAR_REV_6, SFX_BANK_HOTROD, SFX_CAR_HORN_PORSCHE, 10500, SFX_CAR_HORN_JEEP, 9600, OLD_DOOR},
	{SFX_CAR_REV_10, SFX_BANK_PATHFINDER, SFX_CAR_HORN_JEEP, 25513, SFX_CAR_HORN_JEEP, 9700, NEW_DOOR},
	{SFX_CAR_REV_1, SFX_BANK_0, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9800, NEW_DOOR},
	{SFX_CAR_REV_3, SFX_BANK_SPIDER, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9900, NEW_DOOR},
	{SFX_CAR_REV_10, SFX_BANK_PATHFINDER, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9800, NEW_DOOR},
	{SFX_CAR_REV_1, CAR_SFX_BANKS_OFFSET, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9700, NEW_DOOR},
	{SFX_CAR_RC_HELI, SFX_BANK_RC_HELI, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9600, NEW_DOOR},
	{SFX_CAR_REV_6, SFX_BANK_HOTROD, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9700, NEW_DOOR},
	{SFX_CAR_REV_7, SFX_BANK_COBRA, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9600, NEW_DOOR},
	{SFX_CAR_REV_1, CAR_SFX_BANKS_OFFSET, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9500, NEW_DOOR},
	{SFX_CAR_REV_9, SFX_BANK_CADILLAC, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9400, NEW_DOOR},
	{SFX_CAR_REV_2, SFX_BANK_PORSCHE, SFX_CAR_HORN_PORSCHE, 11025, SFX_POLICE_SIREN_SLOW, 11000, NEW_DOOR},
	{SFX_CAR_REV_1, CAR_SFX_BANKS_OFFSET, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9200, NEW_DOOR},
	{SFX_CAR_REV_1, CAR_SFX_BANKS_OFFSET, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9300, NEW_DOOR},
	{SFX_CAR_REV_1, CAR_SFX_BANKS_OFFSET, SFX_CAR_HORN_JEEP, 26513, SFX_CAR_HORN_JEEP, 9400, NEW_DOOR} };


































#ifdef GTA_TRAIN
#endif



#pragma region JUMBO















#pragma endregion Some jumbo crap

#pragma endregion All the vehicle audio code

#pragma region PED AUDIO








#pragma region PED_COMMENTS




































































































#ifdef DEBUGMENU
uint32 nDebugPlayPedComment = SAMPLEBANK_PED_START;

void DebugMenuPlayPedComment()
{
	// Submit the debug selection through the complete C comment producer
	AudioPedPlayback_Debug(&AudioManager, nDebugPlayPedComment);
}

SETTWEAKPATH("Audio");
TWEAKUINT32N(nDebugPlayPedComment, SAMPLEBANK_PED_START, SAMPLEBANK_PED_END, 1, "Ped Comment ID");
TWEAKFUNCN(DebugMenuPlayPedComment, "Play Ped Comment");

#endif




#pragma endregion

#pragma endregion All the ped audio code




#pragma region SCRIPT_OBJECTS


#pragma endregion All the code for script object audio on the map



/*void
cAudioManager::ProcessCrane()
{
	CCrane *crane = (CCrane *)m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_pEntity;
	float distSquared;
	bool8 distCalculated = FALSE;

	if (crane) {
		if (crane->m_nCraneStatus == CCrane::ACTIVATED) {
			if (crane->m_nCraneState != CCrane::IDLE) {
				// Copy the game position into the C sound record
				m_sQueueSample.m_vecPos = AudioSoundPosition_FromVector(crane->m_pCraneEntity->GetPosition());
				distSquared = AudioGeometry_DistanceSquared(&m_sQueueSample.m_vecPos);
				if (distSquared < SQR(CRANE_MAX_DIST)) {
					AudioMath_CalculateDistance(&distCalculated, &m_sQueueSample.m_fDistance, distSquared);
					m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(CRANE_VOLUME, CRANE_MAX_DIST, m_sQueueSample.m_fDistance);
					if (m_sQueueSample.m_nVolume > 0) {
						m_sQueueSample.m_nCounter = 0;
						m_sQueueSample.m_nSampleIndex = SFX_CRANE_MAGNET;
						m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						m_sQueueSample.m_bIs2D = FALSE;
						m_sQueueSample.m_nPriority = 2;
						m_sQueueSample.m_nFrequency = 6000;
						m_sQueueSample.m_nLoopCount = 0;
						SET_EMITTING_VOLUME(CRANE_VOLUME);
						SET_LOOP_OFFSETS(m_sQueueSample.m_nSampleIndex)
						m_sQueueSample.m_fSpeedMultiplier = 4.0f;
						m_sQueueSample.m_MaxDistance = CRANE_MAX_DIST;
						m_sQueueSample.m_bStatic = FALSE;
						m_sQueueSample.m_nFramesToPlay = 3;
						SET_SOUND_REVERB(TRUE);
						SET_SOUND_REFLECTION(FALSE);
						AudioRequests_Submit(this);
					}
					if (m_asAudioEntities[m_sQueueSample.m_nEntityIndex].m_AudioEvents > 0) {
						m_sQueueSample.m_nCounter = 1;
						m_sQueueSample.m_nSampleIndex = SFX_COL_CAR_2;
						m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_COL_CAR_2);
						m_sQueueSample.m_nLoopCount = 1;
						m_sQueueSample.m_bStatic = TRUE;
						SET_SOUND_REVERB(TRUE);
						SET_SOUND_REFLECTION(TRUE);
						AudioRequests_Submit(this);
					}
				}
			}
		}
	}
}*/



//positon of arcade machines



#ifdef GTA_BRIDGE
#pragma region BRIDGE



#pragma endregion
#endif

#pragma region MISSION_AUDIO















//+ rouz edit (ChatGPT)



//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
void
AudioGeometryHost_CameraPosition(AudioSoundPosition *position)
{
    // Copy the existing camera coordinates without assuming the game vector layout
    const CVector &camera = TheCamera.GetPosition();
    position->x = camera.x;
    position->y = camera.y;
    position->z = camera.z;
}

void
AudioGeometryHost_CameraMatrix(AudioCameraMatrix *matrix)
{
    // Copy only the basis and translation fields used by the original inverse multiply
    const CMatrix &camera = TheCamera.GetMatrix();
	matrix->rx = camera.rx;
	matrix->ry = camera.ry;
	matrix->rz = camera.rz;
	matrix->fx = camera.fx;
	matrix->fy = camera.fy;
	matrix->fz = camera.fz;
	matrix->ux = camera.ux;
	matrix->uy = camera.uy;
	matrix->uz = camera.uz;
	matrix->px = camera.px;
	matrix->py = camera.py;
	matrix->pz = camera.pz;
}
//- rouz edit (ChatGPT)
#pragma endregion All the mission audio stuff
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
uint8_t
AudioServiceHost_IsPed(CPed *ped)
{
    // Preserve the original game type test on each physical entity
    return ped->IsPed();
}

void
AudioServiceHost_PedLastStart(CPed *ped, uint32_t time)
{
    // Write the previous sound start before evaluating the randomized next start
    ped->m_lastSoundStart = time;
}

void
AudioServiceHost_PedSoundStart(CPed *ped, uint32_t time)
{
    // Write the randomized sound start using the original unsigned timer arithmetic
    ped->m_soundStart = time;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)











#ifdef GTA_BRIDGE
#endif

int32_t
AudioDispatchHost_Area(void)
{
    // Read each original current-area field access independently
    return CGame::currArea;
}

int32_t
AudioDispatchHost_PhysicalType(void *entity)
{
    // Query the same base physical object used by the original type switch
    return ((CPhysical *)entity)->GetType();
}


//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
float
AudioVehicleRainHost_Rain(void)
{
    // Read rainfall at each original gate and emitting-volume query
    return Weather_Rain;
}



void
AudioVehicleRainHost_Counters(CVehicle *vehicle, uint8_t **audioCounter, uint8_t **sampleCounter)
{
    // Expose the original byte counters without copying or changing the vehicle layout
    *audioCounter = &vehicle->m_bRainAudioCounter;
    *sampleCounter = &vehicle->m_bRainSamplesCounter;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
uint8_t
AudioVehicleRoadHost_WheelStatus(CVehicle *vehicle, int32_t type, int32_t wheel)
{
    // Read each original car damage query or bike wheel status at its loop position
    if(type == VEHICLE_TYPE_CAR) return Damage_GetWheelStatus(&((CAutomobile *)vehicle)->Damage, wheel);
    return ((CBike *)vehicle)->m_wheelStatus[wheel];
}

float
AudioVehicleRoadHost_WheelTimer(CVehicle *vehicle, int32_t type, int32_t wheel)
{
    // Read the wheel contact timer only after the corresponding burst status test
    if(type == VEHICLE_TYPE_CAR) return ((CAutomobile *)vehicle)->m_aWheelTimer[wheel];
    return ((CBike *)vehicle)->m_aWheelTimer[wheel];
}

uint8_t
AudioVehicleRoadHost_WheelsOnGround(CVehicle *vehicle, int32_t type)
{
    // Read the original car or bike ground-contact byte in its selected branch
    if(type == VEHICLE_TYPE_CAR) return ((CAutomobile *)vehicle)->m_nWheelsOnGround;
    return ((CBike *)vehicle)->m_nWheelsOnGround;
}


uint8_t
AudioVehicleRoadHost_Surface(const CVehicle *vehicle)
{
    // Read the current vehicle surface before selecting water or road audio
    return vehicle->m_nSurfaceTouched;
}

float
AudioVehicleRoadHost_WetRoads(void)
{
    // Read current road wetness after the relative velocity calculation
    return Weather_WetRoads;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
int16_t
AudioVehicleReverseHost_Model(const CVehicle *vehicle)
{
    // Query the original model index before the caddy exclusion
    return vehicle->GetModelIndex();
}

uint8_t
AudioVehicleReverseHost_Engine(const CVehicle *vehicle)
{
    // Read the engine bit on the initially captured vehicle
    return vehicle->bEngineOn;
}

float
AudioVehicleReverseHost_Gas(const CVehicle *vehicle)
{
    // Read the gas pedal at each original captured or live vehicle check
    return vehicle->m_fGasPedal;
}

uint8_t
AudioVehicleReverseHost_Gear(const CVehicle *vehicle)
{
    // Read the gear only when the captured vehicle's gas pedal did not select reverse
    return vehicle->m_nCurrentGear;
}

uint8_t
AudioVehicleReverseHost_DriveWheels(const CVehicle *vehicle)
{
    // Read current drive-wheel contact from the later automobile capture
    return ((const CAutomobile *)vehicle)->m_nDriveWheelsOnGround;
}

uint8_t
AudioVehicleReverseHost_PreviousDriveWheels(const CVehicle *vehicle)
{
    // Read previous drive-wheel contact only on the airborne branch
    return ((const CAutomobile *)vehicle)->m_nDriveWheelsOnGroundPrev;
}

float *
AudioVehicleReverseHost_GasAudio(CVehicle *vehicle)
{
    // Expose the original float field so its decay and subsequent read run in C
    return &((CAutomobile *)vehicle)->m_fGasPedalAudio;
}

//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
int16_t
AudioVehicleModelHost_Model(const CVehicle *vehicle)
{
    // Read the original model field before selecting the RC bandit branch
    return vehicle->m_modelIndex;
}

float
AudioVehicleModelHost_BrakePedal(const CVehicle *vehicle)
{
    // Read the current vehicle brake pedal after the non-player gas query
    return vehicle->m_fBrakePedal;
}

uint8_t
AudioVehicleModelHost_Drowning(const CVehicle *vehicle)
{
    // Read the original drowning bit before emitting-volume attenuation
    return vehicle->bIsDrowning;
}

int16_t
AudioVehicleModelHost_Brake(void)
{
    // Query the original primary pad brake at each model-specific control point
    return Pads[0].GetBrake();
}

int16_t
AudioVehicleModelHost_Accelerate(void)
{
    // Query the original primary pad accelerator at every macro evaluation
    return Pads[0].GetAccelerate();
}

int16_t
AudioVehicleModelHost_GunUpDown(void)
{
    // Query the original primary pad gun axis at every helicopter input evaluation
    return Pads[0].GetCarGunUpDown();
}

#ifdef THIS_IS_STUPID
void
AudioVehicleModelHost_Up(CVehicle *vehicle, AudioSoundPosition *up)
{
    // Copy the original up vector for the alternate C dot-product expression
    *up = AudioSoundPosition_FromVector(vehicle->GetUp()); // rouz edit (ChatGPT)
}
#else
float
AudioVehicleModelHost_UpY(CVehicle *vehicle)
{
    // Read only the original up-vector Y component on the normal branch
    return vehicle->GetUp().y; // rouz edit (ChatGPT)
}
#endif
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
int32_t
AudioVehicleEngineHost_Status(const CVehicle *vehicle)
{
    // Read the original field at the engine generator access point
    return vehicle->GetStatus();
}

uint8_t
AudioVehicleEngineHost_Handbrake(const CVehicle *vehicle)
{
    // Read the original field at the engine generator access point
    return vehicle->bIsHandbrakeOn;
}





uint8_t
AudioVehicleEngineHost_BikeWheels(const CVehicle *vehicle)
{
    // Read the original field at the engine generator access point
    return ((const CBike *)vehicle)->m_nDriveWheelsOnGround;
}

uint8_t
AudioVehicleEngineHost_BikePreviousWheels(const CVehicle *vehicle)
{
    // Read the original field at the engine generator access point
    return ((const CBike *)vehicle)->m_nDriveWheelsOnGroundPrev;
}

float *
AudioVehicleEngineHost_BikeGasAudio(CVehicle *vehicle)
{
    // Read the original field at the engine generator access point
    return &((CBike *)vehicle)->m_fGasPedalAudio;
}

enum tWheelState *
AudioVehicleEngineHost_CarStates(CVehicle *vehicle)
{
    // Read the original field at the engine generator access point
    return ((CAutomobile *)vehicle)->m_aWheelState;
}

enum tWheelState *
AudioVehicleEngineHost_BikeStates(CVehicle *vehicle)
{
    // Read the original field at the engine generator access point
    return ((CBike *)vehicle)->m_aWheelState;
}

uint8_t
AudioVehicleEngineHost_Bank(int32_t index)
{
    // Read the original field at the engine generator access point
    return aVehicleSettings[index].m_nBank;
}

int32_t
AudioVehicleEngineHost_Acceleration(int32_t index)
{
    // Read the original field at the engine generator access point
    return aVehicleSettings[index].m_nAccelerationSampleIndex;
}


void
AudioVehicleEngineHost_Unknown(int32_t type)
{
    // Retain the original diagnostic for unsupported vehicle types
    debug(" ** AUDIOLOG: Unrecognised vehicle type %d in ProcessVehicleEngine() * \n", type);
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
float
AudioPlayerEngineHost_CarVelocity(const CVehicle *vehicle)
{
    // Read the original game field at the player engine access point
    return ((const CAutomobile *)vehicle)->m_fVelocityChangeForAudio;
}

float
AudioPlayerEngineHost_BikeVelocity(const CVehicle *vehicle)
{
    // Read the original game field at the player engine access point
    return ((const CBike *)vehicle)->m_fVelocityChangeForAudio;
}

uint8_t
AudioPlayerEngineHost_ExtraSpeed(const CVehicle *vehicle)
{
    // Read the original game field at the player engine access point
    return ((const CBike *)vehicle)->bExtraSpeed;
}

uint8_t
AudioPlayerEngineHost_Sand(const CVehicle *vehicle)
{
    // Read the original game field at the player engine access point
    return ((const CAutomobile *)vehicle)->bStuckInSand;
}

float
AudioPlayerEngineHost_SpeedZ(const CVehicle *vehicle)
{
    // Read the original game field at the player engine access point
    return vehicle->m_vecMoveSpeed.z;
}

uint32_t
AudioPlayerEngineHost_Time(void)
{
    // Read the original game field at the player engine access point
    return (*Timer_GetTimeInMilliseconds());
}

#ifdef FIX_BUGS
uint8_t
AudioPlayerEngineHost_Exiting(void)
{
    // Query the original player ped only after the replay short circuit
    return FindPlayerPed()->GetPedState() == PED_EXIT_CAR;
}
#endif

void
AudioPlayerEngineHost_ChangingGear(CVehicle *vehicle)
{
    // Set the original changing-gear bit before channel initialization or cruising
    vehicle->bAudioChangingGear = TRUE;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
float
AudioVehicleSkidHost_CarGas(const CVehicle *vehicle)
{
    // Copy the original automobile gas-audio value after capturing wheel states
    return ((const CAutomobile *)vehicle)->m_fGasPedalAudio;
}

float
AudioVehicleSkidHost_BikeGas(const CVehicle *vehicle)
{
    // Copy the original bike gas-audio value after capturing wheel states
    return ((const CBike *)vehicle)->m_fGasPedalAudio;
}

float
AudioVehicleSkidHost_TurnZ(const CVehicle *vehicle)
{
    // Read the original turn-speed component at every skid-value expression evaluation
    return vehicle->m_vecTurnSpeed.z;
}

void
AudioVehicleSkidHost_Unknown(int32_t type)
{
    // Retain the original unsupported vehicle diagnostic
    debug("\n * AUDIOLOG:  ProcessVehicleSkidding() Unsupported vehicle type %d * \n", type);
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
uint8_t
AudioVehicleHornHost_Enabled(const CVehicle *vehicle)
{
    // Read the original vehicle predicate or shared sound setting at its access point
    return vehicle->m_bSirenOrAlarm;
}

uint8_t
AudioVehicleHornHost_Alarm(CVehicle *vehicle)
{
    // Read the original vehicle predicate or shared sound setting at its access point
    return vehicle->IsAlarmOn();
}

uint8_t
AudioVehicleHornHost_Uses(CVehicle *vehicle)
{
    // Read the original vehicle predicate or shared sound setting at its access point
    return vehicle->UsesSiren();
}

uint32_t
AudioVehicleHornHost_Timer(const CVehicle *vehicle)
{
    // Read the original vehicle predicate or shared sound setting at its access point
    return vehicle->m_nCarHornTimer;
}

uint8_t
AudioVehicleHornHost_Pattern(const CVehicle *vehicle)
{
    // Read the original vehicle predicate or shared sound setting at its access point
    return vehicle->m_nCarHornPattern;
}

int32_t
AudioVehicleHornHost_HornSample(uint32_t index)
{
    // Read the original vehicle predicate or shared sound setting at its access point
    return aVehicleSettings[index].m_nHornSample;
}

int32_t
AudioVehicleHornHost_HornFrequency(uint32_t index)
{
    // Read the original vehicle predicate or shared sound setting at its access point
    return aVehicleSettings[index].m_nHornFrequency;
}

uint8_t
AudioVehicleHornHost_SirenSample(uint32_t index)
{
    // Read the original vehicle predicate or shared sound setting at its access point
    return aVehicleSettings[index].m_nSirenOrAlarmSample;
}

int32_t
AudioVehicleHornHost_SirenFrequency(uint32_t index)
{
    // Read the original vehicle predicate or shared sound setting at its access point
    return aVehicleSettings[index].m_nSirenOrAlarmFrequency;
}

void
AudioVehicleHornHost_SetTimer(CVehicle *vehicle, uint32_t value)
{
    // Update the original vehicle field at the horn or alarm assignment point
    vehicle->m_nCarHornTimer = value;
}

void
AudioVehicleHornHost_SetPattern(CVehicle *vehicle, uint8_t value)
{
    // Update the original vehicle field at the horn or alarm assignment point
    vehicle->m_nCarHornPattern = value;
}

//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" int32_t
AudioVehicleDoorsHost_Status(CVehicle *vehicle, uint8_t door)
{
	// Query the original damage manager on the captured automobile
	return Damage_GetDoorStatus(&((CAutomobile *)vehicle)->Damage, door);
}
extern "C" int8_t
AudioVehicleDoorsHost_State(const CVehicle *vehicle, uint8_t door)
{
	// Read the original signed door state
	return ((const CAutomobile *)vehicle)->Doors[door].m_nDoorState;
}
extern "C" float
AudioVehicleDoorsHost_Velocity(const CVehicle *vehicle, uint8_t door)
{
	// Read each original angular velocity expression
	return ((const CAutomobile *)vehicle)->Doors[door].m_fAngVel;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" float
AudioVehicleConditionHost_Health(const CVehicle *vehicle)
{
	// Read health at each original condition evaluation
	return vehicle->m_fHealth;
}
extern "C" uint8_t
AudioVehicleConditionHost_Bomb(const CVehicle *vehicle)
{
	// Read the original three-bit bomb type on the current vehicle
	return vehicle->m_bombType;
}
extern "C" void
AudioVehicleConditionHost_Unknown(int32_t type)
{
	// Retain the original unsupported vehicle diagnostic
	debug("\n * AUDIOLOG:  ProcessCarBombTick()  Unsupported vehicle type %d * \n", type);
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" uint8_t
AudioBoatHost_Propeller(const CVehicle *vehicle)
{
	// Read the captured boat propeller-in-water bit
	return ((const CBoat *)vehicle)->bPropellerInWater;
}
extern "C" uint8_t
AudioBoatHost_InWater(const CVehicle *vehicle)
{
	// Read the live boat-in-water bit after the velocity threshold
	return ((const CBoat *)vehicle)->bBoatInWater;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" float
AudioAircraftHost_MovingSpeed(const CVehicle *vehicle)
{
	// Read the original captured rotor speed input
	return ((const CBoat *)vehicle)->m_fMovingSpeed;
}
extern "C" float
AudioAircraftHost_WheelSpeed(const CVehicle *vehicle)
{
	// Read the original captured rotor speed input
	return ((const CAutomobile *)vehicle)->m_aWheelSpeed[1];
}
extern "C" void
AudioAircraftHost_Forward(CVehicle *vehicle, AudioSoundPosition *forward)
{
	// Copy the live vehicle forward vector for the original dot product
	*forward = AudioSoundPosition_FromVector(vehicle->GetMatrix().GetForward());
}
extern "C" void
AudioAircraftHost_Boot(CVehicle *vehicle, AudioSoundPosition *position)
{
	// Resolve the original boot component on the captured automobile
	CVector point;
	((CAutomobile *)vehicle)->GetComponentWorldPosition(CAR_BOOT, point);
	*position = AudioSoundPosition_FromVector(point);
}
extern "C" void
AudioAircraftHost_Matrix(CVehicle *vehicle, AudioCameraMatrix *matrix)
{
	// Copy the original vehicle basis and translation for the rear rotor transform
	const CMatrix &value = vehicle->GetMatrix();
	matrix->rx = value.rx;
	matrix->ry = value.ry;
	matrix->rz = value.rz;
	matrix->fx = value.fx;
	matrix->fy = value.fy;
	matrix->fz = value.fz;
	matrix->ux = value.ux;
	matrix->uy = value.uy;
	matrix->uz = value.uz;
	matrix->px = value.px;
	matrix->py = value.py;
	matrix->pz = value.pz;
}
extern "C" uint8_t
AudioAircraftHost_FixedCamera(void)
{
	// Query the active camera mode only at the original seaplane gate
	return TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_FIXED;
}
//- rouz edit (ChatGPT)
#ifdef GTA_PS2
//+ rouz edit (ChatGPT)
extern "C" void
AudioAircraftHost_CameraBegin(AudioCameraMatrix *matrix, void **attachment, uint8_t *owned)
{
	// Preserve the original PS2 matrix assignment and transfer its cleanup state into C
	CMatrix copy;
	copy = TheCamera.GetMatrix();
	matrix->rx = copy.rx;
	matrix->ry = copy.ry;
	matrix->rz = copy.rz;
	matrix->fx = copy.fx;
	matrix->fy = copy.fy;
	matrix->fz = copy.fz;
	matrix->ux = copy.ux;
	matrix->uy = copy.uy;
	matrix->uz = copy.uz;
	matrix->px = copy.px;
	matrix->py = copy.py;
	matrix->pz = copy.pz;
	*attachment = copy.m_attachment;
	*owned = copy.m_hasRwMatrix;
	copy.m_attachment = nil;
	copy.m_hasRwMatrix = false;
}
extern "C" void
AudioAircraftHost_CameraEnd(void *attachment, uint8_t owned)
{
	// Preserve the original PS2 matrix destructor condition at the generator return
	if (owned && attachment) RwMatrixDestroy((RwMatrix *)attachment);
}
//- rouz edit (ChatGPT)
#endif
//+ rouz edit (ChatGPT)
extern "C" float
AudioPlaneHost_Speed(const CVehicle *vehicle)
{
	// Read the original captured plane speed
	return ((const CPlane *)vehicle)->m_fSpeed;
}
extern "C" float
AudioPlaneHost_Position(const CVehicle *vehicle)
{
	// Read the live path position through the original plane ID
	return PlanePathPosition[((const CPlane *)vehicle)->m_nPlaneId];
}
extern "C" float
AudioPlaneHost_TakeOffPoint(void)
{
	// Read the original live takeoff threshold
	return TakeOffPoint;
}
extern "C" float
AudioPlaneHost_LandingPoint(void)
{
	// Read the original live landing threshold
	return LandingPoint;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" void *
AudioVehicleHost_Handling(CVehicle *vehicle)
{
    // Read the current handling pointer at each original conditional access
    return vehicle->pHandling;
}
extern "C" cTransmission *
AudioVehicleHost_Transmission(void *handling)
{
    // Address the transmission of the selected handling record
    return &((tHandlingData *)handling)->Transmission;
}
extern "C" float
AudioVehicleHost_TrafficSpeed(CVehicle *vehicle)
{
    // Read the original simple-traffic speed field
    return vehicle->AutoPilot.m_fMaxTrafficSpeed;
}
extern "C" void
AudioVehicleHost_VelocityVectors(CVehicle *vehicle, AudioSoundPosition *speed, AudioSoundPosition *direction)
{
    // Resolve the forward reference before copying the current velocity vectors
    const CVector &forward = vehicle->GetForward();
    *speed = AudioSoundPosition_FromVector(vehicle->m_vecMoveSpeed);
    *direction = AudioSoundPosition_FromVector(forward);
}
extern "C" int32_t
AudioVehicleHost_Type(CVehicle *vehicle)
{
    // Read the original vehicle type before routing generators
    return vehicle->m_vehType;
}
extern "C" int32_t
AudioVehicleHost_Appearance(CVehicle *vehicle)
{
    // Query the original appearance method at the car routing gate
    return vehicle->GetVehicleAppearance();
}
extern "C" void
AudioVehicleHost_CarVelocity(CVehicle *vehicle, float velocity)
{
    // Store the final velocity on the original captured automobile
    ((CAutomobile *)vehicle)->m_fVelocityChangeForAudio = velocity;
}
extern "C" void
AudioVehicleHost_BikeVelocity(CVehicle *vehicle, float velocity)
{
    // Store the final velocity on the original captured bike
    ((CBike *)vehicle)->m_fVelocityChangeForAudio = velocity;
}
//- rouz edit (ChatGPT)
#ifdef GTA_TRAIN
//+ rouz edit (ChatGPT)
extern "C" float
AudioTrainHost_Speed(const CVehicle *vehicle)
{
    // Read each original speed expression on the captured train
    return ((const CTrain *)vehicle)->m_fSpeed;
}
extern "C" float
AudioTrainHost_WagonPosition(const CVehicle *vehicle)
{
    // Read the original captured wagon position for the far-layer gate
    return ((const CTrain *)vehicle)->m_fWagonPosition;
}
//- rouz edit (ChatGPT)
#endif
//+ rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" CEntity *
AudioPedOneShotsHost_Entity(CPed *ped)
{
    // Convert the pedestrian to its original entity base
    return ped;
}
extern "C" CWeapon *
AudioPedOneShotsHost_Weapon(CPed *ped)
{
    // Capture the original selected weapon pointer
    return ped->GetWeapon();
}
extern "C" int32_t
AudioPedOneShotsHost_WeaponType(const CWeapon *weapon)
{
    // Read the original captured weapon type
    return weapon->m_eWeaponType;
}
extern "C" uint8_t
AudioPedOneShotsHost_IsPlayer(CPed *ped)
{
    // Query the live pedestrian player predicate
    return ped->IsPlayer();
}
extern "C" CVehicle *
AudioPedOneShotsHost_Vehicle(const CPed *ped)
{
    // Read the live pedestrian vehicle pointer
    return ped->m_pMyVehicle;
}
extern "C" uint8_t
AudioPedOneShotsHost_InAir(const CPed *ped)
{
    // Read the original airborne bit
    return ped->bIsInTheAir;
}
extern "C" uint8_t
AudioPedOneShotsHost_Surface(const CPed *ped)
{
    // Read the original touched surface
    return ped->m_nSurfaceTouched;
}
extern "C" int32_t
AudioPedOneShotsHost_MoveState(const CPed *ped)
{
    // Read the live movement state
    return ped->m_nMoveState;
}
extern "C" uint32_t
AudioPedOneShotsHost_FightMove(const CPed *ped)
{
    // Read the captured fight-move field
    return ped->m_curFightMove;
}
extern "C" int32_t
AudioPedOneShotsHost_State(const CPed *ped)
{
    // Read the original pedestrian state
    return ped->m_nPedState;
}
extern "C" void *
AudioPedOneShotsHost_DamageEntity(const CPed *ped)
{
    // Capture the original damage entity pointer
    return ped->m_pDamageEntity;
}
extern "C" uint8_t
AudioPedOneShotsHost_EntityType(const void *entity)
{
    // Read the damage entity type without changing its pointer
    return ((const CEntity *)entity)->GetType();
}
extern "C" CPed *
AudioPedOneShotsHost_Player(void)
{
    // Query the original player pedestrian at each step event
    return FindPlayerPed();
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)



CVehicle *
AudioEnvironmentHost_Remote(void)
{
    // Capture the focused player's remote vehicle before the player lookup
    return CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle;
}

int32_t
AudioEnvironmentHost_EntityId(const CPlayerPed *player)
{
    // Read each original player audio-entity field access independently
    return player->m_audioEntityId;
}

uint8_t
AudioEnvironmentHost_Entering(CPlayerPed *player)
{
    // Query entering-car state before reading the in-vehicle flag
    return player->EnteringCar();
}

uint8_t
AudioEnvironmentHost_InVehicle(const CPlayerPed *player)
{
    // Read the live in-vehicle flag only after the entering-car test
    return player->bInVehicle;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" uint8_t
AudioVehicleOneShotsHost_DoorType(uint32_t index)
{
    // Read the original audio-table door type at each event's setup point
    return aVehicleSettings[index].m_bDoorType;
}
extern "C" int16_t
AudioVehicleOneShotsHost_HandBrake(void)
{
    // Query the original first pad handbrake value
    return Pads[0].GetHandBrake();
}
extern "C" CPed *
AudioVehicleOneShotsHost_Driver(const CVehicle *vehicle)
{
    // Capture the current driver pointer for the original weapon branch
    return vehicle->pDriver;
}
extern "C" uint8_t
AudioVehicleOneShotsHost_HasWeapon(CPed *ped, uint8_t slot)
{
    // Query the original captured driver's weapon-slot predicate
    return ped->HasWeaponSlot(slot);
}
extern "C" uint8_t
AudioVehicleOneShotsHost_DriveBy(CPed *ped)
{
    // Query the original captured driver's drive-by predicate
    return ped->IsPedDoingDriveByShooting();
}
extern "C" int32_t
AudioVehicleOneShotsHost_Weapon(CPed *ped, uint8_t slot)
{
    // Read the weapon type returned by the original selected slot query
    return ped->GetWeapon(slot).m_eWeaponType;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" void
AudioPedSpeechHost_SetTalking(CPed *ped, uint8_t status)
{
    // Write the original boolean talking permission
    ped->m_canTalk = status;
}
extern "C" uint8_t
AudioPedSpeechHost_CanTalk(const CPed *ped)
{
    // Read talking permission before selecting speech
    return ped->m_canTalk;
}
extern "C" uint8_t
AudioPedSpeechHost_LineClear(const AudioSoundPosition *position)
{
    // Keep both vector arguments and the original visibility flags in the game expression
    return CWorld::GetIsLineOfSightClear(TheCamera.GetPosition(), AudioSoundPosition_ToVector(position), true, false, false, false, false, false);
}
extern "C" uint32_t
AudioPedSpeechHost_MissionTime(void)
{
    // Read each original mission-time expression independently
    return CTheScripts::GetLastMissionPassedTime();
}
extern "C" void
AudioPedSpeechHost_SetMissionTime(uint32_t time)
{
    // Preserve the original reference-returning mission-time assignment
    CTheScripts::GetLastMissionPassedTime() = time;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" uint32_t
AudioPedTalkHost_GetUpTimer(const CPed *ped)
{
    // Read each original get-up timer expression independently
    return ped->m_getUpTimer;
}
extern "C" int16_t
AudioPedTalkHost_Model(const CPed *ped)
{
    // Read the same original signed pedestrian model index
    return ped->GetModelIndex();
}
extern "C" uint32_t *
AudioPedTalkHost_LastComment(CPed *ped)
{
    // Resolve the original writable previous-comment field before phrase selection
    return &ped->m_lastComment;
}
extern "C" int32_t
AudioPedTalkHost_PlayerState(void)
{
    // Query the original live player before reading its state
    return FindPlayerPed()->m_nPedState;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" void
AudioPedPlaybackHost_DebugPosition(PedCommentPosition *position)
{
    // Copy the original player-zero position after its live game query
    const CVector &commentPosition = CWorld::Players[0].m_pPed->GetPosition();
    position->x = commentPosition.x;
    position->y = commentPosition.y;
    position->z = commentPosition.z;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" uint8_t
AudioFiresHost_Ongoing(uint8_t id)
{
    // Read the live ongoing flag at the original short-circuit gate
    return gFireManager.m_aFires[id].m_bIsOngoing;
}
extern "C" uint8_t
AudioFiresHost_AudioSet(uint8_t id)
{
    // Read the audio flag only after the ongoing gate
    return gFireManager.m_aFires[id].m_bAudioSet;
}
extern "C" CEntity *
AudioFiresHost_Entity(uint8_t id)
{
    // Capture the original entity pointer before type and sample queries
    return gFireManager.m_aFires[id].m_pEntity;
}
extern "C" void
AudioFiresHost_Position(uint8_t id, AudioSoundPosition *position)
{
    // Copy the live fire position at the original sound-vector expression
    *position = AudioSoundPosition_FromVector(Fire_VectorToNative(gFireManager.m_aFires[id].m_vecPos));
}
extern "C" uint8_t
AudioFiresHost_Water(uint8_t id)
{
    // Read the live water flag after the burning sound path
    return gFireManager.m_aFires[id].m_bExtinguishedWithWater;
}
extern "C" void
AudioFiresHost_ClearWater(uint8_t id)
{
    // Clear the original water flag before reading its countdown
    gFireManager.m_aFires[id].m_bExtinguishedWithWater = FALSE;
}
extern "C" float
AudioFiresHost_Countdown(uint8_t id)
{
    // Read the countdown after clearing the original water flag
    return gFireManager.m_aFires[id].m_fWaterExtinguishCountdown;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" int32_t
AudioWaterCannonHost_Id(uint32_t index)
{
    // Read the original live cannon identifier at its eligibility gate
    return WaterCannons_aCannons[index].m_nId;
}
extern "C" void
AudioWaterCannonHost_Position(uint32_t index, AudioSoundPosition *position)
{
    // Preserve the original cannon-zero position and live segment-index expression
    const WaterCannonVector *source = &WaterCannons_aCannons[0].m_avecPos[WaterCannons_aCannons[index].m_nCur];
    AudioSoundPosition result = {source->x, source->y, source->z};
    *position = result;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" float
AudioWeatherHost_SoundDistUp(void)
{
    // Read the original camera ceiling distance at the thunder-volume gate
    return TheCamera.SoundDistUp;
}
extern "C" void
AudioWeatherHost_WindModifier(float *x, float *y)
{
    // Query the original camera position and wind modifier after rain processing
    WindModifiers_FindWindModifier(WindModifiers_BackendVector(&TheCamera.GetPosition()), x, y);
}
extern "C" float
AudioWeatherHost_Wind(void)
{
    // Read the live wind value at each original comparison or volume expression
    return Weather_Wind;
}
extern "C" float
AudioWeatherHost_NearestTree(void)
{
    // Read the live tree distance at the original gate or volume expression
    return CObject::fDistToNearestTree;
}
extern "C" void
AudioWeatherHost_ResetNearestTree(void)
{
    // Reset the original tree distance only after submitting the wind sound
    CObject::fDistToNearestTree = 999999.9f;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" uint8_t
AudioWorldLoopsHost_ProjectileInUse(uint8_t index)
{
    // Read the original projectile eligibility flag at each scan step
    return ProjectileInfo_GetProjectileInfo(index)->m_bInUse;
}
extern "C" int32_t
AudioWorldLoopsHost_ProjectileWeapon(uint8_t index)
{
    // Query the original projectile info again at the weapon-selection point
    return ProjectileInfo_GetProjectileInfo(index)->m_eWeaponType;
}
extern "C" void
AudioWorldLoopsHost_ProjectilePosition(uint8_t index, AudioSoundPosition *position)
{
    // Read the live projectile pointer and position after sample-frequency queries
    *position = AudioSoundPosition_FromVector(ProjectileInfo_projectiles[index]->GetPosition());
}
extern "C" int32_t
AudioWorldLoopsHost_EscalatorCount(void)
{
    // Read the live signed escalator count at every loop condition
    // Enter the C controller or read its canonical count at the original call point
    return Escalators_Num;
}
extern "C" uint8_t
AudioWorldLoopsHost_EscalatorActive(int32_t index)
{
    // Query the original escalator activation gate
    // Enter the C controller or read its canonical count at the original call point
    return Escalator_IsActive(index);
}
extern "C" void
AudioWorldLoopsHost_EscalatorPosition(int32_t index, AudioSoundPosition *position)
{
    // Query the escalator again before copying its live midpoint
    // Enter the C controller or read its canonical count at the original call point
    // Copy the C midpoint before publishing the complete original audio position value
    EscalatorVector value=Escalator_GetPosition(index);
    AudioSoundPosition result={value.x,value.y,value.z};*position=result;
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" uint32_t
AudioGarageHydrantHost_Count(void)
{
    // Read the original live garage count at each loop condition
    return CGarages::NumGarages;
}
extern "C" uint8_t
AudioGarageHydrantHost_Type(uint32_t index)
{
    // Read the original live garage type at each eligibility or sample gate
    return CGarages::aGarages[index].m_eGarageType;
}
extern "C" CEntity *
AudioGarageHydrantHost_Door(uint32_t index)
{
    // Capture the original first-door pointer before position and distance queries
    return CGarages::aGarages[index].m_pDoor1;
}
extern "C" uint8_t
AudioGarageHydrantHost_State(uint32_t index)
{
    // Read the original live state without caching subsequent crusher-state queries
    return CGarages::aGarages[index].m_eGarageState;
}
extern "C" void
AudioGarageHydrantHost_Position(void *entity, AudioSoundPosition *position)
{
    // Preserve the original particle-object cast and position query for hydrants
    *position = AudioSoundPosition_FromVector(((CParticleObject *)entity)->GetPosition());
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#ifdef GTA_BRIDGE
extern "C" CEntity *
AudioBridgeHost_Road(void)
{
    // Read the original lift-road pointer at each presence or position query
    return Bridge_pLiftRoad;
}
extern "C" int32_t
AudioBridgeHost_State(void)
{
    // Read the live bridge state separately for each original state-machine gate
    return Bridge_State;
}
extern "C" int32_t
AudioBridgeHost_OldState(void)
{
    // Read the original previous state only after the matching current-state gate
    return Bridge_OldState;
}
extern "C" int32_t
AudioBridgeHost_CommercialPassed(void)
{
    // Preserve the original story-progress gate for the warning generator
    return Stats_CommercialPassed;
}
#endif
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" void *
AudioGameSetupHost_FireManager(void)
{
    // Return the original global fire-manager address used by entity creation
    return &gFireManager;
}
//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
