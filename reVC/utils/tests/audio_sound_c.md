# Sound record and requested ordering C conversion

`AudioSound.h` defines `tSound` as plain C data. Its former CVector member is
`AudioSoundPosition`, containing three floats. All fields, platform conditionals,
field types, byte flags, and field order are preserved. The current PC record
remains 96 bytes. The original default vector constructor performed no writes;
removing it introduces no new initialization or clearing behavior.

`AudioSoundQueue.c` is a real C11 translation unit implementing the former
AddDetailsToRequestedOrderList method with explicit requested-record and order
arrays. The original physical-slot search bound, strict priority comparison,
stable ties, and active-sample-based memmove length are retained. The parent
method declaration and definition are removed, and the owner calls C directly.

`AudioSoundGame.h` contains two explicit game boundary helpers that copy three
coordinates between the plain C position and CVector. It remains a C++ adapter.
Assignment, reflection save/restore, camera translation, distance, and line-of-
sight calls use explicit copies; the existing geometry operations themselves
are unchanged. AudioManager.cpp and AudioLogic.cpp remain C++ game integration.

Run `utils/tests/test_audio_sound.ps1` from the repository root. Saved sources
are in `build/audio-sound-c-tests/before`. The generator verifies every sound
field declaration, the ordering statements, and every sound-position-consuming game function
in both integration files. It verifies the game's empty
default vector constructor and copies its coordinate-constructor body verbatim
into a controlled vector fixture to exercise the real boundary-helper bodies.
Native production compiles use the actual game vector and other game headers.

Regression comparisons cover record size/alignment and every field offset and
size, every present field, copied records, exact coordinate bits through both
boundaries, signed zero, finite extremes, infinity, quiet-NaN payloads, every
active capacity and physical insertion slot, both requested queues, repeated
slot replacement, and ascending/descending/tied/random priorities. Struct
padding is excluded from behavioral comparisons. C and C++ callers link to the
actual C ordering module.

Clang/GCC O0/O2 comparisons pass in six conditional configurations: current PC,
FIX_BUGS disabled, external 3D sound disabled, reflections disabled, and PS2-style
fields with and without reverb. These configurations test record and queue
behavior, not complete alternative-platform game builds. Native MSVC x86/x64
regressions match their same-architecture original implementation. All 220
consumer compile checks passed. Native MSVC compiled both game integration
files and the actual C module; object symbols confirm matching C linkage.

The existing Visual Studio project and filters register both headers and the
new C11 source; CMake and premake already collect C sources. Existing entity
and pedestrian-comment audits now check their converted owning functions
independently of unrelated subsystem migrations. Their regressions are rerun
after this conversion. Verification details are saved in
`build/audio-sound-c-tests/verification.json`. No full linked game build or
in-game audio playback test has been performed.
