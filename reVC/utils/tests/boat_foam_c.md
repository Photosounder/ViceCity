BoatFoam.c removes CSpecialParticleStuff and owns all three complete foam and
animation methods, the private boat timestamp and all eight function-local
animation scalars. All rejection, random narrowing, timer arithmetic, frame
selection, position and matrix-vector calculations execute in C11. Script call
names are updated within their existing disabled command block. The original
typed color object remains in BoatFoam.cpp so the particle owner continues to
borrow the same live CRGBA-to-RwRGBA reference during its callback.

Actual MSVC Win32/Win64 probes verify the original timestamp width, float width,
color conversion and exact particle overload signature. Seven typed backend
operations expose the original camera, matrix fields and particle owner without
assuming C++ layouts. Matrix transformation copies twelve actual named float
coefficients to a C record and performs the original products and additions in
C. The particle call retains the exact enum, zero direction, null entity, size,
live color, rotation speed one and three zero flags. Its return value stays
ignored, so an allocation failure retains the original emission-count behavior.

The random helper preserves both original signed-byte conversions and their
multiply-then-divide order. Both coordinates intentionally use innerFw; innerRg
participates in the later rejection test. Exhaustive comparison caught and
corrected an initial transcription that scaled the second coordinate differently.
The per-attempt camera dot product reads only the original x/y components.
Every original matrix-position and basis read stays live after RNG/camera or
particle callbacks. Native Abs uses the actual fast_absf implementation and
portable profiles use the original float fabsf behavior.

CreateFoamAroundObject retains the one-thousand-attempt limit, zero/negative
particle early path, all original rejection gates, position-z replacement and
post-callback decrement. StartBoatFoamAnimation still writes only BoatFromStart;
it does not reset the separate persistent animation state. Update first creates
ambient foam, then reads previous/current time. Unsigned subtraction and
multiplication, thirty-Hz integer frame quantization, repeated-frame suppression,
expiry after frame fifteen, zero start sentinel, timer-mask rollover and all
RNG/state writes retain their original order. The last-frame write remains after
the particle callback, including when that callback reenters the animation.

boat_foam_fixture.py audits all five entire changed source/project files, all
three complete algorithms and exact typed backend/headers. Original method
statements and static declarations remain intact. Each test variant adds only
an observation binding call before Update's first original operation, exposing
each static through its own correctly typed pointer; the production source has
no such instrumentation. Observers never assume adjacent static storage. The
actual CMatrix constructor/destructor, CVector operators, DotProduct2D and
original matrix-vector operator participate in the comparison. The actual
native sign-bit absolute-value body is checked against its library source.
Separate C11 callers exercise the real public API; native tests use actual
project and RenderWare headers.

Each run observes 1,715 complete persistent-state/matrix/color snapshots,
184,940 random draws, 184,872 camera queries, 3,872 particle submissions including
449 animated ones, and 1,666 update entries across 1,500 mixed frames. All 65,536
random inputs are checked in two scale cases, totaling 131,072 coordinate
comparisons against the unchanged original expressions. Cases cover each
rejection and attempt bound, null-matrix early exits, signed zero, random stream
extremes, exact animation-frame boundaries, unsigned timer wrap, zero sentinel,
start calls during active animation, repeated frames, nonorthogonal transforms,
live color-reference mutation, matrix/timer changes, nested updates and state
writes after reentrant callbacks. Finite float bits compare exactly; NaNs compare
by classification.

Twelve Clang/GCC O0/O2 comparisons and three undefined-behavior/float-cast trap
comparisons cover default, non-librw and GL3 profiles. Five optimized MSVC/LTCG
comparisons cover Win32/Win64 D3D9/GL3 plus vanilla Win32. Twenty actual production
compilations cover affected translation units, the C unit and typed backend.
Debug and Release x64 D3D9/OpenAL projects compile/link with existing dependencies
and Git build events disabled. No game is launched.

All fifty project configurations retain unique object paths, including the
twenty-fourth C/C++ pair, BoatFoam.c.obj and BoatFoam.cpp.obj. All 109 previous
whole-source migration audits pass. Immutable originals, commands, logs and
runtime results are in build/boat-foam-c-tests. Quiet checks are exposed by
test_boat_foam.ps1; build/finalize_boat_foam.py verifies recorded final evidence.
