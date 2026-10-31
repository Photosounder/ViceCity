Markers3D.c owns all seven former C3dMarkers controller methods, the active
count and the reflection snapshot. Placement, slot selection, pulse and alpha
math, borrowed-position changes, distance/depth clipping and draw scheduling
execute in C11. The C3dMarkers class is removed. Existing callers use the C
controller; C++ placement wrappers retain the original mutable CVector reference
and typed C3dMarker return value.

The C3dMarker record and its three methods remain typed C++ owners for this
increment. Each record embeds a constructed CMatrix, so replacing it with an
assumed C layout would lose its lifetime and attachment behavior. Native probes
verify the actual 120-byte Win32 and 136-byte Win64 layouts, embedded matrix
offset and all sixteen scalar pointer types. The boundary borrows those actual
field addresses, reads/writes atomic and clump slots through their real types,
and retains the original record/matrix operations. No C++ record is type-punned
as a C struct, and no typed resource slot is cast to void **.

The controller preserves initialization's partial writes, find-before-push
dictionary ordering, retained clump slots after shutdown, first qualifying
arrow replacement, player lookup before type validation, live field reads after
render callbacks and active-count updates. Position readers access only the
requested components. Ground lookup rereads coordinates after collision
callbacks; a zero ground clears the deferred flag without writing height.
Rotation saves and restores the original matrix position before reading the
borrowed input again. PlaceMarkerSet shares one mutable input across all three
calls. Corona submission retains the actual matrix-position reference, original
enum/default arguments and pointer-to-uint32 identity conversion. Reflection
retains both matrix reads, strict depth/range boundaries and snapshot consumption.

The fixture audits eleven complete changed source/project files and all four
new files against the explicit migration. Its oracle compiles all seven original
controller methods unchanged. Both variants link the original three record
methods and actual CMatrix/CVector implementations. Separate C11 and legacy
callers exercise both public interfaces. Callback mutations test live input,
timers, marker fields, active count, dictionary state and nested reflections.
Observers compare all record fields, all sixteen embedded matrix values,
attachments, resource identities, cache indices and owner-call order.

Each comparison covers 1,623 complete state snapshots, 3,967 placements,
1,254 atomic clones/frame creations, 1,049 deletions, 6,077 draws and 2,111
coronas. Twelve Clang/GCC O0/O2 comparisons and three trap sanitizer comparisons
cover librw, non-librw and GL3 profiles. Five MSVC/LTCG comparisons use actual
project/RenderWare headers for Win32/Win64 D3D9/GL3 and vanilla Win32. All results
match the original, with fingerprint cf225f34ef45adbf. The SpecialFX controller's
six trap comparisons also pass after its three forwarding wrappers are removed.

Forty-five native production compilations cover affected callers and both C
controllers. Actual Debug and Release x64 D3D9/OpenAL builds compile and link.
No game is launched. All fifty configurations retain distinct object paths for
twenty-five C/C++ pairs, including Markers3D.c.obj and Markers3D.cpp.obj. All 110
previous whole-source audits pass. Immutable originals and evidence are under
build/markers-3d-c-tests; test_markers_3d.ps1 runs quiet checks and
build/finalize_markers_3d.py verifies the recorded results.
