# C OpenAL effects interface

oal_utils.h exposes C linkage for effect helpers and extension procedure pointers.
The old re3_openal namespace is removed. Every procedure pointer has an AudioEFX_
prefix so its global C symbol can coexist with SDK exports from static OpenAL.
EFXInit still requests the original unprefixed SDK names from alGetProcAddress.
The implementation now builds as oal_utils.c without common.h or other C++ headers.
The channel and sample-manager callers use the prefixed variables directly.

The existing Visual Studio project and filters now include oal_utils.c as C11.
CMake and Premake already include C files and select the C compiler. MSVC v143
object compilation passes for x86 and x64. The existing float-to-long and
long-to-float conversions in reverb mixing still produce warnings; they are
preserved to retain the original integer millibel behavior.

Run utils/tests/test_audio_efx.ps1 for eight clang/gcc C11/C++17 O0/O2 cases.
Each compares the exact SDK lookup and output-bit trace against the saved original
C++ source in build/audio-efx-c-tests/before-oal_utils.cpp and its original header.
The baseline snapshot is required for these comparisons. Every run checks 33
procedure lookups and 196632 effect/filter setter calls, including 4096 listener
configurations, reverb vector components, clamp boundaries, silence, threshold
and above-unity mixing gains. The new C fixture includes static OpenAL SDK
prototypes and a real alGenEffects symbol to check symbol coexistence.
Neither the production C source nor its calculations are rewritten in the new
fixture. The baseline substitutes only the old common include and its float Min
helper so the original calculation code can run quietly with mock OpenAL calls.

Consumer syntax checks cover both rendering backends, 32/64-bit targets,
threaded/single-thread paths and WAV/optional format selections for stream.c,
sampman_oal.cpp, oal_utils.c and channel.c. Forty configurations pass.
Tests open no window or audio device. A full game build is not part of this phase.

CChannel now uses a C struct and functions in channel.c. See audio_channel.md
for the next phase's state, loop-counting and PCM-upload comparisons. The device
list now uses C as well. The sample manager and other project subsystems still
contain C++ features.
