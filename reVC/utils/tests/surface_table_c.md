SurfaceProperties.c owns the full 6x6 adhesion table, its loader and all eight
surface operations. CSurfaceTable is removed. The three see-through and
shoot-through helpers are ordinary C functions rather than C++ inline bodies.
SurfaceTable.h is a standalone C11 header. Collision, vehicle, physical-object,
pedestrian and game callers enter the C operations directly; collision-point
reference arguments become pointers to the same native objects.

Four small typed native readers retain the original shared 55,000-byte work
buffer, live CWeather::WetRoads float and two CColPoint surface bytes. C owns all
whitespace/comment scanning, triangular input traversal, symmetric table writes,
adhesion grouping, wetness formulas and classification decisions. The native
point is opaque in C; no structure-layout cast or duplicate point is introduced.
Existing scalar widths and SurfaceTypes.h enums remain unchanged.

The original parser's row-name handling, space/tab/comma separators, CR/LF
scanning, skipped negative tokens, initialized-zero conversion fallback and two
symmetric writes are preserved. Every table cell is overwritten in the same
order. File-owner call order, buffer reads after the load and the ignored load
result remain unchanged. Malformed input handling was not altered. All default
surface groups, unbounded wetness formulas and the distinct vertical transparency
predicate remain exactly as before.

The fixture audits ten complete modified files and both new files against
immutable originals and the explicit migration. Native collision-point, weather,
shared buffer, file-manager and enum sources remain unchanged. The original
class, private table definition, five methods and three inline helper bodies
compile in the C++ oracle. A separate C11 caller reaches every operation in both
programs. Production native readers compile from their exact source.

Each program run performs 1,115,408 adhesion queries. Snapshots cover all 65,536
surface-byte pairs, all 256 classification inputs and eight wetness values,
including signed zero and values outside the normal weather range. Every exact
returned float bit and all 55,000 shared-buffer bytes are compared. Inputs cover
all triangular table cells, mixed separators and comments, CR/LF, ignored row
names, negative sentinels, failed numeric conversion, decimal/exponent/hex-float
text and repeated loads. Callbacks mutate borrowed filename characters, return
an ignored negative load result and reenter loading before the outer parser
obtains its shared-buffer cursor. The native collision point must remain
byte-for-byte unchanged. Portable tests deliberately use different point field
offsets; native tests use the actual CColPoint definition.

Twenty Clang/GCC O0/O2 comparisons cover default, plain, MASTER, unfixed and GL
profiles. Five Clang runs enable undefined-behavior and float-conversion traps.
Seven optimized native MSVC comparisons cover Win32 default/GL/MASTER/vanilla
and Win64 default/GL/MASTER with actual project headers and collision points.
Fail-fast stubs cover four unused interpreter dependencies from those headers;
any call aborts. Native probes verify all public signatures, scalar widths,
actual point field types/offsets, weather/buffer owners and the standalone C11
header. Every comparison has the same result fingerprint across architectures.

Forty production compilations cover seven C++ consumers and the C kernel across
five architecture/feature combinations. Actual Debug and Release x64 D3D9/OpenAL
builds compile and link. Fifty project configurations retain unique object paths
for all twenty-eight paired C/C++ units. All 125 preceding source audits pass.
No game or GUI test was launched.

Run utils/tests/test_surface_table.ps1. Optional switches select Sanitized,
Native, Consumers, BuildProject and AuditPrevious. Immutable originals,
generated inputs, logs and manifests are in build/surface-table-c-tests.
