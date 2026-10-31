Game clock in C11
=================

`Clock.c` owns all nine former CClock static fields, fast/freeze flags, seven
complete operations and five scalar/pointer accessors. `Clock.h` is a C header;
the compatibility class is removed. All 29 affected C++ source files use the
C globals and functions. Hours and minutes remain bytes, seconds remain an
unsigned 16-bit field with the original signed getter, and timer fields remain
unsigned 32-bit values. Boolean storage uses C11 bool, retaining the original
one-byte bool console interface on the checked architectures.

`Clock.cpp` retains five typed timer, pad, statistics and console adapters.
Clock initialization emits the original debug messages and registers the same
live hour/freeze pointers in the same order. Pointer accessors replace reference
accessors without copies. The save system retains its original field widths,
order, four-byte alignment and padding behavior.

Update preserves repeated timer reads, the strict elapsed-time threshold,
unsigned timer/multiplication wrap, byte overflow, fast time, freeze time and
the distinct midnight behavior of the shoulder-button shortcut. Restore only
writes the three saved time fields, retaining the stored flag and last tick.
Equal time-range endpoints retain the original empty range.

Validation
----------

Sixteen comparisons cover Clang/GCC, O0/O2 and default, vanilla, MASTER and
PS2 shapes. Six native MSVC comparisons cover x86/x64 and default, MASTER
and PS2 with whole-program optimization. Two trapping undefined-behavior and
float-cast-overflow sanitizer comparisons cover default and MASTER.

Each comparison exercises 73728 operations, observing 55565 callbacks by
default and 43533 in MASTER. The baseline compiles the exact original class,
inline reference/value accessors, definitions and complete method bodies.
Callbacks mutate live timer and clock inputs with nonzero scale and valid bool
representations; a final pass retains state between operations. Independent
checks cover strict thresholds, both midnight paths, byte overflow, signed
seconds, writable aliases, range endpoints and partial restore behavior.
The exact original save/load macros and clock statements verify the 16-byte
serialized clock fragment and its untouched padding bytes.

All 29 actual C++ sources and the new C unit compile with MSVC on both
architectures: 58 C++ and two C compiles. Every complete source/project change
is audited against its immutable original, and all 81 preceding source audits
pass through explicit clock transformations. Full game linking and interactive
gameplay remain outside these quiet checks.
