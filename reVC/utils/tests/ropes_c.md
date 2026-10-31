Ropes.c owns all nine former CRope/CRopes algorithms, the complete active-camera
visibility decision, all eight private records, the original 64-entry index
table and the persistent uint32 SWAT counter. Both classes are removed. The
public interface uses a plain C Rope record, uintptr_t IDs and three-float C
vectors. A C adapter writes legacy CVector outputs through a typed backend only
after successful interpolation; failed lookups preserve the output. Typed vector
readers accept original C++ temporaries at the remaining caller boundary.

Actual MSVC Win32/Win64 probes verify 780/792-byte records, pointer alignment,
flag offsets 0/1/2, ID offsets 4/8, timer offsets 8/16, position offsets 12/20 and
speed offsets 396/404. RwImVertexIndex is uint16_t. Actual native float wrappers
reference powf and sqrtf; the C implementation preserves those operations, vector
operand order and repeated live timer reads. The original CVector operators and
Clamp macro are snapshotted rather than replaced with approximate math.

Registration keeps first-match/first-free selection, conditional unsigned timer
wrap, alternating initial nodes and the existing-record timer behavior. Physics
keeps the strict timeout, sequential node updates, registered flag handling and
existing zero-distance/timestep floating results. SWAT setup remains orchestrated
in C: lazy model check, registration, population call, collision/sentinel/ID
writes, animation blend, then counter increment. The counter is not reset by
Init, and its additions wrap at uint32 width before assignment to pointer-width
rope IDs. No allocation failure behavior is changed.

Rendering preserves the environment-pass camera bypass, node16/radius20 sphere,
shared TempVertexBuffer storage, all vertex/state calls, transform flags, the
fixed line-list versus legacy polyline choice, all 64 index values and the
original 62-index draw count. It ends only successful draws and leaves vertex
alpha enabled. Debug labels and FINAL behavior remain intact.

ropes_fixture.py audits all ten complete changed source/project files against
immutable originals and all relocated statements. Differential programs link
the whole saved original implementation or the entire current C implementation.
Original vector/math code, real D3D/GL vertex records, render enums, buffer union,
timer interface and model/ped/animation enums are preserved. Native runtime
comparisons use actual common/vector/RW headers. Camera, streaming, population,
ped and animation sinks use legal C++ mock owners, with multiple inheritance to
exercise typed pointer conversions. Actual owner layouts are checked separately
by production compilations and the real full-project builds; these are quiet
algorithm comparisons, not gameplay tests.

The original global index table is given C linkage in the fixture solely to
share the observation interface; its initializer and all uses remain unchanged.
Both fixture implementations replace only the SWAT counter initializer with a
test macro: zero in normal profiles, UINT32_MAX-2 in the wrap profile. Every
algorithm statement remains present. Production retains its original zero seed.
The seeded profile verifies wrap without simulating billions of allocations.

Every run observes 647 complete pool states, all record fields, all 64 indices,
lookup outputs and all camera/render/model/population/animation operations.
Coverage includes 10,000 mixed operations, full and duplicate pools, timer wrap,
timeout boundaries, pointer-width IDs, clamped interpolation and output failures,
exceptional physics floats, SWAT failure/success paths, counter persistence/wrap,
all/none/mixed transform results, environment visibility and reentrant rendering
with shared storage. Finite result bits are compared exactly; NaNs are compared
by classification. Undefined NaN-to-index conversion is not treated as a valid
lookup input on a matching active record.

Twenty-four Clang/GCC O0/O2 comparisons and six undefined-behavior trap comparisons
pass across default, no-fix, non-librw, FINAL, software GL3 and counter-wrap
profiles. Eleven MSVC/LTCG comparisons pass across Win32/Win64, including vanilla
Win32. Forty actual production compilations cover seven C++ backend/consumer
units and the C11 implementation in default/software GL3 profiles on both
architectures plus vanilla Win32. Both actual Debug and Release x64 D3D9/OpenAL
projects compile and link, with existing dependencies and Git hash events
disabled. No game is launched. Object collisions are checked in all fifty project
configurations, now with twelve paired C/C++ units. All 97 preceding complete
source audits pass. Commands, inputs and results are in build/ropes-c-tests.
