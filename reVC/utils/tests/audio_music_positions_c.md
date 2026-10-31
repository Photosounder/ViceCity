Radio statistics and position queries in C11
===========================================

`src/audio/MusicPositions.c` owns the complete former `RecordRadioStats`,
`GetTrackStartPos` and `GetRadioPosition` methods, plus the public
`DMAudio_GetRadioPosition` function. Callers enter the C operations directly.
The music owner's fields and constructor remain unchanged. The existing
`MusicInitHost_PauseTime` adapter supplies the pause timer; this phase adds no
C++ host adapter.

Behavior retained
-----------------

The position query first captures the stored position, then performs the
original live timer comparisons. The original `Min` macro re-evaluates its
first argument on the selected branch. Consequently this query can perform
three timer callbacks, and the third value can differ from the second.
The C macro preserves that evaluation rather than caching the elapsed time.
The 270000 cap, strict `position > length` check, unsigned wraparound, and
timer writes on the non-increasing path remain unchanged.

Statistics keep the original promotion from the unsigned timer to `double`,
comparison with the stored `float`, and addition back into the float listen
array. Radio queries retain the station bound and the public interface's
unsigned-to-signed conversion.

Validation
----------

Run `utils/tests/test_audio_music_positions.ps1` for 16 original/C comparisons
using Clang and GCC, O0/O2, and four feature configurations. Each comparison
exercises 16384 operation groups over two owners and observes 11173 timer
callbacks, hashing both complete records before and after callbacks and
operations. Tests cover initialized bytes 0/1/255, invalid stations, elapsed
time cap boundaries, equality with track length, wraparound, float rounding,
and callback mutations of live fields. Explicit assertions verify two versus
three timer reads and the strict equality behavior.

Six MSVC comparisons use x86/x64 with optimized C11/C++17 compilation,
`/GL`, `/LTCG`, and `/OPT:REF`. The sanitizer runner adds two Clang comparisons
with undefined-behavior and float-cast-overflow trapping. Fixture inputs avoid
zero-length modulo and out-of-range track indexing; production retains the
original caller assumptions.

The broad consumer matrix checks 111 C/C++ units across ten configurations,
with ten thread/decoder guard checks. All 72 preceding source audits pass
through the explicit migration chain. Full game linking and interactive
gameplay are outside these quiet checks.

The next self-contained operation is `ResetMusicAfterReload`, which restores
saved radio positions and listening statistics. It still depends on C++ stats
and save-data accessors and needs a separate diagnosis before conversion.
