`RoadBlockSystem.c` contains all six former `CRoadBlocks` operations and the former
inline script-roadblock midpoint operation. It owns all four canonical roadblock
objects. `RoadBlocks.h` is a C header, and `CScriptRoadblock` is a plain record with
the original 28-byte extent, four-byte alignment and field offsets. The optional
SECUROM global retains its original name and transitions.

All path-node discovery, frame batching, spatial gates, wanted-unit priority,
model fallback, cop placement and flags, random consumption, car and barrier
placement, collision rejection, siren choice, pool allocation and temporary-object
reclamation remain in C. Initialization still returns before clearing script
requests when too many eligible nodes are found. Script requests still use the
first free slot, retain stale coordinates when cleared, and deactivate after a
nearby request is consumed. Strict 90/100-distance and 0.94-up-axis thresholds are
unchanged.

`RoadBlocks.cpp` retains 103 typed native primitives for owners that still have C++
records. It contains no roadblock branch or loop. Local native matrices occupy
aligned stack storage checked against `CMatrix` on x86/x64; their construction,
destruction and native matrix operations remain native primitives. Every temporary
is destroyed in its original order across early exits. No heap allocation was
added for local matrices. Vector arithmetic and selection policies are C11.

Whole original/C tests exercise seven public operations with 1,612 complete state
snapshots per fixed profile and 1,588 per classic profile. They compare all canonical
fields, initialized matrix components, pool slots and generations, native entity
fields, native reference-slot addresses, random state and external call effects.
They use the actual project vector/matrix arithmetic and actual C pool allocator.
The model covers every special cop unit and fallback, all six cop offsets, full
script registries, path overflow, all frame batches, divided and undivided roads,
signed lane counts, missing models/ground, collision rejection, rejected-car
destruction, temporary-object reuse, timer wrap and live callbacks.

Clang and GCC run at O0/O2 across fixed, classic, fixed/classic squeeze, SECUROM,
MASTER and PS2 profiles. Clang undefined-behavior checks run all seven. Native MSVC
runs all seven at both O2 and Od on x86/x64. Each original must match its corresponding
C executable exactly. The fixture models `FindPlayerPed` as a pure lookup after
checking its actual original body; logging that pure getter would create a false
observable difference when MSVC changes operand evaluation order. The saved random
provider is also checked to exclude player/wanted access. No production evaluation
order workaround or trace filtering is used.

Native probes check every canonical storage type and script-record layout. The
complete owner, C implementation and all changed callers compile on x86/x64.
All 48 generated MSBuild configurations compile the new object as C11 and retain
the existing 28 paired C/C++ object names. Full D3D9 Debug and Release executables
are built under `.diagnostics/roadblocks-c-tests`, without launching the game.

Run `utils/tests/test_roadblocks.ps1` to repeat the quiet behavior and native checks.
Saved originals under `.diagnostics/roadblocks-c-tests/before` are required.
Full-build helpers accept `--roadblocks`. Portable checks link static compiler
runtimes and run with only Windows directories on PATH.
