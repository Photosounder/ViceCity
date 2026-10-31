//+ rouz edit (ChatGPT)
#include "MusicManager.h"
#include "MusicAmbience.h"
#include "AudioManager.h"
#include "AudioGeometry.h"
#include "AudioSampleConfig.h"
#include "../core/GameAreas.h"
#include "../renderer/WeatherTypes.h"
#include <math.h>

#define MUSIC_AMBIENCE_MIN(a,b) ((a) < (b) ? (a) : (b))
#define MUSIC_AMBIENCE_SQUARE(x) ((x) * (x))

AudioSoundPosition MusicAmbience_RiotPosition = {300.7f, -322.0f, 12.0f};

static float
MusicAmbience_CameraHeight(void)
{
    // Read the camera coordinates through the existing C geometry boundary
    AudioSoundPosition camera;
    AudioGeometryHost_CameraPosition(&camera);
    return camera.z;
}

static float
MusicAmbience_RiotDistanceSquared(void)
{
    // Subtract each riot coordinate before preserving the original sum of float squares
    AudioSoundPosition camera;
    float x, y, z;
    AudioGeometryHost_CameraPosition(&camera);
    x = camera.x - MusicAmbience_RiotPosition.x;
    y = camera.y - MusicAmbience_RiotPosition.y;
    z = camera.z - MusicAmbience_RiotPosition.z;
    return x*x + y*y + z*z;
}

float
MusicAmbience_GetHeightScale(void)
{
	// Preserve the original ambience reads, early returns and shared volume state
	if (MusicAmbience_CameraHeight() > 20.0f) {
		if (MusicAmbience_CameraHeight() < 50.0f)
			return 1.0f - (MusicAmbience_CameraHeight() - 20.0f) / 30.0f;
		return 0.0f;
	}
	return 1.0f;
}

void
MusicAmbience_SetUpCorrectAmbienceTrack(cMusicManager *manager)
{
	// Preserve the original ambience reads, early returns and shared volume state
	switch (MusicAmbienceHost_Area())
	{
	case AREA_MAIN_MAP:
	case AREA_EVERYWHERE:
		if (MusicAmbienceHost_RiotIntensity() != 0 && (MusicAmbience_RiotDistanceSquared() < MUSIC_AMBIENCE_SQUARE(65.0f)))
			manager->m_nFrontendTrack = STREAMED_SOUND_LAW4RIOT_AMBIENT;
		else if (MusicAmbienceHost_DistanceToWater() <= 90.0f) {
			if (MusicAmbienceHost_AtBeach()) {
				if (MusicAmbienceHost_OldWeather() != WEATHER_HURRICANE && MusicAmbienceHost_NewWeather() != WEATHER_HURRICANE || AudioWeatherHost_Wind() <= 1.0f)
					manager->m_nFrontendTrack = STREAMED_SOUND_BEACH_AMBIENT;
				else
					manager->m_nFrontendTrack = STREAMED_SOUND_HAVANA_BEACH_AMBIENT;
			}
			else if (MusicAmbienceHost_OldWeather() != WEATHER_HURRICANE && MusicAmbienceHost_NewWeather() != WEATHER_HURRICANE || AudioWeatherHost_Wind() <= 1.0f)
				manager->m_nFrontendTrack = STREAMED_SOUND_WATER_AMBIENT;
			else
				manager->m_nFrontendTrack = STREAMED_SOUND_HAVANA_WATER_AMBIENT;
		}
		else if (MusicAmbienceHost_OldWeather() != WEATHER_HURRICANE && MusicAmbienceHost_NewWeather() != WEATHER_HURRICANE || AudioWeatherHost_Wind() <= 1.0f)
			manager->m_nFrontendTrack = STREAMED_SOUND_CITY_AMBIENT;
		else
			manager->m_nFrontendTrack = STREAMED_SOUND_HAVANA_CITY_AMBIENT;
		break;
	case AREA_HOTEL:
		manager->m_nFrontendTrack = STREAMED_SOUND_HOTEL_AMBIENT;
		break;
	case AREA_MALL:
		manager->m_nFrontendTrack = STREAMED_SOUND_MALL_AMBIENT;
		break;
	case AREA_STRIP_CLUB:
		manager->m_nFrontendTrack = STREAMED_SOUND_STRIPCLUB_AMBIENT;
		break;
	case AREA_DIRT:
	case AREA_BLOOD:
	case AREA_OVALRING:
		manager->m_nFrontendTrack = STREAMED_SOUND_DIRTRING_AMBIENT;
		break;
	case AREA_MALIBU_CLUB:
		manager->m_nFrontendTrack = STREAMED_SOUND_MALIBU_AMBIENT;
		break;
	case AREA_MANSION:
	case AREA_BANK:
	case AREA_LAWYERS:
	case AREA_COFFEE_SHOP:
	case AREA_CONCERT_HALL:
	case AREA_STUDIO:
	case AREA_RIFLE_RANGE:
	case AREA_BIKER_BAR:
	case AREA_POLICE_STATION:
		manager->m_nFrontendTrack = STREAMED_SOUND_AMBSIL_AMBIENT;
		break;
	}
}

void
MusicAmbience_ComputeAmbienceVol(cMusicManager *manager, uint8_t reset, uint8_t *outVolume)
{
	// Preserve the original ambience reads, early returns and shared volume state
	static float fVol = 0.0f;

	float fHeightScale = MusicAmbience_GetHeightScale();

	if (MusicAmbienceHost_RiotIntensity() > 0) {
		float distToRiotSq = MusicAmbience_RiotDistanceSquared();
		if (distToRiotSq < MUSIC_AMBIENCE_SQUARE(100.0f)) {
			if (distToRiotSq >= MUSIC_AMBIENCE_SQUARE(65.0f))
				(*outVolume) = (sqrtf(distToRiotSq) - 65.0f) / 35.0f * (127.0f * fHeightScale);
			else if (distToRiotSq >= MUSIC_AMBIENCE_SQUARE(20.0f))
				(*outVolume) = (MusicAmbienceHost_RiotIntensity() * (1.0f - (sqrtf(distToRiotSq) - 20.0f) / 45.0f) * (127.0f * fHeightScale)) / MAX_VOLUME;
			else
				(*outVolume) = (MusicAmbienceHost_RiotIntensity() * (127.0f * fHeightScale)) / MAX_VOLUME;
			return;
		}
	}

	if (reset)
		fVol = 0.0f;
	else if (fVol < 60.0f) {
		if ((manager->m_nPlayingTrack >= STREAMED_SOUND_HAVANA_CITY_AMBIENT) && (manager->m_nPlayingTrack <= STREAMED_SOUND_HAVANA_BEACH_AMBIENT))
			fVol += 20.0f;
		else
			fVol += 1.0f;
		fVol = MUSIC_AMBIENCE_MIN(fVol, 60.0f);
	}

	if ((manager->m_nPlayingTrack >= STREAMED_SOUND_MALL_AMBIENT) && (manager->m_nPlayingTrack <= STREAMED_SOUND_AMBSIL_AMBIENT)) {
		(*outVolume) = fVol;
		return;
	}

	if (MusicAmbienceHost_OldWeather() == WEATHER_HURRICANE || MusicAmbienceHost_NewWeather() == WEATHER_HURRICANE) {
		if (AudioWeatherHost_Wind() > 1.0f) {
			(*outVolume) = (AudioWeatherHost_Wind() - 1.0f) * fVol;
			return;
		}
		fVol = (1.0f - AudioWeatherHost_Wind()) * fVol;
	}
	
	if (MusicAmbienceHost_DistanceToWater() > 140.0f) {
		(*outVolume) = fVol;
		return;
	}

	if (MusicAmbienceHost_DistanceToWater() > 90.0f) {
		(*outVolume) = ((MusicAmbienceHost_DistanceToWater() - 90.0f) / 50.0f * fVol * fHeightScale);
		return;
	}

	if (MusicAmbienceHost_DistanceToWater() > 40.0f) {
		(*outVolume) = fVol;
		return;
	}

	(*outVolume) = (90.0f - fHeightScale) / 50.0f * fVol;
}

//- rouz edit (ChatGPT)
