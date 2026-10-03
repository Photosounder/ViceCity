# Audio script object conversion

`src/audio/AudioScriptObject.c` is an actual C11 translation unit. Its header
contains a plain `cAudioScriptObject` struct and C function declarations.
The former constructor, destructor, allocator construction, and static methods
have been replaced with explicit reset/save/load functions. All owners reset
the record before returning it to the pool, preserving the previous cleanup.

The position is three floats rather than a CVector class. Compile-time assertions
retain the 20-byte record size, position offset 4, and entity offset 16. Game
callers copy vector components explicitly and evaluate one-shot positions once.
The existing AUD save header, pool handles/generations, descending save order,
record padding, and loaded entity recreation are preserved.

`CPools_GetAudioScriptObjectPool` exposes the existing pool with C linkage;
CPools itself remains C++. The shared save-validation counter now has C linkage
in both SaveBuf.h and its definition in re3.cpp. The C source keeps the game's
assertion behavior through the existing re3_assert export.

Run `utils/tests/test_audio_script_object.ps1` from the repository root.
The fixture uses the saved pre-conversion files in
`build/audio-script-object-c-tests/before`. It compiles the unchanged original
class bodies with minimal game type definitions, and links the real new C
translation unit to C and C++ callers. Both variants use the actual C pool and
save-buffer implementations with mock audio callbacks.

The fixture checks all 256 occupancy patterns in an eight-slot pool, nonzero
padding, saved generations, record sizes, exact save/re-save bytes, recreated
entity handles, resets, and active/inactive one-shot allocation. Clang and GCC
O0/O2 builds matched the baseline digest. Native MSVC x86 and x64 O2 builds also
matched, including the actual game's save-counter definition extracted from
re3.cpp and a C++ caller linked to the C module.

Production syntax checks passed for 150 audio consumers/configurations and all
28 affected game-caller checks across D3D9 and software rendering. Native MSVC
x86/x64 compiled the real module, facade, pool adapter, sample host, and re3.cpp.
Native object symbols confirm the new C module's pool, assertion, and save-counter
references match the game definitions. Release/MASTER C compilation also passes.
The existing Visual Studio project and filters now register the `.c` source
with C11 compilation; the existing CMake/premake source globs already include C.

These are quiet compile, ABI, and regression checks. A full linked game build,
real audio playback, and in-game save/load testing have not been performed.
