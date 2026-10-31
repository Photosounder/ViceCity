Music mode changes and frontend streaming service
================================================

`MusicModes.c` owns the complete former `ChangeMusicMode` and
`ServiceFrontEndMode` implementations and the public `DMAudio_ChangeMusicMode`
facade. The C++ members and declarations are removed. The service dispatcher
calls the C frontend function with the original owner. No new C++ adapter is
introduced, and stored fields remain unchanged.

The conversion preserves frontend radio pause updates, announcement cleanup,
frame gates, delayed stream acknowledgements, repeated cutscene stop polling,
MP3 restart, mission-completed mode changes, volume latency and ramping. The
frontend statistics flag remains shared across owners and survives mode changes.
Provider arguments retain the original mixture of captured and live track reads.

Native MSVC requires an explicit provider-result temporary before selecting
the destination of dynamic track position/timestamp writes. The direct C
assignment selected the destination earlier in the complete converted body,
which differed from the original C++ callback trace. GNU and Clang retain the
direct expression, which matches their original implementations. This choice
preserves the observed behavior of each tested compiler; it does not assert
portable ordering for those original expressions.

Validation
----------

The runtime runner compares the original C++ bodies with the C implementation
using Clang and GCC, O0/O2, and five configurations: default, vanilla,
no external 3D sound, PS2 and frontend radio pause disabled. Each comparison
exercises 9216 operation groups across two owners, all 256 mode bytes,
raw boolean flags, wrapping timers, signed position returns, stream polling,
volume boundaries and callback-driven changes to live tracks and owner fields.
Complete music/audio records and callback observations are hashed. Both models
link the actual C state and track-position/statistics helpers.

Eight native MSVC comparisons cover x86/x64 and four configurations with
`/O2 /GL /LTCG /OPT:REF`. Three Clang comparisons enable trapping
undefined-behavior and float-cast-overflow sanitizers. The broad matrix compiles
121 actual C/C++ consumers across ten configurations, and seven actual C++
consumers compile with MSVC on both architectures. Ten thread/decoder guard
checks and all 76 preceding source audits are also required.

These quiet checks do not establish full game linking or interactive gameplay.
