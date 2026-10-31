Announcement playback and track-switching service
================================================

`MusicStream.c` owns the complete former `ServiceAnnouncement` and
`ServiceTrack` implementations. Both C++ members and declarations are removed,
and gameplay service enters the C functions with its original captured music
owner, vehicle and pedestrian. Stored record fields remain unchanged.

Three typed readers expose the original slow-motion gate, camera target and
target coordinates. Distance uses the original unweighted sum of float squares;
the general audio distance helper's vertical weighting is not applicable here.
The original `Sqrt` wrapper is `sqrtf`. Panning retains the existing C camera
transform and pan calculation, including a fresh target-position read after
distance and ducking decisions. Plain coordinate/target access adds no simulated
callback in the comparison fixture.

Shared statistics flags persist across calls and music owners. Announcement
stop/start acknowledgement, retuning stops, even-frame track changes, captured
start positions, live next-track reads, radio-station writes, ambience ramping,
hurricane sound submission, debug text, ducking, cutscene recovery, latency and
volume ramps retain their original order. Dynamic track writes use the same
compiler-specific sequencing already established for music modes: native MSVC
captures the provider result before selecting the live destination; GNU and
Clang retain their original direct assignment expression.

Validation
----------

Sixteen original/C comparisons use Clang and GCC, O0/O2 and four configurations:
default, vanilla, no external 3D sound and PS2. Six optimized native MSVC
comparisons cover x86/x64 and three configurations with `/GL /LTCG /OPT:REF`.
Two Clang comparisons enable trapping undefined-behavior and float-cast-overflow
sanitizers.

Each comparison exercises 12288 operation groups across two music owners and
two vehicle records, with 47792 callback observations. One pass mutates live
owner/world inputs after captured callback results; another repeats service
without resetting state. Both models link the actual C state, positions,
ambience, geometry, pan, mission-state and mission-position units. Complete
audio/music/provider records, vehicles, target coordinates, camera inputs,
returns and sample arguments are hashed. Debug and hurricane observers verify
the original message and owner storage/arguments at those boundaries.
Station bytes are hashed as explicit four-byte scalars, respecting the shared
word hash's buffer-size requirement.

Four independent checks verify vertical distances 55, 10 and 20, ducked
mid-distance byte volume, and the null-target branch's omitted volume write
while the volume ramp still advances. Broader cases cover delayed stream
acknowledgements, shared flags, signed positions, wrapping timers, ambient/radio
tracks, mission ducking, raw flags, byte volumes, latency and cutscene counters.
Inputs keep original array accesses, float-to-byte casts and pan conversions
inside their defined domains.

The broad matrix checks 123 actual C/C++ consumers across ten configurations.
Nine actual C++ consumers compile with MSVC on both architectures. Ten
thread/decoder guard checks and all 78 preceding source audits also pass.
Full game linking and interactive gameplay remain outside these quiet checks.
