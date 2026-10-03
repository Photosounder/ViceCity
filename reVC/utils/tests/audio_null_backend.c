#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sampman.h"
#include "oal/AudioStreamHost.h"
static unsigned assertion_calls;
#ifdef __cplusplus
extern "C"
#endif
void re3_assert(const char *expr, const char *file, unsigned line, const char *func) {
    // Record the production assertion boundary without opening an error dialog
    (void)expr; (void)file; (void)line; (void)func; assertion_calls++;
}
int main(void) {
    // Call every null backend API through its public C-compatible header
#ifdef EXTERNAL_3D_SOUND
SampleManager_SetSpeakerConfig(&SampleManager,0);
#endif
#ifdef EXTERNAL_3D_SOUND
printf("SampleManager_GetMaximumSupportedChannels %lld\n",(long long)SampleManager_GetMaximumSupportedChannels(&SampleManager));
#endif
#ifdef EXTERNAL_3D_SOUND
printf("SampleManager_GetNum3DProvidersAvailable %lld\n",(long long)SampleManager_GetNum3DProvidersAvailable(&SampleManager));
#endif
#ifdef EXTERNAL_3D_SOUND
SampleManager_SetNum3DProvidersAvailable(&SampleManager,0);
#endif
#ifdef EXTERNAL_3D_SOUND
SampleManager_Set3DProviderName(&SampleManager,0,NULL);
#endif
#ifdef EXTERNAL_3D_SOUND
printf("SampleManager_GetCurrent3DProviderIndex %lld\n",(long long)SampleManager_GetCurrent3DProviderIndex(&SampleManager));
#endif
#ifdef EXTERNAL_3D_SOUND
printf("SampleManager_SetCurrent3DProvider %lld\n",(long long)SampleManager_SetCurrent3DProvider(&SampleManager,0));
#endif
printf("SampleManager_IsMP3RadioChannelAvailable %lld\n",(long long)SampleManager_IsMP3RadioChannelAvailable(&SampleManager));
SampleManager_ReleaseDigitalHandle(&SampleManager);
SampleManager_ReacquireDigitalHandle(&SampleManager);
printf("SampleManager_Initialise %lld\n",(long long)SampleManager_Initialise(&SampleManager));
SampleManager_Terminate(&SampleManager);
printf("SampleManager_CheckForAnAudioFileOnCD %lld\n",(long long)SampleManager_CheckForAnAudioFileOnCD(&SampleManager));
printf("SampleManager_GetCDAudioDriveLetter %lld\n",(long long)SampleManager_GetCDAudioDriveLetter(&SampleManager));
SampleManager_UpdateEffectsVolume(&SampleManager);
SampleManager_SetEffectsMasterVolume(&SampleManager,0);
SampleManager_SetMusicMasterVolume(&SampleManager,0);
SampleManager_SetMP3BoostVolume(&SampleManager,0);
SampleManager_SetEffectsFadeVolume(&SampleManager,0);
SampleManager_SetMusicFadeVolume(&SampleManager,0);
SampleManager_SetMonoMode(&SampleManager,0);
printf("SampleManager_LoadSampleBank %lld\n",(long long)SampleManager_LoadSampleBank(&SampleManager,0));
SampleManager_UnloadSampleBank(&SampleManager,0);
printf("SampleManager_IsSampleBankLoaded %lld\n",(long long)SampleManager_IsSampleBankLoaded(&SampleManager,0));
printf("SampleManager_IsMissionAudioLoaded %lld\n",(long long)SampleManager_IsMissionAudioLoaded(&SampleManager,0,TOTAL_AUDIO_SAMPLES-1));
printf("SampleManager_LoadMissionAudio %lld\n",(long long)SampleManager_LoadMissionAudio(&SampleManager,0,TOTAL_AUDIO_SAMPLES-1));
printf("SampleManager_IsPedCommentLoaded %lld\n",(long long)SampleManager_IsPedCommentLoaded(&SampleManager,TOTAL_AUDIO_SAMPLES-1));
printf("SampleManager__GetPedCommentSlot %lld\n",(long long)SampleManager__GetPedCommentSlot(&SampleManager,TOTAL_AUDIO_SAMPLES-1));
printf("SampleManager_LoadPedComment %lld\n",(long long)SampleManager_LoadPedComment(&SampleManager,TOTAL_AUDIO_SAMPLES-1));
printf("SampleManager_GetBankContainingSound %lld\n",(long long)SampleManager_GetBankContainingSound(&SampleManager,0));
printf("SampleManager_GetSampleBaseFrequency %lld\n",(long long)SampleManager_GetSampleBaseFrequency(&SampleManager,TOTAL_AUDIO_SAMPLES-1));
printf("SampleManager_GetSampleLoopStartOffset %lld\n",(long long)SampleManager_GetSampleLoopStartOffset(&SampleManager,TOTAL_AUDIO_SAMPLES-1));
printf("SampleManager_GetSampleLoopEndOffset %lld\n",(long long)SampleManager_GetSampleLoopEndOffset(&SampleManager,TOTAL_AUDIO_SAMPLES-1));
printf("SampleManager_GetSampleLength %lld\n",(long long)SampleManager_GetSampleLength(&SampleManager,TOTAL_AUDIO_SAMPLES-1));
printf("SampleManager_UpdateReverb %lld\n",(long long)SampleManager_UpdateReverb(&SampleManager));
SampleManager_SetChannelReverbFlag(&SampleManager,0,0);
printf("SampleManager_InitialiseChannel %lld\n",(long long)SampleManager_InitialiseChannel(&SampleManager,0,TOTAL_AUDIO_SAMPLES-1,0));
#ifdef EXTERNAL_3D_SOUND
SampleManager_SetChannelEmittingVolume(&SampleManager,0,0);
#endif
#ifdef EXTERNAL_3D_SOUND
SampleManager_SetChannel3DPosition(&SampleManager,0,0,0,0);
#endif
#ifdef EXTERNAL_3D_SOUND
SampleManager_SetChannel3DDistances(&SampleManager,0,0,0);
#endif
SampleManager_SetChannelVolume(&SampleManager,MAXCHANNELS,0);
SampleManager_SetChannelPan(&SampleManager,MAXCHANNELS,0);
SampleManager_SetChannelFrequency(&SampleManager,0,0);
SampleManager_SetChannelLoopPoints(&SampleManager,0,0,-1);
SampleManager_SetChannelLoopCount(&SampleManager,0,0);
printf("SampleManager_GetChannelUsedFlag %lld\n",(long long)SampleManager_GetChannelUsedFlag(&SampleManager,0));
SampleManager_StartChannel(&SampleManager,0);
SampleManager_StopChannel(&SampleManager,0);
SampleManager_PreloadStreamedFile(&SampleManager,0,MAX_STREAMS-1);
SampleManager_PauseStream(&SampleManager,0,MAX_STREAMS-1);
SampleManager_StartPreloadedStreamedFile(&SampleManager,MAX_STREAMS-1);
printf("SampleManager_StartStreamedFile %lld\n",(long long)SampleManager_StartStreamedFile(&SampleManager,0,0,MAX_STREAMS-1));
SampleManager_StopStreamedFile(&SampleManager,MAX_STREAMS-1);
printf("SampleManager_GetStreamedFilePosition %lld\n",(long long)SampleManager_GetStreamedFilePosition(&SampleManager,MAX_STREAMS-1));
SampleManager_SetStreamedVolumeAndPan(&SampleManager,0,0,0,MAX_STREAMS-1);
printf("SampleManager_GetStreamedFileLength %lld\n",(long long)SampleManager_GetStreamedFileLength(&SampleManager,MAX_STREAMS-1));
printf("SampleManager_IsStreamPlaying %lld\n",(long long)SampleManager_IsStreamPlaying(&SampleManager,MAX_STREAMS-1));
printf("SampleManager_InitialiseSampleBanks %lld\n",(long long)SampleManager_InitialiseSampleBanks(&SampleManager));
SampleManager_SetStreamedFileLoopFlag(&SampleManager,0,0);
#ifdef EXTERNAL_3D_SOUND
printf("SampleManager_AutoDetect3DProviders %lld\n",(long long)SampleManager_AutoDetect3DProviders(&SampleManager));
#endif
    #ifdef EXTERNAL_3D_SOUND
    // Retain the null provider's stable name
    assert(strcmp(SampleManager_Get3DProviderName(&SampleManager,0),"NULL") == 0);
#endif
    // Preserve no-op volume setters and static state across repeated initialization
    assert(SampleManager_GetMusicVolume(&SampleManager) == 0);
    for(unsigned i=0; i<1000; i++) {
        SampleManager_SetMusicMasterVolume(&SampleManager,127);
        assert(SampleManager_GetMusicVolume(&SampleManager) == 0);
        assert(SampleManager_Initialise(&SampleManager));
        SampleManager_Terminate(&SampleManager);
    }
    printf("assertion calls %u layout %zu\n",assertion_calls,sizeof(SampleManager));
    return 0;
}
