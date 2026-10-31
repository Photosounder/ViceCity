# C rectangles

`Rect.c` owns all rectangle operations and the 16-byte four-float record.
The original field order remains left, bottom, right, top; the explicit
constructor's argument order remains left, top, right, bottom. Thirteen public
C operations replace the two explicit constructors, explicit copy-construction
sites and all geometry operations, including the two inside tests and three
Grow overloads. No native bridge was added.

Ten local default constructors are now explicit `Rect_Init` calls. The 48 water
rectangles initialize at the original array-definition boundary. The existing
script overlay constructor initializes its rectangle before the former empty
color-constructor boundary; its destructor remains empty. Nineteen global
debug-zone initializer expressions retain their original order through C value
construction. Other record members remain ordinary C data: collision slots
already initialize their bounds explicitly through their C pool integration.

Containment preserves ordered source reads and self-containment. Point operations
take pointers to individual float coordinates, so the original conditional reads
and inclusive-test short circuits remain intact. Native vectors are evaluated
once at the remaining C++ caller boundary. Radius expansion, negative radii,
inverted bounds, signed extents and each Grow overload retain the original
arithmetic and field update order. Water data and enclosing native records keep
their original binary sizes and offsets.

Independent strict C11 callers compare all operations against the exact original
producer and inline methods. Each run observes 228,574 states and 28,650 explicit
invariants. Coverage includes the Cartesian product of thirteen IEEE float
patterns, signed zero, infinities and quiet NaN payloads, all fields, value copies,
self-containment, each growth policy, translation, inclusive edges and collapsed
bounds. Literal native water, overlay and debug-zone initialization sites exercise
the C/C++ structure-return boundary on x86 and x64. Source guards preserve the
empty overlay destructor; tests inspect initialized objects while their lifetime
is active.

Original/C results match within each compiler and optimization profile. Different
toolchains and x87/SSE execution can produce different NaN representations from
the same IEEE arithmetic; each profile retains its own original digest instead
of requiring one digest across unrelated implementations. Clang/GCC O0/O2,
UBSan and native MSVC x86/x64 all pass. Native source checks verify the rectangle,
script-overlay and collision-slot layouts and compile every affected producer.
Conditional PS2 source is converted, but these builds use the available D3D9
configuration rather than claiming a full PS2 frontend build.

All 48 generated configurations select C11 for the renamed source and retain
distinct objects for the 28 existing C/C++ source pairs. Full D3D9 Debug/Release
builds run without launching the game. Diagnostics stay under
`.diagnostics/rect-c-tests`. Run `utils/tests/test_rect.ps1` for quiet comparisons.
Full project checks use `events_project.py --rect`, `check_events_project.py --rect`
and `build_events_project.py --rect` (plus `--release`). Regenerate project files
for the `Rect.cpp` to `Rect.c` rename.
