Saved radio data and music reload in C11
======================================

`MusicReload.c` owns the complete former `ResetMusicAfterReload` method and
the zero-initialized radio-static timer. `AudioReload.c` calls it directly
at the original point between the stream fades and player-speech reset.
The obsolete C++ callback and member declaration are removed.

`MusicSavedData.c` owns the two saved-radio arrays, the favorite-radio getter,
and all three saved-position helpers: initialize, get, and populate. The
arrays retain their element types, lengths, zero initialization and writable
representation. The C++ stats, frontend and save code use declarations from
`MusicSavedData.h`; serialized data still copies the same array bytes.
The class-owned favorite-radio array and getter are removed from `CStats`.

The reset retains its partial writes and early return. It first clears the
original playback flags and counters, captures every saved listening time,
and returns if all are zero. Otherwise it restores listen times and positions
one station at a time. Position `-1` remains a sentinel, the length comparison
remains strictly greater, and the original signed/unsigned conversions and
modulo remain intact. Diagnostics and pause-time reads retain their original
arguments and ordering. The existing C logger and pause-time adapter are
reused without adding C++ callbacks.

Validation
----------

`test_audio_music_reload.ps1` runs 16 original/C comparisons with Clang and
GCC, O0/O2, and four feature configurations. Each comparison exercises 10240
operation groups over two owners and all five complete operations, with 40375
callback observations. It hashes both complete owners, saved arrays and
shared counters before and after operations and callbacks. Inputs cover
signed zero, NaN payloads, infinity, negative positions, signed integer
limits, length boundaries and unsigned timer wraparound. Diagnostic, timer
and population callbacks mutate live state to distinguish captured listening
times from later saved-position and length reads. Explicit assertions check
the zero-listening-time early return and initialized saved-position sentinel.

Six MSVC comparisons use x86/x64 with `/O2 /GL /LTCG /OPT:REF`, and two Clang
comparisons use trapping undefined-behavior and float-cast-overflow sanitizers.
Lengths stay nonzero in paths performing modulo, retaining the original
caller's domain assumptions. Six actual C++ consumers also compile with MSVC
on both architectures, including stats, frontend and game-save code.

The existing audio reload regression runner compares 16 configurations of the
outer reset/cleanup operations, checking the direct C music-entry boundary
with the actual global C music record. All 73 preceding source audits pass
through the explicit migration chain.

The broad matrix checks 117 actual C/C++ consumers across ten configurations,
plus ten thread/decoder guards. Its Frontend check defines the source's
Windows input requirements before the single-thread preinclude reads
`common.h`, preserving the source's normal header setup. Full game linking
and interactive gameplay are outside these quiet checks.
