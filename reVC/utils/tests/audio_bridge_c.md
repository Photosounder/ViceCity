# Bridge audio conversion

`src/audio/AudioBridge.c` owns the complete former `ProcessBridge`, `ProcessBridgeWarning`, `ProcessBridgeMotor` and `ProcessBridgeOneShots` operations as C11. The C dispatcher enters the state machine directly. Four typed adapters retain the original lift-road pointer, current state, previous state and story-progress reads. The original state enum now lives in `src/control/BridgeStates.h`; bridge and audio-manager fields are unchanged. The Visual Studio project registers both new files, and the existing CMake/Premake globs discover them.

The conversion preserves the separate presence/position pointer reads, strict squared-distance gate, warning/motor ordering and unconditional transition check inside the distance gate. One-shot conditions retain each repeated current-state read and short-circuit previous-state read. Warning and transition sounds clear reflections explicitly; the motor still leaves that field untouched. Cached distances, samples, bank indices, frequencies, loop arguments, conditional fields and request insertion order are retained.

`GTA_BRIDGE` remains disabled in normal builds. The original optional branch cannot compile against the VC sample/model definitions: `SFX_BRIDGE_OPEN_WARNING`, `MI_BRIDGELIFT`, `MI_BRIDGEROADSEGMENT` and `MI_BRIDGEWEIGHT` are absent. These missing identifiers are preserved rather than assigned guessed production values. `build/audio-bridge-c-tests/original-unsupported.log` records the original audio body's missing-sample errors. Enabled audio comparisons explicitly define the missing sample as `SFX_REVERSE_WARNING` **only in test configuration headers**. This checks the complete generators' C/C++ equivalence, not a working bridge asset mapping or an enabled full-game build.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_bridge.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_bridge_ps2.ps1
```

The source audit compares every statement in all four original operations, the adapters, enum, constants/macros, full owner/header/caller edits and project registration. Runtime comparisons use complete unchanged original C++ bodies or actual separately compiled C11 functions, with identical observers and real C math, geometry, request insertion and ordering.

Each comparison performs 12,288 groups: 1,024 seeds, four separately callable operations and three passes. Two passes exercise live mutations and repeated calls; a fresh pass retains seeded distance boundaries. Inputs cover both owners, all six current/previous states, other integer state values, zero/positive/negative story flags, null lift roads, strict warning/motor distance boundaries, NaN/infinite positions, seeded conditional fields and queue replacement/rejection.

Callbacks redirect the road between presence and position queries, change positions, alter previous/current states between short-circuit conditions, change scratch coordinates during camera queries, replace loop samples, and change cached distance and bridge transitions during submission. The observer hashes both complete initialized owners and all initialized bridge records at callbacks and after groups. Road pointers use stable identities; record padding starts at zero. A non-null presence read always retains a valid road for the following position read. Cached distances used in volume calculations are nonnegative. Disabled-feature comparisons require zero callbacks and submissions.

Verified results in `build/audio-bridge-c-tests`:

- 32 matching Clang/GCC comparisons across two optimization levels and eight feature layouts, including the disabled feature
- Six matching native MSVC comparisons across x86/x64 and three enabled audio layouts
- Matching desktop and PS2 sanitizer comparisons
- 1,040 actual production consumer compile checks with the bridge feature disabled
- 40 focused enabled audio consumer checks using the fixture-only sample definition
- 50 focused disabled-feature consumer checks and 10 configuration guards
- All 65 preceding migration source audits
- 20 matching dispatcher regressions and its sanitizer comparison

The production consumer matrix covers D3D9/software rendering, x86/x64, threaded/single audio and WAV/all decoders with `GTA_BRIDGE` disabled. Focused enabled audio consumers use the documented fixture-only sample definition and exclude the unsupported enabled bridge-model code. No full game link, gameplay test or full PS2 game build was performed. The next audio step is moving game-specific setup/termination out of `AudioLogic.cpp` into C.
