`Profile.c` and `PowerPoints.c` replace the complete former C++ producers. No
C++ bridge translation units remain for either subsystem. Their headers are
C-compatible, and the existing script caller uses the C power-point API. This
removes `CProfile`, `CPowerPoints` and the methods of `CPowerPoint`.

The profiler owns all seven original arrays and all three original operations.
Its identifiers and table sizes are unchanged. The four-byte, byte-aligned
`ProfileColor` record preserves the original RenderWare color component offsets.
Initialization resets only the two maximum arrays, all labels and the six
explicitly initialized colors. It retains the three other time arrays and the
frame-rate color. Reinitialization retains stable canonical label identities.

Suspend and result calculation retain their original float expressions and
ordered updates. The greater-than `Max` macro remains intact, preserving its
selection of the second operand for equal signed zeros and NaNs. No timing
provider or replacement profiler functionality was added. MASTER continues to
exclude all profiler definitions.

All six cut power-point operations remain no-ops. They do not advance save
cursors, write sizes, clear buffers or generate objects. The empty instance type
retains its one-byte extent through a plain C record. Tests check these no-op
contracts directly, including null pointers accepted by the static operations.

Whole original/C profiler comparisons cover 2,385 complete states in normal mode,
including static initialization, partial reset, every profile slot, repeated
suspension, all special float pairings, label relations and all color bytes.
MASTER checks cover the same cut-feature operations without linking profiler
definitions. The original full member bodies and headers are retained in the
comparison fixture; private state is exposed only in its test header.

Clang and GCC run normal/MASTER profiles at O0/O2. Clang undefined-behavior checks
run both profiles. Native MSVC runs both on x86/x64. Every original must match its
corresponding C executable. Native probes verify all profiler storage types, color
layout, instance extent and identifier values, and compile both renamed C units
and their script caller on both architectures.

All 48 generated MSBuild configurations compile both renamed units as C11 and
preserve the existing 28 paired C/C++ object names. Full D3D9 Debug and Release
executables link without being launched. The original producers are preserved
under `.diagnostics/profile-power-c-tests/before`; historical verification follows
the two exact source renames rather than exempting missing files from checks.

Run `utils/tests/test_profile_power.ps1` for quiet source, behavior and native
checks. Full-build helpers accept `--profile-power`. Portable diagnostics use
static compiler runtimes and run with only Windows directories on PATH.
