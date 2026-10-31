# Weather and front-end audio conversion

`src/audio/AudioWeatherFrontend.c` owns the complete former `ProcessWeather` and `ProcessFrontEnd` operations as C11. The C dispatcher calls both directly. Their C++ members, routing trampolines, and private front-end volume constant are removed. Five typed game adapters retain camera ceiling distance, camera/wind-modifier queries, wind reads, tree-distance reads, and the original tree reset; the existing C rainfall and rain-zone adapters are reused. No owner or game-record fields changed.

The conversion preserves thunder sample selection and volume arithmetic, the shared four-value thunder counter, rain short-circuit order and repeated rainfall reads, wind modifier arithmetic and NaN comparisons, repeated wind/tree reads, tree reset after submission, every front-end sound case/default continuation, shared pickup/mission cooldowns and strict frame gates, radio-dial state, byte-counter wrapping, live entity/event reads, stereo/center sample increments, frequency-query order, conditional scratch writes, and real request insertion/rejection.

An initial native trace found that the C maximum expression observed one wind query where the original observed two. An explicit branch alone retained that difference under native optimization. A volatile local comparison value keeps the first query separate from the selected-value query, without changing any stored record. A focused native callback/scalar trace then matched; the complete compiler/native/sanitizer comparisons were refreshed after applying this change.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_weather_frontend.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File utils/tests/test_audio_weather_frontend_ps2.ps1
```

The source audit compares every statement in both complete original functions, all five adapters, original arithmetic and loop/emitting/reverb/reflection macros, full caller/header changes, and project registration. Runtime comparisons use the unchanged original C++ functions or the actual separately compiled C11 operations, with identical observers and the actual C math, requested-queue insertion, and priority ordering.

Each comparison performs 24,576 test groups across all 256 event bytes and 32 input variants. Two passes exercise live mutations and repeated calls; a fresh pass disables mutations to preserve seeded boundaries. Inputs cover both owners, event counts zero through four, all original sound IDs plus unknown IDs, strict frame cooldowns and unsigned frame wraparound, shared byte-counter wrap, signed random-table extremes, defined thunder/rain/front-end float-to-byte volumes, both rain-zone results and short-circuit combinations, ceiling/tree/wind boundaries, NaN values whose original gates avoid invalid conversion, and full queue replacement/rejection.

Callbacks change rainfall between reads, later rain-zone decisions, wind between comparison/value reads, tree distance before later volume reads, sample indices between loop-offset queries, and scratch entity/pan/frame state between stereo/center submissions or later events. The observer hashes both complete initialized owners and every live game-query input at each callback and after groups. Inputs causing undefined original float-to-byte conversions or out-of-range entity/event access are excluded.

Verified results in `build/audio-weather-frontend-c-tests`:

- 28 matching Clang/GCC comparisons across two optimization levels and seven feature layouts
- Six matching native MSVC comparisons across x86/x64 and three desktop layouts
- Matching desktop and PS2 sanitizer comparisons
- 960 actual consumer compile checks and 10 configuration guards, with the final generator rechecked in all 10 consumer layouts after the wind-read change
- All 62 preceding migration source audits
- 20 matching dispatcher regressions and its sanitizer comparison

The consumer matrix checks D3D9/software rendering, x86/x64, threaded/single audio, and WAV/all decoders. No full game link, gameplay test, or full PS2 game build was performed. Remaining audio generators include projectiles, escalators, extra sounds, garages, hydrants, and bridge audio in `AudioLogic.cpp`.
