# DMAudio C interface conversion

`DMAudio.h` is now a standalone C11 header with 62 `DMAudio_*` exports.
The stateless `cDMAudio` class and global `DMAudio` object were removed.
Game callers use functions directly. Crime reporting takes a position pointer;
`CrimeTypes.h` provides the shared C enum without the crime queue class.

`DMAudio.cpp` still contains the game integration and compiles as C++ because
its underlying AudioManager and MusicManager implementations remain classes.
This phase removes the facade class; it does not claim to convert those managers.

Run `utils/tests/test_audio_dma.ps1` from the repository root. The fixture uses
the saved pre-conversion files in `build/audio-dma-c-api-tests/before`.
It checks that all forwarding bodies are identical after the intended name and
crime-pointer substitutions, then compiles every export's caller as strict C11
and links it to the actual facade bodies with mock game managers. Its trace is
compared with the saved C++ class for Clang and GCC at O0 and O2.
The fixture also exercises all 256 input values for each volume clamp, negative,
zero, and positive script-entity handles, scalar and pointer results, crime
positions, and the player speech flag. This validates forwarding and linkage,
not audio playback or the real managers' behavior.

The phase also compiled the production audio consumers in 130 combinations of
renderer, architecture, threading, and decoder configuration. Native MSVC x86
and x64 checks compiled the production facade and sample host plus a C caller;
both native builds reproduced the baseline fixture digest. OpenAL and Miles
sample-host callback regression checks passed after the caller migration.

The wider affected-caller check compares failures against the saved source.
Existing Clang interpreter function-pointer conversion errors in `main.cpp`
and the software `glfw.cpp` failure were reproduced with the saved source.
Of 128 affected-caller checks, 125 passed and three retained baseline failures.
The saved phase reports record which callers
passed and which retained a baseline failure. A full linked game build and
in-game testing are not covered by these checks.
