# Owner entity creation and public entity facade in C

AudioManagerEntities.c is an actual C11 translation unit implementing the former
AudioManager CreateEntity method and five public DMAudio entity operations:
CreateEntity, DestroyEntity, GetEntityStatus, SetEntityStatus, and PlayOneShot.
It reads or updates the existing C-linkage AudioManager record. Six corresponding
C++ owner members and declarations are removed. Game-internal accessors/removal/
status/event sites call the previously converted AudioEntities C kernels directly;
creation sites pass their live owner pointer to the C creation function.

Creation preserves the initialization/null/type checks, first-free-slot policy,
and FIX_BUGS scan of both requested queues. Free IDs still referenced by ordered
queued sounds remain excluded, with the original duplicate-ID collection and
physical-slot iteration order. The C code changes the queued sound reference to
a pointer, qualifies owner fields, and uses equivalent fixed-width scalars and
byte booleans. It retains the selected-record initializer and order/count update.
No extra validation, allocation, or ownership policy is introduced.

The public creation facade removes the void-to-CPhysical-to-void pointer round trip;
this preserves the same opaque object pointer. Other public bodies bind the same
owner arrays/count/initialization fields to the unchanged C entity kernels. All
migrated game arguments are simple values; no callback evaluation is moved across
an initialization gate. Status bytes and event ordering remain unchanged.

Run utils/tests/test_audio_owner_entities.ps1. The generator independently checks
the real C creation body, all five public facade bodies, complete edits to six
caller files, and the owner header against saved sources. The fixture executes
original member/facade bodies and the actual C source using the original stored
field declarations. Both implementations link the unchanged AudioEntities C
kernels. A C caller reaches every public entity export and the live global record.

The trace covers every initial entity occupancy through full capacity, both
requested queues, negative queued handles, duplicate/permuted order entries,
queued-ID exclusion, null/uninitialized/type sentinel gates, and 12,000 mixed
creation/removal/status/event operations. Snapshots compare every entity field,
retained volume/event entries, order/count state, and queued-ID scan inputs.
Controlled opaque pointer tokens avoid process-dependent addresses; record padding
is excluded. Legacy out-of-range queued handles and invalid enum representations
are outside this defined-input comparison.

All sixteen Clang/GCC O0/O2 comparisons pass in default, vanilla,
external-3D-disabled, and PS2-shaped configurations. Native MSVC x86/x64 comparisons
pass with digest 13360601767154732893. All 300 consumer compile checks and native
integration compilation pass. Native symbols show the six C providers and matching
game references, with no old entity-member references left in source.

The earlier sixteen entity-kernel runtime comparisons also pass. That fixture now
uses saved thin C++ adapters solely to drive the unchanged standalone C kernels;
it separately checks the actual C creation body and absence of owner members.
The new fixture verifies current owner/facade integration. All other relevant
source audits pass after their intended caller/member substitutions.

The existing Visual Studio project/filter registers the C11 source; CMake/premake
already collect C files. Saved sources, scripts, native logs, and verification.json
are in build/audio-owner-entities-c-tests. The owner retains its other C++ methods
and lifetime callbacks. No full linked game build or in-game playback was performed.
