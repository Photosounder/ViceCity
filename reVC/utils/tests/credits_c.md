Credits.c removes the CCredits class and owns all seven complete methods and
both private state fields. The complete original sequence remains unrolled in
its original order, including all 401 mutable Unicode text lookups, language
spacing branches and the final font flush. No C++ credits state remains.
All current callers, including the optional PS2 menu and disabled debug-menu
entries, use the C API. Reference helpers use explicit uint32_t pointers.

Twenty-three typed backend operations retain the actual input, text, font,
camera and menu owners. Native Win32/Win64 probes verify the original bool and
timestamp types, uint32 reference width, wchar/uint16_t identity, mutable text
owner signature, pad predicate, exact font print overload and language values.
The font color is constructed through the original CRGBA owner at each call.
Unicode buffers are borrowed directly; no text snapshot or assumed owner layout
is introduced. Screen scaling uses the exact original common-header macros in
C, with current width/height reads and the existing Draw scaling operations.

Start publishes the active flag before reading the live timer. Stop and Init
leave the original timestamp intact. Render checks the active flag and menu
before any render-state setup, then computes elapsed time with unsigned timer
subtraction. The scroll value remains local to that entry even when callbacks
restart credits or change the timer later. Each text helper updates input before
reading its borrowed line offset. Cancellation skips that helper's print/line
advance but does not abort the remaining render sequence or its text lookups.
Space and text helpers preserve float compound assignment followed by unsigned
conversion, including precision behavior above 2^24. Shadow and foreground
prints borrow the same live text pointer and reread screen settings independently.
Language predicates and both final border preferences remain live at their
original points. Fixed and unfixed completion expressions remain distinct.

credits_fixture.py audits nine complete changed source/project files, both new
files, all seven complete methods and the entire ordered localization-key list.
The oracle compiles every original method statement unchanged. An access-only
header adjustment exposes the original private static fields to test observers;
no algorithm or original data type is changed. The C variant compiles the actual
production C file plus a separate C11 caller and typed backend. Native runs use
actual project/RenderWare common definitions and independently checked owner
signatures; mocked owner calls provide quiet, deterministic observations.

Each comparison observes 852 state snapshots, 295,136 text lookups, 295,171 input
updates, 736 font flushes and the complete owner-call sequence. Default cases
submit 9,212 strings and 721 borders. The corpus covers strict print boundaries,
all original language branches, menu and border short circuits, scaling flags,
float-to-uint32 precision boundaries, early/repeated/late cancellation, timer
wrap, full scrolling, changing Unicode contents and line aliases, live viewport
changes, restarts/stops during callbacks and nested rendering. Finite float bits
compare exactly. Default fingerprint: c2d5e4ed06646da8.

Twenty-four Clang/GCC O0/O2 and six trap sanitizer comparisons cover librw,
non-librw, forced PC scaling, unfixed completion, no border preference and no
aspect correction. Five MSVC/LTCG comparisons cover Win32/Win64 D3D9/GL3 and
vanilla Win32. All match the original. Thirty-five actual production compilations
and both Debug/Release x64 D3D9/OpenAL builds pass; no game is launched. All fifty
project configurations preserve distinct objects for twenty-six C/C++ pairs,
including Credits.c.obj and Credits.cpp.obj. All 112 preceding source audits pass.
Immutable originals and final evidence are under build/credits-c-tests.
test_credits.ps1 runs quiet checks; build/finalize_credits.py verifies the evidence.
