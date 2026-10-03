# OpenAL sample backend compiled as C

`src/audio/sampman_oal.c` is the complete OpenAL sample-manager translation unit. It includes C headers, uses C COM calls for Windows shortcuts, retains direct CITA allocations at the owning call sites, and calls `AudioSampleHost.h` for game state. `AudioSampleHost.cpp` contains only the remaining C++ game and platform integration. Reflection and loading-status constants are shared through `AudioReflectionTypes.h`. CMake/premake discover both sources; the existing Visual Studio project explicitly compiles the backend as C11.

Quiet checks from the repository root:

- `powershell -ExecutionPolicy Bypass -File utils/tests/test_audio_sample_manager.ps1`: 25 actual methods compared with the saved class, including cutscene and streaming-volume behavior
- `powershell -ExecutionPolicy Bypass -File utils/tests/test_audio_sample_host.ps1`: every actual game-state callback linked to a C caller, plus the production EAX implementation (now C11) and preset data
- `powershell -ExecutionPolicy Bypass -File utils/tests/test_audio_sample_opus.ps1`: actual mission/ped loaders with bounded partial reads, failed seeks, decoder errors, early EOF, PCM alignment and output guards
- Existing stream, channel and device-list regression scripts cover the lower-level C implementations and cached provider lifetime

The full phase report is `build/audio-sample-native-c-tests/verification.json`: 120 consumer syntax checks cover Win32/Win64, D3D9/software, single/threaded and WAV/all decoder configurations; WAV checks explicitly disable optional decoders. Eight additional clang/GCC C11 checks cover default, Opus SFX, MASTER/JUICY and disabled reflection configurations. Native MSVC x86/x64 compiles the complete C backend and C++ adapter; mixed C/C++ callback/EAX fixtures also link and run on both. Miles/null checks pass.

Verification found and corrected three preexisting problems: SDK include ordering could select an unrelated `config.h`; MSVC Win32 had incompatible duplicate `ssize_t` typedefs; and Opus SFX mission reads incorrectly treated the decoder handle as a `FILE *`, while the ped branch referenced an undefined bank constant. Opus reads now fill the mission PCM buffer before publishing its loaded sample. The raw mission path remains unchanged.

All behavior fixtures use mocks instead of a sound device. Full linked game builds, native POSIX builds and in-game audio remain unverified. An exploratory attempt to link the complete backend alone failed because its other audio modules and SDK dependencies were absent; it is not counted as a passing whole-backend link test. No GUI tests ran.
