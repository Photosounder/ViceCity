# Provider switch and ordered-volume C conversion

`AudioProvider.c` is an actual C11 translation unit replacing the AudioManager
SetCurrent3DProvider method. It receives initialization status, provider ID,
pointers to the live active-count/queue fields, and the owner's existing order,
count, and active-record arrays. It introduces no duplicate owner state.
Both DMAudio and the Miles host callback call this C interface directly.

The original sequence remains: stop channels using the old active count plus
one; clear the selected requested queue; switch the active queue; clear the
other requested queue; reset active records; then ask the backend to switch.
Only a strictly positive returned provider index queries the maximum channel
count. That value still narrows to uint8 before subtracting the reserved channel
when greater than one. Index zero and negative results retain the old active
count even though queues have already been cleared. There are no added guards
or changes to this legacy policy. The initialized flag is gated exactly as before;
external-3D-disabled builds return -1 without accessing queue state.

`AudioSoundVolume.c` is a separate actual C11 module implementing the former
AdjustSamplesVolume method. It takes the selected requested records, order,
and count. It visits records in the same order, skips nonzero 2D flags, and uses
the already converted attenuation kernel. Duplicate order entries retain their
original repeated attenuation behavior. Both old methods and declarations are
removed. Other owner methods, fields, construction, destruction, and startup
random draws remain unchanged.

Run `utils/tests/test_audio_provider.ps1`. The fixture compares the saved original
C++ methods with the actual C modules. Both sides link the existing reset and math
C modules, which were independently verified in earlier conversions. Mock backend
functions observe channel stop ordering, backend identity, requested IDs, queue
state visible at the backend switch, and maximum-channel query counts. This is a
control-flow comparison, not real provider switching or playback.

Tests cover every valid active capacity, both queue orientations, initialization
bytes 0/1/255, provider results -128/-1/0/1/127, maximum-channel results including
zero and uint32 wrap/narrowing cases, all provider-ID bytes, and repeated switches.
Every initialized byte of queue storage is compared, including retained record
fields, padding, inactive tails, and requested records. The ordered-volume pass
covers zero/full counts, reordered/duplicate indices, 2D flags, and repeated
attenuation with defined numeric inputs.

All sixteen Clang/GCC O0/O2 comparisons pass across default, vanilla,
external-3D-disabled, and reflections-disabled shapes. Native MSVC x86/x64
comparisons pass with default digest `10881872612676048543`. All 280 consumer
compile checks pass, as do native game integration compilation and the native
x86 Miles host callback compilation. Object symbols confirm matching C providers
and game/callback references. Source audits independently verify both C bodies
and all owner, header, facade, and host edits; earlier relevant source audits pass.

The existing Visual Studio project/filter registers both C11 sources and the
provider header. CMake/premake already collect C files. Saved sources, native
scripts, logs, and verification.json are in `build/audio-provider-c-tests`.
No full linked game build or in-game playback test has been performed.
