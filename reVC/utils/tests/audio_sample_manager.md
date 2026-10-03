# Sample manager C interface checks

Run `powershell -ExecutionPolicy Bypass -File utils/tests/test_audio_sample_manager.ps1` from the repository root.

The fixture extracts 25 production methods from the current OpenAL backend and compares their output with the saved C++ class in `build/audio-sample-manager-c-tests/before`. Baseline fields are made public only in the fixture to populate identical metadata and inspect state. Tests sweep all byte volume values, initialized and uninitialized effect updates, every sample slot, bank boundaries, provider name publication, and every channel's frequency, signed loop endpoints, loop counts and playback predicate. It also compares cutscene attenuation, finale silence and all stream volume/pan paths including user-track boost. The standalone current fixture compiles as C11 and C++17 with clang and GCC at O0/O2. MSVC x86/x64 checks are recorded in the build directory.

The public manager header and caller conversion cover OpenAL, Miles and null backends. All stream default arguments are supplied explicitly at callers. Miles file callbacks register once during explicit initialization before its SDK startup and file reads. The OpenAL backend is now `sampman_oal.c`; its game-state queries use the C callbacks in `AudioSampleHost.h`, implemented by the small C++ game adapter. The null and Miles backends are also C11 now; all three sample-manager backends use C source files.

The phase's consumer matrix recorded 100 syntax checks across Win32/Win64, D3D9/software, threaded/single and optional decoder configurations, plus four Miles/null checks. Device enumeration and cached provider name lifetime regressions also pass. These are quiet fixture and compile checks; they do not replace a full linked game build or in-game audio testing.
