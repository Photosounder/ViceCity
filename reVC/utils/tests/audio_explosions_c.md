# Explosion audio conversion

`src/audio/AudioExplosions.c` owns the complete former `cAudioManager::ProcessExplosions` generator as C11. The C entity dispatcher calls `AudioExplosions_Process` directly; its C++ owning member and dispatch trampoline are removed. Five typed game adapters retain the sound flag, signed active-counter result, counter reset, byte-sized type result, and original position-copy expression.

The conversion preserves the scan across all `NUM_EXPLOSIONS` slots, sound/counter short-circuit order, reset before type and distance queries, every sample/frequency/bank selection, helicopter frequency adjustment, conditional reflection writes, strict distance gate, guarded square root, volume gate, scratch fields and request submission. Inaudible explosions still consume their active counter. No owner or explosion record fields were added.

`src/weapons/ExplosionTypes.h` shares the unchanged original `eExplosionType` declaration with C and C++. `Explosion.h` includes it while retaining its existing class layout. The C module's distance constants and macros preserve the original values and conditional writes. CMake and Premake already include C sources and headers by glob; the existing Visual Studio project and filters register the new files explicitly with C11 compilation.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_explosions.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_explosions_ps2.ps1
```

The source audit compares every statement in the complete original generator, five adapters, type declaration, constants, macros, full caller/header changes, and project registration. Runtime comparisons compile the untouched original body or the separately compiled C11 operation against identical observers and the actual C math, geometry, request insertion, priority ordering, and reflection implementation.

Each comparison performs 4,096 scans across every explosion slot and two owners, retaining random-displacement history. It covers every original type and representable unknown type, sound permission, signed counter-byte boundaries, repeated scans after consumption, distance/volume boundaries, NaN/infinite positions, callback changes to later state and live scratch reads, full queue replacement/rejection, and actual reflection insertion. Complete initialized owner and explosion records are hashed at callbacks and after each operation.

The fixture excludes the pre-existing undefined reflection-frequency temporary in the optional non-`FIX_BUGS` request implementation by disabling dynamic reflection insertion only in that layout. The vanilla explosion generator's conditional reflection writes are still observed, and defined vanilla queue insertion is tested. Other layouts exercise reflection insertion, including slow-motion frequency behavior. Volume inputs remain within the original defined float-to-byte conversion domain.

Verified results in `build/audio-explosions-c-tests`:

- 28 matching Clang/GCC comparisons across two optimization levels and seven feature layouts
- Six matching native MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 sanitizer comparisons
- 900 actual consumer compile checks and 10 configuration guard checks
- All 58 preceding migration source audits
- 20 matching dispatcher regressions and its sanitizer comparison after removing the explosion trampoline

Consumer checks include the actual `Explosion.cpp` implementation and real game headers/adapters. No full game link, gameplay test, or full PS2 game build was performed. The next audio operation to inspect is `ProcessFires` in `AudioLogic.cpp`.
