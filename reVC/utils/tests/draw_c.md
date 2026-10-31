`CDraw` is removed. Its twelve operations, including seven former inline bodies,
are implemented in Draw.c as C11. Five private float values remain internal to
that unit, while the LOD distance, four fade bytes and three optional bool settings
are C globals. All 104 caller references in fourteen files use the C API or direct
C-global addresses. Debug-menu and INI settings still address live bool storage.
The six functions in Draw.cpp read the existing signed menu preference, camera
widescreen flag, platform-dependent screen dimensions, cutscene running state and
float FOV multiplier. Their owners remain C++; all drawing calculations are C.

The exact existing screen-dimension and angle macros moved from common.h to
Draw_constants.h, shared by C and C++ without duplicating profile-dependent
constants. The aspect enum values and initial 45-degree FOV, 4:3 aspect ratio,
zero clip planes/LOD/fade bytes and enabled optional bool settings are preserved.
Aspect calculation retains all signed preference branches, AUTO dimensions and
the original theatrical scaling order. FOV conversion retains double tangent
results narrowed to float before atanf, followed by the original float arithmetic.
Win32/Win64 compiler probes prove the original tan return type is double and
rouz.fov_mul is float. The portable original uses the same double tangent boundary.
The running-cutscene bypass, scaled-cache update before FOV storage, live
multiplier query, legacy unscaled FOV and proper-scaling height factor are unchanged.

The complete original implementation and inline bodies are compared with actual
Draw.c and a separately compiled C11 caller. Each run covers 3,584 combinations
of all 256 signed menu preferences, both camera flags and seven screen dimensions,
including zero and negative dimensions. Each combination exercises thirteen FOV
inputs, both cutscene states and seven live multipliers. Checks also cover startup
defaults, exact scalar setter storage including signed zero, independent aspect
expectations, live setting addresses, fade byte widths, all proper-scaling states
and the separation between the aspect and scaled-FOV caches.

Twenty-four Clang/GCC O0/O2 comparisons pass across default, legacy, aspect-only,
forced-PC-scaling, PS2 and PAL profiles. Six Clang O1 comparisons trap undefined
behavior and float-cast overflow. The PS2/PAL portable profiles test the exact
constant dimensions; they do not establish a full native console build.
Each run performs 799,238 to 845,854 checks. All finite result bits are compared.
NaNs resulting from zero-dimension arithmetic are compared by classification,
without claiming preservation of their sign or payload.

Eight Win32/Win64 O2/GL/LTCG/OPT:REF comparisons use actual project headers, owner
declarations and the actual new Draw.cpp readers. Fixture-only definitions provide
the camera/menu/matrix/spline startup and shutdown boundaries without graphics
resources. Every field consumed by Draw is explicitly controlled by each test.
The cutscene flag's test address is captured from its actual static definition;
there is no copied owner layout. These tests establish Draw behavior rather than
the unrelated owner lifecycle. Unused rouziclib header-table functions abort if
called, and unused SDL2/GLEW auto-link requests are suppressed only for fixtures.

All 112 direct native compilation checks pass: twelve implementation consumers,
the typed bridge and the C unit across four profiles and both architectures.
The broader General-consumer compilation also passes on both architectures after
the common-header edit, with 176 source compilations. Eight general-helper and
28 music-display runtime comparisons were rerun after updating their fixtures to
follow the shared constants and C ScaleY body; their prior fingerprints are
unchanged. The music-display provider fixture still models external aspect and
proper-scaling state, while the separate Draw comparisons execute the actual C
unit. The owner-initialization audit keeps its exact whole-common-header check
through the explicit Draw transformation. All 93 preceding source audits pass
through the new endpoint. Eighteen complete existing files, the C implementation,
both new headers and the relocated constants are audited against immutable originals
in build/draw-c-tests/before. Touched files use LF endings. Visual Studio explicitly
compiles Draw.c as C11; existing CMake/Premake source globs include the C unit.

Run utils/tests/test_draw.ps1 with optional -Sanitized, -Native, -BroadConsumers
and -AuditPrevious switches. Results, native logs and type probes are stored in
build/draw-c-tests. No graphical tests were run; full game linking and gameplay
remain unverified.
