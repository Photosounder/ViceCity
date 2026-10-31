# Fire audio conversion

`src/audio/AudioFires.c` owns the complete former `cAudioManager::ProcessFires` generator as C11. The C entity dispatcher calls `AudioFires_Process` directly; the owning C++ member and dispatch trampoline are removed. Seven typed fire-state adapters preserve live ongoing/audio flags, captured entity ownership, position copying, water-flag reads and clearing, and countdown reads. The existing entity-type adapter is reused.

The conversion retains the scan across all `NUM_FIRES` slots, short-circuit gates, building/pedestrian/default/unattached selection, base-frequency and slot variation, strict distance and volume gates, burning-loop fields, live loop-offset sample reads, and request submission. The extinguishing path still reads its water flag after the burning path, clears it before reading the countdown, and uses the current scratch distance even when the burning distance gate skipped its update. No owner or fire record fields were added.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_fires.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_fires_ps2.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_fires_edges.ps1
```

The main source audit compares every statement in both complete sound paths, all adapters, constants/macros, full caller/header changes, and project registration. Runtime comparisons compile the untouched original C++ body or the separately compiled C11 operation against identical observers and actual C math, geometry, request insertion, and priority ordering.

Each main comparison performs 4,096 scans across every fire slot and two owners. It covers all entity choices and null entities, eligibility gates, distance and volume edges, post-submission water-flag changes, clear-before-countdown ordering, captured entity pointers, base-frequency callbacks that change positions, loop-start callbacks that change the subsequent loop-end sample identifier, repeated scans, and full queue replacement/rejection. Complete initialized owner and fire records are hashed at callbacks and after each operation, with entity pointer identities normalized.

The separate cached-distance fixture performs 128 scans with one far-away fire, eight cached distances, four countdown values, and two owners. It verifies that the burning distance gate leaves the cached distance untouched, extinguishing uses that value, the water flag is consumed, and the second scan does not repeat the extinguishing sound. All countdown and volume inputs remain within the original defined float-to-byte conversion domain; undefined input values are excluded. A diagnostic run found an invalid synthetic callback combination at seed 621: a fire changed to the pedestrian range while retaining distance 79.999, producing an out-of-range byte volume. The main fixture keeps callback-reenabled water sounds nearby after entity changes. Production calculations are unchanged; the original diagnostic is recorded under `debug`.

Verified results in `build/audio-fires-c-tests`:

- 28 matching Clang/GCC comparisons across two optimization levels and seven feature layouts
- Six matching native MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 sanitizer comparisons
- Four matching cached-distance comparisons plus its sanitizer comparison
- 920 actual consumer compile checks and 10 configuration guard checks
- All 59 preceding migration source audits
- 20 matching dispatcher regressions plus its sanitizer comparison

Result files and compiler logs are stored in `build/audio-fires-c-tests`, with the focused fixture under `edges`. Consumer checks compile real game headers/adapters and `Fire.cpp`. No full game link, gameplay test, or full PS2 game build was performed. The next audio operation to inspect is `ProcessWaterCannon` in `AudioLogic.cpp`.
