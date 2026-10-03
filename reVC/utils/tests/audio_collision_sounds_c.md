# Collision sound generators conversion

AudioCollisionSounds.c owns the former SetUpOneShotCollisionSound, SetUpLoopingCollisionSound and SetLoopingCollisionRequestedSfxFreqAndGetVol members as C11. The one-shot sample table and conditional loop/emitting-volume/reverb/reflection assignments move unchanged into that unit. Collision service calls both generators directly, removing the two C++ generator adapters. AudioCollision.cpp now retains only building and position adapters used by collision reporting.

The conversion preserves the shared static one-shot counter, random displacement history, surface-selection precedence, car/ped attenuation and car-panel substitution, integer narrowing and signed volume arithmetic, nonpositive distance clamping, cached looping distance, sample variation, backend frequency/loop query order, and all request fields. Collision records are passed by pointer so callback mutations remain visible at the same points. Owner and record layouts remain unchanged; no allocations are added. Visual Studio explicitly compiles the new file as C; CMake and premake already discover C sources.

Verification:

- Complete generator bodies, one-shot surface table, conditional macros, owner/header edits and service calls audited against saved original sources
- All 35 current and earlier audio source audits pass
- 20 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no reverb, no external audio and PS2 shapes
- Four native MSVC x86/x64 comparisons across default/no-reverb shapes, plus actual game audio and physical caller compilation
- 39,200 operations per comparison cover all 35-by-35 surface pairs, both owners, all three generators, integrated collision service, actual C callers, counter wraparound, threshold and volume branches, signed base volumes, varied random tables, distances around attenuation boundaries and empty/partial/full request queues
- Actual C collision initialization/insertion, collision math, audio math, request submission and ordered sound queue modules are linked in both implementations
- Complete zero-padded owner and collision bytes are observed at backend/request callbacks and after each operation
- Callbacks mutate shared collision surfaces, distance and intensity, random-table state, and the sample index between loop start/end queries to expose copies or cached reads
- 560 unique consumer compilation checks pass across architecture, renderer, threading and decoder configurations
- All 20 Clang/GCC collision-service regressions and their sanitizer comparison retain prior digests
- An additional Clang O1 before/after comparison with undefined-behavior and float-conversion checks in trap mode passes

The fixture uses finite intensities and bounded signed base volumes to exercise defined behavior; it does not broaden the original generator input contract. Full game linking and playback were not tested. Backend responses and game room status are controlled while actual C request insertion runs.
