# Garage and fire-hydrant audio conversion

`src/audio/AudioGarageHydrant.c` owns the complete former `ProcessGarages` and `ProcessFireHydrant` generators as C11. The C dispatcher calls both directly. Their C++ members, dispatcher trampolines and private constants are removed. Five typed adapters retain live garage count/type/door/state queries and the original hydrant position query. Garage positions use the existing entity adapter; hydrants retain a separate `CParticleObject` cast. Owner and game-object fields are unchanged.

`src/control/GarageTypes.h` shares the original garage-state and garage-type enums with C. `Garages.h` includes it without changing enum values, class fields or methods. The Visual Studio project registers both the C11 source and shared header; CMake and Premake discover them through their existing globs.

The conversion preserves both conditional garage-loop index widths, skipped inactive/null-door records, repeated live state queries, crusher frame/random gates and the original jump to garage events. The distance cache is shared between each garage's looping sound and door events. The one-shot byte counter retains its original static lifetime, starting at 32 and wrapping back to 32. Event loops continue to read the current scratch entity index and event count. Hydrant distance/volume gates, samples, frequencies, conditional fields and request order are unchanged.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_garage_hydrant.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_garage_hydrant_ps2.ps1
```

The source audit compares every statement in both complete original generators, all five adapters, enums, constants/macros, full owner/header/caller edits and project registration. Runtime comparisons execute the untouched original C++ bodies or actual separately compiled C11 functions with identical observers and real C math, geometry, request insertion and priority ordering.

Each comparison performs 3,072 groups across 1,024 seeded inputs. Two passes exercise live mutations and repeated scans; a fresh pass disables mutations to retain seeded boundaries. Inputs cover both owners, all 32 garage indices, all one-shot counters from 32 through 255, zero/full garage counts, inactive and unsupported byte types/states, null doors, crusher states, frame parity, random-table extremes, door events, exact distance/volume boundaries, NaN/infinite positions and queue replacement/rejection.

Callbacks grow garage counts, change live types/states, redirect door pointers, change entity/particle positions, alter scratch coordinates during camera queries, replace live loop samples and change cached distance or scratch entity/event selection during submission. The observer hashes both complete initialized owners and every initialized game record at callbacks and after groups. Raw entity/door pointers are normalized to stable identities. Padding is initialized directly and retained as zero. Counts stay within real array capacity and supported hydrant pointers stay valid. Vanilla dynamic reflection insertion is disabled to avoid an existing uninitialized-frequency path in the unchanged request code; generator reflection writes are still observed.

Verified results in `build/audio-garage-hydrant-c-tests`:

- 28 matching Clang/GCC comparisons across two optimization levels and seven feature layouts
- Six matching native MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 sanitizer comparisons
- 1,020 actual consumer compile checks and 10 configuration guard checks
- All 64 preceding migration source audits
- 20 matching dispatcher regressions and its sanitizer comparison

The consumer matrix includes the actual adapters, `Garages.cpp` and `ParticleObject.cpp` across D3D9/software rendering, x86/x64, threaded/single audio and WAV/all decoders. No full game link, gameplay test or full PS2 game build was performed. The next audio conversion is the bridge state machine and its warning, motor and transition sounds, followed by game-specific setup/termination in `AudioLogic.cpp`.
