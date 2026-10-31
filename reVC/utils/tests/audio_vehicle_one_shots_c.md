# Vehicle one-shot audio conversion

AudioVehicleOneShots.c owns the complete ProcessVehicleOneShots event loop as C11. The main C vehicle dispatcher calls it directly; the old member and AudioVehicleHost_OneShots trampoline are removed. The owning manager and parameter records retain their existing layouts. Visual Studio registers the new C11 source and shared AudioVehicleDoorTypes.h header; CMake and premake already discover them.

All 40 handled event labels and every conditional branch are retained, including GTA_TRAIN and THIS_IS_STUPID event reloads. The seven function-static histories remain shared across managers: wave counter 41, gun counter 53, waterfall deadline 0, helicopter counter 89, jump-wheel counter 82, tyre counter 91 and crunch offset 0. Waterfall gating and unsigned frame-counter wrap retain the original behavior.

The C event loop owns sample selection, event-local volume/frequency math, RNG calls, door sounds, engine-start eligibility, impact layers, gun selection, distance/cache handling, looping/static fields, reflection/reverb flags and stereo submission. Door-kind identifiers and private one-shot constants move into C-compatible storage without changing values, including the existing 60-volume bomb alias.

Existing adapters provide rotor speed, signed model index, live appearance and player vehicle lookup. New narrow game boundaries expose the audio-table door kind, first pad handbrake, captured driver, weapon-slot and drive-by predicates, selected weapon type and the remaining pedestrian-comment generator. The driver remains captured once in the FIX_BUGS weapon branch while later vehicle reads stay live. Pedestrian context initialization and persistence across events remain in C.

Splash processing still mutates the live event volume to 800 only after its minimum gate, even when distance later rejects the sound. The event loop reloads its entity index, event count and volumes at their original access points. The alternate event reload path remains distinct from the captured event. All per-event partial sound writes occur before their original range gate. Stereo gun setup still tests the original near-distance gate, submits the first channel, then changes live sample index, pan and shared gun counter for the second channel.

Verification:

- Complete original event body, all seven histories, 40 event labels, constants, door enum, macros, adapters, declarations, direct C caller and project registrations audited against original snapshots
- The audit delimits the complete function by its neighboring declaration because THIS_IS_STUPID has mutually exclusive opening braces that cannot be counted together
- All 51 earlier audio source audits pass; the earlier bomb-constant audit now follows its exact move into the C one-shot unit
- 820 unique compiler checks cover 82 production consumers across ten x64/x86, D3D9/software, threaded/single and WAV/all-decoder combinations
- Ten explicit preprocessor guards confirm intended thread and decoder combinations
- 32 Clang/GCC O0/O2 original-versus-C comparisons cover default, vanilla, no reverb, no external audio, PS2, disabled audio time scaling, train events and alternate event-table reloads
- Six native MSVC x86/x64 comparisons cover default, no reverb and disabled time scaling; actual game audio, plane and main C vehicle consumers compile
- Each new comparison exercises 16,384 complete event loops over alternating owners without resetting shared counters, every event label, all audio-table door types, live entity changes, cached and exact range boundaries, rotor gates, splash clamps, collision volumes, turret loops, captured drivers, SMG weapon choices, Hunter player/AI handbrake routing and repeated pedestrian comment contexts
- Fixtures require all sound families, stereo right channels, looping requests, splash clamps, changing event tables, pedestrian forwarding, real reflection recursion and desktop loop-offset queries
- Complete initialized manager, parameter, vehicle and pedestrian records are observed at callbacks and operation boundaries with normalized pointers
- Both variants link actual C random/math, geometry, request insertion, reflection recursion and ordered sound queues; a request wrapper observes real insertion and then applies fixture mutations
- Fixture inputs stay within the original defined floating-to-integer domains; existing invalid rotor/jump/turret input domains are not silently normalized by production code
- Undefined-behavior and float-cast-overflow sanitizer comparisons pass for desktop and isolated PS2 branches
- All 24 main vehicle dispatcher Clang/GCC comparisons and its sanitizer comparison pass after replacing its one-shot trampoline with the direct C call

Run utils/tests/test_audio_vehicle_one_shots.ps1 for source auditing, behavior comparisons and the desktop sanitizer check. Native, compiler matrix, mode guard and supplemental PS2 scripts and results are in build/audio-vehicle-one-shots-c-tests. Full game linking and in-game playback remain unverified. Isolated PS2 and train-event fixtures do not establish complete PS2 or train-enabled game builds with the Windows toolchain.

Conditional ProcessTrainNoise is the remaining vehicle generator to move into C before the pedestrian audio family.
