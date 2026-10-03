# Mission labels and preloading in C

AudioMissionData.c owns the original 1,121-entry label table, case-folding loop,
lookup and loaded-label functions, preload operation, and shared load-failure
array. Two owner methods and two public DMAudio facade implementations move out
of C++. Spatial mission playback remains in AudioLogic.cpp. Owner fields and
layout are unchanged.

Table order, duplicate aliases, first matches, unknown-label diagnostics, and
THIS_IS_STUPID fallback are preserved. The comparator retains the original
ASCII_STRCMP conditional. Preloading preserves partial state visible to the
backend, byte-sized stream argument conversion, signed duration multiplication,
division and subsequent frame multiplication. Coordinates and phone flags are
retained. The failure array is a single C definition shared with remaining C++
playback code.

Run utils/tests/test_audio_mission_data.ps1. Its generator compares the complete
table and comparator, all three operation bodies, both facade bodies, and full
owner/header/caller edits against the before snapshot. Controlled backend and
logger callbacks compare exact diagnostics and the state visible during duration
lookup. Every table label is tested with case variants and shortened/extended
names, both slots, four time values, and seven representable signed durations.
Invalid-slot gates and retained state are included.

Verification: 320 consumer syntax checks; 16 Clang/GCC original-versus-C runtime
comparisons at O0/O2; six native MSVC comparisons across x86/x64 and default,
locale, and fallback configurations. Eleven previous phase source audits pass.
Native game consumer compilation also passes for both architectures.

The backend is mocked; no full linked game build or audio playback is verified.
Signed-overflow duration inputs and valid-slot null names are excluded. The
original comparator's signed-character behavior is retained without extending
its defined-input contract.
