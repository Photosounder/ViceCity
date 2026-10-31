# C vehicle damage manager

`DamageManager.c` owns the complete 28-byte damage record, all 20 operations,
the seven mutable component multipliers and the replay packed-panel accessor.
No new native bridge was added. Shared panel/light identifiers moved unchanged
to `VehicleDamageParts.h`, which both the C owner and C++ vehicle header include.

The automobile constructor explicitly initializes damage before its existing
door initialization. The original constructor's partial writes are preserved;
padding is not added to its initialization contract. Reset restores the wheel
effect to `0.5f` and clears statuses while retaining `field_18`. Copy and
destruction remain trivial. Native headers verify every field type/offset and
the enclosing automobile layout on x86 and x64.

The algorithms retain their unusual original behavior: full damage changes only
wheel zero, progression compares the original missing-status values, byte and
nibble statuses can wrap, and engine progression remains a no-op. Damage scaling
uses the live global multiplier table, windscreen scaling remains a separate
multiply, and the strict 150/220 thresholds are unchanged. The unused scalar
argument remains unused. Component mapping retains its original ordered output
writes. Signed C component/light indices preserve the lookup arithmetic and the
existing windscreen-as-light-slot-four path without requiring C enum promotion
rules to match C++.

Private packed-word helpers use unsigned arithmetic so high-nibble bit patterns
retain the original C++ results without introducing signed C shift overflow.
The original classic full-damage branch still passes component identifiers
16/17 as panel indices and therefore exceeds the 32-bit shift width. That
preexisting undefined path is retained; portable and UBSan comparisons omit that
single classic operation. Native MSVC comparisons execute it and match the
original observed x86/x64 results. The old light enum also excludes the
windshield slot used by the original algorithm, so enum sanitization is disabled
for these original/C comparisons; the remaining undefined-behavior checks run.

Independent strict C11 callers compare every original method, the original bit
helpers, constructor behavior and mutable coefficient table. Fixed profiles
compare 19,789 states; portable classic profiles compare 19,533, and native
classic profiles include the 256 full-damage cases for 19,789 states. Coverage
includes all byte statuses, all packed slots and unrelated-bit preservation,
status narrowing, engine clamping, aliased component-map outputs, threshold
values, infinities/NaNs and seeded 28-byte raw save transfers. Constructor tests
use nonzero storage, and source guards retain the empty vector/col-point
construction assumptions crossed by explicit initialization.

Clang/GCC O0/O2, defined-path UBSan checks and native MSVC x86/x64 agree. Every
affected producer compiles against real native headers on both architectures.
All 48 generated configurations compile the renamed producer as C11 and retain
distinct object names for the 28 existing C/C++ source pairs. Both complete
D3D9 builds compile and link without launching the game. Earlier conversion
audits still pass.

Checks live under `.diagnostics/damage-c-tests`. Run `utils/tests/test_damage.ps1`
for quiet behavior/native checks. Use `events_project.py --damage`,
`check_events_project.py --damage` and `build_events_project.py --damage` (plus
`--release`) for the isolated full project. Regenerate project files for the
`DamageManager.cpp` to `DamageManager.c` rename.
