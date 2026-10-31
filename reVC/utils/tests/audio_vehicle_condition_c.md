# Engine-damage and car-bomb ticking audio conversion

AudioVehicleCondition.c owns the complete ProcessEngineDamage and ProcessCarBombTick generators as C11. All original internal callers use explicit manager and vehicle-parameter pointers. Visual Studio registers the C unit and VehicleBombTypes.h; CMake and premake already discover C sources.

VehicleBombTypes.h shares the original eBombType enum with unchanged values. Vehicle.h includes that header without changing vehicle fields or layout. Existing C-facing model, status, engine and drowning queries are reused. New typed adapters read health and the three-bit bomb field and preserve the original unsupported vehicle diagnostic.

Engine damage retains its distance gate, caddy exclusion, wrecked/health short circuit, separate health reads, strict 250 threshold, fire versus smoke sample/frequency selection, drowning attenuation and all sound-request fields. Each vehicle read resolves the live parameter pointer at its original position, including drowning after the fire sample backend query.

Bomb ticking retains engine gating, separate car/bike switch branches, unsupported-type diagnostic and early return, timed-active bomb selection, cached distance, countdown sample and frequency, loop offsets and complete request setup. Both generators preserve conditional emitting/reverb/reflection behavior and original return values.

Private distance constants move into C. The bomb volume remains 60 in both units because the C++ vehicle one-shot generator still uses its original alias. No allocations or stored owner fields change.

Verification:

- Complete generator bodies, adapters, private and shared constants, conditional macros, internal callers and declarations audited against snapshots
- Full bomb enum and vehicle header extraction audited against original source
- All 46 earlier audio source audits pass alongside this conversion audit
- 730 unique compiler checks cover 73 sources across x64/x86, D3D9/software, threaded/single audio and WAV/all decoders
- 24 Clang/GCC O0/O2 before/after comparisons cover default, vanilla, no reverb, no external audio, PS2 and disabled audio time scaling
- Six native MSVC x86/x64 comparisons cover default, no reverb and disabled time scaling; actual game consumers and vehicle implementations compile
- Each comparison exercises 16,384 operations, repeated calls, alternating owners, health values immediately around 250 and 390, NaN/infinite health, model/status exclusions, all eight representable bomb-field values, supported/unsupported vehicle types including signed sentinels, exact distance boundaries, cached distance, live vehicle changes and backend changes before attenuation and between loop-offset queries
- Both variants link actual C audio math, request insertion and ordered sound queues
- Clang undefined-behavior and float-cast-overflow sanitizer comparison passes

Snapshots and results are in build/audio-vehicle-condition-c-tests. Run utils/tests/test_audio_vehicle_condition.ps1 for source auditing, comparisons and sanitizer checking. Native and consumer compiler scripts are in the build directory. Full game linking and in-game playback remain unverified.

Boat engine and water-movement audio are the next related operations.
