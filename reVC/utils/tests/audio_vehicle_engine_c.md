# Main vehicle engine audio conversion

AudioVehicleEngine.c owns the complete former ProcessVehicleEngine member as C11. The car and bike routing paths call it with explicit manager and parameter pointers. Gear selection, drivetrain traction, airborne gas-audio decay, modifier arithmetic, volume and frequency selection, drowning attenuation, golf-cart sounds and request setup now run in C.

Small C++ adapters read the original class fields and shared vehicle sound settings. The player branch still forwards to ProcessPlayersVehicleEngine with the original owner, parameter and vehicle aliases. That larger player-engine operation and UpdateGasPedalAudio remain candidates for subsequent conversion. No allocations or stored owner fields change.

EntityStatus.h, WheelState.h and WheelIndices.h expose the original enum blocks to both languages. Entity, vehicle, automobile and bike headers include them without changing values or field types. The Visual Studio project registers the new C11 unit and shared headers; CMake and premake already discover C sources.

The original boolean absolute-value expression for mopeds and golf carts, handbrake disjunction, minimum-before-division arithmetic, signed entity-frequency variation and vanilla uninitialized frequency branch are retained. The null-transmission fixture uses the idle branch to keep the vanilla baseline defined. Distance, queue, loop, reflection and reverb behavior retain their original feature guards. Shared engine constants remain in AudioLogic.cpp for the remaining player-engine code and are copied unchanged into C.

Verification:

- Complete generator statements, all game adapters, internal callers, owner declarations, shared enum blocks and conditional queue macros audited against saved originals
- All 39 earlier source audits pass alongside the new engine audit
- 610 consumer compile checks cover x64/x86, D3D9/software rendering, threaded/single audio and WAV/all decoder configurations
- 20 Clang/GCC O0/O2 before/after comparisons cover default, vanilla, no reverb, no external audio and PS2 shapes
- Four native MSVC x86/x64 before/after comparisons cover default and no reverb, with the actual C++ game adapters and callers compiled
- 32,768 operations per comparison cover repeated calls, both owner instances, cars and bikes, player handoff and wrecked-player channel stops, supported and unsupported types, models, gears, drivetrain and wheel states, cached distances and range boundaries, drowning, null/zero-velocity transmissions, golf-cart samples and live loop-offset sample arguments
- Actual C audio math, request insertion and sound queue units are linked in both variants
- Clang undefined-behavior and float-cast-overflow sanitizer comparison passes

Clang and GCC fixture field wrappers observe repeated floating reads. MSVC coalesces user-defined floating conversions in minimum expressions, so the native fixture treats floating fields as pure reads, matching the actual game's float fields. Native comparisons still observe other queries, owner/parameter aliases, audio backend operations and complete resulting state. The production C minimum uses a comparison barrier to retain conditional repeated transmission accesses.

Snapshots and results are in build/audio-vehicle-engine-c-tests. Run utils/tests/test_audio_vehicle_engine.ps1 for the source audit, 20 comparisons and sanitizer fixture. Native and consumer compiler scripts are in that build directory. Full game linking and in-game playback have not been verified.
