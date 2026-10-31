SpecialFX.c replaces the complete CSpecialFX controller class: all six methods,
its associated LookForBat callback, three boolean flags and signed snapshot-frame
counter. All project callers use the C API. TimerHost_SnapshotActive now reads
the same live flag from its C owner; the timer algorithm and public host symbol
stay unchanged. FontStyles.h shares the exact original font enum, including its
language guard and MAX_FONTS value, between the C controller and C++ font owner.

All effect initialization, UV setup, texture acquisition, state resets, update,
render and shutdown operations retain their original order. The optional first-
person renderer gate retains its short circuit. The debug group label is unchanged.
Video and lift overlays retain the original scaling expressions, frame-counter
mask, shared ASCII and Unicode buffers, scanline loop, scrolling band, 400 random
draws per lift frame, 200 rectangles and blend-state writes. Snapshot activation,
flash rounding, frozen time and release after frame twenty are unchanged.
Screen dimensions and flags remain live at their original use points.

The three weapon presets and callback perform their matrix-vector arithmetic in
C. Forty-one typed backend operations expose existing game owners without
assuming their object layouts. Two independent lazy static CMatrix instances
retain the original construction, assignment, attachment and destruction rules.
The coefficient reader copies twelve named fields into a plain 48-byte C record;
the original right/up/at interpretation and scalar product/addition order are
preserved. Conversion of the shared CMatrix owner can happen as a later step.
Other marker, light, text and money-message algorithms remain in their current
owners and are called through typed boundaries.

special_fx_fixture.py audits all fourteen complete changed source/project files,
all controller bodies and the exact shared font enum against immutable originals.
The comparison compiles the unchanged original controller, actual CVector and
CMatrix headers, complete original matrix construction/attachment/assignment
bodies, the original vector transform and rectangle constructor, and the actual
Draw_ScaleY function. Native comparisons use actual project and RenderWare
headers. The new algorithm is compiled separately as strict C11.

Each comparison observes 1,258 state snapshots, 9,521 polygons, 20,600 rectangles,
41,200 random draws and 1,422 streaks. The corpus includes all eight flag
combinations, snapshot frames zero through twenty-two, an uninterrupted snapshot
sequence, odd and standard resolutions, optional scaling and Japanese styles,
mid-render screen/flag/time changes, child ordering, renderer short circuits,
null player and weapon branches, repeated final weapon-ID reads, every weapon
preset and 1,000 interleaved uses of the independent lazy matrices. Every float
result is compared bit for bit; process addresses and padding are excluded.

Twenty-four Clang/GCC O0/O2 comparisons cover default, non-librw, disabled new
renderer, PC scaling, disabled language extensions and FINAL configurations.
Six additional comparisons trap undefined behavior and float-cast overflow.
Five optimized MSVC/LTCG comparisons cover Win32/Win64 D3D9/GL3 and vanilla Win32.
Sixty production compilations cover all affected C++ callers, the font owner,
the backend and C controller across the native profiles. Debug and Release x64
D3D9/OpenAL projects compile and link with existing dependencies reused and Git
build events disabled. No game is launched.

All fifty project configurations retain unique object paths, including the
twentieth C/C++ pair, SpecialFX.c.obj and SpecialFX.cpp.obj. All 105 preceding
whole-source conversion audits pass. Commands, immutable originals, logs and
comparison records are in build/special-fx-c-tests. test_special_fx.ps1 exposes
the quiet checks; build/finalize_special_fx.py verifies their recorded evidence.
