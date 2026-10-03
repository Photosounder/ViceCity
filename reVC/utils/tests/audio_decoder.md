The stream decoder interface now uses AudioDecoder and AudioDecoderOps from
src/audio/oal/AudioDecoder.h, which compiles as C11 and C++17. An embedded handle
points to the externally owned concrete state and its operations. Each format
has an explicit table, including ADF's distinct FileOpen operation. Owning stream
allocation sites still allocate the concrete state directly. Close calls the
concrete resource callback before the owning synchronous or worker path frees
that state. The handle itself needs no separate allocation.

The IMA ADPCM and VAG algorithms now use plain C state structs and free functions
from AudioADPCM.h. Immutable step and predictor tables are shared. WAV and VB
explicitly initialize the state arrays and free them without constructor or
destructor loops. Sample calculations and rounding follow the previous code.

Run utils/tests/test_audio_decoder.ps1 for C11/C++17 interface and ADPCM tests
under clang and gcc at O0/O2. The concrete fixture extracts production decoder
implementations and the actual CStream constructor, Open and IsOpened methods on
each run. It verifies WAV PCM and IMA decoding in mono/stereo, raw VB decoding,
metadata, repeated seek, EOF and cleanup. Before/after transcripts also compare
318976 IMA nibble outputs over varied starting samples and step indices, and
116480 VAG samples over all five valid predictors and thirteen shift settings.
The saved pre-change sources in build/audio-decoder-tests supply the baseline.

Run utils/tests/test_audio_buffer_queue.ps1 for worker close/reset and cutscene
regressions using the new function-table interface. No device or window is opened.
The consumer compile matrix covers D3D9/software, threaded/single-threaded audio,
WAV-only/all optional codecs, and 32-bit builds. Optional codec libraries are
covered by compilation; their actual encoded-file decoding has not been tested
in this phase. The six concrete formats now use C structs and functions from AudioFormats.h.
Stereo scratch storage uses an explicit C cleanup function after worker shutdown.
The global scheduling queues now use C linked FIFOs. The stream object and
C++ synchronization remain conversion work.
