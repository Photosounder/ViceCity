Cull zones in C11
================

`ZoneCull.c` owns all seven former CCullZones static fields, all eight complete
operations and the original polygon test. `ZoneCull.h` is a C header with eleven
flag/value accessors; the compatibility class is removed. All 25 affected C++
sources call C functions or access the C globals. CAttributeZone retains its
eight signed 16-bit fields, 16-byte size and two-byte alignment.

Coordinate queries use a three-float C value. The sole AddCullZone caller passes
the original position address as an opaque pointer: that parameter remains
unused, as it was in the C++ implementation. The empty ForceCullZoneCoors
operation also retains its original behavior. Bounds and signed attributes keep
their original conversions, inclusive comparisons, overlapping-zone OR and
maximum wanted-level calculation. Stairs lookup returns the first matching
record's live address. Initialization retains the beach flag and old records.

The eight-frame schedule and descending building, treadable and vehicle pool
scans remain in C. Fifteen small C++ adapters read coordinates, obtain existing
C pools, perform typed base-pointer conversions, read entity/train predicates
and update visibility bits. Elevated trains remain excluded. No entity layout
is guessed or duplicated in production C code. The C++ coordinate conversion
helper remains until the shared CVector type is converted.

Validation
----------

Eight original/C comparisons cover Clang/GCC, O0/O2 and default/MASTER builds.
Four native MSVC comparisons cover x86/x64 with whole-program optimization.
Two trapping undefined-behavior and float-cast-overflow sanitizer comparisons
cover both configurations. Each comparison runs 24576 operation groups and
observes 1537 player-coordinate queries, using the exact saved C++ class,
inline accessors, definitions and complete method bodies as its baseline.
The migrated operations and all flag getters run through an actual C11 caller.

The fixture rotates operation order, mutates live inputs at the player query
and retains state in a final pass. It observes every zone record, signed flag
and wanted-level result, returned record index, pool hole and visibility bit.
Independent checks cover all 2048 flag combinations, partial initialization,
float-to-integer bound truncation, exact edges, overlapping attributes, first
stairs pointer identity, NaN coordinates, degenerate polygons and subway/train
eligibility. Typed mock pools also exercise nonzero base-pointer offsets.

All 26 actual C++ header consumers and the C unit compile with MSVC on both
architectures: 52 C++ and two C compiles. Every complete source/project change
and the complete C kernel are audited against immutable originals; all 83
preceding source audits pass through explicit cull-zone transformations.
Full game linking and interactive gameplay remain outside these quiet checks.
