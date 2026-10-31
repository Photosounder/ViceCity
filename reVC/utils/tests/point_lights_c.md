Both point-light classes are removed. PointLights.c owns all ten public methods
and the complete fog-billboard helper, including registration, distance fading,
object lighting, reflection-list selection/merging, ground-cache replacement,
main fog sampling and auxiliary fog geometry. It also owns the primary records,
conditional reflection records, both counts, the 32-entry cache and eight fog
sizes. PointLights.h exposes plain C records, constants and functions.

Actual MSVC Win32/Win64 probes verify the original 44-byte light record with
alignment four: coordinates at zero, direction at twelve, radius at 24, colors
at 28/32/36, signed type/fog bytes at 40/41 and the shadow flag at 42. Indices
are uint16_t and the original TWOPI expression has float type. Native math-symbol
inspection selects fast_absf, sqrtf, sinf and cosf, all retained by C. Vector
subtraction/addition, three separate divisions, dot products, reciprocal-root
normalization and its x-only fallback preserve the original operations.

PointLights.cpp contains forty typed owner operations and two legacy-vector
boundaries. Actual camera, frame, matrix, collision, sprite and RenderWare types
stay there. Ground queries use the original flags and publish the collision
height only on success. The sprite boundary retains the exact uint8/int16
narrowing signature. A buffer allocation returns an opaque vertex handle and
the actual index pointer; C computes geometry, colors, UVs and indices, and
typed setters write the returned allocation even when the buffer is reentered.

All seventeen consumers call C or the by-value vector input boundary. That
boundary preserves original input copies before camera queries. Object-lighting
inputs use a lazy reader so public-record aliases stay live across owner calls.
The shadow reader explicitly copies the new C coordinate record into a typed
CVector at its former arithmetic expression. The primary and reflection lists
retain their independent capacities, original duplicate tolerance/comparison
order and reflection-specific fades. Original debug labels remain unchanged.

point_lights_fixture.py audits all 21 entire changed source/project files and
all eleven complete algorithm bodies against immutable originals. The original
implementation compiles unchanged except for include selection and C linkage
on the fog-size table used by the common observer. Its original vector methods,
math headers and enums are retained. Native comparisons use actual common,
matrix, camera and RenderWare headers, legally constructed native camera/frame
objects and typed mock owners. Separate C11 callers exercise the C entries;
the actual C++ legacy-vector boundary also participates in comparisons.

The corpus observes 1,041 complete states including inactive light payloads,
both registration counts, all cached positions/results, ring index and fog
sizes. It covers initial-origin cache hits, failed ground queries, replacement
wrap, strict distance/capacity gates, fade boundaries, repeated live camera/LTM
queries, camera/frame/matrix null branches, all light types, directional cones,
darkening, duplicate tolerances, live object-coordinate aliases, fog weather and
cutscene gates, projection failures, complete sprite arguments, merged fog,
degenerate billboard bases and buffer reentry. Every allocated vertex field and
index is observed. Finite bits compare exactly; NaNs compare by classification.

Twenty Clang/GCC O0/O2 and five undefined-behavior trap comparisons cover default,
non-librw, FINAL, software GL3 and MASTER. Nine MSVC/LTCG comparisons cover
Win32/Win64 including vanilla Win32. Ninety-five actual production compilations
cover the backend, seventeen consumers and C unit across default/software GL3
and vanilla Win32. Debug and Release x64 D3D9/OpenAL projects compile and link
with existing dependency libraries and Git-hash events disabled. No game is
launched. All fifty project configurations retain unique object paths, now
including sixteen C/C++ pairs. Commands, results and immutable inputs live under
build/point-lights-c-tests; test_point_lights.ps1 exposes the verification steps.
All 101 preceding complete migration source audits pass. The final evidence is
checked by build/finalize_point_lights.py.
