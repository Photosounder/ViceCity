# Requested sound queue conversion

AudioRequests.c now owns requested sound submission and reflection expansion in C11. The corresponding two cAudioManager methods and the police queue host wrapper were removed. Every game and C police caller submits its existing live owner directly to C. Two conditional C++ scalar adapters retain room and slow-motion access.

The original behavior is preserved: invalid samples are ignored; priority arithmetic retains unsigned wrap; a full queue replaces the worst physical slot only on a strict priority improvement; all sound mutation and insertion order remain unchanged. Room checks keep their original short circuit. Reflection submission remains recursive after clearing the reflection flag. Reflection expansion restores only position and distance, retaining changes to frequency, counter, priority, volume, delay and maximum distance. Slow motion is queried at every original decision point.

Verification:

- Independent complete statement audits against both saved owner bodies, all caller/header edits, and game adapters
- All 21 earlier source audits pass, including the earlier position and sound audits now comparing reflection statements against their actual C implementation
- 24 Clang/GCC O0/O2 before/after comparisons across default, vanilla, unscaled timing, no external 3D audio, no reflections, and PS2 configurations
- Native MSVC x86/x64 default and unscaled before/after comparisons
- Complete zero-initialized owner storage is observed at room/timer callbacks and after operations, including both queues and retained sound fields; tests exercise queue capacities, full replacement, rejected priorities, volume byte range, invalid samples, unsigned counter and priority wrapping, recursive reflections, and alternating owners
- 420 unique consumer compilation checks across the existing architecture, renderer, threading and decoder matrix
- Native x86/x64 compilation of actual game callers and C owner; both default and unscaled game adapters compile
- All 20 police-channel regressions pass using the existing controlled submission behavior; production submission is tested separately here

The original non-FIX_BUGS reflection code can shift an uninitialized oldFreq. That source behavior was retained and excluded from defined runtime comparisons. This phase does not claim equivalence for undefined behavior. Full game linking and playback were not tested.
