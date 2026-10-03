# Crime audio C conversion

`cAMCrime` is now a plain C record in `src/audio/AudioCrimes.h`. Its layout
remains 20 bytes, with three position floats at offset 4 and the timer at
offset 16. `AudioCrimes.c` is compiled as C11 and implements explicit record
initialization, report updates, and aging. AudioManager initializes its embedded
records explicitly instead of invoking their C++ constructors.

The game-facing report adapter retains the original initialization, music mode,
wanted level, enum predicate, and cooldown checks. Duplicate reports refresh the
first matching record without extending its cooldown; new reports use the last
empty slot. Radio reinitialization still clears only crime types. The C position
is copied into a local CVector for FindAudioZone, whose implementation only reads
the position through PointLiesWithinZone.

Run `utils/tests/test_audio_crime.ps1` from the repository root. Saved baseline
sources are in `build/audio-crime-c-tests/before`. The generator verifies that the
report and aging statements match after the intended C substitutions and that
the game eligibility gate is unchanged. Fixtures compare actual old and new
report adapters and link C and C++ callers against the actual C module.

Coverage includes layout, initialization, empty/sparse/full arrays, duplicate
updates, eligibility short-circuit behavior, cooldown boundaries and unsigned
wraparound, signed-zero position copies, the 1200-frame expiration boundary,
inactive records, and 16-bit timer rollover. Only valid enum inputs are exercised;
the original enum predicate is retained. Padding is excluded from comparisons.
Clang and GCC at O0/O2 match the saved baseline. MSVC x86/x64 fixtures also match
their same-architecture baseline. All 190 consumer compile checks passed, and
native MSVC compiled AudioManager.cpp, PolRadio.cpp, and AudioCrimes.c. Native
object symbols confirm that game references match the C exports.

The existing Visual Studio project and filters register the new source/header;
CMake and premake already collect C files. Results and audit details are recorded
in `build/audio-crime-c-tests/verification.json`. These are quiet compile, ABI,
and regression checks; a full linked game build and in-game audio testing have
not been performed.
