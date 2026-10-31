Zone owner and records in C11
============================

`Zones.c` owns all twelve former CTheZones static fields, all 29 complete
operations and the former CZone translated-name method. `Zones.h` exposes six
live record/index accessors and a C coordinate value. The CTheZones, CZone and
CZoneInfo classes are removed. CZone and CZoneInfo are ordinary C records.
Two C++ adapters retain the existing player-coordinate and text-owner queries.

The original level and gang enumerations are shared through C headers.
All 29 nested CCarCtrl car-class values and relative count expressions move
to CarClasses.h with CARCLASS_ names. Car-control arrays and callers use those
same values. Existing callers explicitly include gang, car-control and vehicle
declarations that were previously supplied indirectly by Zones.h.

Hierarchy insertion retains first-child order, recursive containment and moving
existing children into a newly inserted enclosing zone. Coordinate queries keep
inclusive edges and original first-match selection. Label creation still mutates
the caller's text to uppercase and truncates stored labels to seven characters.
FindNext continues the shared signed 16-bit search cursor. Both FIX_BUGS and
original density interpolation implementations remain intact, including live
day/night output aliases. Audio zones retain their depth-first traversal order.

Serialization retains the exact field order, eight-byte header, fixed-width
pointer indices, padding and optional save validation count. CZoneInfo remains
68 bytes with two-byte alignment. CZone remains 56 bytes on x86 and 72 bytes
on x64; its serialized representation remains 56 bytes on both architectures.
The complete zone save remains 35808 bytes and round-trips byte for byte.

Validation
----------

Sixteen exact original/C comparisons cover Clang/GCC, O0/O2 and default,
vanilla, MASTER and SQUEEZE_PERFORMANCE configurations. Eight native MSVC
comparisons cover those configurations on x86/x64 with whole-program
optimization. Four trapping undefined-behavior and float-cast-overflow
sanitizer comparisons cover the same configuration branches.

Each comparison runs 31488 operation groups, using the exact saved C++ records,
class, inline accessors, definitions and complete method bodies as its baseline.
All 36 migrated operations and accessors run through an actual C11 caller.
The fixture rotates operation order, mutates live map inputs at the player
query and retains state in a final pass. It observes every zone/density record,
hierarchy pointer identity, label cursor, count, clock result, day/night output
aliases, optional save
validation count, serialized byte and returned text pointer. Fresh links provide
the original insertion precondition for repeated hierarchy operations.

Independent checks verify record/enum widths, exact query edges, nested zone
selection, duplicate-label continuation, null navigation indices, the fixed save
size and byte-for-byte save/load/save output. Inserting inner zones before their
enclosing zone exercises moving existing children to a new parent. Five extra
updates per sequence cover both normal and reduced update schedules.

All 24 changed actual C++ sources and the complete C unit compile with MSVC on
both architectures: 48 C++ and two C compiles. Every complete source/project
change, shared enum expression and record field is audited against immutable
originals. All 84 preceding source audits pass through explicit zone
transformations. Full game linking and interactive gameplay remain outside
these quiet checks.
