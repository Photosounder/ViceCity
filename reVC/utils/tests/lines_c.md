The `CLines` class has been removed. `Lines_RenderLineWithClipping` and the complete
original default and `THIS_IS_STUPID` algorithms are implemented in `Lines.c` as C11.
All 48 calls in Collision, PathFind, Debug and Script5 use the C API. Both Visual
Studio project files register the C implementation and its private bridge header.
CMake and premake already collect C files automatically.

`Lines.cpp` remains a seven-function typed RenderWare bridge. Its function-local
`static RwIm3DVertex v[2]` preserves the original zero initialization, buffer identity,
unused UV fields, transform mutations, lifetime and reentrant behavior. The C code
holds an opaque pointer to this exact buffer. No vertex layout is duplicated or
reinterpreted as a C record. Converting this backend interface is still part of the
whole-project C goal.

The original complete method is compared with the actual C implementation, a
separately compiled C11 caller and the actual bridge. Both D3D9 and GL3 fixtures use
the exact original librw vertex, vector and RGBA declarations from immutable
snapshots. The native comparisons additionally compile the original method and new
bridge against the actual project headers. The native `common.h` probe confirms
that `sqrt(float)` returns double in this project; the fixture preserves that exact
resolution, and the C code retains double `sqrt` after float squares and summation.
`config.h` is included by C so the original `VANILLA_DEFINES` segment selection survives.

Validation covers 16 GCC/Clang comparisons (O0/O2, two paths, two vertex types), eight
Win32/Win64 comparisons with O2/GL/LTCG/OPT:REF, and four Clang runs with undefined
behavior and float-cast-overflow traps. Each segmented run performs 590,126 transform
attempts; each default run performs 16,747 attempts. Event fingerprints include all
scalar bits, both colors before positions, hooks, transform arguments and outcomes,
render/end order, buffer identity and UV history. Independent checks cover initial
zero state, zero-length lines, channel extraction, axis-aligned segment-count
thresholds, no draws on failure, End even after RenderLine returns false, and
default-path signed zeros, infinities and NaN payloads. Recursive hook calls exercise
the original shared buffer. Segmented test coordinates remain finite and bounded so
the original float-to-int conversion is defined.

All 21 native production compilation checks pass: six consumer/header-consumer
files and the D3D9 bridge on each architecture, the GL3 bridge with the existing
software-polygon configuration on each architecture, and the C kernel in default,
explicit segmented and supported Win32 vanilla configurations. All 89 preceding
whole-source migration audits pass through the new endpoint.

The native runtime fixture aborts if any of four unused rouziclib interpreter
dependencies from header-level function tables is called. These abort-only functions
resolve otherwise unused linker references; they do not implement any tested line
behavior. Header auto-link requests for SDL2 and GLEW are suppressed only in the
fixture, which supplies every called renderer function. Production build flags and
dependencies are unchanged.

Run `utils/tests/test_lines.ps1`, add `-Sanitized` for trap runs, `-Native` for native
validation, and `-AuditPrevious` for the previous source audits. Artifacts, logs and
immutable originals are in `build/lines-c-tests`.

These are source, object and quiet behavioral checks. Full game linking and gameplay
have not been tested, and these checks do not validate rendered images.
