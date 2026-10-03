# Entity and physical-object dispatch conversion

AudioDispatch.c owns the former ProcessEntity and ProcessPhysical switches as C11. AudioEffects_Interrogate calls the C entity dispatcher directly. The physical dispatcher also resides in C, with game adapters for the physical type query and the existing vehicle/pedestrian generators. Other game sound generators remain C++ behind small adapters. Current-area reads use a separate adapter at each original field access.

EntityTypes.h and GameAreas.h contain the original named enums unchanged: six entity-kind constants and nineteen area constants. Entity.h and Game.h include these shared C-compatible headers at the former declaration points. Visual Studio registers the C unit and both headers; CMake and premake already discover the source.

Routing retains the entity-status gate without adding a used/initialized check, scratch entity-index write before dispatch, type-specific pause behavior, reverb writes before generators, weather area short-circuiting and the conditional bridge branch. Front-end sounds still run while paused; garages retain the incoming reverb value; unsupported kinds return after the scratch entity write.

Physical routing retains the initial pointer capture and null gate, the type query on that captured object, then a fresh stored-pointer read before the vehicle or pedestrian generator. The original stored-pointer casts remain at the C++ game boundary. Owner and sound-record fields are unchanged.

Verification:

- Independent complete-body audits, the original conditional reverb macro, all fourteen generator adapters and both game-data adapters, complete owner/logic/header/service edits and both unchanged enum blocks
- All 30 earlier audio source audits pass
- 20 Clang/GCC O0/O2 before/after comparisons across default, no reverb, no external audio, bridge enabled and PS2 shapes
- Four native MSVC x86/x64 comparisons across default and no reverb, plus compilation of actual game callers, enum consumers and adapters
- 4,096 operations per comparison cover both owners, both switches, repeated entity routing, direct C callers, every supported audio kind, unsupported representable kinds, zero/nonzero status, paused/unpaused routing, used/unused and initialized/uninitialized state, main/everywhere/interior/unknown areas, null physical pointers, vehicle/ped/other physical kinds, and stored-pointer changes after type queries
- Coverage assertions require every supported generator, both physical routes and weather-area reads to be reached, including the bridge generator when enabled
- Full zero-padded owner bytes with normalized opaque pointer identities and host-owned type/area values are observed at each callback and after operations
- Instrumented original area reads change the second comparison's area; physical type queries change the later stored pointer to another object or null, exposing an incorrectly cached pointer or added null gate
- Runtime audio-kind values remain inside the original C++ enum's representable range; entity IDs are bounded because the original internal dispatcher performs unchecked indexing
- One additional Clang O1 before/after comparison with undefined-behavior and floating-conversion checks in trap mode passes
- 510 unique consumer compilation checks across architecture, renderer, threading and decoder configurations
- All 24 prior sound-effects orchestration comparisons retain their original digests

Full game linking and playback were not tested. Individual sound generator behavior is controlled in this routing fixture and remains unchanged in production.
