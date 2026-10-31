Club track positioning and direct C script music access
======================================================

`MusicClubs.c` owns the complete former `SetMalibuClubTrackPos` and
`SetStripClubTrackPos` implementations. `AudioScriptSounds.c` enters those
functions directly with the original global music owner and script byte.
Music-mode and playing-track gates use the existing C state getters.
All four `AudioScriptSoundsHost_*` music wrappers and their declarations
are removed, along with the converted music members. No new C++ adapter
is introduced. Both audio and music record fields remain unchanged.

The default position write when music is uninitialized remains before the
next/playing-track guard. Guarded tracks preserve their timer, while unknown
script bytes still update the timer when the guard permits it. Each selected
position retains the original signed `% 128` expression and track offset.
The pause timer is read after position selection, and its captured return
is assigned after callbacks complete, using the existing timer adapter.

Validation
----------

The runtime runner performs 16 original/C comparisons with Clang and GCC,
O0/O2 and four configurations. Each exercises 16384 operation groups over
two owners and both complete operations, with 4096 timer observations.
Inputs cover every script byte, initialized flags 0/1/255, all next/playing
track guard combinations, positive/negative remainder boundaries, signed
integer limits and wrapping timers. Complete audio and music records are
hashed before/after operations and at timer reads. Timer callbacks mutate
positions, tracks, flags, random input and the argument's source storage.
Assertions independently verify the guarded timer, early default position
and final captured timer write.

Both variants link the actual C state getters, including mode and current
track reads. Six MSVC comparisons use x86/x64 with `/O2 /GL /LTCG /OPT:REF`.
Two Clang comparisons enable trapping undefined-behavior and float-cast-
overflow sanitizers. Seven actual C++ consumers, including `AudioLogic.cpp`,
compile with MSVC on both architectures.

The existing script-sound regression runner compares 28 compiler/configuration
combinations plus its sanitizer comparison. Its isolated music-entry observers
check the actual global C record pointer and retain the earlier read/setup
callback tokens. The complete club operations are independently tested above;
the regression checks their placement among sample writes, frequency queries
and queue submissions. Each regression comparison runs 49152 cases.

All 75 preceding source audits pass through the explicit migration chain.
The broad matrix checks 120 actual C/C++ consumers across ten configurations,
plus ten thread/decoder guard checks. Full game linking and interactive
gameplay remain outside these quiet checks.
