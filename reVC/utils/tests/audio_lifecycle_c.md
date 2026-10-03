# Audio initialization and shutdown conversion

AudioLifecycle.c owns initialization, shutdown and the two DMAudio lifecycle facades as C11. The old owner methods were removed. The C++ destructor retains its initialization gate and calls the C shutdown function before its existing script-manager count reset. Four game setup/shutdown callbacks and two music callbacks remain narrow C++ adapters.

Initialization retains the pre-setup call before the backend, raw byte backend result assignment, channel-count narrowing into the owner byte before the <=1 gate, immediate shutdown on insufficient channels, reserved channel decrement, and game/police/music setup order. The non-external path retains the original generic-channel assignment.

Shutdown retains music termination before clearing entity-use flags and order indices, partial entity record retention, count resets before game pre-shutdown, ascending bank queries and conditional unloads, backend termination before clearing the initialized flag, and game post-shutdown afterward.

Verification:

- Independent complete raw-statement audits of both saved methods, destructor binding, game/music adapters, facade implementations and complete owner/header/facade edits
- The initializer has mutually exclusive preprocessor opening braces; its source audit delimits the complete raw function rather than treating both alternatives as simultaneously active
- All 24 previous audio source audits pass
- 16 Clang/GCC O0/O2 before/after comparisons across default, vanilla, no external audio and PS2 shapes
- Four native MSVC x86/x64 before/after comparisons across default and no external audio
- 640 operations per configuration cover initialized/uninitialized entry gates, failed and noncanonical byte startup results, channel limits 0/1/2/42/255/256/257/UINT32_MAX, immediate shutdown, repeated lifecycle calls, alternating owners, live global facades, signed nonzero bank statuses and ordered unloads
- Complete zero-padded owner and global police state are observed before every callback and after operations; the actual C police initialization module is linked and callback frame mutation verifies crime deadline timing
- 450 unique consumer compile checks across the existing architecture, renderer, threading and decoder matrix, plus native compilation of actual game callers, callback adapters and C owner on x86/x64

Full game linking and playback were not tested. Game setup/shutdown and music-manager business logic remain in their existing implementations and are represented by controlled callbacks in runtime comparisons.
