# Police state and initialization in C

AudioPoliceState.c owns five former owner methods: zone initialization, radio
initialization/reset, and mission script setter/status query. The public DMAudio
radio reset facade is also C. Shared zone records, report flag, report deadlines,
and mission sample/status globals each have a single C definition. Their startup
values are preserved. AudioPoliceState.h exposes their fixed-width declarations.

All original partial writes and backend call ordering are retained. Zone names
are cleared before copying the same 14 labels and samples; the unused zone field
is retained. Radio initialization resets queue cursors/count without clearing
sample storage, changes crime types without changing coordinates/timers, applies
the original PS2/reverb conditional, clears the special-report flag, and assigns
the owner's frame counter to every report deadline. Reset retains its initialized
gate and conditional channel stop before initialization. The setter uses only the
original initialization byte and refuses writes while the shared status is
PLAYING. The getter retains signed byte results.

Mission playback directly calls the C setter/getter. Its two police C++ host
adapters are removed; the camera adapters remain. Existing startup, radio service,
MusicManager and facade callers use the C functions directly.

Run utils/tests/test_audio_police_state.ps1. Its generator independently audits
all five bodies, the facade, all storage initializers/zone fields, the owner header,
PolRadio header and complete caller edits against the before snapshot. The trace
covers every initialization byte and every signed-status byte representation,
five sample IDs, both live owners, frame boundaries, used-channel values, retained
fields and callback-visible reset order. A real C caller checks startup globals
and reaches all operations and the public facade. Baseline-only C bridges invoke
the original member implementations for that caller.

Sixteen Clang/GCC O0/O2 comparisons cover default, vanilla, no-external and PS2
without reverb. Native MSVC x86/x64 comparisons pass. Sixteen prior mission
playback runtime comparisons pass with the actual police C module linked; fourteen
prior source audits pass. Consumer compilation includes PolRadio.cpp and both
native architectures. No full linked game build or audio device playback is
verified; channel/reverb backend functions are controlled callbacks.
