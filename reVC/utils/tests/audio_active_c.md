# Active-channel queue processing conversion

AudioActive.c owns the former ProcessActiveQueues method as C11. AudioEffects_Service calls it directly, removing the old C++ trampoline. The new code uses the existing C geometry and scalar math operations, the existing game time-scale callback, and one new camera-switch callback. Visual Studio registers the translation unit as C11; CMake and premake already discover C sources.

The conversion preserves the cached time scale, requested/active playing-flag resets, entity/counter/sample matching, parity-based channel completion checks and loop frame updates. Reused channels retain distance-before-doppler ordering, signed frequency and volume clamps, phone-call multiplier rules, spatial/pan updates, reverb and the static-sound fallthrough behavior.

Startup preserves stale-channel stops, live entity-used checks, delayed reflections, rotated channel selection, sample-length and zero-frequency handling, active-record copies, backend initialization failure behavior and all channel configuration calls. Every original use of the search index and rotated index is retained, including the non-external-audio branches. PS2 offset accumulation and the final time-scale frequency pass remain in order. No owner or sound-record fields were changed.

Verification:

- Independent complete conditional-body audit, exact original minimum/maximum/clamp macros, camera callback, full owner/header edits and the direct service caller
- The extractor uses the function's final macro-undef marker so mutually exclusive PS2 initialization braces do not corrupt body boundaries
- All 28 earlier audio source audits pass; the sound-position consumer audit follows playback into its actual C implementation
- 24 Clang/GCC O0/O2 before/after comparisons across default, vanilla, unscaled audio, no external audio, no reflections and PS2 shapes
- Four native MSVC x86/x64 comparisons across default and no external audio, plus compilation of the actual game callers and camera adapter
- 1,536 operations per comparison cover both owners, both queues, direct C callers, repeated processing, empty/partial/full requested counts, active capacities 1/2/4/8, rotated offsets, completed/playing channel flags, static/dynamic and 2D/3D records, matching/mismatching identities, stale channels, unused entities, finished/no-sample records, looped/nonlooped playback, delayed reflections, zero/sub-frame frequencies, unsigned sample-length multiplication, zero/nonzero time scale, pause state, camera switching, phone calls, volume multipliers and all byte-valued initialization result shapes
- Full zero-padded owner and controlled backend channel bytes are observed at every callback and after each operation, along with exact scalar argument bits and selected channel indices
- Initialization callbacks change phone-call state and frequency callbacks change the live volume multiplier before later reads
- Actual C camera transformation and audio math modules are linked with controlled backend/game boundaries
- One additional Clang O1 before/after comparison with undefined-behavior and floating-conversion checks in trap mode passes
- 490 unique consumer compilation checks across architecture, renderer, threading and decoder configurations
- All 24 prior sound-effects orchestration comparisons retain their original digests

Full game linking and playback were not tested. Invalid zero active-channel/time-spent divisors and other undefined original inputs are outside the runtime fixture; this conversion retains the original behavior rather than introducing repairs.
