//+ rouz edit (ChatGPT)
#include "Weather_backend.h"
#include "Timer.h"
#include "Clock.h"
#include "General.h"
#include "ZoneCull.h"
#include "SpecialFX_control.h"
#include "RenderBuffer.h"
#include "CameraModes.h"
#include "DMAudio.h"
#include <stddef.h>
#include <math.h>
#define WEATHER_ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))
#define TWOPI (((float)3.14159265358979323846)*2)
static WeatherVector Weather_MakeVector(float x,float y,float z)
{
    // Materialize the original scalar coordinates as a plain C value packet
    WeatherVector value={x,y,z};return value;
}
static WeatherVector Weather_Add(WeatherVector a,WeatherVector b)
{
    // Preserve the original component-wise addition order
    return Weather_MakeVector(a.x+b.x,a.y+b.y,a.z+b.z);
}
static WeatherVector Weather_Sub(WeatherVector a,WeatherVector b)
{
    // Preserve the original component-wise subtraction order
    return Weather_MakeVector(a.x-b.x,a.y-b.y,a.z-b.z);
}
static WeatherVector Weather_Scale(float scale,WeatherVector value)
{
    // Retain the original scalar-left multiplication for all three components
    return Weather_MakeVector(scale*value.x,scale*value.y,scale*value.z);
}
static WeatherVector Weather_ScaleRight(WeatherVector value,float scale)
{
    // Retain the original vector-left multiplication for all three components
    return Weather_MakeVector(value.x*scale,value.y*scale,value.z*scale);
}
static float Weather_Magnitude(WeatherVector value)
{
    // Match the original ordered three-component squared magnitude and square root
    return sqrtf(value.x*value.x+value.y*value.y+value.z*value.z);
}
static void Weather_Normalise(WeatherVector *value)
{
    // Preserve the original native vector normalization including its fallback
    float sq=value->x*value->x+value->y*value->y+value->z*value->z;
    if(sq>0.0f){
        // Scale all original components by the reciprocal square root
        float inverse=1.0f/sqrtf(sq);value->x*=inverse;value->y*=inverse;value->z*=inverse;
    }else{
        // Leave the other components unchanged in the original zero-direction branch
        value->x=1.0f;
    }
}
int32_t Weather_SoundHandle = -1;

int32_t Weather_WeatherTypeInList;
int16_t Weather_OldWeatherType;
int16_t Weather_NewWeatherType;
int16_t Weather_ForcedWeatherType;

bool Weather_LightningFlash;
bool Weather_LightningBurst;
uint32_t Weather_LightningStart;
uint32_t Weather_LightningFlashLastChange;
uint32_t Weather_WhenToPlayLightningSound;
uint32_t Weather_LightningDuration;
float Weather_StreamAfterRainTimer; // rouz edit (ChatGPT)

float Weather_ExtraSunnyness;
float Weather_Foggyness;
float Weather_CloudCoverage;
float Weather_Wind;
float Weather_Rain;
float Weather_InterpolationValue;
float Weather_WetRoads;
float Weather_Rainbow;
float Weather_SunGlare;
float Weather_WindClipped;
float Weather_TrafficLightBrightness;

bool Weather_bScriptsForceRain;
static int startStreamAfterRain;

WeatherRainStreak Weather_Streaks[NUM_RAIN_STREAKS];

int16_t Weather_Types[] = {
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_CLOUDY,
	WEATHER_RAINY, WEATHER_RAINY, WEATHER_RAINY, WEATHER_RAINY,
	WEATHER_CLOUDY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY
};

int16_t Weather_HurricaneTypes[] = {
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_CLOUDY,
	WEATHER_HURRICANE, WEATHER_HURRICANE, WEATHER_CLOUDY, WEATHER_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_CLOUDY, WEATHER_HURRICANE, WEATHER_HURRICANE, WEATHER_HURRICANE,
	WEATHER_CLOUDY, WEATHER_SUNNY, WEATHER_SUNNY, WEATHER_SUNNY,
	WEATHER_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY,
	WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY, WEATHER_EXTRA_SUNNY
};

const float Weather_Windyness[] = {
	0.25f,// WEATHER_SUNNY
	0.7f, // WEATHER_CLOUDY
	1.0f, // WEATHER_RAINY
	0.0f, // WEATHER_FOGGY
	0.0f, // WEATHER_EXTRA_SUNNY
	2.0f, // WEATHER_HURRICANE
	0.0f
};

#define MIN_TIME_BETWEEN_LIGHTNING_FLASH_CHANGES (50)

#define RAIN_CHANGE_SPEED (0.003f)
#define STREAM_AFTER_RAIN_TIME (800.0f * (1000.0f / 30.0f))
#define HEAT_HAZE_ADD_INTERVAL (8.0f * (1000.0f / 30.0f))
#define BEASTIE_ADD_INTERVAL (10.0f * (1000.0f / 30.0f))

#define DROPLETS_LEFT_OFFSET (10.0f)
#define DROPLETS_RIGHT_OFFSET (10.0f)
#define DROPLETS_TOP_OFFSET (10.0f)
#define DROPLETS_BOTTOM_OFFSET (10.0f)

#define STREAK_U (10.0f)
#define STREAK_V (18.0f)
#define LARGE_STREAK_COEFFICIENT (1.23f)
#define STREAK_MIN_DISTANCE (8.0f)
#define STREAK_MAX_DISTANCE (16.0f)

#define SPLASH_CHECK_RADIUS (7.0f)
#define SPLASH_OFFSET_RADIUS (2.0f)

#define STREAK_LIFETIME (4.0f)
#define STREAK_INTEROLATION_TIME (0.3f)

#define RAIN_COLOUR_R (200)
#define RAIN_COLOUR_G (200)
#define RAIN_COLOUR_B (256)
#define RAIN_ALPHA (255)

static float
WeatherTimeStep(void)
{
    // Preserve the complete original weather sequence and every live state access
	return Timer_GetTimeStepInSeconds() * 30.0f;
}

void Weather_Init(void)
{
    // Preserve the complete original weather sequence and every live state access
	Weather_NewWeatherType = WEATHER_EXTRA_SUNNY;
	Weather_bScriptsForceRain = false;
	Weather_OldWeatherType = WEATHER_EXTRA_SUNNY;
	Weather_InterpolationValue = 0.0f;
	Weather_WhenToPlayLightningSound = 0;
	Weather_WeatherTypeInList = 0;
	Weather_ForcedWeatherType = WEATHER_RANDOM;
	Weather_SoundHandle = DMAudio_CreateEntity(AUDIOTYPE_WEATHER, (void*)1);
	if (Weather_SoundHandle >= 0)
		DMAudio_SetEntityStatus(Weather_SoundHandle, true);
}

void Weather_Update(void)
{
    // Preserve the complete original weather sequence and every live state access
		static float lightningUpdateCounter;

	lightningUpdateCounter += WeatherTimeStep();
	int32_t lightningUpdates = (int32_t)(lightningUpdateCounter);
	lightningUpdateCounter -= lightningUpdates;
	
	if(!Weather_BackendPlayback()){
		float fNewInterpolation = (Clock_GetMinutes() + Clock_GetSeconds()/60.0f)/60.0f;
		if (fNewInterpolation < Weather_InterpolationValue) {
			// new hour
			Weather_OldWeatherType = Weather_NewWeatherType;
			if (Weather_ForcedWeatherType >= 0)
				Weather_NewWeatherType = Weather_ForcedWeatherType;
			else {
				Weather_WeatherTypeInList = (Weather_WeatherTypeInList + 1) % WEATHER_ARRAY_SIZE(Weather_Types);
				Weather_NewWeatherType = Weather_BackendNoHurricanes() ? Weather_Types[Weather_WeatherTypeInList] : Weather_HurricaneTypes[Weather_WeatherTypeInList];
			}
		}
		Weather_InterpolationValue = fNewInterpolation;
	}

#ifndef FINAL
	if (Weather_BackendShockDown()) {
		Weather_NewWeatherType = (Weather_NewWeatherType + 1) % WEATHER_TOTAL;
		Weather_OldWeatherType = Weather_NewWeatherType;
	}
#endif

	// Lightning
	if (Weather_NewWeatherType != WEATHER_RAINY || Weather_OldWeatherType != WEATHER_RAINY) {
		Weather_LightningFlash = false;
		Weather_LightningBurst = false;
	}
		else {
		for (int32_t i = 0; i < lightningUpdates; i++) {
			if (Weather_LightningBurst) {
				if ((General_GetRandomNumber() & 255) >= 32) {
					// 0.875 probability
					if ((*Timer_GetTimeInMilliseconds()) - Weather_LightningFlashLastChange > MIN_TIME_BETWEEN_LIGHTNING_FLASH_CHANGES) {
						bool bOldLightningFlash = Weather_LightningFlash;
						Weather_LightningFlash = General_GetRandomTrueFalse();
						if (Weather_LightningFlash != bOldLightningFlash)
							Weather_LightningFlashLastChange = (*Timer_GetTimeInMilliseconds());
					}
				}
				else {
					// 0.125 probability
					Weather_LightningBurst = false;
					Weather_LightningDuration = Min(((*Timer_GetTimeInMilliseconds()) - Weather_LightningStart) * 30 / 1000, 20);
					Weather_LightningFlash = false;
					Weather_WhenToPlayLightningSound = (*Timer_GetTimeInMilliseconds()) + 150 * (20 - Weather_LightningDuration);
				}
			}
			else {
				if (General_GetRandomNumber() >= 200) {
					// lower probability on PC due to randomness bug
					Weather_LightningFlash = false;
				}
				else {
					Weather_LightningBurst = true;
					Weather_LightningStart = (*Timer_GetTimeInMilliseconds());
					Weather_LightningFlashLastChange = (*Timer_GetTimeInMilliseconds());
					Weather_LightningFlash = true;
				}
							}
		}
	}
	if (Weather_WhenToPlayLightningSound && (*Timer_GetTimeInMilliseconds()) > Weather_WhenToPlayLightningSound) {
		DMAudio_PlayOneShot(Weather_SoundHandle, SOUND_LIGHTNING, Weather_LightningDuration);
		Weather_BackendShake(40 * Weather_LightningDuration + 100, 2 * Weather_LightningDuration + 80);
		Weather_WhenToPlayLightningSound = 0;
	}

	// Wet roads
	if (Weather_OldWeatherType == WEATHER_RAINY || Weather_OldWeatherType == WEATHER_HURRICANE) {
		if (Weather_NewWeatherType == WEATHER_RAINY || Weather_NewWeatherType == WEATHER_HURRICANE)
			Weather_WetRoads = 1.0f;
		else
			Weather_WetRoads = 1.0f - Weather_InterpolationValue;
	}
	else {
		if (Weather_NewWeatherType == WEATHER_RAINY || Weather_NewWeatherType == WEATHER_HURRICANE)
			Weather_WetRoads = Weather_InterpolationValue;
		else
			Weather_WetRoads = 0.0f;
	}

	// Weather_Rain
	float fNewRain;
	if (Weather_NewWeatherType == WEATHER_RAINY || Weather_NewWeatherType == WEATHER_HURRICANE) {
		// if raining for >1 hour, values: 0, 0.33, switching every ~16.5s
		fNewRain = (((uint16_t)(*Timer_GetTimeInMilliseconds()) >> 14) & 1) * 0.33f;
		if (Weather_OldWeatherType != WEATHER_RAINY && Weather_OldWeatherType != WEATHER_HURRICANE) {
			if (Weather_InterpolationValue < 0.4f)
				// if rain has just started (<24 minutes), always 0.5
				fNewRain = 0.5f;
			else
				// if rain is ongoing for >24 minutes, values: 0.25, 0.5, switching every ~16.5s
				fNewRain = 0.25f + (((uint16_t)(*Timer_GetTimeInMilliseconds()) >> 14) & 1) * 0.25f;
		}
		fNewRain = Max(fNewRain, 0.5f);
	}
	else
		fNewRain = 0.0f;
	Weather_Rain = fNewRain;

	// Clouds
	if (Weather_OldWeatherType != WEATHER_SUNNY && Weather_OldWeatherType != WEATHER_EXTRA_SUNNY)
		Weather_CloudCoverage = 1.0f - Weather_InterpolationValue;
	else
		Weather_CloudCoverage = 0.0f;
	if (Weather_NewWeatherType != WEATHER_SUNNY && Weather_OldWeatherType != WEATHER_EXTRA_SUNNY)
		Weather_CloudCoverage += Weather_InterpolationValue;
	
	// Fog
	if (Weather_OldWeatherType == WEATHER_FOGGY)
		Weather_Foggyness = 1.0f - Weather_InterpolationValue;
	else
		Weather_Foggyness = 0.0f;
	if (Weather_NewWeatherType == WEATHER_FOGGY)
		Weather_Foggyness += Weather_InterpolationValue;

	// Extra Sunnyness
	if (Weather_OldWeatherType == WEATHER_EXTRA_SUNNY)
		Weather_ExtraSunnyness = 1.0f - Weather_InterpolationValue;
	else
		Weather_ExtraSunnyness = 0.0f;
	if (Weather_NewWeatherType == WEATHER_EXTRA_SUNNY)
		Weather_ExtraSunnyness += Weather_InterpolationValue;

	// Weather_Rainbow
	if (Weather_OldWeatherType == WEATHER_CLOUDY && (Weather_NewWeatherType == WEATHER_SUNNY || Weather_NewWeatherType == WEATHER_EXTRA_SUNNY) &&
		Weather_InterpolationValue < 0.5f && Clock_GetHours() > 6 && Clock_GetHours() < 21)
		Weather_Rainbow = 1.0f - 4.0f * fabsf(Weather_InterpolationValue - 0.25f) / 4.0f;
	else
		Weather_Rainbow = 0.0f;

	// Sun Glare
	if (Weather_OldWeatherType == WEATHER_EXTRA_SUNNY)
		Weather_SunGlare = 1.0f - Weather_InterpolationValue;
	else
		Weather_SunGlare = 0.0f;
	if (Weather_NewWeatherType == WEATHER_EXTRA_SUNNY)
		Weather_SunGlare += Weather_InterpolationValue;

	if (Weather_SunGlare > 0.0f) {
		Weather_SunGlare *= Min(1.0f, 7.0 * Weather_BackendSunZ());
		Weather_SunGlare = Clamp(Weather_SunGlare, 0.0f, 1.0f);
		if (!SpecialFX_bSnapShotActive)
			Weather_SunGlare *= (1.0f - (General_GetRandomNumber()&0x1F)*0.007f);
	}

	Weather_Wind = Weather_InterpolationValue * Weather_Windyness[Weather_NewWeatherType] + (1.0f - Weather_InterpolationValue) * Weather_Windyness[Weather_OldWeatherType];
	Weather_WindClipped = Min(1.0f, Weather_Wind);

	if (Clock_GetHours() > 20)
		Weather_TrafficLightBrightness = 1.0f;
	else if (Clock_GetHours() > 19)
		Weather_TrafficLightBrightness = Clock_GetMinutes() / 60.0f;
	else if (Clock_GetHours() > 6)
		Weather_TrafficLightBrightness = 0.0f;
	else if (Clock_GetHours() > 5)
		Weather_TrafficLightBrightness = 1.0f - Clock_GetMinutes() / 60.0f;
	else
		Weather_TrafficLightBrightness = 1.0f;
	Weather_TrafficLightBrightness = Max(Weather_WetRoads, Weather_TrafficLightBrightness);
	Weather_TrafficLightBrightness = Max(Weather_Foggyness, Weather_TrafficLightBrightness);
	Weather_TrafficLightBrightness = Max(Weather_Rain, Weather_TrafficLightBrightness);

	Weather_AddRain();

		static uint32_t nextHeatHazeAddTime;
	if ((Weather_NewWeatherType == WEATHER_SUNNY || Weather_NewWeatherType == WEATHER_EXTRA_SUNNY) &&
		!Weather_BackendInterior() && !Weather_BackendCutscene() && (*Timer_GetTimeInMilliseconds()) >= nextHeatHazeAddTime) {
		nextHeatHazeAddTime = (*Timer_GetTimeInMilliseconds()) + HEAT_HAZE_ADD_INTERVAL;
		#ifdef FIX_BUGS
		if (Weather_BackendPlayerExists() && (!Weather_BackendPlayerInAir() || Weather_BackendPlayerInAir() && Weather_BackendPlayerZ() < 7.5f &&
			Clock_GetHours() > 6 && Clock_GetHours() < 18))
#else
		if (!Weather_BackendPlayerInAir() || Weather_BackendPlayerInAir() && Weather_BackendPlayerZ() < 7.5f &&
			Clock_GetHours() > 6 && Clock_GetHours() < 18)
#endif
			Weather_AddHeatHaze();
	}

	if ((Weather_NewWeatherType == WEATHER_SUNNY || Weather_NewWeatherType == WEATHER_EXTRA_SUNNY) && !Weather_BackendInterior() && !Weather_BackendCutscene())
		Weather_AddBeastie();
}

void Weather_AddHeatHaze(void)
{
    // Preserve the complete original weather sequence and every live state access
	if(Weather_BackendCameraMode() == CAMERA_MODE_TOPDOWN ||
	   Weather_BackendCameraMode() == CAMERA_MODE_TOP_DOWN_PED)
		return;
	WeatherVector pos;
	pos.x = Weather_BackendScreenWidth()*0.5f;
	if(Weather_BackendCameraForwardFirstPerson())
		pos.y = General_GetRandomNumberInRangeFloat(Weather_BackendScreenHeight()*0.25f, Weather_BackendScreenHeight()*0.9f);
	else
		pos.y = General_GetRandomNumberInRangeFloat(Weather_BackendScreenHeight()*0.4f, Weather_BackendScreenHeight()*0.9f);
	pos.z = 100.0f;
	Weather_BackendParticleSimple(PARTICLE_HEATHAZE_IN_DIST, pos, Weather_MakeVector(0.0f, 0.0f, 0.0f));
}

void Weather_AddBeastie(void)
{
    // Preserve the complete original weather sequence and every live state access
		static uint32_t nextBeastieAddTime;
	if(Weather_BackendPlayerVehicle() || (*Timer_GetTimeInMilliseconds()) < nextBeastieAddTime)
		return;

	nextBeastieAddTime = (*Timer_GetTimeInMilliseconds()) + BEASTIE_ADD_INTERVAL;

	if((General_GetRandomNumber()&5) == 0)
		return;
	
	WeatherVector pos = Weather_BackendCameraPosition();
	float dist = General_GetRandomNumberInRangeFloat(90.0f, 60.0f);
	int angle = General_GetRandomNumber() % WEATHER_TRIG_TABLE_SIZE;
	float c = Weather_BackendCosValue(angle);
	float s = Weather_BackendSinValue(angle);
	pos.x += dist*(c - s);
	pos.y += dist*(c + s);
	pos.z += General_GetRandomNumberInRangeFloat(7.5f, 30.0f);
	Weather_BackendParticleSimple(PARTICLE_BEASTIE, pos, Weather_MakeVector(0.0f, 0.0f, 0.0f));
}

void Weather_ForceWeather(int16_t weather)
{
    // Preserve the complete original weather sequence and every live state access
	Weather_ForcedWeatherType = weather;
}

void Weather_ForceWeatherNow(int16_t weather)
{
    // Preserve the complete original weather sequence and every live state access
	Weather_OldWeatherType = weather;
	Weather_NewWeatherType = weather;
	Weather_ForcedWeatherType = weather;
}

void Weather_ReleaseWeather(void)
{
    // Preserve the complete original weather sequence and every live state access
	Weather_ForcedWeatherType = -1;
}

void Weather_AddSplashesDuringHurricane(void)
{
    // Preserve the complete original weather sequence and every live state access
	WeatherColor colour = { 255, 255, 255, 32 };
	WeatherVector pos = Weather_BackendTarget() ? Weather_BackendEntityPosition(Weather_BackendTarget()) : Weather_BackendCameraPosition();
	bool foundGround;
	float groundZ = Weather_BackendGroundZ(pos.x, pos.y, pos.z, &foundGround) + 0.1f;
	if(!foundGround)
		groundZ = pos.z + 0.5f;
	for(int i = 0; i < 20; i++){
		float dist = (General_GetRandomNumber()&0xFF)/255.0f +
			General_GetRandomNumberInRangeFloat(-10.0f, 30.0f);
		float angle;
		uint8_t rnd = General_GetRandomNumber();
		if(rnd&1)
			angle = (General_GetRandomNumber()&0x7F)/128.0f * TWOPI;
		else
			angle = Weather_BackendCameraOrientation() + (rnd-128)/160.0f;
		pos.x = Weather_BackendCameraPosition().x + dist*sinf(angle);
		pos.y = Weather_BackendCameraPosition().y + dist*cosf(angle);
		pos.z = groundZ;
		if(foundGround)
			Weather_BackendParticleColor(PARTICLE_GROUND_STEAM, pos, Weather_MakeVector(-0.002f, -0.002f, 0.015f), 0.0f, colour);
	}
}

void Weather_AddStreamAfterRain(void)
{
    // Preserve the complete original weather sequence and every live state access
	if(Clock_GetHours() > 6 && Clock_GetHours() < 18){
		WeatherColor colour = { 255, 255, 255, 24 };
		WeatherVector pos = Weather_BackendTarget() ? Weather_BackendEntityPosition(Weather_BackendTarget()) : Weather_BackendCameraPosition();
		bool foundGround;
		float groundZ = Weather_BackendGroundZ(pos.x, pos.y, pos.z, &foundGround) + 0.2f;
		if(!foundGround)
			groundZ = pos.z + 0.75f;
		for(int i = 0; i < 20; i++){
			float dist = (General_GetRandomNumber()&0xFF)/255.0f +
				General_GetRandomNumberInRangeFloat(-10.0f, 30.0f);
			float angle;
			uint8_t rnd = General_GetRandomNumber();
			if(rnd&1)
				angle = (General_GetRandomNumber()&0x7F)/128.0f * TWOPI;
			else
				angle = Weather_BackendCameraOrientation() + (rnd-128)/160.0f;
			pos.x = Weather_BackendCameraPosition().x + dist*sinf(angle);
			pos.y = Weather_BackendCameraPosition().y + dist*cosf(angle);
			pos.z = groundZ;
			Weather_BackendParticleColor(PARTICLE_GROUND_STEAM, pos, Weather_MakeVector(0.0f, 0.0f, 0.015f), 0.0f, colour);
		}
	}else{
		startStreamAfterRain = 0;
		Weather_StreamAfterRainTimer = STREAM_AFTER_RAIN_TIME; // rouz edit (ChatGPT)
	}
}

void Weather_AddRain(void)
{
    // Preserve the complete original weather sequence and every live state access
	static float particleUpdateCounter; // rouz edit (ChatGPT)

	if (CullZones_CamNoRain() || CullZones_PlayerNoRain())
		return;
	if (Weather_BackendCameraLRBFirstPerson()) {
		CVehicle* pVehicle = Weather_BackendPlayerVehicle();
		if (pVehicle && Weather_BackendVehicleRoof(pVehicle)) {
			Weather_BackendRemoveParticles(PARTICLE_RAINDROP_2D);
			return;
		}
	}

		particleUpdateCounter += WeatherTimeStep();
	int32_t particleUpdates = (int32_t)(particleUpdateCounter);
	particleUpdateCounter -= particleUpdates;

	if(Weather_Rain > 0.0){
		startStreamAfterRain = 1;
		Weather_StreamAfterRainTimer = STREAM_AFTER_RAIN_TIME;
	}else if(startStreamAfterRain){
		if(Weather_StreamAfterRainTimer > 0){
			for(int i = 0; i < particleUpdates; i++)
				Weather_AddStreamAfterRain();
			Weather_StreamAfterRainTimer -= Timer_GetTimeStepInSeconds() * 1000.0f;
		}else{
			startStreamAfterRain = 0;
			Weather_StreamAfterRainTimer = STREAM_AFTER_RAIN_TIME;
		}
	}

	if (particleUpdates == 0)
		return;

	if (Weather_Wind > 1.1f)
		for(int i = 0; i < particleUpdates; i++)
			Weather_AddSplashesDuringHurricane();
	
	if (Weather_Rain <= 0.1f)
		return;
	static WeatherColor colour;
	int numDrops = 5.0f * Weather_Rain;
	int numSplashes = 2.0f * Weather_Rain;
	WeatherVector pos, dir;
		for(int update = 0; update < particleUpdates; update++){
		for(int i = 0; i < numDrops; i++){
			pos.x = General_GetRandomNumberInRangeInt(0, (int)Weather_BackendScreenWidth());
			pos.y = General_GetRandomNumberInRangeInt(0, (int)Weather_BackendScreenHeight()/5);
			pos.z = 0.0f;
			dir.x = 0.0f;
			dir.y = General_GetRandomNumberInRangeFloat(30.0f, 40.0f);
			dir.z = 0.0f;
			Weather_BackendParticleDrop(PARTICLE_RAINDROP_2D, pos, dir, General_GetRandomNumberInRangeFloat(0.1f, 0.75f), 0, 0, (int)Weather_Rain&3, 0);

			pos.x = General_GetRandomNumberInRangeInt(0, (int)Weather_BackendScreenWidth());
			pos.y = General_GetRandomNumberInRangeInt((int)Weather_BackendScreenHeight()/5, (int)Weather_BackendScreenHeight()/2);
			pos.z = 0.0f;
			dir.x = 0.0f;
			dir.y = General_GetRandomNumberInRangeFloat(30.0f, 40.0f);
			dir.z = 0.0f;
			Weather_BackendParticleDrop(PARTICLE_RAINDROP_2D, pos, dir, General_GetRandomNumberInRangeFloat(0.1f, 0.75f), 0, 0, (int)Weather_Rain&3, 0);

			pos.x = General_GetRandomNumberInRangeInt(0, (int)Weather_BackendScreenWidth());
			pos.y = 0.0f;
			pos.z = 0.0f;
			dir.x = 0.0f;
			dir.y = General_GetRandomNumberInRangeFloat(30.0f, 40.0f);
			dir.z = 0.0f;
			Weather_BackendParticleDrop(PARTICLE_RAINDROP_2D, pos, dir, General_GetRandomNumberInRangeFloat(0.1f, 0.75f), 0, 0, (int)Weather_Rain&3, 0);

			float dist = General_GetRandomNumberInRangeFloat(0.0f, Max(10.0f*Weather_Rain, 40.0f)/2.0f);
			float angle;
			uint8_t rnd = General_GetRandomNumber();
			if(rnd&1)
				angle = (General_GetRandomNumber()&0x7F)/128.0f * TWOPI;
			else
				angle = Weather_BackendCameraOrientation() + (rnd-128)/160.0f;
			pos.x = Weather_BackendCameraPosition().x + dist*sinf(angle);
			pos.y = Weather_BackendCameraPosition().y + dist*cosf(angle);
			pos.z = 0.0f;
			float hitHeight;
			if(Weather_BackendVerticalLine(Weather_Add(pos,Weather_MakeVector(0.0f,0.0f,40.0f)),&hitHeight)){
				pos.z = hitHeight;
				for(int j = 0; j < numSplashes+15; j++){
					WeatherVector pos2 = pos;
					pos2.x += General_GetRandomNumberInRangeFloat(-15.0f, 15.0f);
					pos2.y += General_GetRandomNumberInRangeFloat(-15.0f, 15.0f);
					if(General_GetRandomNumber() & 1)
						Weather_BackendParticleColor(PARTICLE_RAIN_SPLASH, pos2, Weather_MakeVector(0.0f, 0.0f, 0.0f), 0.0f, colour);
					else
						Weather_BackendParticleColor(PARTICLE_RAIN_SPLASHUP, pos2, Weather_MakeVector(0.0f, 0.0f, 0.0f), 0.0f, colour);
				}
							}
		}
	}
}

void Weather_RenderRainStreaks(void)
{
    // Preserve the complete original weather sequence and every live state access
	static float streakGenerationCounter; // rouz edit (ChatGPT)
	// Keep auxiliary reflection renders from advancing rain streak state
		bool renderOnlyExistingStreaks = false;
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	renderOnlyExistingStreaks = Weather_BackendEnvMap();
#endif
	
	if (Timer_GetIsCodePaused())
		return;
	int base_intensity = (64.0f - Weather_BackendFogReduction()) / 64.0f * (int)(255 * Weather_Rain);
	if (base_intensity == 0)
		return;
	if (Weather_BackendCameraSpeed() > 1.75f)
		return;
	// Use the active reflection camera basis when replaying existing streaks
		WeatherVector streakCameraPosition = Weather_BackendCameraPosition();
	WeatherVector streakCameraRight = Weather_BackendCameraRight();
	WeatherVector streakCameraUp = Weather_BackendCameraUp();
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(renderOnlyExistingStreaks && Weather_BackendEnvCamera() != NULL){
		void *effectCamera = Weather_BackendEnvCamera();
		void *effectFrame = Weather_BackendCameraFrame(effectCamera);
		if(effectFrame && Weather_BackendFrameMatrix(effectFrame)){
			void *effectMatrix = Weather_BackendFrameMatrix(effectFrame);
			streakCameraPosition = Weather_BackendMatrixPosition(effectMatrix);
			streakCameraRight = Weather_BackendMatrixRight(effectMatrix);
			streakCameraUp = Weather_BackendMatrixUp(effectMatrix);
			Weather_Normalise(&streakCameraRight);
			Weather_Normalise(&streakCameraUp);
		}
	}
#endif
		// Advance streak generation only while rendering the primary camera
		int32_t streakGenerationUpdates = 0;
	if(!renderOnlyExistingStreaks){
		streakGenerationCounter += WeatherTimeStep();
		streakGenerationUpdates = (int32_t)(streakGenerationCounter);
		streakGenerationCounter -= streakGenerationUpdates;
	}
		TempBufferIndicesStored = 0;
	TempBufferVerticesStored = 0;
	for (int i = 0; i < NUM_RAIN_STREAKS; i++) {
		if (Weather_Streaks[i].timer) {
			float secondsElapsed = ((*Timer_GetTimeInMilliseconds()) - Weather_Streaks[i].timer) / 1024.0f;
			// Retire expired streaks only in the primary camera pass
						if (secondsElapsed > STREAK_LIFETIME){
				if(!renderOnlyExistingStreaks)
					Weather_Streaks[i].timer = 0;
			}
						else{
				int intensity;
				if (secondsElapsed < STREAK_INTEROLATION_TIME)
					intensity = base_intensity * 0.25f * secondsElapsed / STREAK_INTEROLATION_TIME;
				else if (secondsElapsed > (STREAK_LIFETIME - STREAK_INTEROLATION_TIME))
					intensity = (STREAK_LIFETIME - secondsElapsed) * 0.25f * base_intensity / STREAK_INTEROLATION_TIME;
				else
					intensity = base_intensity * 0.25f;
				WeatherVector dir = Weather_Streaks[i].direction;
				Weather_Normalise(&dir);
				WeatherVector pos = Weather_Add(Weather_Streaks[i].position,Weather_Scale(secondsElapsed,Weather_Streaks[i].direction));
				// Keep distance fade and billboard orientation aligned with this render camera
								Weather_RenderOneRainStreak(pos, dir, intensity, false, Weather_Magnitude(Weather_Sub(pos,streakCameraPosition)), streakCameraRight, streakCameraUp);
				#ifndef FIX_BUGS // remove useless code
				// Keep the legacy random-number side effect out of reflection rendering
								if(!renderOnlyExistingStreaks && secondsElapsed > 1.0f && secondsElapsed < STREAK_LIFETIME - 1.0f) {
					General_GetRandomNumber(), General_GetRandomNumber();
				}
				#endif
			}
		}
		// Generate new streaks only during the primary camera pass
				else if(!renderOnlyExistingStreaks) for (int32_t update = 0; update < streakGenerationUpdates; update++) {
			if ((General_GetRandomNumber() & 0xF00) != 0)
				continue;
						// 1/16 probability
			Weather_Streaks[i].direction = Weather_MakeVector(0.0f, 0.0f, -12.0f);
			Weather_Streaks[i].position = Weather_Add(Weather_Add(Weather_Scale(6.0f,Weather_BackendCameraForward()),Weather_BackendCameraPosition()),Weather_MakeVector(-1.8f*Weather_Streaks[i].direction.x,-1.8f*Weather_Streaks[i].direction.y,8.0f));
			if (!Weather_BackendCutscene()) {
				Weather_Streaks[i].position.x += 2.0f * Weather_BackendPlayerSpeedX() * 60.0f;
				Weather_Streaks[i].position.y += 2.0f * Weather_BackendPlayerSpeedY() * 60.0f;
			}
			else
				Weather_Streaks[i].position=Weather_Add(Weather_Streaks[i].position,Weather_ScaleRight(Weather_Sub(Weather_BackendCameraPosition(),Weather_BackendCameraPrevious()),20.0f));
			Weather_Streaks[i].position.x += ((General_GetRandomNumber() & 255) - 128) * 0.04f;
			Weather_Streaks[i].position.y += ((General_GetRandomNumber() & 255) - 128) * 0.04f;
			Weather_Streaks[i].timer = (*Timer_GetTimeInMilliseconds());
			break; // rouz edit (ChatGPT)
		}
	}
	if (TempBufferIndicesStored){
		Weather_BackendRenderState(WEATHER_rwRENDERSTATEZWRITEENABLE,(uintptr_t)false);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATEZTESTENABLE,(uintptr_t)true);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATEFOGENABLE,(uintptr_t)false);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATEFOGTYPE,(uintptr_t)WEATHER_rwFOGTYPELINEAR);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATESRCBLEND,(uintptr_t)WEATHER_rwBLENDONE);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATEDESTBLEND,(uintptr_t)WEATHER_rwBLENDONE);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATEVERTEXALPHAENABLE,(uintptr_t)true);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATETEXTURERASTER,(uintptr_t)Weather_BackendRaster());
		if (Weather_BackendTransform(TempBufferVerticesStored))
		{
			Weather_BackendRenderIndexed(TempBufferIndicesStored);
			Weather_BackendEnd();
		}
		Weather_BackendRenderState(WEATHER_rwRENDERSTATEZWRITEENABLE,(uintptr_t)true);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATEZTESTENABLE,(uintptr_t)true);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATESRCBLEND,(uintptr_t)WEATHER_rwBLENDSRCALPHA);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATEDESTBLEND,(uintptr_t)WEATHER_rwBLENDINVSRCALPHA);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATEFOGENABLE,(uintptr_t)false);
		Weather_BackendRenderState(WEATHER_rwRENDERSTATEVERTEXALPHAENABLE,(uintptr_t)false);
	}
	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;
}

void Weather_RenderOneRainStreak(WeatherVector pos, WeatherVector unused, int intensity, bool scale, float distance,
	WeatherVector cameraRight, WeatherVector cameraUp)
{
    // Preserve the complete original weather sequence and every live state access
	static float RandomTex;
	static float RandomTexX;
	static float RandomTexY;
	TempBufferRenderIndexList[TempBufferIndicesStored + 0] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[TempBufferIndicesStored + 1] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[TempBufferIndicesStored + 2] = TempBufferVerticesStored + 1;
	TempBufferRenderIndexList[TempBufferIndicesStored + 3] = TempBufferVerticesStored + 0;
	TempBufferRenderIndexList[TempBufferIndicesStored + 4] = TempBufferVerticesStored + 3;
	TempBufferRenderIndexList[TempBufferIndicesStored + 5] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[TempBufferIndicesStored + 6] = TempBufferVerticesStored + 1;
	TempBufferRenderIndexList[TempBufferIndicesStored + 7] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[TempBufferIndicesStored + 8] = TempBufferVerticesStored + 4;
	TempBufferRenderIndexList[TempBufferIndicesStored + 9] = TempBufferVerticesStored + 2;
	TempBufferRenderIndexList[TempBufferIndicesStored + 10] = TempBufferVerticesStored + 3;
	TempBufferRenderIndexList[TempBufferIndicesStored + 11] = TempBufferVerticesStored + 4;
		// Build the rain streak around the camera basis selected by the current render pass
	Weather_BackendVertexRGBA(TempBufferVerticesStored + 0, 0, 0, 0, 0);
	Weather_BackendVertexPos(TempBufferVerticesStored + 0, pos.x + 11.0f * cameraUp.x, pos.y + 11.0f * cameraUp.y, pos.z + 11.0f * cameraUp.z);
	Weather_BackendVertexRGBA(TempBufferVerticesStored + 1, 0, 0, 0, 0);
	Weather_BackendVertexPos(TempBufferVerticesStored + 1, pos.x - 9.0f * cameraRight.x, pos.y - 9.0f * cameraRight.y, pos.z - 9.0f * cameraRight.z);
	Weather_BackendVertexRGBA(TempBufferVerticesStored + 2, RAIN_COLOUR_R * intensity / 256, RAIN_COLOUR_G * intensity / 256, RAIN_COLOUR_B * intensity / 256, RAIN_ALPHA);
	Weather_BackendVertexPos(TempBufferVerticesStored + 2, pos.x, pos.y, pos.z);
	Weather_BackendVertexRGBA(TempBufferVerticesStored + 3, 0, 0, 0, 0);
	Weather_BackendVertexPos(TempBufferVerticesStored + 3, pos.x + 9.0f * cameraRight.x, pos.y + 9.0f * cameraRight.y, pos.z + 9.0f * cameraRight.z);
	Weather_BackendVertexRGBA(TempBufferVerticesStored + 4, 0, 0, 0, 0); 
	Weather_BackendVertexPos(TempBufferVerticesStored + 4, pos.x - 11.0f * cameraUp.x, pos.y - 11.0f * cameraUp.y, pos.z - 11.0f * cameraUp.z);
		float u = STREAK_U;
	float v = STREAK_V;
	if (scale) {
		u *= LARGE_STREAK_COEFFICIENT;
		v *= LARGE_STREAK_COEFFICIENT;
	}
	float distance_coefficient;
	if (distance < STREAK_MIN_DISTANCE)
		distance_coefficient = 1.0f;
	else if (distance > STREAK_MAX_DISTANCE)
		distance_coefficient = 0.5f;
	else
		distance_coefficient = 1.0f - 0.5f * (distance - STREAK_MIN_DISTANCE) / (STREAK_MAX_DISTANCE - STREAK_MIN_DISTANCE);
	u *= distance_coefficient;
	v *= distance_coefficient;
	if (!Timer_GetIsPaused()) {
		RandomTex = 0.0f;
		RandomTexX = 0.0f;
		RandomTexY = 0.0f;
	}
	Weather_BackendVertexU(TempBufferVerticesStored + 0, 0.5f * u - RandomTex + RandomTexX);
	Weather_BackendVertexV(TempBufferVerticesStored + 0, -v * 0.5f + RandomTexY);
	Weather_BackendVertexU(TempBufferVerticesStored + 1, RandomTexX);
	Weather_BackendVertexV(TempBufferVerticesStored + 1, RandomTexY);
	Weather_BackendVertexU(TempBufferVerticesStored + 2, 0.5f * u + RandomTexX);
	Weather_BackendVertexV(TempBufferVerticesStored + 2, RandomTexY);
	Weather_BackendVertexU(TempBufferVerticesStored + 3, u + RandomTexX);
	Weather_BackendVertexV(TempBufferVerticesStored + 3, RandomTexY);
	Weather_BackendVertexU(TempBufferVerticesStored + 4, 0.5f * u + RandomTex + RandomTexX);
	Weather_BackendVertexV(TempBufferVerticesStored + 4, 0.5f * v + RandomTexY); // rouz edit (ChatGPT)
	TempBufferIndicesStored += 12;
	TempBufferVerticesStored += 5;
}

#ifdef SECUROM
void Weather_ForceHurricaneWeather(void)
{
    // Preserve the complete original weather sequence and every live state access
	for (int i = 0; i < WEATHER_ARRAY_SIZE(Weather_HurricaneTypes); i++)
	{
		Weather_Types[i] = WEATHER_HURRICANE;
		Weather_HurricaneTypes[i] = WEATHER_HURRICANE;
	}

	Weather_OldWeatherType = WEATHER_HURRICANE;
	Weather_NewWeatherType = WEATHER_HURRICANE;
	Weather_ForcedWeatherType = WEATHER_HURRICANE;
}
#endif

//- rouz edit (ChatGPT)
