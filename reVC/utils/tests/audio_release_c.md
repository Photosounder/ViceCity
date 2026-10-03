# Releasing-sound carryover conversion

AudioRelease.c owns the former AddReleasingSounds method as C11. AudioEffects_Service calls it directly, replacing the old C++ trampoline. Three game callbacks retain entity position, camera line-of-sight testing and fixed time-step reads. The new C unit is registered in Visual Studio; CMake and premake already discover C sources.

The conversion preserves previous-queue selection, ordered traversal and matching by entity/counter without an added sample-index match. Finished sounds are skipped. Delayed reflections bypass attachment, fading, frame reduction and priority changes, then copy into the scratch sound and submit through the existing C request kernel.

Ordinary unmatched sounds retain physical-entity attachment, weighted camera distance with the original nonpositive square-root clamp, pedestrian visibility volumes, squared distance-ratio scaling, byte conversion before the maximum-volume clamp and existing signed-byte fade fields. Fixed and vanilla fading branches retain every time-step query, write and early exit. Static state, frame decrement and priority reduction remain in order. Stored owner and sound-record fields are unchanged.

Verification:

- Independent complete-body and helper audits, three complete game callbacks, full owner/header edits and the direct sound-effects caller
- All 27 earlier audio source audits pass; the sound-record audit follows the releasing consumer into its actual C translation unit
- 24 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no attachment, no external audio, no reflections and PS2 shapes
- Four native MSVC x86/x64 comparisons across default and no attachment, plus compilation of the actual game callers and adapters
- 768 operations per comparison exercise two owners, both queue roles, repeated carryover, direct C callers, matched entity/counter with a different current sample, finished sounds, delayed reflections, empty/full destination capacity, initialization state, physical/nonphysical/null/invalid entities, two-dimensional sounds, both pedestrian sample-range endpoints, both visibility results, priority thresholds, looped/nonlooped sounds, zero/negative/fractional/positive frames and multiple time steps
- Full zero-padded owner bytes with normalized entity pointers and host position bytes are observed at each callback and after operations
- Actual C entity lookup, geometry, scalar math, request submission and ordering modules are linked; the fixture controls game positions, rays, time and room state
- Fractional lifetimes remain covered with explicit fade values when dividing pedestrian volume by the lifetime would exceed the original signed-byte field; undefined original conversions are excluded from runtime equivalence claims
- One additional Clang O1 before/after comparison with undefined-behavior and float-cast-overflow instrumentation in trap mode passes for both the original member and actual C operations/primitives
- 480 unique consumer compilation checks across architecture, renderer, threading and decoder configurations
- All 24 prior sound-effects orchestration comparisons retain their original digests

Full game linking and playback were not tested. This migration preserves existing behavior, including the original byte conversion rules; it does not repair out-of-range fade inputs in production.
