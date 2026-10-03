# Collision history service conversion

AudioCollisionService.c owns the former ServiceCollisions member as C11. AudioEffects_Service calls it directly, removing the last subsystem member trampoline from AudioManager.cpp. One-shot and looping collision generators remain in C++ behind two small adapters taking the original collision record by pointer and dereferencing it into the original reference parameter. Visual Studio registers the C unit; CMake and premake already discover C sources.

Service retains the unconditional scratch collision-entity write, repeated-collision matching by ordered entity pointers and surface IDs, previous base-volume increment before copying into the requested record and looping callback, selective history clearing while preserving base volumes, first-free history-slot assignment for new collisions, one-shot-before-looping order and final queue sentinel/count reset.

New history slots retain the original behavior: service assigns entity/surface identity and base volume, but does not mark the slot occupied in the local repeated-collision flags. Multiple new records can therefore reuse that first free slot. No additional pause/initialization/entity gates were introduced. Live request counts and index reads, captured local request indices across callbacks, and aliased collision records are preserved. Owner and collision-record layouts are unchanged.

Verification:

- Independent complete-body audit, both generator adapters and all owner/collision/header/pipeline edits
- All 31 earlier audio source audits plus the existing collision queue/math audit pass
- 20 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no reverb, no external audio and PS2 shapes
- Four native MSVC x86/x64 comparisons across default and no reverb, plus compilation of actual game callers and collision generator adapters
- 1,536 operations per comparison cover both owners, direct C callers, repeated empty service, empty/partial/full queues, queue replacement after more than ten requests, sorted indices, repeated/new collisions, reversed entity pairs, changed surfaces, duplicate historical identities, null entity identities, retained base volumes, different intensities/positions/distances, and initialized/uninitialized, paused/unpaused and negative collision-entity state
- Actual C collision initialization and request insertion are linked in both fixtures
- Every zero-padded owner byte is observed at callbacks and after service with normalized collision entity pointer identities; the exact aliased requested-record index and history-slot counter are also observed
- One-shot callbacks mutate the referenced record before looping, change a live index-table entry and change the collision-entity field; the first looping callback can reduce the live request count
- Both generator boundaries are required to be reached. Signed base volumes stay below overflow even after repeated-match increments
- One additional Clang O1 before/after comparison with undefined-behavior and floating-conversion checks in trap mode passes for service and the actual C queue primitive
- 520 unique consumer compilation checks across architecture, renderer, threading and decoder configurations
- All 24 prior sound-effects orchestration comparisons retain their original digests

Full game linking and playback were not tested. Individual collision sound generator behavior is controlled in this fixture and remains unchanged in production.
