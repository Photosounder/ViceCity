PlayerSkinData.c owns all six former CPlayerSkin operations, both standalone
player-DFF loader helpers, the canonical clump pointer, saved FOV, dictionary
slot and both persistent preview statics. CPlayerSkin is gone. The C++ file now
contains 41 typed native boundaries and the public header compiles as C11.
Game, front-end and player-info call sites enter the C operations directly.

FindPlayerDff uses explicit uint32_t output pointers in place of C++ references.
The original directory scan, case-insensitive comparison, first matching entry,
assignment order and unchanged outputs on a failed read remain intact. The
original lack of a directory-close call and the loader's existing assumptions
after an unsuccessful search are preserved. Tests inject failed reads into the
standalone helper with initialized outputs; they do not execute the original
undefined loader path with uninitialized offset/size values.

The loader retains image ownership, sector allocation/read/synchronization,
chunk selection, clump publication and release order. A native scope creates a
real RwMemory object, sets its two original fields and keeps that exact object
alive throughout the complete C open/find/read/close sequence. C passes it as an
opaque void pointer and never casts a C record into native RwMemory storage.
The callback type is identical in C and C++, including sanitizer function checks.

Texture lookup retains dictionary push/select/read/pop order, the early cached
texture return, both default-name cases, BMP path formatting, format outputs,
raster/texture publication and image cleanup. Preview begin/end retain clump
loading, actual static render callback identities, first-player skin loading,
saved FOV and final live restoration. Rendering retains its captured frame,
unsigned strict timer gate, persistent two-degree rotation, wrap after 360,
original camera transform, position/axis constants, ambient coefficients and
final live clump read. Vector/color transfers use explicit scalar copies into
actual native RW values. C++ public declarations retain actual native texture
and clump pointer types; C only sees opaque handles.

The fixture checks seven complete changed source/project files, both new
production files and all unchanged native dependencies against immutable
originals. Older file-manager, directory, timer and drawing audits also verify
the entire relocated C kernel and the complete current native boundary when
their old snapshots contain PlayerSkin.cpp. Replacing that file with native
boundaries does not discard their original whole-algorithm evidence.

Both comparison programs compile the actual original method/helper bodies or
the complete C kernel. Test-only public access exposes the original slot without
changing stored data or method logic. Both link actual unchanged Draw.c and use
the actual Timer.h pause-time accessor and DirectoryInfo declaration. A separate
C11 caller enters all eight operations. Native comparisons use real RW types,
including a real RwMemory lifetime and real frame matrices; typed owner models
observe all external arguments and canonical writes. Four unrelated native
interpreter stubs abort if unexpectedly called.

The corpus covers directory matching order and case, aliased helper outputs,
failed reads, both image-ownership paths, chunk success/failure, a null clump read,
all valid test sector sizes, existing/missing textures, image-read failure,
default/empty/custom names, nested texture loading from the player callback,
strict timer boundaries, unsigned time wrap and sustained rotation through 360.
Hooks change the slot during dictionary creation, a borrowed name during lookup,
the actual native memory fields during open, the clump/FOV during renderer setup,
saved FOV during destruction, the live clump/camera after frame capture, the slot
after texture filtering and the texture owner during image cleanup. Each run
compares 455 states, 4,054 external calls, 397 preview renders and 17 balanced
allocation/free and memory-stream open/close sequences.

Twenty Clang/GCC O0/O2 comparisons cover default, plain, MASTER, unfixed and GL
profiles. Five Clang runs enable undefined-behavior, float-conversion and indirect
function-type traps. Seven optimized native MSVC comparisons cover Win32
default/GL/MASTER/vanilla and Win64 default/GL/MASTER. Native probes verify all
eight public contracts, three canonical field types, packet widths, all 41 native
contracts, actual memory layout/fields, first-player receiver and callback types,
and standalone C11 public/backend headers.

Twenty-five production compilations cover the four actual native consumers and
the C unit across five architecture/feature combinations. Actual Debug and
Release x64 D3D9/OpenAL projects compile and link. All fifty configurations retain
unique objects for all twenty-eight paired C/C++ units. All 130 preceding source
audits pass. No game or GUI test is launched.

Run utils/tests/test_player_skin.ps1. Optional switches select Sanitized, Native,
Consumers, BuildProject and AuditPrevious. Immutable originals, generated inputs,
logs and manifests are in build/player-skin-c-tests.
