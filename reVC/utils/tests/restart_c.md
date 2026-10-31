RestartPoints.c owns all nine complete restart operations, thirteen canonical
variables and both eight-element point/heading tables. CRestart is removed.
RestartPoint is a plain three-float C record. The public C API passes native
positions by pointer through an opaque CVector tag; C++ callers retain their
actual CVector objects. Seven caller files and the Visual Studio project use
the new API and its independently named C11 translation unit.

Three native boundaries copy named vector components, assign a real CVector
output and perform the original path search/node conversion. They contain no
restart selection, override or save decisions. The path lookup retains the
actual by-value CVector argument, PATH_PED, 999999.9f limit and four default
false flags. The selected CPathNode uses its actual fixed-point conversion.

Initialization, insertion and override consumption retain their original write
order. Selection retains live counts/tables, captured initial level, preferred
level, fallback to any level, 4000-squared bound and later-wins tie comparison.
Native input coordinates remain live across external zone/path queries. Output
and heading aliases remain valid in the same cases as before. Hospital fallback
formats its diagnostic after output writes; police fallback prints before path
lookup. No capacity, pointer or path-index guards were introduced.

The save format remains exactly 292 bytes, including its eight-byte header,
interleaved point/heading arrays, counts, flags, explicit three reserved zero
bytes, override fields and level bytes. Loading initializes first and skips
the reserved bytes. Transfers use the existing C SaveBuf API. Actual Win32 and
Win64 native probes verify point sizes/offsets, scalar types, capacities,
public signatures and all three backend signatures. A standalone C11 probe
checks the public headers and matching opaque native tag.

The source fixture compares eleven entire changed source/project files and
both new production files with explicit transformations of immutable originals.
Unchanged native dependencies retain byte guards. The earlier zones fixture
verifies the complete relocated original Restart.cpp algorithms as well as
the current native boundary. The migration chain forwards later edits without
discarding its prior complete-source coverage.

Runtime programs compile the complete original methods and the actual C kernel.
They use actual CVector objects, the complete original CPathNode record, actual
zone containment/level algorithms and the production SaveBuf API. Named-field
adapters observe canonical state without reading padding. A separate C11 caller
enters every public operation. Diagnostic interception only redirects printf
after stdio declarations; formatting still uses vsnprintf.

Each run compares 624 states and 2596 external calls. Cases cover both restart
categories, counts zero through eight, every level, level overrides, equal
distances, exact distance limits, override consumption/cancellation, empty or
distant tables, native input/output and heading aliases, and changes to live
input/table/count/heading data during external queries. Diagnostic callbacks
exercise the different police/hospital ordering. Four save-buffer alignments
check every serialized byte and surrounding guards; loading ignores altered
reserved bytes. Test inputs use valid bool representations, point capacities
and path indices, matching the original API's preconditions.

Twenty Clang/GCC comparisons cover O0/O2 with default, plain, MASTER, unfixed
and GL profiles. Five Clang runs enable undefined-behavior, float-conversion
and indirect-function-type traps. Seven optimized MSVC comparisons cover
Win32 default/GL/MASTER/vanilla and Win64 default/GL/MASTER. Every comparison
has the same result hash: 7edcc64912e931c8. Forty-five production compilations
cover the native backend, all seven caller files and the C11 kernel across
five native profiles. Actual x64 D3D9 Debug and Release project builds link.
The object-name fixture checks all 50 configurations and all 28 C/C++ pairs.
The preceding 132 complete-source audits are also checked.

Run utils/tests/test_restart.ps1 for portable comparisons. Add -Sanitized,
-Native, -Consumers, -BuildProject or -AuditPrevious for the respective checks.
The final evidence checker is build/finalize_restart.py. These are quiet
compiler and console checks; they do not launch the game or exercise graphics.
