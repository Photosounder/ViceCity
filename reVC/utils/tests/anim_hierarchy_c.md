# Animation hierarchies in C

`AnimBlendHierarchy.c` owns all eleven hierarchy operations: initialization,
shutdown, naming, packed/unpacked total-time conversion, sign correction,
sequence cleanup, compression state changes, optional relocation and the compressed
flag query. All callers use the C API, including the animation manager, cutscene
code and both game memory-tidying paths. Native sizes stay 40 bytes on x86 and 48
bytes on x64, with every original field offset unchanged.

The original constructor initialized only five fields to zero/null. Static manager
arrays already have exactly that state through zero initialization, so removing
the C++ constructor needs no additional startup reset. Native COFF inspection
confirms this: the array's constructor reference and dynamic
initializer are absent on both x86 and x64 after conversion. Explicit initialization
keeps name and keepCompressed untouched. The original C++ lifetime leaves those
fields indeterminate, so runtime comparisons initialize them explicitly after
construction. GCC O2 demonstrated why pre-construction byte patterns are not a
portable observation of those original indeterminate fields; init-debug.cpp/exe
retain the diagnostic. Padding is excluded from comparisons.

The C algorithms keep the project assertion handler, its MASTER behavior, the
original conditional maximum expression, descending
time differencing, packed integer narrowing, FIX_BUGS empty-sequence behavior,
ANIM_COMPRESSION guards and first-success relocation policy. Cache removal remains
an external manager operation, called before sequence destruction and array free.
Two failed compression preconditions test the actual assertion boundary and
continuation with empty storage. A separate MASTER check verifies reporting is
disabled without changing expression evaluation. The fixture uses the actual native cache removal body and real C linked-list
operations, rather than a model of cache behavior. Buffer frees observe cache and
hierarchy state at their original boundaries.

Original/C executables compare 24,579 states and 217,114 invariants per ordinary
profile, plus 1,024 relocation cases in custom heap profiles. These include 16,384
time/sign cases, 4,096 constructor/name cases and 4,096 owning storage/cache cases.
Classic empty-frame lookups receive a valid preceding record, retaining the
original behavior without introducing undefined fixture accesses. Both animation
compression configurations and fixed/classic policies are covered by Clang/GCC
O0/O2, UBSan and native MSVC x86/x64. Diagnostics run quietly, with static GCC
runtime linkage and explicit DLL import checks.

Native production checks compile 42 complete units across three data profiles,
plus 18 compression/custom allocator checks. The optional whole Game.cpp custom
heap configuration already fails before this change: DrasticTidyUpMemory references
missing GTA III level and loading APIs. Its baseline failure is retained in
custom-heap-baseline-failure.log. Both actual changed game callers are compiled
with native headers after omitting only that identical, unchanged function from
both comparison sources. This does not establish a working full custom-heap game
build. Normal and compression Game.cpp checks use the complete source.

All 48 generated configurations retain distinct object names and C11 metadata.
Full isolated D3D9 Debug/Release builds link without launching the game. Historical
audits cover all five affected sources through 30 earlier stages, for 150 exact
projections. Run test_anim_hierarchy.ps1 after generating the isolated build reports.
Never regenerate a fixture or overlay while its compilers are running.
