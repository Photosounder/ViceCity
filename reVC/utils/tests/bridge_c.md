# Bridge controller in C11

`src/control/BridgeControl.c` owns all ten canonical bridge fields and all five complete operations. The `CBridge` static class has been removed. The four complete callers use `Bridge_` C functions or canonical globals. `Bridge.cpp` contains twelve typed native operations for entity fields, pool ownership, model constants, matrix/frame updates, the commercial-progress field and the existing path-link setter. Selection, timing, motion, light transitions and all bridge decisions remain in C.

Static zero-initialization and the original partial resets are retained. `Init` scans for components and resets only `OldLift` before conditionally capturing heights and enabling bridge lights. `FindBridgeEntities` clears the three component pointers and scans the real building pool in reverse; the lowest occupied matching slot consequently wins. It preserves repeated pool and model reads. `Update` keeps the original unsigned platform-time subtraction followed by sixteen-bit narrowing, so the operating cycle wraps at 65,536 milliseconds. Strict phase thresholds and old/new state comparisons are unchanged.

The motion sequence still writes the lift, synchronizes its native matrix, updates its frame, optionally moves the road, then moves the counterweight. Each use reads the actual live pointer again. The tests change pointers and later height/state fields from native callbacks to check this ordering. The real `CMatrix::UpdateRW`, `CEntity::UpdateRwFrame` and complete path-link rectangle routine are compiled in the oracle. Building shells deliberately have a nonzero base adjustment, so the typed conversion from physical pool slots is exercised.

The normal Vice City configuration disables `GTA_BRIDGE`, and the five functions retain their original inactive behavior. Before conversion, compiling the original with that optional feature enabled failed because this Vice City registry does not define its three bridge model IDs. The original compiles when those IDs are supplied externally. Optional-feature comparison programs and native compilation probes therefore supply test model IDs to both variants; this conversion does not add bridge models to Vice City. The baseline compiler diagnostics are retained in `build/bridge-baseline`.

Run from the repository root:

```powershell
./utils/tests/test_bridge.ps1 -Native -Consumers -BuildProject -AuditPrevious
./utils/tests/test_bridge.ps1 -Sanitized
```

The console oracle compiles the complete immutable original class implementation and canonical fields. It uses actual C pool code and actual native matrix/frame/path bodies. The full path setter is given a second name for instrumentation around its call; every statement of its implementation is retained. Assertions remain active in MASTER. No game or graphical test is launched.

Coverage includes all 64 component subsets; duplicate, empty and sparse pools; a live pool switch after the size lookup; a model changing between repeated comparisons; every millisecond of the sixteen-bit operating cycle and its wrap; unsigned platform timer overflow and underflow; startup at time zero; absent roads/components; unchanged heights; NaN and infinite height inputs; optional rendering attachments/objects; callbacks changing canonical pointers, defaults and states; strict path rectangle edges; and bridge/moving/flashing queries across valid and unknown states/models.

Validation consists of 24 Clang/GCC original/C comparisons (O0/O2, six profiles), nine native MSVC comparisons (x86/x64 with default, bridge, GL, MASTER and x86 vanilla), six sanitizer comparisons, 42 production compilations with real headers, original field/signature checks on both architectures, both actual D3D9 Debug/Release links, all fifty evaluated configurations with unique objects, and 140 preceding whole-source audits. Historical timer snapshots also prove the complete relocated bridge controller and native operations. The sound audit's earlier kernel translator now restores field spelling before binding original native accesses. All four raw original translations remain byte-identical; the affected sound and audio-bridge audits are rerun, and the first-pass diagnostics are retained separately.

Each bridge-enabled comparison reports 65,860 whole-state snapshots, 65,639 updates, 66 initializations, 66 explicit scans, 182 queries, 1,180 pool reads, 782 model reads, 60,179 matrix/frame updates and forty light calls. Its hash is `f39f6444f134aabb`. Default inactive comparisons report 294 snapshots and no native pool/model/render/path calls, with hash `aa3c00f393bf057d`.
