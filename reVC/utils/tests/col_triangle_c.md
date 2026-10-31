# C collision triangles and planes

`ColTriangle.c` owns both complete triangle records and all nine former methods,
including scalar and VU plane setup, indexed setup, queries and unpacking. The
triangle remains 8 bytes with three 16-bit indices, a surface byte and untouched
padding. The scalar plane remains 20 bytes and the VU plane 8 bytes. Parent
collision-model offsets and sizes remain unchanged. All six direction constants
retain their original values. No native bridge was added.

Plane setup retains `(c-a) x (b-a)`, the native magnitude/reciprocal-square-root
grouping, and the normalization fallback that changes only X. Scalar setup writes
and normalizes its destination before computing distance through the still-live
vertex references. `CollisionPointRef` retains separate float references so a
vertex can continue to alias the destination normal. Its small C helper is valid
in both C and C++. VU setup computes the normal and distance before narrowing
the packed fields. Its original undefined nonfinite/out-of-range narrowing cases
do not gain a new policy.

Indexed setup captures all three decoded values before changing the plane,
including the valid VU case where the compressed source is the plane's own normal.
GetNormal retains exact coordinate copies/scaling and ordered output writes.
CalcPoint retains dot-product grouping and distance scaling. Direction selection
retains its strict comparisons, tie behavior and sign tests. Triangle setup
retains integer narrowing and the trailing padding byte.

All collision, loading, glass and shadow consumers now call C. The shadow caller
that previously read three individual components uses the common GetNormal API,
which works with either plane representation. PC Unpack retains all four signed
lanes. The PS2 instruction sequence and clobbers are unchanged; only the implicit
receiver and output reference become explicit pointers. Exact source comparison
guards that assembly without claiming a PS2-target executable build.

Independent C11 callers compare the exact original methods and the actual
`CVector` normalization/cross-product helpers with the C implementation. Scalar
profiles compare 88,997 states and 214,676 invariants; VU compares 146,085 states
and 304,879 invariants. Coverage includes all 65,536 index/packed-code values,
ordinary and degenerate geometry, direction ties, special IEEE inputs, padding,
every valid scalar source-normal alias, self-aliasing normal queries, packed
source aliases, unpack guards and direct C++ calls through the C reference-record
ABI. Input generation is explicitly sequenced across compilers. Finite results
are compared exactly; arithmetic NaN payloads are canonicalized while getters
separately verify exact stored payload copies.

Clang/GCC O0/O2, UBSan and native MSVC x86/x64 agree in float, compressed-vertex
and VU simulation profiles. Native source checks compile every affected producer
in all three configurations and verify record/parent layouts and reference ABI.
All 48 project configurations select C11 for the renamed source and preserve
the 28 existing C/C++ object pairs. Full D3D9 Debug/Release builds run quietly
without launching the game.

Diagnostics stay under `.diagnostics/col-triangle-c-tests`. Run
`utils/tests/test_col_triangle.ps1` for quiet checks. Isolated full project checks
use `events_project.py --col-triangle`, `check_events_project.py --col-triangle`
and `build_events_project.py --col-triangle` (plus `--release`). Regenerate
project files for the `ColTriangle.cpp` to `ColTriangle.c` rename.
