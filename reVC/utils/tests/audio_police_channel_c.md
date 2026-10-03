# Police channel and crackle setup in C

AudioPoliceChannel.c implements the former ServicePoliceRadioChannel and
DoPoliceRadioCrackle members as actual C11 functions. Police radio service and
MusicManager call the C operations directly. Three C-linkage host operations in
PolRadio.cpp retain requested-queue submission and the conditional game timer
getters. The broader report/player/zone logic remains C++.

Channel-open state, wait counter, physical mission-playing status and saved
frequency remain original function-local statics shared across owners. Pause,
resume, mission preload/start/completion, queue removal, noise toggling, optional
scaled frequency, channel setup order, and processed-report reset are preserved.
Backend initialization failure still follows the original subsequent operations.
Unsigned logical-frame subtraction and assignment back to the signed wait counter
retain the original arithmetic; no clamp or alternate wait policy is introduced.

Crackle preparation writes the same fields to the live queue sample. The C code
expands only the existing conditional emitting-volume, loop-endpoint, reverb and
reflection macros. Position, distance, speed and unrelated/internal fields remain
untouched until the existing requested-queue operation changes them.

Run utils/tests/test_audio_police_channel.ps1. Its generator audits both complete
bodies, the host adapters, full PolRadio/MusicManager/header edits, and the exact
sound-macro expansions against the before snapshot. Original member code and the
actual C module link the same existing C police state. The controlled backends
record calls, arguments and partially written state; every stored sound field is
observed without padding. A mutable requested-queue callback verifies the live
owner and sound boundary. A real C caller reaches both exported operations.

Twenty Clang/GCC O0/O2 comparisons cover default, vanilla, unscaled, no-external,
and PS2 without reverb. Native MSVC x86/x64 comparisons pass. The trace exercises
both owners, 256 state combinations, valid queues with noise/sentinel/full-width
sample IDs, mission stream transitions, pause edges, bounded frequency scaling,
negative random remainders, and logical wait values including zero and overshoot.
Fifteen prior phase source audits pass. Consumer and native game compilation
checks cover the changed callers and actual C modules.

Audio backend, requested-queue behavior and game timer reads are controlled
callbacks. Frequency products are nonnegative and representable. No full linked
game build, actual audio playback or game queue implementation is verified by
the runtime fixture; the production host delegates remain unchanged operations.
