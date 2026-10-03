"""Check the entire production stream implementation as C with a game-boundary shim."""
from pathlib import Path
import sys
import re
root = Path(__file__).resolve().parents[2]
source = '#include "' + (root / 'src/audio/oal/stream.c').as_posix() + '"\n'
shim = ''
runtime = """
int main(void)
{
    // Exercise the actual C initialization and destruction with caller-owned arrays
    ALuint sources[2] = {1, 2};
    ALuint buffers[NUM_STREAMBUFFERS] = {0};
    for(unsigned cycle = 0; cycle < 1000; cycle++) {
        CStream stream;
        CStream_Init(&stream, sources, buffers);
        assert(stream.m_pAlSources == sources && stream.m_alBuffers == buffers);
        assert(!stream.m_bPaused && !stream.m_bActive && !stream.m_bReset);
        assert(stream.m_pBuffer == NULL && stream.m_pSoundFile == NULL);
        assert(stream.m_nVolume == 0 && stream.m_nPan == 0);
        assert(stream.m_nPosBeforeReset == 0 && stream.m_nLoopCount == 1);
#ifdef MULTITHREADED_AUDIO
        assert(!stream.m_bIExist && !stream.m_bDoSeek && stream.m_SeekPos == 0);
        assert(AudioBufferQueue_IsEmpty(&stream.m_fillBuffers));
        assert(AudioBufferQueue_IsEmpty(&stream.m_queueBuffers));
        AudioMutex_Lock(&stream.m_mutex);
        AudioMutex_Unlock(&stream.m_mutex);
        AudioMutex_Lock(&stream.m_queueBuffersMutex);
        AudioMutex_Unlock(&stream.m_queueBuffersMutex);
#endif
        CStream_Destroy(&stream);
    }
    puts("Pure C production stream initialization and destruction passed");
    return 0;
}
""" if '--runtime' in sys.argv else ''
mocks = ''
if runtime:
    # Abort if initialization or destruction unexpectedly touches the audio device
    al_header = (root / 'vendor/openal-soft/include/AL/al.h').read_text()
    for name in ['alBufferData', 'alGetSourcei', 'alIsBuffer', 'alSource3f',
                 'alSourcef', 'alSourcePause', 'alSourcePlay',
                 'alSourceQueueBuffers', 'alSourceStop', 'alSourceUnqueueBuffers']:
        declaration = re.search(r'AL_API ([^;\n]*\b' + name + r'\([^;]*\));', al_header).group(1)
        mocks += declaration + '\n{\n    // Fail if a lifecycle check reaches an audio device operation\n    abort();\n}\n'
    mocks += """
#undef malloc
#undef free
#undef realloc
void *cita_win_malloc(size_t size, const char *file, const char *func, int line) {
    // Fail if initialization unexpectedly allocates owning storage
    abort();
}
void cita_win_free(void *memory, const char *file, const char *func, int line) {
    // Fail if destruction unexpectedly frees caller-owned arrays
    abort();
}
void *cita_win_realloc(void *memory, size_t size, const char *file, const char *func, int line) {
    // Fail if lifecycle checks resize decoder scratch storage
    abort();
}
bool AudioStream_IsCutscene(void) {
    // Leave the game music query outside stream initialization
    abort();
}
void re3_debug(const char *format, ...) {
    // Leave decoder diagnostics outside stream initialization
    abort();
}
"""
    shim = '#define AL_LIBTYPE_STATIC\n' + shim
Path(sys.argv[1]).write_text(shim + source + mocks + runtime, newline='\n')
