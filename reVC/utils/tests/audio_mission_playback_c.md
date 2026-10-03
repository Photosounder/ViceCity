# Mission playback in C

AudioMissionPlayback.c is an actual C11 translation unit implementing per-slot
playback and the ordered mission audio service. Both former cAudioManager methods
are removed. AudioManager service calls the C operation directly. Four C-linkage
host functions in AudioLogic.cpp bridge existing C++ camera and police-radio
operations. No owner layout or existing game geometry implementation changes.

The three byte counter arrays remain function-local statics shared across owners.
Loading, 120-frame failure timeout, 90-frame pretend playback, 30-frame startup
checking, post-decrement wraparound, pause edges, camera left/right sounds,
ROK2 restart, phone flags, and global volume restoration retain original statement
order. Timer reset retains duplicate stream stops. Startup positional volume uses
80 and continuous playback still uses MAX_VOLUME, matching the original code.

Run utils/tests/test_audio_mission_playback.ps1. The generator audits both complete
playback bodies, four host adapters, constants and sqrt implementation, full
AudioLogic and AudioManager caller edits, and the owner header against the saved
before snapshot. Controlled backends record every call, arguments and state at
each callback. Original member code and the actual C module use identical live
record fields and the existing C state and math kernels. A real C caller exercises
both exported operations and the shared global owner; baseline-only C bridges
reach the original member implementations for that comparison.

The trace covers both owners/slots, sample and status gates, all 512 Boolean flag
combinations, finite/nonfinite distance boundaries, bounded camera-space pan,
long timeout/startup/pause/restart sequences, and all byte values of the global
volume multiplier. Sixteen Clang/GCC O0/O2 comparisons cover default, vanilla,
no-external and PS2 record shapes. Native MSVC x86/x64 comparisons also pass.
All twelve earlier audio phase source audits pass. 330 consumer compilation checks
and native game consumer compilation pass in both architectures.

Backend and game geometry are controlled fixture callbacks. No full linked game
build, actual camera rendering or playback is verified. Invalid slot calls retain
the original unsafe behavior and are excluded. Production UsesPoliceChannel
always returns false; its retained police branches are statement-audited but are
not reached in the production-policy runtime trace.
