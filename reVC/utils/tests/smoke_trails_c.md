SmokeTrails.c owns all six original smoke methods, the complete visibility
helper, three 860-byte records, the cigarette flag and both mutable tables.
CSmokeTrail and CSmokeTrails are removed. Fluff.cpp and the environment-map
renderer call the C API; the keyboard cheat toggles the same C-owned flag.
All 48 positions, opacity values and signed timestamps, 1,608 retained unused
bytes, seeds, 32 indices and sixteen random-table entries remain canonical in C.
The original two implicitly zero-initialized random entries remain unchanged.

Twenty-three typed boundary operations retain actual player/clump/hierarchy
queries, native point transforms, camera visibility, weather reads, vertex
setters, render states and immediate rendering. Both actual RwV3d temporary
objects stay alive at the C++ boundary throughout each complete update. C writes
their initial component values, controls all queries and registration, and reads
their live fields at the original native vertex writes. Both point transforms
retain identical input/output addresses and the same captured native matrix.
Nested updates own independent temporary objects. No C++ vector, hierarchy,
camera, matrix or vertex representation is type-punned into C data.

Initialization resets only timestamps and seeds; it retains positions, opacity,
unused bytes and the cigarette flag. Registration preserves the strict unsigned
150-millisecond shift, descending position/time/opacity copies, seed increment,
by-value input capture, conditional timestamp refresh and original density math.
Coincident points retain infinite density followed by finite Min saturation.
Rendering preserves signed elapsed-time conversion, strict 2250-millisecond
expiry, reflection-only retention, wind/random offsets and alpha narrowing.
Original sparse vertex indices remain unchanged even though the transform count
is the number of active points. No compaction or extra state cleanup is added.

The update preserves all ordered early gates, three separate player lookups,
both clump queries, head-node conversion, hierarchy indexing, actual matrix-array
pointer arithmetic, two in-place transforms and three differently opaque record
registrations. Point fields remain live through native state/color callbacks.
The shared index array is borrowed through the original polyline submissions.

The vanilla Pad compilation exposed an existing custom Kangaroo cheat call
outside the KANGAROO_CHEAT guard protecting its implementation. That optional
call block now has the same guard. Enabled builds keep the original behavior;
disabled-feature builds no longer reference an unavailable function. The exact
guard change is included in the complete Pad source audit.

smoke_trails_fixture.py audits all six entire changed source/project files,
the three new files, exact typed boundary, all six unchanged original method
bodies, the complete visibility helper and both original class declarations.
Observers address every real field independently. Public calls enter through
a separate C11 translation unit. Production contains no instrumentation.

Default comparisons observe 13,679 state snapshots, 90,058 owner calls, 305
registrations, 914 updates, 1,119 render entries, 1,510 in-place point transforms,
3,478 native transforms and 3,164 indexed draws. Cases cover every early gate,
exact shift/expiry/fade boundaries, timer wraparound, zero timestamps, zero-length
density, opacity saturation, input aliases, sparse holes, retained bytes, both
native transform outcomes, changed player owners, live tables/timers/flags,
mutated temporary vectors, nested render/init/update callbacks and 900 mixed
frames. All declared native fields and retained tail vertices compare exactly.

Sixteen Clang/GCC O0/O2 comparisons, four undefined-behavior/float-conversion
trap comparisons and five MSVC/LTCG Win32/Win64 D3D9/GL3/vanilla comparisons pass.
Default/GL/unfixed fingerprint: bbeb3b5c15c68894; plain/vanilla:
fb8c14bdae8d8daf. Each result matches its own original compiler/profile. Native
probes verify complete record size/alignment/offsets/types, flag/signatures,
player/clump/hierarchy APIs, head-node conversion, native point field pointers,
in-place transform signature, index width and native vertex setters.

Twenty production compilations, Debug/Release x64 D3D9/OpenAL builds, all fifty
evaluated configurations and 118 distinct preceding whole-source audits pass.
The new C source has a unique name; all 28 existing C/C++ pairs retain distinct
objects. No game is launched. Immutable inputs and evidence are under
build/smoke-trails-c-tests. test_smoke_trails.ps1 runs quiet checks;
build/finalize_smoke_trails.py verifies the complete recorded evidence.
