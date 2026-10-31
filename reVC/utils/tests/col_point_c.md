# C collision contacts

`ColPoint.c` owns the complete contact record and eight C operations: normal and
depth queries, both metadata setters, selective assignment, full value copying,
normal normalization and normal reversal. `CColPoint` contains only C coordinates,
two explicit gap words, four metadata bytes and depth. Its former member methods,
custom assignment operator and vector members are removed.

The record remains 40 bytes aligned to 4: point at 0, first gap word at 12,
normal at 16, second gap word at 28, metadata at 32 through 35 and depth at 36.
Actual native COFF constants compare automobile/bike sizes and wheel-contact
offsets before and after in every x86/x64 collision configuration. The original
copy constructor is trivial and copies all fields. Original default construction
only calls empty vector constructors; the native vector base is also verified
trivial. No new initialization policy is added to local or static records.

`ColPoint_Assign` preserves the original selective operation: copy point, normal,
surface A, piece A, surface B and piece B, then return the destination. It leaves
both explicit gap words and depth untouched. Both real assignment sites now call
it explicitly. `ColPoint_CopyValue` preserves full copy construction, including
the gap words and depth; the saved weapon contact uses that separate operation.
SetSurfaces changes only the four metadata bytes. Set writes depth first, then
those same metadata bytes in their original order.

GetNormal borrows the actual C normal subobject and retains visibility of later
contact changes. Native math reads that result through the existing coordinate
adapter. GetDepth returns the stored float. NormaliseNormal retains the actual
native squared-length and reciprocal-square-root grouping. Its zero/nonfinite
fallback changes only X, leaving Y and Z representations untouched. ReverseNormal
retains the original ordered multiplication of all three components by -1.

The spring-collision function now accepts the actual contact record instead of a
native vector reference to its normal. Both existing callers pass their wheel
contact. Reversal therefore still changes the stored normal before either force
application. A legacy bullet-trace call keeps an actual native endpoint alive
through its existing native vector reader. Other point/normal expressions use
explicit native value conversions, avoiding unrelated class views.

Independent strict C11 callers compare all original operations with C. The
88,215-state fixture covers arbitrary gap/depth/coordinate payloads, ordinary and
self-assignment, full C and C++ value-copy boundaries, static/default lifetimes,
65,536 metadata/depth/query cases, live normal pointers, special IEEE normal
triples and random normal values. Stored representations and untouched fields
are compared exactly. Arithmetic NaN payloads and scalar query NaNs are
canonicalized for portable result summaries. The normalization fallback separately
checks that Y and Z bytes remain untouched.

The fixture compiles six actual before/after collision routines. It also compiles
the complete original and updated spring-collision body against force callbacks
that observe the contact normal at both application points. All 4,096 suspension
cases compare reversal, force order, force/position values and untouched contact
fields across compression, step-clamping, mass, bias and heavy-body choices.
The preprocessor alternatives in the VU metadata call retain their original
argument text and directive positions.

Clang/GCC O0/O2, UBSan and MSVC x86/x64 agree in float, compressed-vertex and VU
simulation profiles. All 116 native source/layout checks pass for both staged
and production sources. All 48 evaluated project configurations select C11 for
`ColPoint.c` and retain the 28 paired C/C++ object names. Full D3D9 Debug/Release
builds link. The box and sphere fixtures consume the C contact record and retain
their previous behavior results. Their normal operations link the C producer.

The apply helper verifies every source against the immutable original or exact
converted projection. It can resume a partial apply and retries only transient
Windows invalid-argument open failures after proving the file bytes are intact.

Run `utils/tests/test_col_point.ps1` for quiet verification. Isolated full checks
use `events_project.py --col-point`, `check_events_project.py --col-point` and
`build_events_project.py --col-point` (plus `--release`). Diagnostics stay under
`.diagnostics/col-point-c-tests`, use static compiler runtimes, verify GCC imports
and never launch the game. The user's `build/` directory remains untouched.
Regenerate normal project files for the `ColPoint.cpp` to `ColPoint.c` rename.
