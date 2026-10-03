# Plain C audio state and police-radio queue

Four types no longer use C++ constructors/destructors or member methods:

- `cAudioScriptObjectManager`, `cPedParams`, and `cVehicleParams` are plain
  structs in the standalone C11 header `src/audio/AudioState.h`.
- `cPoliceRadioQueue` is a plain struct with C reset/add/remove functions in
  `src/audio/AudioPoliceQueue.h`.

The game audio manager explicitly initializes its embedded script-object state
and police queue before its existing setup. Its destructor explicitly resets the
script-object count after its existing termination logic. Other live count
resets also use the C helper; indices remain untouched. All five local pedestrian
and vehicle contexts are explicitly initialized before use. All 43 police-radio
queue call sites now use the C functions. The enclosing cAudioManager and its
processing methods still contain C++ and remain future conversion work.

The queue retains its 60-sample capacity, three byte-sized cursors/count fields,
244-byte layout, wraparound arithmetic, and NO_SAMPLE empty sentinel. Reset
keeps the stored samples, and a full queue leaves its storage unchanged. No
queue allocation or synchronization behavior was added.

Run `utils/tests/test_audio_state.ps1` from the repository root. It extracts the
original constructors and queue methods unchanged from the saved headers in
`build/audio-state-c-tests/before`, then compares them with the actual C headers
using Clang and GCC, C and C++ callers, and O0/O2 optimization. Native MSVC x86
and x64 C/C++ runs also matched their same-architecture baseline digests.

The fixture compares sizes, alignments, key offsets, every initialized parameter
field, script-count resets with retained indices, and defined queue contents.
It deliberately excludes uninitialized object bytes and padding from behavior
comparisons. It covers 256 storage seeds, empty/full boundaries, overflow without
writes, wraparound from every starting cursor position, stored NO_SAMPLE values,
and 200,000 mixed queue operations with periodic resets. C++ compile assertions
verify all four converted structs have trivial lifecycles.

All 150 existing audio-consumer configuration checks passed after conversion.
Native MSVC x86/x64 compiled AudioManager.cpp, AudioLogic.cpp, and PolRadio.cpp
with the new headers. The two headers are registered in the existing Visual
Studio project and filters; CMake/premake already collect headers by glob.

These are quiet compile and baseline regression checks. They do not verify a
full linked game build or in-game police radio and vehicle/pedestrian playback.
