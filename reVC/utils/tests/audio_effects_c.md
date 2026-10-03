# Sound-effects orchestration conversion

AudioEffects.c owns sound-effects service and entity interrogation as C11. The two former owner methods and the outer service trampoline were removed. AudioService_Run calls AudioEffects_Service directly. Six small C++ callbacks retain the remaining game subsystem methods at their original service points. Visual Studio registers the new translation unit as C11; CMake and premake already discover C sources.

The conversion retains the logical-frame priority gate, frame-counter wrap, pause transition stops, PS2 generic/surround channel selection, both requested-queue clears and partial active-record reset. Queue switching, reverb, special handling, entity interrogation, comments, police, collisions, releasing sounds, missions, volume adjustment, active processing and optional backend service keep their original order.

Entity interrogation reads the live order slot again after processing and the live count at each iteration. Script cleanup resets each object, deletes it directly through the existing C pool API, then rereads the script index to clear the entity pointer and destroy the entity. The final script count reset remains last. Neither operation introduces an initialization gate.

Verification:

- Independent full-body audits of both converted operations, complete owner/header edits, six callback definitions and the outer C service caller
- All 26 earlier audio source audits pass, including the pedestrian integration audit updated to follow service into its C translation unit
- 24 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no OpenAL, no external audio, no reverb and PS2 shapes
- Four native MSVC x86/x64 before/after comparisons across default and no external audio, plus compilation of the actual game callers and callback implementations
- 1,024 operations per comparison cover both owners, direct C callers, repeated service, all pause combinations, both queues, surround state, zero/nonzero logical frames, frame wrap, initialized/uninitialized owners and zero/one/two/three script objects
- Callback-visible snapshots observe every zero-padded owner byte with normalized script pointer identities, actual pooled script bytes, generation flags and the allocation cursor
- Controlled entity callbacks change the current order slot and entity count; special handling changes the active queue; pool lookup changes a live script index after reset and before deletion
- Actual C entity destruction, requested/active queue reset, volume adjustment and scalar math are linked with the original script reset body compiled as C and the existing inline C pool deletion
- 470 unique consumer compilation checks across architectures, D3D9/software rendering, threading and decoder configurations
- All 20 prior top-level service runtime comparisons still pass with their original digests

Full game linking and playback were not tested. Remaining game subsystem behavior is controlled in this fixture and retains its own earlier tests where already converted.
