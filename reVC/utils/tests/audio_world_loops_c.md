# Projectile, escalator and arcade audio conversion

`src/audio/AudioWorldLoops.c` owns the complete former `ProcessProjectiles`, `ProcessEscalators`, and `ProcessExtraSounds` generators as C11. The C dispatcher calls all three directly. Their C++ members, routing trampolines, and private constants are removed. Six typed game adapters retain separate projectile eligibility/weapon/position queries and escalator count/activation/position queries. No owner or game-object fields changed.

The writable `aVecExtraSoundPosition` array is now a C sound-position array. Its original coordinates, element count, float layout and per-iteration position reads are retained. The original vector constructor assigns only the three float fields; the new C initializer removes its two C++ constructor calls.

The conversion preserves all projectile type/default gates, original frequency arithmetic (including unsigned multiplication before division), live projectile pointers after frequency queries, repeated escalator lookups and signed count reads at every loop condition, strict distance/volume gates, original scratch-write order, conditional emitting/reverb/reflection fields, and actual request insertion/replacement/rejection. Projectile loop offsets read the live scratch sample; escalator and arcade offsets retain their fixed original sample arguments.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_world_loops.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_world_loops_ps2.ps1
```

The source audit compares every statement in all three complete original generators, all six adapters, constants/macros, C array initialization, the original vector constructor, full caller/header changes, and project registration. Runtime comparisons use the untouched original C++ bodies or actual separately compiled C11 operations, with identical observers and real C math, geometry, request insertion and priority ordering.

Each comparison performs 3,072 test groups across 1,024 seeded inputs. Two passes exercise live mutations and repeated scans; a fresh pass disables mutations to preserve seeded distance boundaries. Inputs cover both owners, all 32 projectile indices and 22 escalator indices, supported and unsupported representable weapon types, signed counts including zero/negative values, counts up to the game-array capacity, exact initial arcade coordinates and later writable positions, NaN/infinite positions, near distance/volume gates, very large base frequencies exercising unsigned arithmetic, and full queue replacement/rejection.

Callbacks activate later projectiles, change their weapon types, redirect projectile pointers after frequency queries, change positions before copies, modify scratch coordinates during camera queries, alter live loop-end samples, grow or shorten escalator counts during scans, change later arcade positions, and alter submission frequency state. The observer hashes both complete initialized owners and every initialized game record/input at callbacks and after groups. Projectile pointers are represented by stable array identities so address randomization does not affect comparisons. Automatic record padding is initialized directly and retained as zero. Invalid enum representations, out-of-range counts/indices, and invalid supported-projectile pointers are excluded because the original access is undefined.

Verified results in `build/audio-world-loops-c-tests`:

- 28 matching Clang/GCC comparisons across two optimization levels and seven feature layouts
- Six matching native MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 sanitizer comparisons
- 990 actual consumer compile checks and 10 configuration guard checks
- All 63 preceding migration source audits
- 20 matching dispatcher regressions and its sanitizer comparison

The consumer matrix includes the actual adapters plus `ProjectileInfo.cpp` and `Fluff.cpp` across D3D9/software rendering, x86/x64, threaded/single audio, and WAV/all decoders. No full game link, gameplay test, or full PS2 game build was performed. Remaining audio work includes garages, fire hydrants, bridge generators, and game-specific setup/termination in `AudioLogic.cpp`.
