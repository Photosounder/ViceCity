BrightLights.c removes both CBrightLight and CBrightLights. It owns the complete
four public algorithms, the pass-camera helper, both thirty-two-record lists,
both flicker arrays, local reflection backups, sixteen float geometry tables
and both triangle-index tables. All distance, fade, alpha, matching, merging,
restoration, random-phase and geometry computations execute in C11. The C
SpecialFX controller calls the C manager directly; its two former C++ forwarding
functions are removed. Vehicle, traffic-signal and custom reflection users are
adapted at their original call points.

Actual MSVC Win32/Win64 probes verify the original 56-byte record aligned to
four: position at zero, up at twelve, side at twenty-four, front at thirty-six,
distance at forty-eight, and type/RGB at fifty-two through fifty-five. C11
assertions verify the same layout in every comparison. All thirteen original
type values and both front/rear aliases are checked against the unchanged enum.
Legacy callers retain their four by-value CVector arguments and original zero
color defaults before entering the same C registration algorithm.

Initialization still clears only both list counts. Full-pool registration returns
before the camera query, and distance rejection still retains the distance write.
Registration explicitly finishes the original C++ assignment's right operand
before selecting its live destination; owner callbacks can change the count or
reenter registration. Camera origins retain the original fallback and repeated
RenderWare frame/matrix calls. Matrix fields are read through actual typed owners.
Main-camera position, right and up copies remain at their original use points,
including their queries before an empty renderer returns.

Reflection rendering keeps the original saved-main-list lifetime, cached flicker
matching by type and squared position distance, missing-lamp merge, range checks,
capacity gate, fallback list and final queue restoration. Both early and later
reflection-flag reads remain distinct. Main-pass random draws and writes into
both flicker arrays retain their original order; reuse passes generate no new
phase. Auxiliary right/up normalization uses the exact original reciprocal-square-
root arithmetic and x-only zero fallback. All table products, left-associated
vector additions, integer alpha conversion and final byte narrowing are preserved.

The original strict buffer thresholds, state calls and restoration, vertex/color
write order, twelve/36-index patterns and live counter increments remain in C.
UV fields stay untouched. The index-count flush gate, LittleTest hook, successful
transform/render/end path, post-owner live reads and clears on failure remain.
Eighteen typed backend operations expose game and RenderWare owners; three
existing RenderBuffer backend functions handle transform/render/end. The actual
shared storage, including separate 2D and 3D arrays, stays unchanged.

bright_lights_fixture.py audits all ten entire changed source/project files,
all methods, the camera helper, complete tables and exact headers/backend.
Unchanged original methods, CVector operators and Normalise body, actual typed
vertex methods and the original RenderBuffer.cpp backend participate in the
comparison. Only private access is exposed in the oracle declarations; fields
and method bodies are unchanged. Separate C11 callers and real C++ legacy/default
argument adapters exercise both public input paths. Native comparisons use
actual project and RenderWare headers.

Each default run compares 2,366 complete list/flicker/shared-buffer snapshots,
50,422 owner queries, 27,563 random draws, 3,599 transform attempts, 2,857 successful
draws and 831,812 vertex-field writes across 1,500 mixed frames. Cases include all
pool capacities and light types, exact fade/alpha and buffer boundaries, partial
initialization, rejected distance writes, legal nonfinite registration inputs,
camera/frame/matrix null fallbacks, near-duplicate matching thresholds, cached
missing and out-of-range lamps, capacity-limited merges, original input aliases,
live camera/count/field changes, zero basis normalization, recursive registration,
buffer flushing and reflection rendering. Both arrays, inactive records, both
flicker lists, every vertex/index, pending counts and untouched 2D bytes compare.
Finite float bits compare exactly and NaNs compare by classification. Undefined
render-time nonfinite alpha conversions and unfixed invalid-type cases are not
invoked; the original configuration branches remain unchanged.

Twenty Clang/GCC O0/O2 comparisons cover default, non-librw, GL3 vertex packing,
disabled pipelines and unfixed configurations. Five additional comparisons trap
undefined behavior and float-cast overflow. Five optimized MSVC/LTCG comparisons
cover Win32/Win64 D3D9/GL3 plus vanilla Win32. Forty actual production compiles
cover all affected callers, both C units and the backend. Debug and Release x64
D3D9/OpenAL projects compile/link with existing dependencies and Git build events
disabled. No game is launched. Controller sanitizer regressions also pass.

All fifty project configurations preserve unique object paths, including the
twenty-third C/C++ pair, BrightLights.c.obj and BrightLights.cpp.obj. All 108
preceding whole-source migration audits pass. Immutable originals, commands,
logs and comparison records are under build/bright-lights-c-tests. Quiet checks
are exposed by test_bright_lights.ps1; build/finalize_bright_lights.py verifies
the recorded final evidence.
