RenderBuffer's class is removed. RenderBuffer.c owns all four original algorithms,
the two live geometry counts, two pending reservation counts and 1,024 indices.
The public header provides C11 functions and scalar storage. Five C++ operations
address or publish actual typed vertex pointers and enter RenderWare transform,
indexed-render and end calls. No allocation, reservation, commit or flush logic
remains in C++.

The two capacity checks retain their separate original order and >= boundaries.
Each flush tests the live vertex count, conditionally transforms/renders/ends,
then clears both live counts regardless of transform success. Clearing leaves
pending counts and all payloads intact. Reservation publishes the index pointer,
then the vertex pointer, then the two pending counts. Commit reads live counters
and pending counts, adds the current vertex base to each original uint16 index
with the same narrowing, then advances index and vertex counts in order.

C callers receive an opaque vertex handle through actual void-pointer storage.
Legacy callers supply real RwIm3DVertex-pointer storage as an opaque address;
the typed C++ writer publishes that pointer at the original write point. Both
paths use the same C reservation algorithm. Neither path casts a typed
pointer-to-pointer into void-pointer-to-pointer storage.

VertexBufferUnion remains the original struct with two separate arrays, despite
its name: 512 actual RwIm2DVertex records followed by 512 RwIm3DVertex records.
The original C++ typed storage and its shared direct readers remain available.
Win32/Win64 probes in D3D9 and GL3 validate scalar/index widths, array capacities,
array separation and total native storage. C does not infer vertex layout or
stride. RenderBuffer.h exposes the typed storage only to remaining C++ readers.

render_buffer_fixture.py audits all complete algorithms and changed source/project
files against immutable originals. The oracle compiles the complete original
RenderBuffer.cpp unchanged except for include selection. Separate C11 callers
exercise both output paths. Native comparisons use actual project headers and
RenderWare vertex types. The corpus compares 3,236 complete state snapshots,
all four counters, every index, every 3D vertex field and every byte in the
independent 2D array. It covers both capacity thresholds, empty/full reservations,
one-past zero-size outputs, negative reservations with valid counters, index
wrapping, failed transforms, owner changes to live counters, nested transform/
render/end hooks and 2,500 mixed reservation/commit iterations.

Eight Clang/GCC O0/O2 comparisons and two undefined-behavior trap comparisons
cover both vertex backends. Five MSVC/LTCG comparisons cover native Win32/Win64
D3D9/GL3 and vanilla Win32. All original/C fingerprints agree. Production checks
compile the backend, all fifteen direct header consumers and the C unit across
default/software GL3 and vanilla Win32. Actual Debug/Release x64 D3D9/OpenAL
projects compile and link with existing dependencies and Git-hash events disabled.
No game is launched. All fifty project configurations keep unique object paths,
including all seventeen C/C++ pairs. Commands, immutable inputs and result records
are under build/render-buffer-c-tests.

The broader header-consumer check found a pre-existing vanilla configuration
failure in WaterLevel.cpp: water texture animation called Timer_GetTimeStepFix
unconditionally although both the original C++ and migrated C helpers are
declared only under FIX_BUGS. The call site now uses the helper's exact expression,
(*Timer_GetTimeStep()) / (50.0f / 30.0f). Its normal-build calculation is unchanged
and the vanilla caller compiles. This additional whole-file transformation is
included in the immutable source audit, bringing this phase to twelve files.
All 102 preceding whole-source migration audits pass. The final evidence is
checked by build/finalize_render_buffer.py; test_render_buffer.ps1 exposes the
portable, native, sanitizer, consumer, project-build and source-audit checks.
