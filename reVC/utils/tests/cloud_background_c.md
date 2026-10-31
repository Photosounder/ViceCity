Clouds.c now owns all seven cloud methods. RenderBackground, RenderHorizon,
their eleven original helpers, five private coordinates, the six-float safe
horizon cache and its validity flag join the existing C cloud algorithms.
CClouds has been removed. The main rendering loop calls the C API directly.

Thirteen additional typed boundary operations read the actual camera basis,
matrix coefficients, viewport, debug flag, capture state and ambient channels,
and submit the original drawing operations. Three actual CRGBA objects remain
in Clouds.cpp because CSprite2d borrows references to those objects. C reads
and writes their declared byte fields through typed pointers. The draw calls
receive those same live objects, including repeated aliases, after blend-state
callbacks. No C++ color, vector or matrix representation is type-punned.

The conversion preserves matrix multiplication and translation order, signed
near-zero depth clamping, capture-dependent screen-Y inversion after projection
stores, camera-roll normalization, strict dark/pole gates, safe-horizon cache
retention and smoothstep blending. A pole draw rereads the live safe cache after
the first gradient pass and restores its saved current coordinates afterward.
Color channels narrow before alpha scaling. Horizon bands retain all original
geometry, blend ordering, ambient conversions and actual color references.

cloud_background_fixture.py audits all six complete changed files against an
explicit migration of immutable originals. It compiles the two original public
methods and all eleven original helpers, alongside the preceding five-method
effects oracle. Both variants use the actual original vector normalizer, cross
product, matrix-vector product, matrix lifetime operations and CRect constructor.
Independent C11 callers exercise the new API. Private observers address real
fields individually; production has no test instrumentation.

Each comparison includes the full preceding effects corpus, then 945 background
and 925 horizon entries, 3,220 band draws, 232 rectangles and 5,856 cumulative
state snapshots. Capture profiles observe 2,680 capture queries. Cases cover
exact dark/pole gates, three viewports, debug flicker, camera roll, nonorthogonal
matrices, signed near-zero depth, signed channel narrowing, all six mirror
expressions, live color/cache/viewport mutations and nested background/horizon
calls. Seven hundred sustained frames retain the actual safe-horizon cache.

Twenty Clang/GCC O0/O2 comparisons, five undefined-behavior/float-conversion trap
comparisons and five MSVC/LTCG Win32/Win64 D3D9/GL3/vanilla comparisons pass.
The background fingerprint is 12dc6c8c0fbe8f7d in default profiles and
ced714194c7d0afc for GNU/Win64 capture. Win32 capture matches its own original
at 455c5747b249502d; vanilla matches at 9b7c0122b4f21dbd; unfixed matches at
c7a9807b15fc9310. Fingerprints compare each variant to its own original build.

Twenty-five actual production compilations, native signature/field probes,
Debug/Release x64 D3D9/OpenAL builds and all 115 distinct preceding whole-source audits
pass. Fifty project configurations retain unique objects for all 28 C/C++ pairs.
No game is launched. Evidence and immutable inputs are under
build/cloud-background-c-tests. test_cloud_background.ps1 runs quiet checks;
build/finalize_cloud_background.py checks the complete recorded evidence.
