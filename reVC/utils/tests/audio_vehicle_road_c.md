# Vehicle tyre and road-noise conversion

AudioVehicleRoad.c owns the former ProcessVehicleFlatTyre, ProcessVehicleRoadNoise and ProcessWetRoadNoise members as C11. ProcessVehicle calls the three explicit manager/parameter operations. Six adapters in AudioLogic.cpp retain individual wheel status/timer, ground-contact, transmission maximum velocity, surface and wetness reads. Wheel scanning, sound selection, volume/frequency calculations, queue setup and in-range return values now run in C.

The original eVehicleType and eWheelStatus enums are extracted unchanged into VehicleTypes.h and WheelStatus.h. Their owning C++ headers include the shared C headers, preserving enum names, values and object layouts. Visual Studio registers the C unit and both shared headers; CMake and premake already discover C sources.

Preserved behavior includes strict distance gates, every car/bike wheel status query, timer queries only for burst wheels, captured vehicle pointers during wheel loops, default vehicle type behavior, no added transmission gate in flat-tyre generation, dry/wet transmission null gates, cached distance for all nonzero flag bytes, signed velocity absolute values, surface/wetness precedence, live sample-index reads between loop offset queries, constant flat-tyre loop offset arguments and all queued fields. Each operation still returns true whenever its distance gate passes, even if it produces no sound.

The minimum expression deliberately retains conditional repeated transmission reads. MSVC's C optimizer collapsed the ordinary floating minimum expression into one read while the original C++ expression retained two. A small comparison helper with a volatile local byte prevents that collapse and preserves the original branch/query behavior. No game data or long-lived state becomes volatile, and no allocations are added.

Verification:

- Complete generator bodies, game adapters, six private constants, conditional sound macros, internal callers and owner/header changes audited against saved sources
- Shared enum contents and their original C++ include substitutions audited verbatim
- All 37 current and earlier audio source audits pass
- 20 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no reverb, no external audio and PS2 shapes
- 580 unique consumer compilation checks pass across architecture, D3D9/software, threading and decoder configurations
- An additional Clang O1 before/after comparison with undefined-behavior and floating-conversion checks in trap mode passes
- Four native MSVC x86/x64 before/after comparisons across default/no-reverb, plus compilation of actual game audio and physical callers
- 24,576 operations per comparison cover both owners, all three generators, cars/bikes/boats/unknown vehicle types, wheel status/contact combinations, velocity/modifier thresholds, zero transmission maximum velocity, dry/wet null transmission gates, distance boundaries, cached distance flag bytes, water/tarmac surfaces, wetness levels and empty/partial/full requested queues
- Actual C audio math, request insertion and ordered sound queue units are linked in both variants
- All 20 Clang/GCC rain-audio regressions and their sanitizer comparison retain prior digests
- Complete zero-padded owner, parameter and controlled game records are observed with normalized pointer identities, plus return values and all query tokens
- Wheel queries change the live parameter vehicle while scans retain their original captured vehicle; transmission reads change the live parameter transmission; backend frequency/start-offset queries change the live sample index before later reads

Full game linking and playback were not tested. Game fields are represented by controlled scalar wrappers; the fixture bounds callback-mutated transmission values to retain defined numeric conversions. NaN tyre velocity is excluded from accepted tyre paths; dry/wet NaN velocity rejection is tested.
