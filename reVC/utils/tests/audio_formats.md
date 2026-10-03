AudioFormats.h implements WAV, libsndfile, MP3, ADF, VB and Opus as C structs and
functions. Format initialization explicitly sets fields formerly initialized by
constructors, including the WAV format header. Allocation stays at the existing
owning CStream_Open call sites; initialization and function-table binding need
no additional allocation. ADF reuses the plain MP3 state and common operations,
with separate initialization and reader callbacks for its XOR file-opening path.

AudioStereoBuffer.h replaces the stereo sorting class with plain scratch-buffer
state and C functions. CStream_Terminate releases shared scratch storage after
joining the audio worker. Cleanup clears the pointer and capacity to support
later initialization. Growth and interleaved-to-planar sample ordering follow
the previous code. The Opus sample-rate tag parser uses an int temporary for its
existing %i conversion and then assigns the result to the unsigned rate field;
invalid tags still leave the default rate in place.

Run utils/tests/test_audio_formats.ps1 for actual production WAV/VB playback
through C11 and C++17 under clang and gcc at O0/O2. Tests cover metadata, stereo
ordering, seek, EOF, cleanup from dirty initial storage, scratch-buffer growth,
shutdown and restart. The same script checks all optional format code against
the bundled SDK headers as strict C11/C++17 in both 32-bit and 64-bit modes.

Run utils/tests/test_audio_decoder.ps1 for saved C++ baseline comparisons using
the real CStream_Open path, varied IMA/VAG histories, PCM/ADPCM samples and VB
decode/seek/EOF behavior. Run utils/tests/test_audio_buffer_queue.ps1 for quiet
worker reset, close, playback recycling and cutscene regressions. Verification
reports and pre-change source snapshots are in build/audio-format-tests.

Optional encoded-file playback and real-device audio quality have not been
exercised. Their API bindings and all format implementations compile in C. The global
scheduling queues now use C linked FIFOs. The stream object and C++
synchronization remain conversion work; the whole-project conversion is active.
