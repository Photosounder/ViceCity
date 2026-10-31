# C transmission

`Transmission.c` replaces the complete transmission class with a 96-byte C record
and its three original operations. Its six 12-byte gear records preserve their
original layout. `HandlingFlags.h` relocates all 28 unchanged handling flag values
without importing the C++ handling manager. No transmission bridge was added.

Gas, velocity, gear and time reference arguments become pointers. This preserves
live aliasing into the transmission, including gas bound to current velocity and
gear bound to its flag byte. Recursive shifts retain the shared function statics,
pass the private velocity by address and cancel the cheat flag at every recursive
call. The original integer division in the gear multiplier remains integer
division. The time argument remains unused. Time-step reads use the existing C
timer accessor directly.

Gear initialization still clears only the gear array before setting ratios and
reverse limits. Nonpositive gear counts skip its forward loop. Simple-car shifts
retain their strict comparisons, reverse floor and unclamped upward increments.
Acceleration's out-of-range exits leave current velocity unchanged, while valid
range processing updates it at the original point. Debug and MASTER retain their
different assertion contracts. Invalid physical array indices remain outside the
original defined behavior; no new clamps were introduced.

Five audio C producers now read the public transmission fields directly. Their
six obsolete native readers and declarations were removed. The migration first
verifies that each removed reader is exactly its original pure field expression;
calls remain individual field expressions without cached snapshots. Full native
compiles and layout/type checks verify the C consumers against real handling and
audio structures.

Quiet differential checks compare the literal original implementation with C
through independent C11 callers across 85,741 states and 183,520 checks per
profile. Coverage includes every normal gear, shift thresholds and adjacent float
values, reverse/forward transitions, boost flags, cheat cancellation, time steps,
reference aliases, ignored flags, signed zero, quiet NaNs and untouched metadata.
A separate subprocess verifies debug assertion and MASTER continuation behavior
without windows or out-of-bounds storage access. The actual timer accessor and
its scalar storage are used without artificial callbacks.

Clang/GCC O0/O2, undefined-behavior checks and native MSVC x86/x64 agree. Native
headers verify all field offsets/types, trivial lifetime, the 220-byte handling
record and the 28/40-byte audio vehicle context. Every affected producer compiles
on both architectures. All 48 generated configurations compile `Transmission.c`
as C11 and retain unique object names for the 28 existing C/C++ source pairs.
Both complete D3D9 configurations compile and link without launching the game.
Earlier conversion audits still pass.

Checks live under `.diagnostics/transmission-c-tests`. Run
`utils/tests/test_transmission.ps1` for the quiet behavior and native checks.
Use `events_project.py --transmission`, `check_events_project.py --transmission`
and `build_events_project.py --transmission` (plus `--release`) for the isolated
full project. Regenerate project files for the `Transmission.cpp` to
`Transmission.c` rename.
