# RC model vehicle audio conversion

AudioVehicleModel.c owns the former ProcessModelHeliVehicle and ProcessModelVehicle members as C11. Vehicle processing calls the explicit manager/parameter operations. Model-specific sound selection, controls comparison, volume/frequency clamping, orientation compensation, drowning attenuation and request setup now run in C. Existing player/remote and vehicle gas/drive-wheel adapters are reused. Narrow new adapters retain direct model/brake/drowning fields, primary pad queries and up-vector access in AudioLogic.cpp.

Both function-static frequency histories and the car's function-static LastVol byte remain shared across owners with their original initialization and update conditions. Helicopter history updates only for player vehicles. RC-bandit car history updates after its branch even without a queued sound. Other car history updates only for player slowdown; active player acceleration retains its original behavior. No reset or owner-specific history fields are added.

The conversion preserves strict range gates, player-before-remote lookup, FIX_BUGS remote identity comparison and vanilla any-remote-pointer behavior, helicopter brake-before-accelerator/gun order, repeated controller queries through Max, non-player gas-before-brake reads, signed input narrowing, original integer arithmetic and signed/unsigned Clamp2 operations, constant or live loop offset arguments and all request fields. A local volatile comparison byte preserves conditional repeated pad queries under MSVC's floating maximum optimization. The alternate THIS_IS_STUPID branch copies the full up vector and performs the original three C dot-product terms; the normal branch reads Y only.

Private model constants move into the C unit. The unused original model-car volume constant is retained with its original value. Owner, vehicle and parameter layouts remain unchanged; no allocations are added. Visual Studio registers the C unit; CMake and premake already discover C sources.

Verification:

- Complete generator bodies, game adapters, three constants, conditional sound macros and internal/owner/header edits audited against saved originals
- Original Min/Max/Clamp2 and dot-product definitions checked against the game sources
- All 39 current and earlier audio source audits pass
- 24 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no reverb, no external audio, PS2 and alternate dot-product shapes
- Six native MSVC x86/x64 before/after comparisons across default, no reverb and alternate dot-product configurations
- Actual game audio and physical callers compile; the actual C++ game model adapters also compile with THIS_IS_STUPID on both native architectures
- 32,768 operations per comparison exercise both model generators, both owners, repeated calls and shared history, player/non-player and focus/remote paths, RC-bandit/generic models, grounded/airborne RC behavior, slowdown/acceleration, drowning/orientation compensation, distance boundaries/cached flags and empty/partial/full request queues
- Actual C audio math, request insertion and ordered sound queue units are linked in both variants
- Complete initialized owner, parameter, vehicle and pad records are observed with normalized vehicle identities, plus all query tokens
- Player lookup changes later remote selection; model/gas queries change the parameter vehicle before later live accesses; accelerator queries change their own value to expose repeated reads; start-offset queries change the live sample index
- 600 unique consumer compilation checks pass across architecture, D3D9/software, threading and decoder configurations
- An additional Clang O1 comparison with undefined-behavior and floating-conversion checks in trap mode passes

Full game linking and playback were not tested. Game/control queries use controlled scalar wrappers and accepted numeric inputs remain within the original defined conversion ranges.
