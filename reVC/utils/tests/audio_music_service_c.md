Music dispatcher and gameplay service
=====================================

`MusicService.c` owns the complete former `Service` and `ServiceGameMode`
implementations and four scalar music globals. The C++ methods and obsolete
`AudioServiceHost_Music` wrapper are removed. `AudioService.c` enters the C
music operation directly after sound-effects service. Stored record fields
remain unchanged.

Seven typed C++ control readers preserve the original keyboard, pad, mouse
binding and opposite-wheel queries. Existing C boundaries provide pedestrian,
vehicle, radio, world and timer access. The exact original `RsKeyCodes` enum
and typedef now live in the shared C header `RsKeys.h`.

The conversion retains repeated live queries, captured pedestrian/vehicle
pointers, shared statistics flags, pause/reset gates, signed retune loops,
raw flag bytes, track transitions and wrapping timers. Dynamic indexed track
writes use the compiler-specific sequencing already established for music
modes and stream service: native MSVC captures the provider result before
selecting the live destination; GNU and Clang retain the direct expression.

Validation
----------

Twenty original/C comparisons cover Clang and GCC, O0/O2 and default, vanilla,
no external 3D sound, PS2 and disabled previous-station scrolling. Eight
optimized native MSVC comparisons cover x86/x64 and four configurations with
whole-program optimization. Three Clang comparisons enable trapping
undefined-behavior and float-cast-overflow sanitizers.

Each comparison exercises 6144 operation groups across two music owners,
two vehicles and two pedestrians. Callback counts are 33746 by default,
32471 for PS2 and 33128 without previous-station scrolling. One pass mutates
live state after captured results; another repeats service without resetting
state. Both models link the actual C child operations. Observers compare full
owner/provider/world records, controls, vehicle and pedestrian state, scalar
music globals and callback arguments/order.

Independent startup checks verify the original radio-static counter value
and timer reset before initialization/disabled gates. Inputs keep the original
array accesses and captured-null-vehicle paths inside their defined domains;
callback mutations cannot manufacture an original null dereference.

Twenty outer audio-service comparisons and one sanitizer comparison verify
that the direct C entry preserves its dispatch position. The broad matrix
checks 124 actual C/C++ consumers across ten configurations. Nine actual C++
consumers compile with MSVC on both architectures. Ten thread/decoder guards
and all 79 preceding source audits pass. Full game linking and interactive
gameplay remain outside these quiet checks.
