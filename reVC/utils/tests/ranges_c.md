Range2D.c and Range3D.c replace both complete C++ implementation units. CRange2D
and CRange3D are removed. Their constructors become explicit C initialization
functions, and all six methods become ordinary C operations. No native backend
or compatibility class remains. CMake and Premake already discover C sources;
the Visual Studio source entries explicitly select C11. The old .cpp files are
deleted after preserving immutable originals.

The C records retain the original endpoint order, size and alignment: 16 bytes
for 2D and 24 bytes for 3D, with two plain float point values. The pedestrian
range pointer now names the C record and retains its pointer size/alignment.
Its existing sampling call reads the same returned x/y values before building
the same native seek position. All other pedestrian source remains unchanged.

Containment retains strict component comparisons and their short-circuit order.
Sampling computes every absolute float width, truncates each to int before the
first random draw, performs the original integer remainder, and adds the live
per-axis minimum. Draw order and the original treatment of fractional or
reversed bounds remain unchanged. Debug operations remain empty. Sampling still
requires each truncated width to be nonzero and representable as int; the old
undefined behavior for invalid sampling widths was not changed.

The fixture audits all eight complete source/project changes, including the two
file relocations, and holds original vector, math, General and RNG sources
unchanged. Original class declarations, both complete constructors and all six
methods compile as the C++ oracle. Native objects are genuinely constructed in
aligned storage, with proper destruction before reconstruction. Observers read
all private endpoint bytes without writing private members. A separate C11
caller exercises every operation and its actual point-value ABI.

Both programs link the actual shared General.c. They use complete unchanged
myrand/mysrand implementations extracted from re3.cpp, with an external provider
wrapper for observation, forced boundary values and defined live mutations.
Native tests execute the original fast_absf sign-bit implementation used by the
actual native math headers. Four fail-fast stubs cover unused interpreter
dependencies from those headers; any call aborts.

The corpus covers constructor endpoint bytes, strict faces, degenerate/inverted
bounds, signed zero and predicate-only infinities/NaNs. Sampling covers width
truncation, fractional minima, reversed endpoints, RNG boundary values, both
masked provider widths, signed seed conversion and sustained real sequences.
Mutation tests change later-axis minima and widths after width capture, and
reenter sampling in another dimension while preserving outer state. They avoid
depending on unspecified evaluation order between a draw and its same-axis
minimum read. Empty debug calls leave all records unchanged.

Twenty Clang/GCC O0/O2 comparisons cover default, plain, MASTER, regular and
plain-regular profiles. Five Clang runs enable undefined-behavior and float-
conversion traps. Seven native optimized MSVC comparisons cover Win32 default/
regular/MASTER/vanilla and Win64 default/regular/MASTER. Native probes check both
complete original layouts, all named C point coordinates, operation signatures,
the actual pedestrian pointer type, and standalone C11 headers.

Forty-five production compilations cover seven native consumers and both C
units across five architecture/feature combinations. Actual Debug and Release
x64 D3D9/OpenAL projects compile and link. Fifty project configurations retain
unique object paths for all twenty-eight remaining paired C/C++ units. All 126
preceding source audits pass. The earlier General audit follows both relocated
units and still compares their complete transformed sources; pedestrian enum
audits still compare the complete header after the explicit pointer-type edit.
No game or GUI test was launched.

Run utils/tests/test_ranges.ps1. Optional switches select Sanitized, Native,
Consumers, BuildProject and AuditPrevious. Immutable originals, generated
inputs, logs and result manifests are in build/ranges-c-tests.
