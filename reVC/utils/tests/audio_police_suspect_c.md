# Vehicle suspect reports and fixed model IDs in C

AudioPoliceSuspect.c owns the original 95-row color table and complete former
SetupSuspectLastSeenReport implementation. The police scheduler directly calls
the C report. Its old owner-delegating report adapter is removed. Three game host
functions retain the exact owner vehicle lookup and primary-color/model reads.
There is no new initialization gate.

ModelIds.h contains both original fixed model/count enum blocks verbatim.
ModelIndices.h imports it while retaining dynamic model globals, game declarations
and C++ helpers. No numeric model value or alias changes. The C report includes
the small shared header directly.

Music gate, vehicle lookup before queue capacity, primary-color validation before
model read, unknown-color diagnostic, complete model switch, signed random
remainder, color pre/main/post modifiers and report queue word order are preserved.
Unknown models retain the original early return. Foot reports keep their original
separate capacity gate. The fixed color table is private const storage in C.

Run utils/tests/test_audio_police_suspect.ps1. It independently audits the complete
table/body, three adapters, all caller/header edits, entire ModelIndices edit and
verbatim enum blocks. Original member code uses the saved enum declarations; the
actual C module uses ModelIds.h. Every one of the 304 named model/count constants
is compared against a real C caller. Controlled callbacks record owner lookup,
model access and exact diagnostics, plus full queue/crime/report state.

Sixteen Clang/GCC O0/O2 comparisons cover default, vanilla, no-external and PS2
records. Native MSVC x86/x64 comparisons pass. The trace covers all 256 color
bytes and all 300 default model values, both owners, queue capacity/ring-wrap
boundaries, vehicle/foot/music gates, signed random values and unknown signed
model IDs. Seventeen prior phase source audits pass. The previous scheduler/zone
report comparisons are rerun against a controlled C suspect-report boundary;
this fixture separately verifies the actual new C implementation.

The 390 unique consumer compilation checks include ModelIndices.cpp and ModelInfo.cpp because the
shared enum declarations move. Native game caller/registry compilation is checked
in both architectures. No full linked game build, actual vehicle lookup or audio
playback is verified; game/backend/logger functions are controlled callbacks.
