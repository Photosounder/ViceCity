# Main vehicle audio dispatcher conversion

AudioVehicle.c owns the entire ProcessVehicle dispatcher as C11, including parameter initialization, area eligibility, mall exceptions, velocity calculation, appearance and vehicle-type routing, sound generator ordering, captured car/bike velocity writes and final rain processing. The physical dispatcher now invokes AudioVehicle_Process directly; the old C++ AudioDispatchHost_Vehicle trampoline and member declaration are removed. cAudioManager storage is unchanged.

The original eVehicleAppearance enum is shared in modelinfo/VehicleAppearance.h. All values and existing C++ consumers remain unchanged. Visual Studio registers both the new C11 source and enum header; CMake and premake already discover these files.

Existing adapters provide area, player/attachment lookup, position, signed model index, status and wet-road state. New typed boundaries expose the current handling pointer, selected transmission address, traffic speed, captured vehicle type, live appearance, velocity vectors and final automobile/bike velocity writes. The remaining C++ ProcessVehicleOneShots generator is called through a narrow parameter-pointer adapter.

The dispatcher retains the initial player exemption before unsupported-area rejection, current-area recheck after parameter setup, the later live mall player lookup, and mall's one-shot/siren/damage order and early return. It captures vehicle type before routing but retains repeated live model comparisons. It selects the transmission through the original conditional handling-pointer reads, converts the signed world model to the unsigned audio-table index at the original point, and retains the exact status gate for simple traffic.

Velocity math moves into C with every original dot-product term and operation order. The game boundary resolves the forward reference before copying the current movement and direction vectors. The final velocity write uses the original captured automobile or bike even when called generators change the parameter record's vehicle pointer, type or velocity. Skimmer, remote-control cars and helicopters, car appearances, boats, aircraft, bikes, ordinary road gates and unsupported types retain their original routing order. Wet-road checks remain live after preceding generators.

Verification:

- Complete dispatcher, vector arithmetic, enum extraction, adapters, declarations, direct C physical caller and project registrations audited against original snapshots
- Velocity dot-product terms derived from actual Vector.h, and forward-reference behavior checked against Placeable.h
- All 50 earlier audio source audits pass, including the migrated sound-position consumer and shared vehicle enum changes
- 810 unique compiler checks cover 81 production consumers across ten x64/x86, D3D9/software, threaded/single and WAV/all-decoder combinations, including actual Bike.cpp and Vehicle.cpp
- Ten independent preprocessor guards confirm the intended thread and decoder combinations
- 24 Clang/GCC O0/O2 before/after comparisons cover default, vanilla, no reverb, no external audio, PS2 and disabled audio time scaling
- Six native MSVC x86/x64 comparisons cover default, no reverb and disabled time scaling; actual game audio, plane, bike and vehicle consumers compile
- Each new comparison exercises 32,768 complete dispatcher calls over all areas, vehicle types, appearances, remote-control and Skimmer model branches, null/non-null handling, simple/physical velocity, alternating owners, early returns and changing game queries
- Generator stand-ins observe the original routing boundaries, mutate parameters and scratch state, and exercise dry-road and warning-predicate outcomes; their sound-generation internals are covered by the earlier independent conversion fixtures
- Every routing boundary, both captured velocity-write paths, mall/area early returns and parameter mutation paths are required
- Full initialized owner and vehicle records are compared with normalized pointers; automatic parameter records are compared field by field without reading unspecified padding
- Actual C geometry is linked into original and converted dispatcher fixtures
- Undefined-behavior and float-cast-overflow sanitizer comparisons pass for desktop and isolated PS2 branches
- The existing physical/entity dispatcher is rerun after removing its vehicle trampoline: all 20 Clang/GCC comparisons and its sanitizer comparison pass

Run utils/tests/test_audio_vehicle.ps1 for source auditing, behavior comparisons and desktop sanitizer checks. Compiler matrix, native, mode guard and supplemental PS2 sanitizer scripts and results are in build/audio-vehicle-c-tests. Full game linking and in-game playback remain unverified; the isolated PS2 fixture does not establish a complete PS2 game build with the Windows toolchain.

ProcessVehicleOneShots is the next remaining vehicle audio operation. Its event loop, shared sample counters and event-specific setup remain in C++.
