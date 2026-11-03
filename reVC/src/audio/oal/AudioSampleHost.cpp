//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#ifdef _WIN32
#include <windows.h>
#endif
#include "common.h"
#if defined(AUDIO_OAL) || defined(AUDIO_MSS)
#include "AudioManager.h"
#include "AudioProvider.h"
#include "MusicManager.h"
#include "Timer.h"
#ifdef AUDIO_MSS
#include "Frontend.h"
#include "DMAudio.h"
#endif
#include "crossplatform.h"
#include "AudioSampleHost.h"
#include "AudioStreamHost.h"

#ifdef AUDIO_MSS
extern "C" bool AudioSample_WaitForUserCD(void)
{
    // Run the existing CD prompt and report the resulting quit decision
    FrontEndMenuManager.WaitForUserCD();
    return FrontEndMenuManager.m_bQuitGameNoCD;
}
extern "C" void AudioSample_ApplyCDVolume(bool available)
{
    // Restore menu volume preferences or silence audio before the next service pass
    DMAudio_SetMusicMasterVolume(available ? FrontEndMenuManager.m_PrefsMusicVolume : 0);
    DMAudio_SetEffectsMasterVolume(available ? FrontEndMenuManager.m_PrefsSfxVolume : 0);
    DMAudio_Service();
}
#endif

#ifndef _WIN32
#endif
#endif

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
