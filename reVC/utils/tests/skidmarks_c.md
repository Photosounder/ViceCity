Both skidmark classes are removed. Skidmarks.c owns all seven original algorithms,
including both registration overloads under distinct C names, the private record
array, complete 96-entry index table and opaque texture handle. The typed C++
backend retains only actual owner/resource reads, writes and RenderWare calls;
it owns the original typed vertex buffer. All eight current registration callers
use the explicit-type overload. The unused flags dispatcher is also converted
and compared against its complete original implementation.

Actual Win32/Win64 MSVC probes verify 344/352-byte records, pointer alignment,
positions at offset 0, side arrays at 192/256, pointer ID at 320, last-update at
324/328, last index at 340/344 and state/updated flags at 342/343 or 346/347.
The original index type is uint16_t, CVector2D is 8 bytes/alignment4, and native
absolute/root wrappers call fast_absf and sqrtf. C retains the original float
normalizer with its x-only fallback and the exact native absolute-value function.

Positions remain borrowed until the original three assignment points. A typed
C++ address reader accepts lvalues and temporaries without touching coordinates;
the C kernel calls a typed position reader only where the original reference was
copied. This preserves replay early returns, valid input mutations at the replay
query and full-expression temporary lifetimes. Bool pointer priority, lazy muddy
reads, aliases and output writes remain in C. No CVector storage is cast to an
unrelated C record.

Texture dictionary push/find/set/read/pop, publication before pop, release before
clearing, partial record resets and all index generation are controlled by C.
Registration preserves replay ordering, blood transitions, <=100ms replacement,
point exhaustion, first-continuation side copying, normalizer/turn operation
order, blood cutoffs, first-free selection and original stale-field behavior.
All timer/fade/alpha arithmetic retains uint32 operations and wrap. Rendering
preserves colors, endpoint precedence, coordinates, alpha narrowing, UVs, hooks,
counts, transform success conditions and final state resets even for empty pools.

skidmarks_fixture.py audits all seven complete old bodies and all ten complete
changed source/project files against immutable inputs. The oracle retains its
own original enums under mechanical test-only name prefixes to coexist with the
new C header. Its uninitialized global index table gets C linkage only for the
shared observer interface; initialization and every algorithm use are unchanged.
Private access is exposed in the oracle header solely for storage observation.
Original vector, Vector2D, CRGBA, math wrappers and actual D3D/GL vertex/enums are
preserved. Native comparisons use actual common/vector/RW headers and exact
external fast_absf code; replay, TXD and rendering sinks are quiet mocks. Actual
owner headers and all consumers compile separately and in the actual project.

Every run observes 684 complete pool states, every record field, all 96 indices,
texture publication, bool outputs and render/resource/replay event order. The
fixture covers replay early returns and legal callback mutations, null unused
muddy pointers, aliases to updated fields, all four types, 100/101ms boundaries,
length and blood cutoffs, capacity, pointer-width IDs, timer wrap, fade ranges and
strict expiry, unknown stored render types, normalization edge floats, 12,000
mixed operations, temporary caller expressions, renderer reentry, and texture
lifecycle. Every finite float bit is compared; NaNs are compared by classification.
Artificial integer divide-by-zero states and invalid array indices are excluded.

Sixteen Clang/GCC O0/O2 comparisons and four undefined-behavior trap comparisons
pass across D3D/GL and debug/FINAL profiles. Nine MSVC/LTCG comparisons pass on
Win32/Win64, including vanilla Win32. Forty actual production compilations cover
the backend, six consumer source units and C11 implementation in default and
software GL3 profiles on both architectures plus vanilla Win32. Both actual
Release and Debug x64 D3D9/OpenAL project builds compile and link with existing
dependencies; unrelated Git hash events are disabled and outputs stay in
build/skidmarks-c-tests. No game is launched. Distinct objects are verified in
all 50 project configurations, now with thirteen C/C++ pairs. Full commands,
logs, immutable inputs and results are under build/skidmarks-c-tests.
