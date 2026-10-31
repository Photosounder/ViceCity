Both water-cannon classes are removed. WaterCannon.c owns all nine original
algorithms, all three records and the twelve render indices. The C11 header
exposes plain records and functions. WaterCannon.cpp retains 38 typed operations
for the remaining game owners and RenderWare resources. Five consumers enter
the C algorithms or read their public records through the existing audio API.

Actual MSVC Win32/Win64 probes establish a 412-byte record with alignment four:
ID at zero, current segment at four, creation time at eight, positions at twelve,
velocities at 204 and used flags at 396. Both original and C records have sixteen
segments. Actual render indices are uint16_t. The original vector math uses
sqrtf, float reciprocal square root, component operation order and the x-only
normalization fallback, all retained in C.

Input APIs support C vectors and opaque references to actual legacy CVector
objects. The shared C implementation checks both pointers before reading either,
writes position before reading velocity and preserves real subobject aliases.
It retains ID narrowing and signed/unsigned matching, ID-zero behavior, first-free
selection, partial initialization, creation-based timeout and unsigned timer wrap.
The audio position reader keeps the original cannon-zero position array indexed
by the requested cannon's current segment.

All integration, fire-query short circuits, strict ped bounds, descending sparse
pool traversal, first-hit selection, movement force, planar blending/clamping,
animation selection and particle orchestration execute in C. Typed backends read
actual CPed fields and call owner operations without guessing object layout.
MSVC's C frontend evaluates by-value vector arguments before random arguments
where the original C++ evaluates the random arguments first. Win32/Win64 traces
diagnose this difference; the C path explicitly draws/narrows the ninety-degree
rotation then the ten-unit rotation speed before querying particle position.
Other frontends retain their matching original expression evaluation order.

Rendering retains all camera/frame/matrix null branches, auxiliary-camera
normalization, deterministic auxiliary-pass texture coordinates, main-pass random
draws, ribbon winding, shared vertex buffer, first valid-pair normal reuse,
zero-cross-product NaNs, color narrowing, indices, render-state changes and
conditional transform/render/end calls. Hook reentry uses the same buffer.

water_cannon_fixture.py audits all complete algorithms and nine entire changed
source/project files against immutable originals. Original algorithms compile
unchanged except for includes and index-array linkage needed by the common
observer. Original vector methods, enums and audio reader bodies are compiled;
C entry points are also called from a separate C11 unit. Native comparisons use
actual common/vector/matrix/RenderWare headers and legal C++ mock owners. Actual
owner bindings compile separately and in the full project.

The corpus observes 689 complete states (688 with MASTER assertions disabled),
every cannon field and segment, twelve indices, audio outputs, ped effects,
random/getter/query ordering and complete render arguments and vertices. Cases
cover pointer assertions, signed IDs, full pools, actual input/output aliases,
every ring slot, timeout boundaries/wrap, fire outcomes, sparse/empty ped pools,
all four impact directions, both sides of the planar speed clamp, all auxiliary
camera branches, used-pair gaps, failed transforms, 6,000 mixed frames and reentry.
Finite float bits are compared exactly and NaNs by classification. Fingerprints
are compiler-specific where original argument evaluation order differs.
MinGW assertion recovery uses its supported non-SEH setjmp mode because no
nontrivial objects cross the jump; its default SEH recovery crashes inside
RtlVirtualUnwind with optimized GCC in this test harness.

test_water_cannon.ps1 runs portable comparisons and exposes native, sanitizer,
production-consumer, full-project and preceding-source audit options. Commands,
immutable inputs and results are under build/water-cannon-c-tests. These are
quiet algorithm/build checks; the game is not launched.

Twenty Clang/GCC O0/O2 comparisons and five undefined-behavior trap comparisons
pass across default, non-librw, FINAL, software GL3 and MASTER profiles. Nine
MSVC/LTCG comparisons pass on Win32/Win64, including vanilla Win32. Thirty-five
actual production compilations cover the backend, five consumers and C11 unit
across default/software GL3 and vanilla Win32. Actual Release and Debug x64
D3D9/OpenAL projects compile and link using existing dependency libraries with
Git hash events disabled. Object paths are unique in all fifty configurations,
including all fifteen C/C++ pairs. All one hundred preceding whole-source
migration audits pass. finalize_water_cannon.py verifies the complete evidence.
