# Vehicle skidding audio conversion

AudioVehicleSkid.c owns the complete ProcessVehicleSkidding generator and the former GetVehicleDriveWheelSkidValue and GetVehicleNonDriveWheelSkidValue methods as C11. The wheel-value helpers are file-local C functions because the generator was their only caller. Car and bike routing calls the explicit C generator; all three former owner methods and declarations are removed.

The generator preserves initial vehicle capture, wheel-array capture, total ground-contact count and copied gas-audio value. It retains the per-non-normal-wheel transmission capture, drivetrain selection using the original car wheel indices on both types, drive/non-drive spinning/skidding/fixed formulas, repeated conditional transmission and turn-speed reads, maximum aggregation, stale newSkidVal on unknown drive types, distance calculation and return values.

Surface selection still reads the live parameter vehicle after aggregation. Grass and hedge retain the rain sample, quartered emitting/attenuated volume and early return after a zero result; gravel/mud/sand/water/beach retain the gravel sample; other surfaces retain the default skid sample and bike frequency adjustment. Sample counter, bank, priority, looping, live loop-offset arguments, speed, distance, static/frame/reverb/reflection fields and actual C request submission remain unchanged.

Four narrow game adapters copy car/bike gas-audio values, read turn-speed Z and preserve the original unsupported-type diagnostic. Existing wheel-state, total-contact, transmission and surface adapters are reused. Private skid constants move into C. No allocations or stored owner fields change. Visual Studio registers the C11 unit; CMake and premake already discover C sources.

Verification:

- Complete generator and both helper bodies, private constants, conditional macros, owning declarations, game adapters and internal callers audited against saved originals
- All 42 earlier audio source audits pass alongside the new skid audit
- 660 unique consumer compile checks cover 66 sources across x64/x86, D3D9/software, threaded/single audio and WAV/all decoders
- 20 Clang/GCC O0/O2 before/after comparisons cover default, vanilla, no reverb, no external audio and PS2
- Four native MSVC x86/x64 before/after comparisons cover default and no reverb; actual game audio and physical consumers compile
- 8,192 operations per comparison exercise both owners, cars/bikes, repeated calls, all wheel states, each drivetrain and unknown types, spinning and fixed-wheel gas thresholds, velocity thresholds/signs, clamped turn-speed contributions, distance and cached-distance boundaries, all surface branches and grass-zero return, live parameter vehicle/transmission changes after per-wheel capture, live loop-offset arguments and no-transmission all-normal/airborne guards
- Actual C audio math, request insertion and ordered sound queue units are linked in both variants
- Clang undefined-behavior and float-cast-overflow sanitizer comparison passes

Clang/GCC fixture wrappers observe repeated floating reads. Native MSVC treats floating fields as pure reads to avoid coalescing of user-defined floating conversions in conditional expressions. Every variant observes complete resulting owner/vehicle/parameter/transmission state and other game/audio queries. The C extrema use a comparison barrier to preserve conditional repeated reads.

Snapshots and results are in build/audio-vehicle-skid-c-tests. Run utils/tests/test_audio_vehicle_skid.ps1 for the source audit, comparisons and sanitizer fixture. Native and consumer compile scripts are in that build directory. Full game linking and in-game playback remain unverified.

The next vehicle sound routines are horns and sirens/alarms. They still use the shared C++ vehicle sound settings table and small vehicle timer/counter fields; those dependencies should be inspected before their complete generators move into C.
