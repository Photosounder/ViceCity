# Player engine state machine conversion

AudioPlayerEngine.c owns the complete ProcessPlayersVehicleEngine operation as C11, plus PlayerJustGotInCar and the original empty PlayerJustLeftCar notification. The main vehicle generator calls the C operation directly; its former C++ player trampoline is removed. Ped and script notifications call the explicit C owner operations.

The original GearFreqAdj array and LastAccel, CurrentPretendGear, lost-traction/handbrake history, cruising count and acceleration-stop history remain shared function-static state. The entry flag is C storage. The gear timer uses zero-initialized C storage and a first-call guard, querying the original timer at the same point as the former dynamic C++ static initialization. This preserves the serialized audio-service lifetime, including initialization before the first entry reset or unsupported-type return.

The whole player state machine now runs in C: replay/exit controls and short circuits, model and transmission reads, captured car/bike wheel and gas-audio fields, drivetrain traction checks, vertical-speed modifier, all bank/gear lengths, unsigned timer subtraction, cruising and goto transition, gear-changing mutations, acceleration-channel initialization failure, PS2 surround/player channel distinctions, camera translation, pan/position/distance, optional frequency time scaling, reverb/start operations, slowdown and finger-off/idle sounds, and final history updates. The veh argument remains unused, as in the original.

Small game adapters expose the remaining class fields, timer, exiting-ped query and gear-change mutation. Existing C geometry, math, sound requests, ordered queue and player sample helpers perform their original operations. Floating clamp/min/max comparisons retain conditional repeated reads. The original signed/unsigned frequency expressions, operator precedence, live parameter aliases and live loop-offset arguments remain intact.

Verification:

- Complete state-machine body, first-call timer substitution, entry gate, exit no-op, shared constants, ped/script callers, owning declarations and direct C handoff audited against snapshots
- All 41 earlier audio source audits pass alongside the new state-machine audit; the old sound-position audit follows its migrated player consumer into C
- 650 unique consumer compile checks cover 65 sources across x64/x86, D3D9/software, threaded/single audio and WAV/all decoders, including actual Ped.cpp and Script.cpp consumers
- 24 Clang/GCC O0/O2 before/after comparisons cover default, vanilla, no reverb, no external audio, PS2 and disabled time scaling
- Six native MSVC x86/x64 before/after comparisons cover default, no reverb and disabled time scaling; actual game audio, ped, script and physical callers compile
- 8,192 engine operations per comparison plus entry/exit notifications exercise repeated calls, alternating owners, shared history, initialized/uninitialized entry gates, replay/exit/pad controls, byte input boundaries, car/bike/model/drivetrain/wheel states, unsupported types, unsigned timer wrap, gear timing, channel initialization success/failure, channel start/stop, cruising and release/idle sounds, drowning, extra bike speed, sand, bank variants, geometry callback mutations and loop-offset changes
- Actual C geometry, math, player samples, request insertion and ordered queues are linked in both variants
- Clang undefined-behavior and float-cast-overflow sanitizer comparison passes

Clang/GCC fixture wrappers observe floating field reads; native MSVC treats those fields as pure reads to avoid its coalescing of user-defined floating conversions. All variants observe resulting owner/vehicle/parameter/backend state and other game/audio queries. Shared history is checked through subsequent behavior across repeated calls and owners.

Snapshots and results are in build/audio-player-engine-c-tests. Run utils/tests/test_audio_player_engine.ps1 for all six shapes and sanitizer verification. Native and consumer scripts are in that build directory. The first recorded run covers five shapes; no-time-runs.json records the additional disabled-time-scaling comparisons. Full game linking and in-game playback remain unverified.

The next vehicle audio operations still implemented as C++ members include skidding, horns and sirens/alarms, air brakes, damage and doors. Skidding is the next logical conversion because it uses the same captured wheel arrays, gas-audio storage, transmission limits and vehicle-type enums already exposed here.
