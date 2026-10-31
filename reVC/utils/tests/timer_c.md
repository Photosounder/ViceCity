Game timer in C11
=================

`Timer.c` owns the twelve former CTimer static fields and twelve complete
operations. `Timer.h` is a C header with 28 inline accessors; the compatibility
class is removed. All 122 affected C++ sources use C globals and functions.
Five former const-reference getters return pointers to the same live storage.
Callers dereference those pointers, preserving lvalues and console bindings.

`Timer.cpp` retains four typed adapters for the platform clock, snapshot flag,
game-record playback and chase recording. Windows performance counters and the
portable platform-time fallback retain their original types and arithmetic.
Initialization, nested suspension, pause flags, time scaling, clipping,
unsigned wrap and the persistent logical-frame fractions retain their original
behavior, including the separate FIX_BUGS and vanilla update implementations.

Validation
----------

Twenty comparisons cover Clang/GCC, O0/O2 and default, vanilla, MASTER,
portable and portable-vanilla configurations. Ten native MSVC comparisons
cover the same configurations on x86/x64 with whole-program optimization.
Four trapping undefined-behavior and float-cast-overflow sanitizer comparisons
cover both update implementations and both platform-time paths.

Each comparison exercises 135168 operation groups using the exact saved C++
class, inline accessors and complete method bodies as its baseline. The C
caller exercises all operations and accessors, checks the five getter addresses
and observes exact floating-point bits, timer fields, counter storage, provider
arguments, return values and callback order. Callbacks mutate live inputs;
later passes retain state and run additional updates to expose persistent
fractional-frame behavior. Independent checks cover initial values, suspension
nesting, unsigned depth underflow, cycles per frame and chase-recording timing.

All 122 actual C++ sources and the new C unit compile with MSVC on both
architectures: 246 source compiles. Complete source/project changes are audited
against immutable originals, and all 82 preceding source audits pass through
explicit timer transformations. Full game linking and interactive gameplay remain
outside these quiet checks.
