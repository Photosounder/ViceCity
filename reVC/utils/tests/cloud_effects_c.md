Clouds.c owns the complete Init, Shutdown, Update, Render and optional
RenderForEnvMap algorithms, the private reflection-quad helper, all five scalar
cloud/horizon fields, five opaque texture slots, the persistent 37 screen flags,
174 float layout-table values and eighteen rainbow color bytes. C controls all
animation, weather/time gates, sprite positions/sizes/colors, highlight math,
reflection basis normalization, clipping, quad construction, UVs and indices.
The remaining CClouds background/horizon methods, private helpers and three
typed CRGBA objects remain in C++ for the next conversion stage. Their scalar
reads/writes now address the same C-owned float variables.

The typed boundary has 48 base operations and thirteen optional reflection
operations. Native probes verify every original scalar type, all five sprite
overloads, the projection signature, mutable sun flag, camera orientation and
index width. Camera/texture/vertex handles cross as opaque pointers and are
converted back to their actual types only in the owner boundary. Vectors copy
named fields; no C++ vector, camera or vertex record is type-punned as a C struct.
The original C shared-buffer API supplies actual uint16 indices and an opaque
native vertex pointer. Typed setters retain native D3D9/GL3 vertex layouts.

Init retains push-before-find dictionary order, five individual texture
publications, pop-before-rotation reset and all other scalar/flag retention.
Shutdown destroys each current texture before publishing null, without new
guards. Update preserves the complete fixed/unfixed formulas and original
float-to-uint32 compound assignment. The fixtures use legal conversion ranges.
Main rendering retains the outside early gate, initial sun-blocked reset,
original clock arithmetic, all live owner reads and original render-state/batch
ordering. Highlighting intentionally retains the original last-computed
sundist/hilight values and the original differing Y expressions between its
two loops. Both rotation divisors/constants remain unchanged.

Reflection retains null camera/frame/matrix gates, both matrix reads, copies
position/right/up/forward before clip queries, and normalizes with the original
x-only zero fallback. It flushes pending world geometry before sky setup,
preserves deterministic star pulses, all texture/weather gates and strict depth
comparisons, and restores the original fixed world states. Quad corner algebra,
color-reference selection, UVs and six index writes retain their original order.
Background color-reference ownership is deliberately unchanged in this stage.

cloud_effects_fixture.py audits seven entire changed source/project files and
all three new files against the explicit migration. Its oracle compiles all
five original methods and the whole original quad helper unchanged. One
non-mutating binding call after each actual screen-flag declaration exposes that
real static array through a correctly typed pointer; production is uninstrumented.
Observers address each real scalar/resource/table field independently. Both
variants use the actual original CVector normalizer and native addition/vertex
accessors. Native Abs uses the original library function body and is checked
against its current source. Separate C11 callers exercise the new public API.

Each comparison observes 1,403 state snapshots, 90,789 projections, 72,899 sprite
submissions and 350 random draws. Reflection profiles additionally compare 30,318
complete native quads, including every position, color, UV and index, across
359,000 owner calls. Cases cover lifecycle order and retention, nested shutdown,
fixed/unfixed animation, every hour, varied weather, early/null gates, exact
near/far boundaries, zero camera bases/sun direction, nonorthogonal quad inputs,
same-color aliases, live camera/weather/sun/rotation changes and 900 mixed frames.
The shader sprite callbacks are deterministic and all finite float bits compare
exactly. Main fingerprint: 83a606927bc1b852; GNU/Win64 reflection:
a24dca1f3b402779. Win32 reflection matches its own original at d3692391a34e8642.

Twenty Clang/GCC O0/O2 comparisons, five trap sanitizer comparisons and five
MSVC/LTCG Win32/Win64 D3D9/GL3/vanilla comparisons pass. Twenty-five actual
production compilations and Debug/Release x64 D3D9/OpenAL builds pass. No game is
launched. Fifty project configurations preserve separate objects for twenty-eight
C/C++ pairs, including Clouds.c.obj and Clouds.cpp.obj. All 114 preceding source
audits pass. Immutable inputs and evidence are under build/cloud-effects-c-tests;
test_cloud_effects.ps1 runs quiet checks and build/finalize_cloud_effects.py
verifies the recorded final evidence.

The subsequent background/horizon conversion moves those remaining algorithms
and their private cache into Clouds.c and removes CClouds. The shared typed
color objects stay at the CSprite2d boundary. See cloud_background_c.md for the
combined seven-method comparison and current validation evidence. The effects
fixture follows that conversion so earlier whole-source audits remain usable.
