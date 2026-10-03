//+ rouz edit (ChatGPT)
#include "common.h"

#include "AudioManager.h"
#include "AudioSoundGame.h"
#include "audio_enums.h"

#include "AudioScriptObject.h"
#include "Pools.h" // rouz edit (ChatGPT)
#include "MusicManager.h"
#include "Timer.h"
#include "DMAudio.h"
#include "sampman.h"
#include "Camera.h"
#include "World.h"
#include "ZoneCull.h"

cAudioManager AudioManager;

#define SPEED_OF_SOUND 343.f
#define TIME_SPENT 40

cAudioManager::cAudioManager()
{
	// Initialize live owner state at the original C++ startup callback
	AudioManager_InitState(this);
}

cAudioManager::~cAudioManager()
{
	if (m_bIsInitialised)
		AudioLifecycle_Terminate(this);
	// Preserve the former script-object manager destructor reset
	AudioScriptObjectManager_Reset(&m_sAudioScriptObjectManager);
}



















#ifdef GTA_PC










#ifdef AUDIO_REFLECTIONS
#endif




#endif // GTA_PC










#ifdef AUDIO_REFLECTIONS

#endif // AUDIO_REFLECTIONS




#ifdef GTA_PS2
void
cAudioManager::LoadBankIfNecessary(uint8 bank)
{
	if(!SampleManager_IsSampleBankLoaded(&SampleManager, bank))
		SampleManager_LoadSampleBank(&SampleManager, bank);
}
#endif

#ifdef EXTERNAL_3D_SOUND

#endif
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#ifdef AUDIO_REFLECTIONS
uint8_t
AudioRequestsHost_InRoom(void)
{
    // Read the existing camera room flag at the original sound eligibility check
    return CCullZones::InRoomForAudio();
}
#ifndef USE_TIME_SCALE_FOR_AUDIO
uint8_t
AudioRequestsHost_SlowMotion(void)
{
    // Read slow motion at each original reflection frequency and delay decision
    return CTimer::GetIsSlowMotionActive();
}
#endif
#endif
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#ifdef AUDIO_REFLECTIONS
uint8_t
AudioReflectionsHost_Line(const AudioSoundPosition *start, const AudioSoundPosition *end, AudioSoundPosition *hit)
{
    // Preserve the original line query flags and copy only a successful collision position
    CColPoint point;
    CEntity *entity;
    if (!CWorld::ProcessLineOfSight(AudioSoundPosition_ToVector(start), AudioSoundPosition_ToVector(end), point, entity, true, false, false, true, false, true, true))
        return 0;
    *hit = AudioSoundPosition_FromVector(point.point);
    return 1;
}

uint8_t
AudioReflectionsHost_Vertical(const AudioSoundPosition *start, float endZ, float *hitZ)
{
    // Preserve the original vertical query flags and read only the successful height
    CColPoint point;
    CEntity *entity;
    if (!CWorld::ProcessVerticalLine(AudioSoundPosition_ToVector(start), endZ, point, entity, true, false, false, false, true, false, nil))
        return 0;
    *hitZ = point.point.z;
    return 1;
}
#endif
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
void
AudioReloadHost_ResetMusic(void)
{
    // Reset music at the original reload point before clearing the player speech flag
    MusicManager.ResetMusicAfterReload();
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
void
AudioLifecycleHost_PreInitialise(cAudioManager *manager)
{
	// Invoke the existing game callback at the original lifecycle point
	manager->PreInitialiseGameSpecificSetup();
}

void
AudioLifecycleHost_PostInitialise(cAudioManager *manager)
{
	// Invoke the existing game callback at the original lifecycle point
	manager->PostInitialiseGameSpecificSetup();
}

void
AudioLifecycleHost_PreTerminate(cAudioManager *manager)
{
	// Invoke the existing game callback at the original lifecycle point
	manager->PreTerminateGameSpecificShutdown();
}

void
AudioLifecycleHost_PostTerminate(cAudioManager *manager)
{
	// Invoke the existing game callback at the original lifecycle point
	manager->PostTerminateGameSpecificShutdown();
}

void
AudioLifecycleHost_MusicInitialise(void)
{
	// Invoke the existing music callback at the original lifecycle point
	MusicManager.Initialise();
}

void
AudioLifecycleHost_MusicTerminate(void)
{
	// Invoke the existing music callback at the original lifecycle point
	MusicManager.Terminate();
}

//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
void
AudioServiceHost_ResetMusicTimers(uint32_t time)
{
    // Read the music timer argument at the original deferred reset point
    MusicManager.ResetTimers(time);
}

uint8_t
AudioServiceHost_UserPaused(void)
{
    // Query the existing user pause flag after retaining the previous pause state
    return CTimer::GetIsUserPaused();
}


void
AudioServiceHost_Music(void)
{
    // Invoke music after the sound effects service callback completes
    MusicManager.Service();
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)






//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
float
AudioReleaseHost_TimeStep(void)
{
    // Read the original fixed time step at each fading expression
    return CTimer::GetTimeStepFix();
}

void
AudioReleaseHost_Position(void *entity, AudioSoundPosition *position)
{
    // Copy the current base entity position into the C sound record
    *position = AudioSoundPosition_FromVector(((CEntity *)entity)->GetPosition());
}

uint8_t
AudioReleaseHost_LineClear(const AudioSoundPosition *position)
{
    // Retain the original camera ray and world collision category flags
    return CWorld::GetIsLineOfSightClear(TheCamera.GetPosition(), AudioSoundPosition_ToVector(position), true, false, false, false, false, false);
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
uint8_t
AudioActiveHost_CameraSwitched(void)
{
    // Read the camera switch flag at the original doppler calculation point
    return TheCamera.Get_Just_Switched_Status();
}
//- rouz edit (ChatGPT)
