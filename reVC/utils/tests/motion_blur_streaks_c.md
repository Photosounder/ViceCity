Both motion-blur streak classes are removed from SpecialFX.h. The new C11
MotionBlurStreaks.c owns all seven former methods, the four-record pool, all
twelve indices and the ordered streak geometry setup formerly inside
CSpecialFX::Init. SpecialFX retains its other effect classes and algorithms.
Geometry setup still runs after the trace UV writes and before ID-only streak
initialization, preserving the original shared-effect initialization order.

MotionBlurStreaks.h defines a plain record and functions. Actual MSVC Win32/Win64
probes establish 84/88-byte records aligned to uintptr_t: ID at zero, three color
bytes immediately after the ID, first sample array at 8/12, second array at
44/48 and validity flags at 80/84. IDs retain their full native pointer width.
Actual render indices are uint16_t and the original capacity remains four.

The C algorithms retain descending sample copies, the exact wasUpdated/
lastWasUpdated decisions, leading-sample invalidation, partial initialization,
ID-zero matching, first-free search, full-pool rejection and current-frame query
behavior. Registration has plain C vector inputs and an actual C++ by-value
legacy-vector boundary, preserving copies before pool writes. Reflection-only
temporary records use the same C record and member draw function, with explicit
typed-vector copies at their four existing sample expressions.

C computes integer segment alpha, writes the original vertex winding, owns
all index values and controls transform/render/end short circuits and state
setup/restoration. Fog color is packed in C with unsigned shifts so C11 keeps
the original C++ bit pattern without signed-shift undefined behavior. The
eighteen typed backend operations read actual fog fields, set actual RW states
and write/transform the shared four-vertex C++ allocation. No guessed RenderWare
layout or duplicated vertex arrays are introduced. The original LittleTest hook
and its effects on live records and shared geometry are retained.

motion_blur_streaks_fixture.py audits all seven entire existing source/project
files, every complete original method and the exact geometry setup against
immutable originals. The oracle compiles complete unchanged original methods
and setup assignments; only includes and private-pool observation access change.
Original vectors, vertex declarations and packed-color macro are retained.
Separate C11 callers exercise actual C functions, and the actual legacy input
boundary participates in comparisons. Native comparisons use real project and
RenderWare headers. Actual game callers compile separately and in full builds.

The corpus compares 449 complete pool/geometry states and 7,000 update/input/
render frames with sustained trails. It covers all validity masks, stale
inactive data, repeated initialization, zero/full-width IDs, full pools, real
subobject inputs, isolated local reflection records, integer alpha, failed
transforms, live record changes and initialization during state setup, palette
edges, recursive drawing and shared vertex/index mutations. Every record field,
historical sample, query result, vertex field, index and render argument is
observed. Native 32/64-bit fingerprints intentionally differ with the ID width.

Eight Clang/GCC O0/O2 and two undefined-behavior trap comparisons cover both
vertex backends. Five MSVC/LTCG comparisons cover Win32/Win64 D3D9/GL3 plus vanilla
Win32. Thirty production compilation checks cover the new backend, four callers
and the C unit in those configurations. Actual Debug and Release x64 D3D9/OpenAL
projects compile and link with existing dependencies and Git-hash events
disabled. No game is launched. Unique object paths are checked in all fifty
configurations, now including eighteen C/C++ pairs. Commands, immutable inputs
and results are in build/motion-blur-streaks-c-tests.
All 103 preceding whole-source migration audits pass. The final evidence is
checked by build/finalize_motion_blur_streaks.py; test_motion_blur_streaks.ps1
exposes portable, native, sanitizer, consumer, project and source-audit checks.
