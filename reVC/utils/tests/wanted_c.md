`WantedSystem.c` contains the complete wanted policy, its two canonical maximum-level
globals, explicit value initialization and replay copy. `Wanted.h` and `Crime.h` are
plain C records. All 24 original operations, plus initialization and copy, have C11
interfaces. `Wanted.cpp` retains 18 typed primitives for external owners that still
use C++ records, along with two native vector input adapters.

Native probes confirm the actual original wanted record is 528 bytes on x86 and
568 bytes on x64. Queued crimes remain 28 bytes. Every non-bit-field offset,
alignment, array location and each of the five bit positions matches. The existing
optional `VALIDATE_SIZE(CWanted, 0x204)` check is retained; the actual original record
is larger than that old check. Default builds do not enable that check.

Crime inputs keep live references. `WantedPositionRef` points either to the native
vector or to the canonical C vector. Queue insertion reads it at the original copy
statement. Audio samples it after the original music-mode, player/wanted and
cooldown checks. The new C `DMAudio_ReportCrimeAt` entry shares the entire existing
eligibility policy and reads plain coordinates at that same later point. No native
vector is reinterpreted as a different C record.

The tests compare whole immutable original and C implementations, including the
complete original/current audio entry files and real C pool and audio-crime
primitives. Each profile covers 10,078 complete state snapshots. Cases include all
crime multipliers and wanted thresholds, duplicate/full queues, strict expiry and
wraparound, repeated zone-drop reads, live coordinate mutations, ignored/reinforcement
flags, reverse pool traversal, cop state writes, pursuit-list repair, suspension,
value initialization and replay copying. Tests exercise both settings of
`FIX_SIGNIFICANT_BUGS`, Clang/GCC with and without optimization, native x86/x64 MSVC
and undefined-behavior instrumentation.

`test_wanted.ps1` repeats the quiet source/behavior/native checks using originals
under `.diagnostics/wanted-c-tests/before`. Diagnostic executables use static
compiler runtimes and portable runs use only Windows directories on PATH. Full
D3D9 builds are generated outside the user's disposable `build` directory; no
game executable is launched.
