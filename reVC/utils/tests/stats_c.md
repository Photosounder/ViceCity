`StatisticsSystem.c` contains all 38 former `CStats` operations and all 91 canonical
state objects. `Stats.h` is C compatible, and every caller uses the explicit
`Stats_` namespace. `Stats.cpp` retains only 16 typed accesses to native player,
menu, font and scratch-buffer providers. Shared favorite-radio storage remains
owned by the existing C music subsystem.

All save and load copies retain their original sequence and widths: 88 packed
fields occupy exactly 595 bytes on x86 and x64, without alignment padding. Tests
independently construct the expected payload, check unaligned destinations and
guards, preserve checkpoint updates and radio callback timing, and verify that
the original length assertion occurs after load copies. Omitted initialization
and persistence fields retain their original behavior.

The complete rating thresholds, progress calculation, formatting operations and
conditional display-row policies remain in C. Function-local display caches
persist across initialization and load. Language aliases use the existing C text
constants. Native player references retain the original captured player identity.
Integer divisions, repeated macro operands and floating-point boundaries retain
the original expressions.

Whole original/C comparisons cover every operation, all state objects and shared
scratch buffers, packed saves, display rows, translations and live callbacks.
Clang and GCC run at O0/O2 across five profiles: fixed precise measurements,
classic, meters, classic extra languages, and MASTER. Native MSVC runs all five
on x86/x64; undefined-behavior checks run all five too. Each original executable
must match its corresponding C executable exactly.

Native probes verify every field type, array extent and operation signature.
All changed consumers and the C implementation compile on x86/x64. All 48 generated
MSBuild configurations compile the new object as C11 and preserve the 28 existing
paired C/C++ object names. Full D3D9 Debug and Release builds link successfully.
Wanted and rampage comparisons also pass using their unchanged observed
statistics providers behind test-only name aliases.

The full builds also exposed a Windows `far` macro collision in the external
rouziclib implementation. `rl.c` now saves and restores `far` and `FAR` around
that include, preserving FAR's empty qualifier expansion inside it. The external
library files are unchanged. The precise integration change is checked against
its saved original separately from the statistics relocation.

Run `utils/tests/test_stats.ps1` for quiet source, behavior and native checks.
Saved originals under `.diagnostics/stats-c-tests/before` are required. Full-build
helpers accept `--stats` and store their projects and logs in that same diagnostics
directory. Portable checks link static compiler runtimes and run with only Windows
directories on PATH. No game or GUI executable is launched.
