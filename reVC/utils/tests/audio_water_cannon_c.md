# Water-cannon audio conversion

`src/audio/AudioWaterCannon.c` owns the complete former `cAudioManager::ProcessWaterCannon` generator as C11. The C entity dispatcher calls it directly. The owning C++ member and dispatch trampoline are removed; two typed game adapters retain the live cannon identifier and original position expression. No owner or cannon fields were added.

The conversion preserves the original position selection from cannon zero using each current cannon's signed segment index, identifier gates, strict distance and volume gates, guarded square root, loop sound fields, conditional emitting volume/reverb/reflection writes, live sample reads for loop offsets, and request submission. Correcting the unusual cannon-zero selection is outside this conversion.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_water_cannon.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_water_cannon_ps2.ps1
```

The source audit compares every statement in the complete original generator, both adapters, constants/macros, full caller/header changes, and project registration. Runtime comparisons compile the unchanged original body or the separately compiled C11 operation with identical observers and actual C math, geometry, requested-queue insertion, and priority ordering.

Each comparison performs 8,192 scans across all three cannons and two owners. It exercises every valid segment index, zero and signed identifier extremes, distinct per-cannon positions, distance/volume boundaries, NaN/infinite positions, camera callbacks that change scratch coordinates, identifier/index changes affecting later cannons, loop-start callbacks that change the loop-end sample, repeated scans, and full queue replacement/rejection. Invalid segment indices are excluded because their original array access is undefined.

The observer hashes every initialized owner and cannon field at callbacks and after scans. Scalar fields in the fixture's segment record are initialized directly so its automatic padding remains zero. An initial native x86 diagnostic found different unspecified temporary padding before the first generator callback; the fixture initialization was corrected and all runtime/native comparisons rerun. Production code was unchanged by that fixture correction.

Verified results in `build/audio-water-cannon-c-tests`:

- 28 matching Clang/GCC comparisons across two optimization levels and seven feature layouts
- Six matching native MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 sanitizer comparisons
- 940 actual consumer compile checks and 10 configuration guard checks
- All 60 preceding migration source audits
- Dispatcher regressions after replacing its water-cannon trampoline

Consumer checks include actual game headers/adapters and `WaterCannon.cpp`. No full game link, gameplay test, or full PS2 game build was performed. The next remaining audio work is script-object routing and its one-shot/looping generators in `AudioLogic.cpp`.
