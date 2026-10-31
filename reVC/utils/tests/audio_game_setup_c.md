# Game-specific audio setup and shutdown conversion

`src/audio/AudioGameSetup.c` owns the complete former `PreInitialiseGameSpecificSetup`, `PostInitialiseGameSpecificSetup`, `PreTerminateGameSpecificShutdown` and `PostTerminateGameSpecificShutdown` operations as C11. `AudioLifecycle.c` calls them directly at the original points. Their C++ members and four lifecycle trampolines are removed. One typed adapter returns the original global fire-manager address, and the existing `AudioPlayerEngineHost_Time` adapter supplies the unchanged clock read. Owner fields and global bank storage are unchanged. The Visual Studio project registers the C11 source; CMake/Premake discover it through existing globs.

The conversion preserves bank assignments, entity creation order and pointer arguments, each live initialization/status gate, optional bridge creation, every mission-slot write, the volume multiplier write inside that loop, captured timer/reset order and the final speech/mood reset. Shutdown preserves the original bridge/escalator/extra/police/water/fire/collision/front-end/projectile order, separate nonnegative-handle gates, captured destruction arguments and post-call handle clearing. Post-termination remains a no-op.

The original PS2 bank-initialization branch uses GTA3 bank/sample identifiers missing from the VC headers. Production identifiers remain unchanged. PS2 comparisons use explicit **test-only** bank and sample identities listed in `build/audio-game-setup-c-tests/ps2-identities.txt`; bank identities fit the real array bounds. `original-ps2-unsupported.log` records the original body's missing-identifier errors without those definitions. These comparisons check all original statements and C/C++ equivalence, not a complete PS2 game build or a verified PS2 asset mapping.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_game_setup.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_game_setup_ps2.ps1
```

The source audit compares all four complete original bodies, the adapter, reused timer getter, full owner/header/lifecycle changes and project registration. Runtime comparisons execute unchanged original C++ bodies or separately compiled C11 operations. Both link the actual C entity allocator, status/destruction operations, timer-reset service, mission resets and mission-position unit. Wrappers observe arguments and state around these real operations; unrelated service paths reject unexpected calls.

Each comparison performs 15,360 groups: 1,024 seeds, a complete four-operation sequence or one of four individual operations, and three passes. Two passes exercise live mutations and repetition; a fresh pass retains seeded gates. Inputs cover both owners, disabled initialization, empty/nearly full/full entity arrays, queued references to free slots, successful/disabled/capacity-limited allocation, physical entities with valid ped records, negative/valid/aliased/out-of-range shutdown handles, mission states, random-table extremes and unsigned timer wrapping.

Callbacks change live initialization gates between creation and status updates, redirect later shutdown handles, change the clock after its captured read, modify random data between ped timer writes, and change initialization between mission-slot clears. The observer hashes both complete initialized owners plus all bank, ped, clock and fire-manager records at callbacks and after groups. Every entity pointer uses a stable identity; padding starts at zero. Entity counts, order lists, queued references and supported physical pointers stay within their real bounds.

Verified results in `build/audio-game-setup-c-tests`:

- 32 matching Clang/GCC comparisons across two optimization levels and eight feature layouts
- Six matching native MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 sanitizer comparisons
- 1,050 actual consumer compile checks and 10 configuration guards
- All 66 preceding migration source audits
- 16 matching lifecycle regressions through the new direct C operation boundaries

The production consumer matrix covers D3D9/software rendering, x86/x64, threaded/single audio and WAV/all decoders. No full game link or gameplay test was performed. `AudioLogic.cpp` now has no active `cAudioManager` member implementations; its old crane implementation remains commented out. Remaining owner C++ features include the PS2 bank helper, unused square-root member, constructor and destructor. Lifetime conversion must retain the constructor's five pre-main random draws and the destructor's conditional termination and script-state reset, including their startup/exit ordering.
