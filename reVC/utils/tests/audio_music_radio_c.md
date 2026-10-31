Car-radio selection and tuning
=============================

`MusicRadio.c` owns the complete former `PlayerInCar`, `GetRadioInCar`,
`SetRadioInCar`, `GetNextCarTuning`, `GetCarTuning`, `UsesPoliceRadio` and
`UsesTaxiRadio` implementations. Their C++ members and declarations are removed.
The existing public get/set radio facades and stream-worker radio query also
compile in C. All remaining music callers enter these operations directly.
Stored audio and music record fields remain unchanged.

Three typed accessors expose the original station byte, existing siren rule
and script flag. Existing C boundaries provide player state, vehicle status,
vehicle model, player vehicle and replay state. The station accessor returns
the actual byte field, preserving compound-assignment narrowing before the
ordinary retuning loop. The previous-station configuration retains signed
wrapping before assignment. Neither path replaces its arithmetic with a
different modulo calculation. The discarded public MP3 availability query
and subsequent retune-counter reset remain in their original order.

Repeated player-vehicle lookups, captured vehicle pointers, model exceptions,
short circuit police/taxi rules, pause/replay gates and live random-table reads
remain unchanged. The public getter retains its byte return; the stream-worker
query retains its signed `int` result and original audio-backend guard.

Validation
----------

Twenty original/C comparisons use Clang and GCC, O0/O2 and five configurations:
default, vanilla, no external 3D sound, PS2 and previous-station scrolling
disabled. Eight native MSVC comparisons cover x86/x64 and four configurations
with `/O2 /GL /LTCG /OPT:REF`. Three Clang comparisons enable trapping
undefined-behavior and float-cast-overflow sanitizers.

Each comparison runs 20481 operation groups across two music owners and two
vehicles. Inputs cover every station byte, wide public station arguments,
raw initialized/paused flags, null/changing vehicles, excluded player states,
wrecked vehicles, all explicit police model exceptions, Kaufman taxis, siren
rules, replay, signed random-table limits and positive/negative retune presses.
Original callback results are captured before callbacks mutate later live
inputs. Complete owners, vehicles, provider storage, return values and callback
observations are hashed. The station pointer and script-flag accessors introduce
no simulated callback into an original plain field read.

Independent assertions verify uninitialized get/set gates, the maximum two
vehicle lookups, and station 255 plus one retune press: signed wrapping yields
3 when previous-station scrolling is enabled, while byte narrowing yields 0
when it is disabled. Both paths reset the retune press count afterwards.

The broad matrix compiles 122 actual C/C++ consumers across ten configurations.
Nine actual C++ consumers, including the modified DMAudio and sample-host
sources, compile with MSVC on both architectures. Ten thread/decoder guard
checks and all 77 preceding source audits pass. Full game linking and
interactive gameplay are outside these quiet checks.
