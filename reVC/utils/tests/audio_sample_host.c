#include <assert.h>
#include <string.h>
#include "AudioSampleHost.h"
#include "AudioStreamHost.h"
#include "audio_enums.h"
#include "eax-util.h"
void AudioSampleHost_TestConfigure(unsigned value);
#ifdef AUDIO_MSS
void AudioSampleHost_TestCDCallbacks(void);
#endif
int main(void) {
    // Check every production game-state callback across the actual C and C++ link boundary
    for(unsigned value=0; value<256; value++) {
        AudioSampleHost_TestConfigure(value);
        assert(AudioSample_GetMusicMode() == (int)(value%3));
        assert(AudioStream_IsCutscene() == (value%3 == MUSICMODE_CUTSCENE));
        assert(AudioSample_GetCurrentTrack() == (int)(value+100));
        assert(AudioSample_GetRadioInCar() == (int)(value%8));
        assert(AudioSample_IsMusicInitialised() == (value%2 != 0));
        assert(AudioSample_IsAudioInitialised() == (value%3 == 0));
        assert(AudioSample_IsCodePaused() == (value%7 == 0));
        assert(AudioSample_CheckForMusicInterruptions() == (value%5 == 0));
        assert(AudioSample_GetFrameCounter() == UINT32_MAX-value);
        for(unsigned i=0; i<5; i++) assert(AudioSample_GetRandomValue(i) == -(int32_t)(value+i));
        for(unsigned i=0; i<MAX_REFLECTIONS; i++) assert(AudioSample_GetReflectionDistances()[i] == (float)(value+i));
    }
    // Verify the C backend can link to the production EAX C++ implementation and preset data
    EAXLISTENERPROPERTIES result;
    assert(EAX3ListenerInterpolate(&EAX30_ORIGINAL_PRESETS[0], &EAX30_ORIGINAL_PRESETS[1], 0.0f, &result, false));
    assert(memcmp(&result,&EAX30_ORIGINAL_PRESETS[0],sizeof(result)) == 0);
    assert(EAX3ListenerInterpolate(&EAX30_ORIGINAL_PRESETS[0], &EAX30_ORIGINAL_PRESETS[1], 1.0f, &result, false));
    assert(memcmp(&result,&EAX30_ORIGINAL_PRESETS[1],sizeof(result)) == 0);
#ifdef AUDIO_MSS
    // Check provider selection, CD quit decisions and volume/service ordering
    AudioSampleHost_TestCDCallbacks();
#endif
    return 0;
}
