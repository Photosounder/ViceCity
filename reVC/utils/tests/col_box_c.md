# C boxes and collision boxes

`ColBox.c` owns both complete box records and all five C operations:
`Box_Set`, `Box_GetSize`, `ColBox_SetBounds`, `ColBox_Set` and `ColBox_Assign`.
Both records use plain `CollisionVector3` coordinates. `CColBox` embeds a `CBox`
as `bounds`, replacing inheritance, the inherited setter overload and the custom
C++ assignment operator. Native calls that previously accepted a derived box as
a base reference now pass its actual `bounds` subobject.

`CBox` remains 24 bytes with minimum at offset 0 and maximum at offset 12.
`CColBox` remains 28 bytes with bounds at 0, surface at 24, piece at 25 and two
trailing padding bytes. Both align to 4 bytes. Native x86/x64 probes verify the
original and new layouts, the collision-model parent layout, copy construction,
empty native vector-base construction and removal of the inheritance relationship.

Setters copy the entire minimum before reading the maximum through live float
references. Reversed self-references therefore retain their original ordered
assignment behavior. The inherited two-argument setter becomes `ColBox_SetBounds`
and leaves metadata untouched. The four-argument setter writes minimum, maximum,
surface and piece in their original order. Assignment copies only those fields,
leaves destination tail padding unchanged and returns the destination pointer.
Size queries retain signed `max-min` components, including reversed bounds.
Empty default construction and static zero initialization require no new policy.

Native vector expressions use the existing bit-preserving coordinate adapters.
Borrowed bounding-box coordinates in pedestrian and rubbish code remain pointers
or references to actual C coordinates. The collision-model copy loop explicitly
calls `ColBox_Assign` so it retains the former assignment padding behavior.
Loading retains the original 28-byte input record stride.

Independent C11 callers compare complete original methods against C functions.
Direct C++ callers separately exercise the value-return ABI. Each configuration
covers 100,012 states: all external/minimum/maximum setter aliases, self and
ordinary assignment, both destination padding bytes, all 65,536 surface/piece
pairs, coordinate payload copies, signed/reversed/nonfinite size queries, empty
lifetimes, static records and 16,384 geometric cases.

The geometric cases compile the actual before/after `CCollision::TestSphereBox`,
`TestLineBox`, `TestVerticalLineBox`, `ProcessSphereBox` and `ProcessLineBox`
implementations. They compare hit results, contact records, distance outputs
and unchanged bytes. Arithmetic NaN payloads are canonicalized; setters and
assignment independently compare coordinate representations exactly.

Clang/GCC O0/O2, UBSan and native MSVC x86/x64 agree in float, compressed-vertex
and VU simulation profiles. All 176 affected source/layout checks pass for both
the staged tree and production. All 48 project configurations select C11 for
`ColBox.c` and preserve the 28 paired C/C++ object names. Full D3D9 Debug and
Release builds link. Earlier conversion audits and the line/triangle runtime
checks retain their existing results.

Run `utils/tests/test_col_box.ps1` for quiet verification. Isolated full builds
use `events_project.py --col-box`, `check_events_project.py --col-box` and
`build_events_project.py --col-box` (plus `--release`). Tests remain under
`.diagnostics/col-box-c-tests`, use static diagnostic runtimes, verify GCC imports
and never launch the game. The user's `build/` directory remains untouched.
Regenerate normal project files for the `ColBox.cpp` to `ColBox.c` rename.
