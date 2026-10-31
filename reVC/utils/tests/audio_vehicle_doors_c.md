# Vehicle door audio conversion

AudioVehicleDoors.c owns the complete ProcessVehicleDoors generator as C11. ProcessVehicle calls it with explicit manager and parameter pointers. Visual Studio registers the C unit and shared DoorTypes.h header; CMake and premake already discover C sources.

DoorTypes.h shares the original eDoorState and eDoorStatus enums without changing values. VEHICLE_DOOR_COUNT remains six and supplies the automobile array bound and C loop bound. Door.h and DamageManager.h include the shared declarations. Automobile and damage storage retain their original layout.

Three typed game adapters query the captured automobile's damage manager, signed door state and angular velocity. The C generator retains distance calculation before iteration, original door order, swinging-damage and open/closed gates, repeated angular-velocity reads from the Min expression, absolute value and cap, the strict 0.0035 threshold, volume conversion, signed random-table sample selection and shared random displacement history.

Sound requests retain their original counter, frequency, queue writes, conditional macros, one-shot loop offsets, unchanged frames-to-play field and reflection-enabled submission. Both runtime variants link the actual C request, reflection and sound-order implementations. No allocations or owner fields change.

Verification:

- Complete generator body, constants, conditional macros, adapters, callers and declarations audited against snapshots
- Exact shared enum values and six-door array bound audited against original headers
- All 45 earlier audio source audits pass alongside the door audit
- 720 unique compiler checks cover 72 sources across x64/x86, D3D9/software, threaded/single audio and WAV/all decoders, including the actual automobile, door and damage implementations
- 24 Clang/GCC O0/O2 before/after comparisons cover default, vanilla, no reverb, no external audio, PS2 and disabled audio time scaling
- Six native MSVC x86/x64 comparisons cover default, no reverb and disabled time scaling; actual game consumers and changed header owners compile
- Each comparison covers 16,384 complete generator operations, repeated calls, alternating owners, all six doors, signed states, damage gates, exact velocity and distance boundaries, NaN/infinite angular velocities, cached distance, captured automobile versus changed live vehicle pointers, callback changes to later doors, signed random input, queue saturation and recursive reflections
- Reflection-capable fixtures require reflected requests to occur
- Clang undefined-behavior and float-cast-overflow sanitizer comparisons pass with time scaling enabled and disabled

Snapshots and results are in build/audio-vehicle-doors-c-tests. Run utils/tests/test_audio_vehicle_doors.ps1 for source auditing, comparisons and the default sanitizer fixture. Native, consumer and supplemental disabled-time-scaling sanitizer scripts are in the build directory. Full game linking and in-game playback remain unverified.

Engine damage and car-bomb ticking are the next related vehicle sound operations.
