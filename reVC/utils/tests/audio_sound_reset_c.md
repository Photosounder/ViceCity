# Requested and active sound reset C conversion

`AudioSoundReset.c` is a real C11 translation unit implementing the former
AudioManager ClearRequestedQueue and ClearActiveSamples methods. The C API takes
explicit order/count pointers or active records plus the active sample count.
Both member declarations and definitions are removed, and every owner call site
uses the C interface directly.

Requested reset changes only the selected order prefix and its count. It leaves
requested records, the inactive queue, and order tail entries unchanged. Active
reset writes exactly the original fields for the selected active prefix. It
retains fields the original omitted, including front/rear pan and conditional
emitting-volume change, as well as records beyond the active count. Positions
reset to three explicit positive-zero floats. The implementation uses the
existing bank/sample enums rather than duplicating their values.

The owner's constructor, destructor, and other lifecycle remain C++ for now.
Constructor operations keep their original order, including the five random
number draws at startup. The edit replaces only each logical reset call, without
moving random-table generation or changing the global object's registration.
Native objects retain the original dynamic initializer/destructor callbacks.
This removes two further methods and prepares initialization for a later full
owner-class conversion without changing startup timing in this step.

Run `utils/tests/test_audio_sound_reset.ps1` from the repository root. Saved
sources are in `build/audio-sound-reset-c-tests/before`. The generator verifies
both reset bodies and all AudioManager source/header edits against intended
substitutions. The regression fixture executes original member bodies and actual
C implementations with both C and C++ callers. It defines and compares every
record field, excludes padding, and checks every active capacity including zero,
both queues, retained live state and inactive tails, repeated resets, and exact
positive-zero position bits.

Clang/GCC O0/O2 comparisons pass in six conditional field configurations. Native
MSVC x86/x64 comparisons pass as well. All 240 consumer compile checks passed,
and native MSVC compiled the real C module and both major game integration
files. Object symbols confirm matching C provider/game-reference linkage.
Existing sound and mission/reflection regressions are rerun after the conversion;
entity and pedestrian source audits also pass. Their audits check their relevant
converted operations independently of unrelated method migrations.

The existing Visual Studio project and filters register the C11 source; the
shared C header declares both exports, and CMake/premake already collect C files.
Details are in `build/audio-sound-reset-c-tests/verification.json`. No full linked
game build or in-game audio playback test has been performed.
