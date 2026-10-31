`RampageSystem.c` contains all 14 former `CDarkel` operations and all 18 canonical
state fields, including the complete registered-model counter array. The header is
C compatible and every caller supplies the original default headshot argument
explicitly. `Darkel.cpp` retains 60 typed access primitives for external owners
that still use C++ records; no rampage branch or update loop remains there.

The C implementation preserves replay gates, all weapon-equivalence conditions,
headshot/model rules, statistic updates, signed/unsigned timer arithmetic, fade and
blink boundaries, both platform animation-removal orders, complete startup and
weapon restoration, and the original failure-then-completion behavior in a single
update. Screen arithmetic is copied from the original macros and uses the existing
C draw primitives. Shared scratch arrays and native text-pointer identity remain live.

One native model-removal expression retains both original fresh player lookups and
the native receiver/argument evaluation order. It performs one external model-removal
call with no rampage decision inside it. Ammo selection and its repeated `Min`
operands remain in C. The model uses actual primitive weapon-field types so C++
overloaded observation wrappers cannot change enum conversions or assignment order.

Each whole original/C comparison covers 25,617 complete state snapshots, every
registered model, all initialized scratch bytes, native calls, weapon/stat changes,
overload selection and live callbacks. Clang and GCC run at O0/O2 across six profiles:
fixed PC scaling, classic PC, classic PS2, fixed PS2/PAL, forced PC scaling and classic
aspect scaling. Native MSVC runs all six on x86/x64. Undefined-behavior checks run
the same profiles. Differences between compiler profiles are retained; each original
must match its corresponding C executable exactly.

Native probes compare every original canonical field type on both architectures.
Actual source checks compile the owner, its C policy and every changed consumer.
The isolated full D3D9 project includes the new C11 object with no paired-object
collisions. Full Debug/Release builds and wanted-system regression comparisons are
recorded under `.diagnostics/darkel-c-tests`.

Run `utils/tests/test_darkel.ps1` to repeat the quiet source, behavior and native
checks. Saved originals under `.diagnostics/darkel-c-tests/before` are required.
Portable diagnostics link static compiler runtimes and run with only Windows
directories on PATH. No game or GUI executable is launched.
