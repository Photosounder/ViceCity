# Cessna and car-helicopter audio conversion

AudioAircraft.c owns the complete ProcessCarHeli and ProcessCesna generators as C11. Every internal caller uses explicit manager and vehicle-parameter pointers. Visual Studio registers the C unit; CMake and premake already discover C sources.

The helicopter generator now uses plain AudioCameraMatrix and AudioSoundPosition records. Camera up and forward dot products and the rear rotor matrix-vector product move into C with every original term and operation order retained. Typed game adapters supply rotor inputs, the live vehicle basis, captured automobile boot position and active fixed-camera predicate. Existing player lookup, pad, throttle, model, engine and camera adapters are reused.

All three function-static frequency histories remain shared across managers, initialized to 14287, 7143 and 14287. The generator retains player/AI controls, captured automobile/Skimmer pointers versus later live vehicle reads, rotor zero/cap gates, long-distance fade, front and starting layers, Hunter samples, propeller history and original integer division by 255, fixed-camera Skimmer layer and rear-rotor distance calculation. Queue fields, conditional macros, live loop-offset arguments and actual C request insertion remain unchanged.

Rear-rotor processing preserves its temporary sound position and vehicle-parameter distance/cache changes and restores the original position and parameter fields afterward. It deliberately retains the original scratch sound distance after rear processing. Cessna retains independent idle/rev ranges and request fields, including its later live range test after the first request.

PC camera snapshots have no attachment side effects, so their arithmetic uses plain C storage. The original PS2 CMatrix union copies attachment ownership with its basis; assignment can update the attachment and destruction can destroy it. A PS2-only bridge preserves that original assignment and passes cleanup state into the C generator, which performs the original destructor condition at every return. No new allocation or owner field is introduced.

The vanilla non-Skimmer helicopter rear-position expression discards its result and can subsequently read an uninitialized vector. The C conversion preserves that branch. Vanilla fixtures keep this existing undefined case outside its read domain; defined helicopter cases and the FIX_BUGS assignment are compared.

Verification:

- Complete generator bodies, all three histories, adapters, constants, macros, callers, declarations and temporary-position bookkeeping audited against original snapshots
- Full dot products and all three matrix-vector result terms audited against actual Vector.h/Vector.cpp arithmetic; matrix constructor, assignment and destructor conditions checked against Matrix.cpp
- All 48 earlier audio source audits pass alongside the aircraft audit, including migrated sound-position consumers
- 770 unique consumer compiler checks cover 77 sources across x64/x86, D3D9/software, threaded/single audio and WAV/all decoders
- Ten explicit preprocessor guards verify the intended thread and decoder combinations; the compiler harness preserves both single-thread and WAV-only include flags
- 24 Clang/GCC O0/O2 before/after comparisons cover default, vanilla, no reverb, no external audio, PS2 and disabled audio time scaling
- Six native MSVC x86/x64 comparisons cover default, no reverb and disabled time scaling; actual game audio consumers compile
- Each comparison exercises 32,768 operations, alternating owners, repeated calls and shared history, all rotor/model branches, changing pad inputs, positive/negative camera up values, camera-facing volumes, exact rotor/range boundaries, cached distance, engine state, fixed camera mode, captured versus live vehicles, changing backend loop arguments and partial queue writes
- Fixtures require long-distance, front, starting, propeller/rear and Cessna request setup; PS2 fixtures require assignment-update and cleanup callbacks
- Both variants link actual C math, geometry, request insertion and ordered sound queues; an independently optimized observer hashes the same complete records at every callback and operation boundary
- Clang undefined-behavior and float-cast-overflow sanitizer comparisons pass for desktop and PS2 branch fixtures
- The PS2 ownership bridge compiles against the actual 32-bit CMatrix declaration, with its 64-byte size checked

Snapshots and results are in build/audio-aircraft-c-tests. Run utils/tests/test_audio_aircraft.ps1 for source auditing, behavior comparisons and the desktop sanitizer fixture. Native, consumer, configuration-guard and supplemental PS2 scripts are in the build directory. Full game linking and in-game playback remain unverified. A complete PS2 game compilation is unavailable with the Windows toolchain because existing modules require MIPS assembly and PS2-specific audio definitions; the isolated matrix API check and branch fixtures do not establish a full PS2 build.

Plane/Jumbo routing and sound generators are the next related operations.
