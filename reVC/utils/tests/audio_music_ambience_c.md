Music ambience selection and volume in C11
=========================================

`MusicAmbience.c` owns the complete former `SetUpCorrectAmbienceTrack`,
`GetHeightScale` and `ComputeAmbienceVol` implementations. Volume output now
uses a `uint8_t *`, and the existing C++ service callers pass their local
volume byte by address. The converted member declarations and C++ bodies
are removed. The music owner's stored fields and constructor remain intact.

The riot position is writable C position storage initialized to the same
three float values. All users of the former vector constructor and its
subtraction/magnitude operations are now in C. Scalar differences retain the
original `x*x + y*y + z*z` expression, and square roots use the same `sqrtf`
operation as the original math wrapper.

The existing camera-position copy and wind getter are reused. Six typed
C++ field readers supply area, riot intensity, beach flag, old/new weather
and camera distance to water. They contain only the original field reads;
branching, calculations and state changes live in C. The original weather
enum is shared through `WeatherTypes.h`, and the existing C area enum and
sample-volume constant are reused.

The static ramp remains shared across owners. A riot return occurs before
resetting or advancing it. The original 1/20 increments, 60 cap, weather
attenuation, threshold comparisons and byte conversions are retained.
Height queries preserve each original camera read, and weather conditions
retain their short-circuit behavior and repeated wind reads.

Validation
----------

The runtime runner compares original/C implementations with Clang and GCC,
O0/O2 and four configurations: 16 comparisons. Each executes 18432 operation
groups over two owners, three output-storage choices and all three complete
operations, observing 56943 typed reads. Both complete records, world inputs,
riot coordinates and output bytes are hashed at reads and operation bounds.
The original fixture uses the actual vector and float-math headers, with
typed read observers for game fields. Reads capture their result before
mutating later inputs, owner fields and output bytes.

Cases cover all area choices and unknown areas, riot distance boundaries
20/65/100, height boundaries 20/50, water boundaries 40/90/140, hurricane
old/new combinations, wind boundaries, initialized/reset bytes 0/1/255 and
track-range boundaries. Explicit assertions verify the shared ramp across
owners, nearby-riot return before reset, and reset after leaving riot audio.
Float-to-byte inputs stay within the original defined arithmetic domain;
this conversion does not add clamps or change undefined out-of-range casts.

Six MSVC comparisons use x86/x64 with `/O2 /GL /LTCG /OPT:REF`. Two Clang
comparisons enable trapping undefined-behavior and float-cast-overflow
sanitizers. Six actual C++ consumers compile with MSVC on both architectures.
The broad matrix compiles 119 actual C/C++ consumers across ten configurations,
with ten thread/decoder guard checks. All 74 preceding source audits pass
through the explicit migration chain. Full game linking and interactive
gameplay remain outside these quiet checks.
