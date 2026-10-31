# C spheres and collision spheres

`ColSphere.c` owns both complete sphere records and ray intersection. `CSphere`
uses plain C coordinates and a radius. `CColSphere` embeds a `CSphere` as `bounds`
and retains its surface and piece bytes. The former inheritance, setter overloads,
vector members and ray method are removed. Six C operations provide both setters,
the inherited bounds setter, ray intersection and explicit scratch transfers.

`CSphere` remains 16 bytes: center at 0 and radius at 12. `CColSphere` remains
20 bytes: bounds at 0, surface at 16, piece at 17 and two trailing padding bytes.
Both align to 4. Native x86/x64 probes verify both historical and new layouts,
collision-model parent layout, original trivial copying, empty native vector-base
construction, removal of inheritance and the three-pointer output-reference ABI.

`Sphere_Set` retains center-before-radius writes. `ColSphere_Set` retains
radius-before-center writes followed by surface and piece. `ColSphere_SetBounds`
retains the inherited base operation without changing metadata or padding.
Center transfers preserve complete floating-point representations. No default
initialization policy was added; local empty construction and static zero-filled
records retain their original behavior.

`ColSphere_IntersectRay` retains the complete subtraction, squared-distance and
dot-product grouping and calls the existing C quadratic solver with coefficient
1. It preserves the original treatment of nonunit and zero directions. Failure
leaves outputs untouched. Success evaluates all entry coordinates, writes entry,
then rereads the live origin and direction for exit. `CollisionPointOut` carries
actual writable float pointers, so outputs can alias inputs, each other or the
sphere's center without substituting snapshots for live references.

`Sphere_FromRaw16` copies the complete native center/radius quadword into an actual
C geometry record. `ColSphere_FromRaw16` supplies the same geometry and zeroes
only synthetic metadata absent from the original scratch storage. That metadata
is unused by the VU collision paths; the caller still obtains actual surface
information from the original model. The returned record's trailing padding has
no public value contract. `Sphere_ToVu` transfers all sixteen actual geometry
bytes without an unrelated typed view.

`TransformColSpheres` preserves the original scalar matrix operation, count and
20-byte input stride. The existing assembly branch, its `GTA_PS3` guard,
instructions, operands and clobbers are retained exactly. Source comparison
guards that branch without claiming a PS2/PS3 target build. Native edge-distance
queries keep a local native center alive only where their output is a distinct
local point. Other native math uses the existing coordinate value adapters.
Unrelated RenderWare sphere records remain untouched.

Each configuration compares 183,532 states against complete original methods,
the actual quadratic solver, six actual native collision routines and the
original strided transform. Coverage includes all 65,536 surface/piece pairs,
arbitrary center/radius payloads, setter self-aliases, static/default lifetimes,
65,536 raw quadwords and transforms of zero through eight sphere records.

Ray checks cover all 40,000 combinations of five actual source/output vector
subobjects, including the sphere's center. They compare both independent C11
callers and direct C++ callers through the C reference-record ABI. The set
contains 35,374 hits and 4,626 misses and verifies untouched objects, radius,
metadata and padding. Additional cases cover signed-zero and nonfinite inputs.
Geometric checks retain degenerate cases and the original normalization behavior.
Arithmetic NaN payloads are canonicalized; setters and raw transfers separately
verify exact stored representations.

Clang/GCC O0/O2, UBSan and MSVC x86/x64 agree in float, compressed-vertex and VU
simulation profiles. All 140 native producer/layout checks pass for both staged
and production sources. All 48 evaluated project configurations select C11 for
`ColSphere.c` and retain the 28 paired C/C++ object names. Full D3D9 Debug and
Release builds link. The earlier box fixture uses the explicit sphere record
and retains its previous compiler-matrix, sanitizer and native results.

Run `utils/tests/test_col_sphere.ps1` for quiet verification. Isolated full checks
use `events_project.py --col-sphere`, `check_events_project.py --col-sphere` and
`build_events_project.py --col-sphere` (plus `--release`). Diagnostics stay under
`.diagnostics/col-sphere-c-tests`, link static compiler runtimes, verify GCC
imports and never launch the game. The user's `build/` directory is untouched.
Regenerate normal project files for the `ColSphere.cpp` to `ColSphere.c` rename.
