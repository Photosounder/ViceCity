# Traffic-light controller in C11

`src/control/TrafficControl.c` owns the canonical green-lights cheat flag, all ten former `CTrafficLights` methods and the complete line-intersection helper. The static class is removed. All six complete caller files use the C API or canonical C flag. `TrafficLights.cpp` now contains fifty-nine typed native field/action boundaries and two private vector conversions. Phase selection, rendering decisions, effect extrema, map traversal, path writes, intersections and vehicle stopping remain in C.

The zero-initialized boolean retains its actual type and stable address. All eleven function signatures and all pedestrian/car enum values match the original. The public/backend headers compile independently as C11. Native type checks establish the opaque model/list/node identities and the twelve-byte vector representation on x86/x64. The vector packet is copied through typed native reads and writes, without treating native objects as C records.

The controller preserves the original full 16,384-millisecond phase cycle, strict wind threshold, cheat precedence and storm-warning blink bit. NaN wind still takes the original distinct logical and visual paths. Vehicle stopping retains each next/current/previous route gate, physics-only previous-node handling, both signed directions, unsupported traffic type behavior and strict six/eight-unit distance thresholds. The original optional bridge-light short circuit is retained.

Rendering retains all four model layouts, selected effects/faces, IDs, colors, brightness thresholds, native transforms and external action order. `Min` and `Max` remain macros with their original repeated operand evaluation; effect reads are not cached or replaced by single-evaluation functions. The former point-light and bright-light adapters are replaced by their existing C APIs, with identical values and defaults. Shadows and coronas enter their original typed APIs, including the original narrowed pointer identifiers.

The scan retains x-major sector order, borrowed list/node reads, the real model classifier, native fixed-point path coordinates, connection masks, strict range/height gates and bitfield writes. Its original collinear-intersection behavior remains, including disjoint collinear segments being reported as intersecting. Valid effect-store and path-index preconditions are unchanged.

Run from the repository root:

```powershell
./utils/tests/test_traffic_lights.ps1 -Native -Consumers -BuildProject -AuditPrevious
./utils/tests/test_traffic_lights.ps1 -Sanitized
```

The console oracle compiles the entire immutable original controller and flag. Both variants use the complete real `General.c`, current model-classification function, real vector/matrix implementations, real effect getter and C store resolution, actual path-node/link declarations and complete point/bright legacy adapters. Native owner shells expose the same fields and observe external actions. Assertions remain active in MASTER. No game or graphical test is launched.

Coverage includes every millisecond of a full phase cycle, all logical/visual functions, exact thresholds and timer wrap, NaN/infinite wind, cheat combinations, 3,606 orientation cases, all four model layouts, brightness/visibility/storm combinations, repeated effect lookups with a changing live store, damaged/tilted lights, callbacks changing later live inputs, signed-zero stopping distances and NaN geometry, 12,984 stopping/bridge cases, 1,296 intersection cases, and twelve full 160-by-160 sector scans. Scans cover asymmetric links, preexisting direction flags, crossing flags, exact distance/height boundaries, unsupported models and traversal-dependent results.

Validation passes 24 Clang/GCC original/C comparisons at O0/O2 over six profiles, nine native MSVC comparisons on x86/x64, six sanitizer comparisons, 56 actual production compilations over default/reflection/optional-bridge/vanilla configurations, full signature/type/enum checks, both actual D3D9 Debug/Release links, unique object paths in all fifty configurations, and 141 preceding whole-source audits. Five earlier snapshots independently prove the complete relocated controller and typed native boundaries. The timer audit was rerun against the final native adapter; first-pass diagnostics are retained separately.

Optional-bridge production consumer probes supply the three bridge model IDs externally, as in the verified bridge baseline: this Vice City registry does not define those optional models. The normal production configurations retain their existing definitions. The initial optional-model diagnostic is retained under `build/traffic-lights-c-tests`.

Each comparison observes 1,823 whole-state snapshots, 83,110 phase calls, 1,809 displays, 12,984 stop/bridge cases, twelve scans, 1,296 intersection cases, 3,606 orientation cases, 307,200 sector reads, 529 point lights, 2,464 bright lights, 1,029 shadows and 1,157 coronas. Original and C traces match within every compiler/profile. Portable comparisons report 38,064 effect-store reads; several MSVC LTCG profiles report 29,120, with identical original/C counts in those profiles as well.

| Comparison profile | Hash |
| --- | --- |
| Clang/GCC default/plain/MASTER/unfixed/GL | `6f4e3be2abbcb47c` |
| Clang/GCC optional bridge | `be9636ef1180dd25` |
| MSVC default/GL and x64 MASTER | `f838b14ad1aec594` |
| MSVC optional bridge | `07f2ce8701c0c75d` |
| MSVC x86 MASTER/vanilla | `6f4e3be2abbcb47c` |
