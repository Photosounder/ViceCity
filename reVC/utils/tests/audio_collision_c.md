# Collision audio C conversion

`cAudioCollision` and `cAudioCollisionManager` are plain C structs in the
standalone C11 header `src/audio/AudioCollision.h`. Position storage is three
floats, and the game copies components explicitly at the vector boundaries.
The parent AudioManager explicitly initializes its embedded collision state.
Reset preserves the existing base-volume field rather than clearing it.

Two real C translation units implement the dependency-free collision logic:

- `AudioCollisionQueue.c`: record reset, manager initialization, and the original
  queue insertion/replacement algorithm with an explicit manager pointer.
- `AudioCollisionMath.c`: the three former AudioManager collision-ratio methods.

The existing surface and adhesive enums were moved verbatim into
`src/core/SurfaceTypes.h`; SurfaceTable.h includes that C header and retains its
C++ game helpers. AudioCollision.cpp still contains the game integration,
collision service, and sound setup methods. AudioManager itself remains C++.

Run `utils/tests/test_audio_collision.ps1` from the repository root. The fixtures
use saved sources in `build/audio-collision-c-tests/before`. The generator checks
the queue insertion and ratio bodies match the original code after only the
intended C name/type/state-pointer substitutions. The regression fixture links
both C and C++ callers to the actual C modules and compares against the saved
class implementation with Clang/GCC at O0/O2. Native MSVC x86/x64 runs also match
their same-architecture baseline.

Coverage includes record/manager sizes, alignments and offsets, live reset with
retained base volume, initialization of both collision arrays, ascending,
descending and tied distances, full-queue rejection and replacement, 100,000
mixed requests across repeated frame resets, infinity, NaN, and signed zero.
The existing index-table traversal and stopping condition are preserved exactly.
All surface ratios are sampled over ordinary intensities and immediately below,
at, and above their thresholds. Primitive ratio tests include reversed and zero
ranges. Finite results and infinities are compared by their float bits; NaNs are
compared by classification because constant folding and runtime arithmetic can
produce different NaN payload/sign bits. Struct padding and uninitialized fields
are excluded from behavioral comparisons.

Game integration was checked structurally against the intended substitutions;
the copied surface enums are unchanged. Production checks cover 180 combinations
of audio/surface consumers, rendering, architecture, threading, and decoders.
MSVC x86/x64 compiled the actual C modules, AudioManager.cpp, AudioCollision.cpp,
and SurfaceTable.cpp. Native object symbols confirm the game references match
the new C exports. Both C sources and SurfaceTypes.h are registered in the
existing Visual Studio project and filters; CMake/premake already collect C files.

These are quiet compile, ABI, and regression checks. A full linked game build,
actual collision sound playback, and in-game testing have not been performed.
