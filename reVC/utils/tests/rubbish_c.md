Both rubbish classes are removed. Rubbish.c owns all eight algorithms, the entire
sheet pool, six sentinels, animation table, index values, visibility state and
four texture handles. Rubbish.h exposes plain C records and C functions. The
remaining C++ file contains 47 typed field/query/resource/render operations.
The separate Fluff.cpp texture consumer uses the C-owned handle through its typed
raster reader. No guessed vehicle, camera or RenderWare layout is used by C.

Actual MSVC Win32/Win64 probes verify 68/80-byte records, pointer alignment,
vector offsets 0/12, target Z at 24, signed state/type bytes at 28/29, timers at
32/36, angle at 52, visibility flags at 56/57 and links at 60/64 or 64/72.
Native math wrappers select fast_absf, sqrtf, sinf and cosf. The C implementation
retains these functions, all original float operation order, signed small-integer
promotions and the original double 1.5 lateral-comparison literal.

C controls list insertion/removal, all animation/vehicle eligibility decisions,
random draws, ground/cull queries, visibility fading and partial initialization.
It preserves the exact 3x34 animation table and saved-next list traversal. The
32/64-sheet profiles retain their original pool sizes. Every live wind, timer,
camera and vehicle read remains at its original logical point.

Rendering keeps main-camera access before the zero-visibility return, the entire
conditional auxiliary-camera/frame/matrix decision and repeated matrix queries.
The fourth texture-group raster remains conditional on the mission flag while
the fourth group still renders when the flag is false. C owns quad/index
generation and all shared-counter updates, using pointers to the actual C++
counter storage. Hook reentry therefore retains original buffer/counter effects.
State-call order and final fog/depth-write/alpha values remain unchanged. C also
controls texture push/find/set/readx4/pop, publication and ordered destruction.

rubbish_fixture.py audits every statement in all eight complete old bodies and
all eleven changed source/project files against immutable originals. Its oracle
keeps its original enum and typed texture array under test-only name prefixes;
the original index table gets C linkage solely for the shared observer interface.
Private access is exposed only for complete payload/topology observations.
Original vectors, Vector2D, math macros, actual vertex/index/enum/buffer declarations
and exact fast_absf code are preserved. Native comparisons use real common,
vector, matrix and RW headers. Legal C++ mock owners implement camera, weather,
vehicle, world and resource sinks; actual owners compile separately and in the
full project. These are quiet algorithm comparisons, not gameplay tests.

Each run compares 925 complete pool/sentinel states, every payload and link,
resource publication, all six indices, random/ground/cull/getter order, complete
vertex/index fields and render calls. Independent list checks prove membership
and reciprocal links. Cases cover reinitialization and stale payloads, all camera
null/success branches, zero visibility, fourth-group raster behavior, wind
thresholds, ground failures, all animation rows and interpolation boundaries,
timer wrap, exact frame XOR/seed behavior, speed/width/distance thresholds,
1,600 mixed frames, visibility fades, full pools and renderer reentry. Undefined
float-to-uint animation states are excluded; finite bits are compared exactly
and NaNs by classification. Compiler-specific fingerprints retain permitted
differences in original expression evaluation order.

Twenty-four Clang/GCC O0/O2 comparisons and six undefined-behavior trap comparisons
pass across default, non-librw, FINAL, software GL3 and both sheet-count profiles.
Eleven MSVC/LTCG comparisons pass on Win32/Win64 including vanilla Win32.
Sixty-three actual production compilations cover the backend, seven consumers
and C11 unit across default/software GL3/32-sheet profiles and vanilla Win32.
Actual Release and Debug x64 D3D9/OpenAL projects compile and link with existing
dependencies and Git hash events disabled. No game is launched. Distinct objects
are checked in all fifty project configurations, now with fourteen C/C++ pairs.
All 99 preceding complete source audits pass. Commands, results and immutable
inputs are in build/rubbish-c-tests; test_rubbish.ps1 exposes the main checks.
