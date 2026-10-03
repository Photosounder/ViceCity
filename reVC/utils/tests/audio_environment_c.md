# Reverb and special sound service conversion

AudioEnvironment.c owns the former ProcessReverb and ProcessSpecial methods as C11. AudioEffects_Service calls both C operations directly. Six small game adapters retain replay state, player mood, focused remote vehicle, player audio-entity ID, entering-car state and the live in-vehicle flag. The existing police player-lookup adapter is reused. Visual Studio registers the new C unit; CMake and premake already discover C sources.

Reverb retains the update call before the dynamic-modeling flag read, channel-count configuration, live per-channel reverb eligibility and PS2 exclusion of channel flag writes. Special handling preserves the pause transition's effects fade followed by music fade. The unpaused branch retains replay-before-mood, remote capture before player lookup, two separate audio-entity ID reads, entity-used and entering-car gates, then the live in-vehicle flag and cached remote pointer.

The desktop player-engine channel was a local AudioLogic.cpp macro expanding to m_nActiveSamples, rather than the PS2 enum value. The C unit retains that exact distinction with a local macro scoped to this translation unit. Stored owner and record fields are unchanged.

Verification:

- Independent complete-body audits, six game/field adapters, the actual desktop channel macro, full owner/logic/header edits and the direct C service calls
- All 29 earlier audio source audits pass
- 20 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no reverb, no external audio and PS2 shapes
- Four native MSVC x86/x64 comparisons across default and no reverb, plus compilation of the actual game callers and adapters
- 2,048 operations per comparison cover both owners, independent and combined C operations, repeated special service, pause transitions, replay results, missing players, negative/valid player audio IDs, unused entities, entering-car/in-vehicle gates, focused remote vehicles, reverb return values, dynamic-modeling changes and zero/nonzero reverb eligibility
- Instrumented original field conversions observe both audio-entity reads and the in-vehicle read; host-owned pointer identities are observed as booleans
- Backend reverb update changes the later dynamic flag; channel writes change later reverb eligibility; effects fading changes pause flags; mood changes the already-entered pause state; player lookup changes focused remote state; entering-car queries change the later vehicle flag
- One additional Clang O1 before/after comparison with undefined-behavior and floating-conversion checks in trap mode passes
- 500 unique consumer compilation checks across architecture, renderer, threading and decoder configurations
- All 24 prior sound-effects orchestration comparisons retain their original digests

The original non-FIX_BUGS desktop reverb loop uses NUM_CHANNELS_GENERIC+1 for an array sized NUM_CHANNELS_GENERIC. GCC diagnoses the overread in both old and new bodies. That branch is preserved, but gated off in vanilla runtime tests; the defined fixed, no-reverb and PS2 branches are tested normally. Full game linking and playback were not tested.
