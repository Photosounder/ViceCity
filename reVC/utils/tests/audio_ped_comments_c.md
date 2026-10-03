# Pedestrian-comment queue C conversion

`AudioPedComments.h` defines `tPedComment` and `cPedComments` as plain C
structs. The position is three floats, preserving the original 28-byte comment
layout and conditional emitting-volume field. Queue state retains its original
platform-dependent delay fields and layout. AudioManager explicitly initializes
its embedded state through `PedComments_Init`.

`AudioPedComments.c` is a real C11 translation unit implementing initialization,
volume-ordered insertion/replacement, pending-comment timeout carryover, and
queue switching/clearing. Initialization writes only the original constructor's
fields. The original physical-slot insertion bound, tie handling, lowest-volume
rejection rule, and retained inactive record contents are unchanged.

`PedComments_Process` is a free function with C linkage and an explicit state
pointer. Its implementation remains in AudioLogic.cpp because playback uses
C++ game services. Its static previous-sample history, pause handling, load and
playback decisions, and platform delay handling are preserved. The queue advance
portion now calls the C module. The two comment producers and the playback
consumer copy position components at their C/game-vector boundaries.

Run `utils/tests/test_audio_ped_comments.ps1` from the repository root. Baseline
sources are saved in `build/audio-ped-comments-c-tests/before`. The generator
checks constructor, insertion, and carryover statement equivalence and audits
the entire playback adapter and affected game integration. The fixtures link
both C and C++ callers to the actual C implementation and compare layouts and
all defined queue fields against the saved class implementation. Padding and
otherwise indeterminate constructor fields are excluded from comparison.

Each run covers 160,000 insertions, tied/ascending/descending/random volumes,
full queues, rejection and replacement, both active queues, signed-zero positions,
timeouts from -128 to 127, and draining pending comments. Clang/GCC at O0/O2 pass
in four field layouts: current PC, FIX_BUGS disabled, external 3D sound disabled,
and PS2-style fields. These layouts exercise the queue module; they do not prove
a complete PS2 game build. Native MSVC x86/x64 baseline comparisons also pass.

All 200 consumer compile checks passed. Native MSVC compiled AudioManager.cpp,
AudioLogic.cpp, and the actual C module, and object symbols confirm matching
C linkage for the queue functions and the playback adapter. The existing Visual
Studio project/filter registrations compile the new source as C11; CMake and
premake already collect C sources. Details are saved in
`build/audio-ped-comments-c-tests/verification.json`.

No full linked game build or in-game speech playback test has been performed.
