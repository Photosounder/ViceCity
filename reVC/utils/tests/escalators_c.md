Escalators.c owns all four original escalator-controller methods and the
canonical signed registration count. The CEscalators class and its static
methods are removed. All 22 registrations, startup/shutdown ordering and the
replay-gated update traversal execute as C11. MovingThings.c enters the C API
directly, removing three former C++ escalator adapters. Replay.cpp enters the
C shutdown API, and the audio count reader observes the canonical C global.

The native 22-element CEscalator array remains at its original construction
point in Fluff.cpp. Its CMatrix and step-object ownership, complete class,
constructor, AddThisOne, Update, SwitchOff and inline audio readers are held
unchanged by audit for the subsequent record conversion. The controller's four
typed backend operations retain actual native record calls and the replay
predicate. A const-reference getter gives audio readers the actual stable
native record. No record or CVector is reinterpreted as a C structure.

The following record conversion removes CEscalator completely and owns every
plain field in the C Escalators_aEscalators pool. Its five operations are C,
and original constructor clearing is replaced by equivalent C static zero
initialization. Native CMatrix objects live in a parallel array. The three
controller-to-record adapters and the const-reference compatibility getter are
removed; the controller calls the C record operations directly. The controller
comparison retains the same external record simulator, while the separate
record comparison verifies complete allocation/movement/teardown algorithms.
Historical record inputs are held against the record stage's immutable before
snapshot, and its current complete C relocation is checked explicitly.

AddOne preserves the original count postincrement before the selected record
callback and captures all four endpoints by value. Init retains its initial
shutdown, explicit second count reset, all 22 complete coordinate/direction
registrations and the final 22-record switch-off pass. Update queries replay
once and then visits all 22 slots. Shutdown visits all 22 slots and resets count
after their callbacks. Both loops use the original fixed capacity. Count and
replay mutations during a loop, nested lifecycle calls and count mutations in
the final initialization pass retain their original effects.

The fixture compares eight complete existing files and three complete new
files against immutable originals and the explicit migration. It separately
checks all four complete native record bodies, the complete record declaration,
replay header, CMatrix, CObject and common/config headers unchanged. The original
executable compiles all four extracted controller methods without modification.
Both executables use the same instrumented external record simulator and keep
actual CVector endpoint values and const record/position references. Separate
C11 callers exercise the by-value endpoint and boolean ABI on both sides.

The corpus covers every valid registration index and direction, all original
coordinates, partial initialization, stable native owner identities, signed-zero
and large finite endpoint values, replay skips, extreme signed counts in fixed
traversals, count changes during registration, aliased source records and nested
registration/update/shutdown/initialization calls. Two thousand sustained frames
exercise lifecycle transitions. Every comparison observes exact float bits,
all 22 simulated records, count values inside callbacks and full event order.

Twenty GNU comparisons cover Clang/GCC at O0/O2 in default, plain, reflection,
GL and unfixed profiles. Five Clang comparisons enable undefined-behavior and
float-conversion traps. Five native MSVC comparisons use actual project headers
and link-time optimization for Win32 default/GL/vanilla and Win64 default/GL.
Native probes verify all public signatures, captured endpoint types/layout,
canonical count width, replay return type, stable audio references and the
unchanged 248-byte Win32 / 352-byte Win64 native record.

Twenty-five production compilations cover Fluff.cpp, Replay.cpp, AudioLogic.cpp,
MovingThings.c and Escalators.c in five architecture/feature combinations. Both
actual Debug and Release x64 D3D9/OpenAL projects compile and link. All 50 project
configurations retain distinct objects for their 28 paired C/C++ units. All 121
distinct preceding source audits pass, retaining complete historical audio
bodies until the audio migration consumes them. Existing moving-scenery runtime
comparisons are repeated through the direct C escalator calls with their
previous results. No game or GUI was launched.

Run utils/tests/test_escalators.ps1. Optional switches enable Sanitized, Native,
Consumers, BuildProject and AuditPrevious. Immutable originals, generated
oracles, diagnostics and verification records are under build/escalators-c-tests.
