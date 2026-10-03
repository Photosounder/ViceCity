The per-stream OpenAL queues hold four left/right pairs, derived from the eight
buffer IDs owned by each stream. At startup each pair enters the fill queue once.
The worker borrows its front while decoding, then removes it before publishing
it to the ready queue. Playback transfers ready pairs to the two OpenAL source
queues. Update unqueues each processed pair once, collects it locally, and
returns it to the fill queue. Consequently each stage needs at most four entries;
there is no allocation or capacity growth in the new queue implementation.

StartStreamedFile calls Setup before Start. PreloadStreamedFile closes and opens
the stream and calls Setup before StartPreloadedStreamedFile starts it. Setup
stops and clears the device queues. Loop restart refills only when device, fill
and ready queues are empty. Seek, Close and ProviderTerm discard pending pairs;
provider initialization uses fresh device sources. Callers must continue to
respect these existing stream lifecycle rules.

The stream mutex serializes fill-queue changes and resets; the ready queue also
retains its existing producer/consumer mutex. A revision token replaces the old
front-address comparison across unlocked decoding. Appending preserves the
token, while removal and reset invalidate it, including when an embedded slot
is immediately reused. A second check rejects pending publication invalidated
while the worker unlocks for a seek.

Run `utils/tests/test_audio_buffer_queue.ps1` for C11 and C++17 tests under clang
and gcc at O0/O2. Each standalone run compares one million operations against a
linear FIFO model. The integration fixture extracts production stream methods
from stream.c on every run and uses the real stream.h. Mock OpenAL functions
assert device capacity, FIFO order and unique physical buffer ownership. A gated
decoder forces Start, seek, provider reset and Close during unlocked decoding.
It also exercises 400 normal recycling cycles and 100 synchronous cutscene cycles
per compiler/optimization run. No window or audio device is opened.

The global stream scheduling queues now use C linked FIFOs. Stream state and functions now use the C API in stream.h. Threading and
synchronization use the C API in AudioThread.h. The decoder interface, concrete formats and
ADPCM algorithms now use C.
These tests cover queue ownership and synchronization rather than real-device
audio quality or decoder correctness.
