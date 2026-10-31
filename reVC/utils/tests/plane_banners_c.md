PlaneBanners.c owns all eight original banner methods, both camera helpers,
the five complete 96-byte records and all forty positions. CPlaneBanner and
CPlaneBanners are removed. Fluff.cpp, Plane.cpp, the environment-map renderer
and replay startup enter the C API. Actual application positions are captured
once through named CVector fields; C callers can pass a PlaneBannerVector value.

Twenty-two typed boundary operations retain actual camera/frame/matrix queries,
native vertex setters, render states and immediate rendering. C owns the
reflection-camera selection and null gates, including both original matrix
queries and copying the second returned matrix. It also owns the reflection
sphere bypass. The remaining shared C++ sphere helper still serves smoke and
plane trails; the old C++ GetEffectCameraPosition helper has no remaining user
and is removed. C reads the anchor only after all camera queries finish, as the
original vector subtraction did. No vector, matrix or vertex record is punned.

Initialization retains all eight original positions. Registration changes only
the first position after its independent by-value capture. Update preserves
the strict -50 height gate, the downward step and -100 clamp, and continued
segment processing after clamping. Every overlong link retains the original
subtraction, square-root sum, per-component division, multiplication by eight
and final addition. The original vector operators are held unchanged by audit.

Rendering retains the strict 300-distance gate, 250-distance fade, integer alpha,
all live counter reads, original color/position/UV call order and canonical
shared indices. The original duplicated first vertex pair, fourteen vertices
and thirty-six indices remain unchanged. It queries the current C-owned rubbish
texture through the existing raster boundary. Fixed/unfixed transform flags,
transform failure cleanup, all state changes including duplicate vertex-alpha
disable, and final counter resets retain their original order. Reentrant owner
calls can change counts, positions, indices, texture slots and native vertices;
the C algorithm observes them at the original subsequent reads.

plane_banners_fixture.py audits all seven complete changed source/project files,
the three new source/header files, exact typed boundary, all eight unchanged
original method bodies and both unchanged original camera helpers. The original
oracle uses the real record/controller declarations with test-only field access.
Observers address each actual field separately. Public operations also enter
through a separate C11 translation unit. Production is uninstrumented.

Default comparisons observe 11,033 state snapshots, 251,826 owner calls, 870
registrations, 811 updates, 867 render entries, 3,438 transforms and 3,252 indexed
draws. Cases cover exact activity/fade/distance/link-length gates, zero-length
links, varied time steps, clamping, all auxiliary-camera null gates, two different
matrix results, camera-query mutations of the anchor, live counters and resource
slots, same-record input aliases, nested render/init/update callbacks and 750
mixed frames with retained records. Every record field, all 1,024 shared indices,
all affected native fields and retained tail vertices compare exactly.

Sixteen Clang/GCC O0/O2 comparisons, four undefined-behavior/float-conversion
trap comparisons and five MSVC/LTCG Win32/Win64 D3D9/GL3/vanilla comparisons pass.
Default/GL fingerprint: c3fc9de3e2f7354c; plain: 6d7ea747bc4e7fb1;
unfixed: e9d7e42b5e366fb0; native vanilla: 2d716d070826ebac.
Each result matches its own original compiler/profile. Native probes verify
record layout, registration signatures, shared counter/index types, frame/matrix
query signatures, position scalar types, vertex setters and the existing raster
boundary and texture slots.

Twenty-five production compilations, Debug/Release x64 D3D9/OpenAL builds, all
fifty evaluated configurations and 117 distinct preceding whole-source audits
pass. The new C unit has a unique name and all 28 existing C/C++ pairs retain
separate objects. No game is launched. Evidence and immutable inputs are under
build/plane-banners-c-tests. test_plane_banners.ps1 runs quiet checks;
build/finalize_plane_banners.py verifies the final recorded evidence.
