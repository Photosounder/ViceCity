# C car and train doors

`DoorSystem.c` owns all 16 operations of the former car/boat and train door classes.
Their C records retain the original 36-byte and 24-byte layouts. The cached speed
is a plain 12-byte velocity record. `Door.cpp` retains only two typed native
geometry leaves: physical point velocity and the original vector-first matrix
multiply. Door state, bounds, damping, clamping, bounce and query logic all run
in C.

`Door_Init` and `TrainDoor_Init` preserve the constructors' complete `memset`,
including padding. `Door_SetLimits` and `TrainDoor_SetLimits` preserve the old
inline `Init` methods' partial updates. Automobile, boat and train primary
constructors explicitly initialize their door members before their original body
operations. The train initialization remains unconditional when its optional body
is disabled. The crossed vector constructors are empty, and real native headers
verify that their SDK base performs no default-construction work. Copy and
destruction remain trivial.

The two door owners keep their distinct opening rules. Car partial opening resets
angular velocity only when its resulting angle compares equal to zero. Full
opening does not perform that reset. Train opening retains its translation-only
behavior. Query tie choices, zero guards, the half-radian openness tolerance and
unordered comparisons are unchanged. Swinging retains its unsupported-axis zero
force, strict dead zone, damping/clamp sequence, independent maximum/minimum stop
tests and final cached-world-speed update.

Quiet differential tests use every literal original method, the complete vector
headers, the original physical-speed/cross-product/matrix-multiply bodies and both
literal native bindings. They compare 59,469 states, observing every record field
and query result. Coverage includes normal, equal and inverted limits, negative
ratios, exact/adjacent force and angle thresholds, all direction bytes, unsupported
axes, nonidentity transforms, repeated moving frames, signed zero, infinity and
NaN comparisons. Computed NaN payloads are normalized; finite values and signed
zeros are compared exactly.

All three explicit constructor blocks also run on nonzero storage with the actual
empty vector/col-point member constructors, so implicit zero-filled storage cannot
hide a missing initialization call. Literal record constructors are checked byte
by byte. Source guards retain the empty-constructor assumptions.

Clang/GCC O0/O2, undefined-behavior checks and MSVC x86/x64 agree. Real native
headers verify all door fields and the enclosing automobile, boat and train ABI.
Native checks compile every changed producer and isolate the real train-door
consumer types even when their legacy outer code is disabled. Both complete
D3D9 builds compile and link without launching the game. All 48 generated
configurations compile `DoorSystem.c` as C11 and retain distinct object names for
the 28 existing C/C++ source pairs. Earlier conversion audits still pass.

Checks live under `.diagnostics/doors-c-tests`. Run `utils/tests/test_doors.ps1`
for quiet behavior and native-source checks. Use `events_project.py --doors`,
`check_events_project.py --doors` and `build_events_project.py --doors` (plus
`--release`) for the isolated full project. Regenerate project files to include
`DoorSystem.c`.
