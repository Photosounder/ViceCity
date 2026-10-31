# C pedestrian routes

`PedRoutes.c` owns all six route operations and the single 200-element
`gaRoutes` table. `CRouteNode` is a plain C record with the original signed
16-bit route and a three-float `PedRouteVector`. Its layout remains 16 bytes,
with position at offset 4; the table remains 3,200 bytes on x86 and x64.
The former vector's empty default constructor has no base initialization
effects, and static zero initialization is preserved without a dynamic C++
constructor. No native bridge was added.

The two C++ pedestrian callers copy the returned C value into their existing
vectors. The script caller supplies a C vector value after collecting its
parameters. Route IDs retain the original narrowing to signed 16 bits.
The C owner keeps the same first-free insertion, first-match lookup and
first-contiguous-run removal behavior. The fixed removal algorithm shifts
the whole remainder; classic mode copies only the removed run's length.
Both clear only tail route IDs, retaining stale position data. Initialization
still resets every route to -1 and every coordinate to positive zero.

The original classic code can write past a full table on insertion and read
past its end for some removal spans. Those preexisting undefined paths remain
unchanged. Runtime comparisons cover only defined classic cases; fixed mode
also covers full-table insertion and every contiguous removal span.
No new bounds policy is introduced by this conversion.

Independent strict C11 callers exercise all six methods against the exact
original producer. Fixed mode compares 20,622 states and classic mode 10,520.
Coverage includes static initialization, all indexed getters, signed route
samples, -1 sentinel insertion, repeated IDs, earliest hole reuse, fragmented
routes, missing routes, stale positions and by-value vector isolation, including
signed zero, infinity and quiet NaN payloads. Hashes exclude padding that the
original methods did not initialize.

Clang/GCC at O0/O2, defined-path UBSan, and native MSVC x86/x64 compare original
and C results. Real native header probes verify layout, field types and original
copy/lifetime properties, and every affected producer compiles on both
architectures. All 48 project configurations select C11 for the renamed source
and retain distinct objects for the 28 existing C/C++ source pairs. Full D3D9
Debug/Release builds are checked without launching the game.

Run `utils/tests/test_ped_routes.ps1` for quiet checks. Isolated project checks
use `events_project.py --ped-routes`, `check_events_project.py --ped-routes`,
and `build_events_project.py --ped-routes` (plus `--release`). All diagnostics
stay in `.diagnostics/ped-routes-c-tests`. Regenerate project files for the
`PedRoutes.cpp` to `PedRoutes.c` rename.
