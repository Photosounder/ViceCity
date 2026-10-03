# Audio service and deferred timer reset conversion

AudioService.c owns the top-level service loop, deferred game-timer reset and DMAudio_Service as C11. The two old owner methods were removed, and game post-initialization now calls the C timer helper at the same point. Ped field/type access, sound-effects service, music and user-pause access remain narrow C++ callbacks.

Service retains five unconditional RNG draws before either gate. Deferred game timer reset runs even for an uninitialized owner; music reads the live timer afterward, then the timer-reset flag is cleared. The initialized gate is read after those callbacks. Previous pause is captured before querying user pause, and reflection, sound effects and music remain in order without additional initialization checks.

Deferred game timer reset retains ordered entity traversal, physical-type filtering without an added used-flag check, the original ped test, previous-start write before randomized next-start evaluation, signed remainder with unsigned timer addition, both mission clears and unconditional police channel stop.

Verification:

- Independent complete audits of both bodies, all callback definitions, facade and post-setup caller edits
- All 25 earlier audio source audits pass
- 20 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no reflections, no external audio and PS2 shapes
- Four native MSVC x86/x64 comparisons across default and no reflections
- 1,024 operations and 3,840 RNG draws per configuration cover initialization/reset/pause gates, repeated calls, alternating owners, global C facade, bounded reordered entities, non-peds and nonphysical types, unused physical records, signed random inputs, uint32 timer wrap and direct deferred reset
- Full zero-padded owner state is observed with normalized opaque ped identities, plus ped timer values at callbacks and after operations
- Instrumented ped field assignments change random input after the previous-start write, while callbacks change the live timer and initialization state to expose premature reads or extra gates
- Actual C mission state and position primitives are linked. The unchanged production RNG fill body is extracted into a standalone C unit, excluding unrelated owner-initialization dependencies; myrand inputs are controlled
- Reflection and sound-effects behavior is controlled here; actual reflection arithmetic has its own before/after fixture and sound-effects business logic remains in C++
- 460 unique consumer compilation checks across the existing architecture, renderer, threading and decoder matrix, plus native compilation of actual game callers, ped access adapters and C owner on x86/x64

Full game linking and playback were not tested. Ped layout, game timer storage, music and sound-effects business implementations were not modified.
