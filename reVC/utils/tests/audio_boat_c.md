# Boat audio conversion

AudioBoat.c owns the complete ProcessBoatEngine and ProcessBoatMovingOverWater generators as C11. Internal callers use explicit manager and parameter pointers. Visual Studio registers the C unit; CMake and premake already discover C sources.

Two typed adapters read the original boat and propeller water flags. Existing model, status, player-vehicle, pad, gas and brake adapters are reused. The engine captures its boat after the distance gate and retains that pointer for status, model and propeller reads while throttle expressions use the live parameter vehicle.

Engine behavior retains every supported model's base volume/frequency and modifiers, V12 selection, player versus AI throttle, repeated selected maximum queries, propeller frequency adjustment, unsigned frequency smoothing and volume smoothing. Function-static LastFreq=2000 and LastVol=0 remain shared across managers and boats; only player calls update them, including when distance attenuation prevents sound submission.

The main engine and Reefer idle requests retain separate gates and counters, signed entity-index remainder in the idle frequency, all sample and queue fields, emitting volume, live loop-offset arguments, reverb/reflection conditionals and actual C request insertion. Reefer's later model query remains separate from initial model selection.

Water movement retains absolute velocity, strict 0.0005 threshold, live boat-in-water check, 0.75 clamp, original multiplier and volume/frequency conversions, range and cached distance handling and complete request setup. No allocations or owner fields change.

Verification:

- Complete generator bodies, shared function-static history, adapters, private constants, conditional macros, callers and declarations audited against original snapshots
- Original minimum/maximum and absolute-value operations audited, with a volatile decision helper preserving conditional repeated throttle queries across optimizations
- All 47 earlier audio source audits pass alongside the new boat audit
- 750 unique compiler checks cover 75 sources across x64/x86, D3D9/software, threaded/single audio and WAV/all decoders, including the actual boat implementation
- 24 Clang/GCC O0/O2 before/after comparisons cover default, vanilla, no reverb, no external audio, PS2 and disabled audio time scaling
- Six native MSVC x86/x64 comparisons cover default, no reverb and disabled time scaling; actual game consumers and boat implementation compile
- Each comparison exercises 16,384 operations, alternating owners, repeated calls and shared history, every supported/unsupported model, wrecked gates, player/AI throttle and changing pad inputs, both water flags, velocity/distance boundaries, infinite/NaN water velocity, cached distance, signed idle-frequency inputs, captured boats versus live parameter changes and callbacks altering later model/loop arguments
- Reflection-capable fixtures require ordinary engine, water and Reefer idle requests to occur
- Both variants link actual C audio math, request insertion and ordered sound queues
- Clang undefined-behavior and float-cast-overflow sanitizer comparison passes

Snapshots and results are in build/audio-boat-c-tests. Run utils/tests/test_audio_boat.ps1 for source auditing, comparisons and sanitizer checking. Native and consumer compiler scripts are in the build directory. Full game linking and in-game playback remain unverified.

Cessna and car-helicopter audio are the next related operations.
