# C quaternions

`Quaternion.c` owns the entire quaternion record and all 24 operations: four-float
construction, magnitudes, normalization, multiplication, conjugation, compound
operators, value operators, dot product, slerp and all six overloaded axis/matrix/
Euler conversions. `CQuaternion` is now a plain four-float C record. Its former
constructors, methods and operators are removed, and every native caller uses the
C operations. The producer is renamed from `Quaternion.cpp` to `Quaternion.c`.

The record stays 16 bytes aligned to 4, with X/Y/Z/W at 0/4/8/12. The original
empty default constructor wrote nothing, and static records remain zero-filled.
Ordinary copies remain trivial. Actual native compiler assertions preserve all
keyframe sizes, quaternion/time offsets, translation offsets and packed strides
on x86/x64 in float, compressed-vertex and VU configurations.

Normalization retains its original zero-length fallback, which sets only W to 1.
Multiplication retains ordered X/Y/Z/W writes and live input reads when either
operand aliases the destination. Compound operators retain their destination
return identity. Conjugation negates only XYZ; unary negation changes all four.
Left and right scalar multiplication retain their respective operand orders.
Slerp keeps the zero-angle copy, obtuse-angle branch and complete weighted
temporaries before assignment. No mathematical corrections are mixed into this
conversion.

Native axis and matrix adapters borrow individual float fields. They contain no
math policy and never cast a C record to a native class. Axis-angle conversions
preserve live angle/output aliases. Matrix output snapshots the quaternion
products and writes only the original nine axis coordinates, preserving flags,
padding and position. Matrix input keeps each original trace branch and field
read/write order. Euler output snapshots matrix axes first, retains F3/F1/F2
write order and supports overlapping output pointers and quaternion-field aliases.

The independent strict C11 fixture compares the entire original implementation
against C across 65,042 states and 2,821,670 invariant checks. It includes the
14-value IEEE grid over all four components, full-width payloads, every value
and mutation operation, self/operand aliases, no-write/default/static lifetimes,
all matrix branches, axis/angle aliases, Euler aliases and native adapters.
Arithmetic NaNs are canonicalized for summaries; construction representations,
untouched fields and matrix metadata are checked exactly.

The fixture compiles all actual blend-node methods, quaternion-flip removal and
all nine complete frame callbacks before and after. Its 18,432 frame scenarios
cover skinned and non-skinned paths, compressed and ordinary keyframes, fixed
and classic hemisphere behavior, ignored components, velocity extraction,
partial weights, looping, keyframe state, packed sign corrections and native
frame outputs. Required native velocity pointers remain valid when directly
testing callbacks that assume them. Rotation-only and translated frames occupy
actual objects with their original strides.

Clang/GCC O0/O2, UBSan and MSVC x86/x64 all match their corresponding original
implementations. Different toolchains/architectures retain their own original
floating-point math results; equivalence is checked within each environment.
Both staged and production sources pass 54 native source/layout checks. All 48
project configurations select C11 and preserve all 28 C/C++ basename pairs.
Full D3D9 Debug and Release executables link without launching the game.

Run `utils/tests/test_quaternion.ps1` for quiet verification. Full isolated checks
use `events_project.py --quaternion`, `check_events_project.py --quaternion` and
`build_events_project.py --quaternion` (plus `--release`). Diagnostics remain in
`.diagnostics/quaternion-c-tests`, use static compiler runtimes, verify GCC imports
and leave the user's `build/` directory untouched. Regenerate normal project
files for the producer rename.

After keyframe conversion, this fixture retains the proven historical keyframe
schema for its animation comparison. `keyframes_fixture.py` tests the current
C keyframe records and complete animation consumers against their originals.
