# C animation keyframes

`KeyFrames.h` defines all four keyframe records in C: ordinary, translated, packed
and packed translated. `KeyFrames.c` owns all six former packed member methods.
Both inheritance relationships are replaced with a real common record as the
first member, and translated frames use C coordinates instead of a native vector.
The record strides remain 20, 32, 10 and 16 bytes. Translation remains at byte 20
or byte 10. Native x86/x64 compiler assertions preserve the original sizes,
alignment and field offsets in float, compressed-vertex and VU configurations.

Sequence getters return the actual embedded base subobject for translated arrays.
Common-only animation reads now use ordinary or packed common-record pointers.
Translation-container recovery occurs inside the branch that has translation
data. This keeps the original array strides and first-member identity while
avoiding a synthetic translated object around a rotation-only frame.

The packed methods preserve signed values, scale constants, ordered component
writes and truncation. Time queries retain division by 60; bulk unpacking retains
multiplication by its reciprocal. The exhaustive fixture confirms these can
produce distinct float representations. Time packing keeps the original positive
half bias, including negative-time behavior. Inputs whose scaled truncated value
does not fit a signed 16-bit result remain outside the original defined domain;
no new clamping or rounding policy is introduced.

The native adapters borrow actual vector fields or transfer exact XYZ bits into
actual native values. They contain no animation or packed conversion policy.
All unpacked vector expressions keep their native grouping. Ordinary record
copying stays trivial, original default construction writes nothing, and static
records retain their zero-filled storage.

The strict C11 fixture compares the actual original inline methods with C over
85,005 states and 578,841 invariant checks. It decodes all 65,536 signed 16-bit
representations and exercises exact and fractional packing boundaries, complete
untouched storage, ordinary copies, all four default/static lifetimes and actual
sequence getter pointer identities. It also compiles complete original and
updated sequence allocation, packing and unpacking bodies across 1,024 scenarios,
including the existing compressed-allocation size policy.

All 13 blend-node methods and all nine frame callbacks compile before and after.
The 18,432 frame scenarios cover skinned/non-skinned output, packed/unpacked
records, quaternion sign corrections, fixed/classic behavior, ignored channels,
velocity extraction, partial weights, looping and keyframe state. Both the C
math producer and its actual native consumers are tested. The older quaternion
fixture keeps its historical keyframe schema; this fixture covers the current C
records and animation consumers.

Clang/GCC O0/O2, UBSan and MSVC x86/x64 match their corresponding originals.
All 60 staged and 60 production native source/layout checks pass. All 48 project
configurations select C11 and retain the 28 distinct C/C++ basename pairs.
Historical projections verify all seven changed sources against 25 earlier
conversion stages. Full D3D9 Debug and Release executables link without launching
the game. Diagnostics use static compiler runtimes and verify GCC imports.

Run `utils/tests/test_keyframes.ps1` for quiet verification. Full isolated checks
use `events_project.py --keyframes`, `check_events_project.py --keyframes` and
`build_events_project.py --keyframes` (plus `--release`). Diagnostics stay in
`.diagnostics/keyframes-c-tests` and leave the user's `build/` folder untouched.
Regenerate normal project files to include the new C producer.

After blend-node conversion, this historical fixture retains the node schema
verified at its conversion boundary. `anim_node_fixture.py` tests all current
C node methods and complete frame consumers. Its original C record producer
continues to supply the packed operations under test.
