# Audio entity state C conversion

`AudioEntities.h` defines `tAudioEntity` as a plain C struct with the original
field order, enum type, pointer, byte flags, four events and volumes, and count.
Its layout remains 40 bytes on x86 and 48 on x64. The header uses the existing
C-compatible configuration, audio enums, and script-manager state.

`AudioEntities.c` compiles as C11 and implements six operations: selected-record
initialization, removal from the active entity order list, status get/set,
pointer lookup, and one-shot event insertion. Record initialization preserves
retained volume storage; destruction clears usage and updates the order list
without clearing retained record fields. Status values remain bytes, including
values greater than one. The one-shot priority table is copied unchanged.
Script-object events preserve their separate capacity-limited index list.

AudioManager's existing member functions delegate these operations to the C
module. Creation retains its C++ requested-sound scan, duplicate filtering,
queued-ID exclusion, free-slot selection, and error returns, then calls the C
record initializer. No owning state fields or overall AudioManager layout move.
AudioManager and its game integration remain C++.

Run `utils/tests/test_audio_entity.ps1` from the repository root. Saved sources
are in `build/audio-entity-c-tests/before`. The generator verifies original and
current initialization, removal, accessor, and event statements after intended
C substitutions, checks the exact priority table, and audits the converted AudioManager
entity methods independently of other subsystem migrations. Fixtures execute the actual old and current creation adapters and
link both C and C++ callers against the real C module.

Coverage includes sizes/alignment/offsets, creation errors, missing pointers,
full entity storage, repeated/negative queued references, queued-ID exclusions,
recycling and retained volume fields, arbitrary status bytes, invalid entity
indices, out-of-range sounds, 20,000 mixed lifecycle/event operations, every pair
of sound priorities in both empty and full event queues, signed-zero volume
copies, and the final script slot/full-list rejection boundary. Comparisons
exclude padding and normalize object pointers to stable array indices.

Clang/GCC O0/O2 regressions pass in current, FIX_BUGS-disabled, external-3D-sound-
disabled, and PS2-style configurations. Native MSVC x86/x64 runs match their
same-architecture original implementation. All 210 consumer compile checks
passed; native MSVC compiled AudioManager.cpp, AudioLogic.cpp, and the real C
module. Native object symbols confirm matching C exports and game references.
The Visual Studio project and filters register the new source/header with C11;
CMake and premake already collect C files.

The pedestrian-comment regression audit now checks its owning constructor and
service function independently of unrelated entity-method changes, and its
runtime regression suite is rerun after this conversion. Verification details
are recorded in `build/audio-entity-c-tests/verification.json`. No full linked
game build or in-game audio playback test has been performed.
