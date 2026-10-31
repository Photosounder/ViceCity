//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "WeatherTypes.h"
typedef struct WeatherVector {float x,y,z;} WeatherVector;
typedef struct WeatherColor {uint8_t red,green,blue,alpha;} WeatherColor;
enum {NUM_RAIN_STREAKS=35};
typedef struct WeatherRainStreak {WeatherVector position,direction;uint32_t timer;} WeatherRainStreak;
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t Weather_SoundHandle;
extern int32_t Weather_WeatherTypeInList;
extern int16_t Weather_OldWeatherType;
extern int16_t Weather_NewWeatherType;
extern int16_t Weather_ForcedWeatherType;
extern bool Weather_LightningFlash;
extern bool Weather_LightningBurst;
extern uint32_t Weather_LightningStart;
extern uint32_t Weather_LightningFlashLastChange;
extern uint32_t Weather_WhenToPlayLightningSound;
extern uint32_t Weather_LightningDuration;
extern float Weather_StreamAfterRainTimer;
extern float Weather_ExtraSunnyness;
extern float Weather_Foggyness;
extern float Weather_CloudCoverage;
extern float Weather_Wind;
extern float Weather_Rain;
extern float Weather_InterpolationValue;
extern float Weather_WetRoads;
extern float Weather_Rainbow;
extern float Weather_SunGlare;
extern float Weather_WindClipped;
extern float Weather_TrafficLightBrightness;
extern bool Weather_bScriptsForceRain;
extern WeatherRainStreak Weather_Streaks[NUM_RAIN_STREAKS];
extern int16_t Weather_Types[64],Weather_HurricaneTypes[64];
extern const float Weather_Windyness[7];
void Weather_Init(void);
void Weather_Update(void);
void Weather_AddHeatHaze(void);
void Weather_AddBeastie(void);
void Weather_ForceWeather(int16_t weather);
void Weather_ForceWeatherNow(int16_t weather);
void Weather_ReleaseWeather(void);
void Weather_AddSplashesDuringHurricane(void);
void Weather_AddStreamAfterRain(void);
void Weather_AddRain(void);
void Weather_RenderRainStreaks(void);
void Weather_RenderOneRainStreak(WeatherVector pos,WeatherVector unused,int32_t intensity,bool scale,float distance,WeatherVector right,WeatherVector up);
#ifdef SECUROM
void Weather_ForceHurricaneWeather(void);
#endif
#ifdef __cplusplus
}
#ifdef LIBRW
namespace rw {struct Texture;}
typedef rw::Texture WeatherTexture;
#else
typedef struct RwTexture WeatherTexture;
#endif
extern WeatherTexture *gpRainDropTex;
#endif
//- rouz edit (ChatGPT)
