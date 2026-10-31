PlaneTrails.c owns all seven original plane-trail record/controller methods,
the six complete 256-byte records, all 96 positions and signed timestamps, and
the mutable 32-entry line index array. CPlaneTrail and CPlaneTrails are removed.
Fluff.cpp, Plane.cpp and the environment-map renderer enter the C API. The
actual plane position is copied once through named CVector fields; C callers
can register a plain PlaneTrailVector directly.

Twenty-one typed boundary operations retain camera visibility, native shared
vertex setters, render states, immediate rendering, weather/sky reads and corona
calls. The existing shared Fluff sphere-visibility helper remains with its other
C++ users. The camera visibility routine copies its input and does not retain
that reference; its complete source is held unchanged by the audit. No vector,
native vertex or camera representation is type-punned into a C record.

One actual CVector temporary stays alive at the C++ boundary throughout each
procedural update. C addresses its declared float fields, performs all three
flight-path calculations, captures registration values, and submits the original
corona calls through the same borrowed vector. Reentrant updates have independent
temporaries. This retains vector identity and lifetime without moving any of
the update algorithm back into C++ or allocating temporary objects on the heap.

Initialization clears only timestamps. Registration retains the strict unsigned
2000-millisecond test, descending shift, by-value input capture and conditional
timestamp refresh. Rendering retains sphere/zero-alpha early exits, signed
elapsed-time conversion, strict 30000-millisecond expiry, fade/alpha narrowing,
per-point live reads, color-before-position writes, original native transform
flags and mutable index borrowing. Reflection takes the original early flag
snapshot and leaves expired timestamps unchanged. The visibility expression
retains the original Min/Max macros and repeated sky-channel getter evaluations.

plane_trails_fixture.py audits all six complete changed source/project files,
the three new C files/headers, the exact typed boundary and all seven unchanged
original method bodies. Its original oracle compiles the real original record
and controller declarations with test-only field access. Observers copy each
declared field independently. Public operations also enter through a separate
C11 translation unit. Production contains no instrumentation.

Default comparisons observe 18,110 state snapshots, 127,038 owner calls, 1,081
registrations, 953 updates, 1,161 render entries, 4,892 transforms, 4,435 indexed
draws and 1,129 corona calls. The corpus covers all hours/blink phases, exact
expiry/fade/shift boundaries, zero timestamps, timer wraparound, partial init,
zero/one/sixteen vertices, transform rejection, alpha narrowing, same-record
input aliases, all sky/weather gates, native shared-buffer and index mutations,
nested rendering, nested updates, borrowed corona vector identity and 900 mixed
frames with retained records. Every declared affected native vertex field, all
indices and every record field compare exactly.

Sixteen Clang/GCC O0/O2 comparisons, four undefined-behavior/float-conversion
trap comparisons and five MSVC/LTCG Win32/Win64 D3D9/GL3/vanilla comparisons pass.
GNU/Win64 default fingerprint: 94b32c7f47b0130f; plain: 5e175932681ff5f6.
Win32 matches its own original at 55d85cfe295e792b, and vanilla at
e30691f652d0b29e. Original/C results compare within each compiler/profile.
Native probes check the complete original record size/alignment/field offsets,
actual float pointers, index type, weather/sky types, corona overloads/default
enum values and native vertex setter signatures.

Twenty production compilations, Debug/Release x64 D3D9/OpenAL builds, all fifty
evaluated project configurations and 116 distinct preceding whole-source audits
pass. The new compilation unit has a unique name; all 28 existing C/C++ pairs
retain their separate objects. No game is launched. Immutable originals and
evidence are under build/plane-trails-c-tests. test_plane_trails.ps1 runs quiet
checks; build/finalize_plane_trails.py verifies the final recorded evidence.
