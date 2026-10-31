`CWindModifiers` is removed. Both complete algorithms, their 16-element storage
and the live 32-bit count now reside in WindModifiers.c as C11. The two methods,
private class members, vector operator expressions, two-dimensional conversion
and source-only default argument are replaced with C records, explicit scalar
operations and C entry points. Six caller expressions preserve their original
position evaluation, branch placement, count reset and output pointers.

The two functions remaining in WindModifiers.cpp copy an actual typed CVector
and query the actual camera position. The caller expression passes the address
of its original GetPosition result once. The C algorithm receives a value copy;
no CVector layout is assumed by C. This preserves the player's short circuit and
the same-pointer x/y output call. The camera query remains the second operand
of the capacity check, so a full array makes no query. These typed readers remain
C++ until CVector and the camera owner are converted.

Native probes establish the original vector is 12 bytes/alignment 4 and the
original record is 16 bytes/alignment 4. Original/C runtime assertions establish
position at offset 0 and signed type at offset 12. The static storage starts at
zero and retains inactive records when the count resets. The original empty
CVector constructor has no additional initialization behavior.

All three vector subtractions, three multiplications and three divisions retain
their original operation order. Magnitude uses the original float sum and sqrtf.
On MSVC, rouziclib redirects Abs/fabsf to fast_absf; the C unit retains that same
external call. Elsewhere it retains fabsf. Native fixtures compile the complete
original fast_absf C function from an immutable snapshot of its external source,
rather than substituting a CRT operation or stub. The actual uint16 RNG return
type retains integer promotion before masking/subtracting 16.

Registration still requires capacity below 16 and camera distance strictly below
100. Lookup still filters type 1, applies abs(15 + query.z - source.z) below 40,
requires distance below 50, uses the original fade threshold at 20 and overwrites
direction for each match. The last matching source wins. Exactly one RNG draw
occurs after any successful search. Output writes remain x first then y, including
when the pointers alias. Failure leaves outputs unchanged. Zero-distance sources
retain their original NaN arithmetic, and a negative count skips lookup without
changing outputs. Undefined negative-index registration and out-of-range counts
are excluded from tests; no new capacity or exceptional-input behavior is added.

Four Clang/GCC O0/O2 comparisons and two Win32/Win64 O2/GL/LTCG/OPT:REF comparisons
pass against the complete original methods, real original vector operators and
float wrappers. Each covers 37,123 queries, with 18,714 RNG draws and 65,862 native
or 71,007 portable independent checks. The portable camera getter additionally
counts lazy queries, accounting for the different check totals. Every run yields
the same result fingerprint 791de7b5efa6baac. A Clang O1 comparison traps undefined
behavior and float-cast overflow. Finite outputs are compared by exact bits;
NaNs are compared by classification without claiming sign/payload preservation.

Coverage includes strict nextafter boundaries, failed and full registration,
startup records, resets retaining storage, type filtering, exact simple expected
directions, last-match precedence, shared output pointers, outputs aliasing the
original CVector input, all low RNG bits, zero/negative/infinite/NaN coordinates,
height/fade/radius combinations and randomized full tables. The private C array
is observed by a test accessor in the same translation unit including the actual
full production implementation. The original header exposes only access control
for the observer, with all data declarations and method bodies retained.

Native runs use actual project headers, camera/matrix/vector declarations and the
actual new readers. Fixture-only camera/matrix/spline lifetime boundaries use no
graphics resources; each case explicitly supplies the consumed camera position.
Unused rouziclib header-table functions abort if called. Unused SDL2/GLEW auto-link
requests are suppressed only in fixture linking. Seventeen production compilation
checks cover all six consumers, bridge and C kernel on both architectures, plus
the supported Win32 vanilla C kernel. No Win64 vanilla build is claimed: the
project explicitly rejects that configuration. All 94 preceding source audits
pass through the new endpoint, and ten complete source/project edits are audited
against immutable originals. Touched files use LF endings. Visual Studio compiles
WindModifiers.c as C11, and existing CMake/Premake globs include it.

Run utils/tests/test_wind.ps1 with optional -Sanitized, -Native and -AuditPrevious
switches. Originals, compiler/type logs and results are in build/wind-c-tests.
No graphical tests were run. Full game linking and gameplay remain unverified.
