# C collision lines

`ColLine.c` owns endpoint initialization, ordered endpoint updates, temporary
construction and complete raw-record transfers. The record contains plain C
coordinates and two explicit padding words. It remains 32 bytes with endpoints
at offsets 0 and 16, padding words at 12 and 28, and alignment 4 on both native
architectures. Empty default construction required no explicit replacement.

The C operations capture and write the first endpoint before reading the second.
This preserves self-aliases: setting a line from its own reversed endpoints
duplicates the former second endpoint instead of swapping them. Initialization
and updates leave both padding words untouched. Coordinate copies retain exact
floating-point representations, including signed zero and nonfinite payloads.
Temporary padding retains its original unspecified values.

`CollisionVector.h` shares the plain coordinate and live-reference records with
compressed vertices and triangle planes. Remaining native math consumers use
explicit bit-preserving coordinate conversions. `ColLine_ToVu` copies all 16
bytes of an endpoint, including its fourth lane. `ColLine_FromRaw32` transfers
scratch quadwords into an actual C record instead of using unrelated typed views.

`TransformColLines` preserves endpoint order and the original matrix operation.
It replaces strided `RwV3d` reads for line records with typed C coordinate access.
The existing assembly branch, including its original `GTA_PS3` guard, instructions,
operands and clobbers, is retained exactly. Source comparison checks that branch;
the checks do not claim a PS2/PS3 target build.

Independent strict C11 callers and direct native callers compare 89,089 states
against the original complete methods. Checks cover every legal combination of
external/self endpoint references, all 65,536 repeated-halfword coordinate
patterns, arbitrary initialized coordinate/padding bytes, raw transfers, native
conversion round trips, quadword copies, default lifetimes and batches of zero
through eight transformed lines with output guards. Batch checks inspect XYZ;
the original transform leaves the fourth lane unspecified.

Clang/GCC O0/O2, UBSan and MSVC x86/x64 agree in float, compressed-vertex and
VU simulation profiles. All 80 affected native source/layout checks pass both
before application and against production. The triangle and compressed-vector
fixtures retain their previous results after moving their shared types.
All 48 evaluated project configurations select C11 for `ColLine.c` and retain
the 28 paired C/C++ object names. Full D3D9 Debug and Release builds link.

Checks remain under `.diagnostics/col-line-c-tests` and never launch the game.
GCC diagnostic executables link their runtimes statically and run without MSYS
paths. Their import tables are also checked for GCC/MSYS runtime DLL dependencies.
The user's `build/` directory is not recreated or modified.

Run `utils/tests/test_col_line.ps1` for quiet verification. Isolated full project
checks use `events_project.py --col-line`, `check_events_project.py --col-line`
and `build_events_project.py --col-line` (plus `--release`). Regenerate normal
project files for the `ColLine.cpp` to `ColLine.c` rename. The legacy CodeWarrior
project already references multiple removed C++ producers and remains outside
the validated MSVC/Premake configurations.
