# Reverse warning and air-brake audio conversion

AudioVehicleWarning.c owns the complete reverse-warning and air-brake generators and their two model-index predicates as C11. ProcessVehicle calls the C functions with explicit manager and vehicle-parameter pointers. Visual Studio registers the new C unit; CMake and premake already discover C sources.

Existing vehicle adapters supply engine state, gas pedal, drowning state and stored car audio velocity. No new C++ adapters, allocations or owner fields are required. The shared AudioVehicleModels.h enum supplies the original audio-table indices.

Reverse warnings preserve vehicle capture before the distance gate, engine/gas short circuits, signed entity-index remainder in frequency selection, drowning attenuation, conditional emitting volume, live loop-offset queries and queue writes. Air brakes preserve vehicle capture after the distance gate, the asymmetric positive and negative velocity tests, signed random-table volume calculation, shared random displacement history and the original lack of a frames-to-play assignment.

Verification:

- Complete generator and predicate bodies, private constants, conditional macros, owning declarations and internal callers audited against saved source
- All 44 earlier audio source audits pass alongside this conversion audit
- 680 unique consumer compile checks cover 68 sources across x64/x86, D3D9/software, threaded/single audio and WAV/all decoders
- 20 Clang/GCC comparisons at O0/O2 cover default, vanilla, no reverb, no external audio and PS2
- Four native MSVC x86/x64 comparisons cover default and no reverb; actual game audio consumers compile
- Each comparison exercises 16,384 generator operations, exact velocity and distance boundaries, repeated calls, alternating owners, captured versus live vehicle pointers, signed frequency and random inputs, drowning, cached distance and backend callbacks changing live sample and random state
- Both variants link actual C audio math, request insertion and ordered sound queues
- Clang undefined-behavior and float-cast-overflow sanitizer comparison passes

Snapshots and results are in build/audio-vehicle-warning-c-tests. Run utils/tests/test_audio_vehicle_warning.ps1 for source auditing, comparisons and sanitizer checking. Native and consumer compiler scripts are in the build directory. Full game linking and in-game playback remain unverified.

Vehicle door audio is the next related conversion.
