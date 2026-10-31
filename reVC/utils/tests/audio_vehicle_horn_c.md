# Vehicle horn and siren audio conversion

AudioVehicleHorn.c owns ProcessVehicleHorn, ProcessVehicleSirenOrAlarm and the former UsesSiren/UsesSirenSwitching audio members as C11. The two predicates are file-local C helpers because only the generators called them. Boat, car and bike routing now call the C generators with explicit manager and parameter pointers.

All 352 bits of the original 8×44 const horn pattern move into C unchanged. AudioVehicleModels.h exposes the original eVehicleModel audio-table indices, MAX_CARS and DODO value to both languages. These indices remain distinct from world model IDs. The shared vehicle sound settings table stays in C++ for its remaining consumers and is accessed through typed getters for horn/siren sample and frequency fields.

The horn generator retains siren-switching exclusions, the MRWHOOP model exclusion, alarm checks, positive-timer gate, non-player cap at 44, frame/entity pattern selection, pattern suppression, distance attenuation, drowning volume and the FIX_BUGS difference in emitting volume. The clamped timer is computed before resolving the original live assignment target.

The siren/alarm generator retains the captured initial vehicle, enabled/alarm short circuits, repeated alarm and clock queries, unsigned timer updates and pulse suppression, live siren/status queries and abandoned return, fast-siren counter and sample, FBIRANCH/VICECHEE frequency exceptions, table selection and horn fallback. Queue fields, conditional emitting/reverb/reflection macros, live loop-offset arguments and actual C request submission are preserved.

Small game adapters call the existing CVehicle alarm/siren predicates and access original timer/pattern fields or shared sound settings. Existing model, status, drowning and clock adapters are reused. No allocations or stored owner fields change. Visual Studio registers the new C11 unit and shared index header; CMake and premake already discover C sources.

Verification:

- Complete generator and predicate bodies, private constants, every pattern bit, full shared index enum, conditional macros, internal callers and owning declarations audited against snapshots
- All 43 earlier audio source audits pass alongside the new horn/siren audit
- 670 unique consumer compile checks cover 67 sources across x64/x86, D3D9/software, threaded/single audio and WAV/all decoders
- 20 Clang/GCC O0/O2 before/after comparisons cover default, vanilla, no reverb, no external audio and PS2
- Four native MSVC x86/x64 before/after comparisons cover default and no reverb; actual game audio and physical callers compile
- 16,384 operations per comparison exercise both owners and generators, repeated calls, pattern rows and timer positions, zero/44/45/maximum timers, player/non-player/abandoned/wrecked states, model and audio-index exclusions, alarm-state short circuits, separate clock reads and unsigned wrap, drowning, live parameter changes versus captured siren vehicle, all special sample/frequency branches, cached/range distance boundaries, and callback changes to live loop-offset arguments
- Actual C audio math, request insertion and ordered sound queues are linked in both variants
- Clang undefined-behavior and float-cast-overflow sanitizer comparison passes

Snapshots and results are in build/audio-vehicle-horn-c-tests. Run utils/tests/test_audio_vehicle_horn.ps1 for the source audit, comparisons and sanitizer fixture. Native and consumer compiler scripts are in that build directory. Full game linking and in-game playback remain unverified.

The next related operations are reverse-warning sounds and air brakes. Their model-index predicates can use the shared C-compatible audio index enum introduced here.
