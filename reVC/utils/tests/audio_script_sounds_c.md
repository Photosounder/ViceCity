# Script-object audio conversion

`src/audio/AudioScriptSounds.c` owns the complete former `ProcessScriptObject`, `ProcessOneShotScriptObject`, and `ProcessLoopingScriptObject` operations as C11. The C dispatcher calls the new routing operation directly. Their C++ members and dispatch trampoline are removed; four typed adapters retain the original music-manager reads and club-track updates. Existing typed player and surface queries are reused. Owner, script-object, and player layouts are unchanged.

The conversion preserves the game-music gate, nullable script pointer, position-copy order, exactly-one-event routing, signed script/event IDs narrowed to sound bytes, every original switch case and default return, original sample/frequency choices (including `FIX_BUGS` differences), shared one-shot byte counter, distance/volume gates, conditional scratch-field writes, live loop-offset sample reads, speech context initialization, and actual request insertion/reflection behavior.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_script_sounds.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_script_sounds_ps2.ps1
```

The source audit compares every statement in all three complete original functions, all music adapters, constants, conditional macros, full caller/header changes, and project registration. Runtime comparisons use the unchanged original C++ functions or the separately compiled C11 functions, with identical observers and the actual C math, geometry, requested-queue insertion, reflections, and priority ordering. Existing speech setup is observed at its separately converted boundary, including every defined context field.

Each comparison performs 49,152 test groups covering all 256 sound bytes and 64 input variants, two live-mutation passes and a fresh pass with mutations disabled to retain every seeded distance boundary. Inputs include both owners, null script/player pointers, multiple event counts, signed script IDs narrowed to sound bytes, all game surface classes plus unknown values, both ambient club tracks, shared counter wrapping, signed random-table extremes, strict distance/volume boundaries, NaN/infinite positions, and full queue replacement/rejection. Callback mutations exercise subsequent entity/event reads, player surface reads, club sample selection, camera/scratch coordinates, loop-end sample reads, and submission frequency state.

The observer hashes every initialized owner/script/music/player/camera field at callbacks and after groups. Actual script pointers are represented by stable identities in copied observer records so process address randomization does not affect comparisons. Automatic pedestrian-context padding is excluded; all its defined fields are observed. Dynamic reflection insertion is disabled only in the vanilla layout, where the existing reflection operation reads an uninitialized old frequency; original generator reflection writes remain observed. Invalid entity indices are excluded because the original array access is undefined.

Verified results in `build/audio-script-sounds-c-tests`:

- 28 matching Clang/GCC comparisons across two optimization levels and seven feature layouts
- Six matching native MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 sanitizer comparisons
- 950 actual consumer compile checks and 10 configuration guard checks
- All 61 preceding migration source audits
- 20 matching dispatcher regressions and its sanitizer comparison

Consumer checks include the actual game adapters and callers for D3D9/software rendering, x86/x64, threaded/single audio, and WAV/all decoders. No full game link, gameplay test, or full PS2 game build was performed. The next remaining audio work is weather and front-end sound generation in `AudioLogic.cpp`.
