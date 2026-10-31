Escalator.c owns the complete 22-record plain state pool, the shared deletion
guard and all five original registration/update/teardown/audio-query operations.
CEscalator and its member functions are removed. Endpoints, midpoint, radius,
activation/direction flags, step count, interpolation bounds and all 528 typed
opaque CObject pointer slots are canonical C data. Escalators.c calls the record
operations directly. AudioLogic.cpp copies the same activity and midpoint values
through C queries, removing its native-record reference compatibility getter.

The original record constructor only clears activity and 24 pointers. Original
records are exclusively static and their other fields are also statically zero.
The fixture checks this complete constructor, the empty default CVector
constructor and the native CMatrix constructor's two attachment-field writes.
The C state pool's static initialization therefore supplies the same initial
values. Twenty-two actual CMatrix objects remain in a parallel native owner
array at the original source construction point. Their original construction,
assignment, publication, metadata and destruction remain native typed operations.
The private storage is split: 176/272-byte C state plus 72/80-byte native matrix
equals the original 248/352-byte Win32/Win64 record. Every non-matrix field keeps
its original type and position relative to that removed matrix member.

Twenty typed boundaries retain actual model/camera queries, matrix fields,
CObject construction/destruction, object fields, world inheritance conversions
and frame publication. All pool allocation, exhaustion scanning, temporary-object
recovery, handle reuse and slot release use the actual shared C Pool.h directly.
C owns all path lengths, normalization, interpolation, time-step clamping,
activation/deactivation and lifetime decisions. Scalar-left multiplication,
direct component division, the x-only zero normalizer, exact phase comparisons,
live count/bounds/step references and the original shared guard ordering remain.
The original unfixed allocation behavior and FIX_BUGS recovery branch remain.

The fixture audits eight complete changed files and all three complete new
files against immutable originals and the explicit migration. It holds actual
Pool, Matrix, Vector, Object, Placeable, Camera, model, audio-position and common
sources unchanged. Original and C programs compile the actual pool algorithms
and extracted unmodified native matrix constructor/destructor/assignment/
UpdateRW and CVector normalizer. A typed external object/world simulator checks
raw-slot construction/destruction and derived-to-base conversions with a real
nonzero mock base offset. Original and current audio adapter bodies are both
compiled from their actual source files. Native allocator instrumentation uses
aligned CRT storage and assertion failures stop the quiet harness immediately.

The corpus compares every named field and all 16 native matrix coefficients,
attachment metadata, real pool flags/cursors, live object matrices/speeds/tags,
step identities and external operation order. It covers path-length truncation,
the 24-step cap, zero-length paths, retained active fields/pointers, strict 25/28
distance edges, strict count-plus-ten free-space checks, every interpolation
piece, phase wrap, minimum time-step clamping, allocation exhaustion with and
without recoverable temporary objects, handle generations, changed model indices,
live step replacement and pool replacement, nested record updates/teardown and
deactivation during traversal. Sustained updates verify later lifecycle effects.
Failure injection uses the profiles containing the original recovery branch.

Twenty Clang/GCC O0/O2 comparisons cover default, plain, reflection, GL and
unfixed profiles. Five Clang comparisons enable undefined-behavior and float-
conversion traps. Five optimized native MSVC comparisons with actual project
headers cover Win32 default/GL/vanilla and Win64 default/GL. All compare exact
field and float bits. Native probes check every original field and native matrix
partition, pool signatures, model/tag widths and typed inheritance contracts.

Twenty production compilations cover Fluff.cpp, AudioLogic.cpp, Escalators.c and
Escalator.c across five architecture/feature combinations. Actual Debug and
Release x64 D3D9/OpenAL projects compile and link. All 50 project configurations
retain distinct objects for 28 paired C/C++ units. All 122 distinct preceding
source audits pass. Escalator-controller runtime comparisons and sanitizer
comparisons retain their previous results through direct C record calls. No
game or GUI was launched.

Run utils/tests/test_escalator_record.ps1. Optional switches select Sanitized,
Native, Consumers, BuildProject and AuditPrevious. Immutable originals, generated
oracles, logs and result manifests live under build/escalator-record-c-tests.
