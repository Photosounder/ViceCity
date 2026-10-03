# Police scheduler and zone reports in C

AudioPoliceReports.c implements ServicePoliceRadio, SetupCrimeReport and
PlaySuspectLastSeen as actual C11 operations. The coordinate-based public DMAudio
facade is also C. AudioManager service directly calls the C scheduler. Vehicle
suspect-report construction and other game operations remain in PolRadio.cpp.

The scheduler's unsigned last-seen counter remains a function-local static shared
across owners. Crime processing precedes replay/player checks exactly as before.
Each player lookup retains its original short-circuit position, including the
separate third lookup before the wanted getter. Paused service passes the original
zero wanted level to the channel operation. Nonzero wanted values still narrow to
its byte argument; unsigned logical-frame subtraction and signed random remainder
before assigning the next unsigned delay are preserved.

AudioPoliceGame.h provides a pointer-only zone view referring to the game's name
and four bounds fields. The C code reads those fields at the original formula
sites. GetZone performs the original game lookup; the view does not copy numeric
bounds. FindZone constructs the same temporary game coordinates; the inspected
FindAudioZone/PointLiesWithinZone implementation does not mutate them. Existing
queue capacity gates, crime remapping, FIX_BUGS sample offset, strict quarter-zone
comparisons, record clearing/aging and special-report flag writes are unchanged.
The crime report retains its original lack of an initialization gate; coordinate
reports retain their existing gate.

Run utils/tests/test_audio_police_reports.ps1. It independently audits all three
complete bodies, the public facade, eight conditional game adapters and complete
caller/header edits against saved sources. Original member code and actual C
share the existing C police state/crime aging and controlled game callbacks. The
trace observes queue samples/cursors, every crime field, shared report state,
player/wanted/replay/timer query order and final byte wanted values. A real C
caller reaches the three operations and public coordinate facade. Native builds
also check the five-pointer zone view layout.

Sixteen Clang/GCC O0/O2 original-versus-C comparisons cover default, vanilla,
no-external and PS2 records. Native MSVC x86/x64 comparisons pass. The trace covers
both owners, all crime types, queue capacity boundaries, quarter-zone equality
and adjacent values, invalid zone IDs and unmatched names, replay/player gates,
signed wanted boundaries, retained fields, cross-owner scheduling, signed random
values and unsigned logical frame values. Sixteen prior source audits and 370
consumer compilation checks pass; native game callers compile in both architectures.

Game objects, requested report construction and channel service are controlled
callbacks. Unsafe vanilla null-wanted inputs are excluded. No full linked game
build, actual game zone traversal or audio playback is verified by this fixture.
