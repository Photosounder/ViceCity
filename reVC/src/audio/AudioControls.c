//+ rouz edit (ChatGPT)
#include "AudioControls.h"
#include "DMAudio.h"
#include "sampman.h"

#define AUDIO_CONTROL_CLAMP(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))

void
DMAudio_SetMP3BoostVolume(uint8_t volume)
{
	// Preserve the original game audio operation through the C interface
	uint8_t vol = volume;
	if (vol > MAX_VOLUME) vol = MAX_VOLUME;

	SampleManager_SetMP3BoostVolume(&SampleManager, vol);
}

void
DMAudio_SetEffectsMasterVolume(uint8_t volume)
{
	// Preserve the original game audio operation through the C interface
	uint8_t vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	SampleManager_SetEffectsMasterVolume(&SampleManager, vol);
}

void
DMAudio_SetMusicMasterVolume(uint8_t volume)
{
	// Preserve the original game audio operation through the C interface
	uint8_t vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	SampleManager_SetMusicMasterVolume(&SampleManager, vol);
}

void
DMAudio_SetEffectsFadeVol(uint8_t volume)
{
	// Preserve the original game audio operation through the C interface
	uint8_t vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	SampleManager_SetEffectsFadeVolume(&SampleManager, vol);
}

void
DMAudio_SetMusicFadeVol(uint8_t volume)
{
	// Preserve the original game audio operation through the C interface
	uint8_t vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	SampleManager_SetMusicFadeVolume(&SampleManager, vol);
}

uint8_t
AudioControls_GetNum3DProvidersAvailable(uint8_t initialised)
{
	// Preserve the original backend operation and initialization gate
#ifdef EXTERNAL_3D_SOUND
	if (initialised)
		return SampleManager_GetNum3DProvidersAvailable(&SampleManager);
#endif
	return 0;
}

char *
AudioControls_Get3DProviderName(uint8_t initialised, uint8_t id)
{
	// Preserve the original backend operation and initialization gate
#ifndef EXTERNAL_3D_SOUND
	return NULL;
#else
	if (!initialised)
		return NULL;
#ifdef AUDIO_OAL
	id = AUDIO_CONTROL_CLAMP(id, 0, SampleManager_GetNum3DProvidersAvailable(&SampleManager) - 1);
#else
	// We don't want that either since it will crash the game, but skipping for now
	if (id >= SampleManager_GetNum3DProvidersAvailable(&SampleManager))
		return NULL;
#endif
	return SampleManager_Get3DProviderName(&SampleManager, id);
#endif
}

int8_t
AudioControls_GetCurrent3DProviderIndex(uint8_t initialised)
{
	// Preserve the original backend operation and initialization gate
#ifdef EXTERNAL_3D_SOUND
	if (initialised)
		return SampleManager_GetCurrent3DProviderIndex(&SampleManager);
#endif

	return -1;
}

int8_t
AudioControls_AutoDetect3DProviders(uint8_t initialised)
{
	// Preserve the original backend operation and initialization gate
#ifdef EXTERNAL_3D_SOUND
	if (initialised)
		return SampleManager_AutoDetect3DProviders(&SampleManager);
#endif

	return -1;
}

void
AudioControls_SetSpeakerConfig(int32_t conf)
{
	// Preserve the original backend operation and initialization gate
#ifdef EXTERNAL_3D_SOUND
	SampleManager_SetSpeakerConfig(&SampleManager, conf);
#endif
}

uint8_t
AudioControls_IsMP3RadioChannelAvailable(uint8_t initialised)
{
	// Preserve the original backend operation and initialization gate
	if (initialised)
		return SampleManager_IsMP3RadioChannelAvailable(&SampleManager);

	return 0;
}

void
AudioControls_ReleaseDigitalHandle(uint8_t initialised)
{
	// Preserve the original backend operation and initialization gate
	if (initialised)
		SampleManager_ReleaseDigitalHandle(&SampleManager);
}

void
AudioControls_ReacquireDigitalHandle(uint8_t initialised)
{
	// Preserve the original backend operation and initialization gate
	if (initialised)
		SampleManager_ReacquireDigitalHandle(&SampleManager);
}

uint8_t
AudioControls_CheckForAnAudioFileOnCD(void)
{
	// Preserve the original backend operation and initialization gate
	return SampleManager_CheckForAnAudioFileOnCD(&SampleManager);
}

char
AudioControls_GetCDAudioDriveLetter(uint8_t initialised)
{
	// Preserve the original backend operation and initialization gate
	if (initialised)
		return SampleManager_GetCDAudioDriveLetter(&SampleManager);
	return '\0';
}

//- rouz edit (ChatGPT)
