ShinyTexts.c removes CShinyText and CShinyTexts, replacing their five complete
algorithms and both thirty-two-record arrays with C11. Registration, distance
and fade calculations, per-vertex geometry construction, index writes, texture
grouping, shared-counter updates, reflection caching and replay decisions all
execute in C. Existing controller and custom reflection calls use the C API;
the controller's two former C++ shiny-text forwarding functions are removed.

Actual MSVC Win32/Win64 probes verify the original 88-byte record aligned to
four: four three-float positions at zero, four two-float UV pairs at forty-eight,
camera distance at eighty, and type/red/green/blue at eighty-four through
eighty-seven. C11 layout assertions verify the same structure in every runtime
profile. ShinyTexts.h owns the unchanged walk/flat enum values one and two.

Initialization still clears only the main and reflection counts. Registration
rejects a full pool before querying the camera, copies the original main-camera
position once, retains all original optional reflection camera/frame/LTM reads,
writes distance even on a later distance rejection, and narrows faded colors at
their original assignments. Position and UV inputs remain by value. The original
single-precision subtraction, sum order and sqrtf operation are preserved.

Rendering saves the main list before its empty-list return, retains any pending
shared geometry on that return, and preserves both strict greater-than capacity
thresholds. Walk texture identity is reread at every original use, including
after a flush, raster lookup and state change. Colors remain cached before a
texture flush; positions and UVs remain live at their individual setters. Unknown
types generate no geometry. The original flat-to-walk/flat texture transitions,
six index writes, vertex/index increments and render-state restoration remain.

Reflection replay uses the saved main list only when the current list is empty,
then renders through the same C routine and clears both counts at the original
points. The flush tests the index count, invokes LittleTest before reading the
transform count, reads the index count again after transformation, and clears
both counters after success or failure. A zero index count leaves the vertex
count untouched. Three already-existing typed RenderBuffer backend operations
perform transform/indexed-render/end; eighteen shiny-text backend operations
expose camera, texture, state, hook and vertex owners without assumed layouts.
The actual shared vertex storage and its separate 2D/3D arrays remain unchanged.

shiny_texts_fixture.py audits all seven entire changed source/project files,
all five complete algorithms, both class removals, exact headers and typed
backend. Unchanged original methods, actual vector/UV headers, original typed
vertex methods and the actual RenderBuffer.cpp backend participate in the
comparison. Only private access is exposed in the oracle declarations; fields
and method bodies are unchanged. The new logic and a separate caller compile
as strict C11. Native comparisons use actual project/RenderWare headers.

Each run observes 2,015 complete snapshots of both lists, every declared record
field, all shared vertices and indices, pending reservation counters, and every
untouched byte of the separate 2D array. The corpus includes 1,500 mixed frames,
26,688 registration attempts, 21,240 transform attempts, 16,446 successful draws
and 371,632 vertex-field writes. It covers every capacity and list length,
distance/fade boundaries, signed zero and legal nonfinite inputs, reflection
null branches and fallback, texture groups and null initial walk texture,
unknown types, rejected transforms, exact buffer thresholds, live texture and
field changes, argument aliases, nested registration, recursive flushing and
reflection rendering. Finite floats compare bit for bit and NaNs by
classification; compiler/optimization-specific original fingerprints are retained.

Sixteen Clang/GCC O0/O2 comparisons and four undefined-behavior/float-cast trap
comparisons cover default, non-librw, GL3 vertex packing and disabled extended
pipelines. Five optimized MSVC/LTCG comparisons cover Win32/Win64 D3D9/GL3 plus
vanilla Win32. Twenty-five actual production compilations cover all affected
callers, both C units and the backend. Debug and Release x64 D3D9/OpenAL projects
compile and link using existing dependencies with Git build events disabled.
No game is launched. The changed controller's sanitizer comparisons also pass.

All fifty project configurations keep unique object paths, including the
twenty-second C/C++ pair, ShinyTexts.c.obj and ShinyTexts.cpp.obj. All 107 earlier
whole-source migration audits pass. Immutable originals, commands, logs and
runtime records are in build/shiny-texts-c-tests. test_shiny_texts.ps1 exposes
the quiet checks and build/finalize_shiny_texts.py verifies the recorded evidence.
