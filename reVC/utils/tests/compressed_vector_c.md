# C collision vertices

`CompressedVector.c` owns all three former instance operations: Get, Set and
compressed-mode Unpack. The public record is plain C data, retaining three
signed 16-bit components in compressed mode (6 bytes, alignment 2), or three
float components otherwise (12 bytes, alignment 4). Both representations retain
trivial default lifetime, copying and destruction. The decoded value is a
12-byte `CollisionVector3`; no native bridge was added.

Compressed Get divides each component by `128.0f` in the original order.
Compressed Set multiplies each captured argument by `128.0f` and narrows each
result to its signed 16-bit field. The original undefined conversion cases for
out-of-range or nonfinite compressed inputs are not assigned a new policy.
Uncompressed Get/Set retain coordinate representations without arithmetic.
PC Unpack retains signed extension, three coordinate lanes and a zero fourth
lane. The PS2 assembly instruction sequence and clobbers are unchanged, including
its original fourth-lane behavior; only the implicit receiver and reference
operand become explicit C pointers.

All native consumers now call the C API. Collision/model, glass and vehicle-glare
callers capture decoded values before the original native vector expressions,
preserving their arithmetic grouping. Triangle
plane adapters decode all three temporary inputs before invoking the unchanged
plane operation. File loading retains its float input reads and storage stride.
VU collision callers pass the original quadword storage address to C Unpack.

Strict independent C11 callers compare the exact original methods against C in
float, compressed and VU simulation configurations. Each compressed run observes
131,840 states and 659,456 invariants, including all 65,536 signed codes,
fractional truncation, exact scaled round trips, input retention, unpack lanes,
guards and array stride. Float runs observe 2,496 states and 5,248 invariants,
including signed zero, subnormal values, infinities and quiet NaN payloads.
Literal model and triangle-plane consumer bodies exercise native C/C++ value
returns on x86 and x64.

Clang/GCC O0/O2, UBSan and native MSVC x86/x64 agree in all three configurations.
Real native headers verify component types, sizes, alignment, parent layout and
trivial lifetime. Native source checks also compile the PC VU simulation unit
and its collision paths. The retained MIPS assembly is guarded by exact source
comparison; these checks do not claim a PS2-target executable build.

All 48 generated configurations select C11 for the new producer and preserve
the 28 existing C/C++ object pairs. Full D3D9 Debug/Release builds run quietly
without launching the game. Diagnostics stay in
`.diagnostics/compressed-vector-c-tests`. Run
`utils/tests/test_compressed_vector.ps1` for quiet comparisons. Isolated full
project checks use `events_project.py --compressed-vector`,
`check_events_project.py --compressed-vector`, and
`build_events_project.py --compressed-vector` (plus `--release`). Regenerate
project files to include `CompressedVector.c`.
