# Mission and reflection position C conversion

AudioManager's mission and reflection position arrays now contain plain
`AudioSoundPosition` records rather than CVector instances. No embedded CVector
remains in its state fields. Reflection coordinates copy directly into sound
records; camera coordinates and geometry endpoints use the existing explicit
game-vector boundary helpers. Reflection scheduling, offsets, trace flags,
hit/miss distance updates, and playback calculations are unchanged.

`AudioMissionPosition.c` is a real C11 translation unit implementing the former
mission-location validation and storage operation. It preserves the initialized
flag and slot checks, clears only the selected positional/2D mode flag, and
copies three coordinates. The C header owns the unchanged two-slot constant.
The existing game-facing setter delegates to C. Mission distance and camera
translation still use the original C++ services through copied game vectors.

Run `utils/tests/test_audio_positions.ps1` from the repository root. Saved
sources are in `build/audio-positions-c-tests/before`. The generator audits the
position consumers and stored-field declarations and extracts the actual old and current
mission setters and reflection update bodies. It uses the game's coordinate
constructor verbatim in a controlled vector fixture. Geometry services record
the actual trace endpoints, heights, and flags and supply controlled hit/miss
results and distances to both adapters.

Regression coverage includes all 256 byte slot values, zero and arbitrary
nonzero initialization flags, retained other-slot storage, exact signed-zero,
infinity, and quiet-NaN coordinate bits, all reflection slots, all hit/miss
patterns, shifted ceiling origins, camera lookup counts, retained positions on
non-update frames, and unsigned frame-counter wraparound. Both historical
reflection branches (five 50-unit slots and eight 100-unit slots) are exercised.
Clang/GCC O0/O2 comparisons pass for both branches; C and C++ callers link the
actual C mission setter. Native MSVC x86/x64 baseline comparisons also pass for
the current branch.

All 230 consumer compile checks and four additional early-version game-source
checks passed. Native MSVC compiled AudioManager.cpp, AudioLogic.cpp, and the
actual C module; object symbols confirm matching setter linkage. Native owner
ABI fixtures include the actual before/current AudioManager headers and emit
compiler-computed layout constants. `check_audio_owner_abi.py` reads those
constants directly from COFF objects, avoiding unrelated game/library linking.
The complete owner size/alignment and all 54 declared state fields' offsets,
sizes, and alignments match: 26200 bytes/alignment 4 on x86 and 28376 bytes/
alignment 8 on x64.

The sound regression audit now checks the relevant sound-position consumers
independently of unrelated state migrations; its runtime suite is rerun. Entity
and pedestrian source audits also pass. The existing Visual Studio project and
filters register the new C11 source/header; CMake and premake already collect
C files. Details are in `build/audio-positions-c-tests/verification.json`.
Reflection tracing and mission playback remain C++ game integration. No full
linked game build or in-game playback test has been performed.
