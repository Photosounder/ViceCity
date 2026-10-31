The remaining C3dMarker AddMarker, DeleteMarkerObject and Render member functions
now execute as Markers3D_RecordAdd, Markers3D_RecordDelete and
Markers3D_RecordRender in Markers3D.c. Their member declarations are removed and
the record is a struct containing the original fields. All ten marker/controller
methods now execute in C11. The existing three controller-to-member forwarding
operations are removed, and the C controller calls its C record functions directly.

The record still embeds the actual CMatrix and therefore still has C++ matrix
construction/destruction. This phase removes the marker methods, not the shared
matrix dependency. Native Win32/Win64 probes compare every field's type and
offset, alignment, and complete record size with the immutable original class.
The original 120-byte Win32 and 136-byte Win64 layouts remain unchanged.

Creation retains identifier publication before matrix/resource setup, last-atomic
selection through the exact typed RenderWare callback, resource call ordering,
material flag OR, attachment, live timestamp read, field-write order and final
live atomic success test. Deletion clears the original five fields first,
captures the frame, rereads the live atomic after the frame query, destroys the
two resources in order and publishes null last. Original failure behavior is
preserved without adding new guards.

Rendering retains its null-atomic early return and borrows the actual live
material-color reference. A typed boundary creates an independent local CMatrix
for each render entry and keeps it alive until the entire C render sequence
returns. C attaches that matrix through its typed owner, scales all nine basis
coefficients through actual float pointers, publishes it, and executes frame,
lighting and draw operations in original order. Marker size, brightness, atomic
and type remain live across callbacks; the two depth-write gates are evaluated
independently. Reentrant draws get independent real stack matrices.

The combined fixture compiles all seven original controller methods and all
three original record methods as its oracle. The new variant links the actual C
controller/record implementation, typed backend and separate C/legacy callers.
Both use original CMatrix/CVector implementations; native tests use actual
project and RenderWare headers. All six entire changed files are audited against
the explicit transformation, and the preceding marker-controller audit retains
all eleven complete file comparisons.

Each run compares 1,799 full state snapshots, 3,967 placements, 1,366 resource
clones/frame creations, 1,155 deletions, 6,183 draws and 2,111 coronas, with
90,674 owner calls. Direct record tests cover every original marker type,
null-atomic rendering, creation's live success return, changed color references,
size/brightness mutation before scaling, nested render lifetimes, a changed
atomic between frame query and deletion, and changed type between depth gates.
The complete prior placement/reflection corpus is also retained.

Twelve Clang/GCC O0/O2 comparisons, three trap sanitizer comparisons and five
optimized MSVC/LTCG Win32/Win64 D3D9/GL3/vanilla comparisons match exactly, with
fingerprint ad52853fae64ca87. Forty-five actual production compilations and both
Debug/Release x64 D3D9/OpenAL builds pass. No game is launched. The project still
has twenty-five distinct C/C++ object pairs across all fifty configurations;
this phase changes no project entries. All 111 preceding source audits pass.
Evidence is under build/marker-record-c-tests, quiet checks are exposed through
test_marker_record.ps1, and build/finalize_marker_record.py checks final evidence.
